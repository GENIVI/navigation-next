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
  @file        RouteManagerImpl.cpp
  @defgroup    nbmap
  Description: Class Impl for RouteManagerImpl, which manage route layers.
*/
/*
  (C) Copyright 2012 by TeleCommunications Systems, Inc.

  The information contained herein is confidential, proprietary to
  TeleCommunication Systems, Inc., and considered a trade secret as defined
  in section 499C of the penal code of the State of California. Use of this
  information by anyone other than authorized employees of TeleCommunication
  Systems is granted only under a written non-disclosure agreement, expressly
  prescribing the scope and manner of such use.

  --------------------------------------------------------------------------*/
/*! @{ */

#include "RouteManagerImpl.h"
#include "LayerProvider.h"
#include "LayerFunctors.h"
#include "UnifiedLayerManager.h"
#include "RouteLayer.h"
#include "StringUtility.h"

using namespace std;
using namespace nbmap;

#define MAX_ROUTE_LAYER_ID       (LAYER_DIGITAL_ID_ROUTE_CAP + LAYER_DIGITAL_ID_ROUTE_BASE)
static const char NB_DTS_COLOR_FORMAT[] = "%02X%02X%02X%02X";

RouteManagerImpl::RouteManagerImpl(NB_Context*           context,
                                   LayerManagerPtr       layerManager,
                                   LocalLayerProviderPtr layerProvider,
                                   LayerIdGeneratorPtr   idGenerator)
        : UnifiedLayerListener(context),
          m_pLayerManager(layerManager),
          m_pLayerProvider(layerProvider),
          m_pLayerIdGenerator(idGenerator)
{
    m_pUnifiedLayerManager      = static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
    shared_ptr<AggregateLayerProvider> aggregateLayerProvider =
            ContextBasedSingleton<AggregateLayerProvider>::getInstance(context);
    aggregateLayerProvider->RegisterUnifiedLayerListener(this, false);
}

RouteManagerImpl::~RouteManagerImpl()
{
    Reset();
}

shared_ptr<string> RouteManagerImpl::AddRouteLayer(const vector<RouteInfo*>& routeInfo)
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    vector<EncodedRouteInfo> encodedRouteInfo;
    shared_ptr<string> layersId = CreateIdFromRouteInfo(routeInfo, encodedRouteInfo);
    NB_Error error = NE_OK;

    do
    {
        if (!layersId || layersId->empty() || encodedRouteInfo.empty())
        {
            error = NE_UNEXPECTED;
            break;
        }

        map<string, vector<LayerPtr> >::iterator iter =
                m_routeLayerInstances.find(*layersId);

        if (iter != m_routeLayerInstances.end() &&
            !iter->second.empty())
        {
            error = NE_EXIST;
            break;
        }

        vector<LayerPtr> routeLayers = CreateRouteLayers(encodedRouteInfo);
        if (routeLayers.empty())
        {
            error = NE_NOMEM;
            break;
        }

        m_routeLayerInstances.insert(make_pair(*layersId, routeLayers));
        m_cachedRouteInfo.insert(make_pair(*layersId, encodedRouteInfo));

        vector<LayerPtr> addedLayers;
        for (size_t i = 0; i < routeLayers.size(); ++i)
        {
            const LayerPtr& layer = routeLayers[i];
            if (layer && m_pLayerProvider->AddLayer(layer))
            {
                addedLayers.push_back(layer);
            }
        }

        if (m_pUnifiedLayerManager && !addedLayers.empty())
        {
            m_pUnifiedLayerManager->NotifyLayersAdded(addedLayers);
        }

    } while (0);

    if (error != NE_OK)
    {
        layersId.reset();
    }

    return layersId;
}

NB_Error RouteManagerImpl::RemoveRouteLayer(shared_ptr<string> routeLayerId)
{
    NB_Error error = NE_INVAL;
    map<string, vector<LayerPtr> >::iterator it = m_routeLayerInstances.find(*routeLayerId);
    if (it != m_routeLayerInstances.end())
    {
        vector<LayerPtr>& layers = it->second;
        vector<LayerPtr> removedLayers;

        for (size_t i = 0; i < layers.size(); ++i)
        {
            const LayerPtr& layer = layers[i];
            RouteLayer* routeLayer = static_cast<RouteLayer*>(layer.get());
            if (routeLayer)
            {
                routeLayer->SetEnabled(false);
                routeLayer->UnregisterListener(this);
            }
            if (m_pLayerProvider->RemoveLayer(layer))
            {
                removedLayers.push_back(layer);
            }
        }

        if (m_pUnifiedLayerManager && !removedLayers.empty())
        {
            m_pUnifiedLayerManager->NotifyLayersRemoved(removedLayers);
        }

        layers.clear();
        m_routeLayerInstances.erase(it);
        m_cachedRouteInfo.erase(*routeLayerId);
        error = NE_OK;
    }
    return error;
}

void RouteManagerImpl::RemoveAllRouteLayers()
{
    Reset();
}

void RouteManagerImpl::Reset()
{
    RemoveAllRouteLayersInternal();

    m_cachedRouteInfo.clear();
}


shared_ptr <string>
RouteManagerImpl::CreateIdFromRouteInfo(const vector<RouteInfo*>& routeInfo,
                                        vector<EncodedRouteInfo>& encodedRouteInfo)
{
    shared_ptr<string> id;
    if (!routeInfo.empty())
    {
        id = shared_ptr<string> (new string);
        for (size_t i = 0; i < routeInfo.size(); ++i)
        {
            EncodedRouteInfo info;
            NB_RouteId* routeId = routeInfo[i]->routeId;
            if (routeId && routeId->data && routeId->size > 0)
            {
                char* encodedRouteId = NULL;
                if (encode_base64((const char*)routeId->data, routeId->size,
                                  &encodedRouteId) == NE_OK)
                {
                    info.encodedRouteId = encodedRouteId;
                    id->append(info.encodedRouteId);
                    nsl_free(encodedRouteId);
                }
            }

            nb_color color = routeInfo[i]->routeColor;
            char temp[64] = {0};
            nsl_sprintf(temp,
                        NB_DTS_COLOR_FORMAT,
                        NB_COLOR_GET_R(color),
                        NB_COLOR_GET_G(color),
                        NB_COLOR_GET_B(color),
                        NB_COLOR_GET_A(color));
            info.encodedRouteColor = temp;

            encodedRouteInfo.push_back(info);
        }
    }

    return id;
}

void RouteManagerImpl::UnifiedLayerUpdated()
{
    if (m_cachedRouteInfo.empty())
    {
        return ;
    }

    m_routeLayerFromDTS.reset();
    RemoveAllRouteLayersInternal();

    if (!GetRouteLayers())
    {
        return;
    }

    map<string, vector<EncodedRouteInfo> > :: iterator iter = m_cachedRouteInfo.begin();
    map<string, vector<EncodedRouteInfo> > :: iterator end  = m_cachedRouteInfo.end();
    vector<LayerPtr> addedLayers;

    while (iter != end)
    {
        vector<LayerPtr> routeLayers = CreateRouteLayers(iter->second);
        for (size_t i = 0; i < routeLayers.size(); ++i)
        {
            const LayerPtr& layer = routeLayers[i];
            if (layer && m_pLayerProvider->AddLayer(layer))
            {
                addedLayers.push_back(layer);
            }
        }

        m_routeLayerInstances.insert(make_pair(iter->first, routeLayers));
        ++iter;
    }

    if (m_pUnifiedLayerManager && !addedLayers.empty())
    {
        m_pUnifiedLayerManager->NotifyLayersAdded(addedLayers);
    }
}

vector<LayerPtr> RouteManagerImpl::CreateRouteLayers(const vector <EncodedRouteInfo>& info)
{
    NB_Error    error         = NE_OK;
    RouteLayer* layerInstance = NULL;
    vector<LayerPtr> routeLayers;

    do
    {
        if (!GetRouteLayers())
        {
            error = NE_NOENT;
            break;
        }

        RouteLayer* routeLayer = NULL;
        layerInstance = NULL;
        if ((routeLayer = static_cast<RouteLayer*>(m_routeLayerFromDTS.get())) == NULL)
        {
            error = NE_UNEXPECTED;
            break;
        }

        // For parent layer.
        UnifiedLayerPtr uLayer = routeLayer->Clone();
        layerInstance = static_cast<RouteLayer*>(uLayer.get());
        if (!layerInstance)
        {
            error = NE_NOMEM;
            break;
        }

        error = layerInstance->SetRouteInfo(info);
        if (error != NE_OK)
        {
            break;
        }

        layerInstance->SetEnabled(true);
        layerInstance->RegisterListener(this);
        routeLayers.push_back(uLayer);

        // DVRT is a child of this route layer, add it to routeLayers.

        // Process children layers.
        vector<UnifiedLayerPtr> childLayers = uLayer->GetAllChildrenLayers();
        vector<UnifiedLayerPtr>::iterator iter = childLayers.begin();
        vector<UnifiedLayerPtr>::iterator end  = childLayers.end();
        for (; iter != end; ++iter)
        {
            layerInstance = static_cast<RouteLayer*>(iter->get());
            if (layerInstance && (layerInstance->SetRouteInfo(info) == NE_OK))
            {
                layerInstance->SetEnabled(true);
                layerInstance->RegisterListener(this);
                routeLayers.push_back(*iter);
            }
        }

    } while (0);

    if (error || routeLayers.empty())
    {
        routeLayers.clear();
    }

    return routeLayers;
}

void RouteManagerImpl::RemoveAllRouteLayersInternal()
{
    vector<LayerPtr> removedLayers;

    map<string, vector<LayerPtr> >::iterator iter = m_routeLayerInstances.begin();
    map<string, vector<LayerPtr> >::iterator end  = m_routeLayerInstances.end();

    while (iter != end)
    {
        vector<LayerPtr>& layers = iter->second;
        size_t size = layers.size();
        for (size_t i = 0; i < size; ++i)
        {
            const LayerPtr& layer = layers[i];
            RouteLayer* routeLayer = static_cast<RouteLayer*>(layer.get());
            if (routeLayer)
            {
                routeLayer->SetEnabled(false);
                routeLayer->UnregisterListener(this);
            }
            if (m_pLayerProvider->RemoveLayer(layer))
            {
                removedLayers.push_back(layer);
            }
        }
        layers.clear();
        iter ++;
    }

    m_routeLayerInstances.clear();

    if (m_pUnifiedLayerManager && !removedLayers.empty())
    {
        m_pUnifiedLayerManager->NotifyLayersRemoved(removedLayers);
    }
}

bool RouteManagerImpl::GetRouteLayers()
{
    if (!m_pLayerProvider)
    {
        return false;
    }

    if (!m_routeLayerFromDTS)
    {

        // @note: Currently we only have one route layer of type "route" is optional, and DVRT is
        // a child layer of route layer.
        vector<LayerPtr> pLayers = m_pLayerProvider->GetOptionalLayerPtrVectorFromParent();
        vector<LayerPtr>::iterator iter = find_if(pLayers.begin(), pLayers.end(),
                                                  LayerFindByDataTypeFunctor(TILE_TYPE_ROUTE));

        if (iter != pLayers.end()) // No layer is found, just return.
        {
            m_routeLayerFromDTS = *iter;
        }
    }

    return m_routeLayerFromDTS ? true : false;
}

void RouteManagerImpl::RefreshTiles(const vector<TileKeyPtr>& tileKeys, Layer* layer)
{
    UnifiedLayerManager* layerManager = static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
    if (!layerManager || !layer)
    {
        return;
    }

    map<string, vector<LayerPtr> >::iterator iter = m_routeLayerInstances.begin();
    map<string, vector<LayerPtr> >::iterator end  = m_routeLayerInstances.end();
    while (iter != end)
    {
        vector<LayerPtr>& layers = iter->second;
        vector<LayerPtr>::iterator layerIter = find_if(layers.begin(), layers.end(),
                                                       LayerFindByIdFunctor(layer->GetID()));

        if (layerIter != layers.end() && *layerIter)
        {
            layerManager->RefreshTiles(tileKeys, *layerIter);
            break;
        }
        ++iter;
    }
}

/*! @} */
