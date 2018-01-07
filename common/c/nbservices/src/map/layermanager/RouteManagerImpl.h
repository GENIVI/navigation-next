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
   @file        RouteManager.h
   @defgroup    nbmap
   Description: Interface class for RouteManager, which manage route layers.
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

#ifndef _ROUTEMANAGERIMPL_H_
#define _ROUTEMANAGERIMPL_H_

#include "nbexp.h"
#include "smartpointer.h"
#include "UnifiedLayerManager.h"
#include "AggregateLayerProvider.h"
#include "RouteLayer.h"
#include "LocalLayerProvider.h"
#include "UnifiedLayerListener.h"
#include "nbroutemanager.h"
#include "RefreshLayer.h"

extern "C"
{
#include "nbcontextprotected.h"
}

namespace nbmap
{

/*! RouteManager to manage layers contains routes */

class RouteManagerImpl : public Base,
                         public RouteManager,
                         public UnifiedLayerListener,
                         public RefreshLayerListener

{
public:

    /*! Default Constructor. */
    RouteManagerImpl(NB_Context*           context,      /*!< NB_Contetx instance. */
                     LayerManagerPtr       layerManager, /*!< LayerManager instance. */
                     LocalLayerProviderPtr layerProvider, /*!< Local layer provider to hold
                                                            created layers. */
                     LayerIdGeneratorPtr idGenerator);

    virtual ~RouteManagerImpl();

    /*! Refer to RouteManagerImpl for description. */
    virtual shared_ptr<string> AddRouteLayer(const vector<RouteInfo*>& routeInfo);

    /*! Refer to RouteManagerImpl for description. */
    virtual NB_Error RemoveRouteLayer(shared_ptr<string> routeLayerId);

    /*! Refer to RouteManagerImpl for description. */
    virtual void RemoveAllRouteLayers();

    /*! UnifiedLayerListener. */
    virtual void UnifiedLayerUpdated();

    /*! RefreshLayerListener. */
    virtual void RefreshTiles(const vector<TileKeyPtr>& tileKeys, Layer* layer);

private:

    /*! Resets this RouteManagerImpl.

      @return None
    */
    void Reset();

    /*! Looks for all route layers stored in OffboardLayerProvider.

      @return false if failed to get route layer from DTS server.
    */
    bool
    GetRouteLayers();

    /*! Encode routeInfo and return a key to represent this routeInfo.

      @return shared pointer of the key.
    */
    shared_ptr<string>
    CreateIdFromRouteInfo(const vector<RouteInfo*>& routeInfo,       /*!< RouteInfo to be encoded. */
                          vector<EncodedRouteInfo>& encodedRouteInfo /*!< encoded Route Info  */
                          );

    /*! Creates route layers based on RouteInfo.

      @return vector of created route layers.
    */
    vector<LayerPtr>
    CreateRouteLayers(const vector<EncodedRouteInfo>& info /*!< Route information  */
                      );

    /*! Internal function to remove all route layers created by this manager.

      @return void
    */
    void RemoveAllRouteLayersInternal();


    // Private variables.
    LayerManagerPtr       m_pLayerManager;        /*!< Pointer of LayerManager Instance */
    UnifiedLayerManager*  m_pUnifiedLayerManager; /*!< Pointer of UnifiedLayerManager, we
                                                    can also cast it from m_pLayerManager */
    LocalLayerProviderPtr m_pLayerProvider;       /*!< LayerProvider to communicate
                                                    with AggregateLayerProvider. */

    map<string, vector<EncodedRouteInfo> > m_cachedRouteInfo;
    map<string, vector<LayerPtr> > m_routeLayerInstances;/*!<  m route Layer Instances  */
    LayerPtr       m_routeLayerFromDTS;  /*!< route Layers returned from DTS, keep a copy
                                              of these layers to decide whether we need
                                              to create new route layer instances when
                                              metadata changed. */

    LayerIdGeneratorPtr m_pLayerIdGenerator; /*!< Pointer of LayerIdGenerator instance. */
};

typedef shared_ptr<RouteManagerImpl>   RouteManagerImplPtr;


};

#endif /* _ROUTEMANAGERIMPL_H_ */

/*! @} */
