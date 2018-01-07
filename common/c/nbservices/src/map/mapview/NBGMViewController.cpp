/*
Copyright (c) 2018, TeleCommunication Systems, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
   * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the TeleCommunication Systems, Inc., nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED. IN NO EVENT SHALL TELECOMMUNICATION SYSTEMS, INC.BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!--------------------------------------------------------------------------
 @file     NBGMViewController.cpp
 */
/*
 (C) Copyright 2011 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
/*! @{ */

#include "palstdlib.h"
#include "palmath.h"
#include "paldisplay.h"
#include "NBGMViewController.h"
#include "NBUIConfig.h"
#include "Frame.h"
#include "nbgmmapviewprofile.h"
#include "StringUtility.h"
#include "nbmacros.h"
#include "nbgmmapcamerahelper.h"
#include "nbgmlinearinterpolator.h"
#include "nbgmdecelerateinterpolator.h"
#include "nbgmmapscaleutility.h"
#include "nbgmacceleratedecelerateinterpolator.h"
#include <algorithm>

static const float BUBBLE_POSITION_EPSILON   = 1.f; // The epsilon of bubble position is 1 pixel.

extern "C"
{
#include "transformutility.h"
}

using namespace nbcommon;
using namespace nbmap;

/*
    @TODO: We have to fix all those warnings. But we have to be careful, casting the values causes the tile download to fail!!!
*/
// "Warning C4244: Conversion ... Possible loss of data"
#pragma warning(disable: 4244)


// Uncomment define to enable profiling for NBGM
/*
    This has to be enabled in order for the frame-rate to be calculated in the Debug-UI. We should refactor this to allow
    the retrieval of the render-time from NBGM without having to enable all the profiling!
 */
#define ENABLE_PROFILING_FOR_NBGM

// Uncomment define to enable loaded tile output
// #define DEBUG_OUTPUT_LOADED_TILES

#define SCREEN_WIDTH  PAL_DisplayGetWidth()
#define SCREEN_HEIGHT PAL_DisplayGetHeight()

//TODO: AVATAR_ANIMATION_DURATION is temp code for the current decelerate interpolation method.
// If set to 1000, smooth effect of avatar movement will not be so obvious.
// We need linear interpolation or better method in future.
#define AVATAR_ANIMATION_DURATION 1500

//It is a mercator value of infinite close to latitude 85.
#define MERCATOR_Y_BOUND 3.1313013314716454

// Custom avatar ID
#define CUSTOM_AVATAR_ID "CustomAvatarID"

//Material Category Name
#define DEFAULT_PIN_MATERIAL "NBM_PIN_MATERIALS"
#define PIN_MATERIAL_CATEGORY "pin"

static const string COMMON_MATERIAL_CATEGORY("MAT");

// Local functions and data structures.
/*! Helper class to convert a DataStream into binary buffer.

    @todo: Remove this conversion if NBGM is able to accept dataStream directly.
 */
class BinaryData
{
public:
    BinaryData(const DataStreamPtr& dataStream)
            : m_buffer(NULL),
              m_size(0)
    {
        if (dataStream &&
            ((m_size = dataStream->GetDataSize()) > 0) &&
            ((m_buffer = CCC_NEW uint8[m_size]) != NULL))
        {
            uint32 remainedSize = m_size;
            uint8* pos = m_buffer;
            while (remainedSize > 0)
            {
                uint32 receivedDataSize = dataStream->GetData(pos, 0, remainedSize);
                remainedSize -= receivedDataSize;
                pos += receivedDataSize;
            }
        }
    }

    virtual ~BinaryData()
    {
        if (m_buffer)
        {
            delete[] m_buffer;
        }
    }

    uint8* GetBuffer()
    {
        return m_buffer;
    }

    uint32 GetSize()
    {
        return m_size;
    }

private:
    uint8  *m_buffer;
    uint32  m_size;
};


/*! A Listener class for nbgm tile loading finished.
 */
class TileLoadingListener: public NBGM_TileLoadingListener
{
public:
    TileLoadingListener(NBGMViewController &controller):m_Controller(controller),
                                                        m_ReloadTileFlag(false)
    {
    }

    ~TileLoadingListener()
    {
    }

public:
     void SetReloadFlag(bool flag)
     {
         m_ReloadTileFlag = flag;
     }

     void OnLoadingFinished(const std::string& tileID)
     {
         //@todo: It's temp solution to filter tiles, we should figure out a better way to solve this issue.
         // For now we only need to care about pin tile loading.
         if(tileID.find("PIN") != string::npos)
         {
             m_Controller.SelectPendingPin();
             // Force to select the current pin after reloading tiles.
             m_Controller.SelectCurrentPin();
         }
     }
private:
    NB_DISABLE_COPY(TileLoadingListener)
    NBGMViewController &m_Controller;
    bool m_ReloadTileFlag;
};

class MaterialRemover
{
public:
    MaterialRemover(NBGM_MapView* mapView)
            : m_pMapView(mapView)
    { }

    virtual ~MaterialRemover()
    { }

    void operator() (const MaterialInfo& material) const
    {
        if (m_pMapView && !material.m_categoryName.empty() && !material.m_materialName.empty())
        {
            m_pMapView->UnloadExternalMaterial(material.m_categoryName, material.m_materialName);
        }
    }

private:
    NBGM_MapView *m_pMapView;
};

// NBGMViewController ............................................................................................................

NBGMViewController::NBGMViewController(NBGM_MapView* nbgmMapView, MapViewCamera* mapViewCamera)
    : m_nbgmMapView(nbgmMapView)
    , m_mapViewCamera(mapViewCamera)
    , m_currentZoomLevel(0.0)
    , m_materialsAreLoaded(false)
    , m_bubblelistener(NULL)
    //, m_selectedPinId()
    , m_pinVisibleOnScreen(false)
    , m_staticPoiVisibleOnScreen(false)
    , m_avatarMode(NBGM_AM_MAP_FOLLOW_ME)
    , m_avatarState(NBGM_AS_DISABLED)
    , m_customAvatar(NULL)
    , m_customAvatarVisibility(false)
    , m_profilingRender(false)           // Set to 'false' for production code!
    , m_profilingLoad(false)             // Set to 'false' for production code!
    , m_layerVisibility(false)
    , m_animationLayerOpacity(100)      // Set to full opacity by default
    , m_locationBubbleInfo()
    , m_background(false)
    , m_useTileService(false)
    , m_AvatarUpdate(false)
    , m_PinPendingFlag(FALSE)
    , m_TileLoadingListener(new TileLoadingListener(*this))
    , m_preBubblePositionX(-1.f)
    , m_preBubblePositionY(-1.f)
    , m_showBubble(false)
    , m_gestrueProcessing(false)
{
    nsl_memset(&m_avatarScale, 0, sizeof(m_avatarScale));
    nsl_memset(&m_avatarLocation, 0, sizeof(m_avatarLocation));
}

NBGMViewController::~NBGMViewController()
{
    if (m_nbgmMapView)
    {
        delete m_nbgmMapView;
    }
}

NB_Error
NBGMViewController::Initiallize()
{
    m_nbgmMapView->Initialize();
    NBGM_MaterialParameters params;
    params.categoryName = PIN_MATERIAL_CATEGORY;
    params.materialName = DEFAULT_PIN_MATERIAL;
    params.buffer = 0;
    params.bufferSize = 0;
    params.filePath = string(NBGM_GetConfig()->workSpace) + DEFAULT_PIN_MATERIAL;
    m_nbgmMapView->LoadExternalMaterial(params, NBGM_MLF_NONE);
    m_nbgmMapView->SetCurrentExternalMaterial(PIN_MATERIAL_CATEGORY, DEFAULT_PIN_MATERIAL);

    // TEST !!!!
    m_nbgmMapView->EnableProfiling(0);

    return NE_OK;
}

NB_Error
NBGMViewController::Finalize()
{
    if(m_nbgmMapView)
    {
        m_nbgmMapView->Finalize();
        return NE_NOTINIT;
    }
    return NE_OK;
}

bool
NBGMViewController::LoadMapData(TileWrapperPtr tileWrapper)
{
    /*
      This function gets called in the context of the rendering thread!
    */
    if (!tileWrapper || !tileWrapper->m_tile)
    {
        return true;
    }

    // Load tiles until the common materials has already been loaded.
    if (m_materialsAreLoaded)
    {
        // Add the new tile to the render list and the load list
        m_tilesToLoad.push_back(tileWrapper);
        return true;;
    }

    // Do nothing because this is not MAT and MAT is not loaded yet. According to NBGM
    // team, we need to explicitly make sure only load other tiles after MAT is loaded.
    // Caller should be responsible to schedule a load operation again. Or there may be
    // occational blank screen at startup!

    cout << "IGNORE TILE: Should not load tiles before MAT tiles loaded: "
         << tileWrapper->m_tile->GetContentID()->c_str() << endl;

    return false;
}

/*! Unload tiles from NBGM */
void
NBGMViewController::RemoveMapData(const vector<TilePtr>& tiles        /*!< Tiles to unload */
                                  )
{
    /*
        This function gets called in the context of the rendering thread!
     */

    // Add tiles to unload vector
    m_tilesToUnload.insert(m_tilesToUnload.end(), tiles.begin(), tiles.end());
}

/* See header for description */
bool
NBGMViewController::LoadAndUnloadTiles()
{
    /*
        This function gets called in the context of the rendering thread!
     */

    bool tilesModified = false;

    size_t tilesToLoad = 0;
    size_t tilesToUnload = 0;

    m_profilingLoad.RecordTick(0);

    tilesToLoad = m_tilesToLoad.size();
    tilesToUnload = m_tilesToUnload.size();

    m_profilingLoad.RecordTick(1);

    if (tilesToLoad > 0)
    {
        // Check if we need to load any new tiles
        for (size_t i = 0; i < m_tilesToLoad.size(); ++i)
        {
            LoadTile(m_tilesToLoad[i]);
            tilesModified = true;
#ifdef DEBUG_OUTPUT_LOADED_TILES
            const TilePtr& tile =  m_tilesToLoad[i]->m_tile;
            const TileKeyPtr& key  = tile->GetTileKey();

            NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelDebug, "[map_tile_perf] NBGM x:%d,y:%d,z:%d", key->m_x, key->m_y, key->m_zoomLevel);

            // Insert the loaded tile. This ignores if it is already there.
            pair<set<string>::iterator, bool> result = m_debugLoadedTiles.insert(m_tilesToLoad[i]->m_tile->GetContentID()->c_str());
            if (! result.second)
            {
                NB_TEST_LOG(PAL_LogBuffer_0, PAL_LogLevelError, "ERROR: ADDED duplicate tile to NBGM!!!!");
            }
#endif
        }
        m_tilesToLoad.clear();

#ifdef DEBUG_OUTPUT_LOADED_TILES

        NB_TEST_LOG(PAL_LogBuffer_0, PAL_LogLevelDebug, " * Currently loaded tiles[%d]\r\n", (int)m_debugLoadedTiles.size());

//            // Output all currently loaded tiles
//            for (set<string>::const_iterator iterator = m_debugLoadedTiles.begin(); iterator != m_debugLoadedTiles.end(); ++iterator)
//            {
//                printf(" - %s\r\n", (*iterator).c_str());
//            }
//            printf("\r\n");

#endif
    }

    // We have to record this tick-count in order to record the preceeding tick correctly
    if ((tilesToLoad > 0) || (tilesToUnload > 0))
    {
        m_profilingLoad.RecordTick(2);

        if (tilesToUnload > 0)
        {
            // Check if we need to unload any obsolete tiles
            tilesModified = UnloadTilesIfNecessary() || tilesModified;

            m_profilingLoad.RecordTick(3);
        }
    }

    if (m_profilingLoad.IsEnabled())
    {
        if ((tilesToLoad > 0) || (tilesToUnload > 0))
        {
            static int count = 0;
            printf("\r\n ---------------------- NBGMViewController::LoadAndUnloadTiles[%d] ----------------------\r\n", ++count);
            printf("Lock:           \t");                           m_profilingLoad.Print(1);
            printf("Load %d tiles:   \t", (signed)tilesToLoad);     m_profilingLoad.Print(2);
            printf("Unload %d tiles: \t", (signed)tilesToUnload);   m_profilingLoad.Print(3);
        }
    }

    return tilesModified;
}

/* See header for description */
void
NBGMViewController::UnloadAllTiles()
{
    /*
        This function gets called in the context of the rendering thread!
    */

    if (m_nbgmMapView)
    {
        m_nbgmMapView->UnLoadAllTiles();
    }
}


bool
NBGMViewController::Render(NBGM_FrameListener* listener)
{
    double mx, my, distance;
    float rotationAngle, tiltAngle;

    if(m_gestrueProcessing || m_mapViewCamera->IsAnimating())
    {
        m_nbgmMapView->SetRenderInterval(MIN_RENDER_INTERVAL);
    }
    else
    {
        m_nbgmMapView->SetRenderInterval(MAX_RENDER_INTERVAL);
    }

    m_mapViewCamera->GetRenderCamera(mx, my, distance, rotationAngle, tiltAngle);

    NBGM_MapCamera& camera = m_nbgmMapView->GetMapCamera();
    camera.SetViewCenter(mx, my);
    camera.SetViewPointDistance((float)distance);
    camera.SetRotateAngle(rotationAngle);
    camera.SetTiltAngle(90-tiltAngle);

    double viewDistance = 0;
    double viewDistanceRasterNear = 0;  // Not needed here
    NBUIConfig::getFrustumDistance(m_currentZoomLevel, viewDistance, viewDistanceRasterNear);
    camera.SetHorizonDistance(viewDistance);
    int width, height;
    m_mapViewCamera->GetScreenSize(width, height);
    camera.SetViewSize(0, 0, width, height);
    camera.SetPerspective(m_mapViewCamera->GetFov(), static_cast<float>(width)/height);

    if(m_AvatarUpdate)
    {
        m_nbgmMapView->SetAvatarLocation(m_avatarLocation);
        m_AvatarUpdate = false;
    }
    m_nbgmMapView->RequestRender(listener);


    UpdateLocationInformation();
    UpdateStaticPoiInformation();


    bool animating = m_mapViewCamera->IsAnimating(MapViewAnimation::AT_TILT);
    if(m_mapViewCamera->GetTiltIsBouncingBackFlag() && !animating)
    {
        m_mapViewCamera->SetTiltIsBouncingBackFlag(false);
    }
    animating = m_mapViewCamera->IsAnimating(MapViewAnimation::AT_ROTATE);
    if (m_mapViewCamera->GetRotateIsBouncingBackFlag() && !animating)
    {
        m_mapViewCamera->SetRotateIsBouncingBackFlag(false);
    }
    animating = m_mapViewCamera->IsAnimating(MapViewAnimation::AT_ZOOM);
    if (m_mapViewCamera->GetZoomIsBouncingBackFlag() && !animating)
    {
        m_mapViewCamera->SetZoomIsBouncingBackFlag(false);
    }
    return m_mapViewCamera->IsAnimating();
}


void
NBGMViewController::SetBackground(bool background)
{
    if(m_background!=background)
    {
        m_background = background;
        m_nbgmMapView->SetBackground(m_background);
    }
}


NB_Error
NBGMViewController::UpdateCommonSettings(const vector<TilePtr>& tiles, bool isDay, bool isSatellite)
{
    NB_Error error = NE_INVAL;

    // reorder tiles by denpendency
    std::vector<string> dependency;
    std::vector<string> noDependency;
    list<TilePtr> tilesOrdered;
    for (uint32 i = 0; i < tiles.size(); ++i)
    {
        TilePtr tile = tiles[i];
        if (find(tilesOrdered.begin(), tilesOrdered.end(), tile) != tilesOrdered.end())
        {
            continue;
        }

        if (!tile->GetMaterialBaseType())//PBMAT
        {
            tilesOrdered.push_front(tile);
            if (tile->GetContentID())
            {
                dependency.push_back(*tile->GetContentID());
            }
        }
        else 
        {
            if (tile->GetMaterialBaseType()->compare("base") == 0)//BMAT
            {
                tilesOrdered.push_front(tile);
                if (tile->GetContentID())
                {
                    dependency.push_back(*tile->GetContentID());
                }
            }
            else if (tile->GetMaterialBaseType()->compare("theme") == 0)
            {
                if (isSatellite)
                {
                    if (tile->GetMaterialThemeType() && tile->GetMaterialThemeType()->compare("satellite") == 0)
                    {
                        tilesOrdered.push_back(tile);
                    }
                }
                else
                {
                    if (isDay)
                    {
                        if (tile->GetMaterialThemeType() && tile->GetMaterialThemeType()->compare("day") == 0)
                        {
                            tilesOrdered.push_back(tile);
                        }
                    }
                    else
                    {
                        if (tile->GetMaterialThemeType() && tile->GetMaterialThemeType()->compare("night") == 0)
                        {
                            tilesOrdered.push_back(tile);
                        }
                    }
                }
            }
        }
    }

    PAL_Error err = PAL_Ok;
    for (list<TilePtr>::iterator it = tilesOrdered.begin();
        it != tilesOrdered.end(); ++it)
    {
        TilePtr tile = *it;
        BinaryData data(tile->GetData());
        uint32 bufferSize = data.GetSize();
        uint8* buffer     = data.GetBuffer();
        shared_ptr<string> contentId = tile->GetContentID();
        if (buffer && bufferSize && contentId && !contentId->empty())
        {
            string materialName(*contentId);
            string materialCategory =
                (tile->GetMaterialCategory() && !tile->GetMaterialCategory()->empty())? \
                *tile->GetMaterialCategory() : COMMON_MATERIAL_CATEGORY;
            
            if (materialCategory.compare("PMAT") == 0)
            {
                materialCategory = COMMON_MATERIAL_CATEGORY;
            }
      
            NBGM_MaterialParameters params;
            params.categoryName = materialCategory;
            params.materialName = materialName;
            params.buffer = buffer;
            params.bufferSize = bufferSize;
            if (tile->GetMaterialBaseType() && tile->GetMaterialBaseType()->compare("theme") == 0)
            {
                params.dependsMaterial = dependency;
            }
            PAL_Error palError = m_nbgmMapView->LoadExternalMaterialFromBuffer(params, NBGM_MLF_NONE);
            if(palError == PAL_Ok)
            {
                m_loadedMaterials.insert(MaterialInfo(materialCategory, materialName));
            }
            palError = palError ? palError :
                m_nbgmMapView->SetCurrentExternalMaterial(materialCategory, materialName);
            if (palError != PAL_Ok)
            {
                err = palError;
            }
        }
    }

    if (err == PAL_Ok)
    {
        m_materialsAreLoaded = true;
        m_nbgmMapView->SetSkyDayNight(isDay);
        m_nbgmMapView->SetCompassDayNightMode(isDay);
        m_nbgmMapView->SetSatelliteMode(isSatellite);
        error = NE_OK;
    }

    return error;
}


/* See header for description */
shared_ptr<list<string> >
NBGMViewController::GetInteractedPois(float x, float y)
{
    std::vector<std::string> poiList;
    list<string> *result = NULL;

    uint32 count = m_nbgmMapView->GetInteractedPois(poiList, x, y);
    if (count > 0)
    {
        result = new list<string>();
        for (uint32 i = 0; i < count; i++)
        {
            string item(poiList[i]);
            result->push_back(item);
        }
        poiList.clear();
    }

    return shared_ptr<list<string> >(result);
}

/* See header for description */
bool
NBGMViewController::GetPoiBubblePosition(const string& poiId, float* x, float* y)
{
    NBGM_Point2d pos;
    // get POI bubble position to draw bubbles
    if (m_nbgmMapView->GetPoiBubblePosition(poiId, pos))
    {
        *x = pos.x;
        *y = pos.y;
        return true;
    }
    return false;
}

/* See header for description */
void
NBGMViewController::GetStaticPois(float x, float y, std::vector<std::string>& poiList)
{
    m_nbgmMapView->GetStaticPois(poiList, x, y);
}

/* See header for description */
bool
NBGMViewController::GetStaticPoiInfo(const string& poiId, float* bubbleOffsetX, float* bubbleOffsetY, string& name, double* achorLat, double* anchorLon)
{
    NBGM_StaticPoiInfo info;
    // get POI bubble position to draw bubbles
    if (m_nbgmMapView->GetStaticPoiInfo(poiId, info))
    {
        name = info.name;
        *bubbleOffsetX = info.bubbleOffset.x;
        *bubbleOffsetY = info.bubbleOffset.y;
        mercatorReverse(info.position.x, info.position.y, achorLat, anchorLon);
        return true;
    }
    return false;
}

/* See header for description */
bool
NBGMViewController::GetStaticPoiBubblePosition(const string& poiId, float* x, float* y)
{
    NBGM_StaticPoiInfo info;
    // get POI bubble position to draw bubbles
    if (m_nbgmMapView->GetStaticPoiInfo(poiId, info))
    {
        float screenX = 100;
        float screenY = 100;
        if (m_mapViewCamera->MapPositionToWindow(info.position.x, info.position.y, &screenX, &screenY))
        {
            *x = (screenX + info.bubbleOffset.x);
            *y = (screenY + info.bubbleOffset.y);
            return true;
        }
    }
    return false;
}

/* See header for description */
void
NBGMViewController::SetBubblelistener(BubblelistenerInterface* listener)
{
    m_bubblelistener = listener;
}

/* See header for description */
void
NBGMViewController::SelectPin(shared_ptr<string> pinId, bool selected)
{
    shared_ptr<PinGroup> pinGroup = NULL;
    if(pinId)
    {
        pinGroup = getPinGroupByPinId(*pinId);
    }
    if(!selected)
    {
        if(m_PinPendingFlag && StringUtility::IsStringEqual(m_PendingPinId, pinId))
        {
            // Cancel the selection operation for the pending pin directly
            CancelPendingPin();
        }
        else if(!m_PinPendingFlag && (!pinId || pinId->empty()))
        {
            UnselectPin(pinGroup);
        }
        return;
    }

    if(!pinId || !pinGroup)
    {
        return;
    }

    if(StringUtility::IsStringEqual(pinGroup->m_selectedPinId, pinId))
    {
        CancelPendingPin();
        return;
    }
    UnselectPin(pinGroup);
    // Try to set the selected pin ID and the flag to identify if it is visible on the screen.
    bool existed = m_nbgmMapView->SelectAndTrackPoi(*pinId);
    if(existed)
    {
        pinGroup->m_selectedPinId = pinId;
        // Notify to show the bubble.
        if (m_bubblelistener)
        {
            float positionX = 0.0;
            float positionY = 0.0;
            m_pinVisibleOnScreen = GetPoiBubblePosition(*pinGroup->m_selectedPinId, &positionX, &positionY);

            if (m_pinVisibleOnScreen)
            {
                ShowBubble(pinGroup->m_selectedPinId, true, positionX, positionY);
            }
        }
        CancelPendingPin();
    }
    else
    {
        // We should try it after pin loading finished.
        m_PendingPinId = pinId;
        m_PinPendingFlag = true;
    }

}

/* See header for description */
void
NBGMViewController::UnselectPin(shared_ptr<PinGroup> pinGroup)
{
    if(pinGroup)
    {
        DoUnselectPin(pinGroup);
    }
    else
    {
        map<string, shared_ptr<PinGroup> >::iterator itor;
        for(itor = m_pinGroups.begin(); itor != m_pinGroups.end(); itor++)
        {
            shared_ptr<PinGroup> group = itor->second;
            DoUnselectPin(group);
        }
    }
}

void NBGMViewController::DoUnselectPin(shared_ptr<PinGroup> group)
{
    // Clear the current selected pin ID.
    if (group && group->m_selectedPinId && (!(group->m_selectedPinId->empty())))
    {
        shared_ptr<string> tempPinId = group->m_selectedPinId;
        group->m_selectedPinId.reset();

        m_nbgmMapView->UnselectAndStopTrackingPoi(tempPinId->c_str());
        if (m_pinVisibleOnScreen)
        {
            m_pinVisibleOnScreen = false;
            if (m_bubblelistener)
            {
                ShowBubble(tempPinId, false);
            }
        }
    }
}

/* See header for description */
void
NBGMViewController::RemovePin()
{
    map<string, shared_ptr<PinGroup> >::iterator itor;
    for(itor = m_pinGroups.begin(); itor != m_pinGroups.end(); itor++)
    {
        shared_ptr<PinGroup> pinGroup = itor->second;
        if (pinGroup->m_selectedPinId && (!(pinGroup->m_selectedPinId->empty())))
        {
            m_nbgmMapView->UnselectAndStopTrackingPoi(pinGroup->m_selectedPinId->c_str());
            m_pinVisibleOnScreen = false;
            pinGroup->m_selectedPinId.reset();
            /* This function does not notify to remove the bubble here, because the pin ID is
             useless, use the shared pointer to Pin to remove the bubble in the map view.
             */
        }
    }
}

/* See header for description */
shared_ptr<string>
NBGMViewController::GetSelectedPinId(std::string pinId)
{
    shared_ptr<PinGroup> pinGroup = getPinGroupByPinId(pinId);
    if(pinGroup)
    {
        return pinGroup->m_selectedPinId;
    }
    return NULL;
}

vector<shared_ptr<string> > NBGMViewController::GetSelectedPinIds()
{
    vector<shared_ptr<string> > selectedIds;
    map<string, shared_ptr<PinGroup> >::iterator itor;
    for(itor = m_pinGroups.begin(); itor != m_pinGroups.end(); itor++)
    {
        shared_ptr<PinGroup> pinGroup = itor->second;
        if (pinGroup && pinGroup->m_selectedPinId)
        {
            selectedIds.push_back(pinGroup->m_selectedPinId);
        }
    }
    return selectedIds;
}

/* See header for description */
shared_ptr<string>
NBGMViewController::GetSelectedStaticPoiId()
{
    return m_selectedStaticPoiId;
}

/* See header for description */
void
NBGMViewController::SetSelectedStaticPoiId(shared_ptr<string> id)
{
    if (m_selectedStaticPoiId && (!(m_selectedStaticPoiId->empty())))
    {
        m_nbgmMapView->SelectStaticPoi(*m_selectedStaticPoiId, FALSE);
    }
    m_selectedStaticPoiId = id;
    if (m_selectedStaticPoiId && (!(m_selectedStaticPoiId->empty())))
    {
        m_nbgmMapView->SelectStaticPoi(*m_selectedStaticPoiId, TRUE);
    }
}


/* See header file for description */
void
NBGMViewController::SetAvatarMode(NBGM_AvatarMode avatarMode)
{
    // Check if this avatar mode has already been set.
    if (avatarMode == m_avatarMode)
    {
        return;
    }

    // Set current avatar mode and modified flag for rendering.
    m_avatarMode = avatarMode;
    m_nbgmMapView->SetAvatarMode(m_avatarMode);
}

/* See header file for description */
void
NBGMViewController::SetAvatarState(NBGM_AvatarState avatarState)
{
    // Check if this avatar state has already been set.
    if (avatarState == m_avatarState)
    {
        return;
    }

    // Set current avatar state and modified flag for rendering.
    m_avatarState = avatarState;
    if (m_customAvatarVisibility)
    {
        if (m_customAvatar)
        {
            m_customAvatar->SetState(m_avatarState);
        }
    }
    else
    {
        m_nbgmMapView->SetAvatarState(m_avatarState);
    }
}

/* See header file for description */

void
NBGMViewController::SetAvatarLocation(const NBGM_Location64& avatarLocation)
{
    // I think there is no need to check if two NBGM locations are equal.
    m_avatarLocation = avatarLocation;
    if (m_customAvatarVisibility)
    {
        if (m_customAvatar)
        {
            m_customAvatar->SetLocation(m_avatarLocation);
        }
    }
    else
    {
        m_AvatarUpdate = true;
    }
}

/* See header file for description */
void
NBGMViewController::SetCustomAvatar(shared_ptr<nbcommon::DataStream> directionalAvatar,
                                    shared_ptr<nbcommon::DataStream> directionlessAvatar,
                                    int8 directionalCalloutOffsetX, int8 directionalCalloutOffsetY,
                                    int8 directionlessCalloutOffsetX, int8 directionlessCalloutOffsetY)
{
    if (directionalAvatar && directionlessAvatar && CreateCustomAvatar())
    {
        NBGM_BinaryBuffer followMeTextureBuffer;
        followMeTextureBuffer.size = directionalAvatar->GetDataSize();
        followMeTextureBuffer.addr = new uint8[followMeTextureBuffer.size];
        nsl_memset(followMeTextureBuffer.addr, 0, followMeTextureBuffer.size);
        directionalAvatar->GetData(followMeTextureBuffer.addr, 0, followMeTextureBuffer.size);

        NBGM_BinaryBuffer standByTextureBuffer;
        standByTextureBuffer.size = directionlessAvatar->GetDataSize();
        standByTextureBuffer.addr = new uint8[standByTextureBuffer.size];
        nsl_memset(standByTextureBuffer.addr, 0, standByTextureBuffer.size);
        directionlessAvatar->GetData(standByTextureBuffer.addr, 0, standByTextureBuffer.size);

        NBGM_Point2d directionalOffset;
        directionalOffset.x = (float)(directionalCalloutOffsetX / 100.0);
        directionalOffset.y = (float)(directionalCalloutOffsetY / 100.0);
        NBGM_Point2d directionlessOffset;
        directionlessOffset.x = (float)(directionlessCalloutOffsetX / 100.0);
        directionlessOffset.y = (float)(directionlessCalloutOffsetY / 100.0);

        m_customAvatar->SetDirectionalTexture(followMeTextureBuffer, directionalOffset);
        m_customAvatar->SetDirectionlessTexture(standByTextureBuffer, directionlessOffset);
        m_customAvatar->SetState(m_avatarState);
        m_nbgmMapView->SetAvatarState(NBGM_AS_DISABLED);
        delete [] followMeTextureBuffer.addr;
        delete [] standByTextureBuffer.addr;
        m_customAvatarVisibility = true;
    }
    else
    {
        ResetDefaultAvatar();
    }
}

/* See header file for description */
void
NBGMViewController::SetCustomAvatar(shared_ptr<std::string> directionalAvatarPath,
                                    shared_ptr<std::string> directionlessAvatarPath,
                                    int8 directionalCalloutOffsetX, int8 directionalCalloutOffsetY,
                                    int8 directionlessCalloutOffsetX, int8 directionlessCalloutOffsetY)
{
    if (directionalAvatarPath && directionlessAvatarPath &&
        !(*directionalAvatarPath).empty() && !(*directionlessAvatarPath).empty() &&
        CreateCustomAvatar())
    {
        NBGM_Point2d directionalOffset;
        directionalOffset.x = (float)(directionalCalloutOffsetX / 100.0);
        directionalOffset.y = (float)(directionalCalloutOffsetY / 100.0);
        NBGM_Point2d directionlessOffset;
        directionlessOffset.x = (float)(directionlessCalloutOffsetX / 100.0);
        directionlessOffset.y = (float)(directionlessCalloutOffsetY / 100.0);

        m_customAvatar->SetDirectionalTexture(*directionalAvatarPath, directionalOffset);
        m_customAvatar->SetDirectionlessTexture(*directionlessAvatarPath, directionlessOffset);
        m_customAvatar->SetState(m_avatarState);
        m_nbgmMapView->SetAvatarState(NBGM_AS_DISABLED);
        m_customAvatarVisibility = true;
    }
    else
    {
        ResetDefaultAvatar();
    }
}

/* See header file for description */
void
NBGMViewController::SetCustomAvatarHaloStyle(uint32 edgeColor, uint32 fillColor, uint8 edgeSize, NBGM_HaloEdgeStyle edgeStyle)
{
    if (m_customAvatarVisibility && m_customAvatar)
    {
        NBGM_Color nbgmEdgeColor;
        NBGM_Color nbgmFillColor;
        nbgmEdgeColor.red = ((edgeColor >> 24) & 0xFF) / 255.0f;
        nbgmEdgeColor.green = ((edgeColor >> 16) & 0xFF) / 255.0f;
        nbgmEdgeColor.blue = ((edgeColor >> 8) & 0xFF) / 255.0f;
        nbgmEdgeColor.alpha = (edgeColor & 0xFF) / 255.0f;

        nbgmFillColor.red = ((fillColor >> 24) & 0xFF) / 255.0f;
        nbgmFillColor.green = ((fillColor >> 16) & 0xFF) / 255.0f;
        nbgmFillColor.blue = ((fillColor >> 8) & 0xFF) / 255.0f;
        nbgmFillColor.alpha = (fillColor & 0xFF) / 255.0f;

        m_customAvatar->SetHaloStyle(nbgmEdgeColor, nbgmFillColor, edgeSize, edgeStyle);
    }
}

/* See header file for description */
bool NBGMViewController::IsAvatarHit(float screenX, float screenY)
{
    if (m_customAvatarVisibility)
    {
        return m_customAvatar ? m_customAvatar->SelectAndTrack(screenX, screenY) : false;
    }
    return m_nbgmMapView ? m_nbgmMapView->SelectAndTrackAvatar(screenX, screenY) : false;
}

void NBGMViewController::EnableLocationBubble(bool enabled)
{
    if (m_locationBubbleInfo.m_enabled == enabled || !m_bubblelistener)
    {
        return;
    }

    m_locationBubbleInfo.m_enabled = enabled;
    (void)UpdateAvatarLocationInternally();

    if (enabled)
    {
        m_bubblelistener->NotifyShowLocationBubble(m_locationBubbleInfo.m_x,
                                                   m_locationBubbleInfo.m_y, true,
                                                   m_locationBubbleInfo.m_latitude,
                                                   m_locationBubbleInfo.m_longitude);
    }
    else
    {
        m_bubblelistener->NotifyHideLocationBubble();
    }
}

/* See header file for description */
void
NBGMViewController::GetDebugInfo(DebugInfo& info)
{
    double mx, my, distance;
    float rotationAngle, tiltAngle;

    m_mapViewCamera->GetRenderCamera(mx, my, distance, rotationAngle, tiltAngle);

    NBGM_MapViewProfile profile = {0};
    m_nbgmMapView->DumpProfile(profile);
    double mercatorX = mx;
    double mercatorY = my;
    mercatorReverse(mercatorX, mercatorY, &info.latitude, &info.longitude);

    info.actualZoomLevel        = m_mapViewCamera->GetZoomLevel();
    info.rotationAngle          = rotationAngle;
    info.tiltAngle              = tiltAngle;
    info.cameraHeight           =  (float)(distance * nsl_sin(TO_RAD(tiltAngle)));
    info.renderTime             = profile.mTotalRenderTime;
    info.totalNumOfTriangles    = profile.mTotalTrianglesNum;
    info.numberOfLoadedTiles    = profile.mLoadedTileNum;
}

/* See header file for description */
void
NBGMViewController::EnableFarNearVisibility(bool flag)
{
    if (m_layerVisibility == flag)
    {
        return;
    }
    m_layerVisibility = flag;
    NBGM_EnableLayerFarNearVisibility(m_layerVisibility);
}


/* See header file for description */
uint32
NBGMViewController::GetSupportedNBMFileVersion()
{
    return NBGM_GetSupportedNBMFileVersion();
}


/* See header file for description */
void
NBGMViewController::AddAnimationLayer(uint32 layerID)
{
    /*
        This has to be called from the render thread.
        No lock necessary
     */

    NBGM_TileAnimationLayer* pLayer = m_nbgmMapView->CreateTileAnimationLayer();
    if (pLayer)
    {
        // Set the default opacity to the last set value
        pLayer->SetOpacity((uint8)m_animationLayerOpacity);

        m_animationLayers.insert(make_pair(layerID, pLayer));
    }
}

/* See header file for description */
void
NBGMViewController::RemoveAnimationLayer(uint32 layerID)
{
    /*
        This has to be called from the render thread.
        No lock necessary
     */

    NBGMAnimationLayerMap::iterator pLayer = m_animationLayers.find(layerID);
    if (pLayer != m_animationLayers.end())
    {
        // Remove layer, this will unload any still loaded tiles for this layer.
        m_nbgmMapView->RemoveTileAnimationLayer(pLayer->second);
        m_animationLayers.erase(layerID);
    }
}

/* See header file for description */
void
NBGMViewController::SetAnimationFrameData(const map<uint32, ANIMATION_DATA>& data)
{
    /*
        This has to be called from the render thread.
        No lock necessary
     */

    // For all animation layers, most likely this is only one
    map<uint32, ANIMATION_DATA>::const_iterator end = data.end();
    for (map<uint32, ANIMATION_DATA>::const_iterator layer = data.begin(); layer != end; ++layer)
    {
        // Find the NBGM animation layer
        NBGMAnimationLayerMap::iterator nbgmLayer = m_animationLayers.find(layer->first);
        if (nbgmLayer != m_animationLayers.end())
        {
            const ANIMATION_DATA& layerData = layer->second;

            /*
                For now we always update both the frame data and the frame index at the same time.
             */
            // Set the content-IDs for the currently animated frame. The tiles should be already loaded.
            nbgmLayer->second->UpdateFrame(layerData.first, layerData.second);
            nbgmLayer->second->SetCurrentFrame(layerData.first);
        }
    }
}

/* See header file for description */
void
NBGMViewController::UnloadAnimationTiles(uint32 layerID, const vector<shared_ptr<string> >& tiles)
{
    /*
        This function gets called in the context of the rendering thread!
        No lock necessary
     */
    NBGMAnimationLayerMap::iterator layer = m_animationLayers.find(layerID);
    if (layer != m_animationLayers.end())
    {
        layer->second->UnloadTiles(tiles);
    }
}

void
NBGMViewController::SetAnimationLayerOpacity(uint32 opacity)
{
    // Save the value in our member variable. We will use this value the next time an animation layer gets added
    m_animationLayerOpacity = opacity;

    // If we have any animation layer(s), then we'll update them

    for (NBGMAnimationLayerMap::iterator nbgmLayer = m_animationLayers.begin(); nbgmLayer != m_animationLayers.end(); ++nbgmLayer)
    {
        nbgmLayer->second->SetOpacity((uint8)opacity);
    }
}


// Private Functions ......................................................................................................



void NBGMViewController::SelectCurrentPin()
{
    if(m_PendingPinId)
    {
        shared_ptr<PinGroup> pinGroup = getPinGroupByPinId(*m_PendingPinId);
        if(pinGroup && !pinGroup->m_selectedPinId->empty())
        {
            m_nbgmMapView->SelectAndTrackPoi(*pinGroup->m_selectedPinId);
            float positionX = 0.f;
            float positionY = 0.f;
            bool visible = GetPoiBubblePosition(*pinGroup->m_selectedPinId, &positionX, &positionY);
            if(visible && !m_showBubble)
            {
                ShowBubble(pinGroup->m_selectedPinId, true, positionX, positionY);
            }
        }
    }
}

void
NBGMViewController::CancelPendingPin()
{
    m_PinPendingFlag = false;
    m_PendingPinId.reset();
}

void
NBGMViewController::SelectPendingPin()
{
    if(!m_PinPendingFlag)
    {
        return;
    }

    if(m_PendingPinId)
    {
        shared_ptr<PinGroup> pinGroup = getPinGroupByPinId(*m_PendingPinId);
        if(pinGroup && !StringUtility::IsStringEqual(m_PendingPinId, pinGroup->m_selectedPinId))
        {
            UnselectPin(pinGroup);
            bool pin = m_nbgmMapView->SelectAndTrackPoi(*m_PendingPinId);
            if(pin)
            {
                pinGroup->m_selectedPinId = m_PendingPinId;
                float positionX = 0.0;
                float positionY = 0.0;
                m_pinVisibleOnScreen = GetPoiBubblePosition(*pinGroup->m_selectedPinId, &positionX, &positionY);
                if (m_pinVisibleOnScreen && m_bubblelistener)
                {
                    // Show the bubble.
                    ShowBubble(pinGroup->m_selectedPinId, true, positionX, positionY);
                }
                CancelPendingPin();
            }
        }
    }

    m_nbgmMapView->Invalidate();
}

void
NBGMViewController::UpdatePinInformation()
{
    map<string, shared_ptr<PinGroup> >::iterator itor;
    for(itor = m_pinGroups.begin(); itor != m_pinGroups.end(); itor++)
    {
        shared_ptr<PinGroup> pinGroup = itor->second;
        if (pinGroup && pinGroup->m_selectedPinId && (!(pinGroup->m_selectedPinId->empty())))
        {
            // Save the previous flag to identify if the pin is visible on the screen before.
            bool previousVisibleFlag = m_pinVisibleOnScreen;

            // Get the current position of the pin.
            NBGM_Point2d pos;
            m_pinVisibleOnScreen = m_nbgmMapView->GetPoiBubblePosition(*pinGroup->m_selectedPinId, pos);
            // The pin is always visible.
            if (previousVisibleFlag && m_pinVisibleOnScreen)
            {
                if (m_bubblelistener)
                {
                    if(fabs(m_preBubblePositionX -pos.x) >= BUBBLE_POSITION_EPSILON ||
                       fabs(m_preBubblePositionY -pos.y) >= BUBBLE_POSITION_EPSILON ||
                       m_preSelectedPinId.compare(*pinGroup->m_selectedPinId) != 0)
                    {
                        // Update the position.
                        m_bubblelistener->NotifyUpdateBubble(pinGroup->m_selectedPinId, pos.x, pos.y, true);
                        m_preBubblePositionX = pos.x;
                        m_preBubblePositionY = pos.y;
                        m_preSelectedPinId = *pinGroup->m_selectedPinId;
                    }
                }
            }

            // The pin is moved outside the screen.
            else if (previousVisibleFlag && (!m_pinVisibleOnScreen))
            {
                if (m_bubblelistener)
                {
                    // Hide the bubble.
                    ShowBubble(pinGroup->m_selectedPinId, false);
                }
            }

            // The pin is moved from the outside to the screen.
            else if ((!previousVisibleFlag) && m_pinVisibleOnScreen)
            {
                // The pin tile may be unloaded, select the pin again.
                m_pinVisibleOnScreen = m_nbgmMapView->SelectAndTrackPoi(pinGroup->m_selectedPinId->c_str());

                if (m_pinVisibleOnScreen && m_bubblelistener)
                {
                    // Show the bubble.
                    ShowBubble(pinGroup->m_selectedPinId, true, pos.x, pos.y);
                }
            }
        }
    }
}

void
NBGMViewController::ResetPoiBubbleState()
{
    if (m_selectedStaticPoiId && (!(m_selectedStaticPoiId->empty())))
    {
        m_nbgmMapView->SelectStaticPoi(*m_selectedStaticPoiId, FALSE);
    }
    m_selectedStaticPoiId.reset();
    m_staticPoiVisibleOnScreen = false;
}

void
NBGMViewController::UpdateStaticPoiInformation()
{
    if (m_selectedStaticPoiId && (!(m_selectedStaticPoiId->empty())))
    {
        //need to set static poi selected status to true, because it maybe referenced by load and unload logic.
        m_nbgmMapView->SelectStaticPoi(*m_selectedStaticPoiId, TRUE);
        // Get the current position of the pin.
        float positionX = 0.0;
        float positionY = 0.0;
        m_staticPoiVisibleOnScreen = GetStaticPoiBubblePosition(*m_selectedStaticPoiId, &positionX, &positionY);

        if (m_staticPoiVisibleOnScreen)
        {
            if (m_bubblelistener)
            {
                m_bubblelistener->NotifyShowStaticPoiBubble(positionX, positionY, true);
            }
        }
        // The pin is moved outside the screen.
        else
        {
            if (m_bubblelistener)
            {
                m_bubblelistener->NotifyHideStaticPoiBubble();
            }
        }
    }
}

/* See header for description */
void NBGMViewController::UpdateLocationInformation()
{
    if (!m_locationBubbleInfo.m_enabled || !m_nbgmMapView)
    {
        return ;
    }

    if (UpdateAvatarLocationInternally())
    {
        m_bubblelistener->NotifyUpdateLocationBubble(m_locationBubbleInfo.m_x,
                                                     m_locationBubbleInfo.m_y, true,
                                                     m_locationBubbleInfo.m_latitude,
                                                     m_locationBubbleInfo.m_longitude);

    }
}

/* See header for description */
bool NBGMViewController::UpdateAvatarLocationInternally()
{
    bool needUpdate = false;

    if (m_locationBubbleInfo.m_enabled)
    {
        float positionX = 0.0;
        float positionY = 0.0;
        if (m_customAvatarVisibility ? m_customAvatar->GetScreenPosition(positionX, positionY) : m_nbgmMapView->GetAvatarScreenPosition(positionX, positionY))
        {
            double mercatorX;
            double mercatorY;
            m_mapViewCamera->ScreenToMapPosition(positionX, positionY, mercatorX, mercatorY);

            double latitude;
            double longitude;
            NB_SpatialConvertMercatorToLatLong(mercatorX,mercatorY, &latitude, &longitude);

            float x = positionX;
            float y = positionY;
            if (x != m_locationBubbleInfo.m_x || y != m_locationBubbleInfo.m_y ||
                (float)latitude != m_locationBubbleInfo.m_latitude ||
                (float)longitude != m_locationBubbleInfo.m_longitude)
            {
                needUpdate = true;
                m_locationBubbleInfo.m_x = x;
                m_locationBubbleInfo.m_y = y;
                m_locationBubbleInfo.m_latitude  = (float)latitude;
                m_locationBubbleInfo.m_longitude = (float)longitude;
            }
        }
        else
        {
            m_locationBubbleInfo.m_enabled = false;
            m_bubblelistener->NotifyHideLocationBubble();
        }
    }

    return needUpdate;
}





void
NBGMViewController::SetAvatar(const NBGM_Location64& location)
{
    SetAvatarLocation(location);
}

/* See header for description */
void
NBGMViewController::LoadTile(TileWrapperPtr tileWrapper)
{
    if (!tileWrapper || !tileWrapper->m_tile)
    {
        return;
    }

    TilePtr& tile = tileWrapper->m_tile;
    shared_ptr<string>& previousTileId = tileWrapper->m_previousID;

    // Copy data from datastream

    // @todo: NBGM should read the data directly from the stream
    BinaryData data(tile->GetData());
    uint32 bufferSize = data.GetSize();
    uint8* buffer     = data.GetBuffer();
    if (buffer && bufferSize)
    {
        //@todo: it's temp solution to avoid enable picking up function in most case.
        NBGM_NBMLoadFlags loadFlag     = (tile->GetContentID()->find("GEOGRAPHIC") == string::npos) ? \
                                            NBGM_NLF_NONE : NBGM_NLF_ENABLE_PICKING;

        string materialCategory = tile->GetMaterialCategory() ? \
                                  *tile->GetMaterialCategory() : COMMON_MATERIAL_CATEGORY;
        if (materialCategory.compare("PMAT") == 0)
        {
            materialCategory = "MAT";
        }
        
        if (tile->GetTimeStamp() > 0)
        {
            // Find animation layer, we should always find it.
            NBGMAnimationLayerMap::iterator pLayer = m_animationLayers.find(tile->GetLayerID());
            if (pLayer != m_animationLayers.end())
            {
                NBGM_TileAnimationData tileData =
                        {
                            tile->GetContentID(),
                            shared_ptr<string> (new string(materialCategory)),
                            { buffer, bufferSize }
                        };

                if (previousTileId)
                {
                    // Trying to refresh animation layer, but there is no interface to do
                    // it, so unload old one first, and then load it again.
                    pLayer->second->UnloadTiles(vector<shared_ptr<string> > (1, previousTileId));
                }

                /*
                  @todo: We should improve the tile handling to allow bulk loads of tiles,
                  instead of loading one tile at a time. We have to switch to the new
                  LayerManager::GetTiles() interface and update MapView and
                  NBGMViewControler to handle bulk tile loads.
                */
                vector<NBGM_TileAnimationData> tiles;
                tiles.push_back(tileData);
                pLayer->second->LoadTiles(tiles, (uint8)tile->GetDrawOrder(), (uint8)tile->GetLabelDrawOrder());
            }
            else
            {
                cout << "Error: Can't find animation layer!" << endl;
            }
        }
        // Load or reload non-animation tile.
        else
        {
            DrawOrder orders = tile->GetDrawOrders();
            //NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelWarning,
            //            "Loading tile: %s with orders: %d - %d -:%d\n",
            //            tile->GetContentID()->c_str(),
            //            orders.baseDrawOrder,
            //            orders.subDrawOrder,
            //            orders.labelDrawOrder);
            if(tile->GetContentID()->find("GEOGRAPHIC_GeographicPolylineLayer") == string::npos)
            {
                NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelWarning,
                            "Updating tile: %s with orders: %d - %d -:%d\n",
                            tile->GetContentID()->c_str(),
                            orders.baseDrawOrder,
                            orders.subDrawOrder,
                            orders.labelDrawOrder);
            }

            string layerIdStr = nbcommon::StringUtility::NumberToString(tile->GetLayerID());
            if (previousTileId)
            {
                m_TileLoadingListener->SetReloadFlag(true);
                if(*previousTileId == *(tile->GetContentID()))
                {
                    m_nbgmMapView->UpdateNBMTileFromBuffer(*(tile->GetContentID()),
                                        materialCategory,
                                        layerIdStr,
                                        orders.baseDrawOrder,
                                        orders.subDrawOrder,
                                        orders.labelDrawOrder,
                                        buffer, bufferSize,
                                        loadFlag);
                }
                else
                {
                    m_nbgmMapView->ReloadNBMTileFromBuffer(*previousTileId,
                                        *(tile->GetContentID()),
                                        materialCategory,
                                        layerIdStr,
                                        orders.baseDrawOrder,
                                        orders.subDrawOrder,
                                        orders.labelDrawOrder,
                                        buffer, bufferSize,
                                        loadFlag, m_TileLoadingListener.get());
                }
            }
            else
            {
                m_TileLoadingListener->SetReloadFlag(false);
                m_nbgmMapView->LoadNBMTileFromBuffer(*(tile->GetContentID()),
                                                     materialCategory,
                                                     layerIdStr,
                                                     orders.baseDrawOrder,
                                                     orders.subDrawOrder,
                                                     orders.labelDrawOrder,
                                                     buffer, bufferSize,
                                                     loadFlag, m_TileLoadingListener.get());
            }

            if (loadFlag != NBGM_NLF_NONE)
            {
                NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelWarning,
                            "loading tile: %s with orders: %d - %d -:%d\n",
                            tile->GetContentID()->c_str(),
                            orders.baseDrawOrder,
                            orders.subDrawOrder,
                            orders.labelDrawOrder);
            }
        }
    }
}

/* See header for description */
bool
NBGMViewController::UnloadTilesIfNecessary()
{
    /*
      This function gets called in the context of the rendering thread!
     */

    if (m_tilesToUnload.empty())
    {
        return false;
    }

    /*
        Map of all animation layers with m_tilesToUnloadtiles to unload. The key is the layer ID. The vector contains content-IDs to unload.
     */
    typedef map<uint32, vector<shared_ptr<string> > > ANIMATION_MAP;
    ANIMATION_MAP animationTilesToUnload;

    // For all m_tilesToUnloadtiles to unload
    vector<TilePtr>::const_iterator iter = m_tilesToUnload.begin();
    vector<TilePtr>::const_iterator end  = m_tilesToUnload.end();
    vector<string> tileIds;
    for (; iter != end; ++iter)
    {
        Tile* tile = iter->get();
        // TilePtr tile = *iter;
        if (!tile || nbcommon::StringUtility::IsEmpty(tile->GetContentID())) //@todo: is it OK to ignore this tile?
        {
            continue;
        }

        // Check if the tile to unload is a tile from an animation layer
        if (tile->GetTimeStamp() > 0)
        {
            // Insert an empty vector into the map. If an entry with that layerID already exists, then the function call returns an iterator
            // to the existing entry. If not it creates a new entry.
            pair<ANIMATION_MAP::iterator, bool> insertResult = animationTilesToUnload.insert(make_pair(tile->GetLayerID(), vector<shared_ptr<string> >()));

            // Insert the new content-ID
            insertResult.first->second.push_back(tile->GetContentID());
        }
        // Non-animation layer
        else
        {
            tileIds.push_back(*tile->GetContentID());
        }

#ifdef DEBUG_OUTPUT_LOADED_TILES

        cout << " -> Unload tile: " << tile->GetContentID()->c_str() << ", Timestamp: " << tile->GetTimeStamp() << endl;

        // Find the tile to unload
        string contentID = tile->GetContentID()->c_str();
        set<string>::iterator iterator = m_debugLoadedTiles.find(contentID);
        if (iterator == m_debugLoadedTiles.end())
        {
            printf("ERROR: Try to remove tile which is not loaded: %s !!!!\r\n", contentID.c_str());
        }
        else
        {
            // remove tile
            m_debugLoadedTiles.erase(iterator);
        }
        fflush(stdout);

#endif

    }

    m_nbgmMapView->UnLoadTile(tileIds);

    // Go through all the animation layers where we have m_tilesToUnloadtiles to unload
    ANIMATION_MAP::iterator layerEnd = animationTilesToUnload.end();
    for (ANIMATION_MAP::iterator layerIterator = animationTilesToUnload.begin(); layerIterator != layerEnd; ++layerIterator)
    {
        // Find the NBGM layer pointer in our layer map.
        NBGMAnimationLayerMap::iterator pNBGMLayer = m_animationLayers.find(layerIterator->first);
        if (pNBGMLayer != m_animationLayers.end())
        {
            if (pNBGMLayer->second)
            {
                // Unload the vector of m_tilesToUnloadtiles
                pNBGMLayer->second->UnloadTiles(layerIterator->second);
            }
        }
        else
        {
            cout << "ERROR: Can't find animation layer to unload. Could be because layer was removed before we got a chance to unload all the m_tilesToUnloadtiles." << endl;
        }
    }

    m_tilesToUnload.clear();
    return true;
}



void
NBGMViewController::GenerateMapImage(uint8* buffer, uint32 bufferSize, int32 x, int32 y, uint32 width, uint32 height)
{
    if(!buffer || bufferSize == 0)
    {
        return;
    }

    m_nbgmMapView->SnapScreenshot(buffer, bufferSize, x, y, width, height);
}

bool
NBGMViewController::IsAvatarInScreen(float mercatorX, float mercatorY)
{
    return m_nbgmMapView->IsAvatarInScreen(mercatorX, mercatorY);
}

/* See header file for description */
NBGM_DeviceLocation*
NBGMViewController::CreateCustomAvatar()
{
    if (!m_customAvatar)
    {
        m_customAvatar = m_nbgmMapView->CreateDeviceLocation(CUSTOM_AVATAR_ID);
    }
    return m_customAvatar;
}

/* See header file for description */
void
NBGMViewController::ResetDefaultAvatar()
{
    if (m_customAvatar)
    {
        m_customAvatar->SetState(NBGM_AS_DISABLED);
    }
    m_nbgmMapView->SetAvatarState(m_avatarState);
    m_customAvatarVisibility = false;
}

/* See header file for description */
PAL_Error NBGMViewController::AsyncGenerateMapImage(const shared_ptr<NBGM_AsyncSnapScreenshotCallback>& snapScreenshot)
{
    return m_nbgmMapView->AsyncSnapScreenshot(snapScreenshot);
}




nb_boolean
NBGMViewController::IsCompassTapped(float screenX, float screenY)
{
    if(m_nbgmMapView)
    {
        return m_nbgmMapView->TapCompass(screenX, screenY);
    }
    return FALSE;
}

void
NBGMViewController::ShowCompass(bool enabled)
{
    if(m_nbgmMapView)
    {
        m_nbgmMapView->EnableCompass(enabled);
    }
}

void
NBGMViewController::SetCompassPosition(float screenX, float screenY)
{
    if(m_nbgmMapView)
    {
        m_nbgmMapView->SetCompassPosition(screenX, screenY);
    }
}

void
NBGMViewController::SetCompassIcons(const std::string& dayModeIconPath, const std::string& nightModeIconPath)
{
    if(m_nbgmMapView)
    {
        m_nbgmMapView->SetCompassIcons(dayModeIconPath, nightModeIconPath);
    }
}

void
NBGMViewController::OnRenderFrameEnd()
{
    if(!m_nbgmMapView)
    {
        return;
    }
    UpdatePinInformation();
}

void NBGMViewController::GetElementsAt(float screenX, float screenY, set<std::string>& elements)
{
    m_nbgmMapView->GetElementsAt(screenX, screenY, elements);
}

void NBGMViewController::EnableCustomLayerCollisionDetection(const std::string& layerId, bool enable)
{
    m_nbgmMapView->EnableCollisionDetection(layerId, enable);
}

// Helper function to check if ExternalMaterial is valid or not.
static inline bool IsExternalMaterialValid(const ExternalMaterialPtr&material,
                                           bool checkData=false)
{
    bool valid = material &&
                 material->m_categoryName && !material->m_categoryName->empty() &&
                 material->m_materialName && !material->m_materialName->empty();
    if (checkData)
    {
        valid &= (material->m_materialData && material->m_materialData->GetDataSize());
    }

    return valid;
}
void NBGMViewController::LoadExternalMaterial(const ExternalMaterialPtr& material)
{
    if (IsExternalMaterialValid(material, true))
    {
        BinaryData data(material->m_materialData);
        uint32 bufferSize = data.GetSize();
        uint8* buffer     = data.GetBuffer();
        if (buffer && bufferSize)
        {
            NBGM_MaterialParameters params;
            params.categoryName = *material->m_categoryName;
            params.materialName = *material->m_materialName;
            params.buffer = buffer;
            params.bufferSize = bufferSize;
            m_nbgmMapView->LoadExternalMaterialFromBuffer(params, NBGM_MLF_NONE);
            m_loadedMaterials.insert(MaterialInfo(params.categoryName, params.materialName));
        }
    }
}

void NBGMViewController::UnloadExternalMaterial(const ExternalMaterialPtr& material)
{
    if (IsExternalMaterialValid(material))
    {
        m_nbgmMapView->UnloadExternalMaterial(*material->m_categoryName,
                                              *material->m_materialName);
        m_loadedMaterials.erase(MaterialInfo(*material->m_categoryName, *material->m_materialName));
    }
}

void NBGMViewController::ActivateExternalMaterial(const ExternalMaterialPtr& material)
{
    if (IsExternalMaterialValid(material))
    {
        m_nbgmMapView->SetCurrentExternalMaterial(*material->m_categoryName,
                                                  *material->m_materialName);
    }
}


void NBGMViewController::ReloadTiles(const vector<TilePtr>& tiles)
{
    vector<TilePtr>::const_iterator iter = tiles.begin();
    vector<TilePtr>::const_iterator end  = tiles.end();
    for (; iter != end; ++iter)
    {
        const TilePtr& tile = *iter;
        // reload should not happen for animation tiles.
        if (tile && !tile->GetTimeStamp() && tile->GetContentID())
        {
            shared_ptr<string> contentId = tile->GetContentID();
            DrawOrder orders = tile->GetDrawOrders();

            //@todo: Add function to pass following info to NBGM:
            //   1. ContendID,
            //   2. baseDrawOrder,
            //   3. subDrawOrder,
            //   4. labelDrawOrder
            if(contentId->find("GEOGRAPHIC_GeographicPolylineLayer") == string::npos)
            {
                NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelWarning,
                            "Updating tile: %s with orders: %d - %d -:%d\n",
                            contentId->c_str(),
                            orders.baseDrawOrder,
                            orders.subDrawOrder,
                            orders.labelDrawOrder);
            }

            //@todo: it's temp solution to avoid enable picking up function in most case.
            NBGM_NBMLoadFlags loadFlag     = (tile->GetContentID()->find("GEOGRAPHIC") == string::npos) ? \
                                             NBGM_NLF_NONE : NBGM_NLF_ENABLE_PICKING;
            const string& materialCategory = tile->GetMaterialCategory() ? \
                                             *tile->GetMaterialCategory() : COMMON_MATERIAL_CATEGORY;
            char layerIdStr[30];
            sprintf(layerIdStr, "%d", tile->GetLayerID());
            m_nbgmMapView->UpdateNBMTileFromBuffer(*(tile->GetContentID()),
                                materialCategory,
                                layerIdStr,
                                orders.baseDrawOrder,
                                orders.subDrawOrder,
                                orders.labelDrawOrder,
                                NULL, 0,
                                loadFlag);

        }
    }
}

void NBGMViewController::GetCompassBoundingBox(float& leftBottomX, float& leftBottomY, float& rightTopX, float& rightTopY)const
{
    m_nbgmMapView->GetCompassBoundingBox(leftBottomX, leftBottomY, rightTopX, rightTopY);
}


void NBGMViewController::AddCircle(int circleId, const NBGM_CircleParameters &circlePara)
{
    if(m_nbgmMapView)
    {
        NBGMCircleMap::iterator it = m_Circles.find(circleId);
        if(it == m_Circles.end())
        {
            NBGM_Circle *circle = m_nbgmMapView->AddCircle(circleId, circlePara);
            if(circle != NULL)
            {
                m_Circles.insert(NBGMCircleMap::value_type(circleId, circle));
            }
        }
    }
}

void NBGMViewController::RemoveCircle(int circleId)
{
    if(m_nbgmMapView)
    {
        NBGMCircleMap::iterator it = m_Circles.find(circleId);
        if(it != m_Circles.end())
        {
            m_nbgmMapView->RemoveCircle(it->second);
            m_Circles.erase(it);
        }
    }
}

void NBGMViewController::RemoveAllCircles()
{
    m_Circles.clear();
    if(m_nbgmMapView)
    {
        m_nbgmMapView->RemoveAllCircles();
    }
}

void NBGMViewController::SetCircleCenter(int circleId, double x, double y)
{
    if(m_nbgmMapView)
    {
        NBGMCircleMap::iterator it = m_Circles.find(circleId);
        if(it != m_Circles.end())
        {
            NBGM_Circle *circle = it->second;
            if(circle != NULL)
            {
                circle->SetCenter(x, y);
            }
        }
    }
}

void NBGMViewController::SetCircleVisible(int circleId, bool visible)
{
    if(m_nbgmMapView)
    {
        NBGMCircleMap::iterator it = m_Circles.find(circleId);
        if(it != m_Circles.end())
        {
            NBGM_Circle *circle = it->second;
            if(circle != NULL)
            {
                circle->SetVisible(visible);
            }
        }
    }
}

void NBGMViewController::SetCircleStyle(int circleId, float radius, const NBGM_Color& fillColor, const NBGM_Color& outlineColor)
{
    if(m_nbgmMapView)
    {
        NBGMCircleMap::iterator it = m_Circles.find(circleId);
        if(it != m_Circles.end())
        {
            NBGM_Circle *circle = it->second;
            if(circle != NULL)
            {
                circle->SetStyle(radius, fillColor, outlineColor);
            }
        }
    }
}

void NBGMViewController::AddRect2d(NBGM_Rect2dId rectId, const NBGM_Rect2dParameters &para)
{
    if(m_nbgmMapView)
    {
        NBGMRect2dMap::iterator it = m_Rect2ds.find(rectId);
        if(it == m_Rect2ds.end())
        {
            NBGM_CustomRect2d *rect = m_nbgmMapView->AddRect2d(rectId, para);
            if(rect != NULL)
            {
                m_Rect2ds.insert(NBGMRect2dMap::value_type(rectId, rect));
            }
        }
    }
}

void NBGMViewController::RemoveRect2d(NBGM_Rect2dId rectId)
{
    if(m_nbgmMapView)
    {
        NBGMRect2dMap::iterator it = m_Rect2ds.find(rectId);
        if(it != m_Rect2ds.end())
        {
            m_nbgmMapView->RemoveRect2d(it->second);
            m_Rect2ds.erase(it);
        }
    }
}

void NBGMViewController::UpdateRect2d(NBGM_Rect2dId rectId, double mx, double my, float heading)
{
    if(m_nbgmMapView)
    {
        NBGMRect2dMap::iterator it = m_Rect2ds.find(rectId);
        if(it != m_Rect2ds.end())
        {
            NBGM_CustomRect2d *rect = it->second;
            if(rect != NULL)
            {
                rect->Update(mx, my, heading);
            }
        }
    }
}

void NBGMViewController::SetRect2dVisible(NBGM_Rect2dId rectId, bool visible)
{
    if(m_nbgmMapView)
    {
        NBGMRect2dMap::iterator it = m_Rect2ds.find(rectId);
        if(it != m_Rect2ds.end())
        {
            NBGM_CustomRect2d *rect = it->second;
            if(rect != NULL)
            {
                rect->SetVisible(visible);
            }
        }
    }
}

void NBGMViewController::SetRect2dSize(NBGM_Rect2dId rectId, float width, float height)
{
    if(m_nbgmMapView)
    {
        NBGMRect2dMap::iterator it = m_Rect2ds.find(rectId);
        if(it != m_Rect2ds.end())
        {
            NBGM_CustomRect2d *rect = it->second;
            if(rect != NULL)
            {
                rect->SetSize(width, height);
            }
        }
    }
}

void NBGMViewController::RemoveAllRect2ds()
{
    m_Rect2ds.clear();
    if(m_nbgmMapView)
    {
        m_nbgmMapView->RemoveAllRect2ds();
    }
}

bool NBGMViewController::AddTexture(NBGM_TextureId textureId, const NBGM_BinaryBuffer &textureData)
{
    if(m_nbgmMapView)
    {
        return m_nbgmMapView->AddTexture(textureId, textureData);
    }
    return false;
}

void NBGMViewController::RemoveTexture(NBGM_TextureId textureId)
{
    if(m_nbgmMapView)
    {
        m_nbgmMapView->RemoveTexture(textureId);
    }
}

void NBGMViewController::RemoveAllTextures()
{
    if(m_nbgmMapView)
    {
        m_nbgmMapView->RemoveAllTextures();
    }
}

void NBGMViewController::ShowBubble(shared_ptr<string> pinId, bool visible, float x, float y)
{
    if(!pinId || pinId->empty() || !m_bubblelistener)
    {
        return;
    }

    bool flag = (pinId->compare(m_prePinId) == 0);
    if(m_showBubble == visible && flag)
    {
        return;
    }

    if(!flag && m_showBubble && visible)
    {
        m_bubblelistener->NotifyHideBubble(shared_ptr<string> (new string(m_prePinId)));
    }

    if(visible)
    {
        m_bubblelistener->NotifyShowBubble(pinId, x, y, true);
    }
    else
    {
        m_bubblelistener->NotifyHideBubble(pinId);
    }
    m_showBubble = visible;
    m_prePinId = *pinId;
}

shared_ptr<PinGroup> NBGMViewController::getPinGroupByGroupId(string groupId)
{
    if(m_pinGroups.find(groupId) == m_pinGroups.end())
    {
        m_pinGroups[groupId] = shared_ptr<PinGroup>(new PinGroup());
    }
    return m_pinGroups[groupId];
}

shared_ptr<PinGroup> NBGMViewController::getPinGroupByPinId(string pinId)
{
    map<string, shared_ptr<PinGroup> >::iterator itor;
    for(itor = m_pinGroups.begin(); itor != m_pinGroups.end(); itor++)
    {
        shared_ptr<PinGroup> pinGroup = itor->second;
        for(int i = 0; i < pinGroup->m_pinIds.size(); i++)
        {
            if(*(pinGroup->m_pinIds[i]) == pinId)
            {
                return pinGroup;
            }
        }
    }
    return NULL;
}

void NBGMViewController::SetFontScale(float scale)
{
    if(m_nbgmMapView)
    {
        m_nbgmMapView->SetFontScale(scale);
    }
}

void NBGMViewController::SetAvatarScale(float scale)
{
    if(m_nbgmMapView)
    {
        m_nbgmMapView->SetAvatarScale(scale);
    }
}

void NBGMViewController::SetGestureProcessingFlag(bool gestureProcessing)
{
    m_gestrueProcessing = gestureProcessing;
}

void
NBGMViewController::AddPins(const std::vector<NBGM_PinParameters>& pinParameters)
{
    if(m_nbgmMapView)
    {
        for(int i = 0; i < pinParameters.size(); i++)
        {
            shared_ptr<PinGroup> pinGroup = getPinGroupByGroupId(pinParameters[i].groupID);
            pinGroup->addPin(pinParameters[i].pinID);
        }
        m_nbgmMapView->AddPins(pinParameters);
    }
}

void
NBGMViewController::RemovePins(const vector<shared_ptr<string> >& pinIDs)
{
    if(m_nbgmMapView)
    {
        vector<shared_ptr<string> > pinIDsCopy = pinIDs;
        map<string, shared_ptr<PinGroup> >::iterator itor;
        for(itor = m_pinGroups.begin(); itor != m_pinGroups.end(); itor++)
        {
            shared_ptr<PinGroup> pinGroup = itor->second;
            for(int i = 0; i < pinGroup->m_pinIds.size();)
            {
                bool ifErase = false;
                for(int j = 0; j < pinIDsCopy.size(); )
                {
                    if(*(pinGroup->m_pinIds[i]) == *pinIDsCopy[j])
                    {
                        pinGroup->m_pinIds.erase(pinGroup->m_pinIds.begin() + i);
                        pinIDsCopy.erase(pinIDsCopy.begin() + j);
                        ifErase = true;
                        break;
                    }
                    else
                    {
                        j++;
                    }
                }
                if(!ifErase)
                {
                    i++;
                }
            }
        }
        m_nbgmMapView->RemovePins(pinIDs);
    }
}

void
NBGMViewController::RemoveAllPins()
{
    if(m_nbgmMapView)
    {
        m_pinGroups.clear();
        m_nbgmMapView->RemoveAllPins();
    }
}

void
NBGMViewController::UpdatePinPosition(shared_ptr<string> pinID, double locationX, double locationY)
{
    if(m_nbgmMapView)
    {
        m_nbgmMapView->UpdatePinPosition(pinID, locationX, locationY);
    }
}


void
NBGMViewController::SetHBAOParameters(const HBAOParameters& parameters)
{
    if(m_nbgmMapView)
    {
        NBGM_HBAOParameters nbgmParameters;
        nbgmParameters.enable = parameters.enable;
        nbgmParameters.radius = parameters.radius;
        nbgmParameters.bias = parameters.bias;
        nbgmParameters.powerExponent = parameters.powerExponent;
        nbgmParameters.detailAO = parameters.detailAO;
        nbgmParameters.coarseAO = parameters.coarseAO;
        nbgmParameters.enableBlur = parameters.enableBlur;
        nbgmParameters.blurRadius = parameters.blurRadius;
        nbgmParameters.blurSharpness = parameters.blurSharpness;
        m_nbgmMapView->SetHBAOParameters(nbgmParameters);
    }
}

void
NBGMViewController::SetGlowParameters(const GlowParameters& parameters)
{
    if(m_nbgmMapView)
    {
        NBGM_GlowParameters nbgmParameters;
        nbgmParameters.enable = parameters.enable;
        nbgmParameters.clearColor[0] = parameters.clearColor[0];
        nbgmParameters.clearColor[1] = parameters.clearColor[1];
        nbgmParameters.clearColor[2] = parameters.clearColor[2];

        nbgmParameters.scaleColor[0] = parameters.scaleColor[0];
        nbgmParameters.scaleColor[1] = parameters.scaleColor[1];
        nbgmParameters.scaleColor[2] = parameters.scaleColor[2];
        nbgmParameters.scaleColor[3] = parameters.scaleColor[3];

        nbgmParameters.blurRadius = parameters.blurRadius;
        nbgmParameters.blurSamples = parameters.blurSamples;
        m_nbgmMapView->SetGlowParameters(nbgmParameters);
    }
}

void
NBGMViewController::SetDPI(float dpi)
{
    if(m_nbgmMapView)
    {
        m_nbgmMapView->SetDPI(dpi);
    }
}

void NBGMViewController::UnloadCommonMaterials()
{
    if(m_nbgmMapView && !m_loadedMaterials.empty())
    {
        // Must set current material to null first.
        m_nbgmMapView->SetCurrentExternalMaterial(COMMON_MATERIAL_CATEGORY, "");
        MaterialRemover functor(m_nbgmMapView);
        for_each(m_loadedMaterials.begin(), m_loadedMaterials.end(), functor);
        m_loadedMaterials.clear();
    }
}


/*! @} */
