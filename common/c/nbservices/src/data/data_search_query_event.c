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

    @file     data_search_query_event.c
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
#include "data_search_query_event.h"

NB_Error
data_search_query_event_init(data_util_state* pds,
        data_search_query_event* psqe)
{
    NB_Error err = NE_OK;

    DATA_INIT(pds, err, &psqe->search_filter, data_search_filter);
    DATA_INIT(pds, err, &psqe->point, data_point);
    DATA_INIT(pds, err, &psqe->search_event_cookie, data_search_event_cookie);
    DATA_INIT(pds, err, &psqe->scheme, data_string);
    DATA_INIT(pds, err, &psqe->input_method, data_string);

    return err;
}

void
data_search_query_event_free(data_util_state* pds,
        data_search_query_event* psqe)
{
    DATA_FREE(pds, &psqe->search_filter, data_search_filter);
    DATA_FREE(pds, &psqe->point, data_point);
    DATA_FREE(pds, &psqe->search_event_cookie, data_search_event_cookie);
    DATA_FREE(pds, &psqe->scheme, data_string);
    DATA_FREE(pds, &psqe->input_method, data_string);
}

tpselt
data_search_query_event_to_tps(data_util_state* pds,
        data_search_query_event* psqe)
{
    tpselt te = NULL;
    tpselt ce = NULL;

    const char* input_method = NULL;

    te = te_new("search-query-event");

    if (te == NULL)
    {
        goto errexit;
    }

    if ((ce = data_search_filter_to_tps(pds, &psqe->search_filter)) != NULL &&
        te_attach(te, ce))
    {
        ce = NULL;
    }
    else
    {
        goto errexit;
    }

    if ((ce = data_point_to_tps(pds, &psqe->point)) != NULL &&
        te_attach(te, ce))
    {
        ce = NULL;
    }
    else
    {
        goto errexit;
    }

    if (psqe->search_event_cookie.provider_id)
    {
        if ((ce = data_search_event_cookie_to_tps(pds, &psqe->search_event_cookie)) != NULL &&
            te_attach(te, ce))
        {
            ce = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (!te_setattrc(te, "scheme", data_string_get(pds, &psqe->scheme)))
        goto errexit;

    input_method = data_string_get(pds, &psqe->input_method);
    if (input_method && *input_method)
    {
        if (!te_setattrc(te, "input-method", input_method))
            goto errexit;
    }

    return te;

errexit:

    te_dealloc(te);
    return NULL;
}

boolean
data_search_query_event_equal(data_util_state* pds,
        data_search_query_event* psqe1, data_search_query_event* psqe2)
{
    int ret = TRUE;

    DATA_EQUAL(pds, ret, &psqe1->search_filter, &psqe2->search_filter, data_search_filter);
    DATA_EQUAL(pds, ret, &psqe1->point, &psqe2->point, data_point);
    DATA_EQUAL(pds, ret, &psqe1->search_event_cookie, &psqe2->search_event_cookie, data_search_event_cookie);
    DATA_EQUAL(pds, ret, &psqe1->scheme, &psqe2->scheme, data_string);
    DATA_EQUAL(pds, ret, &psqe1->input_method, &psqe2->input_method, data_string);

    return (boolean) ret;
}

NB_Error
data_search_query_event_copy(data_util_state* pds,
        data_search_query_event* psqe_dest, data_search_query_event* psqe_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, psqe_dest, data_search_query_event);

    DATA_COPY(pds, err, &psqe_dest->search_filter, &psqe_src->search_filter, data_search_filter);
    DATA_COPY(pds, err, &psqe_dest->point, &psqe_src->point, data_point);
    DATA_COPY(pds, err, &psqe_dest->search_event_cookie, &psqe_src->search_event_cookie, data_search_event_cookie);
    DATA_COPY(pds, err, &psqe_dest->scheme, &psqe_src->scheme, data_string);
    DATA_COPY(pds, err, &psqe_dest->input_method, &psqe_src->input_method, data_string);

    return err;
}

uint32
data_search_query_event_get_tps_size(data_util_state* pds,
        data_search_query_event* psqe)
{
    uint32 size = 0;

    size += data_search_filter_get_tps_size(pds, &psqe->search_filter);
    size += data_point_get_tps_size(pds, &psqe->point);
    size += data_search_event_cookie_get_tps_size(pds, &psqe->search_event_cookie);
    size += data_string_get_tps_size(pds, &psqe->scheme);
    size += data_string_get_tps_size(pds, &psqe->input_method);

    return size;
}

void
data_search_query_event_to_buf(data_util_state* pds,
            data_search_query_event* psqe,
            struct dynbuf* pdb)
{
    data_search_filter_to_buf(pds, &psqe->search_filter, pdb);
    data_point_to_buf(pds, &psqe->point, pdb);
    data_search_event_cookie_to_buf(pds, &psqe->search_event_cookie, pdb);
    data_string_to_buf(pds, &psqe->scheme, pdb);
    data_string_to_buf(pds, &psqe->input_method, pdb);
}

NB_Error
data_search_query_event_from_binary(data_util_state* pds,
            data_search_query_event* psqe,
            byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_search_filter_from_binary(pds, &psqe->search_filter, pdata, pdatalen);
    err = err ? err : data_point_from_binary(pds, &psqe->point, pdata, pdatalen);
    err = err ? err : data_search_event_cookie_from_binary(pds, &psqe->search_event_cookie, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &psqe->scheme, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &psqe->input_method, pdata, pdatalen);

    return err;
}


/*! @} */

