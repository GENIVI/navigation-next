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

    @file     data_coupon_analytics_action_event.c
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

/*! @{ */

#include "data_coupon_analytics_action_event.h"

NB_Error
data_coupon_analytics_action_event_init(data_util_state* pds, data_coupon_analytics_action_event* pae)
{
    NB_Error err = NE_OK;

    DATA_INIT(pds, err, &pae->coupon_data, data_coupon_data);

    pae->store_data = NULL;

    DATA_INIT(pds, err, &pae->action, data_string);

    return err;
}

void
data_coupon_analytics_action_event_free(data_util_state* pds, data_coupon_analytics_action_event* pae)
{
    DATA_FREE(pds, &pae->coupon_data, data_coupon_data);

    DATA_PTR_FREEIF(pds, pae->store_data, data_store_data);

    DATA_FREE(pds, &pae->action, data_string);
}

boolean
data_coupon_analytics_action_event_equal(data_util_state* pds, data_coupon_analytics_action_event* pae1, data_coupon_analytics_action_event* pae2)
{
    // compare store_data
    if (pae1->store_data)
    {
        if (pae2->store_data)
        {
            if (!data_store_data_equal(pds, pae1->store_data, pae2->store_data))
            {
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        if (pae2->store_data)
        {
            return FALSE;
        }
    }

    return (boolean)data_coupon_data_equal(pds, &pae1->coupon_data, &pae2->coupon_data)
        && (boolean)data_string_equal(pds, &pae1->action, &pae2->action);
}

NB_Error
data_coupon_analytics_action_event_copy(data_util_state* pds, data_coupon_analytics_action_event* pae_dest, data_coupon_analytics_action_event* pae_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, pae_dest, data_coupon_analytics_action_event);

    DATA_COPY(pds, err, &pae_dest->coupon_data, &pae_src->coupon_data, data_coupon_data);
    
    if (pae_src->store_data)
    {
        pae_dest->store_data = (data_store_data*)nsl_malloc(sizeof(data_store_data));
        if (!pae_dest->store_data)
        {
            return NE_NOMEM;
        }
        data_store_data_init(pds, pae_dest->store_data);
        DATA_COPY(pds, err, pae_dest->store_data, pae_src->store_data, data_store_data);
    }

    DATA_COPY(pds, err, &pae_dest->action, &pae_src->action, data_string);

    return err;
}

tpselt
data_coupon_analytics_action_event_to_tps(data_util_state* pds, data_coupon_analytics_action_event* pae)
{
    tpselt te = NULL;
    tpselt ce = NULL;

    DATA_ALLOC_TPSELT(errexit, te, "coupon-analytics-action-event");

    if ((ce = data_coupon_data_to_tps(pds, &pae->coupon_data)) != NULL &&
        te_attach(te, ce))
    {
        ce = NULL;
    }
    else
    {
        goto errexit;
    }

    if (pae->store_data)
    {
        if ((ce = data_store_data_to_tps(pds, pae->store_data)) != NULL &&
            te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    DATA_STR_SETATTR(pds, errexit, te, "action", &pae->action );

    return te;

errexit:
    te_dealloc(te);
    return NULL;
}

uint32
data_coupon_analytics_action_event_get_tps_size(data_util_state* pds, data_coupon_analytics_action_event* pae)
{
    uint32 size = 0;

    size += data_coupon_data_get_tps_size(pds, &pae->coupon_data);

    if (pae->store_data)
    {
        size += data_store_data_get_tps_size(pds, pae->store_data);
    }

    size += data_string_get_tps_size(pds, &pae->action);

    return size;
}

void
data_coupon_analytics_action_event_to_buf(data_util_state* pds, data_coupon_analytics_action_event* pae, struct dynbuf* pdb)
{
    boolean hasStoreData = (pae->store_data) ? TRUE : FALSE;

    data_coupon_data_to_buf(pds, &pae->coupon_data, pdb);

    dbufcat(pdb, (const byte*)&hasStoreData, sizeof(hasStoreData));
    if (hasStoreData)
    {
        data_store_data_to_buf(pds, pae->store_data, pdb);
    }

    data_string_to_buf(pds, &pae->action, pdb);
}

NB_Error
data_coupon_analytics_action_event_from_binary(data_util_state* pds, data_coupon_analytics_action_event* pae, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;
    boolean hasStoreData = FALSE;

    err = (err) ? err : data_coupon_data_from_binary(pds, &pae->coupon_data, pdata, pdatalen);

    err = (err) ? err : data_boolean_from_binary(pds, &hasStoreData, pdata, pdatalen);
    if (hasStoreData)
    {
        if (!pae->store_data)
        {
            pae->store_data = (data_store_data*)nsl_malloc(sizeof(data_store_data));
            if (!pae->store_data)
            {
                return NE_NOMEM;
            }
            data_store_data_init(pds, pae->store_data);
        }
        err = (err) ? err : data_store_data_from_binary(pds, pae->store_data, pdata, pdatalen);
    }

    err = (err) ? err : data_string_from_binary(pds, &pae->action, pdata, pdatalen);

    return err;
}

/*! @} */
