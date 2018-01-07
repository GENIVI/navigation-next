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
    @file     MapServicesConfiguration.h
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
#ifndef __MAP_SERVICES_CONFIGURATION__
#define __MAP_SERVICES_CONFIGURATION__

extern "C"
{
    #include "nbcontext.h"
}

#include "smartpointer.h"
#include "PinBubbleResolver.h"
#include "MapViewInterface.h"
#include "LayerManager.h"
#include "nbroutemanager.h"
#include "TrafficManager.h"
#include "OptionalLayersAgent.h"
#include "AsyncCallback.h"

namespace nbmap
{
class LayerProvider;
class AggregateLayerProvider;
class PinManager;
class RouteManagerImpl;
class GeographyManagerImpl;
class TrafficManagerImpl;
class PinManagerImpl;
class CustomLayerManager;
class PinCushion;
class TrafficBubbleProvider;
class OptionalLayersAgentImpl;
class MetadataConfiguration;

/*! Get an instance of map view

    This class provides clients a way to get the map view interface. Offboard v/s
    Onboard will be handled by inside this class.
*/
class MapServicesConfiguration
{
public:
    virtual ~MapServicesConfiguration();

    /*! Generate singleton instance of MapServicesConfiguration.

        @return shared pointer of MapServicesConfiguration instance.
    */
    static shared_ptr<MapServicesConfiguration> GetInstance();

    /*! Get MapViewInterface instance.

        @return MapViewInterface instance if succeeded, or NULL otherwise.
    */
    virtual MapViewInterface* GetMap(NB_Context* context, MapConfiguration &config) = 0;

    /*! Set preferred language code for MapView.

      @param preferredLanguageCode Preferred language code for labels. 0 means local
             language. See NBM format doc for full code list.

      @return None
    */
    virtual void SetPreferredLanguageCode(uint8 preferredLanguageCode) = 0;

    /*! Set day & night mode for MapView.

      @param theme for map view.

      @return None
    */
    virtual void SetTheme(MapViewTheme theme) = 0;

private:
    static shared_ptr<MapServicesConfiguration> m_pMapServicesConfiguration;
};

typedef shared_ptr<MapServicesConfiguration> MapServicesConfigurationPtr;

}

#endif

/*! @} */
