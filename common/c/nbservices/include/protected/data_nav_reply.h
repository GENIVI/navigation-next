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
/* (C) Copyright 2004 by Networks In Motion, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to Networks In Motion, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/

/*-
 * data_nav_reply.h: created 2005/01/06 by Mark Goddard.
 */

#ifndef DATA_NAV_REPLY_H
#define DATA_NAV_REPLY_H

#include "datautil.h"
#include "vec.h"
#include "data_nav_maneuver.h"
#include "data_file.h"
#include "data_location.h"
#include "data_detour_avoid.h"
#include "data_route_map.h"
#include "data_traffic_record_identifier.h"
#include "data_traffic_flow.h"
#include "data_traffic_incidents.h"
#include "data_extended_content.h"
#include "nbroutesettings.h"
#include "data_box.h"
#include "data_speed_limits_data.h"
#include "paldebuglog.h"
#include "data_nav_reply_mult_form.h"
#include "data_pronun_list.h"
#include "data_route_summary.h"
#include "data_special_region.h"

#define NAV_REPLY_MANEUVER_NONE     0xFFFFFFFF
#define NAV_REPLY_MANEUVER_START    0xFFFFFFFE
#define SINGLE_ROUTE                ((uint32)-1)

typedef struct data_on_route_information_ {
    boolean  isTollsOnRoute;
    boolean  isHovLanesOnRoute;
    boolean  isHighWayOnRoute;
    boolean  isUnpavedOnRoute;
    boolean  isFerryOnRoute;
    boolean  isEnterCoutryOnRoute;
    boolean  isPrivateRoadOnRoute;
    boolean  isGatedAccessOnRoute;
} data_on_route_information;

typedef struct data_nav_reply_ {

    /* Child Elements */
    struct CSL_Vector*      vec_nav_maneuver;
    struct CSL_Vector*      vec_file;
    struct CSL_Vector*      vec_detour_avoid;

    data_location           origin_location;
    data_location           destination_location;

    data_traffic_record_identifier  traffic_record_identifier;

    data_traffic_flow       standard_traffic_flow;
    data_traffic_flow       historical_traffic_flow;
    data_traffic_flow       realtime_traffic_flow;

    uint32                  ferry_on_route;
    struct CSL_Vector*      vec_cameras;    // list of data_place containing data_speed_camera
    struct CSL_Vector*      via_point;

    data_traffic_incidents  traffic_incidents;
    data_box                route_extents;
    boolean                 hasRouteExtents;

    data_extended_content   extended_content;
    data_speed_limits_data* speed_regions;
    data_roadinfo*          first_major_road;
    data_pronun_list        pronun_list;

    struct CSL_Vector*     route_summarys;
    struct CSL_Vector*     vec_special_regions;

    /* Attributes */
    data_blob              route_id;

    double                 polyline_length;
    boolean                polyline_complete;

    uint32                 navManeuverStart;
    uint32                 navManeuverEnd;
    uint32                 navManeuverTotal;

    NB_RouteSettings       route_settings;

    data_box               route_extents_all;
    boolean                hasRouteExtentsAll;
    data_nav_reply_mult_form  route_selector_reply;
    uint32                 active_route;

    boolean                tolls_on_route;
    boolean                hov_lanes_on_route;
    boolean                highway_on_route;
    boolean                unpaved_on_route;
    boolean                        enter_coutry_on_route;
    boolean                        private_road_on_route;
    boolean                        gated_access_on_route;

    data_label_point       currentLabelPoint;
} data_nav_reply;

typedef NB_Error (*nav_speed_cb) (data_util_state* pds, const char* tmcloc, TrafficSpeedType speed_type, double minspeed,
                                  double* pspeed, char* pcolor, double* pfreeflowspeed,
                                  TrafficSpeedType* pspeed_type_out, void* user);


NB_Error    data_nav_reply_init(data_util_state* pds, data_nav_reply* pnr);
void        data_nav_reply_free(data_util_state* pds, data_nav_reply* pnr);

NB_Error    data_nav_reply_from_tps(data_util_state* pds, data_nav_reply* pnr, tpselt te, uint32 polyline_pack_passes);
NB_Error    data_nav_reply_single_from_tps(data_util_state* pds, data_nav_reply* pnr, tpselt te, uint32 polyline_pack_passes);

NB_Error    data_nav_reply_copy(data_util_state* pds, data_nav_reply* pnr_dest,
                                data_nav_reply* pnr_src);

uint32        data_nav_reply_num_nav_maneuver(data_util_state* pds, data_nav_reply* pnr);
data_nav_maneuver* data_nav_reply_get_nav_maneuver(data_util_state* pds, data_nav_reply* pnr, uint32 index);
uint32        data_nav_reply_num_nav_camera(data_util_state* pds, data_nav_reply* pnr);
data_place* data_nav_reply_get_nav_camera(data_util_state* pds, data_nav_reply* pnr, uint32 index);

NB_Error    data_nav_reply_last_point(data_util_state* pds, data_nav_reply* pnr, double* plat, double* plon);

void        data_nav_reply_file_foreach(data_util_state* pds, data_nav_reply* pnr,
                void (*cb)(data_file* pf, void* pUser), void* pUser);
void        data_nav_reply_discard_files(data_util_state* pds, data_nav_reply* pnr);

NB_Error    data_nav_reply_get_speed(data_util_state* pds, const char* tmcloc, TrafficSpeedType speed_type, double minspeed,
                                     double* pspeed, char* pcolor, double* pfreeflowspeed,
                                     TrafficSpeedType* pspeed_type_out, data_nav_reply* pnr);

NB_Error    data_nav_reply_get_traffic_info(data_util_state* pds, data_nav_reply* pnr, double minspeed,
                    uint32 min_no_data_disable_traffic_bar, uint32 start, double start_offset, uint32 end,
                    TrafficSpeedType speed_type, uint32* pdelay, double* pmeter,
                    uint32* ptotal_time, nav_speed_cb speed_cb, void* speed_user);

NB_Error    data_nav_reply_select_active(data_util_state* pds, data_nav_reply* pnr, uint32 activeRoute);

NB_Error    data_nav_reply_get_active(data_util_state* pds, data_nav_reply* pnr, uint32* activeRoute);

NB_Error    data_nav_reply_get_route_count(data_util_state* pds, data_nav_reply* pnr, uint32* routeCount);

data_nav_route* data_nav_reply_get_nav_route(data_util_state* pds, data_nav_reply* pnr, uint32 index);

NB_Error    data_nav_reply_get_single_nav_reply(data_util_state* pds, data_nav_reply* pnr, uint32 index, data_nav_reply* psnr);

NB_Error    data_nav_reply_get_on_route_info(data_util_state* pds, data_nav_reply* pnr, data_on_route_information* onRouteInfo);

NB_Error    data_nav_reply_get_label_point(data_util_state* pds, data_nav_reply* pnr, data_point* point);

NB_Error    data_nav_reply_get_current_label_point(data_util_state* pds, data_nav_reply* pnr, data_point* point);
NB_Error    data_nav_reply_from_mult_form(data_util_state*          pds,
                                          data_nav_reply_mult_form* pnr,
                                          data_nav_reply*           pdest,
                                          uint32                    routeIdx);

#endif
