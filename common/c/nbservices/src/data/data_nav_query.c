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
 * data_nav_query.c: created 2005/01/06 by Mark Goddard.
 */

#include "data_nav_query.h"
#include "tpsdebug.h"
#include "data_via_point.h"

static tpselt data_nav_query_want_maneuver(data_util_state* pds, nb_boolean wantAllManeuvers);
static tpselt data_nav_query_recreate_route_id(data_util_state* pds, data_nav_query* pnq);
static boolean AttachSimpleWantElement(boolean attach, const char* name, tpselt parent);

static void
data_nav_query_clear(data_util_state* pds, data_nav_query* pnq)
{
    pnq->command_set_version = 0;
    pnq->want_origin_location = FALSE;
    pnq->want_destination_location = FALSE;
    pnq->has_recalc = FALSE;
    pnq->disable_navigation = TRUE;
    pnq->want_route_map = FALSE;
    pnq->want_cross_streets = FALSE;
    pnq->want_maneuvers = FALSE;
    pnq->want_all_maneuvers = FALSE;
    pnq->route_map_angle_of_view = 0;
    pnq->route_map_distance_of_view = 0;
    pnq->route_map_min_time = 0;
    pnq->route_map_max_tiles = 0;

    pnq->want_historical_traffic = 0;
    pnq->want_realtime_traffic = 0;
    pnq->want_speed_cameras = 0;

    pnq->want_ferry_maneuvers    = FALSE;
    pnq->want_unpaved_info = FALSE;
    pnq->want_towards_roadinfo = FALSE;
    pnq->want_intersection_roadinfo = FALSE;
    pnq->want_country_info = FALSE;
    pnq->want_enter_country_maneuvers = FALSE;
    pnq->want_bridge_maneuvers = FALSE;
    pnq->want_tunnel_maneuvers = FALSE;
    pnq->want_enter_roundabout_maneuvers = FALSE;
    pnq->want_route_extents = FALSE;
    pnq->want_realistic_road_signs = FALSE;
    pnq->want_junction_models = FALSE;
    pnq->want_city_models = FALSE;
    pnq->want_speed_regions = FALSE;
    pnq->want_first_major_road = TRUE;
    pnq->want_label_point = FALSE;
    pnq->want_non_guidance_maneuvers = FALSE;
    pnq->want_complete_route = FALSE;
    pnq->want_max_turn_for_street_count = FALSE;
    pnq->want_route_summary = FALSE;
    pnq->want_special_regions = FALSE;
    pnq->want_first_maneuver_cross_streets = FALSE;
    pnq->want_secondary_name_for_exits = FALSE;
    pnq->want_check_unsupported_countries = FALSE;
    pnq->want_pedestrian_aneuvers = TRUE;
    pnq->start_to_navigate = FALSE;
    pnq->want_route_features = TRUE;
    pnq->want_entry_indicators = TRUE;

    pnq->max_pronun_files = 0;
}

NB_Error
data_nav_query_init(data_util_state* pds, data_nav_query* pnq)
{
    NB_Error err = NE_OK;

    err = err ? err : data_origin_init(pds, &pnq->origin);
    err = err ? err : data_destination_init(pds, &pnq->destination);
    err = err ? err : data_cache_contents_init(pds, &pnq->cache_contents);
    err = err ? err : data_map_cache_contents_init(pds, &pnq->map_cache_contents);
    err = err ? err : data_route_style_init(pds, &pnq->route_style);
    err = err ? err : data_recalc_init(pds, &pnq->recalc);
    err = err ? err : data_string_init(pds, &pnq->pronun_style);
    err = err ? err : data_string_init(pds, &pnq->language);
    err = err ? err : data_blob_init(pds, &pnq->recreate_route_id);
    err = err ? err : data_want_guidance_config_init(pds, &pnq->want_guidance_config);
    err = err ? err : data_want_alternate_routes_init(pds, &pnq->want_alternate_routes);
    err = err ? err : data_want_extended_pronun_data_init(pds, &pnq->want_extended_pronun_data);

    pnq->vec_detour_avoid = CSL_VectorAlloc(sizeof(data_detour_avoid));
    if (pnq->vec_detour_avoid == NULL)
    {
        err = NE_NOMEM;
    }

    pnq->via_point = CSL_VectorAlloc(sizeof(data_via_point));
    if (pnq->via_point == NULL)
    {
        err = NE_NOMEM;
    }

    pnq->extra_destination = CSL_VectorAlloc(sizeof(data_destination));
    if (pnq->extra_destination == NULL)
    {
        err = NE_NOMEM;
    }

    data_nav_query_clear(pds, pnq);

    if (err)
    {
        data_nav_query_free(pds, pnq);
    }

    return err;
}

void
data_nav_query_free(data_util_state* pds, data_nav_query* pnq)
{
    int n, l;
    data_origin_free(pds, &pnq->origin);
    data_destination_free(pds, &pnq->destination);
    data_cache_contents_free(pds, &pnq->cache_contents);
    data_map_cache_contents_free(pds, &pnq->map_cache_contents);
    data_route_style_free(pds, &pnq->route_style);
    data_recalc_free(pds, &pnq->recalc);
    data_string_free(pds, &pnq->pronun_style);
    data_string_free(pds, &pnq->language);
    data_blob_free(pds, &pnq->recreate_route_id);
    data_want_alternate_routes_free(pds, &pnq->want_alternate_routes);
    data_want_extended_pronun_data_free(pds, &pnq->want_extended_pronun_data);

    if (pnq->vec_detour_avoid) {
        l = CSL_VectorGetLength(pnq->vec_detour_avoid);

        for (n=0;n<l;n++)
            data_detour_avoid_free(pds, (data_detour_avoid*) CSL_VectorGetPointer(pnq->vec_detour_avoid, n));

        CSL_VectorDealloc(pnq->vec_detour_avoid);
    }
    pnq->vec_detour_avoid = NULL;
    if (pnq->via_point) {
        l = CSL_VectorGetLength(pnq->via_point);

        for (n=0;n<l;n++)
            data_via_point_free(pds, (data_via_point*) CSL_VectorGetPointer(pnq->via_point, n));

        CSL_VectorDealloc(pnq->via_point);
    }
    pnq->via_point = NULL;

    if (pnq->extra_destination) {
        l = CSL_VectorGetLength(pnq->extra_destination);

        for (n=0;n<l;n++)
            data_destination_free(pds, (data_destination*) CSL_VectorGetPointer(pnq->extra_destination, n));

        CSL_VectorDealloc(pnq->extra_destination);
    }
    pnq->extra_destination = NULL;

    data_nav_query_clear(pds, pnq);
}


void data_nav_add_via_point(data_util_state* pds, CSL_Vector* viapoint, tpselt parent)
{
    size_t i = 0;
    tpselt te;
    tpselt tchild;
    data_via_point* via;

    size_t length = CSL_VectorGetLength(viapoint);
    if(length > 0)
    {
        for(i = 0; i < length; i++)
        {
            via = (data_via_point*)CSL_VectorGetPointer(viapoint, i);
            te = data_via_point_to_tps(pds, via);
            if(te)
            {
                te_attach(parent, te);
            }
        }
    }
}

tpselt
data_nav_query_to_tps(data_util_state* pds, data_nav_query* pnq)
{
    tpselt te;
    tpselt ce = NULL;
    int n,l;

    te = te_new("nav-query");

    if (te == NULL)
        goto errexit;

    if (pnq->recreate_route_id.size > 0)
    {
        if ((ce = data_nav_query_recreate_route_id(pds, pnq)) != NULL && te_attach(te, ce))
            ce = NULL;
        else
            goto errexit;
    }
    else
    {
        if ((ce = data_origin_to_tps(pds, &pnq->origin)) != NULL && te_attach(te, ce))
            ce = NULL;
        else
            goto errexit;

        if ((ce = data_destination_to_tps(pds, &pnq->destination)) != NULL && te_attach(te, ce))
            ce = NULL;
        else
            goto errexit;
        if (pnq->extra_destination)
        {
            l = CSL_VectorGetLength(pnq->extra_destination);
            for(n=0; n<l; n++)
            {
                data_destination* dest = (data_destination*) CSL_VectorGetPointer(pnq->extra_destination, n);
                if((ce = data_destination_to_tps(pds, dest)) != NULL && te_attach(te, ce))
                {
                    ce = NULL;
                }
                else
                {
                    goto errexit;
                }
            }
        }
    }

    if (pnq->via_point)
    {
        data_nav_add_via_point(pds, pnq->via_point, te);
    }
    if (!te_setattrc(te, "pronun-style", data_string_get(pds, &pnq->pronun_style)))
        goto errexit;

    if (!te_setattru(te, "command-set-version", pnq->command_set_version))
        goto errexit;

    if (!nsl_strempty(data_string_get(pds, &pnq->language)))
    {
        if (!te_setattrc(te, "language", data_string_get(pds, &pnq->language)))
            goto errexit;
    }

    if (!te_setattru(te, "max-pronun-files", pnq->max_pronun_files))
        goto errexit;

    if (pnq->want_route_summary)
    {
        if (!AttachSimpleWantElement(pnq->want_route_summary, "want-route-summary", te))
        {
            goto errexit;
        }
        if (pnq->want_route_summary_delaytime)
        {
            if (!AttachSimpleWantElement(pnq->want_route_summary_delaytime, "want-route-summary-delay-time", te))
            {
                goto errexit;
            }
        }
        return te;
    }

    if (!nsl_strempty(data_string_get(pds, &pnq->want_extended_pronun_data.supported_phonetics_formats)))
    {
        if ((ce = data_want_extended_pronun_data_to_tps(pds, &pnq->want_extended_pronun_data)) != NULL && te_attach(te, ce))
            ce = NULL;
        else
            goto errexit;
    }

    if ((ce = data_map_cache_contents_to_tps(pds, &pnq->map_cache_contents)) != NULL && te_attach(te, ce))
        ce = NULL;
    else
        goto errexit;

    if ((ce = data_route_style_to_tps(pds, &pnq->route_style)) != NULL && te_attach(te, ce))
        ce = NULL;
    else
        goto errexit;

    if (pnq->has_recalc && !pnq->start_to_navigate) {

        if ((ce = data_recalc_to_tps(pds, &pnq->recalc)) != NULL && te_attach(te, ce))
            ce = NULL;
        else
            goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_origin_location, "want-origin-location", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_destination_location, "want-destination-location", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->disable_navigation, "disable-navigation", te))
    {
        goto errexit;
    }

    if (pnq->want_route_map) {

        if ((ce = te_new("want-route-map")) != NULL &&
            te_setattru(ce, "angle-of-view", pnq->route_map_angle_of_view) &&
            te_setattru(ce, "distance-of-view", pnq->route_map_distance_of_view) &&
            te_setattru(ce, "min-time", pnq->route_map_min_time) &&
            te_setattru(ce, "tz", pnq->route_map_tile_z) &&
            te_setattru(ce, "passes", pnq->route_map_passes) &&
            te_setattru(ce, "max-tiles", pnq->route_map_max_tiles) &&
            te_attach(te, ce))
            ce = NULL;
        else
            goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_cross_streets, "want-cross-streets", te))
    {
        goto errexit;
    }

    l = CSL_VectorGetLength(pnq->vec_detour_avoid);
    for (n = 0; n < l; n++) {

        if ((ce = data_detour_avoid_to_tps(pds, CSL_VectorGetPointer(pnq->vec_detour_avoid, n))) != NULL && te_attach(te, ce))
            ce = NULL;
        else
            goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_towards_roadinfo, "want-toward-roadinfo", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_ferry_maneuvers, "want-ferry-maneuvers", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_historical_traffic, "want-historical-traffic", te))
    {
        goto errexit;
    }


    {if (!AttachSimpleWantElement(pnq->want_realtime_traffic, "want-realtime-traffic", te))
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_traffic_notification, "want-traffic-notification", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_historical_speed, "want-historical-speed", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_realtime_speed, "want-realtime-speed", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_speed_cameras, "want-speed-cameras", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_unpaved_info, "want-unpaved-info", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_intersection_roadinfo, "want-intersection-roadinfo", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_country_info, "want-country-info", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_enter_country_maneuvers, "want-enter-country-maneuvers", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_bridge_maneuvers, "want-bridge-maneuvers", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_tunnel_maneuvers, "want-tunnel-maneuvers", te))
    {
        goto errexit;
    }
    
    if (!AttachSimpleWantElement(pnq->want_entry_indicators, "want-entry-indicators", te))
    {
        goto errexit;
    }
    
    if (!AttachSimpleWantElement(pnq->want_route_features, "want-route-features", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_enter_roundabout_maneuvers, "want-enter-roundabout-maneuvers", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_route_extents, "want-route-extents", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_realistic_road_signs, "want-realistic-road-signs", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_junction_models, "want-junction-models", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_city_models, "want-city-models", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_speed_regions, "want-speed-regions", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_first_major_road, "want-first-major-road", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_label_point, "want-label-points", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_exit_numbers, "want-exit-numbers", te)) 
    {
        goto errexit;
    }

    if (pnq->want_guidance_config.want_guidance_config)
    {
        if ((ce = data_want_guidance_config_to_tps(pds, &pnq->want_guidance_config)) != NULL && te_attach(te, ce))
            ce = NULL;
        else
            goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_guidance_config.want_play_times, "want-play-times", te))
    {
        goto errexit;
    }

    if (pnq->want_alternate_routes.max_routes != 0)
    {
        if ((ce = data_want_alternate_routes_to_tps(pds, &pnq->want_alternate_routes)) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }
    if (!nsl_strempty(data_string_get(pds, &pnq->language)))
    {
        if (!te_setattrc(te, "language", data_string_get(pds, &pnq->language)))
            goto errexit;
    }
    if (!AttachSimpleWantElement(pnq->want_non_guidance_maneuvers, "want-non-guidance-maneuvers", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_complete_route, "want-complete-route", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_max_turn_for_street_count, "want-max-turn-distance-for-street-count", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_special_regions, "want-special-regions", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_first_maneuver_cross_streets, "want-first-maneuver-cross-streets", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_secondary_name_for_exits, "want-secondary-name-for-exits", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_check_unsupported_countries, "want-supported-country-check", te))
    {
        goto errexit;
    }

    if (!AttachSimpleWantElement(pnq->want_pedestrian_aneuvers, "want-continue-as-pedestrian-maneuvers", te))
    {
        goto errexit;
    }

    if (pnq->start_to_navigate)
    {
        tpselt ce = te_new("start-to-navigate");

        if (ce == NULL)
            goto errexit;

        if (te_setattr(ce, "route-id", (const char*)pnq->recalc.route_id.data, pnq->recalc.route_id.size) &&
            te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    return te;

errexit:

    if (te) te_dealloc(te);
    if (ce) te_dealloc(ce);

    return NULL;
}

static tpselt
data_nav_query_want_maneuver(data_util_state* pds, nb_boolean wantAllManeuvers)
{
    tpselt te;
    tpselt ce = NULL;

    te = te_new("want-maneuvers");

    if (te == NULL)
        goto errexit;

    /* If we want all the maneuvers, just leave off the sliceres element */
    if (!wantAllManeuvers)
    {
        if ((ce = te_new("sliceres")) != NULL && te_setattru(ce, "start", 0) &&
             te_setattru(ce, "end", 5) && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    return te;

errexit:

    if (te) te_dealloc(te);
    if (ce)    te_dealloc(ce);

    return NULL;
}

static tpselt
data_nav_query_recreate_route_id(data_util_state* pds, data_nav_query* pnq)
{

    tpselt te;

    te = te_new("recreate-route");

    if (te == NULL)
        goto errexit;

    if (!te_setattr(te, "route-id", (const char*) pnq->recreate_route_id.data, pnq->recreate_route_id.size))
        goto errexit;

    return te;

errexit:

    if (te) te_dealloc(te);

    return NULL;
}


NB_Error
data_nav_query_copy_detours_from_reply(data_util_state* pds, data_nav_query* pnq, data_nav_reply* pnr)
{
    NB_Error err = NE_OK;
    int n, l;
    data_detour_avoid detour_avoid;
    err = err ? err : data_detour_avoid_init(pds, &detour_avoid);

    l = CSL_VectorGetLength(pnr->vec_detour_avoid);
    for (n = 0; n < l && err == NE_OK; n++) {

        err = err ? err : data_detour_avoid_copy(pds, &detour_avoid, CSL_VectorGetPointer(pnr->vec_detour_avoid, n), FALSE);
        err = err ? err : CSL_VectorAppend(pnq->vec_detour_avoid, &detour_avoid) ? NE_OK : NE_NOMEM;

        if (err)
            data_detour_avoid_free(pds, &detour_avoid);
        else
            data_detour_avoid_init(pds, &detour_avoid);
    }

    // free scratch detour
    data_detour_avoid_free(pds, &detour_avoid);

    return err;
}

NB_Error data_nav_query_copy(data_util_state* pds, data_nav_query* dst, data_nav_query* src)
{
    NB_Error err = NE_OK;
    int n, l;
    data_detour_avoid detour_avoid;
    data_via_point viapoint;
    data_destination dest;
    err = err ? err : data_detour_avoid_init(pds, &detour_avoid);
    err = err ? err : data_via_point_init(pds, &viapoint);
    err = err ? err : data_destination_init(pds, &dest);
    data_want_extended_pronun_data_copy(pds, &dst->want_extended_pronun_data, &src->want_extended_pronun_data);

    // free scratch detour
    data_detour_avoid_free(pds, &detour_avoid);
    data_via_point_free(pds, &viapoint);
    data_destination_free(pds, &dest);

    // copy each field
    data_origin_copy(pds, &dst->origin, &src->origin);
    data_destination_copy(pds, &dst->destination, &src->destination);
    data_cache_contents_copy(pds, &dst->cache_contents, &src->cache_contents);
    data_map_cache_contents_copy(pds, &dst->map_cache_contents, &src->map_cache_contents);
    data_route_style_copy(pds, &dst->route_style, &src->route_style);
    data_recalc_copy(pds, &dst->recalc, &src->recalc);
    dst->want_guidance_config = src->want_guidance_config;
    data_want_alternate_routes_copy(pds, &dst->want_alternate_routes, &src->want_alternate_routes);
    data_want_extended_pronun_data_copy(pds, &dst->want_extended_pronun_data, &src->want_extended_pronun_data);
    dst->want_complete_route = src->want_complete_route;

    l = CSL_VectorGetLength(src->vec_detour_avoid);
    for (n = 0; n < l && err == NE_OK; n++) {

        err = err ? err : data_detour_avoid_copy(pds, &detour_avoid, CSL_VectorGetPointer(src->vec_detour_avoid, n), FALSE);
        err = err ? err : CSL_VectorAppend(dst->vec_detour_avoid, &detour_avoid) ? NE_OK : NE_NOMEM;

        if (err)
            data_detour_avoid_free(pds, &detour_avoid);
        else
            data_detour_avoid_init(pds, &detour_avoid);
    }

    l = CSL_VectorGetLength(src->via_point);

    for (n = 0; n < l && err == NE_OK; n++) {
        data_via_point_init(pds, &viapoint);
        err = err ? err : data_via_point_copy(pds, &viapoint, CSL_VectorGetPointer(src->via_point, n));
        if(viapoint.passed)
        {
            continue;
        }
        err = err ? err : CSL_VectorAppend(dst->via_point, &viapoint) ? NE_OK : NE_NOMEM;
        if (err)
            data_via_point_free(pds, &viapoint);
    }

    l = CSL_VectorGetLength(src->extra_destination);
    for (n = 0; n < l && err == NE_OK; n++) {
        data_destination_init(pds, &dest);
        err = err ? err : data_destination_copy(pds, &dest, CSL_VectorGetPointer(src->extra_destination, n));
        err = err ? err : CSL_VectorAppend(dst->extra_destination, &dest) ? NE_OK : NE_NOMEM;
        if (err)
        {
            data_destination_free(pds, &dest);
        }
    }

    dst->want_origin_location = src->want_origin_location;
    dst->want_destination_location = src->want_destination_location;
    dst->has_recalc = src->has_recalc;
    dst->disable_navigation = src->disable_navigation;
    dst->want_route_map = src->want_route_map;
    dst->want_cross_streets = src->want_cross_streets;
    dst->want_maneuvers = src->want_maneuvers;
    dst->want_all_maneuvers = src->want_all_maneuvers;

    dst->route_map_angle_of_view = src->route_map_angle_of_view;
    dst->route_map_distance_of_view = src->route_map_distance_of_view;
    dst->route_map_min_time = src->route_map_min_time;
    dst->route_map_passes = src->route_map_passes;
    dst->route_map_tile_z = src->route_map_tile_z;
    dst->route_map_max_tiles = src->route_map_max_tiles;

    dst->want_historical_traffic = src->want_historical_traffic;
    dst->want_realtime_traffic = src->want_realtime_traffic;
    dst->want_historical_speed = src->want_historical_speed;
    dst->want_realtime_speed = src->want_realtime_speed;
    dst->want_traffic_notification = src->want_traffic_notification;
    dst->want_unpaved_info = src->want_unpaved_info;
    dst->want_speed_cameras = src->want_speed_cameras;
    dst->want_ferry_maneuvers = src->want_ferry_maneuvers;
    dst->want_towards_roadinfo = src->want_towards_roadinfo;
    dst->want_intersection_roadinfo = src->want_intersection_roadinfo;
    dst->want_country_info = src->want_country_info;
    dst->want_enter_roundabout_maneuvers = src->want_enter_roundabout_maneuvers;
    dst->want_enter_country_maneuvers = src->want_enter_country_maneuvers;
    dst->want_bridge_maneuvers = src->want_bridge_maneuvers;
    dst->want_tunnel_maneuvers = src->want_tunnel_maneuvers;
    dst->want_route_extents = src->want_route_extents;
    dst->want_realistic_road_signs = src->want_realistic_road_signs;
    dst->want_junction_models = src->want_junction_models;
    dst->want_city_models = src->want_city_models;
    dst->want_exit_numbers = src->want_exit_numbers;
    dst->want_speed_regions = src->want_speed_regions;
    dst->want_first_major_road = src->want_first_major_road;
    dst->want_label_point = src->want_label_point;

    dst->want_non_guidance_maneuvers = src->want_non_guidance_maneuvers;

    data_string_copy(pds, &dst->pronun_style, &src->pronun_style);
    dst->command_set_version = src->command_set_version;

    nsl_memcpy(&dst->route_settings, &src->route_settings, sizeof(NB_RouteSettings));

    data_blob_copy(pds, &dst->recreate_route_id, &src->recreate_route_id);

    dst->max_pronun_files = src->max_pronun_files;

    data_string_copy(pds, &dst->language, &src->language);

    dst->want_max_turn_for_street_count = src->want_max_turn_for_street_count;
    dst->want_route_summary = src->want_route_summary;
    dst->want_route_summary_delaytime = src->want_route_summary_delaytime;
    dst->want_special_regions = src->want_special_regions;
    dst->want_first_maneuver_cross_streets = src->want_first_maneuver_cross_streets;
    dst->want_secondary_name_for_exits = src->want_secondary_name_for_exits;
    dst->want_check_unsupported_countries = src->want_check_unsupported_countries;
    dst->want_pedestrian_aneuvers = src->want_pedestrian_aneuvers;
    dst->start_to_navigate = src->start_to_navigate;
    dst->want_entry_indicators = src->want_entry_indicators;
    dst->want_route_features = src->want_route_features;

    return err;
}

NB_Error
data_nav_query_add_detour(data_util_state* pds, data_nav_query* pnq, data_blob* rid, double start, double length, const char* label, boolean want_historical_traffic, boolean want_realtime_traffic)
{
    NB_Error err = NE_OK;
    data_detour_avoid detour_avoid;

    err = err ? err : data_detour_avoid_init(pds, &detour_avoid);
    err = err ? err : data_blob_copy(pds, &detour_avoid.route_id, rid);
    err = err ? err : data_string_set(pds, &detour_avoid.label, label);

    detour_avoid.start = start;
    detour_avoid.length = length;

    detour_avoid.want_historical_traffic = want_historical_traffic;
    detour_avoid.want_realtime_traffic = want_realtime_traffic;

    err = err ? err : CSL_VectorAppend(pnq->vec_detour_avoid, &detour_avoid) ? NE_OK : NE_NOMEM;

    if (err)
        data_detour_avoid_free(pds, &detour_avoid);

    return err;
}

NB_Error
data_nav_query_clear_detours(data_util_state* pds, data_nav_query* pnq)
{
    NB_Error err = NE_OK;
    int n, l;

    // reset the vector after freeing all of the detour_avoid objects
    if (pnq->vec_detour_avoid) {
        l = CSL_VectorGetLength(pnq->vec_detour_avoid);

        for (n=0;n<l;n++)
            data_detour_avoid_free(pds, (data_detour_avoid*) CSL_VectorGetPointer(pnq->vec_detour_avoid, n));

        CSL_VectorDealloc(pnq->vec_detour_avoid);
    }

    pnq->vec_detour_avoid = CSL_VectorAlloc(sizeof(data_detour_avoid));

    if (pnq->vec_detour_avoid == NULL) {

        err = NE_NOMEM;
    }

    return err;
}

NB_Error
data_nav_query_set_route_selector_params(data_util_state* pds, data_nav_query* pnq, uint32 alternate_routes_count, boolean want_labels)
{
    if (!pnq)
    {
        return NE_INVAL;
    }

    pnq->want_alternate_routes.max_routes = alternate_routes_count;
    pnq->want_label_point = want_labels;

    return NE_OK;
}

boolean AttachSimpleWantElement(boolean attach, const char* name, tpselt parent)
{
    if (attach)
    {
        tpselt child = te_new(name);
        if (!child || !te_attach(parent, child))
        {
            return FALSE;
        }
    }

    return TRUE;
}
