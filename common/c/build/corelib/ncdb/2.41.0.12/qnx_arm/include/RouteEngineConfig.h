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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

/*****************************************************************/
#ifndef NCDB_ROUTE_ENGINE_CONFIG_H
#define NCDB_ROUTE_ENGINE_CONFIG_H
#include "NcdbTypes.h"

namespace Ncdb {
class RoutingProperties;

//! Sets the configuration parameters for the routing engine 
  //! These will include
  //!A)Engine mechanics like max memory ,max no of segments that can be explored
  //!max allowed time to find a route,max allowed cost for a route
  //!B)Static costs -turncosts,A* Pull factor,search tree pruning factors
  //!road transition costs  
  //!C)Dynamic Costs -Traffic information on roads ,information on blocked roads/detour segments etc
  //!Some of these parameters are derived from the user preferences while others are not.
  //!D)Point Projection Constants.
class  RouteEngineConfig
{
public:
	RouteEngineConfig();
	//!For now the Engine Config will be only set by passing in the Routing Properties 
	void SetRouteEngineConfig(const RoutingProperties* routingProps);
	//!Get the Point Projection Constants
	float GetProjectionMultFactor()const;
	float GetProjectionAddFactor()const;
	~RouteEngineConfig(){};
private:
	//!Point Projection Constants based on the speed of the vehicle
	//!to be used as speed *m_ProjectionMult+ m_ProjectionAdd;
	float m_ProjectionMultFactor;
	float m_ProjectionAddFactor;

};
};
#endif // NCDB_ROUTE_ENGINE_CONFIG_H
