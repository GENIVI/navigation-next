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
/* nimtypes.h:                                                   */
/*                                                               */
/*                                                               */
/*****************************************************************/

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2005 by Networks In Motion, Inc. (NIM)          */
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

#ifndef NAVTYPES_H
#define NAVTYPES_H

// Define "Least Recently Used" flag for voice cache.
// @todo: (BUG 55819) The define should probably be removed and the code always enabled.
#ifndef ENABLE_VOICE_LRU
#define ENABLE_VOICE_LRU
#endif

#include "nberror.h"
#include "paltypes.h"
#include "cslutil.h"
#include "nbplace.h"

#define SEGMENT_NONE	0xFFFFFFFF

enum seg_match_type
{
	match_none = 0,
	match_proj,
	match_turn,
	match_start,
	match_end
};

enum route_match_type
{
	rt_match_none = 0,
	rt_match_normal,
	rt_match_turn,
	rt_match_destination
};

enum nav_update_reason
{
	nav_update_none = 0,
	nav_update_initial,
	nav_update_restart,
	nav_update_reload,
	nav_update_off_route,
	nav_update_wrong_way,
	nav_update_user,
	nav_update_detour,
	nav_update_detour_clear
};

struct seg_match
{
	uint32					time;
	double					travel_speed;
	uint32					maneuver;
	uint32					segment;
	double					remain;
	double					distance;
	double					proj_lat;
	double					proj_lon;
	double					heading;
	boolean					dirmatch;
	enum seg_match_type		type;
};

struct route_position
{
	uint32					time;
	uint32					closest_segment;
	uint32					closest_maneuver;

	double					segment_remain;
	double					segment_distance;
	double					segment_heading;
	double					proj_lat;
	double					proj_lon;

	double					est_speed_along_route;
	boolean					wrong_way;
	boolean					valid_heading;

	double					maneuver_max_inst_distance;   /* Note - this value does't take into account collapsed maneuvers */
	double					maneuver_distance_remaining;  /* Note - this value does't take into account collapsed maneuvers */
	double					maneuver_base_speed;

	double					trip_distance_after;
	uint32					trip_time_after;

	boolean					snap_map;
	boolean					near_window_end;

	enum route_match_type	match_type;
};

typedef struct
{
    double                  lat;
    double                  lon;
    double                  radius;
} NB_StartupRegion;

#endif
