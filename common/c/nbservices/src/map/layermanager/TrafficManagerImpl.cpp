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
  @file        TrafficManagerImpl.cpp
  @defgroup    nbmap
  Description: Class Impl for TrafficManagerImpl, which manage traffic layers.
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

#include "LayerFunctors.h"
#include "LayerProvider.h"
#include "LocalLayerProvider.h"
#include "RefreshLayer.h"
#include "TrafficIncidentLayer.h"
#include "TrafficManagerImpl.h"
#include "TrafficManagerListener.h"
#include "UnifiedLayerManager.h"
#include <algorithm>
#include <functional>
#include <utility>
#include "PinCushion.h"

using namespace nbmap;

/* The unique ID for traffic incident layer */
const char TRAFFIC_INCIDENT_LAYER_ID[] = "traffic-incident-layer";

/*! Hardcoded IDs of possible traffic layers.*/
static const char* TRAFFIC_TILE_TYPES[] =
{
    "traffic", NULL
};

/*! Functor used to enable or disable the layer */
class LayerEnabledFunctor : public Base
{
public:
    /*! LayerEnabledFunctor constructor */
    LayerEnabledFunctor(bool enabled,                       /*!< Whether to enable or disable the layer */
                        UnifiedLayerManager* layerManager   /*!< A layer manager used to refresh the layer */
                        )
            : m_enabled(enabled),
              m_layerManager(layerManager)
    {
        // Nothing to do here.
    }

    /*! LayerEnabledFunctor destructor */
    ~LayerEnabledFunctor()
    {
        // Nothing to do here.
    }

    /*! Operator to enable or disable the layer */
    void operator() (const pair<Layer*, LayerPtr>& layerPair)
    {
        LayerPtr layer = layerPair.second;
        if (!layer)
        {
            return;
        }

        // Convert to an unified layer to set enabled.
        UnifiedLayer* unifiedLayer = static_cast<UnifiedLayer*>(layer.get());
        if (unifiedLayer)
        {
            unifiedLayer->SetEnabled(m_enabled);
        }

        // Refresh the tiles of this layer.
        if (m_layerManager)
        {
            m_layerManager->RefreshTilesOfLayer(layer);
        }
    }


private:
    bool m_enabled;                         /*!< Whether to enable or disable the layer */
    UnifiedLayerManager* m_layerManager;    /*!< A layer manager used to refresh the layer */
};


TrafficManagerImpl::TrafficManagerImpl(NB_Context* context, shared_ptr<PinCushion> pinCushion)
        : UnifiedLayerListener(context),
          m_enabled(false),
          m_pinCushion(pinCushion)
{
    shared_ptr<AggregateLayerProvider> aggregateLayerProvider =
            ContextBasedSingleton<AggregateLayerProvider>::getInstance(context);
    aggregateLayerProvider->RegisterUnifiedLayerListener(this, false);
}

TrafficManagerImpl::~TrafficManagerImpl()
{
    // Unregister the listener of saved traffic layers.
    DisableTraffic();

    // Unregister the listener of the traffic incident layer.
    if (m_trafficIncidentLayer)
    {
        m_trafficIncidentLayer->UnregisterListener(this);
        m_trafficIncidentLayer.reset();
    }
}

/* See description in TrafficManagerImpl.h */
void
TrafficManagerImpl::EnableTraffic()
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    // Check if layers has already been enabled.
    if (m_enabled)
    {
        return;
    }

    // Get the unified layer manager.
    UnifiedLayerManager* layerManager = static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
    if (!layerManager)
    {
        return;
    }

    if (m_trafficLayerInstances.empty())
    {
        CreateTrafficLayerCopies();
    }

    // do not proceed if m_trafficLayersFromDTS is empty
    // to allow the upper layer another chance to enable traffic again
    // KARMAEXT-333
    if (m_trafficLayersFromDTS.empty())
    {
        return;
    }

    vector<LayerPtr> addedLayers;
    map<Layer*, LayerPtr>::iterator iter = m_trafficLayerInstances.begin();
    map<Layer*, LayerPtr>::iterator end  = m_trafficLayerInstances.end();
    while (iter != end)
    {
        const LayerPtr& layer = iter->second;
        // Unregister the layer from this traffic manager.
        RefreshLayer* refreshLayer = static_cast<RefreshLayer*>(layer.get());
        if (refreshLayer)
        {
            refreshLayer->SetEnabled(true);
            refreshLayer->RegisterListener(this);
            if (m_pLayerProvider->AddLayer(layer))
            {
                addedLayers.push_back(layer);
            }
        }

        ++iter;
    }

    CreateTrafficIncidentLayerIfNecessary();
    // Add the traffic incident layer to layer provider.
    if (m_trafficIncidentLayer)
    {
        // Enable traffic incidents.
        TrafficIncidentLayer* trafficIncidentLayer =
                static_cast<TrafficIncidentLayer*>(m_trafficIncidentLayer.get());
        if (trafficIncidentLayer)
        {
            trafficIncidentLayer->SetEnabled(true);
            if (m_pLayerProvider->AddLayer(m_trafficIncidentLayer))
            {
                addedLayers.push_back(m_trafficIncidentLayer);
            }
        }
    }

    // Set the flag to identify layers enabled.
    m_enabled = true;

    // Notify layers are added.
    if (!addedLayers.empty())
    {
        layerManager->NotifyLayersAdded(addedLayers);
    }
}

/* See description in TrafficManagerImpl.h */
void
TrafficManagerImpl::DisableTraffic()
{
    NB_ASSERT_CCC_THREAD(m_pContext);

    // Check if layers has already been disabled.
    if (!m_enabled)
    {
        return;
    }

    // Get the unified layer manager.
    UnifiedLayerManager* layerManager = static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
    if (!layerManager)
    {
        return;
    }

    // Remove all traffic layers.
    vector<LayerPtr> removedLayers;
    map<Layer*, LayerPtr>::const_iterator layerIterator = m_trafficLayerInstances.begin();
    map<Layer*, LayerPtr>::const_iterator layerEnd = m_trafficLayerInstances.end();
    for (; layerIterator != layerEnd; ++layerIterator)
    {
        const LayerPtr& layer = layerIterator->second;
        if (!layer)
        {
            continue;
        }

        RefreshLayer* refreshLayer = static_cast<RefreshLayer*>(layer.get());
        if (refreshLayer)
        {
            refreshLayer->SetEnabled(false);
            refreshLayer->UnregisterListener(this);
            // Add the layer to notify removed.
            m_pLayerProvider->RemoveLayer(layer);
            removedLayers.push_back(layer);
        }
    }

    // Set the flag to identify layers disabled.
    m_enabled = false;

    // Disable traffic incidents.
    if (m_trafficIncidentLayer)
    {
        TrafficIncidentLayer* trafficIncidentLayer =
                static_cast<TrafficIncidentLayer*>(m_trafficIncidentLayer.get());
        if (trafficIncidentLayer)
        {
            trafficIncidentLayer->SetEnabled(false);
            if (m_pLayerProvider->RemoveLayer(m_trafficIncidentLayer))
            {
                removedLayers.push_back(m_trafficIncidentLayer);
            }
        }
    }

    // Notify layers removed.
    if (!removedLayers.empty())
    {
        layerManager->NotifyLayersRemoved(removedLayers);
    }
}

/* See description in TrafficManagerImpl.h */
bool
TrafficManagerImpl::IsTrafficEnabled()
{
    return m_enabled;
}

/* See description in TrafficManagerImpl.h */
vector<NB_TrafficIncident>
TrafficManagerImpl::GetTrafficIncidents()
{
    vector<NB_TrafficIncident> incidents;

    if (m_trafficIncidentLayer)
    {
        // Get the traffic incidents from the traffic incident layer.
        incidents = m_trafficIncidentLayer->GetTrafficIncidents();
    }

    return incidents;
}

/* See description in TrafficManagerImpl.h */
void
TrafficManagerImpl::RegisterListener(TrafficManagerListener* listener)
{
    if (listener)
    {
        m_listeners.insert(listener);
    }
}

/* See description in TrafficManagerImpl.h */
void
TrafficManagerImpl::UnregisterListener(TrafficManagerListener* listener)
{
    if (listener)
    {
        m_listeners.erase(listener);
    }
}

/* See description in RefreshLayer.h */
void
TrafficManagerImpl::RefreshTiles(const vector<TileKeyPtr>& tileKeys,
                                 Layer* layer)
{
    // Check if the parameters are valid.
    if (!layer)
    {
        return;
    }

    // Get the unified layer manager.
    UnifiedLayerManager* layerManager = static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
    if (!layerManager)
    {
        return;
    }

    // Refresh tiles of traffic incident layer
    m_trafficIncidentLayer->RefreshSearchResults(tileKeys);
    if(tileKeys.empty())
    {
        layerManager->RefreshTilesOfLayer(m_trafficIncidentLayer);
    }
    else
    {
        layerManager->RefreshTiles(tileKeys, m_trafficIncidentLayer);
    }

    LayerPtr foundLayer;

    // Check if the layer exists in this traffic manager.
    map<Layer*, LayerPtr>::iterator layerIterator = m_trafficLayerInstances.find(layer);
    if (layerIterator != m_trafficLayerInstances.end())
    {
        foundLayer = layerIterator->second;
    }

    // Check if the layer is found.
    if (!foundLayer)
    {
        return;
    }

    // Refresh tiles of this layer.
    if(tileKeys.empty())
    {
        layerManager->RefreshTilesOfLayer(foundLayer);
    }
    else
    {
        layerManager->RefreshTiles(tileKeys, foundLayer);
    }
}

/* See description in TrafficIncidentListener.h */
void
TrafficManagerImpl::TrafficIncidentsUpdated(const vector<NB_TrafficIncident>& trafficIncidents)
{
    // Notify each listener that the traffic incidents are updated.
    set<TrafficManagerListener*>::const_iterator listenerIterator = m_listeners.begin();
    set<TrafficManagerListener*>::const_iterator listenerEnd = m_listeners.end();
    for (; listenerIterator != listenerEnd; ++listenerIterator)
    {
        TrafficManagerListener* listener = *listenerIterator;
        if (listener)
        {
            listener->TrafficIncidentsUpdated(trafficIncidents);
        }
    }
}

/* See description in TrafficIncidentListener.h */
void TrafficManagerImpl::GetTrafficLayers()
{
    // Check if the traffic layers has been got before.
    if (!(m_trafficLayersFromDTS.empty()))
    {
        return;
    }

    UnifiedLayerManager* unifiedLayerManager =
            static_cast<UnifiedLayerManager*>(m_pLayerManager.get());
    if (!unifiedLayerManager || !m_pLayerProvider)
    {
        return;
    }

    vector<LayerPtr> pLayers = m_pLayerProvider->GetOptionalLayerPtrVectorFromParent();
    if (pLayers.empty())
    {
        return;
    }

    int i = 0;
    vector<LayerPtr>::iterator end = pLayers.end();
    while (TRAFFIC_TILE_TYPES[i] != NULL)
    {
        shared_ptr<string> trafficId =
                shared_ptr<string>(new string(TRAFFIC_TILE_TYPES[i]));
        if (trafficId)
        {
            LayerFindByDataTypeFunctor finder(trafficId);
            RefreshLayer* refreshLayer = NULL;

            vector<LayerPtr>::iterator iter = find_if(pLayers.begin(), end, finder);
            if (iter != end &&
                (refreshLayer = static_cast<RefreshLayer*>((*iter).get())) != NULL &&
                refreshLayer->IsCreatedByOffboardLayerManager())
            {
                m_trafficLayersFromDTS.push_back(*iter);
            }
        }
        ++i;
    }
}

void TrafficManagerImpl::UnifiedLayerUpdated()
{
    m_trafficLayersFromDTS.clear();
    m_trafficIncidentLayer.reset();
    if (!m_trafficLayerInstances.empty())
    {
        if (m_enabled)
        {
            DisableTraffic();
            m_trafficLayerInstances.clear();
            EnableTraffic();
        }
        else
        {
            m_trafficLayerInstances.clear();
        }
    }
}


/* See description in header file. */
void TrafficManagerImpl::Initialize(LayerManagerPtr layerManager,
                                    PinManagerPtr pinManager,
                                    shared_ptr < LocalLayerProvider > layerProvider,
                                    LayerIdGeneratorPtr idGenerator,
                                    shared_ptr <string> language)
{
    m_language          = language;
    m_pLayerManager     = layerManager;
    m_pPinManager       = pinManager;
    m_pLayerProvider    = layerProvider;
    m_pLayerIdGenerator = idGenerator;
}

/* See description in header file. */
void TrafficManagerImpl::CreateTrafficLayerCopies()
{
    GetTrafficLayers();

    // Check if there are traffic layers.
    if (m_trafficLayersFromDTS.empty())
    {
        return;
    }

    // Add traffic layers.
    map<Layer*, LayerPtr> newTrafficLayerInstances;
    vector<LayerPtr>::const_iterator layerIterator = m_trafficLayersFromDTS.begin();
    vector<LayerPtr>::const_iterator layerEnd = m_trafficLayersFromDTS.end();
    for (; layerIterator != layerEnd; ++layerIterator)
    {
        LayerPtr layer = *layerIterator;
        if (!layer)
        {
            continue;
        }

        // Convert this layer to a refresh layer.
        RefreshLayer* refreshLayer = static_cast<RefreshLayer*>(layer.get());
        if (!refreshLayer)
        {
            continue;
        }

        // Create a new refresh layer copied from the template layer.
        UnifiedLayerPtr uLayer = refreshLayer->Clone();
        RefreshLayer* newLayer = static_cast<RefreshLayer*>(uLayer.get());
        if (!newLayer)
        {
            return;
        }
        // Add the layer to layer provider.
        newLayer->RegisterListener(this);
        newTrafficLayerInstances.insert(make_pair(newLayer, uLayer));
    }

    // Replace traffic layers. Traffic incident layer should not be added.
    m_trafficLayerInstances.swap(newTrafficLayerInstances);
}

/* See description in header file. */
void TrafficManagerImpl::CreateTrafficIncidentLayerIfNecessary()
{
    if (m_trafficIncidentLayer || !m_pLayerIdGenerator)
    {
        return;
    }

    // Generate a digital ID for the traffic incident layer.
    uint32 incidentDigitalId = m_pLayerIdGenerator->GenerateId();
    if (incidentDigitalId == LayerIdGenerator::InvalidId)
    {
        return;
    }

    // Create a string of layer ID for the traffic incident layer.
    shared_ptr<string> incidentLayerId(new string(TRAFFIC_INCIDENT_LAYER_ID));
    if (!incidentLayerId)
    {
        return;
    }
    m_trafficIncidentLayer =
            TrafficIncidentLayerPtr(new TrafficIncidentLayer(m_pContext, incidentDigitalId,
                                                             incidentLayerId, m_pPinManager, m_pinCushion));
    if (m_trafficIncidentLayer)
    {
        m_trafficIncidentLayer->RegisterListener(this);
        m_trafficIncidentLayer->SetLanguage(m_language);
    }
}

/*! @} */
