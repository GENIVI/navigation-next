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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2014 by TeleCommunication Systems, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to TeleCommunication Systems, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of TeleCommunication Systems is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/
#ifndef NCDB_CORRIDORROUTE_H
#define NCDB_CORRIDORROUTE_H



#include "Route.h"

namespace Ncdb {


/*!
   OffRouteSegment is a  structure which stores A)The IDs of potential offroute links,
   B)Their angle w.r.t the original route and C)A potential link on the route(OnRouteSegID)
   where they can get back onto the original route.

*/
struct NCDB_API OffRouteSegment
{
	int dir;
	float heading;
	float m_mapDataSpeed;//! Speed as we get from the map data	
	WorldPoint ptLoc;
	NodeSegmentID m_SegmentID;
};
typedef AutoArray<OffRouteSegment> OffRouteSegmentList;
#ifdef _MSC_VER
template class NCDB_API AutoSharedPtr < OffRouteSegmentList >;
#endif

/*! 
    CorridorRoute associated with a maneuver in the Route 
	is the set of all routes formed with the potential offroute links along the Route as origin
	and links along the route as the destination.
*/
class NCDB_API CorridorRoute
{
    public:
		//!Default Constructor
		CorridorRoute();
		~CorridorRoute();
       // const RoadSegmentIDList& GetOffRouteSegmentList();
        const RoadSegmentIDList& GetOnRouteSegmentList() const;
		AutoSharedPtr<OffRouteSegmentList> GetOffRouteSegmentList() const;
        AutoSharedPtr<RouteList> GetRouteList() const;
    private:
         RoadSegmentIDList   m_OnRouteSegmentsList; 
		//!List of OffRouteSegment
		AutoSharedPtr<OffRouteSegmentList>        mp_OffRouteSegmentList;
		AutoSharedPtr<RouteList>                  mp_RouteList;
		friend class RouteEngine;
};
/*! Set of all Corridor Routes for a given route
 */
typedef AutoArray<CorridorRoute> CorridorRouteList;
#ifdef _MSC_VER
template class NCDB_API  AutoArray<CorridorRoute>; 
#endif

};
#endif // NCDB_CORRIDORROUTE_H

