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

/*!--------------------------------------------------------------------------o
  @file     MapServicesConfigurationImpl.cpp
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
#include "MapServicesConfigurationImpl.h"
#include "contextbasedsingleton.h"
#include "UnifiedLayerManager.h"
#include "PinManagerImpl.h"
#include "CustomLayerManagerImpl.h"
#include "PinCushionImpl.h"
#include "LocalLayerProvider.h"
#include "AggregateLayerProvider.h"
#include "RouteManagerImpl.h"
#include "TrafficManagerImpl.h"

#include "OptionalLayersAgentImpl.h"
#include "MapView.h"
#include "PinBubbleResolver.h"
#include "TileServicesConfiguration.h"
#include "NBUITemplateInstantiator.h"
#include "LayerAgentImpl.h"
#include "GeographyManagerImpl.h"
#include "MetadataConfiguration.h"
#include <algorithm>
#include <functional>

// Default maximum count of requests to download tiles
#define DEFAULT_MAX_TILE_REQUEST_COUNT 64

// Default retry times of the tile requests
#define DEFAULT_TILE_REQUEST_RETRY_TIMES 3

const uint8 DEFAULT_LANGUAGE_CODE = 0;

using namespace nbmap;
using namespace protocol;

shared_ptr<MapServicesConfiguration> MapServicesConfiguration::m_pMapServicesConfiguration =
        shared_ptr<MapServicesConfiguration>();

MapServicesConfiguration::~MapServicesConfiguration()
{
}

shared_ptr<MapServicesConfiguration> MapServicesConfiguration::GetInstance()
{
    if (!MapServicesConfiguration::m_pMapServicesConfiguration)
    {
        MapServicesConfiguration::m_pMapServicesConfiguration =
                shared_ptr<MapServicesConfiguration>(new MapServicesConfigurationImpl());
    }

    return MapServicesConfiguration::m_pMapServicesConfiguration;
}

// Implementation of MapServiceConfigurationImpl.

MapServicesConfigurationImpl::MapServicesConfigurationImpl()
    : m_preferredLanguageCode(DEFAULT_LANGUAGE_CODE),m_Theme(MVT_DAY)
{
}

MapServicesConfigurationImpl::~MapServicesConfigurationImpl()
{
}

/* See description in MapServicesConfiguration.h */
MapViewInterface*
MapServicesConfigurationImpl::GetMap(NB_Context* context, MapConfiguration &config)
{
    NB_ASSERT_CCC_THREAD(context);

    if (!config.metadataConfig)
    {
        return NULL;
    }

    shared_ptr<LayerManager> layerManager =
            TileServicesConfiguration::GetLayerManager(context,
                                                       config.maximumCachingTileCount,
                                                       config.maximumTileRequestCountPerLayer,
                                                       config.metadataConfig);
    if (!layerManager)
    {
        return NULL;
    }

    shared_ptr<std::string> language = config.metadataConfig->m_pLanguage;

    PinCushion*     pinCushion     = NULL;
    GeographyManagerImpl* geographyManager = new GeographyManagerImpl(context);

    // This gives NBUI a chance to instantiate some templates.
    NBUI_InstantiateTemplates(this, context, config.outPinManager, pinCushion);
    // OK, NBUI has finished the instantiation, check those initialized objects.

    if (!config.outPinManager || !pinCushion)
    {
        return NULL;
    }

    shared_ptr<PinCushion> pinCushionPtr = shared_ptr<PinCushion>(pinCushion);
    TrafficManagerImpl* trafficManager = new TrafficManagerImpl(context, pinCushionPtr);
    if(!trafficManager)
    {
        return NULL;
    }

    OptionalLayersAgentImpl* agent             = NULL;

    // Now create other objects...
    NB_Error error = CreateManagersForMapView(context,
                                              language,
                                              layerManager,
                                              config.outPinManager,
                                              config.outCustomLayerManager,
                                              trafficManager,
                                              geographyManager,
                                              agent);
    if (error)
    {
        return NULL;
    }
    config.outOptionalLayersAgent.reset(agent);
    return InitializeMapView(context,
                             config.initializeCallback,
                             config.destroyCallback,
                             config.workFolder,
                             config.resourceFolder,
                             config.defaultLatitude,
                             config.defaultLongitude,
                             config.defaultTiltAngle,
                             layerManager,
                             pinCushionPtr,
                             shared_ptr<PinManager>(config.outPinManager),
                             config.outCustomLayerManager,
                             config.zorderLevel,
                             config.metadataConfig,
                             config.mapViewContext,
                             config.mapViewListener,
                             config.fontMagnifierLevel,
                             config.scaleFactor);
}

NB_Error
MapServicesConfigurationImpl::CreateManagersForMapView(NB_Context* context,
                                                       shared_ptr<std::string> language,
                                                       shared_ptr<LayerManager> layerManager,
                                                       shared_ptr<PinManager> pinManager,
                                                       shared_ptr<CustomLayerManager> customLayerManager,
                                                       TrafficManagerImpl*& trafficManager,
                                                       GeographyManagerImpl*& geographyManager,
                                                       OptionalLayersAgentImpl*& agent)
{
    NB_ASSERT_CCC_THREAD(context);

    NB_Error result = NE_OK;
    do
    {
        if (!layerManager || !pinManager || !trafficManager || !geographyManager)
        {
            result = NE_INVAL;
            break;
        }

        UnifiedLayerManager* unifiedLayerManager =
                static_cast<UnifiedLayerManager*>(layerManager.get());
        if (!unifiedLayerManager)
        {
            result = NE_UNEXPECTED;
            break;
        }

        LayerProviderPtr provider = unifiedLayerManager->GetLayerProvider();
        if (!provider)
        {
            result = NE_UNEXPECTED;
            break;
        }

        AggregateLayerProvider* aggregateLayerProvider =
                static_cast<AggregateLayerProvider*>(provider.get());
        if (!aggregateLayerProvider)
        {
            result = NE_UNEXPECTED;
            break;
        }

        LocalLayerProviderPtr optionalLayerProvider(
            new LocalLayerProvider(context, aggregateLayerProvider));
        if (!optionalLayerProvider)
        {
            result = NE_NOMEM;
            break;
        }
        aggregateLayerProvider->AddLayerProvider(optionalLayerProvider);

        // Get LayerIdGenerator from context.
        LayerIdGeneratorPtr layerIdGenerator =
                ContextBasedSingleton<LayerIdGenerator>::getInstance(context);

        agent = new OptionalLayersAgentImpl(context, layerManager,
                                            optionalLayerProvider, layerIdGenerator);
        if (!agent)
        {
            result = NE_NOMEM;
            break;
        }

        LocalLayerProviderPtr geographyLayerProvider(new LocalLayerProvider(context, aggregateLayerProvider));
        if(geographyManager)
        {
            geographyManager->Initialize(geographyLayerProvider, layerManager, layerIdGenerator);
            aggregateLayerProvider->AddLayerProvider(geographyLayerProvider);
            agent->RegisterGeographyManager(shared_ptr<GeographyManager> (geographyManager));
        }

        // CustomLayerManager is craeted, but not fully initialized, initialize it here.
        LocalLayerProviderPtr customLayerProvider(new LocalLayerProvider(context,
                                                                      aggregateLayerProvider));

        CustomLayerManagerImpl* customLayerManagerImpl = static_cast<CustomLayerManagerImpl*>(customLayerManager.get());
        if (customLayerManagerImpl)
        {
            customLayerManagerImpl->Initialize(customLayerProvider, layerManager, layerIdGenerator);
            aggregateLayerProvider->AddLayerProvider(customLayerProvider);
        }

        // PinManager is craeted, but not fully initialized, initialize it here.
        LocalLayerProviderPtr pinLayerProvider(new LocalLayerProvider(context,
                                                                      aggregateLayerProvider));

        PinManagerImpl* pinManagerImpl = static_cast<PinManagerImpl*>(pinManager.get());
        if (pinManagerImpl)
        {
            pinManagerImpl->Initialize(pinLayerProvider, layerManager, layerIdGenerator);
            aggregateLayerProvider->AddLayerProvider(pinLayerProvider);
        }

        // TrafficManager is crated, but not fully initialized, initialize it here.
        LocalLayerProviderPtr trafficLayerProvider(
            new LocalLayerProvider(context, aggregateLayerProvider));

        trafficManager->Initialize(layerManager,
                                   pinManager,
                                   trafficLayerProvider,
                                   layerIdGenerator,
                                   language);
        aggregateLayerProvider->AddLayerProvider(trafficLayerProvider);
        agent->RegisterTrafficManger(shared_ptr<TrafficManager> (trafficManager));

        // Create RouteManager.
        LocalLayerProviderPtr routeLayerProvider(
            new LocalLayerProvider(context, aggregateLayerProvider));
        aggregateLayerProvider->AddLayerProvider(routeLayerProvider);

        RouteManagerPtr routeManager(new RouteManagerImpl(context, layerManager,
                                                          routeLayerProvider, layerIdGenerator));
        if (!routeManager)
        {
            result = NE_NOMEM;
            break;
        }
        agent->RegisterRouteManger(shared_ptr<RouteManager> (routeManager));

        // Let OptionalLayerAgent to create all agent layers (if any).
        agent->UnifiedLayerUpdated();

    } while (0);

    if (result)
    {
        if (agent)
        {
            delete agent;
            agent = NULL;
        }
    }
    return result;
}


MapViewInterface*
MapServicesConfigurationImpl::InitializeMapView(NB_Context* context,
                                                shared_ptr<AsyncCallback<void*> > initializeCallback,
                                                shared_ptr<AsyncCallback<void*> > destroyCallback,
                                                const char* workFolder,
                                                const char* resourceFolder,
                                                double defaultLatitude,
                                                double defaultLongitude,
                                                double defaultTiltAngle,
                                                shared_ptr<LayerManager> layerManager,
                                                shared_ptr<PinCushion> pinCushion,
                                                shared_ptr<PinManager> pinManager,
                                                shared_ptr<CustomLayerManager> customLayerManager,
                                                int zorderLevel,
                                                shared_ptr<nbmap::MetadataConfiguration> metadataConfig,
                                                void* mapViewContext,
                                                MapViewListener* mapViewListener,
                                                MapViewFontMagnifierLevel fontMagnifierLevel,
                                                float scaleFactor)
{
    NB_ASSERT_CCC_THREAD(context);

    MapView* mapView = new MapView(this);
    if (mapView)
    {
        layerManager->SetPreferredLanguageCode(m_preferredLanguageCode);
        layerManager->SetTheme(m_Theme);
        NB_Error error = mapView->Initialize(context,
                                             initializeCallback,
                                             destroyCallback,
                                             workFolder,
                                             resourceFolder,
                                             defaultLatitude,
                                             defaultLongitude,
                                             defaultTiltAngle,
                                             layerManager,
                                             pinCushion,
                                             pinManager,
                                             customLayerManager,
                                             m_preferredLanguageCode,
                                             zorderLevel,
                                             mapViewContext,
                                             metadataConfig,
                                             mapViewListener,
                                             fontMagnifierLevel,
                                             scaleFactor);

        if (error != NE_OK)
        {
            mapView->Destroy();
            mapView = NULL;
        }
    }
    return mapView;
}

/* See description in header file. */
void MapServicesConfigurationImpl::SetPreferredLanguageCode(uint8 preferredLanguageCode)
{
    // @todo: this API should be called in CCC thread
    // NB_ASSERT_CCC_THREAD(context);

    if (m_preferredLanguageCode !=  preferredLanguageCode)
    {
        m_preferredLanguageCode = preferredLanguageCode;
        for_each(m_listeners.begin(), m_listeners.end(),
                 bind2nd(mem_fun(&MapServicesConfigurationListener::PreferredLanguageChanged),
                         preferredLanguageCode));
    }
}

/* See description in header file. */
void MapServicesConfigurationImpl::SetTheme(MapViewTheme theme)
{
    // this API should be called in CCC thread
    if (m_Theme != theme)
    {
        m_Theme = theme;
        for_each(m_listeners.begin(), m_listeners.end(),
                 bind2nd(mem_fun(&MapServicesConfigurationListener::MapThemeChanged),
                         theme));
    }
}

/* See description in header file. */
NB_Error MapServicesConfigurationImpl::RegisterConfigurationListener(MapServicesConfigurationListener* listener)
{
    if (!listener)
    {
        return NE_INVAL;
    }

    pair<set<MapServicesConfigurationListener*>::iterator, bool> result =
            m_listeners.insert(listener);
    if (result.second)
    {
        return NE_OK;
    }
    else
    {
        return NE_EXIST;
    }
}

/* See description in header file. */
void MapServicesConfigurationImpl::UnregisterConfigurationListener(MapServicesConfigurationListener* listener)
{
    m_listeners.erase(listener);
}

void
MapServicesConfigurationImpl::CreateAndSpecifyManagers(
    NB_Context*                       context,
    shared_ptr<PinManager>&           pinManager,
    PinCushion*&                      pinCushion,
    shared_ptr<PinBubbleResolver> pinBubbleResolver,
    shared_ptr<TrafficBubbleProvider> /*trafficBubbleProvider*/)

{
    if (!context)
    {
        return;
    }

    // Create an uninitialized PinManager, will be initialized later.
    pinManager.reset(new PinManagerImpl(context));

    pinCushion = new PinCushionImpl(pinManager,
                                    pinBubbleResolver,
                                    context);
}

/*! @} */
