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

    @file     data_route_state_event.c
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
#include "data_route_state_event.h"

NB_Error
data_route_state_event_init(data_util_state* pds, data_route_state_event* prse)
{
    NB_Error err = NE_OK;

    prse->nav_session_id = 0;
    prse->route_request_event_id = 0;
    prse->route_reply_event_id = 0;
    prse->duration = 0;
    prse->distance = 0;
    prse->vec_content_displayed = CSL_VectorAlloc(sizeof(data_content_displayed));
    DATA_INIT(pds, err, &prse->state, data_string);
    DATA_INIT(pds, err, &prse->route_id, data_blob);
    prse->distance_traveled = 0;

    return err;
}

void
data_route_state_event_free(data_util_state* pds, data_route_state_event* prse)
{
    int l = 0;
    int n = 0;

    if (prse->vec_content_displayed)
    {
        l = CSL_VectorGetLength(prse->vec_content_displayed);

        for (n = 0; n < l; n++)
        {
            DATA_FREE(pds, (data_content_displayed*) CSL_VectorGetPointer(prse->vec_content_displayed, n), data_content_displayed);
        }
        CSL_VectorDealloc(prse->vec_content_displayed);
        prse->vec_content_displayed = NULL;
    }

    DATA_FREE(pds, &prse->state, data_string);
    DATA_FREE(pds, &prse->route_id, data_blob);
}

tpselt
data_route_state_event_to_tps(data_util_state* pds,
        data_route_state_event* prse)
{
    tpselt te = NULL;
    tpselt ce = NULL;
    int l = 0;
    int n = 0;

    te = te_new("route-state-event");

    if (te == NULL)
    {
        goto errexit;
    }

    if (!te_setattru(te, "nav-session-id", prse->nav_session_id))
    {
        goto errexit;
    }

    if (!te_setattru(te, "route-request-event-id", prse->route_request_event_id))
    {
        goto errexit;
    }

    if (!te_setattru(te, "route-reply-event-id", prse->route_reply_event_id))
    {
        goto errexit;
    }

    if (!te_setattrc(te, "state", data_string_get(pds, &prse->state)))
    {
        goto errexit;
    }

    if (!te_setattru(te, "duration", prse->duration))
    {
        goto errexit;
    }

    if (!te_setattrf(te, "distance", prse->distance))
    {
        goto errexit;
    }

    if (prse->vec_content_displayed)
    {
        l = CSL_VectorGetLength(prse->vec_content_displayed);

        for (n = 0; n < l; n++)
        {
            if ((ce = data_content_displayed_to_tps(pds, CSL_VectorGetPointer(prse->vec_content_displayed, n))) != NULL && te_attach(te, ce))
            {
                ce = NULL;
            }
            else
            {
                goto errexit;
            }
        }
    }

    if (!te_setattr(te, "route-id", (const char*)prse->route_id.data, prse->route_id.size))
    {
        goto errexit;
    }

    if (!te_setattrf(te, "distance-traveled", prse->distance_traveled))
    {
        goto errexit;
    }
    return te;

errexit:

    te_dealloc(te);
    return NULL;
}

boolean
data_route_state_event_equal(data_util_state* pds,
        data_route_state_event* prse1, data_route_state_event* prse2)
{
    int l1 = 0;
    int l2 = 0;
    int  n = 0;

    if (prse1->vec_content_displayed && prse2->vec_content_displayed)
    {
        l1 = CSL_VectorGetLength(prse1->vec_content_displayed);
        l2 = CSL_VectorGetLength(prse2->vec_content_displayed);

        if (l1 != l2)
        {
            return FALSE;
        }

        for (n = 0; n < l1; n++)
        {
            if (!data_content_displayed_equal(pds, (data_content_displayed*) CSL_VectorGetPointer(prse1->vec_content_displayed, n), (data_content_displayed*) CSL_VectorGetPointer(prse2->vec_content_displayed, n)))
                return FALSE;
        }
    }
    else if ((prse1->vec_content_displayed && !prse2->vec_content_displayed) || (!prse1->vec_content_displayed && prse2->vec_content_displayed))
    {
        return FALSE;
    }

    if ((prse1->nav_session_id == prse2->nav_session_id) &&
        (prse1->route_request_event_id == prse2->route_request_event_id) &&
        (prse1->route_reply_event_id == prse2->route_reply_event_id) &&
        data_string_equal(pds, &prse1->state, &prse2->state) &&
        (prse1->duration == prse2->duration) &&
        (prse1->distance == prse2->distance) &&
        data_blob_equal(pds, &prse1->route_id, &prse2->route_id) &&
        prse1->distance_traveled == prse2->distance_traveled)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

NB_Error
data_route_state_event_copy(data_util_state* pds,
        data_route_state_event* prse_dest, data_route_state_event* prse_src)
{
    NB_Error err = NE_OK;
    int l = 0;
    int n = 0;
    data_content_displayed pcd = {0};

    DATA_REINIT(pds, err, prse_dest, data_route_state_event);

    prse_dest->nav_session_id = prse_src->nav_session_id;
    prse_dest->route_request_event_id = prse_src->route_request_event_id;
    prse_dest->route_reply_event_id = prse_src->route_reply_event_id;
    DATA_COPY(pds, err, &prse_dest->state, &prse_src->state, data_string);
    prse_dest->duration = prse_src->duration;
    prse_dest->distance = prse_src->distance;

    DATA_INIT(pds, err, &pcd, data_content_displayed);

    if (prse_src->vec_content_displayed)
    {
        l = CSL_VectorGetLength(prse_src->vec_content_displayed);

        for (n = 0; n < l && err == NE_OK;n++)
        {
            DATA_COPY(pds, err, &pcd, CSL_VectorGetPointer(prse_src->vec_content_displayed, n), data_content_displayed);
            err = err ? err : CSL_VectorAppend(prse_dest->vec_content_displayed, &pcd) ? NE_OK : NE_NOMEM;

            if (err)
            {
                DATA_FREE(pds, &pcd, data_content_displayed);
            }
            else
            {
                DATA_INIT(pds, err, &pcd, data_content_displayed);
            }
        }
    }

    DATA_COPY(pds, err, &prse_dest->route_id, &prse_src->route_id, data_blob);
    prse_dest->distance_traveled = prse_src->distance_traveled;
    return err;
}

uint32
data_route_state_event_get_tps_size(data_util_state* pds,
            data_route_state_event* prse)
{
    uint32 size = 0;
    int l = 0;
    int n = 0;

    size += sizeof(prse->nav_session_id);
    size += sizeof(prse->route_request_event_id);
    size += sizeof(prse->route_reply_event_id);

    if (prse->state)
    {
        size += nsl_strlen(prse->state);
    }

    size += sizeof(prse->duration);
    size += sizeof(prse->distance);

    if (prse->vec_content_displayed)
    {
        l = CSL_VectorGetLength(prse->vec_content_displayed);

        for (n = 0; n < l; n++)
        {
            size += data_content_displayed_get_tps_size(pds, (data_content_displayed*)(CSL_VectorGetPointer(prse->vec_content_displayed, n)));
        }
    }

    size += prse->route_id.size;
    size += sizeof(prse->distance_traveled);

    return size;
}

void
data_route_state_event_to_buf(data_util_state* pds,
            data_route_state_event* prse,
            struct dynbuf* pdb)
{
    int len = 0;

    if (prse->vec_content_displayed)
    {
        int i = 0;

        len = CSL_VectorGetLength(prse->vec_content_displayed);
        dbufcat(pdb, (const byte*)&len, sizeof(len));

        for (i = 0; i < len; i++)
        {
            data_content_displayed* pcd = (data_content_displayed*)CSL_VectorGetPointer(prse->vec_content_displayed, i);
            data_content_displayed_to_buf(pds, pcd, pdb);
        }
    }
    else
    {
        dbufcat(pdb, (const byte*)&len, sizeof(len));
    }

    dbufcat(pdb, (const byte*) &prse->nav_session_id,
            sizeof(prse->nav_session_id));

    dbufcat(pdb, (const byte*) &prse->route_request_event_id,
            sizeof(prse->route_request_event_id));

    dbufcat(pdb, (const byte*) &prse->route_reply_event_id,
            sizeof(prse->route_reply_event_id));

    data_string_to_buf(pds, &prse->state, pdb);

    dbufcat(pdb, (const byte*) &prse->duration, sizeof(prse->duration));
    dbufcat(pdb, (const byte*) &prse->distance, sizeof(prse->distance));

    data_blob_to_buf(pds, &prse->route_id, pdb);
    dbufcat(pdb, (const byte*) &prse->distance_traveled, sizeof(prse->distance_traveled));
}

NB_Error
data_route_state_event_from_binary(data_util_state* pds,
            data_route_state_event* prse,
            byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;
    int len = 0;

    err = data_uint32_from_binary(pds, (uint32*)&len, pdata, pdatalen);
    if (err)
    {
        return err;
    }

    if (len > 0)
    {
        int i = 0;

        if(prse->vec_content_displayed == NULL)
        {
            prse->vec_content_displayed = CSL_VectorAlloc(sizeof(data_content_displayed));
        }

        if(prse->vec_content_displayed == NULL)
        {
            return NE_NOMEM;
        }

        for (i=0; i<len; i++)
        {
            data_content_displayed pcd = {0};

            DATA_INIT(pds, err, &pcd, data_content_displayed);
            if (err)
            {
                break;
            }

            err = data_content_displayed_from_binary(pds, &pcd, pdata, pdatalen);
            if (err)
            {
            	if (pcd.type) {
            		DATA_FREE(pds, &pcd.type, data_string);
            	}
            	if (pcd.country) {
            		DATA_FREE(pds, &pcd.country, data_string);
            	}
                break;
            }

            if (!CSL_VectorAppend(prse->vec_content_displayed, &pcd))
            {
                DATA_FREE(pds, &pcd, data_content_displayed);
                err = NE_NOMEM;
                break;
            }
        }
    }

    err = err ? err : data_uint32_from_binary(pds, &prse->nav_session_id,
                                              pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds,
                                              &prse->route_request_event_id,
                                              pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds,
                                              &prse->route_reply_event_id,
                                              pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds,
                                              &prse->state,
                                              pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &prse->duration,
                                              pdata, pdatalen);
    err = err ? err : data_double_from_binary(pds, &prse->distance,
                                              pdata, pdatalen);

    err = err ? err : data_blob_from_binary(pds, &prse->route_id,
                                            pdata, pdatalen);

    err = err ? err : data_double_from_binary(pds, &prse->distance_traveled,
                                              pdata, pdatalen);
    return err;
}


/*! @} */

