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
    @file       CustomLayer.cpp

    See header file for description.
*/
/*
    (C) Copyright 2014 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

extern "C"
{
#include "csltypes.h"
#include "cslutil.h"
#include "transformutility.h"
#include "palclock.h"
}

#include "palmath.h"
#include "CustomLayer.h"
#include "PinImpl.h"
#include "CustomLayerManagerImpl.h"
#include "TileImpl.h"
#include "datastreamimplementation.h"
#include "StringUtility.h"
#include "LayerFunctors.h"
#include "PinManagerImpl.h"
#include "UnifiedLayer.h" // Several useful macros are defined there.
#include "PinMaterialParser.h"
#include "paltestlog.h"
#include "paluitaskqueue.h"
#include "nbcontextprotected.h"
#include "TileKeyUtils.h"
#include "nbmacros.h"

#include <sstream>
#include <limits>
#include <map>
#include <set>
#include <iterator>

// Disable warning C4245: "conversion from 'int' to 'unsigned int', signed/unsigned mismatch"
// (we don't own this code, the core team does!)
#pragma warning( push )
#pragma warning(disable:4245)
    #include "NBMMaterialChunk.h"
#pragma warning( pop )


/*! @{ */

using namespace nbcommon;
using namespace nbmap;

// Default draw order for pin layer
#define DEFAULT_CUSTOM_LAYER_DRAW_ORDER 209

// Constants ....................................................................................

// Default reference tile grid level of custom layers
//const uint32 CUSTOM_LAYER_REFERENCE_TILE_GRID_LEVEL = 12;

// Template to generate a content ID
const char PARAMETER_CUSTOM_LAYER_ID[]   = "$layerid";
const char PARAMETER_CUSTOM_LAYER_PIN_TILE_X[]     = "$x";
const char PARAMETER_CUSTOM_LAYER_PIN_TILE_Y[]     = "$y";
const char PARAMETER_CUSTOM_LAYER_PIN_ZOOM_LEVEL[] = "$z";
//const char PARAMETER_CUSTOM_LAYER_PIN_UNIFIED_ID[] = "$uid";
const char CUSTOM_LAYER_PIN_CONTENT_ID_TEMPLATE[]  = "CUSTOM_LAYER_PIN_$layerid_$x_$y_$z";

#define RGBA_TO_BGRA(X) ((X & 0x00FF00FF) | ((X >> 16) & 0xFF00) | ((X & 0xFF00) << 16))
// Underscore used to append timestamp to the content ID of tile
static const char   UNDERSCORE[] = "_";

class InterestData
{
public:
    InterestData(TileKey tileKey, CustomLayer& customLayer):m_tileKey(tileKey), m_customLayer(customLayer){}
    ~InterestData(){}

    TileKey m_tileKey;
    CustomLayer& m_customLayer;
private:
    NB_DISABLE_COPY(InterestData);
};
// Local structures.

class ErrorNotifier
{
public:
    ErrorNotifier(AsyncTileRequestWithRequestPtr callback,
                  NB_Error error)
            : m_pCallback(callback),
              m_error(error)
    { }

    virtual ~ErrorNotifier()
    { }

    void operator() (const TileKeyPtr& key) const
    {
        if (m_pCallback && key)
        {
            m_pCallback->Error(key, m_error);
        }
    }

private:
    AsyncTileRequestWithRequestPtr m_pCallback;
    NB_Error m_error;
};

class FindPinByIDFunctor
{
public:
    FindPinByIDFunctor(shared_ptr<string> pinId)
            : m_pinId(pinId) {}
    virtual ~FindPinByIDFunctor(){}

    bool operator() (const PinPtr& pin) const
    {
        return pin ? StringUtility::IsStringEqual(pin->GetPinID(), m_pinId) : false;
    }

private:
    shared_ptr<string> m_pinId;
};

class TileKeySetToPtrVector
{
public:
    TileKeySetToPtrVector(vector<TileKeyPtr>& container)
            : m_container(container)  {}
    virtual ~TileKeySetToPtrVector() {}

    void operator() (const TileKey& key)
    {
        m_container.push_back(TileKeyPtr(new TileKey(key)));
    }

    // Empty assignment operator to avoid warning C4512: "assignment operator could not be generated" (due to const member)
    TileKeySetToPtrVector& operator =(const TileKeySetToPtrVector& /*input*/) {return *this;};

private:
    vector<TileKeyPtr>& m_container;
};

class RecordPinCoordinateFunctor
{
public:
    RecordPinCoordinateFunctor(shared_ptr<vector<NB_LatitudeLongitude> > record)
            : m_records(record) {}
    virtual ~RecordPinCoordinateFunctor(){}

    void operator() (const PinPtr& pin)
    {
        if (pin && m_records)
        {
            NB_LatitudeLongitude coordinate = {0};
            coordinate.latitude = pin->GetLatitude();
            coordinate.longitude = pin->GetLongitude();
            m_records->push_back(coordinate);
        }
    }

private:
    shared_ptr<vector<NB_LatitudeLongitude> > m_records;
};

// Implementation of CustomLayer
/* See header file for description */
CustomLayer::CustomLayer(NB_Context* context,
                         shared_ptr<string> layerID,
                         shared_ptr<string> materialConfigPath,
                         uint32  layerIdDigital,
                         CustomLayerManager* customLayerManager,
                         TileProviderPtr provider, int mainOrder, int subOrder, int refZoom,
                         int minZoom, int maxZoom, bool visible
                        ): UnifiedLayer(TileManagerPtr(), layerIdDigital, context),
                           m_currentGeneratedID(0),
                           m_layerIdDigital(layerIdDigital),
                           m_IdSeperator(ID_SEPERATOR),
                           m_layerID(layerID),
                           m_materialConfigPath(materialConfigPath),
                           m_customLayerManager(customLayerManager),
                           m_materialParser(materialConfigPath ? m_materialConfigPath->c_str() : NULL),
                           m_provider(provider),
                           m_mainOrder(mainOrder),
                           m_subOrder(subOrder),
                           m_refZoom(refZoom),
                           m_minZoom(minZoom),
                           m_maxZoom(maxZoom),
                           m_visible(visible),
                           m_pal(NB_ContextGetPal(context))
{
    // @note: After creating CustomLayer, use CustomLayer::IsValid() to check whether this
    //        object is valid. Do not use it if it is in invalid state.
    if (m_tileLayerInfo)
    {
        m_tileLayerInfo->drawOrder = m_mainOrder;
        m_tileLayerInfo->subDrawOrder = m_subOrder;
        m_tileLayerInfo->tileDataType = layerID;
        m_tileLayerInfo->refTileGridLevel = refZoom;
        //Custom layer should only work from min to max zoom level. The min and max must be >= the reference.
        m_tileLayerInfo->minZoom = m_minZoom;
        m_tileLayerInfo->maxZoom = m_maxZoom;

        /*! @TODO: Hard-coded value, here we use the 'MAT' as the category name, which relate to NBGM
            internal logic. so please don't change this before you discuss with NBGM team.
        */
        m_tileLayerInfo->materialCategory.reset(new string("MAT"));
    }
    m_isOverlay     = false;
    m_isRasterLayer = false;

    // Update characteristics.
    SetCharacteristics(TILE_ADDITIONAL_KEY_OPTIONAL, "");
    if (layerID)
    {
        SetCharacteristics(TILE_ADDITIONAL_KEY_NAME, *layerID);
    }
    ProcessCharacteristics();

    // Set this layer as enabled by default!
    SetEnabled(visible);
}

/* See header file for description */
CustomLayer::~CustomLayer()
{
    // Nothing to do here.
}

/* See description in Layer.h */
shared_ptr<string>
CustomLayer::GetTileDataType()
{
    // Layer ID is data type of this layer.
    return m_layerID;
}

shared_ptr<string> CustomLayer::GetLayerID()
{
    return m_layerID;
}

void
CustomLayer::Invalidate(int x, int y, int zoom)
{
    TileKeyPtr tileKeyToRefresh(new TileKey(x, y, zoom));
    if(!tileKeyToRefresh)
    {
        return;
    }

    int refTileGridLevel = static_cast<int>(m_tileLayerInfo->refTileGridLevel);
    if(refTileGridLevel != 0)
    {
        if (tileKeyToRefresh->m_zoomLevel > refTileGridLevel)
        {
            tileKeyToRefresh = ConvertTileKeyToLessZoomLevel(tileKeyToRefresh, refTileGridLevel);
            if (!tileKeyToRefresh)
            {
                return;
            }
        }
    }

    std::map<TileKey, nbcommon::DataStreamPtr> ::iterator it = m_cache.find(*tileKeyToRefresh);
    if(it != m_cache.end())
    {
       m_cache.erase(it);
    }
    m_refreshTiles.insert(*tileKeyToRefresh);

    vector<TileKeyPtr> tileKeysToRefresh;
    tileKeysToRefresh.push_back(tileKeyToRefresh);
    RefreshPinsOfTiles(tileKeysToRefresh);
}

void
CustomLayer::Invalidate(int /*zoom*/)
{
    //todo:Need consider more for this.
}

void
CustomLayer::Invalidate()
{
    m_cache.clear();
    m_markerIDMap.clear();
    m_refreshTiles.clear();
    RefreshAllPins();
}

bool
CustomLayer::GetMarkerID(const std::string& internalMarkerID, int& userMarkerID)
{
    if(internalMarkerID.empty())
    {
        return false;
    }

    std::map<std::string, int>::iterator it = m_markerIDMap.find(internalMarkerID);
    if(it == m_markerIDMap.end())
    {
        return false;
    }
    userMarkerID = it->second;
    return true;
}

void
CustomLayer::AppendMarkerID(const std::map<std::string, int>& markerIDMap)
{
    std::map<std::string, int>::const_iterator iter = markerIDMap.begin();
    std::map<std::string, int>::const_iterator end  = markerIDMap.end();
    for (; iter != end; ++iter)
    {
        m_markerIDMap.insert(std::pair<std::string, int>(iter->first, iter->second));
    }
}

void
CustomLayer::LoadTile(CustomNBMTilePtr customNBMTilePtr)
{
    if(!customNBMTilePtr || !customNBMTilePtr->m_dataStreamPtr)
    {
        return;
    }

    std::pair<std::map<TileKey, nbcommon::DataStreamPtr>::iterator, bool> ret;
    ret = m_cache.insert(std::pair<TileKey, nbcommon::DataStreamPtr>(customNBMTilePtr->m_tileKey, customNBMTilePtr->m_dataStreamPtr) );
    AppendMarkerID(customNBMTilePtr->m_markerIDMap);
    if (ret.second)
    {
        //insert success
        vector<TileKeyPtr> tileKeysToRefresh;
        tileKeysToRefresh.push_back(TileKeyPtr(new TileKey(customNBMTilePtr->m_tileKey)));
        RefreshPinsOfTiles(tileKeysToRefresh);
    }
}

/* See description in Layer.h */
void
CustomLayer::GetTiles(vector<TileKeyPtr> tileKeys,
                      AsyncTileRequestWithRequestPtr callback,
                      uint32 /*priority*/)
{
    if (!IsEnabled())
    {
        return;
    }

    // Tile key cannot be NULL. Also, pinTiles will not be cached, so just skip request if
    // no callback is provided, or we are wasting time.
    if (tileKeys.empty() || !callback)
    {
        return;
    }

    // Check 'm_tileLayerInfo' in function IsValid.
    if (!IsValid() && callback)
    {
        ErrorNotifier functor(callback, NE_NOTINIT);
        for_each (tileKeys.begin(), tileKeys.end(), functor);
        return;
    }

    vector<TileKeyPtr>::const_iterator iter = tileKeys.begin();
    vector<TileKeyPtr>::const_iterator end  = tileKeys.end();

    set<TileKey> tileRequestFilter;

    for (; iter != end; ++iter)
    {
        const TileKeyPtr& tileKey = *iter;

        // Skip out of range requests.
        if (tileKey->m_zoomLevel < (int)m_tileLayerInfo->minZoom ||
            tileKey->m_zoomLevel > (int)m_tileLayerInfo->maxZoom)
        {
            continue;
        }

        /* If zoom level of tile key is greater than reference tile grid zoom level of
           this pin layer, convert this tile key to a tile key of reference tile grid
           zoom level. */
        TileKeyPtr tileKeyToRequest = tileKey;
        int refTileGridLevel = static_cast<int>(m_tileLayerInfo->refTileGridLevel);
        if(refTileGridLevel != 0)
        {
            if (tileKeyToRequest->m_zoomLevel > refTileGridLevel)
            {
                tileKeyToRequest = ConvertTileKeyToLessZoomLevel(tileKeyToRequest, refTileGridLevel);
                if (!tileKeyToRequest)
                {
                    if (callback)
                    {
                        callback->Error(tileKey, NE_UNEXPECTED);
                    }
                    continue;
                }
            }
        }

        pair<set<TileKey>::iterator, bool> filterResult =
                tileRequestFilter.insert(*tileKeyToRequest);
        if (!filterResult.second) // Skip duplicated requests.
        {
            continue;
        }

        GetTile(tileKey, tileKeyToRequest, callback);
    }
}

void
CustomLayer::UI_Interest(PAL_Instance* pal, void* userData)
{
    InterestData *interestData = static_cast<InterestData*>(userData);
    if(!pal || !interestData)
    {
       return;
    }

    CustomLayer& customLayer = interestData->m_customLayer;
    customLayer.m_provider->interest(interestData->m_tileKey.m_x,
                                     interestData->m_tileKey.m_y,
                                     interestData->m_tileKey.m_zoomLevel);
    delete interestData;
}

void
CustomLayer::GetTile(TileKeyPtr tileKey, TileKeyPtr convertedTileKey,
                      AsyncTileRequestWithRequestPtr callback)
{
    if (!callback || !m_provider)
    {
        return;
    }

    //Check this tile key is refresh tile key or not, if yes, trigger the interest request.
    std::set<TileKey>::iterator iterTileKey = m_refreshTiles.find(*convertedTileKey);
    if(iterTileKey != m_refreshTiles.end())
    {
        callback->Error(tileKey, NE_NOENT);
        PAL_UiTaskQueueAdd(m_pal, UI_Interest, new InterestData(*convertedTileKey, *this));
        m_refreshTiles.erase(iterTileKey);
        return;
    }

    std::map<TileKey, nbcommon::DataStreamPtr >::iterator it = m_cache.find(*convertedTileKey);
    if(it == m_cache.end())
    {
        callback->Error(tileKey, NE_NOENT);
        PAL_UiTaskQueueAdd(m_pal, UI_Interest, new InterestData(*convertedTileKey, *this));
        return;
    }

    nbcommon::DataStreamPtr& tileData = it->second;
    if(!tileData)
    {
        callback->Error(tileKey, NE_NOENT);
        return;
    }

    TileImplPtr tile;
    // Get content ID of the pin tile.
    shared_ptr<string> contentId = GetContentID(*convertedTileKey);
    if ((!contentId) || (contentId->empty()))
    {
        callback->Error(tileKey, NE_UNEXPECTED);
        return;
    }

    tile.reset(new TileImpl(contentId, m_tileLayerInfo, false));
    if (!tile)
    {
        callback->Error(tileKey, NE_NOMEM);
        return;
    }

    // Data of the pin tile must not be empty.
    if ((!tileData) || (tileData->GetDataSize() == 0))
    {
        callback->Error(tileKey, NE_UNEXPECTED);
        return;
    }

    // Set the tile data.
    tile->SetData(tileData);
    tile->SetTileKey(convertedTileKey);
    callback->Success(tileKey, tile);
}


/* See description in Layer.h */
void
CustomLayer::RemoveAllTiles()
{
    // Nothing to do here.
}

/* See description in PinLayer.h */
uint32
CustomLayer::GetID()
{
    return m_tileLayerInfo->layerID;
}

/* See description in PinLayer.h */
void
CustomLayer::RemovePins(const vector<shared_ptr<string> >& pinIDs)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    // Check if the vector of pin IDs is empty.
    if (pinIDs.empty())
    {
        return;
    }

    // Find and erase pins by pin IDs.

    set<TileKey> uniqueTileKeys;
    shared_ptr<vector<PinPtr> > pinsToRemove(new vector<PinPtr>());

    vector<shared_ptr<string> >::const_iterator idIterator = pinIDs.begin();
    vector<shared_ptr<string> >::const_iterator idEnd = pinIDs.end();
    for (; idIterator != idEnd; ++idIterator)
    {
        // Get the pin ID.
        shared_ptr<string> pinId = *idIterator;
        if (!pinId)
        {
            continue;
        }

        // Find the pin information by pin ID.
        FindPinByIDFunctor functor(pinId);
        map<TileKey, vector<PinPtr> >::iterator iter = m_Pins.begin();
        map<TileKey, vector<PinPtr> >::iterator end  = m_Pins.end();
        for (; iter != end; ++iter)
        {
            vector<PinPtr>& pins = iter->second;
            vector<PinPtr>::iterator pinEnd = pins.end();
            vector<PinPtr>::iterator pinIter = find_if (pins.begin(), pinEnd, functor);
            if (pinIter == pinEnd)
            {
                continue; // pinId does not exist in this vector, try next.
            }

            // Pin is found in this vector, record TileKey and pin for later use
            uniqueTileKeys.insert(iter->first);
            PinPtr pin = *pinIter;

            // Clean up.
            pins.erase(pinIter);
            if (pins.empty())
            {
                m_Pins.erase(iter);
            }

            if (pinsToRemove && pin)
            {
                // Add the pin to remove.
                pinsToRemove->push_back(pin);
            }

            break;
        }
    }

    // Notify changed pin tiles to refresh if necessary.
    if (!uniqueTileKeys.empty())
    {
        vector<TileKeyPtr> tileKeysToRefresh;
        TileKeySetToPtrVector functor(tileKeysToRefresh);
        for_each (uniqueTileKeys.begin(), uniqueTileKeys.end(), functor);
        RefreshPinsOfTiles(tileKeysToRefresh);
    }

    // Notify that the pins are removed.
    NotifyPinsRemoved(pinsToRemove);
}

/* See description in PinLayer.h */
void
CustomLayer::RemoveAllPins()
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    if (m_Pins.empty())
    {
        return;
    }

    shared_ptr<vector<PinPtr> > pinsToRemove(new vector<PinPtr>(GetAllPins()));

    vector<TileKeyPtr> tileKeysToRefresh;
    map<TileKey, vector<PinPtr> >::const_iterator iter = m_Pins.begin();
    map<TileKey, vector<PinPtr> >::const_iterator end = m_Pins.end();
    for(; iter != end; ++iter)
    {
        tileKeysToRefresh.push_back(TileKeyPtr(new TileKey(iter->first)));
    }

    m_Pins.clear();
    RefreshPinsOfTiles(tileKeysToRefresh);
    NotifyPinsRemoved(pinsToRemove);
}

/* See description in PinLayer.h */
vector<PinPtr>
CustomLayer::GetAllPins()
{
    vector<PinPtr> vecPins;
    map<TileKey, vector<PinPtr> >::const_iterator iter = m_Pins.begin();
    map<TileKey, vector<PinPtr> >::const_iterator end  = m_Pins.end();
    for (; iter != end; ++iter)
    {
        copy(iter->second.begin(), iter->second.end(), back_inserter(vecPins));
    }

    return vecPins;
}

/* See description in PinLayer.h */
void
CustomLayer::SetMaterialConfigPath(shared_ptr<string> materialConfigPath)
{

    NB_ASSERT_CCC_THREAD(m_pContext);

    m_materialConfigPath = materialConfigPath;
    if (m_materialConfigPath && (!(m_materialConfigPath->empty())))
    {
        m_materialParser.LoadFromFile(m_materialConfigPath->c_str());
    }
}

/* See description in PinLayer.h */
PinPtr
CustomLayer::GetPin(shared_ptr<string> pinID)
{

    NB_ASSERT_CCC_THREAD(m_pContext);

    PinPtr pin;
    if (pinID && !pinID->empty())
    {
        // Find the pin information by pin ID.
        FindPinByIDFunctor functor(pinID);
        map<TileKey, vector<PinPtr> >::const_iterator iter = m_Pins.begin();
        map<TileKey, vector<PinPtr> >::const_iterator end  = m_Pins.end();
        for (; iter != end; ++iter)
        {
            const vector<PinPtr>& pins = iter->second;
            vector<PinPtr>::const_iterator pinEnd = pins.end();
            vector<PinPtr>::const_iterator pinIter = find_if (pins.begin(), pinEnd, functor);
            if (pinIter != pinEnd)
            {
                pin = *pinIter;
                break;
            }
        }
    }
    return pin;
}

/* See description in PinLayer.h */
shared_ptr<vector<NB_LatitudeLongitude> >
CustomLayer::GetAllPinCoordinates()
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    // Check if there are some pins in the layer.
    if (m_Pins.empty())
    {
        return shared_ptr<vector<NB_LatitudeLongitude> >();
    }

    // Create a vector to return the coordinates.
    shared_ptr<vector<NB_LatitudeLongitude> > vecCoordinates( new vector<NB_LatitudeLongitude>());
    if (!vecCoordinates)
    {
        return shared_ptr<vector<NB_LatitudeLongitude> >();
    }

    // Get the coordinates for all pins of the layer.
    map<TileKey, vector<PinPtr> >::const_iterator iter = m_Pins.begin();
    map<TileKey, vector<PinPtr> >::const_iterator end  = m_Pins.end();
    for (; iter != end; ++iter)
    {
        RecordPinCoordinateFunctor functor(vecCoordinates);
        for_each (iter->second.begin(), iter->second.end(), functor);
    }

    return vecCoordinates;
}

// Protected functions .....................................................................

/* See description in PinLayer.h */
shared_ptr<string>
CustomLayer::GeneratePinID()
{
    // Layer ID should not be empty.
    if ((!m_layerID) || (m_layerID->empty()))
    {
        return shared_ptr<string>();
    }

    // This segment is added for extra protection, but I don't think it will be used ...
    if (++m_currentGeneratedID == numeric_limits<uint32>::max())
    {
        m_IdSeperator += StringUtility::NumberToString(PAL_ClockGetUnixTime());
        m_currentGeneratedID = 0;
    }

    // Just increase m_currentGeneratedID and append it to form a unique id.
    shared_ptr<string> pinID(new string((*m_layerID) + m_IdSeperator +
                                        StringUtility::NumberToString(m_currentGeneratedID)));
    return pinID;
}

/* See description in PinLayer.h */
TileKeyPtr
CustomLayer::AddPinPtr(PinPtr pin)
{
    // Check if pin is empty.
    if (!pin)
    {
        return TileKeyPtr();
    }

    // Get pin ID.
    shared_ptr<string> pinID = pin->GetPinID();
    if ((!pinID) || pinID->empty())
    {
        return TileKeyPtr();
    }

    // Get latitude and longitude.
    double latitude  = pin->GetLatitude();
    double longitude = pin->GetLongitude();
    if ((latitude == INVALID_LATLON) ||
        (longitude == INVALID_LATLON))
    {
        return TileKeyPtr();
    }

    // Get a tile key of reference tile grid zoom level.
    TileKeyPtr referenceTileKey = GetReferenceTileKeyByCoordinate(latitude, longitude);
    if (!referenceTileKey)
    {
        return TileKeyPtr();
    }


    // Check if same pinId exists, return empty tileKey if so.
    vector<PinPtr>&          pins = m_Pins[*referenceTileKey];
    vector<PinPtr>::iterator end  = pins.end();
    FindPinByIDFunctor       functor(pinID);
    if (find_if (pins.begin(), end, functor) != end)
    {
        return TileKeyPtr();
    }

    pins.push_back(pin);

    return referenceTileKey;
}

/* See description in PinLayer.h */
void
CustomLayer::RefreshAllPins()
{
    if (m_customLayerManager)
    {
        CustomLayerManagerImpl* manager = static_cast<CustomLayerManagerImpl*>(m_customLayerManager);
        if (manager)
        {
            manager->RefreshAllPinsOfLayer(m_layerIdDigital);
        }
    }
}

/* See description in PinLayer.h */
void
CustomLayer::RefreshPinsOfTiles(const vector<TileKeyPtr>& tileKeys)
{
    if (m_customLayerManager)
    {
        CustomLayerManagerImpl* manager = static_cast<CustomLayerManagerImpl*>(m_customLayerManager);
        if (manager)
        {
            manager->RefreshTilesOfLayer(tileKeys, m_layerIdDigital);
        }
    }
}

/*! Notify that the pins are removed

    @return None
*/
void
CustomLayer::NotifyPinsRemoved(shared_ptr<vector<PinPtr> > pins)
{
    if (pins && (!(pins->empty())) && m_customLayerManager)
    {
        CustomLayerManagerImpl* manager = static_cast<CustomLayerManagerImpl*>(m_customLayerManager);
        if (manager)
        {
            manager->NotifyPinsRemoved(pins);
        }
    }
}

/*! Convert the tile key to a tile key of specified less zoom level

    @return Converted tile key of specified zoom level
*/
TileKeyPtr
CustomLayer::ConvertTileKeyToLessZoomLevel(TileKeyPtr sourceTileKey,   /*!< Source tile key */
                                            int targetZoomLevel         /*!< Specified zoom level to convert */
                                            )
{
    // Check if source tile key is empty.
    if (!sourceTileKey)
    {
        return TileKeyPtr();
    }

    /* If zoom level of source tile key is not greater than target zoom level,
       return source tile key. */
    int sourceZoomLevel = sourceTileKey->m_zoomLevel;
    if (sourceZoomLevel <= targetZoomLevel)
    {
        return sourceTileKey;
    }

    // Create a tile key of target zoom level.
    TileKeyPtr targetTileKey(new TileKey());
    if (!targetTileKey)
    {
        return TileKeyPtr();
    }
    targetTileKey->m_zoomLevel = targetZoomLevel;

    // Convert coordinate x and y with target zoom level.
    int zoomLevelDiffer = sourceZoomLevel - targetZoomLevel;
    targetTileKey->m_x = (sourceTileKey->m_x) >> zoomLevelDiffer;
    targetTileKey->m_y = (sourceTileKey->m_y) >> zoomLevelDiffer;

    return targetTileKey;
}

/*! Convert coordinate to a tile key of reference tile grid zoom level

    @return Converted tile key of reference tile grid zoom level
*/
TileKeyPtr
CustomLayer::GetReferenceTileKeyByCoordinate(double latitude,  /*!< Latitude to convert */
                                              double longitude  /*!< Longitude to convert */
                                              )
{
    // Check 'm_tileLayerInfo' in function IsValid.
    if (!IsValid())
    {
        return TileKeyPtr();
    }

    int refTileGridLevel = static_cast<int>(m_tileLayerInfo->refTileGridLevel);
    int tileX = 0;
    int tileY = 0;
    GetTileByCoordinate(latitude,
                        longitude,
                        refTileGridLevel,
                        tileX,
                        tileY);

    return TileKeyPtr(new TileKey(tileX, tileY, refTileGridLevel));
}

/*! Convert coordinate to tile

    This function converts coordinate to mercator and then converts mercator to tile.

    @return None
*/
void
CustomLayer::GetTileByCoordinate(double latitude,      /*!< Latitude to convert */
                                  double longitude,     /*!< Longitude to convert */
                                  int zoomLevel,        /*!< Zoom level specified */
                                  int& tileX,           /*!< On return converted X position */
                                  int& tileY            /*!< On return converted Y position */
                                  )
{
    double mercatorX = 0.0;
    double mercatorY = 0.0;
    double halfScale = static_cast<double>(1 << (zoomLevel - 1));

    // Convert coordinate to mercator.
    mercatorForward(latitude, longitude, &mercatorX, &mercatorY);

    // Get bound mercator to avoid out of range.
    mercatorX = GetBoundMercator(mercatorX);
    mercatorY = GetBoundMercator(mercatorY);

    tileX = static_cast<int>((mercatorX / PI + 1.0) * halfScale);
    tileY = static_cast<int>((1.0 - mercatorY / PI) * halfScale);
}

/*! Convert tile to coordinate

    This function converts tile to mercator and then converts mercator to coordinate.

    @return None
*/
void
CustomLayer::GetCoordinateByTile(int tileX,            /*!< X position of tile */
                                  int tileY,            /*!< Y position of tile */
                                  int zoomLevel,        /*!< Zoom level of tile */
                                  double& latitude,     /*!< On return converted latitude */
                                  double& longitude     /*!< On return converted longitude */
                                  )
{
    double mercatorX = 0.0;
    double mercatorY = 0.0;
    double halfScale = static_cast<double>(1 << (zoomLevel - 1));

    // Convert tile to mercator.
    mercatorX = (static_cast<double>(tileX) / halfScale - 1.0) * PI;
    mercatorY = (1.0 - static_cast<double>(tileY) / halfScale) * PI;

    // Get bound mercator to avoid out of range.
    mercatorX = GetBoundMercator(mercatorX);
    mercatorY = GetBoundMercator(mercatorY);

    // Convert mercator to coordinate.
    mercatorReverse(mercatorX, mercatorY, &latitude, &longitude);
}

/*! Get bound mercator

    This function limits value of mercator to [-PI, PI).

    @return Bound mercator
*/
double
CustomLayer::GetBoundMercator(double mercator      /*!< Value of mercator to limit */
                               )
{
    // Limit value of mercator to [-PI, PI).
    while (mercator < -PI)
    {
        mercator += TWO_PI;
    }
    while (mercator >= PI)
    {
        mercator -= TWO_PI;
    }

    return mercator;
}

/*! Get content ID of a pin tile by a tile key

    @return Content ID of the pin tile if success, otherwise return empty shared pointer.
*/
shared_ptr<string>
CustomLayer::GetContentID(const TileKey& tileKey)
{
    if (/*pins.empty() || */(!m_layerID) || (m_layerID->empty()))
    {
        return shared_ptr<string>();
    }

    shared_ptr<string> contentId(new string(CUSTOM_LAYER_PIN_CONTENT_ID_TEMPLATE));
    if (!contentId)
    {
        return shared_ptr<string>();
    }

    // Format template with layer ID.
    StringUtility::ReplaceString(*contentId, PARAMETER_CUSTOM_LAYER_ID, *m_layerID);

    // Format template with tile key.
    StringUtility::ReplaceString(*contentId, PARAMETER_CUSTOM_LAYER_PIN_TILE_X,
                                 StringUtility::NumberToString(tileKey.m_x));

    StringUtility::ReplaceString(*contentId, PARAMETER_CUSTOM_LAYER_PIN_TILE_Y,
                                 StringUtility::NumberToString(tileKey.m_y));

    StringUtility::ReplaceString(*contentId, PARAMETER_CUSTOM_LAYER_PIN_ZOOM_LEVEL,
                                 StringUtility::NumberToString(tileKey.m_zoomLevel));

    /* Append the content ID with timestamp. Because NBGM saves tiles with content IDs
      in cache and does not update the tile data if content IDs are same. Even unload
      the tiles, unload operation only make the tiles not display and does not remove
      the tile data from NBGM cache.
   */

    // Format the timestamp to a string.
    contentId->append(UNDERSCORE + StringUtility::NumberToString(PAL_ClockGetGPSTime()));

    return contentId;
}

bool CustomLayer::IsValid()
{
    bool bValid = true;
    if (!m_tileLayerInfo)
    {
        bValid = false;
    }
    return bValid;
}

/* See description in header file. */
void CustomLayer::PostLayerToggled()
{
    RefreshAllPins();
}

/* See description in header file. */
NB_Error CustomLayer::GetCachedTiles(const vector <TileKeyPtr>& /*tileKeys*/,
                                      AsyncTileRequestWithRequestPtr /*callback*/)
{
    return NE_OK;
}

/*! @} */
