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

    @file nbenhancedvectormapprivatetypes.h
    @date 02/21/2012

*/
/*
    (C) Copyright 2012 by TeleCommunication Systems

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret
    as defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly prescribing
    the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef NB_ENHANCED_VECTOR_MAP_PRIVATE_TYPES_H
#define NB_ENHANCED_VECTOR_MAP_PRIVATE_TYPES_H

#include "paltypes.h"
#include "nbcontext.h"
#include "nbrouteinformation.h"
#include "nbnavigation.h"
#include "nbenhancedvectormaptypes.h"
#include "nbenhancedcontenttypes.h"

#include "smartpointer.h"
#include "TileKey.h"
#include "datastream.h"

#include "LayerManagerListener.h"

#include <map>
#include <set>
#include <string>
#include <vector>

#define TDT_DETAILED_VECTOR_AREAS      "DVA"
#define TDT_DETAILED_VECTOR_ROADS      "DVR"
#define TDT_UNTEXTURED_3D_BUILDINGS    "B3D"
#define TDT_LANDMARKS_3D               "LM3D"
#define TDT_MAPTILE_ROAD               "road"
#define TDT_MAPTILE_TRAFFIC            "traffic"
#define TDT_MAPTILE_ROUTE              "route"
#define TDT_MAPTILE_SATELLITE          "satellite"
#define TDT_MAPTILE_HYBRID             "hybrid"
#define TDT_MAPTILE_MAP                "map"
#define TDT_INVALID                    ""

namespace nbmap
{
    class MapServicesConfiguration;
    class LayerManager;
    class VisibleProcessor;
    class PrefetchProcessor;
    class Layer;
}

class MetadataAsyncCallback;
class CommonMaterialAsyncCallback;
class GetTileAsyncCallback;

typedef enum NB_EnhancedVectorMapDataStatusFlag
{
    NB_EnhancedVectorMapData_MetaData           = 0x1
    , NB_EnhancedVectorMapData_DayMaterial      = 0x2
    , NB_EnhancedVectorMapData_NightMaterial    = 0x4
} NB_EnhancedVectorMapDataStatusFlag;

class RefreshMaterialListener
    : public nbmap::LayerManagerListener
{
public:
    RefreshMaterialListener(NB_EnhancedVectorMapManager* pManager) : m_pEnhancedVectorMapManager(pManager) {}
    ~RefreshMaterialListener() {}

    virtual void RefreshAllTiles();
    virtual void RefreshTilesOfLayer(shared_ptr<nbmap::Layer> layer);
    virtual void RefreshTiles(const std::vector<shared_ptr<nbmap::TileKey> >& tileKeys, shared_ptr<nbmap::Layer> layer);
    virtual void LayersUpdated(const vector<shared_ptr<nbmap::Layer> >& layers);
    virtual void LayersAdded(const std::vector<shared_ptr<nbmap::Layer> >& layers);
    virtual void LayersRemoved(const std::vector<shared_ptr<nbmap::Layer> >& layers);
    virtual void AnimationLayerAdded(shared_ptr<nbmap::Layer> layer);
    virtual void AnimationLayerUpdated(shared_ptr<nbmap::Layer> layer, const vector<uint32>& frameList);
    virtual void AnimationLayerRemoved(shared_ptr<nbmap::Layer> layer);
    virtual void PinsRemoved(shared_ptr<std::vector<shared_ptr<nbmap::Pin> > > pins);
    virtual void EnableMapLegend(bool enabled, shared_ptr <nbmap::MapLegendInfo> mapLegend);

private:
    NB_EnhancedVectorMapManager* m_pEnhancedVectorMapManager;
};

#pragma warning(disable: 4610 4510)

struct NB_EnhancedVectorMapManager
{
    NB_Context*                                             context;

    // configurations
    uint32                                                  screenWidth;
    uint32                                                  screenHeight;

    // flags
    uint32                                                  dataStatusFlags;

    // callbacks
    EnhancedVectorMapMetadataRequestCallback                metadataCallback;
    EnhancedVectorMapCommonMaterialRequestCallback          commonMaterialCallback;

    // low layer needed objects
    shared_ptr<std::string>                                 workPath;
    shared_ptr<std::string>                                 language;
    shared_ptr<nbmap::LayerManager>                         layerManager;
    shared_ptr<MetadataAsyncCallback>                       metadataAsyncCallback;
    shared_ptr<CommonMaterialAsyncCallback>                 dayMaterialAsyncCallback;
    shared_ptr<CommonMaterialAsyncCallback>                 nightMaterialAsyncCallback;
    std::vector<shared_ptr<nbmap::Layer> >                  layers;
    RefreshMaterialListener                                 layerManagerListener;

    nbcommon::DataStreamPtr                                 dayMaterial;
    nbcommon::DataStreamPtr                                 nightMaterial;

    uint32                                                  enabledLayers;
};

struct NB_EnhancedCityModelBoundingBoxParameters
{
    NB_RouteId                                              routeId;
    std::set<std::string>                                   ecmDataSetIds;
    std::vector<NB_CityBoundingBox>                         ecmBoundingBoxes;
    std::multimap<double, uint32>                           lowerLatBoundings;
    std::multimap<double, uint32>                           upperLatBoundings;
    std::multimap<double, uint32>                           lowerLonBoundings;
    std::multimap<double, uint32>                           upperLonBoundings;
    double                                                  maxLatSize;
    double                                                  maxLonSize;
};

class RefreshTileListener
    : public nbmap::LayerManagerListener
{
public:
    RefreshTileListener(NB_EnhancedVectorMapState* pState) : m_pEnhancedVectorMapState(pState) {}
    ~RefreshTileListener() {}

    virtual void RefreshAllTiles();
    virtual void RefreshTilesOfLayer(shared_ptr<nbmap::Layer> layer);
    virtual void RefreshTiles(const std::vector<shared_ptr<nbmap::TileKey> >& tileKeys, shared_ptr<nbmap::Layer> layer);
    virtual void LayersUpdated(const vector<shared_ptr<nbmap::Layer> >& layers);
    virtual void LayersAdded(const std::vector<shared_ptr<nbmap::Layer> >& layers);
    virtual void LayersRemoved(const std::vector<shared_ptr<nbmap::Layer> >& layers);
    virtual void AnimationLayerAdded(shared_ptr<nbmap::Layer> layer);
    virtual void AnimationLayerUpdated(shared_ptr<nbmap::Layer> layer, const vector<uint32>& frameList);
    virtual void AnimationLayerRemoved(shared_ptr<nbmap::Layer> layer);
    virtual void PinsRemoved(shared_ptr<std::vector<shared_ptr<nbmap::Pin> > > pins);
    virtual void EnableMapLegend(bool enabled, shared_ptr <nbmap::MapLegendInfo> mapLegend);
private:
    NB_EnhancedVectorMapState* m_pEnhancedVectorMapState;
};

struct NB_EnhancedVectorMapState
{
    NB_Context*                                             context;
    NB_Navigation*                                          navigation;
    NB_EnhancedVectorMapUpdateTileCallback                  updateTileCallback;

    nbmap::VisibleProcessor*                                visibleProcessor;
    nbmap::PrefetchProcessor*                               prefetchProcessor;
    shared_ptr<std::string>                                 workPath;
    shared_ptr<nbmap::LayerManager>                         layerManager;
    shared_ptr<GetTileAsyncCallback>                        tileCallback;

    std::set<nbmap::TileKey>                                visibleTiles;
    std::vector<NB_TileKey>                                 visibleTileKeys;
    std::map<NB_TileId, nbcommon::DataStreamPtr>            tilesToLoad;
    RefreshTileListener                                     layerManagerListener;

    NB_EnhancedCityModelBoundingBoxParameters               ecmBoundingBoxes;

    nb_boolean                                              exclude3DTilesInECM;
};

#endif

/*! @} */
