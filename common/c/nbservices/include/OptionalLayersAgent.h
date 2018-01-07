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
   @file        OptionalLayersAgent.h
   @defgroup    nbmap

   Description: OptionalLayersAgent is a proxy of all managers who manages
   optional layer. Client can toggle optional layers through it.
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

#ifndef _OPTIONALLAYERSAGENT_H_
#define _OPTIONALLAYERSAGENT_H_


extern "C"
{
#include "nbrouteinformation.h"
#include "nbplace.h"
}

#include "smartpointer.h"
#include "TrafficManagerListener.h"
#include "LayerAgent.h"
#include "GeographyOperation.h"
#include "GeoPolyline.h"

namespace nbmap
{
class OptionalLayersAgent
{
public:
    virtual ~OptionalLayersAgent(){}

    /*! Add new geography manager identified by layerId.

        @return geography operation object pointer.
    */
    virtual GeographyOperationPtr AddGeographyManager(const string& layerId) = 0;

    /*! Remove geography operation object identified by layerId.

        @return None.
     */
    virtual void RemoveGeographyManager(const string& layerId) = 0;

    // Interfaces to operate route layers.
    /*! Add new route layer identified by routeId.

      @return Identifier of added layers. Client should store it and pass it back when
      RemoveRouteLayer() is called..
    */
    virtual shared_ptr<string> AddRouteLayer(const vector<RouteInfo*>& routeInfo) = 0;

    /*! Remove route layer(s) identified by routeLayerId.

      @return NE_Ok if succeeded.
    */
    virtual NB_Error RemoveRouteLayer(shared_ptr<string> routeLayerId) = 0;

    /*! Remove all route layers in Managed by this manager.

      @return None.
    */
    virtual NB_Error RemoveAllRouteLayers() = 0;

    // Interfaces to operate traffic layers
    /*! Enable traffic

      @return None
    */
    virtual NB_Error EnableTraffic() = 0;

    /*! Disable traffic

      @return None
    */
    virtual NB_Error DisableTraffic() = 0;

    /*! Get the traffic incidents

      This function should be called to get the current traffic incidents. User should also
      register a listener to monitor the traffic incidents updated.

      @todo: This function returns all traffic incidents so far. I think it should return the
      visible traffic incidents on map.

      @return A vector of the traffic incidents
    */
    virtual vector<NB_TrafficIncident> GetTrafficIncidents() = 0;

    //@TODO: Is this interface really neede?
    virtual void RegisterTrafficManagerListener(TrafficManagerListener* listener) = 0;
    virtual void UnegisterTrafficManagerListener(TrafficManagerListener* listener) = 0;

    /*! Return all optional layer agents except route and traffic.

      These layers are returned with LayerAgent, client can operate these agents, refer to
      description of LayerAgent for more details.

      @return vector
    */
    virtual const vector<LayerAgentPtr>& GetOptionalLayerAgents() = 0;

    /*! Add a polyline with given option.

      @return Pointer of created GeoPolyline object.
    */
    virtual GeoPolyline* AddPolyline(const GeoPolylineOption& option) = 0;

    /*! set one polyline zorder.

      @return no return
    */
    virtual void SetPolylineZorder(GeoPolyline* polyline, int zorder) = 0;

    virtual void SetPolylineSelected(GeoPolyline* polyline, bool selected) = 0;

    virtual void SetPolylineVisible(GeoPolyline* polyline, bool visible) = 0;

    /**
     * @name RemoveGeoGraphic - remove an geographic from Map.
     * @param graphic -  graphic to be removed.
     * @return None.
     */
    virtual void RemoveGeographic(vector<Geographic*>& graphic) = 0;

    /*! Returns all GeoGraphic Objects.

       @return void
    */
    virtual void GetAllGeographics(vector<Geographic*>& graphic) = 0;
};

typedef shared_ptr<OptionalLayersAgent> OptionalLayersAgentPtr;

}

#endif /* _OPTIONALLAYERSAGENT_H_ */

/*! @} */
