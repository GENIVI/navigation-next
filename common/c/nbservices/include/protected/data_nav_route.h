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

 @file     data_nav_route.h
 */
/*
 (C) Copyright 2012 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunications Systems, Inc. is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 ---------------------------------------------------------------------------*/

/*! @{ */

#ifndef data_nav_route_H
#define data_nav_route_H

#include "datautil.h"
#include "vec.h"
#include "data_nav_maneuver.h"
#include "data_detour_avoid.h"
#include "data_route_map.h"
#include "data_traffic_record_identifier.h"
#include "data_traffic_flow.h"
#include "data_traffic_incidents.h"
#include "data_extended_content.h"
#include "data_box.h"
#include "data_speed_limits_data.h"
#include "data_route_style.h"
#include "data_label_point.h"
#include "paldebuglog.h"
#include "data_special_region.h"

typedef struct data_nav_route_ {

    /* Child Elements */
    struct CSL_Vector*             vec_nav_maneuver;
    struct CSL_Vector*             vec_detour_avoid;
    struct CSL_Vector*             vec_cameras;

    uint32                         ferry_on_route;
    boolean                        hasRouteExtents;
    data_extended_content          extended_content;

    data_traffic_incidents         traffic_incidents;
    data_traffic_record_identifier traffic_record_identifier;

    data_traffic_flow              standard_traffic_flow;
    data_traffic_flow              historical_traffic_flow;
    data_traffic_flow              realtime_traffic_flow;

    data_label_point               label_point;
    data_box                       route_extents;
    data_speed_limits_data*        speed_regions;
    data_roadinfo*                 first_major_road;

    data_route_style               route_style;

    boolean                        tolls_on_route;
    boolean                        hov_lanes_on_route;
    boolean                        highway_on_route;
    boolean                        unpaved_on_route;
    boolean                        enter_coutry_on_route;
    boolean                        private_road_on_route;
    boolean                        gated_access_on_route;

    struct CSL_Vector*             vec_special_regions;

    /* Attributes */
    data_blob             route_id;

    double                polyline_length;
    boolean               polyline_complete;

    uint32                navManeuverStart;
    uint32                navManeuverEnd;
    uint32                navManeuverTotal;
} data_nav_route;


NB_Error    data_nav_route_init(data_util_state* pds, data_nav_route* pnr);

void        data_nav_route_free(data_util_state* pds, data_nav_route* pnr);

NB_Error    data_nav_route_from_tps(data_util_state* pds, data_nav_route* pnr, tpselt te, uint32 polyline_pack_passes);

NB_Error    data_nav_route_copy(data_util_state* pds, data_nav_route* pnr_dest,
                                data_nav_route* pnr_src);

uint32      data_nav_route_num_nav_maneuver(data_util_state* pds, data_nav_route* pnr);
data_nav_maneuver* data_nav_route_get_nav_maneuver(data_util_state* pds, data_nav_route* pnr, uint32 index);
/*! }@ */
#endif
