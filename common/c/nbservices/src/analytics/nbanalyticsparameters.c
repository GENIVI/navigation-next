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

    @file     nbanalyticsparameters.c
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


#include "nbcontextprotected.h"
#include "nbanalyticsparameters.h"
#include "data_analytics_events_query.h"
#include "data_analytics_event.h"
#include "datautil.h"

struct NB_AnalyticsParameters {
    NB_Context*                 context;

    data_analytics_events_query query;
};

NB_Error
NB_AnalyticsParametersCreate(NB_Context* context, NB_AnalyticsWantAnalyticsConfig* configuration, NB_AnalyticsParameters** parameters)
{
    NB_AnalyticsParameters* pThis = 0;
    data_util_state* pds = NULL;
    NB_Error err = NE_OK;

    if (!context || !parameters)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }

    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;

    pds = NB_ContextGetDataState(pThis->context);
    err = data_analytics_events_query_init(pds, &pThis->query);
    if (configuration)
    {
        pThis->query.want_analytics_config.event_categories.gold_category = configuration->wantGoldCategory;
        pThis->query.want_analytics_config.event_categories.poi_category = configuration->wantPoiCategory;
        pThis->query.want_analytics_config.event_categories.route_tracking_category = configuration->wantRouteTrackingCategory;
        pThis->query.want_analytics_config.event_categories.gps_probes_category = configuration->wantGpsProbesCategory;
        pThis->query.want_analytics_config.event_categories.wifi_probes_category = configuration->wantWifiProbesCategory;
        pThis->query.want_analytics_config.event_categories.feedback_category = configuration->wantFeedbackCategory;
        pThis->query.want_analytics_config.event_categories.app_errors_category = configuration->wantAppErrorsCategory;
        pThis->query.want_analytics_config.event_categories.session_category = configuration->wantSessionCategory;
        pThis->query.want_analytics_config.event_categories.settings_category = configuration->wantSettingsCategory;
        pThis->query.want_analytics_config.event_categories.share_category = configuration->wantShareCategory;
        pThis->query.want_analytics_config.event_categories.user_actions_category = configuration->wantUserActionsCategory;
        pThis->query.want_analytics_config.event_categories.transaction_category = configuration->wantTransactionCategory;
        pThis->query.want_analytics_config.event_categories.map_category = configuration->wantMapCategory;
        pThis->query.want_analytics_config.event_categories.coupon_category = FALSE;
        pThis->query.want_analytics_config.event_categories.nav_invocation_category = FALSE;
    }

    if (!err)
    {
        *parameters = pThis;
    }
    else
    {
        nsl_free(pThis);
    }

    return err;
}

NB_Error
NB_AnalyticsParametersDestroy(NB_AnalyticsParameters* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    data_analytics_events_query_free(NB_ContextGetDataState(pThis->context),
            &pThis->query);

    nsl_free(pThis);

    return NE_OK;
}

NB_Error
NB_AnalyticsParametersAddEvent( NB_AnalyticsParameters* pThis,
        data_analytics_event* event)
{
    NB_Error err = NE_OK;
    data_util_state* pds = NB_ContextGetDataState(pThis->context);

    err = data_analytics_events_query_add_event(pds, &pThis->query, event);

    return err;
}

uint32
NB_AnalyticsParametersGetSize(NB_AnalyticsParameters* pThis)
{
    data_util_state* pds = NB_ContextGetDataState(pThis->context);
    return data_analytics_events_query_get_tps_size(pds, &pThis->query);
}

tpselt
NB_AnalyticsParametersToTPSQuery(NB_AnalyticsParameters* pThis)
{
    data_util_state* pds = NB_ContextGetDataState(pThis->context);
    return data_analytics_events_query_to_tps(pds, &pThis->query);
}

/*! @} */

