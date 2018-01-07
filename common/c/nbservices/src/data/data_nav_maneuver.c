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
 * data_nav_maneuver.c: created 2005/01/02 by Mark Goddard.
 */

#include "data_nav_maneuver.h"
#include "transformutility.h"
#include "palmath.h"

#define GUIDANCE_COUNT_MAX 2

NB_Error
data_nav_maneuver_init(data_util_state* pds, data_nav_maneuver* pnm)
{
	NB_Error err = NE_OK;

	err = err ? err : data_point_init(pds, &pnm->point);
	
	err = err ? err : data_roadinfo_init(pds, &pnm->current_roadinfo);
	err = err ? err : data_roadinfo_init(pds, &pnm->turn_roadinfo);
	err = err ? err : data_roadinfo_init(pds, &pnm->intersection_roadinfo);
    err = err ? err : data_guidance_point_init(pds, &pnm->guidance_pointer[0]);
    err = err ? err : data_guidance_point_init(pds, &pnm->guidance_pointer[1]);
    err = err ? err : data_lane_info_init(pds, &pnm->lane_info);
    err = err ? err : data_max_turn_distance_for_street_count_init(pds, &pnm->max_turn_distance_for_street_count);
	pnm->stack_advise = FALSE;
    pnm->disable_guidance = FALSE;

	err = err ? err : data_polyline_init(pds, &pnm->polyline);

	pnm->distance = 0;
	pnm->current_heading = 0;
	pnm->speed = 0;
	pnm->max_instruction_distance = 0;
    pnm->isEnterCoutryOnRoute = FALSE;
    pnm->isFerryOnRoute = FALSE;
    pnm->isGatedAccessOnRoute = FALSE;
    pnm->isHighWayOnRoute = FALSE;
    pnm->isHovLanesOnRoute = FALSE;
    pnm->isPrivateRoadOnRoute = FALSE;
    pnm->isTollsOnRoute = FALSE;
    pnm->isUnpavedOnRoute = FALSE;

	err = err ? err : data_string_init(pds, &pnm->command);

	DATA_VEC_ALLOC(err, pnm->vec_traffic_regions, data_traffic_region);
	DATA_VEC_ALLOC(err, pnm->vec_cross_streets, data_cross_street);
    DATA_VEC_ALLOC(err, pnm->vec_realistic_signs, data_realistic_sign);

	return err;
}

void		
data_nav_maneuver_free(data_util_state* pds, data_nav_maneuver* pnm)
{
    if(!pnm)
    {
        return;
    }
	data_point_free(pds, &pnm->point);
	
	data_roadinfo_free(pds, &pnm->current_roadinfo);
	data_roadinfo_free(pds, &pnm->turn_roadinfo);
	data_roadinfo_free(pds, &pnm->intersection_roadinfo);
    data_lane_info_free(pds, &pnm->lane_info);
    data_max_turn_distance_for_street_count_free(pds, &pnm->max_turn_distance_for_street_count);
    data_guidance_point_free(pds, &pnm->guidance_pointer[0]);
    data_guidance_point_free(pds, &pnm->guidance_pointer[1]);
	pnm->stack_advise = FALSE;
    pnm->disable_guidance = FALSE;
	data_polyline_free(pds, &pnm->polyline);

	pnm->distance = 0;
	pnm->current_heading = 0;
	pnm->speed = 0;
	pnm->max_instruction_distance = 0;

	data_string_free(pds, &pnm->command);

	DATA_VEC_FREE(pds, pnm->vec_traffic_regions, data_traffic_region);
	DATA_VEC_FREE(pds, pnm->vec_cross_streets, data_cross_street);
    DATA_VEC_FREE(pds, pnm->vec_realistic_signs, data_realistic_sign);
}

int cross_street_comparer(const void* left, const void* right)
{
    const data_cross_street* leftStreet = (data_cross_street*)left;
    const data_cross_street* rightStreet = (data_cross_street*)right;
    if (leftStreet->progress < rightStreet->progress)
    {
        return -1;
    }
    else if (leftStreet->progress > rightStreet->progress)
    {
        return 1;
    }
    return 0;
}


NB_Error
data_nav_maneuver_from_tps(data_util_state* pds, data_nav_maneuver* pnm, tpselt te)
{
	NB_Error err = NE_OK;
	tpselt ce;
	int iter;
    int guidanceCount = 0;

	if (te == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	data_nav_maneuver_free(pds, pnm);

	err = data_nav_maneuver_init(pds, pnm);

	if (err != NE_OK)
		return err;
	
	ce = te_getchild(te, "point");

	if (ce == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	err = err ? err : data_point_from_tps(pds, &pnm->point, ce);

	ce = te_getchild(te, "current-roadinfo");

	if (ce == NULL) {
		err = NE_INVAL;
		goto errexit;
	}

	err = err ? err : data_roadinfo_from_tps(pds, &pnm->current_roadinfo, ce);

	ce = te_getchild(te, "turn-roadinfo");

    if (ce != NULL)
    {
        err = err ? err : data_roadinfo_from_tps(pds, &pnm->turn_roadinfo, ce);
    }
    else
    {
        /*
          for the last maneuver (DT.), there is only current-roadinfo and no turn-roadinfo,
          so we need to use current-roadinfo for turn_roadinfo for this case.
        */
        err = err ? err : data_roadinfo_copy(pds, &pnm->turn_roadinfo, &pnm->current_roadinfo);
    }

    ce = te_getchild(te, "route-feature-list");
    if(ce != NULL)
    {
        if (te_getchild(ce, "tolls-on-route"))
        {
            pnm->isTollsOnRoute = TRUE;
        }
        
        if (te_getchild(ce, "hov-lanes-on-route"))
        {
            pnm->isHovLanesOnRoute = TRUE;
        }
        
        if (te_getchild(ce, "highway-on-route"))
        {
            pnm->isHighWayOnRoute = TRUE;
        }
        
        if (te_getchild(ce, "enter-country-on-route"))
        {
            pnm->isEnterCoutryOnRoute = TRUE;
        }
        
        if (te_getchild(ce, "private-road-on-route"))
        {
            pnm->isPrivateRoadOnRoute = TRUE;
        }
        
        if (te_getchild(ce, "gated-access-on-route"))
        {
            pnm->isGatedAccessOnRoute = TRUE;
        }
        
        if (te_getchild(ce, "unpaved-on-route"))
        {
            pnm->isUnpavedOnRoute = TRUE;
        }
        
        if (te_getchild(ce, "ferry-on-route"))
        {
            pnm->isFerryOnRoute = TRUE;
        }
    }
	ce = te_getchild(te, "intersection-roadinfo");

	if (ce != NULL)
		err = err ? err : data_roadinfo_from_tps(pds, &pnm->intersection_roadinfo, ce);

	pnm->stack_advise = (boolean)(te_getchild(te, "stack-advise") != NULL);
    pnm->disable_guidance = (boolean)(te_getchild(te, "disable-guidance") != NULL);

	if (data_polyline_from_tps_attr(pds, &pnm->polyline, te, "polyline", TRUE) != NE_OK)
		data_polyline_init(pds, &pnm->polyline);

	if (!te_getattrf(te, "distance", &pnm->distance) || 
		!te_getattrf(te, "speed", &pnm->speed) ||
		!te_getattrf(te, "current-heading", &pnm->current_heading)) {

		err = NE_INVAL;
	}

	te_getattrf(te, "max-instruction-distance", &pnm->max_instruction_distance);

	err = err ? err : data_string_from_tps_attr(pds, &pnm->command, te, "command");

	iter = 0;

    while ((ce = te_nextchild(te, &iter)) != NULL)
    {
        const char * name = te_getname(ce);
        if (nsl_strcmp(name, "traffic-region") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, pnm->vec_traffic_regions, data_traffic_region);
        }

        else if (nsl_strcmp(name, "cross-street") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, pnm->vec_cross_streets, data_cross_street);
            CSL_VectorSort(pnm->vec_cross_streets, cross_street_comparer);
        }

        else if (nsl_strcmp(name, "realistic-sign") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, pnm->vec_realistic_signs, data_realistic_sign);
        }
        else if (nsl_strcmp(name, "guidance-point") == 0)
        {
            if (guidanceCount < GUIDANCE_COUNT_MAX)
            {
                DATA_FROM_TPS(pds, err, ce, &pnm->guidance_pointer[guidanceCount], data_guidance_point);
                guidanceCount ++;
            }
        }
        else if (nsl_strcmp(name, "lane-info") == 0)
        {
            DATA_FROM_TPS(pds, err, ce, &pnm->lane_info, data_lane_info);
        }
        else if (nsl_strcmp(name, "max-turn-distance-for-street-count") == 0)
        {
            DATA_FROM_TPS(pds, err, ce, &pnm->max_turn_distance_for_street_count, data_max_turn_distance_for_street_count);
        }

        if (err != NE_OK)
            goto errexit;
    }

errexit:
	if (err != NE_OK) {
		DEBUGLOG(LOG_SS_DATA, LOG_SEV_INFO, ("data_nav_maneuver failed (error_code: %d)", err));
		data_nav_maneuver_free(pds, pnm);
	}
	return err;
}

NB_Error	
data_nav_maneuver_copy(data_util_state* pds, data_nav_maneuver* pnm_dest, 
					   data_nav_maneuver* pnm_src)
{
	NB_Error err = NE_OK;

	data_nav_maneuver_free(pds, pnm_dest);
	err = data_nav_maneuver_init(pds, pnm_dest);

	err = err ? err : data_point_copy(pds, &pnm_dest->point, &pnm_src->point);

	err = err ? err : data_roadinfo_copy(pds, &pnm_dest->current_roadinfo, &pnm_src->current_roadinfo);
	err = err ? err : data_roadinfo_copy(pds, &pnm_dest->turn_roadinfo, &pnm_src->turn_roadinfo);
	err = err ? err : data_roadinfo_copy(pds, &pnm_dest->intersection_roadinfo, &pnm_src->intersection_roadinfo);
    err = err ? err : data_guidance_point_copy(pds, &pnm_dest->guidance_pointer[0], &pnm_src->guidance_pointer[0]);
    err = err ? err : data_guidance_point_copy(pds, &pnm_dest->guidance_pointer[1], &pnm_src->guidance_pointer[1]);
    err = err ? err : data_lane_info_copy(pds, &pnm_dest->lane_info, &pnm_src->lane_info);
    err = err ? err : data_max_turn_distance_for_street_count_copy(pds,
        &pnm_dest->max_turn_distance_for_street_count, &pnm_src->max_turn_distance_for_street_count);
	pnm_dest->stack_advise = pnm_src->stack_advise;
    pnm_dest->disable_guidance = pnm_src->disable_guidance;

	err = err ? err : data_polyline_copy(pds, &pnm_dest->polyline, &pnm_src->polyline);

	pnm_dest->distance = pnm_src->distance;
	pnm_dest->current_heading = pnm_src->current_heading;
	pnm_dest->speed = pnm_src->speed;
	pnm_dest->max_instruction_distance = pnm_src->max_instruction_distance;

	err = err ? err : data_string_copy(pds, &pnm_dest->command, &pnm_src->command);

	DATA_VEC_COPY(pds, err, pnm_dest->vec_traffic_regions, pnm_src->vec_traffic_regions, data_traffic_region);
	DATA_VEC_COPY(pds, err, pnm_dest->vec_cross_streets, pnm_src->vec_cross_streets, data_cross_street);
    DATA_VEC_COPY(pds, err, pnm_dest->vec_realistic_signs, pnm_src->vec_realistic_signs, data_realistic_sign);
    pnm_dest->isUnpavedOnRoute = pnm_src->isUnpavedOnRoute;
    pnm_dest->isTollsOnRoute = pnm_src->isTollsOnRoute;
    pnm_dest->isPrivateRoadOnRoute = pnm_src->isPrivateRoadOnRoute;
    pnm_dest->isHovLanesOnRoute = pnm_src->isHovLanesOnRoute;
    pnm_dest->isHighWayOnRoute = pnm_src->isHighWayOnRoute;
    pnm_dest->isGatedAccessOnRoute = pnm_src->isGatedAccessOnRoute;
    pnm_dest->isFerryOnRoute = pnm_src->isFerryOnRoute;
    pnm_dest->isEnterCoutryOnRoute = pnm_src->isEnterCoutryOnRoute;

	return err;
}

NB_Error
data_nav_maneuver_merge(data_util_state* pds, data_nav_maneuver* from,
                       data_nav_maneuver* to)
{
    NB_Error err = NE_OK;
    
    data_nav_maneuver temp;
    err = data_nav_maneuver_init(pds, &temp);
    
    //err = err ? err : data_polyline_copy(pds, &pnm_dest->polyline, &pnm_src->polyline);
    temp.polyline.numsegments = from->polyline.numsegments + to->polyline.numsegments;
    temp.polyline.segments = nsl_malloc(temp.polyline.numsegments * sizeof(pl_segment));
    if (temp.polyline.segments == NULL)
    {
        temp.polyline.numsegments = 0;
        return NE_NOMEM;
    }
    if(from->polyline.numsegments > 0)
    {
        nsl_memcpy(temp.polyline.segments, from->polyline.segments, from->polyline.numsegments * sizeof(pl_segment));
    }
    if(to->polyline.numsegments > 0)
    {
        nsl_memcpy(temp.polyline.segments + from->polyline.numsegments , to->polyline.segments, to->polyline.numsegments * sizeof(pl_segment));
    }
    temp.polyline.min_lat = MIN(from->polyline.min_lat, to->polyline.min_lat);
    temp.polyline.min_lon = MIN(from->polyline.min_lon, to->polyline.min_lon);
    temp.polyline.max_lat = MAX(from->polyline.max_lat, to->polyline.max_lat);
    temp.polyline.max_lon = MAX(from->polyline.max_lon, to->polyline.max_lon);
    temp.distance = from->distance + to->distance;
    
    DATA_VEC_COPY(pds, err, temp.vec_traffic_regions, from->vec_traffic_regions, data_traffic_region);
    for(uint32 i = 0; i < CSL_VectorGetLength(to->vec_traffic_regions); i++)
    {
        data_traffic_region* reg = (data_traffic_region*)CSL_VectorGetPointer(to->vec_traffic_regions, i);
        reg->start += from->distance;
    }
    DATA_VEC_COPY(pds, err, temp.vec_traffic_regions, to->vec_traffic_regions, data_traffic_region);
    DATA_VEC_COPY(pds, err, temp.vec_cross_streets, from->vec_cross_streets, data_cross_street);
    for(uint32 i = 0; i < CSL_VectorGetLength(to->vec_cross_streets); i++)
    {
        data_cross_street* data = (data_cross_street*)CSL_VectorGetPointer(to->vec_cross_streets, i);
        data->progress += from->distance;
    }
    DATA_VEC_COPY(pds, err, temp.vec_cross_streets, to->vec_cross_streets, data_cross_street);
    DATA_VEC_COPY(pds, err, temp.vec_realistic_signs, from->vec_realistic_signs, data_realistic_sign);
    for(uint32 i = 0; i < CSL_VectorGetLength(to->vec_realistic_signs); i++)
    {
        data_realistic_sign* data = (data_realistic_sign*)CSL_VectorGetPointer(to->vec_realistic_signs, i);
        data->position += from->distance;
    }
    DATA_VEC_COPY(pds, err, temp.vec_realistic_signs, to->vec_realistic_signs, data_realistic_sign);
    temp.isUnpavedOnRoute = from->isUnpavedOnRoute||to->isUnpavedOnRoute;
    temp.isTollsOnRoute = from->isTollsOnRoute || to->isTollsOnRoute;
    temp.isPrivateRoadOnRoute = from->isPrivateRoadOnRoute || to->isPrivateRoadOnRoute;
    temp.isHovLanesOnRoute = from->isHovLanesOnRoute || to->isHovLanesOnRoute;
    temp.isHighWayOnRoute = from->isHighWayOnRoute || to->isHighWayOnRoute;
    temp.isGatedAccessOnRoute = from->isGatedAccessOnRoute || to->isGatedAccessOnRoute;
    temp.isFerryOnRoute = from->isFerryOnRoute || to->isFerryOnRoute;
    temp.isEnterCoutryOnRoute = from->isEnterCoutryOnRoute || to->isEnterCoutryOnRoute;
    err = err ? err : data_point_copy(pds, &temp.point, &to->point);
    err = err ? err : data_roadinfo_copy(pds, &temp.current_roadinfo, &to->current_roadinfo);
    err = err ? err : data_roadinfo_copy(pds, &temp.turn_roadinfo, &to->turn_roadinfo);
    err = err ? err : data_roadinfo_copy(pds, &temp.intersection_roadinfo, &to->intersection_roadinfo);
    err = err ? err : data_guidance_point_copy(pds, &temp.guidance_pointer[0], &to->guidance_pointer[0]);
    err = err ? err : data_guidance_point_copy(pds, &temp.guidance_pointer[1], &to->guidance_pointer[1]);
    err = err ? err : data_lane_info_copy(pds, &temp.lane_info, &to->lane_info);
    err = err ? err : data_max_turn_distance_for_street_count_copy(pds,&temp.max_turn_distance_for_street_count, &to->max_turn_distance_for_street_count);
    temp.stack_advise = to->stack_advise;
    temp.disable_guidance = to->disable_guidance;
    temp.current_heading = from->current_heading;
    temp.speed = to->speed;
    temp.max_instruction_distance = to->max_instruction_distance;
    err = err ? err : data_string_copy(pds, &temp.command, &to->command);
    err = data_nav_maneuver_copy(pds, to, &temp);
    return err;
}


boolean		
data_nav_maneuver_polyline_complete(data_util_state* pds, data_nav_maneuver* pnm)
{
	uint32 len = data_polyline_len(pds, &pnm->polyline);
	double lat;
	double lon;

	if (len == 0)
		return FALSE;

	if (data_polyline_get(pds, &pnm->polyline, len-1, &lat, &lon, NULL, NULL) != NE_OK)
		return FALSE;

	return compare_latlon(lat, lon, pnm->point.lat, pnm->point.lon, 0.0001);
}
