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
 * data_nav_reply.c: created 2005/01/06 by Mark Goddard.
 */

#include "data_nav_reply.h"
#include "csltypes.h"
#include "data_via_point.h"
NB_Error
data_nav_reply_init(data_util_state* pds, data_nav_reply* pnr)
{
    NB_Error err = NE_OK;

    pnr->vec_nav_maneuver = CSL_VectorAlloc(sizeof(data_nav_maneuver));

    if (pnr->vec_nav_maneuver == NULL)
    {
        err = NE_NOMEM;
        goto errexit;
    }

    pnr->vec_file = CSL_VectorAlloc(sizeof(data_file));
    if (pnr->vec_file == NULL)
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

    pnr->vec_detour_avoid = CSL_VectorAlloc(sizeof(data_detour_avoid));

    if (pnr->vec_detour_avoid == NULL)
    {
        err = NE_NOMEM;
        goto errexit;
    }

    pnr->via_point = CSL_VectorAlloc(sizeof(data_via_point));

    if (pnr->via_point == NULL)
    {
        err = NE_NOMEM;
        goto errexit;
    }

    pnr->route_summarys = CSL_VectorAlloc(sizeof(data_route_summary));

    if (pnr->route_summarys == NULL)
    {
        err = NE_NOMEM;
        goto errexit;
    }

    err = data_location_init(pds, &pnr->origin_location);
    err = err ? err : data_location_init(pds, &pnr->destination_location);

    err = err ? err : data_traffic_record_identifier_init(pds, &pnr->traffic_record_identifier);

    err = err ? err : data_traffic_flow_init(pds, &pnr->standard_traffic_flow);
    err = err ? err : data_traffic_flow_init(pds, &pnr->historical_traffic_flow);
    err = err ? err : data_traffic_flow_init(pds, &pnr->realtime_traffic_flow);

    err = err ? err : data_traffic_incidents_init(pds, &pnr->traffic_incidents);
    err = err ? err : data_blob_init(pds, &pnr->route_id);
    err = err ? err : data_box_init(pds, &pnr->route_extents);
    err = err ? err : data_box_init(pds, &pnr->route_extents_all);
    err = err ? err : data_nav_reply_mult_form_init(pds, &pnr->route_selector_reply);

    err = err ? err : data_extended_content_init(pds, &pnr->extended_content);
    if (err != NE_OK)
    {
        goto errexit;
    }

    pnr->speed_regions = (data_speed_limits_data*)nsl_malloc(sizeof(data_speed_limits_data));
    if (pnr->speed_regions == NULL)
    {
        err = NE_NOMEM;
        goto errexit;

    }
    err = err ? err : data_speed_limits_data_init(pds, pnr->speed_regions);

    pnr->first_major_road = (data_roadinfo*)nsl_malloc(sizeof(data_roadinfo));
    if (pnr->first_major_road == NULL)
    {
        err = NE_NOMEM;
        goto errexit;

    }
    err = err ? err : data_roadinfo_init(pds, pnr->first_major_road);
    err = err ? err : data_pronun_list_init(pds, &pnr->pronun_list);

    err = err ? err : data_label_point_init(pds, &pnr->currentLabelPoint);

    pnr->vec_special_regions = CSL_VectorAlloc(sizeof(data_special_region));
    if (!pnr->vec_special_regions)
    {
        err = NE_NOMEM;
        goto errexit;
    }

    pnr->polyline_length = 0;
    pnr->polyline_complete = FALSE;

    pnr->hasRouteExtents = FALSE;
    pnr->hasRouteExtentsAll = FALSE;

    pnr->navManeuverStart = 0;
    pnr->navManeuverEnd = 0xFFFFFF;
    pnr->navManeuverTotal = 0xFFFFFF;

    pnr->active_route = SINGLE_ROUTE;

    pnr->ferry_on_route = 0;
    pnr->tolls_on_route = FALSE;
    pnr->highway_on_route = FALSE;
    pnr->hov_lanes_on_route = FALSE;
    pnr->unpaved_on_route = FALSE;
    pnr->private_road_on_route = FALSE;
    pnr->gated_access_on_route = FALSE;
    pnr->enter_coutry_on_route = FALSE;

errexit:
    if (err != NE_OK)
    {
        data_nav_reply_free(pds, pnr);
    }

    return err;
}

void
data_nav_reply_free(data_util_state* pds, data_nav_reply* pnr)
{
    int n,l;

    if (pnr->vec_nav_maneuver) {
        l = CSL_VectorGetLength(pnr->vec_nav_maneuver);

        for (n=0;n<l;n++)
            data_nav_maneuver_free(pds, (data_nav_maneuver*) CSL_VectorGetPointer(pnr->vec_nav_maneuver, n));

        CSL_VectorDealloc(pnr->vec_nav_maneuver);
    }

    pnr->vec_nav_maneuver = NULL;

    data_nav_reply_discard_files(pds, pnr);

    if (pnr->vec_detour_avoid) {
        l = CSL_VectorGetLength(pnr->vec_detour_avoid);

        for (n=0;n<l;n++)
            data_detour_avoid_free(pds, (data_detour_avoid*) CSL_VectorGetPointer(pnr->vec_detour_avoid, n));

        CSL_VectorDealloc(pnr->vec_detour_avoid);
    }

    pnr->vec_detour_avoid = NULL;

    if (pnr->vec_cameras) {
        l = CSL_VectorGetLength(pnr->vec_cameras);

        for (n=0;n<l;n++)
        {
            data_place_free(pds, (data_place*) CSL_VectorGetPointer(pnr->vec_cameras, n));
        }
        CSL_VectorDealloc(pnr->vec_cameras);
    }
    pnr->vec_cameras = NULL;

    if (pnr->vec_file) {
        l = CSL_VectorGetLength(pnr->vec_file);

        for (n=0;n<l;n++)
        {
            data_file_free(pds, (data_file*) CSL_VectorGetPointer(pnr->vec_file, n));
        }
        CSL_VectorDealloc(pnr->vec_file);
    }
    pnr->vec_file = NULL;

    if (pnr->via_point) {
        l = CSL_VectorGetLength(pnr->via_point);
        for (n=0;n<l;n++)
        {
            data_via_point_free(pds, (data_via_point*) CSL_VectorGetPointer(pnr->via_point, n));
        }
        CSL_VectorDealloc(pnr->via_point);
    }
    pnr->via_point = NULL;

    if (pnr->route_summarys) {
        l = CSL_VectorGetLength(pnr->route_summarys);
        for (n=0;n<l;n++)
        {
            data_route_summary_free(pds, (data_route_summary*) CSL_VectorGetPointer(pnr->route_summarys, n));
        }
        CSL_VectorDealloc(pnr->route_summarys);
    }
    pnr->route_summarys = NULL;


    data_location_free(pds, &pnr->origin_location);
    data_location_free(pds, &pnr->destination_location);

    data_traffic_record_identifier_free(pds, &pnr->traffic_record_identifier);
    data_traffic_flow_free(pds, &pnr->standard_traffic_flow);
    data_traffic_flow_free(pds, &pnr->historical_traffic_flow);
    data_traffic_flow_free(pds, &pnr->realtime_traffic_flow);
    data_traffic_incidents_free(pds, &pnr->traffic_incidents);
    data_box_free(pds, &pnr->route_extents_all);
    data_box_free(pds, &pnr->route_extents);
    data_extended_content_free(pds, &pnr->extended_content);
    data_blob_free(pds, &pnr->route_id);
    data_nav_reply_mult_form_free(pds, &pnr->route_selector_reply);

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
    data_pronun_list_free(pds, &pnr->pronun_list);
    data_label_point_free(pds, &pnr->currentLabelPoint);

    if (pnr->vec_special_regions)
    {
        l = CSL_VectorGetLength(pnr->vec_special_regions);
        for (n = 0; n < l; n++)
        {
            data_special_region_free(pds, (data_special_region*)CSL_VectorGetPointer(pnr->vec_special_regions, n));
        }
        CSL_VectorDealloc(pnr->vec_special_regions);
    }
    pnr->vec_special_regions = NULL;
}

static uint32
data_nav_reply_get_tps_traffic_speed_type(TrafficSpeedType st)
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
data_nav_reply_from_tps(data_util_state* pds, data_nav_reply* pnr, tpselt te, uint32 polyline_pack_passes)
{
    NB_Error err = NE_OK;

    if(!te)
    {
        err = NE_INVAL;
        goto errexit;
    }

    data_nav_reply_free(pds, pnr);
    err = data_nav_reply_init(pds, pnr);
    if (err != NE_OK)
        goto errexit;

    if (te_getchild(te, "nav-route"))
    {
        data_nav_reply_mult_form  mult_reply;

        err = data_nav_reply_mult_form_init(pds, &mult_reply);
        err = err ? err : data_nav_reply_mult_form_from_tps(pds, &mult_reply, te, polyline_pack_passes);
        err = err ? err : data_nav_reply_from_mult_form(pds, &mult_reply, pnr, 0);
        err = err ? err : data_nav_reply_mult_form_copy(pds, &pnr->route_selector_reply, &mult_reply);
        pnr->active_route = 0;
        data_nav_reply_mult_form_free(pds, &mult_reply);
    }
    else
    {
        err = data_nav_reply_single_from_tps(pds, pnr, te, polyline_pack_passes);
        pnr->active_route = SINGLE_ROUTE;
    }
    //via point
    tpselt ce;
    int iter = 0;
    while ((ce = te_nextchild(te, &iter)) != NULL)
    {
        const char * name = te_getname(ce);
        if (nsl_strcmp(name, "waypoint-location") == 0)
        {
            data_via_point viaPoint;
            data_via_point_init(pds, &viaPoint);
            err = err ? err : data_via_point_from_tps(pds, &viaPoint, ce);
            if(err == NE_OK)
            {
                CSL_VectorAppend(pnr->via_point, &viaPoint);
            }
        }
    }

errexit:
    if (err != NE_OK)
    {
        data_nav_reply_free(pds, pnr);
    }

    return err;
}


NB_Error
data_nav_reply_single_from_tps(data_util_state* pds, data_nav_reply* pnr, tpselt te, uint32 polyline_pack_passes)
{
    NB_Error err = NE_OK;
    tpselt    ce, ce2;
    int iter;
    data_nav_maneuver    nav_maneuver;
    data_file            file;
    data_place            place;
    data_detour_avoid    detour_avoid;
    data_special_region  specialRegion;

    int n,l;
    data_nav_maneuver*  pman;

    err = data_nav_maneuver_init(pds, &nav_maneuver);

    if (err != NE_OK)
        return err;

    err = data_file_init(pds, &file);
    if (err != NE_OK)
        return err;

    err = data_place_init(pds, &place);

    if (err != NE_OK)
        return err;

    err = data_special_region_init(pds, &specialRegion);
    if (err != NE_OK)
        return err;

    err = data_detour_avoid_init(pds, &detour_avoid);

    if (err != NE_OK)
        return err;

    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    data_nav_reply_free(pds, pnr);

    err = data_nav_reply_init(pds, pnr);

    if (err != NE_OK)
        return err;

    iter = 0;

    while ((ce = te_nextchild(te, &iter)) != NULL)
    {
        const char * name = te_getname(ce);

        if (nsl_strcmp(name, "nav-maneuver") == 0) {

            err = data_nav_maneuver_from_tps(pds, &nav_maneuver, ce);

            if (err == NE_OK) {
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
        else if (nsl_strcmp(name, "file") == 0) {

            err = data_file_from_tps(pds, &file, ce);

            if (err == NE_OK) {
                if (!CSL_VectorAppend(pnr->vec_file, &file))
                    err = NE_NOMEM;

                if (err == NE_OK)
                    nsl_memset(&file, 0, sizeof(file)); // clear out the file since we have copied it (shallow copy)
                else
                    data_file_free(pds, &file); // free the file if it was not copied
            }

            if (err != NE_OK)
                goto errexit;
        }
        else if (nsl_strcmp(name, "detour-avoid") == 0) {

            err = data_detour_avoid_from_tps(pds, &detour_avoid, ce);

            if (err == NE_OK) {
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

            if (age == TrafficAgeNew && type == data_nav_reply_get_tps_traffic_speed_type(TrafficSpeedHistoric))
                err = err ? err : data_traffic_flow_from_tps(pds, &pnr->historical_traffic_flow, ce);
            else if (age == TrafficAgeNew && type == data_nav_reply_get_tps_traffic_speed_type(TrafficSpeedRealTime))
                err = err ? err : data_traffic_flow_from_tps(pds, &pnr->realtime_traffic_flow, ce);
            else if (age == TrafficAgeNew && type == data_nav_reply_get_tps_traffic_speed_type(TrafficSpeedStandard))
                err = err ? err : data_traffic_flow_from_tps(pds, &pnr->standard_traffic_flow, ce);
        }
        else if (nsl_strcmp(name, "traffic-incidents") == 0) {

            uint32 age  = te_getattru(ce, "age");

            if (age == TrafficAgeNew) {

                err = err ? err : data_traffic_incidents_from_tps(pds, &pnr->traffic_incidents, ce);
            }
        }
        else if (nsl_strcmp(name, "route-extents") == 0)
        {
            tpselt box = te_getchild(ce, "box");
            err = err ? err : data_box_from_tps(pds, &pnr->route_extents, box);
            pnr->hasRouteExtents = (boolean)(err == NE_OK);
        }

        else if (nsl_strcmp(name, "route-summary") == 0)
        {
            data_route_summary routeSummary;
            data_route_summary_init(pds, &routeSummary);
            err = data_route_summary_from_tps(pds, &routeSummary, ce);
            if(err == NE_OK)
            {
                CSL_VectorAppend(pnr->route_summarys, &routeSummary);
            }
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

    ce = te_getchild(te, "origin-location");

    if (ce != NULL)
        err = err ? err : data_location_from_tps(pds, &pnr->origin_location, ce);
    else
        data_location_free(pds, &pnr->origin_location);

    ce = te_getchild(te, "destination-location");

    if (ce != NULL)
        err = err ? err : data_location_from_tps(pds, &pnr->destination_location, ce);
    else
        data_location_free(pds, &pnr->destination_location);

    ce = te_getchild(te, "speed-cameras");
    if (ce != NULL)
    {
        // Loop all places/cameras
        iter = 0;
        while ((ce2 = te_nextchild(ce, &iter)) != NULL) {
            if (nsl_strcmp(te_getname(ce2), "place") == 0) {
                err = err ? err : data_place_from_tps(pds, &place, ce2);
                if (err == NE_OK) {
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

    err = err ? err : data_blob_from_tps_attr(pds, &pnr->route_id, te, "route-id");

    pnr->polyline_length = 0;

    l = CSL_VectorGetLength(pnr->vec_nav_maneuver);

    for (n=0;n<l;n++) {

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
    else
    {
        data_speed_limits_data_free(pds, pnr->speed_regions);
        nsl_free(pnr->speed_regions);
        pnr->speed_regions = NULL;
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

    ce = te_getchild(te, "pronun-list");

    if (ce != NULL)
        err = err ? err : data_pronun_list_from_tps(pds, &pnr->pronun_list, ce);
    else
        data_pronun_list_free(pds, &pnr->pronun_list);

    if (err != NE_OK) {
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
    data_file_free(pds, &file);
    data_place_free(pds, &place);
    data_detour_avoid_free(pds, &detour_avoid);
    data_special_region_free(pds, &specialRegion);

    if (err != NE_OK)
    {
        data_nav_reply_free(pds, pnr);
    }
    return err;
}

NB_Error
data_nav_reply_copy(data_util_state* pds, data_nav_reply* pnr_dest, data_nav_reply* pnr_src)
{
    NB_Error err = NE_OK;
    int n,l;
    data_nav_maneuver    nav_maneuver;
    data_place            nav_camera;
    data_via_point       viapoint;
    data_detour_avoid    detour_avoid;
    data_special_region  special_region;
    data_route_summary   routeSummary;

    data_nav_maneuver_init(pds, &nav_maneuver);
    data_detour_avoid_init(pds, &detour_avoid);
    data_place_init(pds, &nav_camera);
    data_special_region_init(pds, &special_region);
    data_route_summary_init(pds, &routeSummary);

    data_nav_reply_free(pds, pnr_dest);
    data_nav_reply_init(pds, pnr_dest);

    l = CSL_VectorGetLength(pnr_src->vec_nav_maneuver);

    for (n=0;n<l && err == NE_OK;n++) {

        err = err ? err : data_nav_maneuver_copy(pds, &nav_maneuver, CSL_VectorGetPointer(pnr_src->vec_nav_maneuver, n));
        err = err ? err : CSL_VectorAppend(pnr_dest->vec_nav_maneuver, &nav_maneuver) ? NE_OK : NE_NOMEM;

        if (err)
            data_nav_maneuver_free(pds, &nav_maneuver);
        else
            data_nav_maneuver_init(pds, &nav_maneuver);
    }

    l = CSL_VectorGetLength(pnr_src->vec_cameras);
    for (n=0;n<l && err == NE_OK;n++) {

        err = err ? err : data_place_copy(pds, &nav_camera, CSL_VectorGetPointer(pnr_src->vec_cameras, n));
        err = err ? err : CSL_VectorAppend(pnr_dest->vec_cameras, &nav_camera) ? NE_OK : NE_NOMEM;

        if (err)
            data_place_free(pds, &nav_camera);
        else
            data_place_init(pds, &nav_camera);
    }

    l = CSL_VectorGetLength(pnr_src->via_point);
    for (n=0;n<l && err == NE_OK;n++) {
        err = data_via_point_init(pds, &viapoint);
        err = err ? err : data_via_point_copy(pds, &viapoint, CSL_VectorGetPointer(pnr_src->via_point, n));
        err = err ? err : CSL_VectorAppend(pnr_dest->via_point, &viapoint) ? NE_OK : NE_NOMEM;
        if (err)
            data_via_point_free(pds, &viapoint);
    }

    l = CSL_VectorGetLength(pnr_src->route_summarys);
    for (n=0;n<l && err == NE_OK;n++) {
        err = err ? err : data_route_summary_copy(pds, &routeSummary, CSL_VectorGetPointer(pnr_src->route_summarys, n));
        err = err ? err : CSL_VectorAppend(pnr_dest->route_summarys, &routeSummary) ? NE_OK : NE_NOMEM;
        if (err)
        {
            data_route_summary_free(pds, &routeSummary);
        }
        else
        {
            data_route_summary_init(pds, &routeSummary);
        }
    }

    pnr_dest->ferry_on_route = pnr_src->ferry_on_route;

    // Don't copy files

    l = CSL_VectorGetLength(pnr_src->vec_detour_avoid);

    for (n=0;n<l && err == NE_OK;n++) {

        err = err ? err : data_detour_avoid_copy(pds, &detour_avoid, CSL_VectorGetPointer(pnr_src->vec_detour_avoid, n), TRUE);
        err = err ? err : CSL_VectorAppend(pnr_dest->vec_detour_avoid, &detour_avoid) ? NE_OK : NE_NOMEM;

        if (err)
            data_detour_avoid_free(pds, &detour_avoid);
        else
            data_detour_avoid_init(pds, &detour_avoid);
    }

    err = err ? err : data_location_copy(pds, &pnr_dest->origin_location, &pnr_src->origin_location);
    err = err ? err : data_location_copy(pds, &pnr_dest->destination_location, &pnr_src->destination_location);
    err = err ? err : data_traffic_record_identifier_copy(pds, &pnr_dest->traffic_record_identifier, &pnr_src->traffic_record_identifier);
    err = err ? err : data_traffic_flow_copy(pds, &pnr_dest->standard_traffic_flow, &pnr_src->standard_traffic_flow);
    err = err ? err : data_traffic_flow_copy(pds, &pnr_dest->historical_traffic_flow, &pnr_src->historical_traffic_flow);
    err = err ? err : data_traffic_flow_copy(pds, &pnr_dest->realtime_traffic_flow, &pnr_src->realtime_traffic_flow);
    err = err ? err : data_traffic_incidents_copy(pds, &pnr_dest->traffic_incidents, &pnr_src->traffic_incidents);
    err = err ? err : data_blob_copy(pds, &pnr_dest->route_id, &pnr_src->route_id);
    err = err ? err : data_box_copy(pds, &pnr_dest->route_extents, &pnr_src->route_extents);
    err = err ? err : data_box_copy(pds, &pnr_dest->route_extents_all, &pnr_src->route_extents_all);
    err = err ? err : data_extended_content_copy(pds, &pnr_dest->extended_content, &pnr_src->extended_content);
    err = err ? err : data_nav_reply_mult_form_copy(pds, &pnr_dest->route_selector_reply, &pnr_src->route_selector_reply);
    if (pnr_src->first_major_road != NULL)
    {
        err = err ? err : data_roadinfo_copy(pds, pnr_dest->first_major_road, pnr_src->first_major_road);
    }
    if (pnr_src->speed_regions != NULL) {
        err = err ? err : data_speed_limits_data_copy(pds, pnr_dest->speed_regions, pnr_src->speed_regions);
    }
    else
    {
        if (pnr_dest->speed_regions != NULL)
        {
            data_speed_limits_data_free(pds, pnr_dest->speed_regions);
            nsl_free(pnr_dest->speed_regions);
            pnr_dest->speed_regions = NULL;
        }
    }
    err = err ? err : data_pronun_list_copy(pds, &pnr_dest->pronun_list, &pnr_src->pronun_list);
    err = err ? err : data_label_point_copy(pds, &pnr_dest->currentLabelPoint, &pnr_src->currentLabelPoint);

    pnr_dest->hasRouteExtents = pnr_src->hasRouteExtents;
    pnr_dest->hasRouteExtentsAll = pnr_src->hasRouteExtentsAll;
    pnr_dest->polyline_length = pnr_src->polyline_length;
    pnr_dest->polyline_complete = pnr_src->polyline_complete;

    pnr_dest->active_route = pnr_src->active_route;

    pnr_dest->route_settings = pnr_src->route_settings;

    // free the scratch maneuver
    data_nav_maneuver_free(pds, &nav_maneuver);
    // free the scratch camera
    data_place_free(pds, &nav_camera);
    // free the scratch detour
    data_detour_avoid_free(pds, &detour_avoid);

    if (pnr_src->vec_special_regions)
    {
        l = CSL_VectorGetLength(pnr_src->vec_special_regions);
        for (n=0;n<l && err == NE_OK;n++)
        {
            err = err ? err : data_special_region_copy(pds, &special_region, CSL_VectorGetPointer(pnr_src->vec_special_regions, n));
            err = err ? err : CSL_VectorAppend(pnr_dest->vec_special_regions, &special_region) ? NE_OK : NE_NOMEM;

            if (err)
                data_special_region_free(pds, &special_region);
            else
                data_special_region_init(pds, &special_region);
        }
    }
    else
    {
        CSL_VectorDealloc(pnr_dest->vec_special_regions);
        pnr_dest->vec_special_regions = NULL;
    }

    return err;
}

uint32
data_nav_reply_num_nav_maneuver(data_util_state* pds, data_nav_reply* pnr)
{
    return (uint32) CSL_VectorGetLength(pnr->vec_nav_maneuver);
}

uint32
data_nav_reply_num_nav_camera(data_util_state* pds, data_nav_reply* pnr)
{
    return (uint32) CSL_VectorGetLength(pnr->vec_cameras);
}

data_nav_maneuver*
data_nav_reply_get_nav_maneuver(data_util_state* pds, data_nav_reply* pnr, uint32 index)
{
    if (index >= data_nav_reply_num_nav_maneuver(pds, pnr))
        return NULL;

    return (data_nav_maneuver*) CSL_VectorGetPointer(pnr->vec_nav_maneuver, index);
}

data_place*
data_nav_reply_get_nav_camera(data_util_state* pds, data_nav_reply* pnr, uint32 index)
{
    if (index >= data_nav_reply_num_nav_camera(pds, pnr))
        return NULL;

    return (data_place*) CSL_VectorGetPointer(pnr->vec_cameras, index);
}

NB_Error
data_nav_reply_last_point(data_util_state* pds, data_nav_reply* pnr, double* plat, double* plon)
{
    uint32 nman;
    uint32 numman = data_nav_reply_num_nav_maneuver(pds, pnr);
    uint32 pllen;
    data_nav_maneuver* pman;

    if (plat == NULL || plon == NULL)
        return NE_INVAL;

    *plat = *plon = INVALID_LATLON;

    for (nman = 0; nman < numman; nman++) {

        pman = data_nav_reply_get_nav_maneuver(pds, pnr, nman);

        if (pman != NULL) {

            pllen = data_polyline_len(pds, &pman->polyline);

            if (pllen > 0)
                data_polyline_get(pds, &pman->polyline, pllen-1, plat, plon, NULL, NULL);
        }
    }

    return NE_OK;
}

void
data_nav_reply_file_foreach(data_util_state* pds, data_nav_reply* pnr,
                void (*cb)(data_file* pf, void* pUser), void* pUser)
{
    data_file* f;
    int n,l;

    l = CSL_VectorGetLength(pnr->vec_file);

    for (n=0; n<l; n++) {

        f = (data_file*) CSL_VectorGetPointer(pnr->vec_file, n);

        if (cb)
            cb(f, pUser);
    }
}

void
data_nav_reply_discard_files(data_util_state* pds, data_nav_reply* pnr)
{
    int n,l;

    if (pnr->vec_file) {
        l = CSL_VectorGetLength(pnr->vec_file);

        for (n=0;n<l;n++)
            data_file_free(pds, (data_file*) CSL_VectorGetPointer(pnr->vec_file, n));

        CSL_VectorDealloc(pnr->vec_file);
    }

    pnr->vec_file = NULL;
}

NB_Error
data_nav_reply_get_speed(data_util_state* pds, const char* tmcloc, TrafficSpeedType speed_type, double minspeed,
                         double* pspeed, char* pcolor, double* pfreeflowspeed,
                         TrafficSpeedType* pspeed_type_out, data_nav_reply* pnr)
{
    NB_Error err = NE_OK;

    if (speed_type & TrafficSpeedRealTime) {

        err = data_traffic_flow_get_speed(pds, &pnr->realtime_traffic_flow, minspeed, tmcloc, pspeed, pcolor, pfreeflowspeed);

        if (err == NE_OK && pspeed_type_out != NULL) {

            *pspeed_type_out = TrafficSpeedRealTime;
            goto exit;
        }
    }

    if ((speed_type & TrafficSpeedHistoric)) {

        err = data_traffic_flow_get_speed(pds, &pnr->historical_traffic_flow, minspeed, tmcloc, pspeed, pcolor, pfreeflowspeed);

        if (err == NE_OK && pspeed_type_out != NULL) {

            *pspeed_type_out = TrafficSpeedHistoric;
            goto exit;
        }
    }

    if ((speed_type & TrafficSpeedStandard)) {

        err = data_traffic_flow_get_speed(pds, &pnr->standard_traffic_flow, minspeed, tmcloc, pspeed, pcolor, pfreeflowspeed);

        if (err == NE_OK && pspeed_type_out != NULL) {

            *pspeed_type_out = TrafficSpeedStandard;
            goto exit;
        }
    }

exit:
    return err;
}


#define THRESHOLD_TOLERANCE 0.0001

NB_Error
data_nav_reply_get_traffic_info(data_util_state* pds,
                                data_nav_reply* pnr,
                                double minspeed,
                                uint32 min_no_data_disable_traffic_bar,
                                uint32 start,
                                double start_offset,
                                uint32 end,
                                TrafficSpeedType speed_type,
                                uint32* pdelay,
                                double* pmeter,
                                uint32* ptotal_time,
                                nav_speed_cb speed_cb,
                                void* speed_user)
{
    NB_Error err = NE_OK;
    data_nav_maneuver* pman;
    data_traffic_region* ptr;
    uint32 delay = 0;
    uint32 nman;
    int n,l;
    double speed = -1;
    double ffspeed = -1;
    uint32 time;
    uint32 fftime;
    double length;
    char color;
    double total_meter_length = 0;
    double nodata_meter_length = 0;
    double meter_sum = 0;
    double tr_start;
    double tr_end;
    double ratio;
    TrafficSpeedType speed_type_out = TrafficSpeedNone;
    const char* tmcloc;
    uint32 total_time = 0;
    double delay_percent;
    TrafficSpeedType eff_speed_type;

    if (speed_cb == NULL)
    {
        return NE_INVAL;
    }

    if (start == NAV_REPLY_MANEUVER_NONE ||
        start == NAV_REPLY_MANEUVER_START)
    {
        start = 0;
    }

    if (end == NAV_REPLY_MANEUVER_START)
    {
        end = 1;
    }

    if (end == NAV_REPLY_MANEUVER_NONE)
    {
        end = data_nav_reply_num_nav_maneuver(pds, pnr);
    }

    if (start == end)
    {
        /* If the request is for a single maneuver, increment end maneuver */
        end++;
    }

    for (nman = start; nman < end && err == NE_OK; nman++)
    {
        eff_speed_type = speed_type;

        // This function return NULL if the range is wrong
        pman = data_nav_reply_get_nav_maneuver(pds, pnr, nman);
        if (!pman)
        {
            return NE_INVAL;
        }

        l = CSL_VectorGetLength(pman->vec_traffic_regions);
        if (l <= 0)
        {
            /* there is no traffic region, traffic is disabled, use average speed in maneuver */
            length = pman->distance;
            speed = pman->speed;

            if (nman == start)
            {
                /* for current maneuver */
                length -= start_offset;
            }

            if (length < 0)
            {
                /* should never happen, but just in case */
                length = 0.0;
            }

            if (speed < minspeed)
            {
                speed = minspeed;
            }

            total_time += (uint32) (length / speed);
            total_meter_length += length;
            nodata_meter_length += length;
        }
        else
        {

            for (n=0; n<l && err == NE_OK; n++)
            {

                ptr = CSL_VectorGetPointer(pman->vec_traffic_regions, n);

                length = ptr->length;

                if (nman == start) {

                    tr_start = ptr->start;
                    tr_end = tr_start + ptr->length;

                    if (start_offset > tr_end)
                        continue;
                    else if (start_offset > tr_start)
                        length = tr_end - start_offset;
                }

                tmcloc = data_string_get(pds, &ptr->location);

                err = speed_cb(pds, tmcloc, eff_speed_type, minspeed, &speed, &color, &ffspeed, &speed_type_out, speed_user);
                if (err == NE_OK)
                {
                    time   = (uint32) (length / speed);
                    fftime = (uint32) (length / ffspeed);

                    if (time > fftime)
                    {
                        delay += (uint32) (time - fftime);
                    }

                    if (ffspeed >= 0)
                    {
                        ratio = 100.0 * speed / ffspeed;
                    }
                    else
                    {
                        ratio = 100.0;
                    }

                    if (ratio > 100.0)
                    {
                        ratio = 100.0;
                    }

                    meter_sum += length * ratio;
                    total_meter_length += length;

                    if (speed_type_out != TrafficSpeedRealTime &&
                        speed_type_out != TrafficSpeedHistoric)
                    {
                        nodata_meter_length += length;
                    }

                    total_time += time;
                }
                else if (err == NE_NOENT)
                {
                    err = NE_OK;

                    // treat areas with no traffic data as free flowing, but track the
                    // total length with no data

                    meter_sum += length * 100.0;
                    total_meter_length += length;
                    nodata_meter_length += length;
                }
            }
        }
    }

    if (total_time == 0)
    {
        err = NE_NOENT;
        delay_percent = 0;
    }
    else if ((total_time - delay) == 0)
    {
        delay_percent = 100;
    }
    else if (delay < 30)
    {
        delay_percent = 0;
    }
    else
    {
        delay_percent = delay * 100.0 / (total_time - delay);
    }

    if (pdelay)
    {
        *pdelay = delay;
    }

    if (pmeter)
    {
        *pmeter = 100 - ((delay_percent > 100) ? 100 : delay_percent);
    }

    if (((100*nodata_meter_length) / total_meter_length) >= (min_no_data_disable_traffic_bar - THRESHOLD_TOLERANCE) && pmeter)
    {
        *pmeter = -1.0;
    }

    if (ptotal_time)
    {
        *ptotal_time = total_time;
    }

    return err;
}

NB_Error
data_nav_reply_select_active(data_util_state* pds, data_nav_reply* pnr, uint32 activeRoute)
{
    NB_Error err = NE_OK;
    int alternateRoutes = CSL_VectorGetLength(pnr->route_selector_reply.vec_routes);
    data_nav_reply  new_route_reply;

    if (pnr->active_route == SINGLE_ROUTE || activeRoute > (uint32)alternateRoutes)
    {
        return NE_RANGE;
    }

    err = data_nav_reply_init(pds, &new_route_reply);
    err = err ? err : data_nav_reply_from_mult_form(pds, &pnr->route_selector_reply, &new_route_reply, activeRoute);
    err = err ? err : data_nav_reply_mult_form_copy(pds, &new_route_reply.route_selector_reply, &pnr->route_selector_reply);
    new_route_reply.route_settings = pnr->route_settings;
    err = err ? err : data_nav_reply_copy(pds, pnr, &new_route_reply);
    data_nav_reply_free(pds, &new_route_reply);

    if (err == NE_OK)
    {
        pnr->active_route = activeRoute;
    }

    return err;
}

NB_Error
data_nav_reply_get_active(data_util_state* pds, data_nav_reply* pnr, uint32* activeRoute)
{
    NB_Error err = NE_OK;

    if (!pnr || !activeRoute)
    {
        return NE_INVAL;
    }
    else if( pnr->active_route == SINGLE_ROUTE )
    {
        return NE_BADDATA;
    }

    *activeRoute = pnr->active_route;
    return err;
}

NB_Error
data_nav_reply_get_route_count(data_util_state* pds, data_nav_reply* pnr, uint32* routeCount)
{
    if (routeCount)
    {
        if (pnr->active_route == SINGLE_ROUTE)
        {
            *routeCount = 1; // if no alternate routes presented - it means that we have just single route
        }
        else
        {
            *routeCount = (uint32)CSL_VectorGetLength(pnr->route_selector_reply.vec_routes);
        }

        return NE_OK;
    }

    return NE_INVAL;
}

data_nav_route*
data_nav_reply_get_nav_route(data_util_state* pds, data_nav_reply* pnr, uint32 index)
{
    uint32 routeCount = 0;
    if (pnr->active_route == SINGLE_ROUTE)
    {
        return NULL;
    }

    data_nav_reply_get_route_count(pds, pnr, &routeCount);
    if (index >= routeCount)
        return NULL;

    return (data_nav_route*) CSL_VectorGetPointer(pnr->route_selector_reply.vec_routes, index);
}

NB_Error
data_nav_reply_get_single_nav_reply(data_util_state* pds, data_nav_reply* pnr, uint32 index, data_nav_reply* psnr)
{
    NB_Error err = NE_OK;
    int alternateRoutes = CSL_VectorGetLength(pnr->route_selector_reply.vec_routes);

    if (pnr->active_route == SINGLE_ROUTE)
    {
        err = err ? err : data_nav_reply_copy(pds, psnr, pnr);
        psnr->route_settings = pnr->route_settings;
        psnr->active_route = SINGLE_ROUTE;
        return err;
    }

    if (index > (uint32)alternateRoutes)
    {
        return NE_RANGE;
    }

    err = err ? err : data_nav_reply_from_mult_form(pds, &pnr->route_selector_reply, psnr, index);

    data_via_point       viapoint;
    data_special_region  special_region;
    data_route_summary   routeSummary;
    data_special_region_init(pds, &special_region);
    data_route_summary_init(pds, &routeSummary);
    uint32 length = CSL_VectorGetLength(pnr->via_point);
    for (uint32 n=0;n<length && err == NE_OK;n++)
    {
        err = err ? err : data_via_point_init(pds, &viapoint);
        err = err ? err : data_via_point_copy(pds, &viapoint, CSL_VectorGetPointer(pnr->via_point, n));
        err = err ? err : CSL_VectorAppend(psnr->via_point, &viapoint) ? NE_OK : NE_NOMEM;
        if (err)
        data_via_point_free(pds, &viapoint);
    }
    
    length = CSL_VectorGetLength(pnr->route_summarys);
    for (uint32 n=0;n<length && err == NE_OK;n++)
    {
        err = err ? err : data_route_summary_copy(pds, &routeSummary, CSL_VectorGetPointer(pnr->route_summarys, n));
        err = err ? err : CSL_VectorAppend(psnr->route_summarys, &routeSummary) ? NE_OK : NE_NOMEM;
        if (err)
        {
            data_route_summary_free(pds, &routeSummary);
        }
        else
        {
            data_route_summary_init(pds, &routeSummary);
        }
    }
    if (pnr->vec_special_regions)
    {
        length = CSL_VectorGetLength(pnr->vec_special_regions);
        for (uint32 n=0;n<length && err == NE_OK;n++)
        {
            err = err ? err : data_special_region_copy(pds, &special_region, CSL_VectorGetPointer(pnr->vec_special_regions, n));
            err = err ? err : CSL_VectorAppend(psnr->vec_special_regions, &special_region) ? NE_OK : NE_NOMEM;
            
            if (err)
            data_special_region_free(pds, &special_region);
            else
            data_special_region_init(pds, &special_region);
        }
    }
    else
    {
        CSL_VectorDealloc(psnr->vec_special_regions);
        psnr->vec_special_regions = NULL;
    }
    psnr->route_settings = pnr->route_settings;
    psnr->active_route = SINGLE_ROUTE;

    return err;
}

NB_Error
data_nav_reply_get_on_route_info(data_util_state* pds, data_nav_reply* pnr, data_on_route_information* onRouteInfo)
{
    NB_Error err = NE_NOENT;

    if (pnr->active_route == SINGLE_ROUTE)
    {
        // in this case we have no route information from server
        onRouteInfo->isFerryOnRoute = (boolean)(pnr->ferry_on_route > 0);
        onRouteInfo->isHighWayOnRoute = pnr->highway_on_route;
        onRouteInfo->isHovLanesOnRoute = pnr->hov_lanes_on_route;
        onRouteInfo->isTollsOnRoute = pnr->tolls_on_route;
        onRouteInfo->isUnpavedOnRoute = pnr->unpaved_on_route;
        onRouteInfo->isPrivateRoadOnRoute = pnr->private_road_on_route;
        onRouteInfo->isGatedAccessOnRoute = pnr->gated_access_on_route;
        onRouteInfo->isEnterCoutryOnRoute = pnr->enter_coutry_on_route;
        err = NE_OK;
    }
    else
    {
        data_nav_route* current_route = CSL_VectorGetPointer(pnr->route_selector_reply.vec_routes, pnr->active_route);

        if (current_route)
        {
            onRouteInfo->isFerryOnRoute = (boolean)(current_route->ferry_on_route > 0);
            onRouteInfo->isHighWayOnRoute = current_route->highway_on_route;
            onRouteInfo->isHovLanesOnRoute = current_route->hov_lanes_on_route;
            onRouteInfo->isTollsOnRoute = current_route->tolls_on_route;
            onRouteInfo->isUnpavedOnRoute = current_route->unpaved_on_route;
            onRouteInfo->isPrivateRoadOnRoute = current_route->private_road_on_route;
            onRouteInfo->isGatedAccessOnRoute = current_route->gated_access_on_route;
            onRouteInfo->isEnterCoutryOnRoute = current_route->enter_coutry_on_route;
            err =  NE_OK;
        }
    }

    return err;
}

NB_Error
data_nav_reply_get_label_point(data_util_state* pds, data_nav_reply* pnr, data_point* point)
{
    NB_Error err = NE_OK;
    data_nav_route* current_route = NULL;

    DATA_REINIT(pds, err, point, data_point);

    if (pnr->active_route == SINGLE_ROUTE)
    {
        data_point_copy(pds, point, &pnr->currentLabelPoint.point);
        return err;
    }

    current_route = (data_nav_route*)CSL_VectorGetPointer(pnr->route_selector_reply.vec_routes, pnr->active_route);

    if (current_route)
    {
        err = data_point_copy(pds, point, &current_route->label_point.point);
    }
    else
    {
        err = NE_NOENT;
    }

    return err;
}

NB_Error
data_nav_reply_get_current_label_point(data_util_state* pds, data_nav_reply* pnr, data_point* point)
{
    NB_Error err = NE_OK;

    if (pnr->active_route == SINGLE_ROUTE)
    {
        return NE_NOENT;
    }

    DATA_REINIT(pds, err, point, data_point);

    err = data_point_copy(pds, point, &pnr->route_selector_reply.current_label_point.point);

    return err;
}

NB_Error
data_nav_reply_from_mult_form(data_util_state* pds,
                              data_nav_reply_mult_form* pnr,
                              data_nav_reply* pdest,
                              uint32 routeIdx)
{
    NB_Error err = NE_OK;
    data_nav_route* nav_route = NULL;

    data_nav_reply_free(pds, pdest);
    err = data_nav_reply_init(pds, pdest);
    if (err != NE_OK)
        goto errexit;

    if ((uint32)CSL_VectorGetLength(pnr->vec_routes) < routeIdx || pnr->route_error)
    {
        err = NE_RANGE;
        goto errexit;
    }

    nav_route = (data_nav_route*)CSL_VectorGetPointer(pnr->vec_routes, (int)routeIdx);

    if(!nav_route)
    {
        err = NE_NOENT;
        goto errexit;
    }

    DATA_VEC_COPY(pds, err, pdest->vec_file, pnr->vec_file, data_file);
    DATA_VEC_COPY(pds, err, pdest->vec_nav_maneuver, nav_route->vec_nav_maneuver, data_nav_maneuver);
    DATA_VEC_COPY_EX(pds, err, pdest->vec_detour_avoid, nav_route->vec_detour_avoid, data_detour_avoid, TRUE);
    DATA_VEC_COPY(pds, err, pdest->vec_cameras, nav_route->vec_cameras, data_place);
    DATA_VEC_COPY(pds, err, pdest->vec_special_regions, nav_route->vec_special_regions, data_special_region);

    err = err ? err : data_box_copy(pds, &pdest->route_extents, &nav_route->route_extents);
    err = err ? err : data_location_copy(pds, &pdest->origin_location, &pnr->origin_location);
    err = err ? err : data_location_copy(pds, &pdest->destination_location, &pnr->destination_location);
    err = err ? err : data_blob_copy(pds, &pdest->route_id, &nav_route->route_id);
    err = err ? err : data_extended_content_copy(pds, &pdest->extended_content, &nav_route->extended_content);
    err = err ? err : data_speed_limits_data_copy(pds, pdest->speed_regions, nav_route->speed_regions);
    err = err ? err : data_roadinfo_copy(pds, pdest->first_major_road, nav_route->first_major_road);
    err = err ? err : data_traffic_incidents_copy(pds, &pdest->traffic_incidents, &nav_route->traffic_incidents);
    err = err ? err : data_traffic_record_identifier_copy(pds, &pdest->traffic_record_identifier, &nav_route->traffic_record_identifier);
    err = err ? err : data_traffic_flow_copy(pds, &pdest->standard_traffic_flow, &nav_route->standard_traffic_flow);
    err = err ? err : data_traffic_flow_copy(pds, &pdest->historical_traffic_flow, &nav_route->historical_traffic_flow);
    err = err ? err : data_traffic_flow_copy(pds, &pdest->realtime_traffic_flow, &nav_route->realtime_traffic_flow);
    err = err ? err : data_box_copy(pds, &pdest->route_extents_all, &pnr->route_extents);
    err = err ? err : data_label_point_copy(pds, &pdest->currentLabelPoint, &nav_route->label_point);

    pdest->hasRouteExtentsAll = (boolean)(err == NE_OK);
    pdest->hasRouteExtents = nav_route->hasRouteExtents;
    pdest->ferry_on_route = nav_route->ferry_on_route;
    pdest->polyline_length = nav_route->polyline_length;
    pdest->polyline_complete = nav_route->polyline_complete;
    pdest->navManeuverStart = nav_route->navManeuverStart;
    pdest->navManeuverEnd = nav_route->navManeuverEnd;
    pdest->navManeuverTotal = nav_route->navManeuverTotal;
    pdest->tolls_on_route = nav_route->tolls_on_route;
    pdest->hov_lanes_on_route = nav_route->hov_lanes_on_route;
    pdest->highway_on_route = nav_route->highway_on_route;
    pdest->unpaved_on_route = nav_route->unpaved_on_route;
    pdest->private_road_on_route = nav_route->private_road_on_route;
    pdest->gated_access_on_route = nav_route->gated_access_on_route;
    pdest->enter_coutry_on_route = nav_route->enter_coutry_on_route;

    pdest->route_settings = pnr->route_settings;

errexit:
    if (err != NE_OK)
    {
        data_nav_reply_free(pds, pdest);
    }

    return err;
}
