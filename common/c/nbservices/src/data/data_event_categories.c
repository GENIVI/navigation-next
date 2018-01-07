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

    @file     data_event_categories.c
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
#include "data_event_categories.h"

NB_Error
data_event_categories_init(data_util_state* pds, data_event_categories* pec)
{
    NB_Error err = NE_OK;

    pec->gold_category = TRUE;
    pec->poi_category = TRUE;
    pec->route_tracking_category = TRUE;
    pec->gps_probes_category = TRUE;
    pec->wifi_probes_category = TRUE;
    pec->feedback_category = TRUE;
    pec->app_errors_category = TRUE;
    pec->coupon_category = FALSE;
    pec->nav_invocation_category = FALSE;
    pec->session_category = TRUE;
    pec->settings_category = TRUE;
    pec->share_category = TRUE;
    pec->user_actions_category = TRUE;
    pec->transaction_category = TRUE;
    pec->map_category = TRUE;
    return err;
}

void
data_event_categories_free(data_util_state* pds, data_event_categories* pec)
{
    /* nothing to free */
}

tpselt
data_event_categories_to_tps(data_util_state* pds, data_event_categories* pec)
{
    tpselt te = NULL;
    tpselt ce = NULL;
    tpselt cce = NULL;

    te = te_new("event-categories");

    if (te == NULL)
    {
        goto errexit;
    }

    if (pec->gold_category)
    {
        if ((ce = te_new("gold-category")) != NULL && te_attach(te, ce))
        {
            if ((cce = te_new("want-max-hold-time")) != NULL && te_attach(ce, cce))
            {
                cce = NULL;
            }
            else
            {
                goto errexit;
            }
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (pec->poi_category)
    {
        if ((ce = te_new("poi-category")) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (pec->route_tracking_category)
    {
        if ((ce = te_new("route-tracking-category")) != NULL &&
                te_attach(te, ce))
        {
            if ((cce = te_new("want-route-tracking-interval")) != NULL && te_attach(ce, cce))
            {
                cce = NULL;
            }
            else
            {
                goto errexit;
            }
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (pec->gps_probes_category)
    {
        if ((ce = te_new("gps-probes-category")) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (pec->wifi_probes_category)
    {
        if ((ce = te_new("wifi-probes-category")) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (pec->coupon_category)
    {
        if ((ce = te_new("coupons-category")) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (pec->nav_invocation_category)
    {
        if ((ce = te_new("nav-invocation-category")) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (pec->feedback_category)
    {
        if ((ce = te_new("feedback-category")) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (pec->app_errors_category)
    {
        if ((ce = te_new("app-errors-category")) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (pec->session_category)
    {
        if ((ce = te_new("session-category")) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (pec->app_errors_category)
    {
        if ((ce = te_new("settings-category")) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (pec->app_errors_category)
    {
        if ((ce = te_new("share-category")) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (pec->app_errors_category)
    {
        if ((ce = te_new("user-action-category")) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (pec->app_errors_category)
    {
        if ((ce = te_new("transaction-category")) != NULL && te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (pec->app_errors_category)
    {
        if ((ce = te_new("maps-category")) != NULL && te_attach(te, ce))
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

    te_dealloc(te);
    te_dealloc(ce);
    te_dealloc(cce);
    return NULL;
}

boolean
data_event_categories_equal(data_util_state* pds,
        data_event_categories* pec1, data_event_categories* pec2)
{
    return (boolean)((pec1->gold_category == pec2->gold_category) &&
        (pec1->poi_category == pec2->poi_category) &&
        (pec1->route_tracking_category == pec2->route_tracking_category) &&
        (pec1->gps_probes_category == pec2->gps_probes_category) &&
        (pec1->wifi_probes_category == pec2->wifi_probes_category) &&
        (pec1->coupon_category == pec2->coupon_category) &&
        (pec1->nav_invocation_category == pec2->nav_invocation_category) &&
        (pec1->feedback_category == pec2->feedback_category) &&
        (pec1->app_errors_category == pec2->app_errors_category)&&
        (pec1->session_category == pec2->session_category)&&
        (pec1->settings_category == pec2->settings_category)&&
        (pec1->share_category == pec2->share_category)&&
        (pec1->user_actions_category == pec2->user_actions_category)&&
        (pec1->transaction_category == pec2->transaction_category)&&
        (pec1->map_category == pec2->map_category));
}

NB_Error
data_event_categories_copy(data_util_state* pds,
        data_event_categories* pec_dest, data_event_categories* pec_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, pec_dest, data_event_categories);

    pec_dest->gold_category = pec_src->gold_category;
    pec_dest->poi_category = pec_src->poi_category;
    pec_dest->route_tracking_category = pec_src->route_tracking_category;
    pec_dest->gps_probes_category = pec_src->gps_probes_category;
    pec_dest->wifi_probes_category = pec_src->wifi_probes_category;
    pec_dest->coupon_category = pec_src->coupon_category;
    pec_dest->nav_invocation_category = pec_src->nav_invocation_category;
    pec_dest->feedback_category = pec_src->feedback_category;
    pec_dest->app_errors_category = pec_src->app_errors_category;
    pec_dest->session_category = pec_src->session_category;
    pec_dest->settings_category = pec_src->settings_category;
    pec_dest->share_category = pec_src->share_category;
    pec_dest->user_actions_category = pec_src->user_actions_category;
    pec_dest->transaction_category = pec_src->transaction_category;
    pec_dest->map_category = pec_src->map_category;

    return err;
}

uint32 data_event_categories_get_tps_size(data_util_state* pds,
            data_event_categories* pec)
{
    uint32 size = 0;

    size += sizeof(pec->gold_category);
    size += sizeof(pec->poi_category);
    size += sizeof(pec->route_tracking_category);
    size += sizeof(pec->gps_probes_category);
    size += sizeof(pec->wifi_probes_category);
    size += sizeof(pec->coupon_category);
    size += sizeof(pec->nav_invocation_category);
    size += sizeof(pec->feedback_category);
    size += sizeof(pec->app_errors_category);
    size += sizeof(pec->session_category);
    size += sizeof(pec->settings_category);
    size += sizeof(pec->share_category);
    size += sizeof(pec->user_actions_category);
    size += sizeof(pec->transaction_category);
    size += sizeof(pec->map_category);
    return size;
}

/*! @} */

