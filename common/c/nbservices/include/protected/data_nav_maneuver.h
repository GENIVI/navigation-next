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
 * data_nav_maneuver.h: created 2005/01/02 by Mark Goddard.
 */

#ifndef DATA_NAV_MANEUVER_H
#define DATA_NAV_MANEUVER_H

#include "datautil.h"
#include "data_polyline.h"
#include "data_string.h"
#include "data_blob.h"
#include "data_point.h"
#include "data_roadinfo.h"
#include "data_turn_map.h"
#include "data_traffic_region.h"
#include "data_cross_street.h"
#include "data_realistic_sign.h"
#include "data_lane_info.h"
#include "data_guidance_point.h"
#include "data_max_turn_distance_for_street_count.h"
#include "nbexp.h"

#define GUIDANCE_POINTS_MAX 2
typedef struct data_nav_maneuver_ {

    /* Child Elements */
    data_point        point;

    data_roadinfo    current_roadinfo;
    data_roadinfo    turn_roadinfo;
    data_roadinfo    intersection_roadinfo;

    boolean          stack_advise;
    boolean          disable_guidance;

    data_guidance_point guidance_pointer[GUIDANCE_POINTS_MAX];
    data_lane_info      lane_info;
    data_max_turn_distance_for_street_count max_turn_distance_for_street_count;

	/* Attributes */
	data_polyline	polyline;

    double            current_heading;
    double            max_instruction_distance;
    double            speed;
    double            distance;
    
    /* route features.*/
    boolean  isTollsOnRoute;
    boolean  isHovLanesOnRoute;
    boolean  isHighWayOnRoute;
    boolean  isUnpavedOnRoute;
    boolean  isFerryOnRoute;
    boolean  isEnterCoutryOnRoute;
    boolean  isPrivateRoadOnRoute;
    boolean  isGatedAccessOnRoute;

    data_string        command;

    struct CSL_Vector*      vec_traffic_regions;
    struct CSL_Vector*      vec_cross_streets;
    struct CSL_Vector*      vec_realistic_signs;

} data_nav_maneuver;

NB_Error    data_nav_maneuver_init(data_util_state* pds, data_nav_maneuver* pnm);
void        data_nav_maneuver_free(data_util_state* pds, data_nav_maneuver* pnm);

NB_Error    data_nav_maneuver_from_tps(data_util_state* pds, data_nav_maneuver* pnm, tpselt te);
NB_Error    data_nav_maneuver_copy(data_util_state* pds, data_nav_maneuver* pnm_dest,
                                   data_nav_maneuver* pnm_src);

boolean     data_nav_maneuver_polyline_complete(data_util_state* pds, data_nav_maneuver* pnm);

NB_Error    data_nav_maneuver_merge(data_util_state* pds, data_nav_maneuver* from,
                                    data_nav_maneuver* to);

#endif
