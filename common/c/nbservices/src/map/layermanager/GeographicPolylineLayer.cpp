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
  @file         GeographicPolylineLayer.cpp
  @defgroup     nbmap
*/
/*
  (C) Copyright 2013 by TeleCommunications Systems, Inc.

  The information contained herein is confidential, proprietary to
  TeleCommunication Systems, Inc., and considered a trade secret as defined
  in section 499C of the penal code of the State of California. Use of this
  information by anyone other than authorized employees of TeleCommunication
  Systems is granted only under a written non-disclosure agreement, expressly
  prescribing the scope and manner of such use.
  --------------------------------------------------------------------------*/

/*! @{ */

#include "GeographicPolylineLayer.h"
#include "nbmacros.h"
#include <algorithm>
#include <iterator>
#include <functional>
#include "paltestlog.h"
#include "datastreamimplementation.h"
#include "StringUtility.h"
#include "TileImpl.h"
#include "paltestlog.h"
#include "NBUIConfig.h"
#include "TileKeyUtils.h"
#include "paldebuglog.h"

using namespace std;
using namespace nbmap;
using namespace nbcommon;

// Local constant variables ................................................................

static const uint32 POLYLINE_DEFAULT_ZOOMLEVEL   = 16;
//The requirement need the polyline layer draw order between the route layer and LM3D/B3D layer.
//The LM3D draw order is 148, B3D is 144, so the polyline layer draw order is 144 - 1.
static const uint32 POLYLINE_LAYER_DEFAULT_DRAWORDER = 143;

// Template to generate a content ID
static const char PARAMETER_GEOGRAPHIC_POLYLINE_LAYER_ID[]   = "$layerid";
static const char PARAMETER_GEOGRAPHIC_POLYLINE_TILE_X[]     = "$x";
static const char PARAMETER_GEOGRAPHIC_POLYLINE_TILE_Y[]     = "$y";
static const char PARAMETER_GEOGRAPHIC_POLYLINE_ZOOM_LEVEL[] = "$z";
static const char GEOGRAPHIC_POLYLINE_CONTENT_ID_TEMPLATE[]  = "GEOGRAPHIC_$layerid_$x_$y_$z";

static const string GEOGRAPHIC_POLYLINE_LAYER_NAME("GeographicPolylineLayer");

// Implementation of GeographicPolylineLayer

/* See description in header file. */
GeographicPolylineLayer::GeographicPolylineLayer(NB_Context* context,
                                                 LayerIdGeneratorPtr layerIdGenerator,
                                                 GeographicLayerListener* listener,
                                                 const GeoPolylineOption& option)
        : GeographicLayer(shared_ptr<string> (new string(GEOGRAPHIC_POLYLINE_LAYER_NAME)),
                          layerIdGenerator->GenerateId(), context, listener),
          m_tileAvaiable(false),
          m_polyline(CCC_NEW GeoPolylineImpl(*this, option)),
          m_tileGenerator(CCC_NEW GeoPolylineTileGenerator(m_tileLayerInfo->materialCategory))
{
    // Setup this layer (parent layer).
    uint32 baseLevel = POLYLINE_DEFAULT_ZOOMLEVEL;
    SetReferenceTileGridLevel(baseLevel);
    SetMinZoom(baseLevel);
    SetMaxZoom(static_cast<uint32>(NBUIConfig::getMaxZoom()));
    SetDrawOrder(POLYLINE_LAYER_DEFAULT_DRAWORDER);

    // Now add child layers...
    // I don't like these hard-coded values, but it is copied from RTS SDS.

    // | Polyline Layer | Reference Tile Grid Level | Min Z-Level |
    // |              0 |                         0 |           0 |
    // |              1 |                         4 |           4 |
    // |              2 |                         8 |           8 |
    // |              3 |                        12 |          12 |
    // |              4 |                        16 |          16 |

    uint32 levelInfo[][3] = { {0, 0, 3}, {4, 4, 7}, {8, 8, 11}, {12, 12, 15}};
    size_t size = sizeof(levelInfo) / sizeof(levelInfo[0]);
    for (size_t i = 0; i < size; ++i)
    {
        GeographicPolylineLayerPtr child(
            CCC_NEW GeographicPolylineLayer(m_tileLayerInfo->tileDataType,
                                            layerIdGenerator->GenerateId(), context,
                                            m_tileLayerInfo->materialCategory,
                                            m_tileGenerator));
        if (child)
        {
            child->SetReferenceTileGridLevel(levelInfo[i][0]);
            child->SetMinZoom(levelInfo[i][1]);
            child->SetMaxZoom(levelInfo[i][2]);
            AddChildrenLayer(child);
        }
    }
}

/* See description in header file. */
GeographicPolylineLayer::GeographicPolylineLayer(shared_ptr<string> parentName,
                                                 uint32 layerIdDigital,
                                                 NB_Context* context,
                                                 shared_ptr<const string> materialCategory,
                                                 GeoPolylineTileGeneratorPtr tileGenerator)
        : GeographicLayer(shared_ptr<string> (new string(*parentName)), layerIdDigital, context),
          m_tileAvaiable(false),
          m_tileGenerator(tileGenerator)
{
    m_tileLayerInfo->materialCategory = materialCategory;
}

/* See description in header file. */
GeographicPolylineLayer::~GeographicPolylineLayer()
{
}

/* See description in header file. */
void GeographicPolylineLayer::GetTile(TileKeyPtr tileKey,
                                      TileKeyPtr convertedTileKey,
                                      AsyncTileRequestWithRequestPtr callback,
                                      uint32 priority,
                                      bool cachedOnly)
{
    if (!callback)
    {
        return;
    }

    NB_Error error = NE_OK;
    do
    {
        //@todo: do we need cache ???
        if (cachedOnly || !m_tileAvaiable)
        {
            error = NE_NOENT;
            break;
        }

        DataStreamPtr data;
        error = m_tileGenerator->GenerateTileData(convertedTileKey, data);
        if (error != NE_OK)
        {
            break;
        }

//        static int count =0;
//        if(data->GetDataSize())
//        {
//            char buffer[255];
//            nsl_memset(buffer, 0, 255);
//            sprintf(buffer, "/data/data/com.navbuilder.sample.navkituisampleapp/files/jasonbai[%d-%d-%d]", convertedTileKey->m_x, convertedTileKey->m_y, convertedTileKey->m_zoomLevel);
//            //sprintf(buffer, "/data/data/com.locationtoolkit.sample.map3d/files/jasonbai[%d-%d-%d]", convertedTileKey->m_x, convertedTileKey->m_y, convertedTileKey->m_zoomLevel);
//            PAL_File* palFile = NULL;
//            PAL_Error palError = PAL_FileOpen(NB_ContextGetPal(m_pContext), buffer, PFM_Create, &palFile);
//            if (palFile)
//            {
//                data->DumpToPALFile(palFile);
//                PAL_FileClose(palFile);
//                DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_DEBUG, ("###Tilekey.x[%d]y[%d]z[%d]-convertedTileKey.x[%d]y[%d]z[%d]-NBM DATA SIZE[%d]",
//                        tileKey->m_x, tileKey->m_y, tileKey->m_zoomLevel, convertedTileKey->m_x, convertedTileKey->m_y, convertedTileKey->m_zoomLevel,
//                        data->GetDataSize()));
//            }
//            else
//            {
//                DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_DEBUG, ("###[0-3]error PAL_FileOpen"));
//            }
//
//
//        }

        shared_ptr<string> contentId = GetContentId(convertedTileKey);
        if (!contentId || contentId->empty())
        {
            error = NE_UNEXPECTED;
            break;
        }

        TileImplPtr tile(new TileImpl(contentId, m_tileLayerInfo, false));
        tile->SetData(data);
        tile->SetTileKey(convertedTileKey);

        callback->Success(tileKey, tile);
    } while (0);

    if (error)
    {
        callback->Error(tileKey, error);
    }
}

/* See description in header file. */
shared_ptr<string> GeographicPolylineLayer::GetContentId(const TileKeyPtr& tileKey)
{
    shared_ptr<string> contentId (CCC_NEW string(GEOGRAPHIC_POLYLINE_CONTENT_ID_TEMPLATE));
    if (contentId)
    {
        // Format template with layer ID.
        StringUtility::ReplaceString(*contentId, PARAMETER_GEOGRAPHIC_POLYLINE_LAYER_ID,
                                     *(GetTileDataType()));

        // Format template with tile key.
        StringUtility::ReplaceString(*contentId, PARAMETER_GEOGRAPHIC_POLYLINE_TILE_X,
                                     StringUtility::NumberToString(tileKey->m_x));

        StringUtility::ReplaceString(*contentId, PARAMETER_GEOGRAPHIC_POLYLINE_TILE_Y,
                                     StringUtility::NumberToString(tileKey->m_y));

        StringUtility::ReplaceString(*contentId, PARAMETER_GEOGRAPHIC_POLYLINE_ZOOM_LEVEL,
                                     StringUtility::NumberToString(tileKey->m_zoomLevel));
    }

    return contentId;
}

/* See description in header file. */
void GeographicPolylineLayer::GraphicObjectSelected(bool selected)
{
    size_t index = selected ? 0 : 1;
    if (m_materials.size() > index)
    {
        ExternalMaterialPtr aaa = m_materials[index];
        //DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_DEBUG, ("###Polyline name[%s]-[%s]-[%d]", aaa->m_categoryName->c_str(), aaa->m_materialName->c_str(), aaa->m_materialData->GetDataSize()));
        m_listener->MaterialActivate(m_materials[index]);
    }
}

/* See description in header file. */
GeographicUpdateType GeographicPolylineLayer::ProcessUpdatedGraphic(Geographic* /*graphic*/)
{
    uint32 subDrawOrder = m_polyline->GetDrawOrder();
    SetSubDrawOrder(subDrawOrder);

    bool                              regenerate = m_polyline->NeedsUpdate();
    vector<UnifiedLayerPtr>           children   = GetAllChildrenLayers();
    vector<UnifiedLayerPtr>::iterator iter       = children.begin();
    vector<UnifiedLayerPtr>::iterator end        = children.end();
    if (!regenerate)
    {
        for (; iter != end; ++iter)
        {
            if (*iter)
            {
                (*iter)->SetSubDrawOrder(subDrawOrder);
            }
        }
        return GUT_Reload;
    }

    m_tileGenerator->Reset();

    // @tricky: External materials can be unloaded because they are in use, so we add
    //          new materials first, then reload tiles using new material, and unload
    //          old materials at last.
    m_oldMaterials.swap(m_materials);
    FeedTileGenerator();
    LoadAndActivateExteranMaterials();
    int tileCount = m_tileGenerator->PrepareForZoomLevel(GetReferenceTileGridLevel());
    m_tileAvaiable = tileCount ? true : false;
    DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_DEBUG, ("###Polyline zoomlevel[%d] has [%d] tiles", GetReferenceTileGridLevel(), tileCount));


    for (; iter != end; ++iter)
    {
        GeographicPolylineLayer* layer = static_cast<GeographicPolylineLayer*>(iter->get());
        if (layer)
        {
            layer->SetSubDrawOrder(subDrawOrder);
            int count = m_tileGenerator->PrepareForZoomLevel(layer->GetReferenceTileGridLevel());
            layer->m_tileAvaiable = count ? true : false;
            DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_DEBUG, ("###Polyline zoomlevel[%d] has [%d] tiles", layer->GetReferenceTileGridLevel(), count));
        }
    }
    return GUT_Refresh;
}

/* See description in header file. */
void GeographicPolylineLayer::PostGraphicUpdated(Geographic* /*graphic*/)
{
    UnloadExternalMaterials(true);
}

/* See description in header file. */
void GeographicPolylineLayer::UnloadExternalMaterials(bool old)
{
    vector<ExternalMaterialPtr>& materials = old ? m_oldMaterials : m_materials;

    if (m_listener && !m_materials.empty())
    {
        for_each(materials.begin(), materials.end(),
                 bind1st(mem_fun(&GeographicLayerListener::MaterialRemoved), m_listener));
    }
}

/* See description in header file. */
void GeographicPolylineLayer::LoadAndActivateExteranMaterials()
{
    m_materials.clear();
    m_tileGenerator->GetMaterials(m_materials);
    if (!m_materials.empty() && m_listener)
    {
        // Tell NBGM to load all materials.
        for_each(m_materials.begin(), m_materials.end(),
                 bind1st(mem_fun(&GeographicLayerListener::MaterialAdded), m_listener));

        // Tell NBGM to activate the second one.
        m_listener->MaterialActivate(m_materials[m_polyline->IsSelected() ? 0 : m_materials.size() - 1]);
//        std::vector<ExternalMaterialPtr>::iterator it = m_materials.begin();
//        static int tt = 0;
//        for(; it < m_materials.end(); it++)
//        {
//            char buffer[255];
//            nsl_memset(buffer, 0, 255);
//            sprintf(buffer, "/data/data/com.navbuilder.sample.navkituisampleapp/files/ExternalMaterial[%d]", tt++);
//            //sprintf(buffer, "/data/data/com.locationtoolkit.sample.map3d/files/ExternalMaterial[%d]", tt++);
//            PAL_File* palFile = NULL;
//            PAL_Error palError = PAL_FileOpen(NB_ContextGetPal(m_pContext), buffer, PFM_Create, &palFile);
//            if (palFile)
//            {
//                (*it)->m_materialData->DumpToPALFile(palFile);
//                PAL_FileClose(palFile);
////                DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_DEBUG, ("###Tilekey.x[%d]y[%d]z[%d]-convertedTileKey.x[%d]y[%d]z[%d]-NBM DATA SIZE[%d]",
////                        tileKey->m_x, tileKey->m_y, tileKey->m_zoomLevel, convertedTileKey->m_x, convertedTileKey->m_y, convertedTileKey->m_zoomLevel,
////                        data->GetDataSize()));
//            }
//            else
//            {
//                DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_DEBUG, ("###ExternalMaterial error PAL_FileOpen"));
//            }
//        }
    }
}

/* See description in header file. */
GeoPolylineImpl* GeographicPolylineLayer::GetPolylineObject()
{
    return m_polyline.get();
}


/*! @} */
