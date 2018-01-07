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
   @file        TrafficManagerImpl.h
   @defgroup    nbmap
   Description: Interface class for TrafficManagerImpl, which manage traffic layers.
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

#ifndef _TRAFFICMANAGERIMPL_H_
#define _TRAFFICMANAGERIMPL_H_

#include "nbexp.h"
#include "smartpointer.h"
#include "UnifiedLayerManager.h"
#include "PinManager.h"
#include "TrafficIncidentListener.h"
#include "UnifiedLayerListener.h"
#include "LocalLayerProvider.h"
#include "TrafficManager.h"
#include <map>
#include <string>
extern "C"
{
#include "nbcontextprotected.h"
}

namespace nbmap
{

class TrafficIncidentLayer;
class LocalLayerProvider;
class TrafficManagerListener;
class PinCushion;

/*! TrafficManagerImpl to manage layers contains traffics */
class TrafficManagerImpl : public TrafficIncidentListener,
                           public TrafficManager,
                           public UnifiedLayerListener,
                           public Base
{
public:
    TrafficManagerImpl(NB_Context* context, shared_ptr<PinCushion> pinCushion);

    virtual ~TrafficManagerImpl();

    void Initialize(LayerManagerPtr layerManager,
                    PinManagerPtr pinManager,
                    shared_ptr<LocalLayerProvider> layerProvider,
                    LayerIdGeneratorPtr idGenerator,
                    shared_ptr<string> language);
    /* See description in TrafficManagerImpl.h */
    void EnableTraffic();
    void DisableTraffic();
    bool IsTrafficEnabled();
    vector<NB_TrafficIncident> GetTrafficIncidents();
    virtual void RegisterListener(TrafficManagerListener* listener);
    virtual void UnregisterListener(TrafficManagerListener* listener);

    /* See description in RefreshLayer.h */
    virtual void RefreshTiles(const vector<TileKeyPtr>& tileKeys,
                              Layer* layer);
    virtual void UnifiedLayerUpdated();

    /* See description in TrafficIncidentListener.h */
    virtual void TrafficIncidentsUpdated(const vector<NB_TrafficIncident>& trafficIncidents);


private:

    /*! Walk through layers provided by  OffboardProvider to find all traffic layers.

      @return None.
    */
    void
    GetTrafficLayers();

    /*! Creates copies of TrafficLayers instances.

      @return void
    */
    void CreateTrafficLayerCopies();

    /*! Creates a new TrafficIncidentLayer instance if necessary.

      @return void
    */
    void CreateTrafficIncidentLayerIfNecessary();

    // Private variables.
    bool m_enabled;                                 /*!< Flag to avoid repeated operations to enable or disable layers */
    shared_ptr<string> m_language;                  /*!< Language used to initialize the traffic incident layer */
    LayerManagerPtr m_pLayerManager;                /*!< Pointer of LayerManager instance */
    PinManagerPtr m_pPinManager;                    /*!< Pointer of PinManager instance */
    LocalLayerProviderPtr m_pLayerProvider;         /*!< LayerProvider to communicate with AggregateLayerProvider. */
    LayerIdGeneratorPtr m_pLayerIdGenerator;        /*!< Pointer of LayerIdGenerator instance. */
    vector<LayerPtr> m_trafficLayersFromDTS;        /*!< traffic Layers From DTS  */
    map<Layer*, LayerPtr> m_trafficLayerInstances;  /*!< Created instances of traffic layers */
    shared_ptr<TrafficIncidentLayer> m_trafficIncidentLayer; /*!< A layer used to search
                                                                  traffic incidents */
    set<TrafficManagerListener*> m_listeners;  /*!< The listeners of the traffic manager */
    shared_ptr<PinCushion> m_pinCushion;       /*!< pin cushion for getting traffic bubble resolver */
};

typedef shared_ptr<TrafficManagerImpl> TrafficManagerImplPtr;

};

#endif /* _TRAFFICMANAGERIMPL_H_ */

/*! @} */
