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

   @file     data_feedback_event.c
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, Inc. is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "data_feedback_event.h"

NB_Error
data_feedback_event_init(data_util_state* pds, data_feedback_event* pfe)
{
    NB_Error err = NE_OK;

    DATA_MEM_ZERO(pfe, data_feedback_event);

    pfe->ppoi_place = NULL;
    pfe->pnav_origin = NULL;
    pfe->pnav_destination = NULL;

    DATA_INIT(pds, err, &pfe->entered_text, data_text);

    DATA_VEC_ALLOC(err, pfe->vec_issue_type, data_issue_type);

    DATA_INIT(pds, err, &pfe->screen_id, data_string);
    DATA_INIT(pds, err, &pfe->provider_id, data_string);
    DATA_INIT(pds, err, &pfe->poi_id, data_string);

    pfe->nav_session_id = 0;
    pfe->route_request_event_id = 0;
    pfe->search_query_event_id = 0;

    if (err)
    {
        DATA_FREE(pds, pfe, data_feedback_event);
    };

    return err;
}

void
data_feedback_event_free(data_util_state* pds, data_feedback_event* pfe)
{
    if (pfe->ppoi_place)
    {
        DATA_FREE(pds, pfe->ppoi_place, data_place);
        nsl_free(pfe->ppoi_place);
        pfe->ppoi_place = NULL;
    }

    if (pfe->pnav_origin)
    {
        DATA_FREE(pds, pfe->pnav_origin, data_origin);
        nsl_free(pfe->pnav_origin);
        pfe->pnav_origin = NULL;
    }

    if (pfe->pnav_destination)
    {
        DATA_FREE(pds, pfe->pnav_destination, data_destination);
        nsl_free(pfe->pnav_destination);
        pfe->pnav_destination = NULL;
    }

    DATA_FREE(pds, &pfe->entered_text, data_text);

    DATA_VEC_FREE(pds, pfe->vec_issue_type, data_issue_type);

    DATA_FREE(pds, &pfe->screen_id, data_string);
    DATA_FREE(pds, &pfe->provider_id, data_string);
    DATA_FREE(pds, &pfe->poi_id, data_string);

    pfe->nav_session_id = 0;
    pfe->route_request_event_id = 0;
    pfe->search_query_event_id = 0;
}


NB_Error
data_feedback_event_copy(data_util_state* pds, data_feedback_event* dst, data_feedback_event* src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, dst, data_feedback_event);

    if (src->ppoi_place)
    {
        dst->ppoi_place = nsl_malloc(sizeof(data_place));
        if (NULL != dst->ppoi_place)
        {
            DATA_INIT(pds, err, dst->ppoi_place, data_place);
            DATA_COPY(pds, err, dst->ppoi_place, src->ppoi_place, data_place);
        }
        else
        {
            err = NE_NOMEM;
            goto errexit;
        }
    }

    if (!err && src->pnav_origin)
    {
        dst->pnav_origin = nsl_malloc(sizeof(data_origin));
        if (NULL != dst->pnav_origin)
        {
            DATA_INIT(pds, err, dst->pnav_origin, data_origin);
            DATA_COPY(pds, err, dst->pnav_origin, src->pnav_origin, data_origin);
        }
        else
        {
            err = NE_NOMEM;
            goto errexit;
        }
    }

    if (!err && src->pnav_destination)
    {
        dst->pnav_destination = nsl_malloc(sizeof(data_destination));
        if (NULL != dst->pnav_destination)
        {
            DATA_INIT(pds, err, dst->pnav_destination, data_destination);
            DATA_COPY(pds, err, dst->pnav_destination, src->pnav_destination, data_destination);
        }
        else
        {
            err = NE_NOMEM;
            goto errexit;
        }
    }

    DATA_COPY(pds, err, &dst->entered_text, &src->entered_text, data_text);

    DATA_VEC_COPY(pds, err, dst->vec_issue_type, src->vec_issue_type, data_issue_type);

    DATA_COPY(pds, err, &dst->screen_id, &src->screen_id, data_string);
    DATA_COPY(pds, err, &dst->provider_id, &src->provider_id, data_string);
    DATA_COPY(pds, err, &dst->poi_id, &src->poi_id, data_string);

    dst->nav_session_id = src->nav_session_id;
    dst->route_request_event_id = src->route_request_event_id;
    dst->search_query_event_id = src->search_query_event_id;

errexit:
    if (err)
    {
        if (NULL != dst->ppoi_place)
        {
            DATA_FREE(pds, dst->ppoi_place, data_place);
            nsl_free(dst->ppoi_place);
            dst->ppoi_place = NULL;
        }

        if (NULL != dst->pnav_origin)
        {
            DATA_FREE(pds, dst->pnav_origin, data_origin);
            nsl_free(dst->pnav_origin);
            dst->pnav_origin = NULL;
        }

        if (NULL != dst->pnav_destination)
        {
            DATA_FREE(pds, dst->pnav_destination, data_destination);
            nsl_free(dst->pnav_destination);
            dst->pnav_destination = NULL;
        }
    }

    return err;
}

boolean
data_feedback_event_equal(data_util_state* pds, data_feedback_event* pfe1, data_feedback_event* pfe2)
{
    int ret = TRUE;

    ret = (pfe1->nav_session_id == pfe2->nav_session_id);
    ret = ret ? (pfe1->route_request_event_id == pfe2->route_request_event_id) : ret;
    ret = ret ? (pfe1->search_query_event_id == pfe2->search_query_event_id) : ret;

    if (pfe1->ppoi_place && pfe2->ppoi_place)
    {
        DATA_EQUAL(pds, ret, pfe1->ppoi_place, pfe2->ppoi_place, data_place);
    }
    else if(pfe1->ppoi_place || pfe2->ppoi_place)
    {
        ret = FALSE;
    }

    if (pfe1->pnav_origin && pfe2->pnav_origin)
    {
        DATA_EQUAL(pds, ret, pfe1->pnav_origin, pfe2->pnav_origin, data_origin);
    }
    else if(pfe1->pnav_origin || pfe2->pnav_origin)
    {
        ret = FALSE;
    }

    if (pfe1->pnav_destination && pfe2->pnav_destination)
    {
        DATA_EQUAL(pds, ret, pfe1->pnav_destination, pfe2->pnav_destination, data_destination);
    }
    else if(pfe1->pnav_destination || pfe2->pnav_destination)
    {
        ret = FALSE;
    }

    DATA_EQUAL(pds, ret, &pfe1->entered_text, &pfe2->entered_text, data_text);

    DATA_VEC_EQUAL(pds, ret, pfe1->vec_issue_type, pfe2->vec_issue_type, data_issue_type);

    DATA_EQUAL(pds, ret, &pfe1->screen_id, &pfe2->screen_id, data_string);
    DATA_EQUAL(pds, ret, &pfe1->provider_id, &pfe2->provider_id, data_string);
    DATA_EQUAL(pds, ret, &pfe1->poi_id, &pfe2->poi_id, data_string);

    return (boolean)ret;
}


tpselt
data_feedback_event_to_tps(data_util_state* pds, data_feedback_event* pfe)
{
    tpselt te = NULL;

    te = te_new("feedback-event");

    if (te == NULL)
    {
        goto errexit;
    }

    if (!te_setattrc(te, "screen-id", data_string_get(pds, &pfe->screen_id)))
    {
        goto errexit;
    }

    if (!te_setattru(te, "nav-session-id", pfe->nav_session_id))
    {
        goto errexit;
    }

    if (!te_setattru(te, "route-request-event-id", pfe->route_request_event_id))
    {
        goto errexit;
    }

    if (!te_setattru(te, "search-query-event-id", pfe->search_query_event_id))
    {
        goto errexit;
    }

    if (!te_setattrc(te, "provider-id", data_string_get(pds, &pfe->provider_id)))
    {
        goto errexit;
    }

    if (!te_setattrc(te, "poi-id", data_string_get(pds, &pfe->poi_id)))
    {
        goto errexit;
    }

    if (pfe->vec_issue_type)
    {
        DATA_VEC_TO_TPS(pds,errexit,te,pfe->vec_issue_type,data_issue_type);
    }

    if (pfe->ppoi_place)
    {
        DATA_TO_TPS(pds, errexit, te, pfe->ppoi_place, data_place);
    }

    if (pfe->pnav_origin)
    {
        DATA_TO_TPS(pds, errexit, te, pfe->pnav_origin, data_origin);
    }

    if (pfe->pnav_destination)
    {
        DATA_TO_TPS(pds, errexit, te, pfe->pnav_destination, data_destination);
    }

    if (pfe->entered_text.data)
    {
        DATA_TO_TPS(pds, errexit, te, &pfe->entered_text, data_text);
    }

    return te;

errexit:

    te_dealloc(te);
    return NULL;
}

NB_Error
data_feedback_event_add_issue_type(data_util_state* pds, data_feedback_event* pfe, const char* val)
{
    NB_Error err = NE_OK;

    data_issue_type issue_type = {0};

    DATA_INIT(pds, err, &issue_type, data_issue_type);
    err = err ? err : data_string_set(pds, &issue_type.type, val);
    err = err ? err : CSL_VectorAppend(pfe->vec_issue_type, &issue_type) ? NE_OK : NE_NOMEM;

    if (err)
    {
        DATA_FREE(pds, &issue_type, data_issue_type);
    }

    return err;
}

uint32
data_feedback_event_get_tps_size(data_util_state* pds,
        data_feedback_event* pfe)
{
    uint32 size = 0;

    if (pfe->ppoi_place)
    {
        size += data_place_get_tps_size(pds, pfe->ppoi_place);
    }

    if (pfe->pnav_origin)
    {
        size += data_origin_get_tps_size(pds, pfe->pnav_origin);
    }

    if (pfe->pnav_destination)
    {
        size += data_destination_get_tps_size(pds, pfe->pnav_destination);
    }

    size += data_text_get_tps_size(pds, &pfe->entered_text);
    DATA_VEC_GET_TPS_SIZE(pds, size, pfe->vec_issue_type, data_issue_type);
    size += data_string_get_tps_size(pds, &pfe->screen_id);
    size += sizeof(pfe->nav_session_id);
    size += sizeof(pfe->route_request_event_id);
    size += sizeof(pfe->search_query_event_id);
    size += data_string_get_tps_size(pds, &pfe->provider_id);
    size += data_string_get_tps_size(pds, &pfe->poi_id);

    return size;
}

void
data_feedback_event_to_buf(data_util_state* pds,
            data_feedback_event* pfe,
            struct dynbuf* pdb)
{
    nb_boolean exist = FALSE;

    //optional place exist flag is stored to buf
    if (pfe->ppoi_place)
    {
        exist = TRUE;
        dbufcat(pdb, (const byte*)&exist, sizeof(exist));
        data_place_to_buf(pds, pfe->ppoi_place, pdb);
    }
    else
    {
        exist = FALSE;
        dbufcat(pdb, (const byte*)&exist, sizeof(exist));
    }

    //optional origin exist flag is stored to buf
    if (pfe->pnav_origin)
    {
        exist = TRUE;
        dbufcat(pdb, (const byte*)&exist, sizeof(exist));
        data_origin_to_buf(pds, pfe->pnav_origin, pdb);
    }
    else
    {
        exist = FALSE;
        dbufcat(pdb, (const byte*)&exist, sizeof(exist));
    }

    //optional destination exist flag is stored to buf
    if (pfe->pnav_destination)
    {
        exist = TRUE;
        dbufcat(pdb, (const byte*)&exist, sizeof(exist));
        data_destination_to_buf(pds, pfe->pnav_destination, pdb);
    }
    else
    {
        exist = FALSE;
        dbufcat(pdb, (const byte*)&exist, sizeof(exist));
    }

    data_text_to_buf(pds, &pfe->entered_text, pdb);
    DATA_VEC_TO_BUF(pds, pdb, pfe->vec_issue_type, data_issue_type);
    data_string_to_buf(pds, &pfe->screen_id, pdb);
    dbufcat(pdb, (const byte*)&pfe->nav_session_id, sizeof(pfe->nav_session_id));
    dbufcat(pdb, (const byte*)&pfe->route_request_event_id, sizeof(pfe->route_request_event_id));
    dbufcat(pdb, (const byte*)&pfe->search_query_event_id, sizeof(pfe->search_query_event_id));
    data_string_to_buf(pds, &pfe->provider_id, pdb);
    data_string_to_buf(pds, &pfe->poi_id, pdb);
}

NB_Error
data_feedback_event_from_binary(data_util_state* pds,
            data_feedback_event* pfe,
            byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;
    nb_boolean exist = FALSE;

    //read place exist flag
    err = data_boolean_from_binary(pds, &exist, pdata, pdatalen);
    if (!err && exist)
    {
        pfe->ppoi_place = nsl_malloc(sizeof(data_place));
        if (NULL != pfe->ppoi_place)
        {
            DATA_INIT(pds, err, pfe->ppoi_place, data_place);
            err = err ? err : data_place_from_binary(pds, pfe->ppoi_place, pdata, pdatalen);
        }
        else
        {
            err = NE_NOMEM;
            goto errexit;
        }
    }

    //read origin exist flag
    err = err ? err : data_boolean_from_binary(pds, &exist, pdata, pdatalen);
    if (!err && exist)
    {
        pfe->pnav_origin = nsl_malloc(sizeof(data_origin));
        if (NULL != pfe->pnav_origin)
        {
            DATA_INIT(pds, err, pfe->pnav_origin, data_origin);
            err = err ? err : data_origin_from_binary(pds, pfe->pnav_origin, pdata, pdatalen);
        }
        else
        {
            err = NE_NOMEM;
            goto errexit;
        }
    }

    //read destination exist flag
    err = err ? err : data_boolean_from_binary(pds, &exist, pdata, pdatalen);
    if (!err && exist)
    {
        pfe->pnav_destination = nsl_malloc(sizeof(data_destination));
        if (NULL != pfe->pnav_destination)
        {
            DATA_INIT(pds, err, pfe->pnav_destination, data_destination);
            err = err ? err : data_destination_from_binary(pds, pfe->pnav_destination, pdata, pdatalen);
        }
        else
        {
            err = NE_NOMEM;
            goto errexit;
        }
    }

    err = err ? err : data_text_from_binary(pds, &pfe->entered_text, pdata, pdatalen);
    DATA_VEC_FROM_BINARY(pds, err, pdata, pdatalen, pfe->vec_issue_type, data_issue_type);
    err = err ? err : data_string_from_binary(pds, &pfe->screen_id, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pfe->nav_session_id, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pfe->route_request_event_id, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pfe->search_query_event_id, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pfe->provider_id, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pfe->poi_id, pdata, pdatalen);

errexit:
    if (err)
    {
        if (NULL != pfe->ppoi_place)
        {
            DATA_FREE(pds, pfe->ppoi_place, data_place);
            nsl_free(pfe->ppoi_place);
            pfe->ppoi_place = NULL;
        }

        if (NULL != pfe->pnav_origin)
        {
            DATA_FREE(pds, pfe->pnav_origin, data_origin);
            nsl_free(pfe->pnav_origin);
            pfe->pnav_origin = NULL;
        }

        if (NULL != pfe->pnav_destination)
        {
            DATA_FREE(pds, pfe->pnav_destination, data_destination);
            nsl_free(pfe->pnav_destination);
            pfe->pnav_destination = NULL;
        }
    }

    return err;
}

/*! @} */
