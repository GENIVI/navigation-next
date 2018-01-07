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

    @file nbenhancedvectormapprocessor.cpp
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

#include "nbenhancedvectormapprocessor.h"
#include "nbenhancedvectormapprocessorprivate.h"
#include "nbenhancedvectormapprivatetypes.h"
#include "nbcontextaccess.h"
#include "nbcontextprotected.h"

#include "VisibleProcessor.h"
#include "PrefetchProcessor.h"
#include "LayerManager.h"

#include "palstdlib.h"
#include "palfile.h"
#include <list>

#include "palmath.h"

extern "C"
{
#include "nbspatial.h"
#include "spatialvector.h"
#include "transformutility.h"
#include "datautil.h"
#include "nbnavigationprivate.h"
#include "nbnavigationstateprivate.h"
#include "nbcontextprotected.h"
#include "nbpointiteration.h"
#include "nbenhancedcontentmanagerprotected.h"
#include "paldebuglog.h"
}

#include <algorithm>

static const char* s_DVA_KeyWord = "DVA";
static const char* s_DVR_KeyWord = "DVR";
static const char* s_LM3D_KeyWord = "LM3D";
static const char* s_B3D_KeyWord = "B3D";

static void
UpdateEnhancedCityModelBoundingBox(NB_EnhancedContentState* enhancedContentState, NB_EnhancedCityModelBoundingBoxParameters* boundingBoxes);

static bool operator <(const NB_TileKey& key1, const NB_TileKey& key2)
{
    if (key1.x == key2.x)
    {
        if (key1.y == key2.y)
        {
            return key1.zoomLevel < key2.zoomLevel;
        }
        return key1.y < key2.y;
    }
    return key1.x < key2.x;
}

static bool operator <(const NB_TileId& id1, const NB_TileId& id2)
{
    if (id1.layerType == id2.layerType)
    {
        return id1.tileKey < id2.tileKey;
    }
    return id1.layerType < id2.layerType;
}
///////////////////////////////////////////////////////////////////////////////////////////////
/*
    GetTileAsyncCallback is used for adapting low layer interface

    To handle asynchronized network request event.

*/
class GetTileAsyncCallback
    : public nbmap::AsyncCallbackWithRequest<nbmap::TileKeyPtr, nbmap::TilePtr>
{
public:
    GetTileAsyncCallback();
    virtual ~GetTileAsyncCallback();

    /* See description in AsyncCallback.h */
    virtual void Success(nbmap::TileKeyPtr request, nbmap::TilePtr response);
    virtual void Error(nbmap::TileKeyPtr request, NB_Error error);
    virtual bool Progress(int percentage);

    inline void SetEnhancedVectorMapState(NB_EnhancedVectorMapState* pState) { m_pEnhancedVectorMapState = pState; }
private:
    GetTileAsyncCallback(const CommonMaterialAsyncCallback&);
    GetTileAsyncCallback& operator=(const CommonMaterialAsyncCallback&);

    NB_EnhancedVectorMapState* m_pEnhancedVectorMapState;
};

///////////////////////////////////////////////////////////////////////////////////////////////
/*
    NB_EnhancedVectorMapManager interface implementation
*/

/*! See header file for detail */
NB_Error
NB_EnhancedVectorMapStateCreate(NB_Context* pContext, NB_Navigation* pNavigation, NB_EnhancedVectorMapConfiguration* configuration, NB_EnhancedVectorMapState** pState)
{
    NB_EnhancedVectorMapManager* pEnhancedVectorMapManager = NB_ContextGetEnhancedVectorMapManager(pContext);
    if (!pEnhancedVectorMapManager)
    {
        return NE_INVAL;
    }
    NB_EnhancedVectorMapState* result = (NB_EnhancedVectorMapState*)nsl_malloc(sizeof(NB_EnhancedVectorMapState));
    if (!result)
    {
        return NE_NOMEM;
    }
    nsl_memset(result, 0, sizeof(NB_EnhancedVectorMapState));

    result->context = pContext;
    result->navigation = pNavigation;
    result->updateTileCallback = configuration->updateTileCallback;
    result->exclude3DTilesInECM = configuration->exclude3DTilesInECM;
    result->ecmBoundingBoxes.routeId.size = 0;
    result->ecmBoundingBoxes.routeId.data = NULL;

    // Don't create instance directly. We have to make sure all ptr is initiallized correctly.
    new(&result->tileCallback) shared_ptr<GetTileAsyncCallback>();
    new(&result->visibleTiles) std::set<nbmap::TileKey>();
    new(&result->visibleTileKeys) std::vector<NB_TileKey>();
    new(&result->tilesToLoad) std::map<NB_TileId, nbcommon::DataStreamPtr>();
    new(&result->layerManagerListener) RefreshTileListener(result);

    new(&result->ecmBoundingBoxes.ecmBoundingBoxes) std::vector<NB_CityBoundingBox>();
    new(&result->ecmBoundingBoxes.ecmDataSetIds) std::set<std::string>();
    new(&result->ecmBoundingBoxes.lowerLatBoundings) std::multimap<double, uint32>();
    new(&result->ecmBoundingBoxes.upperLatBoundings) std::multimap<double, uint32>();
    new(&result->ecmBoundingBoxes.lowerLonBoundings) std::multimap<double, uint32>();
    new(&result->ecmBoundingBoxes.upperLonBoundings) std::multimap<double, uint32>();
    new(&result->workPath) shared_ptr<std::string>(pEnhancedVectorMapManager->workPath);
    new(&result->layerManager) shared_ptr<nbmap::LayerManager>(pEnhancedVectorMapManager->layerManager);
    if (!result->layerManager)
    {
        NB_EnhancedVectorMapStateDestroy(result);
        return NE_INVAL;
    }

    // Initiallize visible processor
    nbmap::NB_CameraProjectionParameters camerProjectionConfig = {0};
    camerProjectionConfig.cameraHeightMeters = configuration->cameraConfig.cameraHeightMeters;
    camerProjectionConfig.cameraToAvatarHorizontalMeters = configuration->cameraConfig.cameraToAvatarHorizontalMeters;
    camerProjectionConfig.avatarToHorizonHorizontalMeters = configuration->cameraConfig.avatarToHorizonHorizontalMeters;
    camerProjectionConfig.horizontalFieldOfViewDegrees = configuration->cameraConfig.horizontalFieldOfViewDegrees;
    camerProjectionConfig.avatarToMapBottomPixel = configuration->cameraConfig.avatarToMapBottomPixel;
    camerProjectionConfig.mapFieldWidthPixel = configuration->cameraConfig.mapFieldWidthPixel;
    camerProjectionConfig.mapFieldHeightPixel = configuration->cameraConfig.mapFieldHeightPixel;
    result->visibleProcessor = new nbmap::VisibleProcessor(camerProjectionConfig);
    if (!result->visibleProcessor)
    {
        NB_EnhancedVectorMapStateDestroy(result);
        return NE_NOMEM;
    }

    // Initiallize prefetch processor
    nbmap::PrefecthConfiguration prefetchConfig = {0};
    prefetchConfig.prefetchWindow[0] = configuration->prefetchConfig.prefetchWindow[0];
    prefetchConfig.prefetchWindow[1] = configuration->prefetchConfig.prefetchWindow[1];
    prefetchConfig.prefetchWindow[2] = configuration->prefetchConfig.prefetchWindow[2];
    prefetchConfig.prefetchMinWindow[0] = configuration->prefetchConfig.prefetchMinWindow[0];
    prefetchConfig.prefetchMinWindow[1] = configuration->prefetchConfig.prefetchMinWindow[1];
    prefetchConfig.prefetchMinWindow[2] = configuration->prefetchConfig.prefetchMinWindow[2];
    prefetchConfig.prefetchExtensionLengthMeters = configuration->cameraConfig.avatarToHorizonHorizontalMeters;
    prefetchConfig.prefetchExtensionWidthMeters = configuration->cameraConfig.avatarToHorizonHorizontalMeters;
    result->prefetchProcessor = new nbmap::PrefetchProcessor(prefetchConfig);
    if (!result->prefetchProcessor)
    {
        NB_EnhancedVectorMapStateDestroy(result);
        return NE_NOMEM;
    }

    // Initiallize callback object
    result->tileCallback.reset(new GetTileAsyncCallback());
    if (!result->tileCallback)
    {
        NB_EnhancedVectorMapStateDestroy(result);
        return NE_NOMEM;
    }
    result->tileCallback->SetEnhancedVectorMapState(result);
    pEnhancedVectorMapManager->layerManager->RegisterListener(&result->layerManagerListener);

    *pState = result;
    return NE_OK;
}

/*! See header file for detail */
NB_Error
NB_EnhancedVectorMapStateUpdate(NB_EnhancedVectorMapState* pState)
{
    std::vector<shared_ptr<nbmap::Layer> > ignoreLayers;
    std::vector<shared_ptr<nbmap::Layer> > ignore3DLayers;
    const std::vector<nbmap::LayerPtr>* pIgnoreLayers = NULL;
    const std::vector<nbmap::LayerPtr>* pIgnore3DLayers = NULL;

    NB_EnhancedVectorMapManager* pEnhancedVectorMapManager = NB_ContextGetEnhancedVectorMapManager(pState->context);
    if (!pEnhancedVectorMapManager)
    {
        return NE_INVAL;
    }

    // get all layers we care and don't care
    std::list<std::pair<shared_ptr<nbmap::Layer>, NB_TileLayerType> > layers;
    std::vector<shared_ptr<nbmap::Layer> >::iterator iter = pEnhancedVectorMapManager->layers.begin();
    std::vector<shared_ptr<nbmap::Layer> >::iterator iterEnd = pEnhancedVectorMapManager->layers.end();
    while (iter != iterEnd)
    {
        shared_ptr<nbmap::Layer>& layerPtr = *iter;
        shared_ptr<std::string> layerDataType = layerPtr->GetTileDataType();
        if (layerDataType->compare(s_DVA_KeyWord) == 0)
        {
            layers.push_back(std::pair<shared_ptr<nbmap::Layer>, NB_TileLayerType>(layerPtr, NB_TileLayer_Areas));
        }
        else if (layerDataType->compare(s_DVR_KeyWord) == 0)
        {
            layers.push_back(std::pair<shared_ptr<nbmap::Layer>, NB_TileLayerType>(layerPtr, NB_TileLayer_Roads));
        }
        else if (layerDataType->compare(s_LM3D_KeyWord) == 0)
        {
            layers.push_back(std::pair<shared_ptr<nbmap::Layer>, NB_TileLayerType>(layerPtr, NB_TileLayer_Landmarks));
            ignore3DLayers.push_back(layerPtr);
        }
        else if (layerDataType->compare(s_B3D_KeyWord) == 0)
        {
            layers.push_back(std::pair<shared_ptr<nbmap::Layer>, NB_TileLayerType>(layerPtr, NB_TileLayer_UntexBuilding));
            ignore3DLayers.push_back(layerPtr);
        }
        else
        {
            ignore3DLayers.push_back(layerPtr);
            ignoreLayers.push_back(layerPtr);
        }
        iter++;
    }

    // Get max reference zoom level
    uint32 maxReferenceZoomLevel = 0;
    std::list<std::pair<shared_ptr<nbmap::Layer>, NB_TileLayerType> >::iterator iterLayer = layers.begin();
    std::list<std::pair<shared_ptr<nbmap::Layer>, NB_TileLayerType> >::iterator iterLayerEnd = layers.end();
    while (iterLayer != iterLayerEnd)
    {
        shared_ptr<nbmap::Layer>& layerPtr = iterLayer->first;
        uint32 referenceZoomLevel = layerPtr->GetReferenceTileGridLevel();
        if (referenceZoomLevel > maxReferenceZoomLevel)
        {
            maxReferenceZoomLevel = referenceZoomLevel;
        }
        iterLayer++;
    }
    if (pState->exclude3DTilesInECM && !ignore3DLayers.empty())
    {
        pIgnore3DLayers = &ignore3DLayers;
    }
    if (!ignoreLayers.empty())
    {
        pIgnoreLayers = &ignoreLayers;
    }

    std::set<nbmap::TileKey> tilesNotInECM, tilesInECM;
    std::vector<nbmap::TileKeyPtr> requestTiles, ignore3DTiles;
    NB_Error err = NE_OK;

    if (pState->navigation->enhancedContentState)
    {
        UpdateEnhancedCityModelBoundingBox(pState->navigation->enhancedContentState, &pState->ecmBoundingBoxes);
    }

    // get tiles to prefetch
    pState->prefetchProcessor->Update(pState->navigation, &pState->ecmBoundingBoxes, maxReferenceZoomLevel, tilesNotInECM, tilesInECM);

    std::set<nbmap::TileKey> tileKeysToRequest;
    // convert to min zoom level for each layers
    std::set<nbmap::TileKey>::iterator iterTile = tilesNotInECM.begin();
    std::set<nbmap::TileKey>::iterator iterTileEnd = tilesNotInECM.end();
    while (iterTile != iterTileEnd)
    {
        const nbmap::TileKey& tileKey = *iterTile;
        iterLayer = layers.begin();
        iterLayerEnd = layers.end();
        while (iterLayer != iterLayerEnd)
        {
            shared_ptr<nbmap::Layer>& layerPtr = iterLayer->first;
            int32 diffZoomLevel = layerPtr->GetReferenceTileGridLevel() - layerPtr->GetMinZoom();
            if ( diffZoomLevel > 0 )
            {
                tileKeysToRequest.insert(nbmap::TileKey(tileKey.m_x >> diffZoomLevel, tileKey.m_y >> diffZoomLevel, layerPtr->GetMinZoom()));
            }
            else
            {
                tileKeysToRequest.insert(nbmap::TileKey(tileKey.m_x << -diffZoomLevel, tileKey.m_y << -diffZoomLevel, layerPtr->GetMinZoom()));
            }
            iterLayer ++;
        }
        iterTile++;
    }
    // push into request list
    iterTile = tileKeysToRequest.begin();
    iterTileEnd = tileKeysToRequest.end();
    while (iterTile != iterTileEnd)
    {
        const nbmap::TileKey& tileKey = *iterTile;
        requestTiles.push_back(nbmap::TileKeyPtr(new nbmap::TileKey(tileKey)));
        iterTile++;
    }

    tileKeysToRequest.clear();
    // convert to min zoom level for each layers
    iterTile = tilesInECM.begin();
    iterTileEnd = tilesInECM.end();
    while (iterTile != iterTileEnd)
    {
        const nbmap::TileKey& tileKey = *iterTile;
        iterLayer = layers.begin();
        iterLayerEnd = layers.end();
        while (iterLayer != iterLayerEnd)
        {
            shared_ptr<nbmap::Layer>& layerPtr = iterLayer->first;
            NB_TileLayerType layerDataType = iterLayer->second;
            if (layerDataType == NB_TileLayer_Areas
                || layerDataType == NB_TileLayer_Roads)
            {
                int32 diffZoomLevel = layerPtr->GetReferenceTileGridLevel() - layerPtr->GetMinZoom();
                if ( diffZoomLevel > 0 )
                {
                    tileKeysToRequest.insert(nbmap::TileKey(tileKey.m_x >> diffZoomLevel, tileKey.m_y >> diffZoomLevel, layerPtr->GetMinZoom()));
                }
                else
                {
                    tileKeysToRequest.insert(nbmap::TileKey(tileKey.m_x << -diffZoomLevel, tileKey.m_y << -diffZoomLevel, layerPtr->GetMinZoom()));
                }
            }
            iterLayer ++;
        }
        iterTile++;
    }
    // push into request list
    iterTile = tileKeysToRequest.begin();
    iterTileEnd = tileKeysToRequest.end();
    while (iterTile != iterTileEnd)
    {
        const nbmap::TileKey& tileKey = *iterTile;
        ignore3DTiles.push_back(nbmap::TileKeyPtr(new nbmap::TileKey(tileKey)));
        iterTile++;
    }

    // add prefetch tiles first
    // the last request tile key will be inserted in front of the request list
    // prefetch has lower priority than visible tiles, so add it first
    if (!requestTiles.empty())
    {
        pState->layerManager->GetTiles(requestTiles, pIgnoreLayers);
    }
    if (!ignore3DTiles.empty())
    {
        pState->layerManager->GetTiles(ignore3DTiles, pIgnore3DLayers);
    }

    tilesInECM.clear();
    tilesNotInECM.clear();
    pState->visibleProcessor->GetVisibleTiles(&pState->ecmBoundingBoxes, maxReferenceZoomLevel, tilesNotInECM, tilesInECM);

    // update visible tile keys
    std::set<nbmap::TileKey> newVisibleTiles;
    tileKeysToRequest.clear();
    requestTiles.clear();
    ignore3DTiles.clear();
    pState->visibleTileKeys.clear();

    iterTile = tilesNotInECM.begin();
    iterTileEnd = tilesNotInECM.end();
    while (iterTile != iterTileEnd)
    {
        const nbmap::TileKey& tileKey = *iterTile;
        NB_TileKey nbTileKey = {tileKey.m_x, tileKey.m_y, tileKey.m_zoomLevel};
        pState->visibleTileKeys.push_back(nbTileKey);
        newVisibleTiles.insert(tileKey);
        if (pState->visibleTiles.find(tileKey) == pState->visibleTiles.end())
        {
            iterLayer = layers.begin();
            iterLayerEnd = layers.end();
            while (iterLayer != iterLayerEnd)
            {
                shared_ptr<nbmap::Layer>& layerPtr = iterLayer->first;
                int32 diffZoomLevel = layerPtr->GetReferenceTileGridLevel() - layerPtr->GetMinZoom();
                if ( diffZoomLevel > 0 )
                {
                    tileKeysToRequest.insert(nbmap::TileKey(tileKey.m_x >> diffZoomLevel, tileKey.m_y >> diffZoomLevel, layerPtr->GetMinZoom()));
                }
                else
                {
                    tileKeysToRequest.insert(nbmap::TileKey(tileKey.m_x << -diffZoomLevel, tileKey.m_y << -diffZoomLevel, layerPtr->GetMinZoom()));
                }
                iterLayer ++;
            }
        }
        iterTile++;
    }
    // push into request list
    iterTile = tileKeysToRequest.begin();
    iterTileEnd = tileKeysToRequest.end();
    while (iterTile != iterTileEnd)
    {
        const nbmap::TileKey& tileKey = *iterTile;
        requestTiles.push_back(nbmap::TileKeyPtr(new nbmap::TileKey(tileKey)));
        iterTile++;
    }

    tileKeysToRequest.clear();
    iterTile = tilesInECM.begin();
    iterTileEnd = tilesInECM.end();
    while (iterTile != iterTileEnd)
    {
        const nbmap::TileKey& tileKey = *iterTile;
        NB_TileKey nbTileKey = {tileKey.m_x, tileKey.m_y, tileKey.m_zoomLevel};
        pState->visibleTileKeys.push_back(nbTileKey);
        newVisibleTiles.insert(tileKey);
        if (pState->visibleTiles.find(tileKey) == pState->visibleTiles.end())
        {
            iterLayer = layers.begin();
            iterLayerEnd = layers.end();
            while (iterLayer != iterLayerEnd)
            {
                shared_ptr<nbmap::Layer>& layerPtr = iterLayer->first;
                NB_TileLayerType layerDataType = iterLayer->second;
                if (layerDataType == NB_TileLayer_Areas
                    || layerDataType == NB_TileLayer_Roads)
                {
                    int32 diffZoomLevel = layerPtr->GetReferenceTileGridLevel() - layerPtr->GetMinZoom();
                    if ( diffZoomLevel > 0 )
                    {
                        tileKeysToRequest.insert(nbmap::TileKey(tileKey.m_x >> diffZoomLevel, tileKey.m_y >> diffZoomLevel, layerPtr->GetMinZoom()));
                    }
                    else
                    {
                        tileKeysToRequest.insert(nbmap::TileKey(tileKey.m_x << -diffZoomLevel, tileKey.m_y << -diffZoomLevel, layerPtr->GetMinZoom()));
                    }
                }
                iterLayer ++;
            }
        }
        iterTile++;
    }

    // unload tiles not visible anymore
    iterTile = pState->visibleTiles.begin();
    iterTileEnd = pState->visibleTiles.end();
    while (iterTile != iterTileEnd)
    {
        const nbmap::TileKey& tileKey = *iterTile;
        if (newVisibleTiles.find(tileKey) == newVisibleTiles.end())
        {
            if (pState->updateTileCallback.unloadTileCallback)
            {
                NB_UnloadTileCallbackData callbackData = {0};
                callbackData.tileKey.x = tileKey.m_x;
                callbackData.tileKey.y = tileKey.m_y;
                callbackData.tileKey.zoomLevel = tileKey.m_zoomLevel;
                pState->updateTileCallback.unloadTileCallback(pState,
                                                              callbackData,
                                                              pState->updateTileCallback.callbackData);
            }
        }
        iterTile ++;
    }
    // update visible tile key
    pState->visibleTiles = newVisibleTiles;

    // remove all tiles not in the visible list
    std::map<NB_TileId, nbcommon::DataStreamPtr>::iterator iterTileData = pState->tilesToLoad.begin();
    std::map<NB_TileId, nbcommon::DataStreamPtr>::iterator iterTileDataEnd = pState->tilesToLoad.end();
    while (iterTileData != iterTileDataEnd)
    {
        const NB_TileKey& nbTileKey = iterTileData->first.tileKey;
        nbmap::TileKey tileKey(nbTileKey.x, nbTileKey.y, nbTileKey.zoomLevel);
        if (newVisibleTiles.find(tileKey) == newVisibleTiles.end())
        {
            pState->tilesToLoad.erase(iterTileData++);
        }
        else
        {
            iterTileData ++;
        }
    }

    // push into request list
    iterTile = tileKeysToRequest.begin();
    iterTileEnd = tileKeysToRequest.end();
    while (iterTile != iterTileEnd)
    {
        const nbmap::TileKey& tileKey = *iterTile;
        ignore3DTiles.push_back(nbmap::TileKeyPtr(new nbmap::TileKey(tileKey)));
        iterTile++;
    }

    if (!requestTiles.empty())
    {
        err = pState->layerManager->GetTiles(requestTiles, pState->tileCallback, pIgnoreLayers);
    }
    if (!ignore3DTiles.empty())
    {
        pState->layerManager->GetTiles(ignore3DTiles, pState->tileCallback, pIgnore3DLayers);
    }

    ignoreLayers.clear();
    return err;
}

/*! See header file for detail */
void
NB_EnhancedVectorMapStateDestroy(NB_EnhancedVectorMapState* pState)
{
    NB_EnhancedVectorMapManager* pEnhancedVectorMapManager = NB_ContextGetEnhancedVectorMapManager(pState->context);
    if (pEnhancedVectorMapManager->layerManager)
    {
        pEnhancedVectorMapManager->layerManager->UnregisterListener(&pState->layerManagerListener);
    }
    if (pState->prefetchProcessor)
    {
        delete pState->prefetchProcessor;
        pState->prefetchProcessor = NULL;
    }
    if (pState->visibleProcessor)
    {
        delete pState->visibleProcessor;
        pState->visibleProcessor = NULL;
    }

    // In case that NB_EnhancedVectorMapManager has been destroyed, but request still in progress,
    // we have to tell callback objects about this.
    if (pState->tileCallback)
    {
        pState->tileCallback->SetEnhancedVectorMapState(NULL);
    }

    if (pState->ecmBoundingBoxes.routeId.data)
    {
        nsl_free(pState->ecmBoundingBoxes.routeId.data);
        pState->ecmBoundingBoxes.routeId.data = NULL;
    }
    pState->ecmBoundingBoxes.ecmBoundingBoxes.~vector();
    pState->ecmBoundingBoxes.ecmDataSetIds.~set();
    pState->ecmBoundingBoxes.lowerLatBoundings.~multimap();
    pState->ecmBoundingBoxes.upperLatBoundings.~multimap();
    pState->ecmBoundingBoxes.lowerLonBoundings.~multimap();
    pState->ecmBoundingBoxes.upperLonBoundings.~multimap();

    pState->tileCallback.~shared_ptr();
    pState->layerManager.~shared_ptr();
    pState->workPath.~shared_ptr();
    pState->visibleTiles.~set();
    pState->visibleTileKeys.~vector();
    pState->tilesToLoad.~map();
    pState->layerManagerListener.~RefreshTileListener();
    nsl_free(pState);
}

/*! See header file for detail */
NB_Error
NB_EnhancedVectorMapStateSetCameraPosition(NB_EnhancedVectorMapState* state, double latitude, double longitude, double heading)
{
    if (!state || !state->visibleProcessor)
    {
        return NE_INVAL;
    }
    state->visibleProcessor->SetBasePosition(latitude, longitude, heading);
    return NE_OK;
}

/*! See header file for detail */
void
NB_EnhancedVectorMapStateGetVisibleTileKeyList(NB_EnhancedVectorMapState* state, const NB_TileKey** tileKeys, uint32* count)
{
    if (state->visibleTileKeys.empty())
    {
        tileKeys = NULL;
        count = 0;
    }
    else
    {
        *tileKeys = &state->visibleTileKeys.front();
        *count = state->visibleTileKeys.size();
    }
}

/*! See header file for detail */
void
NB_EnhancedVectorMapStateUpdateLoadedTileIdList(NB_EnhancedVectorMapState* state, const NB_TileId* tileIds, uint32 count)
{
    // load tiles exist in tilesToLoad but not exist in loadedTileIds.
    std::set<NB_TileId> loadedTileIds;
    for (uint32 i = 0; i < count; ++i)
    {
        loadedTileIds.insert(tileIds[i]);
    }
    std::map<NB_TileId, nbcommon::DataStreamPtr>::iterator iter = state->tilesToLoad.begin();
    std::map<NB_TileId, nbcommon::DataStreamPtr>::iterator iterEnd = state->tilesToLoad.end();
    while ( iter != iterEnd )
    {
        if (loadedTileIds.find(iter->first) == loadedTileIds.end())
        {
            NB_LoadTileCallbackData callbackData = {0};
            callbackData.tileId = iter->first;
            callbackData.dataSize = iter->second->GetDataSize();
            callbackData.tileData = (uint8*)nsl_malloc(callbackData.dataSize);
            if (callbackData.tileData != NULL)
            {
                if (callbackData.dataSize == iter->second->GetData(callbackData.tileData, 0, callbackData.dataSize))
                {
                    state->updateTileCallback.loadTileCallback(state,
                                                               callbackData,
                                                               state->updateTileCallback.callbackData);
                }
                else
                {
                    nsl_free(callbackData.tileData);
                }
            }
        }
        iter++;
    }
}

/*! See header file for detail */
void
NB_EnhancedVectorMapStateGetVisibleQuadrangle(NB_EnhancedVectorMapState* state, NB_MercatorPoint visibleQuadrangle[4])
{
    if (state && state->visibleProcessor)
    {
        NB_Vector visibleQuad[4];
        state->visibleProcessor->GetVisibleQuadrangle(visibleQuad);
        for (uint32 i = 0; i < 4; ++i)
        {
            visibleQuadrangle[i].mx = visibleQuad[i].x;
            visibleQuadrangle[i].my = visibleQuad[i].y;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
/*
    GetTileAsyncCallback implementation
*/

GetTileAsyncCallback::GetTileAsyncCallback()
    : m_pEnhancedVectorMapState(NULL)
{
}

GetTileAsyncCallback::~GetTileAsyncCallback()
{
}

void
GetTileAsyncCallback::Success(nbmap::TileKeyPtr request, nbmap::TilePtr response)
{
    if (m_pEnhancedVectorMapState)
    {
        nbmap::TileKeyPtr referenceTileKeyPtr = response->GetTileKey();
        if (referenceTileKeyPtr != NULL)
        {
            // if tile exists in visible tile list.
            std::set<nbmap::TileKey>::iterator iter = m_pEnhancedVectorMapState->visibleTiles.find(*referenceTileKeyPtr);
            if (iter != m_pEnhancedVectorMapState->visibleTiles.end())
            {
                // invoke callback to show tile.
                nbcommon::DataStreamPtr tileDataPtr = response->GetData();
                if (m_pEnhancedVectorMapState->updateTileCallback.loadTileCallback && tileDataPtr)
                {
                    shared_ptr<std::string> dataType = response->GetDataType();
                    NB_LoadTileCallbackData callbackData = {0};
                    if (dataType->compare(s_DVR_KeyWord) == 0)
                    {
                        callbackData.tileId.layerType = NB_TileLayer_Roads;
                    }
                    else if (dataType->compare(s_DVA_KeyWord) == 0)
                    {
                        callbackData.tileId.layerType = NB_TileLayer_Areas;
                    }
                    else if (dataType->compare(s_B3D_KeyWord) == 0)
                    {
                        callbackData.tileId.layerType = NB_TileLayer_UntexBuilding;
                    }
                    else if (dataType->compare(s_LM3D_KeyWord) == 0)
                    {
                        callbackData.tileId.layerType = NB_TileLayer_Landmarks;
                    }
                    callbackData.tileId.tileKey.x = referenceTileKeyPtr->m_x;
                    callbackData.tileId.tileKey.y = referenceTileKeyPtr->m_y;
                    callbackData.tileId.tileKey.zoomLevel = referenceTileKeyPtr->m_zoomLevel;
                    if (m_pEnhancedVectorMapState->tilesToLoad.find(callbackData.tileId) == m_pEnhancedVectorMapState->tilesToLoad.end())
                    {
                        m_pEnhancedVectorMapState->tilesToLoad.insert(std::pair<NB_TileId, nbcommon::DataStreamPtr>(callbackData.tileId, tileDataPtr));
                        callbackData.dataSize = tileDataPtr->GetDataSize();
                        callbackData.tileData = (uint8*)nsl_malloc(callbackData.dataSize);
                        if (callbackData.tileData != NULL)
                        {
                            if (callbackData.dataSize == tileDataPtr->GetData(callbackData.tileData, 0, callbackData.dataSize))
                            {
                                m_pEnhancedVectorMapState->updateTileCallback.loadTileCallback(m_pEnhancedVectorMapState,
                                                                                               callbackData,
                                                                                               m_pEnhancedVectorMapState->updateTileCallback.callbackData);
                            }
                            else
                            {
                                nsl_free(callbackData.tileData);
                            }
                        }
                    }
                }
            }
        }
    }
}

void
GetTileAsyncCallback::Error(nbmap::TileKeyPtr request, NB_Error error)
{
    // just ignore failed request
}

bool
GetTileAsyncCallback::Progress(int percentage)
{
    return true;
}

void
UpdateEnhancedCityModelBoundingBox(NB_EnhancedContentState* enhancedContentState, NB_EnhancedCityModelBoundingBoxParameters* boundingBoxes)
{
    if (enhancedContentState->contentRegions && enhancedContentState->routeId.size && enhancedContentState->routeId.data)
    {
        // update bounding box parameters if route changes
        if (enhancedContentState->routeId.size != boundingBoxes->routeId.size || nsl_memcmp(enhancedContentState->routeId.data, boundingBoxes->routeId.data, boundingBoxes->routeId.size))
        {
            // clear old parameters
            boundingBoxes->maxLatSize = 0.;
            boundingBoxes->maxLonSize = 0.;
            boundingBoxes->lowerLatBoundings.clear();
            boundingBoxes->upperLatBoundings.clear();
            boundingBoxes->lowerLonBoundings.clear();
            boundingBoxes->upperLonBoundings.clear();
            boundingBoxes->ecmBoundingBoxes.clear();
            boundingBoxes->ecmDataSetIds.clear();

            // copy route id
            if (boundingBoxes->routeId.data)
            {
                nsl_free(boundingBoxes->routeId.data);
                boundingBoxes->routeId.data = NULL;
                boundingBoxes->routeId.size = 0;
            }
            boundingBoxes->routeId.data = (byte*)nsl_malloc(enhancedContentState->routeId.size);
            if (!boundingBoxes->routeId.data)
            {
                return;
            }
            boundingBoxes->routeId.size = enhancedContentState->routeId.size;
            nsl_memcpy(boundingBoxes->routeId.data, enhancedContentState->routeId.data, boundingBoxes->routeId.size);

            // iterate all regions
            uint32 regionCount = CSL_VectorGetLength(enhancedContentState->contentRegions);
            for (uint32 i = 0; i < regionCount; ++i)
            {
                ContentRegion* currentRegion = (ContentRegion*)CSL_VectorGetPointer(enhancedContentState->contentRegions, i);
                if (currentRegion->region.type != NB_RCRT_CityModel)
                {
                    continue;
                }
                if (boundingBoxes->ecmDataSetIds.find(currentRegion->region.datasetId) != boundingBoxes->ecmDataSetIds.end())
                {
                    continue;
                }
                boundingBoxes->ecmDataSetIds.insert(currentRegion->region.datasetId);
                NB_CityBoundingBox* cityBoundingBoxArray = NULL;
                uint32 cityBoundingBoxArraySize = 0;
                NB_Error err = NB_EnhancedContentManagerGetCityTileBoundingBoxes(
                                    enhancedContentState->enhancedContentManager,
                                    currentRegion->region.datasetId,
                                    &cityBoundingBoxArraySize,
                                    &cityBoundingBoxArray);

                if (err != NE_OK || cityBoundingBoxArraySize == 0 || cityBoundingBoxArray == NULL)
                {
                    // No city tile boxes returned
                    continue;
                }

                // update bounding parameters
                uint32 index = boundingBoxes->ecmBoundingBoxes.size();
                for (uint32 i = 0; i < cityBoundingBoxArraySize; ++i)
                {
                    NB_CityBoundingBox* boundingBox = cityBoundingBoxArray + i;
                    boundingBoxes->ecmBoundingBoxes.push_back(*boundingBox);
                    boundingBoxes->lowerLatBoundings.insert(std::pair<double, uint32>(MIN(boundingBox->point1.latitude, boundingBox->point2.latitude), index));
                    boundingBoxes->upperLatBoundings.insert(std::pair<double, uint32>(MAX(boundingBox->point1.latitude, boundingBox->point2.latitude), index));
                    boundingBoxes->lowerLonBoundings.insert(std::pair<double, uint32>(MIN(boundingBox->point1.longitude, boundingBox->point2.longitude), index));
                    boundingBoxes->upperLonBoundings.insert(std::pair<double, uint32>(MAX(boundingBox->point1.longitude, boundingBox->point2.longitude), index));
                    double latSize = nsl_fabs(boundingBox->point1.latitude - boundingBox->point2.latitude);
                    double lonSize = nsl_fabs(boundingBox->point1.latitude - boundingBox->point2.latitude);
                    if (latSize > boundingBoxes->maxLatSize)
                    {
                        boundingBoxes->maxLatSize = latSize;
                    }
                    if (lonSize > boundingBoxes->maxLonSize)
                    {
                        boundingBoxes->maxLonSize = lonSize;
                    }
                    ++index;
                }
            }
        }
    }
}

/*! See header file for detail */
void
NB_GetECMBoundingBoxes(const NB_Vector* area, uint32 pointCount, NB_EnhancedCityModelBoundingBoxParameters* ecmBoundingBoxes, uint32* boundingBoxesCount, uint32** boundingBoxIndexes)
{
    *boundingBoxesCount = 0;
    *boundingBoxIndexes = NULL;
    if (pointCount < 3)
    {
        return;
    }

    double minLatitude  = 0.0;
    double maxLatitude  = 0.0;
    double minLongitude = 0.0;
    double maxLongitude = 0.0;

    /*
        We have to first convert the are to lat/lon values. We then find the minimum and maximum lat/lon values.
    */

    // Convert first point. Don't use vector_to_lat_lon()!
    NB_SpatialConvertMercatorToLatLong(area[0].x, area[0].y, &minLatitude, &minLongitude);
    maxLatitude = minLatitude;
    maxLongitude = minLongitude;

    // Convert remaining points
    for (uint32 i = 1; i < pointCount; ++i)
    {
        double latitude  = 0.0;
        double longitude = 0.0;

        // Convert to lat/lon. Don't use vector_to_lat_lon()!
        NB_SpatialConvertMercatorToLatLong(area[i].x, area[i].y, &latitude, &longitude);

        minLatitude  = MIN(latitude, minLatitude);
        minLongitude = MIN(longitude, minLongitude);

        maxLatitude  = MAX(latitude, maxLatitude);
        maxLongitude = MAX(longitude, maxLongitude);
    }

    std::set<uint32> set1, set2;

    // cull latitude lower bound
    std::multimap<double, uint32>::const_iterator iter = ecmBoundingBoxes->lowerLatBoundings.upper_bound(minLatitude - ecmBoundingBoxes->maxLatSize);
    std::multimap<double, uint32>::const_iterator iterEnd = ecmBoundingBoxes->lowerLatBoundings.lower_bound(maxLatitude);
    while (iter != iterEnd)
    {
        set1.insert(iter->second);
        iter ++;
    }

    // cull latitude upper bound
    iter = ecmBoundingBoxes->upperLatBoundings.upper_bound(minLatitude);
    iterEnd = ecmBoundingBoxes->upperLatBoundings.lower_bound(maxLatitude + ecmBoundingBoxes->maxLatSize);
    while (iter != iterEnd)
    {
        set2.insert(iter->second);
        iter ++;
    }

    // intersect set1 and set2 to get bounding boxes in range [minLatitude, maxLatitude]
    std::vector<uint32> latSet(set1.size() + set2.size());
    std::set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(), latSet.begin());

    set1.clear();
    set2.clear();
    // cull longitude lower bound
    iter = ecmBoundingBoxes->lowerLonBoundings.upper_bound(minLongitude - ecmBoundingBoxes->maxLonSize);
    iterEnd = ecmBoundingBoxes->lowerLonBoundings.lower_bound(maxLongitude);
    while (iter != iterEnd)
    {
        set1.insert(iter->second);
        iter ++;
    }

    // cull longitude upper bound
    iter = ecmBoundingBoxes->upperLonBoundings.upper_bound(minLongitude);
    iterEnd = ecmBoundingBoxes->upperLonBoundings.lower_bound(maxLongitude + ecmBoundingBoxes->maxLonSize);
    while (iter != iterEnd)
    {
        set2.insert(iter->second);
        iter ++;
    }

    // intersect set1 and set2 to get bounding boxes in range [minLongitude, maxLongitude]
    std::vector<uint32> lonSet(set1.size() + set2.size());
    std::set_intersection(set1.begin(), set1.end(), set2.begin(), set2.end(), lonSet.begin());

    // intersect latSet and lonSet to get final result
    std::vector<uint32> result(latSet.size() + lonSet.size());
    std::set_intersection(latSet.begin(), latSet.end(), lonSet.begin(), lonSet.end(), result.begin());

    if (!set1.empty())
    {
        *boundingBoxIndexes = (uint32*)nsl_malloc(sizeof(uint32) * result.size());
        if (*boundingBoxIndexes == NULL)
        {
            return;
        }
        *boundingBoxesCount = set1.size();
        std::vector<uint32>::const_iterator iterResult = result.begin();
        std::vector<uint32>::const_iterator iterResultEnd = result.end();
        uint32 i = 0;
        while (iterResult != iterResultEnd)
        {
            (*boundingBoxIndexes)[i] = *iterResult;
            ++i;
            iterResult++;
        }
    }
}

void
RefreshTileListener::RefreshAllTiles()
{
    std::map<NB_TileId, nbcommon::DataStreamPtr>::iterator iter = m_pEnhancedVectorMapState->tilesToLoad.begin();
    std::map<NB_TileId, nbcommon::DataStreamPtr>::iterator iterEnd = m_pEnhancedVectorMapState->tilesToLoad.end();
    while (iter != iterEnd)
    {
        NB_UnloadTileCallbackData callbackData = {0};
        callbackData.tileKey = iter->first.tileKey;
        if (m_pEnhancedVectorMapState->updateTileCallback.unloadTileCallback)
        {
            m_pEnhancedVectorMapState->updateTileCallback.unloadTileCallback(m_pEnhancedVectorMapState, callbackData, m_pEnhancedVectorMapState->updateTileCallback.callbackData);
        }
        iter ++;
    }
    m_pEnhancedVectorMapState->tilesToLoad.clear();
    m_pEnhancedVectorMapState->visibleTileKeys.clear();
    m_pEnhancedVectorMapState->visibleTiles.clear();
    NB_EnhancedVectorMapStateUpdate(m_pEnhancedVectorMapState);
}

void
RefreshTileListener::RefreshTilesOfLayer(shared_ptr<nbmap::Layer> layer)
{
    RefreshAllTiles();
}
void
RefreshTileListener::RefreshTiles(const std::vector<shared_ptr<nbmap::TileKey> >& tileKeys, shared_ptr<nbmap::Layer> layer)
{
    RefreshAllTiles();
}

void
RefreshTileListener::LayersUpdated(const vector<shared_ptr<nbmap::Layer> >& layers)
{
    RefreshAllTiles();
}

void
RefreshTileListener::LayersAdded(const std::vector<shared_ptr<nbmap::Layer> >& layers)
{
}

void
RefreshTileListener::LayersRemoved(const std::vector<shared_ptr<nbmap::Layer> >& layers)
{
}

void
RefreshTileListener::AnimationLayerAdded(shared_ptr<nbmap::Layer> layer)
{
}

void
RefreshTileListener::AnimationLayerUpdated(shared_ptr<nbmap::Layer> layer, const vector<uint32>& frameList)
{
}

void
RefreshTileListener::AnimationLayerRemoved(shared_ptr<nbmap::Layer> layer)
{
}

void
RefreshTileListener::PinsRemoved(shared_ptr<std::vector<shared_ptr<nbmap::Pin> > > pins)
{
}

void
RefreshTileListener::EnableMapLegend(bool enabled, shared_ptr <nbmap::MapLegendInfo> mapLegend)
{
}

/*! @} */
