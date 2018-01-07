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
    @file     MapServicesConfigurationImpl.h
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
#ifndef __MAP_SERVICES_CONFIGURATIONIMPL__
#define __MAP_SERVICES_CONFIGURATIONIMPL__

#include "MapServicesConfiguration.h"
#include <set>

namespace nbmap
{

class MapServicesConfigurationListener;
class MapView;
class GeographyManagerImpl;
class LayerManager;
class TrafficBubbleProvider;
class MetadataConfiguration;

/*! Get an instance of map view

    This class provides clients a way to get the map view interface. Offboard v/s
    Onboard will be handled by inside this class.
*/
class MapServicesConfigurationImpl : public MapServicesConfiguration
{
public:

    MapServicesConfigurationImpl();
    virtual ~MapServicesConfigurationImpl();

    /* See description in MapServicesConfiguration.h */
    virtual MapViewInterface* GetMap(NB_Context* context, MapConfiguration &config);

    /*! Set preferred language code for MapView.

      @param preferredLanguageCode Preferred language code for labels. 0 means local
      language. See NBM format doc for full code list.

      @return None
    */
    virtual void SetPreferredLanguageCode(uint8 preferredLanguageCode);

    /*! Set day & night mode for MapView.

      @param theme for map view.

      @return None
    */
    virtual void SetTheme(MapViewTheme theme);

    void CreateAndSpecifyManagers(NB_Context* context,
                                  shared_ptr<PinManager>& pinManager,
                                  PinCushion*& pinCushion,
                                  shared_ptr<PinBubbleResolver> pinBubbleResolver,
                                  shared_ptr<TrafficBubbleProvider> trafficBubbleProvider);

    /*! Register and unregister MapServicesConfigurationListener */
    NB_Error RegisterConfigurationListener(MapServicesConfigurationListener* listener);
    void UnregisterConfigurationListener(MapServicesConfigurationListener* listener);

private:
    NB_Error CreateManagersForMapView(NB_Context* context,
                                      shared_ptr<std::string> language,
                                      shared_ptr<LayerManager> layerManager,
                                      shared_ptr<PinManager> pinManager,
                                      shared_ptr<CustomLayerManager> customLayerManager,
                                      TrafficManagerImpl*& trafficManager,
                                      GeographyManagerImpl*& geographyManager,
                                      OptionalLayersAgentImpl*& agent);

    MapViewInterface* InitializeMapView(NB_Context* context,
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
                                        void* mapViewContext = NULL,
                                        MapViewListener* mapViewListener = NULL,
                                        MapViewFontMagnifierLevel fontMagnifierLeve = MVFML_LARGE,
                                        float scaleFactor = 1.f);

    uint8 m_preferredLanguageCode;      /*!< Preferred language code, used by MapViw to
                                             display labels in proper language.*/
    set<MapServicesConfigurationListener*> m_listeners;
    MapViewTheme m_Theme;
};

typedef shared_ptr<MapServicesConfigurationImpl> MapServicesConfigurationImplPtr;

}

#endif

/*! @} */
