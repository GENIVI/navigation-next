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

 @file     data_nav_route.c
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

#include "data_nav_route.h"
#include "csltypes.h"

NB_Error
data_nav_route_init(data_util_state* pds, data_nav_route* pnr)
{
    NB_Error err = NE_OK;

    pnr->vec_nav_maneuver = CSL_VectorAlloc(sizeof(data_nav_maneuver));
    if (pnr->vec_nav_maneuver == NULL)
    {
        err = NE_NOMEM;
        goto errexit;
    }

    pnr->vec_detour_avoid = CSL_VectorAlloc(sizeof(data_detour_avoid));
    if (pnr->vec_detour_avoid == NULL)
    {
        err = NE_NOMEM;
        goto errexit;
    }

    pnr->vec_cameras = CSL_VectorAlloc(sizeof(data_place));
    if (pnr->vec_cameras == NULL)
    {
        err = NE_NOMEM;
        goto errexit;
    }

    pnr->speed_regions = (data_speed_limits_data*)nsl_malloc(sizeof(data_speed_limits_data));
    if (pnr->speed_regions == NULL)
    {
        err = NE_NOMEM;
        goto errexit;
    }

    pnr->first_major_road = (data_roadinfo*)nsl_malloc(sizeof(data_roadinfo));
    if (pnr->first_major_road == NULL)
    {
        err = NE_NOMEM;
        goto errexit;
    }

    pnr->vec_special_regions = CSL_VectorAlloc(sizeof(data_special_region));
    if (pnr->vec_special_regions == NULL)
    {
        err = NE_NOMEM;
        goto errexit;
    }


    err = data_label_point_init(pds, &pnr->label_point);
    err = err ? err : data_blob_init(pds, &pnr->route_id);
    err = err ? err : data_box_init(pds, &pnr->route_extents);
    err = err ? err : data_extended_content_init(pds, &pnr->extended_content);
    err = err ? err : data_speed_limits_data_init(pds, pnr->speed_regions);
    err = err ? err : data_roadinfo_init(pds, pnr->first_major_road);
    err = err ? err : data_route_style_init(pds, &pnr->route_style);

    err = err ? err : data_traffic_incidents_init(pds, &pnr->traffic_incidents);
    err = err ? err : data_traffic_record_identifier_init(pds, &pnr->traffic_record_identifier);
    err = err ? err : data_traffic_flow_init(pds, &pnr->standard_traffic_flow);
    err = err ? err : data_traffic_flow_init(pds, &pnr->historical_traffic_flow);
    err = err ? err : data_traffic_flow_init(pds, &pnr->realtime_traffic_flow);

    if (err != NE_OK)
    {
        goto errexit;
    }

    pnr->tolls_on_route = FALSE;
    pnr->hov_lanes_on_route = FALSE;
    pnr->highway_on_route = FALSE;
    pnr->unpaved_on_route = FALSE;

    pnr->hasRouteExtents = FALSE;
    pnr->ferry_on_route = 0;

    pnr->polyline_length = 0.0;
    pnr->polyline_complete = FALSE;

    pnr->navManeuverStart = 0;
    pnr->navManeuverEnd = 0xFFFFFF;
    pnr->navManeuverTotal = 0xFFFFFF;
    pnr->gated_access_on_route = FALSE;
    pnr->private_road_on_route = FALSE;
    pnr->enter_coutry_on_route = FALSE;

errexit:
    if (err != NE_OK)
    {
        data_nav_route_free(pds, pnr);
    }

    return err;
}

void
data_nav_route_free(data_util_state* pds, data_nav_route* pnr)
{
    if (!pnr)
    {
        return;
    }
    DATA_VEC_FREE(pds, pnr->vec_nav_maneuver, data_nav_maneuver);
    DATA_VEC_FREE(pds, pnr->vec_detour_avoid, data_detour_avoid);
    DATA_VEC_FREE(pds, pnr->vec_cameras, data_place);
    DATA_VEC_FREE(pds, pnr->vec_special_regions, data_special_region);

    data_label_point_free(pds, &pnr->label_point);
    data_traffic_record_identifier_free(pds, &pnr->traffic_record_identifier);
    data_traffic_flow_free(pds, &pnr->standard_traffic_flow);
    data_traffic_flow_free(pds, &pnr->historical_traffic_flow);
    data_traffic_flow_free(pds, &pnr->realtime_traffic_flow);
    data_traffic_incidents_free(pds, &pnr->traffic_incidents);
    data_route_style_free(pds, &pnr->route_style);

    data_box_free(pds, &pnr->route_extents);
    data_extended_content_free(pds, &pnr->extended_content);
    data_blob_free(pds, &pnr->route_id);

    if (pnr->speed_regions != NULL)
    {
        data_speed_limits_data_free(pds, pnr->speed_regions);
        nsl_free(pnr->speed_regions);
        pnr->speed_regions = NULL;
    }

    if (pnr->first_major_road != NULL)
    {
        data_roadinfo_free(pds, pnr->first_major_road);
        nsl_free(pnr->first_major_road);
        pnr->first_major_road = NULL;
    }
}


NB_Error
data_nav_route_copy(data_util_state* pds, data_nav_route* pnr_dest, data_nav_route* pnr_src)
{
    NB_Error err = NE_OK;

    data_nav_route_free(pds, pnr_dest);
    data_nav_route_init(pds, pnr_dest);

    DATA_VEC_COPY(pds, err, pnr_dest->vec_nav_maneuver, pnr_src->vec_nav_maneuver, data_nav_maneuver);
    DATA_VEC_COPY_EX(pds, err, pnr_dest->vec_detour_avoid, pnr_src->vec_detour_avoid, data_detour_avoid, TRUE);
    DATA_VEC_COPY(pds, err, pnr_dest->vec_cameras, pnr_src->vec_cameras, data_place);
    DATA_VEC_COPY(pds, err, pnr_dest->vec_special_regions, pnr_src->vec_special_regions, data_special_region);

    err = err ? err : data_label_point_copy(pds, &pnr_dest->label_point, &pnr_src->label_point);
    err = err ? err : data_blob_copy(pds, &pnr_dest->route_id, &pnr_src->route_id);
    err = err ? err : data_box_copy(pds, &pnr_dest->route_extents, &pnr_src->route_extents);
    err = err ? err : data_extended_content_copy(pds, &pnr_dest->extended_content, &pnr_src->extended_content);
    err = err ? err : data_speed_limits_data_copy(pds, pnr_dest->speed_regions, pnr_src->speed_regions);
    err = err ? err : data_roadinfo_copy(pds, pnr_dest->first_major_road, pnr_src->first_major_road);
    err = err ? err : data_route_style_copy(pds, &pnr_dest->route_style, &pnr_src->route_style);

    err = err ? err : data_traffic_incidents_copy(pds, &pnr_dest->traffic_incidents, &pnr_src->traffic_incidents);
    err = err ? err : data_traffic_record_identifier_copy(pds, &pnr_dest->traffic_record_identifier, &pnr_src->traffic_record_identifier);
    err = err ? err : data_traffic_flow_copy(pds, &pnr_dest->standard_traffic_flow, &pnr_src->standard_traffic_flow);
    err = err ? err : data_traffic_flow_copy(pds, &pnr_dest->historical_traffic_flow, &pnr_src->historical_traffic_flow);
    err = err ? err : data_traffic_flow_copy(pds, &pnr_dest->realtime_traffic_flow, &pnr_src->realtime_traffic_flow);

    pnr_dest->tolls_on_route = pnr_src->tolls_on_route;
    pnr_dest->hov_lanes_on_route = pnr_src->hov_lanes_on_route;
    pnr_dest->highway_on_route = pnr_src->highway_on_route;
    pnr_dest->unpaved_on_route = pnr_src->unpaved_on_route;
    pnr_dest->private_road_on_route = pnr_src->private_road_on_route;
    pnr_dest->gated_access_on_route = pnr_src->gated_access_on_route;
    pnr_dest->enter_coutry_on_route = pnr_src->enter_coutry_on_route;

    pnr_dest->hasRouteExtents = pnr_src->hasRouteExtents;
    pnr_dest->ferry_on_route = pnr_src->ferry_on_route;

    pnr_dest->polyline_length = pnr_src->polyline_length;
    pnr_dest->polyline_complete = pnr_src->polyline_complete;

    pnr_dest->navManeuverStart = pnr_src->navManeuverStart;
    pnr_dest->navManeuverEnd = pnr_src->navManeuverEnd;
    pnr_dest->navManeuverTotal = pnr_src->navManeuverTotal;

    return err;
}

static uint32
data_nav_route_get_tps_traffic_speed_type(TrafficSpeedType st)
{
    switch (st) {

        case TrafficSpeedNone:
            break;
        case TrafficSpeedHistoric:
            return 0;
        case TrafficSpeedRealTime:
            return 1;
        case TrafficSpeedStandard:
            return 2;
        default:
            break;
    }

    return 0xFFFFFFFF;
}

NB_Error
data_nav_route_from_tps(data_util_state* pds, data_nav_route* pnr, tpselt te, uint32 polyline_pack_passes)
{
    NB_Error err = NE_OK;

    tpselt ce = NULL;
    tpselt ce2 = NULL;

    int iter = 0;
    int n = 0;
    int l = 0;

    data_nav_maneuver*  pman = NULL;
    data_nav_maneuver    nav_maneuver = {{0}};
    data_place           place = {{{0}}};
    data_detour_avoid    detour_avoid = {{{0}}};
    data_special_region  specialRegion = {0};

    if (te == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    err = data_nav_maneuver_init(pds, &nav_maneuver);
    if (err != NE_OK)
        goto errexit;

    err = data_place_init(pds, &place);
    if (err != NE_OK)
        goto errexit;

    err = data_detour_avoid_init(pds, &detour_avoid);
    if (err != NE_OK)
        goto errexit;

    err = data_special_region_init(pds, &specialRegion);
    if (err != NE_OK)
        goto errexit;

    data_nav_route_free(pds, pnr);
    err = data_nav_route_init(pds, pnr);
    if (err != NE_OK)
        goto errexit;

    while ((ce = te_nextchild(te, &iter)) != NULL)
    {
        const char* name = te_getname(ce);

        if (nsl_strcmp(name, "nav-maneuver") == 0)
        {
            err = data_nav_maneuver_from_tps(pds, &nav_maneuver, ce);

            if (err == NE_OK)
            {
                if (!CSL_VectorAppend(pnr->vec_nav_maneuver, &nav_maneuver))
                    err = NE_NOMEM;

                if (err == NE_OK)
                    nsl_memset(&nav_maneuver, 0, sizeof(nav_maneuver)); // clear out the nav_maneuver since we have copied it (shallow copy)
                else
                    data_nav_maneuver_free(pds, &nav_maneuver); // free the nav_maneuver if it was not copied
            }

            if (err != NE_OK)
                goto errexit;
        }
        else if (nsl_strcmp(name, "detour-avoid") == 0)
        {
            err = data_detour_avoid_from_tps(pds, &detour_avoid, ce);

            if (err == NE_OK)
            {
                if (!CSL_VectorAppend(pnr->vec_detour_avoid, &detour_avoid))
                    err = NE_NOMEM;

                if (err == NE_OK)
                    nsl_memset(&detour_avoid, 0, sizeof(detour_avoid)); // clear out the detour_avoid since we have copied it (shallow copy)
                else
                    data_detour_avoid_free(pds, &detour_avoid); // free the detour_avoid if it was not copied
            }

            if (err != NE_OK)
                goto errexit;
        }
        else if (nsl_strcmp(name, "traffic-flow") == 0)
        {
            uint32 type = te_getattru(ce, "type");
            uint32 age  = te_getattru(ce, "age");

            if (age == TrafficAgeNew && type == data_nav_route_get_tps_traffic_speed_type(TrafficSpeedHistoric))
                err = err ? err : data_traffic_flow_from_tps(pds, &pnr->historical_traffic_flow, ce);
            else if (age == TrafficAgeNew && type == data_nav_route_get_tps_traffic_speed_type(TrafficSpeedRealTime))
                err = err ? err : data_traffic_flow_from_tps(pds, &pnr->realtime_traffic_flow, ce);
            else if (age == TrafficAgeNew && type == data_nav_route_get_tps_traffic_speed_type(TrafficSpeedStandard))
                err = err ? err : data_traffic_flow_from_tps(pds, &pnr->standard_traffic_flow, ce);
        }
        else if (nsl_strcmp(name, "traffic-incidents") == 0)
        {
            uint32 age  = te_getattru(ce, "age");

            if (age == TrafficAgeNew)
            {
                err = err ? err : data_traffic_incidents_from_tps(pds, &pnr->traffic_incidents, ce);
            }
        }
        else if (nsl_strcmp(name, "route-extents") == 0)
        {
            tpselt box = te_getchild(ce, "box");
            err = err ? err : data_box_from_tps(pds, &pnr->route_extents, box);
            pnr->hasRouteExtents = (boolean)(err == NE_OK);
        }
    }

    ce = te_getchild(te, "extended-content");

    if (ce != NULL)
    {
        err = err ? err : data_extended_content_from_tps(pds, &pnr->extended_content, ce);
    }
    else
    {
        data_extended_content_free(pds, &pnr->extended_content);
    }

    ce = te_getchild(te, "speed-cameras");
    if (ce != NULL)
    {
        // Loop all places/cameras
        iter = 0;
        while ((ce2 = te_nextchild(ce, &iter)) != NULL)
        {
            if (nsl_strcmp(te_getname(ce2), "place") == 0)
            {
                err = err ? err : data_place_from_tps(pds, &place, ce2);
                if (err == NE_OK)
                {
                    if (!CSL_VectorAppend(pnr->vec_cameras, &place))
                        err = NE_NOMEM;

                    if (err == NE_OK)
                        err = data_place_init(pds, &place);
                    else
                        data_place_free(pds, &place); // free the place if it was not copied
                }

                if (err != NE_OK)
                    goto errexit;
            }
        }
    }

    ce = te_getchild(te, "label-point");
    if (ce != NULL)
        err = err ? err : data_label_point_from_tps(pds, &pnr->label_point, ce);
    else
        data_label_point_free(pds, &pnr->label_point);

    ce = te_getchild(te, "route-style");
    if (ce != NULL)
        err = err ? err : data_route_style_from_tps(pds, &pnr->route_style, ce);
    else
        data_route_style_free(pds, &pnr->route_style);

    ce = te_getchild(te, "traffic-record-identifier");
    if (ce != NULL)
        err = err ? err : data_traffic_record_identifier_from_tps(pds, &pnr->traffic_record_identifier, ce);
    else
        data_traffic_record_identifier_free(pds, &pnr->traffic_record_identifier);

    ce = te_getchild(te, "ferry-on-route");
    if (ce != NULL)
    {
        pnr->ferry_on_route = te_getattru(ce, "num-ferries");

        if (pnr->ferry_on_route == 0)
        {
            // if child "ferry-on-route" exist, but have no attribute "num-ferries"
            // it indicates just that ferry(ies) are presented on the route,
            // so ferry_on_route should have positive value
            pnr->ferry_on_route = 1;
        }
    }
    else
    {
        pnr->ferry_on_route = 0;
    }

    if (te_getchild(te, "tolls-on-route"))
    {
        pnr->tolls_on_route = TRUE;
    }

    if (te_getchild(te, "hov-lanes-on-route"))
    {
        pnr->hov_lanes_on_route = TRUE;
    }

    if (te_getchild(te, "highway-on-route"))
    {
        pnr->highway_on_route = TRUE;
    }
    
    if (te_getchild(te, "enter-country-on-route"))
    {
        pnr->enter_coutry_on_route = TRUE;
    }
    
    if (te_getchild(te, "private-road-on-route"))
    {
        pnr->private_road_on_route = TRUE;
    }
    
    if (te_getchild(te, "gated-access-on-route"))
    {
        pnr->gated_access_on_route = TRUE;
    }

    if (te_getchild(te, "unpaved-on-route"))
    {
        pnr->unpaved_on_route = TRUE;
    }

    err = err ? err : data_blob_from_tps_attr(pds, &pnr->route_id, te, "route-id");

    pnr->polyline_length = 0;

    l = CSL_VectorGetLength(pnr->vec_nav_maneuver);

    for (n=0;n<l;n++)
    {
        pman = CSL_VectorGetPointer(pnr->vec_nav_maneuver, n);
        pnr->polyline_length += pman->polyline.length;
    }

    pnr->polyline_complete = FALSE;

    if (l > 0)
        pnr->polyline_complete = data_nav_maneuver_polyline_complete(pds, CSL_VectorGetPointer(pnr->vec_nav_maneuver, l-1));

    ce = te_getchild(te, "sliceres");

    if (ce != NULL)
    {
        pnr->navManeuverStart = te_getattru(ce, "start");
        pnr->navManeuverEnd = te_getattru(ce, "end");
        pnr->navManeuverTotal = te_getattru(ce, "total");
    }

    ce = te_getchild(te, "speed-limits-data");
    if (ce != NULL)
    {
        if (pnr->speed_regions == NULL)
        {
            pnr->speed_regions = (data_speed_limits_data *)nsl_malloc(sizeof(data_speed_limits_data));
            if (pnr->speed_regions == NULL)
            {
                err = NE_NOMEM;
                goto errexit;
            }
            err = data_speed_limits_data_init(pds, pnr->speed_regions);
            if (err != NE_OK) {
                goto errexit;
            }
        }
        err = err ?  err : data_speed_limits_data_from_tps(pds, pnr->speed_regions, ce);
    }

    ce = te_getchild(te, "first-major-road");
    if (ce != NULL)
    {
        if (pnr->first_major_road == NULL)
        {
            pnr->first_major_road = (data_roadinfo*)nsl_malloc(sizeof(data_roadinfo));
            if (pnr->first_major_road == NULL)
            {
                err = NE_NOMEM;
                goto errexit;
            }
            err = data_roadinfo_init(pds, pnr->first_major_road);
            if (err != NE_OK)
            {
                goto errexit;
            }
        }

        ce = te_getchild(ce, "roadinfo");
        if (ce != NULL)
        {
            err = err ? err : data_roadinfo_from_tps(pds, pnr->first_major_road, ce);
            if (err == NE_OK)
            {
                DEBUGLOG(LOG_SS_NB_NAV, LOG_SEV_DEBUG, ("[FNS] First major road:"));
                DEBUGLOG(LOG_SS_NB_NAV, LOG_SEV_DEBUG, ("[FNS] primary: %s", pnr->first_major_road->primary));
                DEBUGLOG(LOG_SS_NB_NAV, LOG_SEV_DEBUG, ("[FNS] secondary: %s", pnr->first_major_road->secondary));
            }
        }
    }

    if (err != NE_OK)
    {
        goto errexit;
    }

    ce = te_getchild(te, "special-regions-data");
    if (ce)
    {
        iter = 0;
        while((ce2 = te_nextchild(ce, &iter)) != NULL)
        {
            if (nsl_strcmp(te_getname(ce2), "special-region") == 0)
            {
                err = err ? err : data_special_region_from_tps(pds, &specialRegion, ce2);
                if (err == NE_OK)
                {
                    if (!CSL_VectorAppend(pnr->vec_special_regions, &specialRegion))
                        err = NE_NOMEM;
                    if (err == NE_OK)
                        data_special_region_init(pds, &specialRegion);
                    else
                        data_special_region_free(pds, &specialRegion);
                }
            }
        }
    }
    else
    {
        CSL_VectorDealloc(pnr->vec_special_regions);
        pnr->vec_special_regions = NULL;
    }


errexit:
    data_nav_maneuver_free(pds, &nav_maneuver);
    data_place_free(pds, &place);
    data_detour_avoid_free(pds, &detour_avoid);

    if (err != NE_OK)
    {
        data_nav_route_free(pds, pnr);
    }

    return err;
}

uint32
data_nav_route_num_nav_maneuver(data_util_state* pds, data_nav_route* pnr)
{
    return (uint32) CSL_VectorGetLength(pnr->vec_nav_maneuver);
}

data_nav_maneuver*
data_nav_route_get_nav_maneuver(data_util_state* pds, data_nav_route* pnr, uint32 index)
{
    if (index >= data_nav_route_num_nav_maneuver(pds, pnr))
        return NULL;

    return (data_nav_maneuver*) CSL_VectorGetPointer(pnr->vec_nav_maneuver, index);
}
