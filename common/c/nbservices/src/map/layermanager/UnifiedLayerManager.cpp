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
@file     UnifiedLayerManager.cpp
@defgroup nbmap

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
extern "C"
{
#include "palclock.h"
#include "cslnetwork.h"
}
#include "UnifiedLayerManager.h"
#include "atlasbookprotocol.h"
#include "UnifiedLayer.h"
#include "LayerProvider.h"
#include "TileManager.h"
#include "StringUtility.h"
#include "CommonMaterialLayer.h"
#include "AnimationLayer.h"
#include "LayerFunctors.h"
#include "contextbasedsingleton.h"
#include "paltaskqueue.h"
#include <iterator>
#include <cmath>
#include <functional>
#include "paldebuglog.h"
#include "UnifiedService.h"

using namespace nbmap;
using namespace protocol;
using namespace nbcommon;

/*! Layers which are should check Layer Availability Matrix (LAM) before downloading tiles. */
//static const uint32 DEFAULT_LAM_LEVEL                     = 15;

/*! Max times of retry for HTTP 404 error within a retry threshold. */
static const long HTTP_404_RETRY_TIME_THRESHOLD_SEC = 60;

/*! Threshold value for retry, constant 30 minutes. */
static const  uint32 HTTP_404_RETRY_MAX_TIMES_IN_THRESHOLD = 3;

//static const  int DUMMY_VALUE = -1;


// Assume number of tile keys passed each call of GetTiles() will not exceed 256 (2<<8).
// I think this is enough, and if it exceed 256, it is not big deal, just wrap to restart
// from 0.
#define BATCH_PRIORITY_OFFSET  24
#define REQUEST_PRIROTY_OFFSET 8
#define BATCH_PRIORITY_SHIFT   REQUEST_PRIROTY_OFFSET
#define REQUEST_PRIROTY_MASK   ((1 << (REQUEST_PRIROTY_OFFSET + 1)) - 1)
#define MAX_BATCH_PRIORITY     ((1 << (BATCH_PRIORITY_OFFSET + 1)) - 1)
#define INVALID_PRIORIRY       0xFFFFFFFF


//static const uint32 METADATA_ACTIVE_RETRY_INTERVAL_MSEC  = 1000;

// define this if want verbose output.
// #define LAYERMANAGER_VERBOSE

// Local structors.

/*! Functor to be applied to notify listener to refresh all tiles. */
struct RefreshAllTilesFunctor
{
    void operator() (LayerManagerListener* listener)
    {
        if (listener)
        {
            listener->RefreshAllTiles();
        }
    }
};

/*! Functor to be applied to notify listener to refresh some tiles of layers*/
struct RefreshTileFunctor
{
    RefreshTileFunctor(const vector<TileKeyPtr>& tileKeys, LayerPtr layer)
    : m_tileKeys(tileKeys),
      m_layer(layer)
    {
    }

    void operator() (LayerManagerListener* listener)
    {
        if (listener)
        {
            listener->RefreshTiles(m_tileKeys, m_layer);
        }
    }

    // Empty assignment operator to avoid warning C4512: "assignment operator could not be generated" (due to const member)
    RefreshTileFunctor& operator =(const RefreshTileFunctor& /*input*/) {return *this;}
    
private:
    const vector<TileKeyPtr>& m_tileKeys;
    LayerPtr m_layer;
};

/*! Functor to be applied to notify listener about the update of layers*/
struct UpdateLayersFunctor
{
    UpdateLayersFunctor(const vector<LayerPtr>& layers)
            : m_layers(layers)
    {
    }

    void operator() (LayerManagerListener* listener)
    {
        if (listener)
        {
            listener->LayersUpdated(m_layers);
        }
    }

private:
    vector<LayerPtr> m_layers;
};

/*! Functor to be applied to notify listener about the adding of layers*/
struct LayersAddedFunctor
{
    LayersAddedFunctor(const vector<LayerPtr>& layers)
            : m_layers(layers)
    {
    }

    void operator() (LayerManagerListener* listener)
    {
        if (listener)
        {
            listener->LayersAdded(m_layers);
        }
    }

private:
    vector<LayerPtr> m_layers;
};

/*! Functor to be applied to notify listener about the removal of layers*/
struct LayersRemovedFunctor
{
    LayersRemovedFunctor(const vector<LayerPtr>& layers)
            : m_layers(layers)
    {
    }

    void operator() (LayerManagerListener* listener)
    {
        if (listener)
        {
            listener->LayersRemoved(m_layers);
        }
    }

private:
    vector<LayerPtr> m_layers;
};

/*! Functor to notify listeners to toggle map legend. */
struct EnabledMapLegendFunctor
{
    EnabledMapLegendFunctor(bool enabled, shared_ptr <MapLegendInfo> mapLegend)
            : m_enabled(enabled),
              m_mapLegend(mapLegend)
    {
    }

    void operator() (LayerManagerListener* listener) const
    {
        if (listener)
        {
            listener->EnableMapLegend(m_enabled, m_mapLegend);
        }
    }

private:
    bool  m_enabled;
    shared_ptr <MapLegendInfo> m_mapLegend;
};

/*! Functor to show if a layer is overlay or not. */
struct IsOverlayFunctor
{
    bool operator() (const LayerPtr& layer) const
    {
        return layer->IsOverlay();
    }
};

class GetTilesFunctor
{
public:
    GetTilesFunctor(const vector < TileKeyPtr >& tileKeys,
                    shared_ptr <AsyncCallbackWithRequest <TileKeyPtr, TilePtr> > callback,
                    uint32 priority)
            : m_tileKeys(tileKeys),
              m_callback(callback),
              m_priority(priority)
    {
    }

    virtual ~GetTilesFunctor()
    {
    }

    void operator ()(LayerPtr layer) const
    {
        UnifiedLayer* uLayer = NULL;
        if (layer &&
            ((uLayer = static_cast<UnifiedLayer*>(layer.get())) != NULL) &&
            uLayer->IsEnabled())
        {
            uLayer->GetTiles(m_tileKeys, m_callback, m_priority);
        }
    }

    // Empty assignment operator to avoid warning C4512: "assignment operator could not be generated" (due to const member)
    GetTilesFunctor& operator =(const GetTilesFunctor& /*input*/) {return * this;}

private:
    const vector < TileKeyPtr >& m_tileKeys;
    shared_ptr <AsyncCallbackWithRequest <TileKeyPtr, TilePtr> > m_callback;
    uint32 m_priority;
};

class GetCachedTilesFunctor
{
public:
    GetCachedTilesFunctor(const vector < TileKeyPtr >& tileKeys,
                          TileRequestCallbackPtr callback)
            : m_tileKeys(tileKeys),
              m_callback(callback)
    {
    }

    virtual ~GetCachedTilesFunctor()
    {
    }

    void operator ()(LayerPtr layer) const
    {
        UnifiedLayer* uLayer = NULL;
        if (m_callback && layer && (uLayer = static_cast<UnifiedLayer*>(layer.get())) &&
            uLayer->IsEnabled())
        {
            (void)uLayer->GetCachedTiles(m_tileKeys, m_callback);
        }
    }

    // Empty assignment operator to avoid warning C4512: "assignment operator could not be generated" (due to const member)
    GetCachedTilesFunctor& operator =(const GetCachedTilesFunctor& /*input*/) {return *this;}

private:
    const vector <TileKeyPtr>& m_tileKeys;
    TileRequestCallbackPtr m_callback;
};

// Implementation of  UnifiedlayerManager.
UnifiedLayerManager::UnifiedLayerManager(LayerProviderPtr layerProvider,
                                         NB_Context* context)
        : UnifiedLayerListener(context),
          HybridModeListener(context),
          m_layerProviderPtr(layerProvider),
          m_taskParam(NULL),
          m_taskParamForMetadata(NULL)
{
    // Initialize flag of common materials available.
    m_errorHandlingInProgress = false;
    m_retriedTimes            = 0;
    m_thresholdTimestamp      = PAL_ClockGetUnixTime();
    m_currentBatchPriority    = MAX_BATCH_PRIORITY;
    m_pMetadataConfig.reset(new MetadataConfiguration());
    m_contextCallback.callback     = &(UnifiedLayerManager::MasterClear);
    m_contextCallback.callbackData = (void*)this;
    NB_ContextRegisterMasterClearCallback(m_pContext, &m_contextCallback);

    NB_NetworkNotifyEventCallback callback = {NULL, NULL};
    callback.callback = (NB_NetworkNotifyEventCallbackFunction) LayerManagerNetworkCallback;
    callback.callbackData = this;
    CSL_NetworkAddEventNotifyCallback(NB_ContextGetNetwork(context), &callback);
    m_pManagerWrapper.reset(new LayerManagerWrapperForCallback(this));
}

UnifiedLayerManager::~UnifiedLayerManager()
{
    m_pManagerWrapper->m_pManager = NULL;
    PAL_Instance* pal = NB_ContextGetPal(m_pContext);
    if (m_taskParam)
    {
        if (pal)
        {
            (void)PAL_EventTaskQueueRemove(pal, m_taskParam->m_taskId, false);
        }
        delete m_taskParam;
    }

    if (m_taskParamForMetadata)
    {
        if (pal)
        {
            (void)PAL_EventTaskQueueRemove(pal, m_taskParamForMetadata->m_taskId, false);
        }
        delete m_taskParamForMetadata;
    }

    NB_ContextUnregisterMasterClearCallback(m_pContext, &m_contextCallback );
}

LayerProviderPtr UnifiedLayerManager::GetLayerProvider()
{
    return  m_layerProviderPtr;
}

NB_Error UnifiedLayerManager::SetCachePath(shared_ptr<string> cachePath)
{
    m_layerProviderPtr->SetCachePath(cachePath);
    return NE_OK;
}

/* See description in LayerManager.h */
NB_Error
UnifiedLayerManager::SetPersistentMetadataPath(shared_ptr<string> metadataPath)
{
    return m_layerProviderPtr->SetPersistentMetadataPath(metadataPath);
}

/* See description in LayerManager.h */
void
UnifiedLayerManager::SetMetadataRetryTimes(int number)
{
    m_layerProviderPtr->SetMetadataRetryTimes(number);
}

/* See description in LayerManager.h */
NB_Error
UnifiedLayerManager::GetBackgroundRasterRange(uint32& minZoomLevel,
                                              uint32& maxZoomLevel)
{
    if (m_layerProviderPtr)
    {
        return m_layerProviderPtr->GetBackgroundRasterRange(minZoomLevel,
                                                            maxZoomLevel);
    }

    return NE_NOENT;
}

/* See description in header file. */
NB_Error
UnifiedLayerManager::GetTiles(const std::vector <TileKeyPtr>& tileKeys,
                              shared_ptr <AsyncCallback <TilePtr> > callback,
                              const std::vector <LayerPtr>& layerList,
                              const std::vector <LayerPtr>& ignoreList,
                              bool includeOverlay,
                              bool isPrefetch)

{
    NB_ASSERT_CCC_THREAD(m_pContext);

    NB_Error error = NE_OK;
    do
    {
        if (tileKeys.empty() || (!layerList.empty() && !ignoreList.empty()))
        {
            error = NE_INVAL;
            break;
        }

        if ((!m_layerProviderPtr) || (m_pContext == NULL))
        {
            error = NE_NOTINIT;
            break;
        }

        TileRequestCallbackPtr callbackWrapper(
            new TileRequestCallback(m_pManagerWrapper, callback));
        if (!callbackWrapper)
        {
            if (callback)
            {
                callback->Error(NE_NOMEM);
            }
            error = NE_NOMEM;
            break;
        }

        // If no layers are provided explicitly, download tiles from all layers and return.
        if (layerList.empty())
        {
            error = GetTilesInternal(tileKeys, callbackWrapper, ignoreList, includeOverlay, isPrefetch);
            break;
        }

        // Else, remove overlay layers from layerList if necessary, and download tiles.
        vector<LayerPtr> tmpLayers(layerList);
        vector<LayerPtr>::const_iterator layerIter    = tmpLayers.begin();
        vector<LayerPtr>::const_iterator layerIterEnd = tmpLayers.end();
        if (!includeOverlay)
        {
            layerIterEnd = remove_if(tmpLayers.begin(), tmpLayers.end(), IsOverlayFunctor());
        }
        if (layerIter ==layerIterEnd)  // All layers are ignored.
        {
            error = NE_NOENT;
            break;
        }

        GetTilesFunctor functor(tileKeys, callbackWrapper, GetBatchPriority());
        for_each (layerIter, layerIterEnd, functor);
    } while (0);

    if (error && callback)
    {
        callback->Error(error);
    }

    return error;
}

/* See description in header file. */
NB_Error UnifiedLayerManager::GetTiles(const vector<TileKeyPtr>& tileKeys,
                                       vector<LayerPtr> const * ignoreList,
                                       bool includeOverlay,
                                       bool isPrefetch)
{
    vector<LayerPtr> emptyLayers;
    return GetTiles(tileKeys,shared_ptr<AsyncCallback<TilePtr> >(), emptyLayers,
                    ignoreList ? *ignoreList : emptyLayers, includeOverlay, isPrefetch);
}


/* See description in header file. */
NB_Error
UnifiedLayerManager::GetTiles(const vector<TileKeyPtr>& tileKeys,
                              shared_ptr<AsyncCallbackWithRequest<TileKeyPtr, TilePtr> > callback,
                              vector<LayerPtr> const * ignoreList,
                              bool includeOverlay,
                              bool /*isPrefetch*/)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    if (tileKeys.empty())
    {
        return NE_INVAL;
    }

    if ((!m_layerProviderPtr) || (m_pContext == NULL))
    {
        return (NE_NOTINIT);
    }

    TileRequestCallbackPtr callbackWrapper(
        new TileRequestCallback(m_pManagerWrapper, callback));
    if (!callbackWrapper)
    {
        //TODO: Do we need to notify client about this error with every tile Key?
        if (callback)
        {
            callback->Error(TileKeyPtr(), NE_NOMEM);
        }
        return NE_NOMEM;
    }

    vector<LayerPtr> ignoredLayers;
    if (ignoreList)
    {
        ignoredLayers = *ignoreList;
    }

    return GetTilesInternal(tileKeys, callbackWrapper, ignoredLayers, includeOverlay);
}

/* See description in LayerManager.h */
NB_Error
UnifiedLayerManager::GetAnimationTiles(LayerPtr layer,
                                       const vector<TileKeyPtr>& tileKeys,
                                       const vector<uint32>& timestamps,
                                       shared_ptr<AsyncCallback<TilePtr> > callback)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    if (tileKeys.empty() || !layer)
    {
        return NE_INVAL;
    }

    if ((!m_pContext) || (!m_layerProviderPtr))
    {
        return NE_NOTINIT;
    }

    AnimationLayer* animationLayer = static_cast<AnimationLayer*>(layer.get());
    if (!animationLayer)
    {
        return NE_UNEXPECTED;
    }

    // Use the lower priority for the animation tiles
    animationLayer->GetAnimationTiles(tileKeys, timestamps, callback, (m_currentBatchPriority + 2) << REQUEST_PRIROTY_OFFSET);
    return NE_OK;
}

/* See description in header file. */
NB_Error UnifiedLayerManager::GetCachedTiles(const vector <TileKeyPtr>& tileKeys,
                                             shared_ptr<AsyncCallback<TilePtr> > callback)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    if (!callback)
    {
        return NE_INVAL;
    }

    TileRequestCallbackPtr callbackWrapper(
        new TileRequestCallback(m_pManagerWrapper, callback));
    if (!callbackWrapper)
    {
        if (callback)
        {
            callback->Error(NE_NOMEM);
        }
        return NE_NOMEM;
    }

    vector<LayerPtr> layers = m_layerProviderPtr->GetLayerPtrVector();
    vector<TilePtr> tiles;
    GetCachedTilesFunctor functor(tileKeys, callbackWrapper);
    for_each (layers.begin(), layers.end(), functor);
    return NE_OK;
}

NB_Error
UnifiedLayerManager::GetCommonMaterials(shared_ptr<AsyncCallbackWithRequest<TileKeyPtr, TilePtr> > callback,
                                        shared_ptr<string> type)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    NB_Error error = NE_NOTINIT;
    const vector<LayerPtr>& layerArray = m_layerProviderPtr->GetCommonMaterialLayers();
    if (!layerArray.empty())
    {
        vector<LayerPtr>::const_iterator iter = layerArray.begin();
        vector<LayerPtr>::const_iterator end  = layerArray.end();
        for (; iter < end; ++iter)
        {
            nsl_assert(iter->get() != NULL);

            CommonMaterialLayer* cLayer = static_cast<CommonMaterialLayer*>((*iter).get());

            TileLayerInfoPtr info = (cLayer == NULL) ?                      \
                                    TileLayerInfoPtr() : cLayer->GetTileLayerInfo();
            shared_ptr<string> materialType = info ?                    \
                                              info->materialType : shared_ptr<string>();
            if (!type || type->empty() || !materialType ||
                (nbcommon::StringUtility::IsStringEqual(materialType, type)))
            {
                cLayer->GetMaterial(callback, HIGHEST_TILE_REQUEST_PRIORITY);
                error = NE_OK;
            }
        }
    }

    if (error != NE_OK && callback)
    {
        callback->Error(TileKeyPtr(), error);
    }

    return error;
}

void UnifiedLayerManager::RemoveAllTiles()
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    if ((!m_layerProviderPtr) || (m_pContext == NULL))
    {
        return;
    }

    const vector<LayerPtr>& layerArray = m_layerProviderPtr->GetLayerPtrVector();
    for (unsigned int i = 0; i < layerArray.size(); i++)
    {
        LayerPtr layer = layerArray[i];
        layer->RemoveAllTiles();
    }
}

/* See description in LayerManager.h */
void UnifiedLayerManager::RemoveBackgroundRasterTiles()
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    if ((!m_layerProviderPtr) || (m_pContext == NULL))
    {
        return;
    }

    const vector<LayerPtr>& layerArray = m_layerProviderPtr->GetLayerPtrVector();
    for (size_t i = 0; i < layerArray.size(); ++i)
    {
        LayerPtr layer = layerArray[i];
        UnifiedLayerPtr unifiedLayer = dynamic_pointer_cast<UnifiedLayer>(layer);
        if(unifiedLayer->IsBackgroundRasterLayer())
        {
            layer->RemoveAllTiles();
        }
    }
}

/* See description in LayerManager.h */
vector<LayerPtr> UnifiedLayerManager::GetBackgroundRasterLayers()
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    vector<LayerPtr> layers = vector<LayerPtr>();

    if ((!m_layerProviderPtr) || (m_pContext == NULL))
    {
        return layers;
    }

    const vector<LayerPtr>& layerArray = m_layerProviderPtr->GetLayerPtrVector();
    for (size_t i = 0; i < layerArray.size(); ++i)
    {
        LayerPtr layer = layerArray[i];
        UnifiedLayerPtr unifiedLayer = dynamic_pointer_cast<UnifiedLayer>(layer);
        if(unifiedLayer->IsBackgroundRasterLayer())
        {
            layers.push_back(layer);
        }
    }

    return layers;
}

/* See description in LayerManager.h */
NB_Error
UnifiedLayerManager::RegisterListener(LayerManagerListener* listener)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    if (!listener)
    {
        return NE_INVAL;
    }

    // Find if this listener is already existing.
    vector<LayerManagerListener*>::iterator end = m_pListeners.end();
    vector<LayerManagerListener*>::iterator iterator = find(m_pListeners.begin(), end, listener);
    if (iterator != end)
    {
        return NE_EXIST;
    }

    // Add this listener.
    m_pListeners.push_back(listener);
    return NE_OK;
}

/* See description in LayerManager.h */
void
UnifiedLayerManager::UnregisterListener(LayerManagerListener* listener)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    // Find this listener.
    vector<LayerManagerListener*>::iterator end = m_pListeners.end();
    vector<LayerManagerListener*>::iterator iterator = find(m_pListeners.begin(), end, listener);
    if (iterator != end)
    {
        // Erase this listener.
        m_pListeners.erase(iterator);
    }
}

/* See description in LayerManager.h */
void
UnifiedLayerManager::GetDAMTiles(vector<TileKeyPtr>& tileKeys, shared_ptr<AsyncCallbackWithRequest<TileKeyPtr, TilePtr> > callback)
{

    NB_ASSERT_CCC_THREAD(m_pContext);

    if ((!m_layerProviderPtr) || (m_pContext == NULL))
    {
        return;
    }

    LayerPtr damLayer = m_layerProviderPtr->GetDAMLayer();
    if (damLayer)
    {
        // When tilekeys is empty, download all tiles of DAM, and do not need to callback to user.
        if (tileKeys.empty())
        {
            int damReferenceGridLevel = damLayer->GetReferenceTileGridLevel();
            int tileCount = static_cast<int>(pow(2.0, damReferenceGridLevel));
            for (int x = 0; x < tileCount; ++x)
            {
                for (int y = 0; y < tileCount; ++y)
                {
                    TileKeyPtr tileKey(new TileKey(x, y, damReferenceGridLevel));
                    tileKeys.push_back(tileKey);
                }
            }
        }
        UnifiedLayer* uLayer = NULL;
        if (damLayer && ((uLayer = static_cast<UnifiedLayer*>(damLayer.get())) != NULL))
        {
            uLayer->GetTiles(tileKeys, callback, GetBatchPriority());
        }
    }
}

/* See description in LayerManager.h */
void UnifiedLayerManager::UnifiedLayerUpdated()
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    // @TRICKY: the passed in `newLayers` are just layers created by
    // UnifiedLayerProvider, do not return these layers to MapView directly, because
    // layers such as route/DVRT/traffic and Pins are stored in other LayerProviders,
    // so schedule a new task to retrieve all the layers later. Also, this deferred
    // task can avoid circular callbacks.
    if (!m_pGetLayersCallback)
    {
        PAL_Instance* pal = NB_ContextGetPal(m_pContext);
        if (pal && !m_taskParam)
        {
            m_taskParam = new LayerManagerTaskParameter(this);
            PAL_Error error = PAL_EventTaskQueueAdd(
                pal, &UnifiedLayerManager::RetrivedLayersAndNotifyListener,
                m_taskParam, &m_taskParam->m_taskId);

            if (error != PAL_Ok)
            {
                delete m_taskParam;
                m_taskParam = NULL;
            }
        }
    }
}

/* See description in LayerManager.h */
void
UnifiedLayerManager::RefreshAllTiles()
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    RefreshAllTilesFunctor refresher;
    for_each(m_pListeners.begin(), m_pListeners.end(), refresher);
}

/* See description in LayerManager.h */
void
UnifiedLayerManager::RefreshTilesOfLayer(LayerPtr layer)
{
    NB_ASSERT_CCC_THREAD(m_pContext);
    for_each(m_pListeners.begin(), m_pListeners.end(),
             bind2nd(mem_fun(&LayerManagerListener::RefreshTilesOfLayer), layer));
}

/* See description in LayerManager.h */
void
UnifiedLayerManager::ReloadTilesOfLayer(LayerPtr layer)
{
    NB_ASSERT_CCC_THREAD(m_pContext);
    for_each(m_pListeners.begin(), m_pListeners.end(),
             bind2nd(mem_fun(&LayerManagerListener::ReloadTilesOfLayer), layer));
}

/* See description in LayerManager.h */
void
UnifiedLayerManager::RefreshTiles(const vector<TileKeyPtr>& tileKeys,
                                  LayerPtr layer)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    RefreshTileFunctor refresher(tileKeys, layer);
    for_each(m_pListeners.begin(), m_pListeners.end(), refresher);
}

void UnifiedLayerManager::UpdateLayers(const vector<LayerPtr>& layers)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    m_errorHandlingInProgress = false;
    // Notify listener that layers are updated. Listener should drop old layers and use new
    // ones.
    vector<LayerPtr> sortedArray = layers;
    sort(sortedArray.begin(), sortedArray.end());
    UpdateLayersFunctor functor(sortedArray);
    for_each(m_pListeners.begin(), m_pListeners.end(), functor);
}

void
UnifiedLayerManager::NotifyAnimationLayerAdded(shared_ptr<Layer> layer)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    if (layer)
    {
        for_each (m_pListeners.begin(), m_pListeners.end(),
                  bind2nd(mem_fun(&LayerManagerListener::AnimationLayerAdded), layer));
    }
}

void
UnifiedLayerManager::NotifyAnimationLayerUpdated(shared_ptr<Layer> layer,
                                                 const vector<uint32 >& frameList)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    //@todo: Refactor later to call using functors.
    for (size_t i=0; i < m_pListeners.size(); i++)
    {
        m_pListeners[i]->AnimationLayerUpdated(layer,frameList);
    }
}

void
UnifiedLayerManager::NotifyAnimationLayerRemoved(shared_ptr<Layer> layer)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    if (layer)
    {
        for_each (m_pListeners.begin(), m_pListeners.end(),
                  bind2nd(mem_fun(&LayerManagerListener::AnimationLayerRemoved), layer));
    }
}

void UnifiedLayerManager::NotifyLayersAdded(const vector <LayerPtr>& layers)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    m_errorHandlingInProgress = false;
    if (!layers.empty())
    {
        vector<LayerPtr> sortedArray = layers;
        sort(sortedArray.begin(), sortedArray.end());
        LayersAddedFunctor functor(sortedArray);
        for_each(m_pListeners.begin(), m_pListeners.end(), functor);
    }
}

void UnifiedLayerManager::NotifyLayersRemoved(const vector <LayerPtr>& layers)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    m_errorHandlingInProgress = false;
    if (!layers.empty())
    {
        vector<LayerPtr> sortedArray = layers;
        sort(sortedArray.begin(), sortedArray.end());

        LayersRemovedFunctor functor(sortedArray);
        for_each(m_pListeners.begin(), m_pListeners.end(), functor);
    }
}

void UnifiedLayerManager::NotifyPinsRemoved(shared_ptr<vector<PinPtr> > pins)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    if (pins && (!(pins->empty())))
    {
        for_each(m_pListeners.begin(), m_pListeners.end(),
                 bind2nd(mem_fun(&LayerManagerListener::PinsRemoved), pins));
    }
}


/* See description in header file. */
void
UnifiedLayerManager::NotifyShowMapLegend(bool enabled, shared_ptr <MapLegendInfo> mapLegend)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    EnabledMapLegendFunctor functor(enabled, mapLegend);
    for_each(m_pListeners.begin(), m_pListeners.end(), functor);
}

NB_Error
UnifiedLayerManager::GetLayers(shared_ptr<AsyncCallback<const vector<LayerPtr>& > > callback,
                               shared_ptr<MetadataConfiguration> metadataConfig)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    if ((!m_layerProviderPtr) || (m_pContext == NULL))
    {
        return (NE_NOTINIT);
    }

    m_pMetadataConfig = metadataConfig;
    m_pGetLayersCallback = callback;

    GenericLayerCallbackPtr layerCallback = GenericLayerCallbackPtr(
        new GenericLayerCallback(m_pManagerWrapper, m_pContext, callback));
    m_layerProviderPtr->GetLayers(layerCallback, m_pMetadataConfig);

    return NE_OK;
}

void
UnifiedLayerManager::LayerRequestSuccess(const vector<LayerPtr>& layers)
{
    if (m_pGetLayersCallback)
    {
        m_pGetLayersCallback->Success(layers);
    }
}

void
UnifiedLayerManager::LayerRequestError(NB_Error error)
{
    shared_ptr<AsyncCallback<const vector<LayerPtr>& > > callback;
    m_pGetLayersCallback.swap(callback);
    if(callback)
    {
        callback->Error(error);
    }
}

/* See description in LayerManager.h */
void
UnifiedLayerManager::ClearLayers()
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    // Clear information about layers in layer provider.
    if (m_layerProviderPtr)
    {
        m_layerProviderPtr->ClearLayers();
    }

    /* Reset available flag of common materials and current priority of tile requests.
       Because all layers are reset and user should call function GetLayers to set
       these values. */
    m_currentBatchPriority = MAX_BATCH_PRIORITY;
}

void UnifiedLayerManager::MasterClear(void* pObject)
{
    UnifiedLayerManager * pThis = (UnifiedLayerManager *)pObject;
    if (pThis == NULL)
    {
        return;
    }
    pThis->ClearLayers();
}

/* See description in header file. */
void UnifiedLayerManager::MetadataUpdated(bool changed,  NB_Error /*error*/)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    if (changed)
    {
        m_retriedTimes       = 0;
        m_thresholdTimestamp = PAL_ClockGetUnixTime();
        // clear all layers and requests..
        ClearLayers();
        // Call GetLayers() with RetryGetLayerCallback.
        RetryGetLayerCallbackPtr callback =
                RetryGetLayerCallbackPtr(new RetryGetLayerCallback(m_pManagerWrapper));
        GetLayers(callback, m_pMetadataConfig);
    }
    else
    {
        // Do nothing.
    }
    m_errorHandlingInProgress  = false;
}


void UnifiedLayerManager::TileRequestError(NB_Error error)
{
    if (error == NE_HTTP_RESOURCE_NOT_FOUND || error == NE_HTTP_BAD_REQUEST)
    {
        if (m_errorHandlingInProgress)
        {
            // Just return if we are handling this.
            return;
        }

        nb_unixTime now = PAL_ClockGetUnixTime();

        if (now - m_thresholdTimestamp > HTTP_404_RETRY_TIME_THRESHOLD_SEC)
        {
            // Do nothing but update statistics.
            m_retriedTimes       = 0;
            m_thresholdTimestamp = now;
        }
        else if (++m_retriedTimes == HTTP_404_RETRY_MAX_TIMES_IN_THRESHOLD)
        {
            m_retriedTimes       = 0;
            m_thresholdTimestamp = now;
            CheckMetadataChanges(false);
            return;
        }
    }
}

/* See description in header file. */
NB_Error
UnifiedLayerManager::GetTilesInternal(const vector <TileKeyPtr>& tileKeys,
                                      shared_ptr <TileRequestCallback> callback,
                                      const vector <LayerPtr>& ignoreList,
                                      bool includeOverlay,
                                      bool isPrefetch)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    // Skip checking of tileKeys and internal status because we have checked them before
    // goes here.

    vector<LayerPtr> layers = m_layerProviderPtr->GetLayerPtrVector();

    // Following checks should only happen when ignore list is provided.
    if (!ignoreList.empty())
    {
        vector<LayerPtr> tmpIgnoredList(ignoreList);
        vector<LayerPtr> tmpLayerArray;
        tmpLayerArray.swap(layers);

        sort(tmpLayerArray.begin(), tmpLayerArray.end());
        sort(tmpIgnoredList.begin(), tmpIgnoredList.end());
        set_difference(tmpLayerArray.begin(), tmpLayerArray.end(),
                       tmpIgnoredList.begin(), tmpIgnoredList.end(),
                       back_inserter(layers));

        //@note: Enabled following code to check if ignoreList is working or not.
#if 0
        vector<pair<LayerPtr, LayerPtr> > dupLayers;

        for (size_t i= 0; i < ignoreList.size(); ++i)
        {
            LayerPtr layer = ignoreList[i];
            LayerFindByDataTypeFunctor functor(layer->GetTileDataType());
            vector<LayerPtr>::iterator itt =
                    find_if(layers.begin(), layers.end(), functor);
            if (itt != layers.end())
            {
                //@todo: Only enabled layers should be appeared here, this should be
                // done when we can distinguish optional layers in metadata.
                if (layer->IsEnabled())
                {
                    dupLayers.push_back(make_pair(layer, *itt));
                }
            }
        }
        if (!dupLayers.empty())
        {
            printf("Ignored layers found in available layers!\n"
                   "\t\tType:\t\t\t\t\tWantToIgnore\t\tStillAvailable\n");
            vector<pair<LayerPtr, LayerPtr> >::iterator iter = dupLayers.begin();
            vector<pair<LayerPtr, LayerPtr> >::iterator end  = dupLayers.end();
            while (iter != end)
            {
                printf("\t\t%s\t\t\t\t\t\t\%p\t\t%p\n",
                       iter->first->GetTileDataType()->c_str(), iter->first.get(),
                       iter->second.get());
                ++iter;
            }
            printf("\n");
        }
#endif
    }

    vector<LayerPtr>::const_iterator layerIter    = layers.begin();
    vector<LayerPtr>::const_iterator layerIterEnd = layers.end();

    if (!includeOverlay)
    {
        layerIterEnd = remove_if(layers.begin(), layers.end(), IsOverlayFunctor());
    }

    if (layerIter !=layerIterEnd)  // All layers are ignored.
    {
        //We should make the prefetch download priority low during the common 'GetTiles' conflict with it.
        uint32 batchPriority = isPrefetch ? MAX_BATCH_PRIORITY : GetBatchPriority();
        DEBUGLOG(LOG_SS_NB_MAP, LOG_SEV_INFO, ("^^^^^isisPrefetch[%s],batchPriority[%d]^^^^^^^", (isPrefetch?"YES":"NO"), batchPriority));
        GetTilesFunctor functor(tileKeys, callback, batchPriority);
        for_each (layerIter, layerIterEnd, functor);
    }

    return NE_OK;
}

/* See description in header file. */
uint32 UnifiedLayerManager::GetBatchPriority()
{
    if (--m_currentBatchPriority == INVALID_PRIORIRY)
    {
        m_currentBatchPriority = MAX_BATCH_PRIORITY;
    }

    return (m_currentBatchPriority << REQUEST_PRIROTY_OFFSET);
}

/* See description in header file. */
void UnifiedLayerManager::CheckMetadataChanges(bool forceUpdate)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    // Check whether metadata is changed, remained task will be scheduled by
    // callback.
    shared_ptr<AsyncCallback <bool > > callback(
        new CheckMetadataChangesCallback<UnifiedLayerManager> (
            this, &UnifiedLayerManager::MetadataUpdated));
    if(m_layerProviderPtr)
    {
        m_layerProviderPtr->CheckMetadataChanges(callback, shared_ptr<MetadataConfiguration>(), true, forceUpdate);
        m_errorHandlingInProgress = true;
    }
}

/* See description in header file. */
void UnifiedLayerManager::RetrivedLayersAndNotifyListener(PAL_Instance* /*pal*/, void* userData)
{
    LayerManagerTaskParameter* param   = static_cast<LayerManagerTaskParameter*>(userData);
    UnifiedLayerManager*       manager = NULL;
    if (param)
    {
        manager = param ? param->m_manager : NULL;
        delete param;
    }

    if (manager)
    {
        vector<LayerPtr> layers;
        if (manager->m_layerProviderPtr)
        {
            layers = manager->m_layerProviderPtr->GetLayerPtrVector();
        }
        manager->m_taskParam = NULL;
        manager->UpdateLayers(layers);
    }
}

void UnifiedLayerManager::SetPreferredLanguageCode(uint8 languageCode)
{
    if (m_layerProviderPtr)
    {
        m_layerProviderPtr->SetPreferredLanguageCode(languageCode);
    }
}

void UnifiedLayerManager::SetTheme(MapViewTheme theme)
{
    if (m_layerProviderPtr)
    {
        m_layerProviderPtr->SetTheme(theme);
    }
}

/* See description in header file. */
void UnifiedLayerManager::NotifyMaterialAdded(const ExternalMaterialPtr& material)
{
    if (material)
    {
        for_each(m_pListeners.begin(), m_pListeners.end(),
                 bind2nd(mem_fun(&LayerManagerListener::MaterialAdded), material));
    }
}

/* See description in header file. */
void UnifiedLayerManager::NotifyMaterialRemoved(const ExternalMaterialPtr& material)
{
    if (material)
    {
        for_each(m_pListeners.begin(), m_pListeners.end(),
                 bind2nd(mem_fun(&LayerManagerListener::MaterialRemoved), material));
    }
}

/* See description in header file. */
void UnifiedLayerManager::NotifyMaterialActived(const ExternalMaterialPtr& material)
{
    if (material)
    {
        for_each(m_pListeners.begin(), m_pListeners.end(),
                 bind2nd(mem_fun(&LayerManagerListener::MaterialActived), material));
    }
}

void UnifiedLayerManager::HybridModeChanged(HybridMode /*mode*/)
{
    //metadata is definitely updated here
    //update layer list immeditately
    MetadataUpdated(true);
}

/* See description in header file. */
void UnifiedLayerManager::CheckMetadataCallback(PAL_Instance* /*pal*/, void* userData)
{
    LayerManagerTaskParameter* param   = static_cast<LayerManagerTaskParameter*>(userData);
    UnifiedLayerManager* manager = param ? param->m_manager : NULL;
    
    if (manager)
    {
        manager->CheckMetadataChanges(param->m_forceUpdateMetadata);
        manager->m_taskParamForMetadata = NULL;
    }
    
    delete param;
}

/* See description in header file. */
void UnifiedLayerManager::UpdateMetadata(bool forceUpdate)
{
    PAL_Instance* pal = NB_ContextGetPal(m_pContext);
    if (pal && !m_taskParamForMetadata)
    {
        m_taskParamForMetadata = new LayerManagerTaskParameter(this);
        m_taskParamForMetadata->m_forceUpdateMetadata = forceUpdate;
        PAL_Error error = PAL_EventTaskQueueAdd(
            pal, &UnifiedLayerManager::CheckMetadataCallback,
            m_taskParamForMetadata, &m_taskParamForMetadata->m_taskId);

        if (error != PAL_Ok)
        {
            delete m_taskParamForMetadata;
            m_taskParamForMetadata = NULL;
        }
    }
}

/* See description in header file. */
void UnifiedLayerManager::LayerManagerNetworkCallback(NB_NetworkNotifyEvent event,
                                                      void* /*data*/,
                                                      UnifiedLayerManager* layerManager)
{
    if ((event == NB_NetworkNotifyEvent_ConnectionReset) && layerManager)
    {
         layerManager->UpdateMetadata(true);
    }
}


/*! @} */
