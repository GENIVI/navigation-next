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
   @file        OptionalLayersAgentImpl.h
   @defgroup    nbmap

   Description: OptionLayersAgenImpl inherits from and implemented
   OptionalLayersAgent.

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

#ifndef _OPTIONALLAYERSAGENTIMPL_H_
#define _OPTIONALLAYERSAGENTIMPL_H_

#include "OptionalLayersAgent.h"
#include "UnifiedLayerListener.h"
#include "GeographicLayer.h"
#include "base.h"
#include <map>

namespace nbmap
{

class TrafficManager;
class RouteManager;
class GeographyManager;
class LocalLayerProvider;
class LayerIdGenerator;
class LayerManager;

class OptionalLayersAgentImpl : public OptionalLayersAgent,
                                public UnifiedLayerListener,
                                public GeographicLayerListener
{
public:
    OptionalLayersAgentImpl(NB_Context* context,
                            shared_ptr<LayerManager> layerManager,
                            shared_ptr<LocalLayerProvider> layerProvider,
                            shared_ptr<LayerIdGenerator> layerIdGenerator);
    virtual ~OptionalLayersAgentImpl();

    // Refer to Base class for description, commands'll be forward to corresponding managers.
    virtual shared_ptr<string> AddRouteLayer(const vector<RouteInfo*>& routeInfo);
    virtual NB_Error RemoveRouteLayer(shared_ptr<string> routeLayerId);
    virtual NB_Error RemoveAllRouteLayers();
    virtual GeographyOperationPtr AddGeographyManager(const string& layerId);
    virtual void RemoveGeographyManager(const string& layerId);
    virtual NB_Error EnableTraffic();
    virtual NB_Error DisableTraffic();
    virtual vector<NB_TrafficIncident> GetTrafficIncidents();
    virtual void RegisterTrafficManagerListener(TrafficManagerListener* listener);
    virtual void UnegisterTrafficManagerListener(TrafficManagerListener* listener);
    virtual const vector<LayerAgentPtr>& GetOptionalLayerAgents();

    virtual GeoPolyline* AddPolyline(const GeoPolylineOption& option);
    virtual void SetPolylineZorder(GeoPolyline* polyline, int zorder);
    virtual void SetPolylineSelected(GeoPolyline* polyline, bool selected);
    virtual void SetPolylineVisible(GeoPolyline* polyline, bool visible);
    virtual void RemoveGeographic(vector<Geographic*>& graphic);
    virtual void GetAllGeographics(vector<Geographic*>& graphic);

    // Refer to UnifiedLayerListener for description.
    virtual void UnifiedLayerUpdated();

    // Refer to GeographicLayerListener for description.
    virtual void GeographicUpdated(Geographic* graphic,  GeographicUpdateType updateType);
    virtual void GeographicToggled(Geographic* graphic);
    virtual void MaterialAdded(ExternalMaterialPtr material);
    virtual void MaterialRemoved(ExternalMaterialPtr material);
    virtual void MaterialActivate(ExternalMaterialPtr material);
    /*! Register RouteManager into this Agent class.

      @return None.
    */
    void RegisterRouteManger(shared_ptr<RouteManager> routeManager);

    /*! Register GeographyManager into this Agent class.

        @return None.
     */
    void RegisterGeographyManager(shared_ptr<GeographyManager> geographyManager);

    /*! Register TrafficManager into this Agent class.

      @return None.
    */
    void RegisterTrafficManger(shared_ptr<TrafficManager> trafficManager);

private:

    /*! Clear all previously cloned optional layers.

      @return None
    */
    void ClearOptionalLayers();

    static void  CCC_LayersModified(PAL_Instance* pal, void* userData);
    void UnifiedLayersAdded(const vector<UnifiedLayerPtr>& layers);
    void UnifiedLayersRemoved(const vector<UnifiedLayerPtr>& layers);
    static void CCC_DestoryTrafficManager(PAL_Instance* pal, void* userData);

    shared_ptr<TrafficManager>     m_pTrafficManager;/*!< Pointer of TrafficManager  */
    shared_ptr<RouteManager>       m_pRouteManager;  /*!< Pointer of RouteManager  */
    shared_ptr<LocalLayerProvider> m_pLayerProvider; /*!< LocalLayerProvider intance */
    vector<LayerAgentPtr>          m_pLayerAgents;   /*!< Cached optional layers (cloned
                                                          from original optional layers
                                                          of OffboardLayerProvider) */
    shared_ptr<LayerIdGenerator> m_pLayerIdGenerator; /*!< ID generator */
    shared_ptr<LayerManager>     m_pLayerManager;     /*!< Pointer of UnifiedLayerManager */
    //@todo: remove m_pGeographyManager when new NBM library is read!
    shared_ptr<GeographyManager> m_pGeographyManager; /*!< Pointer of GeographyManagerImpl */
    shared_ptr<GeographicLayer>  m_pGeographicLayer;  /*!< Pointer of geographic Layer  */

    typedef map<Geographic*, shared_ptr<GeographicLayer> > GEOGRAPHIC_LAYER_MAP;

    GEOGRAPHIC_LAYER_MAP m_geoGraphicMap;

    set<Geographic*> m_combinPolylines;

    /*! Lock for thread synchronization. */
    PAL_Lock* m_lock;
    shared_ptr<bool> m_isValid;
};

}

#endif /* _OPTIONALLAYERSAGENTIMPL_H_ */

/*! @} */
