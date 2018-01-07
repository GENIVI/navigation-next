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
 * data_nav_query.h: created 2005/01/06 by Mark Goddard.
 */

#ifndef DATA_NAV_QUERY_H
#define DATA_NAV_QUERY_H

#include "datautil.h"
#include "data_origin.h"
#include "data_destination.h"
#include "data_cache_contents.h"
#include "data_map_cache_contents.h"
#include "data_route_style.h"
#include "data_string.h"
#include "data_recalc.h"
#include "data_detour_avoid.h"
#include "data_nav_reply.h"
#include "data_want_guidance_config.h"
#include "data_want_alternate_routes.h"
#include "data_want_extended_pronun_data.h"
#include "nbroutesettings.h"
#include "nbexp.h"

typedef struct data_nav_query_ {

    /* Child Elements */
    data_origin                 origin;
    data_destination            destination;
    data_cache_contents         cache_contents;
    data_map_cache_contents     map_cache_contents;
    data_route_style            route_style;
    data_recalc                 recalc;
    data_want_guidance_config   want_guidance_config;
    data_want_alternate_routes  want_alternate_routes;
    data_want_extended_pronun_data want_extended_pronun_data;
    boolean                     want_complete_route;

    struct CSL_Vector*          vec_detour_avoid;
    struct CSL_Vector*          via_point;
    struct CSL_Vector*          extra_destination;

    boolean                     want_origin_location;
    boolean                     want_destination_location;
    boolean                     has_recalc;
    boolean                     disable_navigation;
    boolean                     want_route_map;
    boolean                     want_cross_streets;
    boolean                     want_maneuvers;
    boolean                     want_all_maneuvers;

    uint32                      route_map_angle_of_view;
    uint32                      route_map_distance_of_view;
    uint32                      route_map_min_time;
    uint32                      route_map_passes;
    uint32                      route_map_tile_z;
    uint32                      route_map_max_tiles;

    boolean                     want_historical_traffic;
    boolean                     want_realtime_traffic;
    boolean                     want_historical_speed;
    boolean                     want_realtime_speed;
    boolean                     want_traffic_notification;
    boolean                     want_unpaved_info;
    boolean                     want_speed_cameras;
    boolean                     want_ferry_maneuvers;
    boolean                     want_towards_roadinfo;
    boolean                     want_intersection_roadinfo;
    boolean                     want_country_info;
    boolean                     want_enter_roundabout_maneuvers;
    boolean                     want_enter_country_maneuvers;
    boolean                     want_bridge_maneuvers;
    boolean                     want_tunnel_maneuvers;
    boolean                     want_route_extents;
    boolean                     want_realistic_road_signs;
    boolean                     want_junction_models;
    boolean                     want_city_models;
    boolean                     want_exit_numbers;
    boolean                     want_speed_regions;
    boolean                     want_first_major_road;
    boolean                     want_label_point;

    boolean                     want_non_guidance_maneuvers;

    /* Attributes */
    data_string                 pronun_style;
    uint32                      command_set_version;

    NB_RouteSettings            route_settings;

    data_blob                   recreate_route_id;

    uint32                      max_pronun_files;

    data_string                 language;

    boolean                     want_max_turn_for_street_count;
    boolean                     want_route_summary;
    boolean                     want_route_summary_delaytime;
    boolean                     want_special_regions;
    boolean                     want_first_maneuver_cross_streets;
    boolean                     want_secondary_name_for_exits;
    boolean                     want_check_unsupported_countries;
    boolean                     want_pedestrian_aneuvers;
    boolean                     start_to_navigate;
    boolean                     want_entry_indicators;
    boolean                     want_route_features;
} data_nav_query;

NB_Error    data_nav_query_init(data_util_state* pds, data_nav_query* pnq);
void        data_nav_query_free(data_util_state* pds, data_nav_query* pnq);

tpselt      data_nav_query_to_tps(data_util_state* pds, data_nav_query* pnq);
NB_Error    data_nav_query_copy(data_util_state* pds, data_nav_query* dst, data_nav_query* src);
NB_Error    data_nav_query_copy_detours_from_reply(data_util_state* pds, data_nav_query* pnq, data_nav_reply* pnr);
NB_Error    data_nav_query_add_detour(data_util_state* pds, data_nav_query* pnq, data_blob* rid, double start, double length, const char* label, boolean want_historical_traffic, boolean want_realtime_traffic);
NB_Error    data_nav_query_clear_detours(data_util_state* pds, data_nav_query* pnq);
NB_Error    data_nav_query_set_route_selector_params(data_util_state* pds, data_nav_query* pnq, uint32 alternate_routes_count, boolean want_labels);

#endif
