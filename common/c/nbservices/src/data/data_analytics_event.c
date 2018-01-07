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

    @file     data_analytics_event.c
*/
/*
    See file description in header file.

    (C) Copyright 2004 - 2009 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary 
    to Networks In Motion, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */
#include "data_analytics_event.h"

NB_Error
data_analytics_event_init(data_util_state* pds, data_analytics_event* pae)
{
    NB_Error err = NE_OK;

    nsl_memset(pae, 0, sizeof(*pae));
    pae->event_type = analytics_event_none;

    DATA_INIT(pds, err, &pae->invocation_context, data_invocation_context);
    DATA_INIT(pds, err, &pae->active_session, data_string);

    return err;
}

void
data_analytics_event_free(data_util_state* pds, data_analytics_event* pae)
{
    switch (pae->event_type)
    {
        case analytics_event_search_query:
            data_search_query_event_free(pds, &pae->event.search_query_event);
            break;
        case analytics_event_search_detail:
            data_search_detail_event_free(pds, &pae->event.search_detail_event);
            break;
        case analytics_event_map:
            data_map_event_free(pds, &pae->event.map_event);
            break;
        case analytics_event_place_message:
            data_place_message_event_free(pds, &pae->event.place_message_event);
            break;
        case analytics_event_call:
            data_call_event_free(pds,&pae->event.call_event);
            break;
        case analytics_event_arrival:
            data_arrival_event_free(pds,&pae->event.arrival_event);
            break;
        case analytics_event_web_url:
            data_web_url_event_free(pds,&pae->event.web_url_event);
            break;
        case analytics_event_add_favorites:
            data_add_favorites_event_free(pds, &pae->event.add_favorites_event);
            break;
        case analytics_event_route_request:
            data_route_request_event_free(pds, &pae->event.route_request_event);
            break;
        case analytics_event_route_reply:
            data_route_reply_event_free(pds, &pae->event.route_reply_event);
            break;
        case analytics_event_route_state:
            data_route_state_event_free(pds, &pae->event.route_state_event);
            break;
        case analytics_event_gps_probes:
            data_gps_probes_event_free(pds, &pae->event.gps_probes_event);
            break;
        case analytics_event_impression:
            data_impression_event_free(pds, &pae->event.impression_event);
            break;
        case analytics_event_facebook_update:
            data_facebook_update_event_free(pds,&pae->event.facebook_update_event);
            break;
        case analytics_event_wifi_probes:
            data_wifi_probes_event_free(pds, &pae->event.wifi_probes_event);
            break;
        case analytics_event_coupon_analytics_action:
            data_coupon_analytics_action_event_free(pds, &pae->event.coupon_analytics_action_event);
            break;
        case analytics_event_nav_invocation:
            data_nav_invocation_event_free(pds, &pae->event.nav_invocation_event);
            break;
        case analytics_event_feedback:
            data_feedback_event_free(pds, &pae->event.feedback_event);
            break;
        case analytics_event_app_error:
            data_app_error_event_free(pds, &pae->event.app_error_event);
            break;
        case analytics_event_asr:
            data_asr_event_free(pds, &pae->event.asr_event);
            break;
        case analytics_event_reveal_review:
            data_reveal_review_event_free(pds, &pae->event.reveal_review_event);
            break;
        case analytics_event_reveal_description:
            data_reveal_description_event_free(pds, &pae->event.reveal_description_event);
            break;
        case analytics_event_poiext:
            data_poiext_event_free(pds, &pae->event.poiext_event);
            break;
        default:
            break;
    }

    DATA_FREE(pds, &pae->invocation_context, data_invocation_context);
    DATA_FREE(pds, &pae->active_session, data_string);

    nsl_memset(pae, 0, sizeof(*pae));
    pae->event_type = analytics_event_none;
}

tpselt
data_analytics_event_to_tps(data_util_state* pds, data_analytics_event* pae)
{
    tpselt te = NULL;
    tpselt ce = NULL;

    te = te_new("analytics-event");

    if (te == NULL)
    {
        goto errexit;
    }

    if (!te_setattru(te, "active-session", pae->id))
        goto errexit;

    if (!te_setattru(te, "id", pae->id))
        goto errexit;

    if (!te_setattru(te, "ts", pae->ts))
        goto errexit;

    if (!te_setattru(te, "session-id", pae->session_id))
        goto errexit;

    switch (pae->event_type)
    {
        case analytics_event_search_query:
            ce = data_search_query_event_to_tps(pds,
                    &pae->event.search_query_event);
            break;
        case analytics_event_search_detail:
            ce = data_search_detail_event_to_tps(pds,
                    &pae->event.search_detail_event);
            break;
        case analytics_event_map:
            ce = data_map_event_to_tps(pds,
                    &pae->event.map_event);
            break;
        case analytics_event_place_message:
            ce = data_place_message_event_to_tps(pds,
                    &pae->event.place_message_event);
            break;
        case analytics_event_call:
            ce = data_call_event_to_tps(pds,&pae->event.call_event);
            break;
        case analytics_event_arrival:
            ce = data_arrival_event_to_tps(pds,&pae->event.arrival_event);
            break;
        case analytics_event_web_url:
            ce = data_web_url_event_to_tps(pds,&pae->event.web_url_event);
            break;
        case analytics_event_add_favorites:
            ce = data_add_favorites_event_to_tps(pds,
                    &pae->event.add_favorites_event);
            break;
        case analytics_event_route_request:
            ce = data_route_request_event_to_tps(pds,
                    &pae->event.route_request_event);
            break;
        case analytics_event_route_reply:
            ce = data_route_reply_event_to_tps(pds,
                    &pae->event.route_reply_event);
            break;
        case analytics_event_route_state:
            ce = data_route_state_event_to_tps(pds,
                    &pae->event.route_state_event);
            break;
        case analytics_event_gps_probes:
            ce = data_gps_probes_event_to_tps(pds,
                    &pae->event.gps_probes_event);
            break;
        case analytics_event_impression:
            ce = data_impression_event_to_tps(pds, &pae->event.impression_event);
            break;
        case analytics_event_facebook_update:
            ce = data_facebook_update_event_to_tps(pds, &pae->event.facebook_update_event);
            break;
        case analytics_event_wifi_probes:
            ce = data_wifi_probes_event_to_tps(pds,
                    &pae->event.wifi_probes_event);
            break;
        case analytics_event_coupon_analytics_action:
            ce = data_coupon_analytics_action_event_to_tps(pds,
                    &pae->event.coupon_analytics_action_event);
            break;
        case analytics_event_nav_invocation:
            ce = data_nav_invocation_event_to_tps(pds,
                    &pae->event.nav_invocation_event);
            break;
        case analytics_event_feedback:
            ce = data_feedback_event_to_tps(pds,
                    &pae->event.feedback_event);
            break;
        case analytics_event_app_error:
            ce = data_app_error_event_to_tps(pds,
                    &pae->event.app_error_event);
            break;
        case analytics_event_asr:
            ce = data_asr_event_to_tps(pds,
                    &pae->event.asr_event);
            break;
        case analytics_event_reveal_review:
            ce = data_reveal_review_event_to_tps(pds, &pae->event.reveal_review_event);
            break;
        case analytics_event_reveal_description:
            ce = data_reveal_description_event_to_tps(pds, &pae->event.reveal_description_event);
            break;
        case analytics_event_poiext:
            ce = data_poiext_event_to_tps(pds, &pae->event.poiext_event);
            break;
        default:
            break;
    }

    if ((ce != NULL) && te_attach(te, ce))
    {
        ce = NULL;
    }
    else
    {
        goto errexit;
    }

    ce = data_invocation_context_to_tps(pds, &pae->invocation_context);
    if ((ce != NULL) && te_attach(te, ce))
    {
        ce = NULL;
    }
    else
    {
        goto errexit;
    }


    return te;

errexit:

    te_dealloc(te);
    return NULL;
}

boolean
data_analytics_event_equal(data_util_state* pds,
        data_analytics_event* pae1, data_analytics_event* pae2)
{
    int ret = TRUE;

    if (pae1->event_type != pae2->event_type)
    {
        return FALSE;   /* no need to compare further */
    }

    DATA_EQUAL(pds, ret, &pae1->active_session, &pae2->active_session, data_string);
    ret = ret ? (pae1->id == pae2->id): ret;
    ret = ret ? (pae1->ts == pae2->ts): ret;
    ret = ret ? (pae1->session_id == pae2->session_id): ret;

    switch (pae1->event_type)
    {
        case analytics_event_search_query:
            DATA_EQUAL(pds, ret, &pae1->event.search_query_event,
                    &pae2->event.search_query_event, data_search_query_event);
            break;
        case analytics_event_search_detail:
            DATA_EQUAL(pds, ret, &pae1->event.search_detail_event,
                    &pae2->event.search_detail_event, data_search_detail_event);
            break;
        case analytics_event_map:
            DATA_EQUAL(pds, ret, &pae1->event.map_event,
                    &pae2->event.map_event, data_map_event);
            break;
        case analytics_event_place_message:
            DATA_EQUAL(pds, ret, &pae1->event.place_message_event,
                    &pae2->event.place_message_event, data_place_message_event);
            break;
        case analytics_event_call:
            DATA_EQUAL(pds, ret, &pae1->event.call_event,
                    &pae2->event.call_event, data_call_event);
            break;
        case analytics_event_arrival:
            DATA_EQUAL(pds, ret, &pae1->event.arrival_event,
                       &pae2->event.arrival_event, data_arrival_event);
            break;
        case analytics_event_web_url:
            DATA_EQUAL(pds, ret, &pae1->event.web_url_event,
                       &pae2->event.web_url_event, data_web_url_event);
            break;
        case analytics_event_add_favorites:
            DATA_EQUAL(pds, ret, &pae1->event.add_favorites_event,
                    &pae2->event.add_favorites_event, data_add_favorites_event);
            break;
        case analytics_event_route_request:
            DATA_EQUAL(pds, ret, &pae1->event.route_request_event,
                    &pae2->event.route_request_event, data_route_request_event);
            break;
        case analytics_event_route_reply:
            DATA_EQUAL(pds, ret, &pae1->event.route_reply_event,
                    &pae2->event.route_reply_event, data_route_reply_event);
            break;
        case analytics_event_route_state:
            DATA_EQUAL(pds, ret, &pae1->event.route_state_event,
                    &pae2->event.route_state_event, data_route_state_event);
            break;
        case analytics_event_gps_probes:
            DATA_EQUAL(pds, ret, &pae1->event.gps_probes_event,
                    &pae2->event.gps_probes_event, data_gps_probes_event);
            break;
        case analytics_event_impression:
            DATA_EQUAL(pds, ret, &pae1->event.impression_event, &pae2->event.impression_event, data_impression_event);
            break;
        case analytics_event_facebook_update:
            DATA_EQUAL(pds, ret, &pae1->event.facebook_update_event, &pae2->event.facebook_update_event, data_facebook_update_event);
            break;
        case analytics_event_wifi_probes:
            DATA_EQUAL(pds, ret, &pae1->event.wifi_probes_event,
                    &pae2->event.wifi_probes_event, data_wifi_probes_event);
            break;
        case analytics_event_coupon_analytics_action:
            DATA_EQUAL(pds, ret, &pae1->event.coupon_analytics_action_event,
                    &pae2->event.coupon_analytics_action_event, data_coupon_analytics_action_event);
            break;
        case analytics_event_nav_invocation:
            DATA_EQUAL(pds, ret, &pae1->event.nav_invocation_event,
                    &pae2->event.nav_invocation_event, data_nav_invocation_event);
            break;
        case analytics_event_feedback:
            DATA_EQUAL(pds, ret, &pae1->event.feedback_event,
                    &pae2->event.feedback_event, data_feedback_event);
            break;
        case analytics_event_app_error:
            DATA_EQUAL(pds, ret, &pae1->event.app_error_event,
                    &pae2->event.app_error_event, data_app_error_event);
            break;
        case analytics_event_asr:
            DATA_EQUAL(pds, ret, &pae1->event.asr_event,
                    &pae2->event.asr_event, data_asr_event);
            break;
        case analytics_event_reveal_review:
            DATA_EQUAL(pds, ret, &pae1->event.reveal_review_event,
                       &pae2->event.reveal_review_event, data_reveal_review_event);
            break;
        case analytics_event_reveal_description:
            DATA_EQUAL(pds, ret, &pae1->event.reveal_description_event,
                       &pae2->event.reveal_description_event, data_reveal_description_event);
            break;
        case analytics_event_poiext:
            DATA_EQUAL(pds, ret, &pae1->event.poiext_event, &pae2->event.poiext_event, data_poiext_event);
            break;
        default:
            break;
    }

    DATA_EQUAL(pds, ret, &pae1->invocation_context, &pae2->invocation_context, data_invocation_context);

    return (boolean) ret;
}

NB_Error
data_analytics_event_copy(data_util_state* pds,
        data_analytics_event* pae_dest, data_analytics_event* pae_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, pae_dest, data_analytics_event);

    DATA_COPY(pds, err, &pae_dest->active_session, &pae_src->active_session, data_string);
    pae_dest->id = pae_src->id;
    pae_dest->ts = pae_src->ts;
    pae_dest->session_id = pae_src->session_id;
    pae_dest->event_type = pae_src->event_type;

    switch (pae_src->event_type)
    {
        case analytics_event_search_query:
            DATA_COPY(pds, err, &pae_dest->event.search_query_event,
                    &pae_src->event.search_query_event,
                    data_search_query_event);
            break;
        case analytics_event_search_detail:
            DATA_COPY(pds, err, &pae_dest->event.search_detail_event,
                    &pae_src->event.search_detail_event,
                    data_search_detail_event);
            break;
        case analytics_event_map:
            DATA_COPY(pds, err, &pae_dest->event.map_event,
                    &pae_src->event.map_event, data_map_event);
            break;
        case analytics_event_place_message:
            DATA_COPY(pds, err, &pae_dest->event.place_message_event,
                    &pae_src->event.place_message_event,
                    data_place_message_event);
            break;
        case analytics_event_call:
            DATA_COPY(pds, err, &pae_dest->event.call_event,
                    &pae_src->event.call_event, data_call_event);
            break;
        case analytics_event_arrival:
            DATA_COPY(pds, err, &pae_dest->event.arrival_event,
                      &pae_src->event.arrival_event, data_arrival_event);
            break;
        case analytics_event_web_url:
            DATA_COPY(pds, err, &pae_dest->event.web_url_event,
                      &pae_src->event.web_url_event, data_web_url_event);
            break;
        case analytics_event_add_favorites:
            DATA_COPY(pds, err, &pae_dest->event.add_favorites_event,
                    &pae_src->event.add_favorites_event,
                    data_add_favorites_event);
            break;
        case analytics_event_route_request:
            DATA_COPY(pds, err, &pae_dest->event.route_request_event,
                    &pae_src->event.route_request_event,
                    data_route_request_event);
            break;
        case analytics_event_route_reply:
            DATA_COPY(pds, err, &pae_dest->event.route_reply_event,
                    &pae_src->event.route_reply_event, data_route_reply_event);
            break;
        case analytics_event_route_state:
            DATA_COPY(pds, err, &pae_dest->event.route_state_event,
                    &pae_src->event.route_state_event, data_route_state_event);
            break;
        case analytics_event_gps_probes:
            DATA_COPY(pds, err, &pae_dest->event.gps_probes_event,
                    &pae_src->event.gps_probes_event, data_gps_probes_event);
            break;
        case analytics_event_impression:
            DATA_COPY(pds, err, &pae_dest->event.impression_event, &pae_src->event.impression_event, data_impression_event);
            break;
        case analytics_event_facebook_update:
            DATA_COPY(pds, err, &pae_dest->event.facebook_update_event, &pae_src->event.facebook_update_event, data_facebook_update_event);
            break;
        case analytics_event_wifi_probes:
            DATA_COPY(pds, err, &pae_dest->event.wifi_probes_event,
                    &pae_src->event.wifi_probes_event, data_wifi_probes_event);
            break;
        case analytics_event_coupon_analytics_action:
            DATA_COPY(pds, err, &pae_dest->event.coupon_analytics_action_event,
                    &pae_src->event.coupon_analytics_action_event, data_coupon_analytics_action_event);
            break;
        case analytics_event_nav_invocation:
            DATA_COPY(pds, err, &pae_dest->event.nav_invocation_event,
                    &pae_src->event.nav_invocation_event, data_nav_invocation_event);
            break;
        case analytics_event_feedback:
            DATA_COPY(pds, err, &pae_dest->event.feedback_event,
                    &pae_src->event.feedback_event, data_feedback_event);
            break;
        case analytics_event_app_error:
            DATA_COPY(pds, err, &pae_dest->event.app_error_event,
                    &pae_src->event.app_error_event, data_app_error_event);
            break;
        case analytics_event_asr:
            DATA_COPY(pds, err, &pae_dest->event.asr_event,
                    &pae_src->event.asr_event, data_asr_event);
            break;
        case analytics_event_reveal_review:
            DATA_COPY(pds, err, &pae_dest->event.reveal_review_event,
                      &pae_src->event.reveal_review_event, data_reveal_review_event);
            break;
        case analytics_event_reveal_description:
            DATA_COPY(pds, err, &pae_dest->event.reveal_description_event,
                      &pae_src->event.reveal_description_event, data_reveal_description_event);
            break;
        case analytics_event_poiext:
            DATA_COPY(pds, err, &pae_dest->event.poiext_event,
                      &pae_src->event.poiext_event, data_poiext_event);
            break;
        default:
            break;
    }

    DATA_COPY(pds, err, &pae_dest->invocation_context, &pae_src->invocation_context, data_invocation_context);

    return err;
}

uint32
data_analytics_event_get_tps_size(data_util_state* pds,
        data_analytics_event* pae)
{
    uint32 size = 0;

    size += sizeof(uint32); // event_type

    switch (pae->event_type)
    {
        case analytics_event_search_query:
            size += data_search_query_event_get_tps_size(pds,
                    &pae->event.search_query_event);
            break;
        case analytics_event_search_detail:
            size += data_search_detail_event_get_tps_size(pds,
                    &pae->event.search_detail_event);
            break;
        case analytics_event_map:
            size += data_map_event_get_tps_size(pds, &pae->event.map_event);
            break;
        case analytics_event_place_message:
            size += data_place_message_event_get_tps_size(pds,
                    &pae->event.place_message_event);
            break;
        case analytics_event_call:
            size += data_call_event_get_tps_size(pds, &pae->event.call_event);
            break;
        case analytics_event_arrival:
            size += data_arrival_event_get_tps_size(pds, &pae->event.arrival_event);
            break;
        case analytics_event_web_url:
            size += data_web_url_event_get_tps_size(pds, &pae->event.web_url_event);
            break;
        case analytics_event_add_favorites:
            size += data_add_favorites_event_get_tps_size(pds,
                    &pae->event.add_favorites_event);
            break;
        case analytics_event_route_request:
            size += data_route_request_event_get_tps_size(pds,
                    &pae->event.route_request_event);
            break;
        case analytics_event_route_reply:
            size += data_route_reply_event_get_tps_size(pds,
                    &pae->event.route_reply_event);
            break;
        case analytics_event_route_state:
            size += data_route_state_event_get_tps_size(pds,
                    &pae->event.route_state_event);
            break;
        case analytics_event_gps_probes:
            size += data_gps_probes_event_get_tps_size(pds,
                    &pae->event.gps_probes_event);
            break;
        case analytics_event_impression:
            size += data_impression_event_get_tps_size(pds, &pae->event.impression_event);
            break;
        case analytics_event_facebook_update:
            size += data_facebook_update_event_get_tps_size(pds, &pae->event.facebook_update_event);
            break;
        case analytics_event_wifi_probes:
            size += data_wifi_probes_event_get_tps_size(pds,
                    &pae->event.wifi_probes_event);
            break;
        case analytics_event_coupon_analytics_action:
            size += data_coupon_analytics_action_event_get_tps_size(pds,
                    &pae->event.coupon_analytics_action_event);
            break;
        case analytics_event_nav_invocation:
            size += data_nav_invocation_event_get_tps_size(pds,
                    &pae->event.nav_invocation_event);
            break;
        case analytics_event_feedback:
            size += data_feedback_event_get_tps_size(pds,
                    &pae->event.feedback_event);
            break;
        case analytics_event_app_error:
            size += data_app_error_event_get_tps_size(pds,
                    &pae->event.app_error_event);
            break;
        case analytics_event_asr:
            size += data_asr_event_get_tps_size(pds,
                    &pae->event.asr_event);
            break;
        case analytics_event_reveal_review:
            size += data_reveal_review_event_get_tps_size(pds, &pae->event.reveal_review_event);
            break;
        case analytics_event_reveal_description:
            size += data_reveal_description_event_get_tps_size(pds, &pae->event.reveal_description_event);
            break;
        case analytics_event_poiext:
            size += data_poiext_event_get_tps_size(pds, &pae->event.poiext_event);
            break;
        default:
            break;
    }

    size += data_invocation_context_get_tps_size(pds, &pae->invocation_context);

    size += data_string_get_tps_size(pds, &pae->active_session);
    size += sizeof(pae->id);
    size += sizeof(pae->ts);
    size += sizeof(pae->session_id);

    return size;
}

NB_Error
data_analytics_event_set_event_type(data_util_state* pds,
        data_analytics_event* pae, analytics_event_type event_type)
{
    NB_Error err = NE_OK;

    if (pae->event_type == event_type)
    {
        return NE_OK;   /* no change */
    }

    DATA_REINIT(pds, err, pae, data_analytics_event);
    if (err != NE_OK)
    {
        return err;
    }

    switch (event_type)
    {
        case analytics_event_search_query:
            err = data_search_query_event_init(pds,
                    &pae->event.search_query_event);
            break;
        case analytics_event_search_detail:
            err = data_search_detail_event_init(pds,
                    &pae->event.search_detail_event);
            break;
        case analytics_event_map:
            err = data_map_event_init(pds,
                    &pae->event.map_event);
            break;
        case analytics_event_place_message:
            err = data_place_message_event_init(pds,
                    &pae->event.place_message_event);
            break;
        case analytics_event_call:
            err = data_call_event_init(pds,
                    &pae->event.call_event);
            break;
        case analytics_event_arrival:
            err = data_arrival_event_init(pds, &pae->event.arrival_event);
            break;
        case analytics_event_web_url:
            err = data_web_url_event_init(pds, &pae->event.web_url_event);
            break;
        case analytics_event_add_favorites:
            err = data_add_favorites_event_init(pds,
                    &pae->event.add_favorites_event);
            break;
        case analytics_event_route_request:
            err = data_route_request_event_init(pds,
                    &pae->event.route_request_event);
            break;
        case analytics_event_route_reply:
            err = data_route_reply_event_init(pds,
                    &pae->event.route_reply_event);
            break;
        case analytics_event_route_state:
            err = data_route_state_event_init(pds,
                    &pae->event.route_state_event);
            break;
        case analytics_event_gps_probes:
            err = data_gps_probes_event_init(pds,
                    &pae->event.gps_probes_event);
            break;
        case analytics_event_impression:
            err = data_impression_event_init(pds, &pae->event.impression_event);
            break;
        case analytics_event_facebook_update:
            err = data_facebook_update_event_init(pds, &pae->event.facebook_update_event);
            break;
        case analytics_event_wifi_probes:
            err = data_wifi_probes_event_init(pds,
                    &pae->event.wifi_probes_event);
            break;
        case analytics_event_coupon_analytics_action:
            err = data_coupon_analytics_action_event_init(pds,
                    &pae->event.coupon_analytics_action_event);
            break;
        case analytics_event_nav_invocation:
            err = data_nav_invocation_event_init(pds,
                    &pae->event.nav_invocation_event);
            break;
        case analytics_event_feedback:
            err = data_feedback_event_init(pds,
                    &pae->event.feedback_event);
            break;
        case analytics_event_app_error:
            err = data_app_error_event_init(pds,
                    &pae->event.app_error_event);
            break;
        case analytics_event_asr:
            err = data_asr_event_init(pds,
                    &pae->event.asr_event);
            break;
        case analytics_event_reveal_review:
            err = data_reveal_review_event_init(pds, &pae->event.reveal_review_event);
            break;
        case analytics_event_reveal_description:
            err = data_reveal_description_event_init(pds, &pae->event.reveal_description_event);
            break;
        case analytics_event_poiext:
            err = data_poiext_event_init(pds, &pae->event.poiext_event);
            break;
        default:
            break;
    }

    if (!err)
    {
        pae->event_type = event_type;
    }

    return err;
}

void
data_analytics_event_to_buf(data_util_state* pds,
            data_analytics_event* pae,
            struct dynbuf *pdb)
{
    uint32 type = pae->event_type;

    dbufcat(pdb, (const byte*) &type, sizeof(type));
    switch (pae->event_type)
    {
        case analytics_event_search_query:
            data_search_query_event_to_buf(pds,
                    &pae->event.search_query_event, pdb);
            break;
        case analytics_event_search_detail:
            data_search_detail_event_to_buf(pds,
                    &pae->event.search_detail_event, pdb);
            break;
        case analytics_event_map:
            data_map_event_to_buf(pds, &pae->event.map_event, pdb);
            break;
        case analytics_event_place_message:
            data_place_message_event_to_buf(pds,
                    &pae->event.place_message_event, pdb);
            break;
        case analytics_event_call:
            data_call_event_to_buf(pds,&pae->event.call_event, pdb);
            break;
        case analytics_event_arrival:
            data_arrival_event_to_buf(pds,&pae->event.arrival_event, pdb);
            break;
        case analytics_event_web_url:
            data_web_url_event_to_buf(pds,&pae->event.web_url_event, pdb);
            break;
        case analytics_event_add_favorites:
            data_add_favorites_event_to_buf(pds,
                    &pae->event.add_favorites_event, pdb);
            break;
        case analytics_event_route_request:
            data_route_request_event_to_buf(pds,
                    &pae->event.route_request_event, pdb);
            break;
        case analytics_event_route_reply:
            data_route_reply_event_to_buf(pds,
                    &pae->event.route_reply_event, pdb);
            break;
        case analytics_event_route_state:
            data_route_state_event_to_buf(pds,
                    &pae->event.route_state_event, pdb);
            break;
        case analytics_event_gps_probes:
            data_gps_probes_event_to_buf(pds,
                    &pae->event.gps_probes_event, pdb);
            break;
        case analytics_event_impression:
            data_impression_event_to_buf(pds, &pae->event.impression_event, pdb);
            break;
        case analytics_event_facebook_update:
            data_facebook_update_event_to_buf(pds, &pae->event.facebook_update_event, pdb);
            break;
        case analytics_event_wifi_probes:
            data_wifi_probes_event_to_buf(pds,
                    &pae->event.wifi_probes_event, pdb);
            break;
        case analytics_event_coupon_analytics_action:
            data_coupon_analytics_action_event_to_buf(pds,
                    &pae->event.coupon_analytics_action_event, pdb);
            break;
        case analytics_event_nav_invocation:
            data_nav_invocation_event_to_buf(pds,
                    &pae->event.nav_invocation_event, pdb);
            break;
        case analytics_event_feedback:
            data_feedback_event_to_buf(pds,
                    &pae->event.feedback_event, pdb);
            break;
        case analytics_event_app_error:
            data_app_error_event_to_buf(pds,
                    &pae->event.app_error_event, pdb);
            break;
        case analytics_event_asr:
            data_asr_event_to_buf(pds,
                    &pae->event.asr_event, pdb);
            break;
        case analytics_event_reveal_review:
            data_reveal_review_event_to_buf(pds, &pae->event.reveal_review_event, pdb);
            break;
        case analytics_event_reveal_description:
            data_reveal_description_event_to_buf(pds, &pae->event.reveal_description_event, pdb);
            break;
        case analytics_event_poiext:
            data_poiext_event_to_buf(pds, &pae->event.poiext_event, pdb);
            break;
        default:
            break;
    }

    data_invocation_context_to_buf(pds, &pae->invocation_context, pdb);

    data_string_to_buf(pds, &pae->active_session, pdb);
    dbufcat(pdb, (const byte*) &pae->id, sizeof(pae->id));
    dbufcat(pdb, (const byte*) &pae->ts, sizeof(pae->ts));
    dbufcat(pdb, (const byte*) &pae->session_id, sizeof(pae->session_id));
}

NB_Error data_analytics_event_from_binary(data_util_state* pds,
            data_analytics_event* pae,
            byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;
    uint32 type = analytics_event_none;

    err = err ? err : data_uint32_from_binary(pds, &type, pdata, pdatalen);
    if (err)
    {
        return err;
    }

    err = data_analytics_event_set_event_type(pds, pae,
            (analytics_event_type)type);
    if (err)
    {
        return err;
    }

    switch (pae->event_type)
    {
        case analytics_event_search_query:
            err = data_search_query_event_from_binary(pds,
                    &pae->event.search_query_event, pdata, pdatalen);
            break;
        case analytics_event_search_detail:
            err = data_search_detail_event_from_binary(pds,
                    &pae->event.search_detail_event, pdata, pdatalen);
            break;
        case analytics_event_map:
            err = data_map_event_from_binary(pds,
                    &pae->event.map_event, pdata, pdatalen);
            break;
        case analytics_event_place_message:
            err = data_place_message_event_from_binary(pds,
                    &pae->event.place_message_event, pdata, pdatalen);
            break;
        case analytics_event_call:
            err = data_call_event_from_binary(pds,
                    &pae->event.call_event, pdata, pdatalen);
            break;
        case analytics_event_arrival:
            err = data_arrival_event_from_binary(pds, &pae->event.arrival_event, pdata, pdatalen);
            break;
        case analytics_event_web_url:
            err = data_web_url_event_from_binary(pds, &pae->event.web_url_event, pdata, pdatalen);
            break;
        case analytics_event_add_favorites:
            err = data_add_favorites_event_from_binary(pds,
                    &pae->event.add_favorites_event, pdata, pdatalen);
            break;
        case analytics_event_route_request:
            err = data_route_request_event_from_binary(pds,
                    &pae->event.route_request_event, pdata, pdatalen);
            break;
        case analytics_event_route_reply:
            err = data_route_reply_event_from_binary(pds,
                    &pae->event.route_reply_event, pdata, pdatalen);
            break;
        case analytics_event_route_state:
            err = data_route_state_event_from_binary(pds,
                    &pae->event.route_state_event, pdata, pdatalen);
            break;
        case analytics_event_gps_probes:
            err = data_gps_probes_event_from_binary(pds,
                    &pae->event.gps_probes_event, pdata, pdatalen);
            break;
        case analytics_event_impression:
            err = data_impression_event_from_binary(pds, &pae->event.impression_event, pdata, pdatalen);
            break;
        case analytics_event_facebook_update:
            err = data_facebook_update_event_from_binary(pds, &pae->event.facebook_update_event, pdata, pdatalen);
            break;
        case analytics_event_wifi_probes:
            err = data_wifi_probes_event_from_binary(pds,
                    &pae->event.wifi_probes_event, pdata, pdatalen);
            break;
        case analytics_event_coupon_analytics_action:
            err = data_coupon_analytics_action_event_from_binary(pds,
                    &pae->event.coupon_analytics_action_event, pdata, pdatalen);
            break;
        case analytics_event_nav_invocation:
            err = data_nav_invocation_event_from_binary(pds,
                    &pae->event.nav_invocation_event, pdata, pdatalen);
            break;
        case analytics_event_feedback:
            err = data_feedback_event_from_binary(pds,
                    &pae->event.feedback_event, pdata, pdatalen);
            break;
        case analytics_event_app_error:
            err = data_app_error_event_from_binary(pds,
                    &pae->event.app_error_event, pdata, pdatalen);
            break;
        case analytics_event_asr:
            err = data_asr_event_from_binary(pds,
                    &pae->event.asr_event, pdata, pdatalen);
            break;
        case analytics_event_reveal_review:
            err = data_reveal_review_event_from_binary(pds, &pae->event.reveal_review_event, pdata, pdatalen);
            break;
        case analytics_event_reveal_description:
            err = data_reveal_description_event_from_binary(pds, &pae->event.reveal_description_event, pdata, pdatalen);
            break;
        case analytics_event_poiext:
            err = data_poiext_event_from_binary(pds, &pae->event.poiext_event, pdata, pdatalen);
            break;
        default:
            break;
    }

    err = err ? err : data_invocation_context_from_binary(pds, &pae->invocation_context, pdata, pdatalen);

    err = err ? err : data_string_from_binary(pds, &pae->active_session, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pae->id, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pae->ts, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pae->session_id, pdata,
                                              pdatalen);

    return err;
}

/*! @} */
