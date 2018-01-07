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

    @file     data_analytics_config.c
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
#include "data_analytics_config.h"

NB_Error
data_analytics_config_init(data_util_state* pds, data_analytics_config* pac)
{
    NB_Error err = NE_OK;

    DATA_INIT(pds, err, &pac->gold_events_config,
            data_gold_events_config);

    DATA_INIT(pds, err, &pac->poi_events_config,
            data_poi_events_config);

    DATA_INIT(pds, err, &pac->route_tracking_config,
            data_route_tracking_config);

    DATA_INIT(pds, err, &pac->gps_probes_config,
            data_gps_probes_config);

    DATA_INIT(pds, err, &pac->wifi_probes_config,
            data_wifi_probes_config);

    DATA_INIT(pds, err, &pac->coupon_events_config,
            data_coupon_events_config);

    DATA_INIT(pds, err, &pac->nav_invocation_config,
            data_nav_invocation_config);

    DATA_INIT(pds, err, &pac->feedback_config,
            data_feedback_config);

    DATA_INIT(pds, err, &pac->app_errors_config,
            data_app_errors_config);

    DATA_INIT(pds, err, &pac->asr_event_config,
            data_asr_event_config);

    DATA_INIT(pds, err, &pac->poiext_event_config,
            data_poiext_event_config);

    DATA_INIT(pds, err, &pac->session_config,
            data_session_config);

    DATA_INIT(pds, err, &pac->settings_events_config,
            data_settings_events_config);

    DATA_INIT(pds, err, &pac->share_events_config,
            data_share_events_config);

    DATA_INIT(pds, err, &pac->user_actions_events_config,
            data_user_actions_events_config);

    DATA_INIT(pds, err, &pac->transaction_events_config,
            data_transaction_events_config);

    DATA_INIT(pds, err, &pac->map_events_config,
            data_map_events_config);

    pac->max_hold_time = 0;
    pac->max_data_age = 0;
    pac->max_request_size = 0;

    return err;
}

void
data_analytics_config_free(data_util_state* pds, data_analytics_config* pac)
{
    DATA_FREE(pds, &pac->gold_events_config,
            data_gold_events_config);

    DATA_FREE(pds, &pac->poi_events_config,
            data_poi_events_config);

    DATA_FREE(pds, &pac->route_tracking_config,
            data_route_tracking_config);

    DATA_FREE(pds, &pac->gps_probes_config,
            data_gps_probes_config);

    DATA_FREE(pds, &pac->wifi_probes_config,
            data_wifi_probes_config);

    DATA_FREE(pds, &pac->coupon_events_config,
            data_coupon_events_config);

    DATA_FREE(pds, &pac->nav_invocation_config,
            data_nav_invocation_config);

    DATA_FREE(pds, &pac->feedback_config,
            data_feedback_config);

    DATA_FREE(pds, &pac->app_errors_config,
            data_app_errors_config);

    DATA_FREE(pds, &pac->asr_event_config,
            data_asr_event_config);

    DATA_FREE(pds, &pac->poiext_event_config,
            data_poiext_event_config);

    DATA_FREE(pds, &pac->session_config,
            data_session_config);

    DATA_FREE(pds, &pac->settings_events_config,
            data_settings_events_config);

    DATA_FREE(pds, &pac->share_events_config,
            data_share_events_config);

    DATA_FREE(pds, &pac->user_actions_events_config,
            data_user_actions_events_config);

    DATA_FREE(pds, &pac->transaction_events_config,
            data_transaction_events_config);

    DATA_FREE(pds, &pac->map_events_config,
            data_map_events_config);
}

NB_Error
data_analytics_config_from_tps(data_util_state* pds, data_analytics_config* pac, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt ce = NULL;

    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    data_analytics_config_free(pds, pac);

    err = data_analytics_config_init(pds, pac);

    if (err != NE_OK)
        return err;

    pac->max_hold_time = te_getattru(te, "max-hold-time");
    pac->max_data_age = te_getattru(te, "max-data-age");
    pac->max_request_size = te_getattru(te, "max-request-size");

    ce = te_getchild(te, "gold-events-config");
    if (ce != NULL)
        err = err ? err : data_gold_events_config_from_tps(pds,
                            &pac->gold_events_config, ce);

    ce = te_getchild(te, "poi-events-config");
    if (ce != NULL)
        err = err ? err : data_poi_events_config_from_tps(pds,
                            &pac->poi_events_config, ce);

    ce = te_getchild(te, "route-tracking-config");
    if (ce != NULL)
        err = err ? err : data_route_tracking_config_from_tps(pds,
                            &pac->route_tracking_config, ce);

    ce = te_getchild(te, "gps-probes-config");
    if (ce != NULL)
        err = err ? err : data_gps_probes_config_from_tps(pds,
                            &pac->gps_probes_config, ce);

    ce = te_getchild(te, "wifi-probes-config");
    if (ce != NULL)
        err = err ? err : data_wifi_probes_config_from_tps(pds,
                            &pac->wifi_probes_config, ce);

    ce = te_getchild(te, "coupon-events-config");
    if (ce != NULL)
        err = err ? err : data_coupon_events_config_from_tps(pds,
                            &pac->coupon_events_config, ce);

    ce = te_getchild(te, "nav-invocation-config");
    if (ce != NULL)
        err = err ? err : data_nav_invocation_config_from_tps(pds,
                            &pac->nav_invocation_config, ce);

    ce = te_getchild(te, "feedback-config");
    if (ce != NULL)
        err = err ? err : data_feedback_config_from_tps(pds,
                            &pac->feedback_config, ce);

    ce = te_getchild(te, "app-errors-config");
    if (ce != NULL)
        err = err ? err : data_app_errors_config_from_tps(pds,
                            &pac->app_errors_config, ce);

    ce = te_getchild(te, "asr-event-config");
    if (ce != NULL)
        err = err ? err : data_asr_event_config_from_tps(pds,
                            &pac->asr_event_config, ce);

    ce = te_getchild(te, "extpoi-event-config");
    if (ce != NULL)
        err = err ? err : data_poiext_event_config_from_tps(pds,
                            &pac->poiext_event_config, ce);

    ce = te_getchild(te, "session-config");
    if (ce != NULL)
        err = err ? err : data_session_config_from_tps(pds,
                            &pac->session_config, ce);

    ce = te_getchild(te, "settings-events-config");
    if (ce != NULL)
        err = err ? err : data_settings_events_config_from_tps(pds,
                            &pac->settings_events_config, ce);

    ce = te_getchild(te, "share-events-config");
    if (ce != NULL)
        err = err ? err : data_share_events_config_from_tps(pds,
                            &pac->share_events_config, ce);

    ce = te_getchild(te, "user-action-events-config");
    if (ce != NULL)
        err = err ? err : data_user_actions_events_config_from_tps(pds,
                            &pac->user_actions_events_config, ce);

    ce = te_getchild(te, "transaction-events-config");
    if (ce != NULL)
        err = err ? err : data_transaction_events_config_from_tps(pds,
                            &pac->transaction_events_config, ce);

    ce = te_getchild(te, "map-events-config");
    if (ce != NULL)
        err = err ? err : data_map_events_config_from_tps(pds,
                            &pac->map_events_config, ce);
errexit:
    if (err != NE_OK)
        data_analytics_config_free(pds, pac);
    return err;
}

boolean
data_analytics_config_equal(data_util_state* pds, data_analytics_config* pac1, data_analytics_config* pac2)
{
    int ret = TRUE;

    ret = ret ? (pac1->max_hold_time == pac2->max_hold_time): ret;
    ret = ret ? (pac1->max_data_age == pac2->max_data_age): ret;
    ret = ret ? (pac1->max_request_size == pac2->max_request_size): ret;

    DATA_EQUAL(pds, ret, &pac1->gold_events_config, &pac2->gold_events_config,
            data_gold_events_config);

    DATA_EQUAL(pds, ret, &pac1->poi_events_config,
            &pac2->poi_events_config, data_poi_events_config);

    DATA_EQUAL(pds, ret, &pac1->route_tracking_config,
            &pac2->route_tracking_config, data_route_tracking_config);

    DATA_EQUAL(pds, ret, &pac1->gps_probes_config,
            &pac2->gps_probes_config, data_gps_probes_config);

    DATA_EQUAL(pds, ret, &pac1->wifi_probes_config,
            &pac2->wifi_probes_config, data_wifi_probes_config);

    DATA_EQUAL(pds, ret, &pac1->coupon_events_config,
            &pac2->coupon_events_config, data_coupon_events_config);

    DATA_EQUAL(pds, ret, &pac1->nav_invocation_config,
            &pac2->nav_invocation_config, data_nav_invocation_config);

    DATA_EQUAL(pds, ret, &pac1->feedback_config,
            &pac2->feedback_config, data_feedback_config);

    DATA_EQUAL(pds, ret, &pac1->app_errors_config,
            &pac2->app_errors_config, data_app_errors_config);

    DATA_EQUAL(pds, ret, &pac1->asr_event_config,
            &pac2->asr_event_config, data_asr_event_config);

    DATA_EQUAL(pds, ret, &pac1->poiext_event_config,
            &pac2->poiext_event_config, data_poiext_event_config);

    DATA_EQUAL(pds, ret, &pac1->session_config,
            &pac2->session_config, data_session_config);

    DATA_EQUAL(pds, ret, &pac1->settings_events_config,
            &pac2->settings_events_config, data_settings_events_config);

    DATA_EQUAL(pds, ret, &pac1->share_events_config,
            &pac2->share_events_config, data_share_events_config);

    DATA_EQUAL(pds, ret, &pac1->user_actions_events_config,
            &pac2->user_actions_events_config, data_user_actions_events_config);

    DATA_EQUAL(pds, ret, &pac1->transaction_events_config,
            &pac2->transaction_events_config, data_transaction_events_config);

    DATA_EQUAL(pds, ret, &pac1->map_events_config,
            &pac2->map_events_config, data_map_events_config);

    return (boolean) ret;

}

NB_Error
data_analytics_config_copy(data_util_state* pds, data_analytics_config* pac_dest, data_analytics_config* pac_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, pac_dest, data_analytics_config);

    pac_dest->max_hold_time = pac_src->max_hold_time;
    pac_dest->max_data_age = pac_src->max_data_age;
    pac_dest->max_request_size = pac_src->max_request_size;

    DATA_COPY(pds, err, &pac_dest->gold_events_config, &pac_src->gold_events_config,
            data_gold_events_config);

    DATA_COPY(pds, err, &pac_dest->poi_events_config,
            &pac_src->poi_events_config, data_poi_events_config);

    DATA_COPY(pds, err, &pac_dest->route_tracking_config,
            &pac_src->route_tracking_config, data_route_tracking_config);

    DATA_COPY(pds, err, &pac_dest->gps_probes_config,
            &pac_src->gps_probes_config, data_gps_probes_config);

    DATA_COPY(pds, err, &pac_dest->wifi_probes_config,
            &pac_src->wifi_probes_config, data_wifi_probes_config);

    DATA_COPY(pds, err, &pac_dest->coupon_events_config,
            &pac_src->coupon_events_config, data_coupon_events_config);

    DATA_COPY(pds, err, &pac_dest->nav_invocation_config,
            &pac_src->nav_invocation_config, data_nav_invocation_config);

    DATA_COPY(pds, err, &pac_dest->feedback_config,
            &pac_src->feedback_config, data_feedback_config);

    DATA_COPY(pds, err, &pac_dest->app_errors_config,
            &pac_src->app_errors_config, data_app_errors_config);

    DATA_COPY(pds, err, &pac_dest->asr_event_config,
            &pac_src->asr_event_config, data_asr_event_config);

    DATA_COPY(pds, err, &pac_dest->poiext_event_config,
            &pac_src->poiext_event_config, data_poiext_event_config);

    DATA_COPY(pds, err, &pac_dest->session_config,
            &pac_src->session_config, data_session_config);

    DATA_COPY(pds, err, &pac_dest->settings_events_config,
            &pac_src->settings_events_config, data_settings_events_config);

    DATA_COPY(pds, err, &pac_dest->share_events_config,
            &pac_src->share_events_config, data_share_events_config);

    DATA_COPY(pds, err, &pac_dest->user_actions_events_config,
            &pac_src->user_actions_events_config, data_user_actions_events_config);

    DATA_COPY(pds, err, &pac_dest->transaction_events_config,
            &pac_src->transaction_events_config, data_transaction_events_config);

    DATA_COPY(pds, err, &pac_dest->map_events_config,
            &pac_src->map_events_config, data_map_events_config);
    return err;
}
/*! @} */

