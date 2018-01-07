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

    @file     data_search_event_cookie.c
*/
/*
    See file description in header file.

    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

/*! @{ */

#include "data_search_event_cookie.h"

NB_DEF NB_Error
data_search_event_cookie_init(data_util_state* pds, data_search_event_cookie* psec)
{
    NB_Error err = NE_OK;

    err = err ? err : data_string_init(pds, &psec->provider_id);
    err = err ? err : data_blob_init(pds, &psec->state);

    return err;
}

NB_DEF void
data_search_event_cookie_free(data_util_state* pds, data_search_event_cookie* psec)
{
    data_string_free(pds, &psec->provider_id);
    data_blob_free(pds, &psec->state);
}

NB_DEF NB_Error
data_search_event_cookie_from_tps(data_util_state* pds, data_search_event_cookie* psec, tpselt te)
{
    NB_Error err = NE_OK;

    if (te == NULL)
    {
        err = NE_INVAL;
        return err;
    }

    data_search_event_cookie_free(pds, psec);

    err = data_search_event_cookie_init(pds, psec);

    if (err != NE_OK)
    {
        return err;
    }

    err = err ? err : data_string_from_tps_attr(pds, &psec->provider_id, te, "provider-id");
    err = err ? err : data_blob_from_tps_attr(pds, &psec->state, te, "state");

    if (err != NE_OK)
    {
        data_search_event_cookie_free(pds, psec);
    }

    return err;
}

NB_DEF boolean      
data_search_event_cookie_equal(data_util_state* pds, data_search_event_cookie* psec1, data_search_event_cookie* psec2)
{
    return (boolean) (data_string_equal(pds, &psec1->provider_id, &psec2->provider_id) &&
                        data_blob_equal(pds, &psec1->state, &psec2->state));
}

NB_DEF NB_Error    
data_search_event_cookie_copy(data_util_state* pds, data_search_event_cookie* psec_dest, data_search_event_cookie* psec_src)
{
    NB_Error err = NE_OK;

    data_search_event_cookie_free(pds, psec_dest);

    err = err ? err : data_search_event_cookie_init(pds, psec_dest);
    err = err ? err : data_string_copy(pds, &psec_dest->provider_id, &psec_src->provider_id);
    err = err ? err : data_blob_copy(pds, &psec_dest->state, &psec_src->state);

    return err;
}

NB_DEF tpselt
data_search_event_cookie_to_tps(data_util_state* pds, data_search_event_cookie* psec)
{
    tpselt te;

    te = te_new("search-event-cookie");

    if (te == NULL)
    {
        goto errexit;
    }

    if (!te_setattrc(te, "provider-id", data_string_get(pds, &psec->provider_id)))
    {
        goto errexit;
    }

    if (!te_setattr(te, "state", (const char*) psec->state.data, psec->state.size))
    {
        goto errexit;
    }

    return te;

errexit:

    te_dealloc(te);
    return NULL;
}

NB_DEF uint32
data_search_event_cookie_get_tps_size(data_util_state* pds, data_search_event_cookie* psec)
{
    uint32 size = 0;

    if (psec->provider_id)
    {
        size += nsl_strlen(psec->provider_id);
    }

    size += psec->state.size;

    return size;
}

NB_DEF void
data_search_event_cookie_to_buf(data_util_state* pds,data_search_event_cookie* psec, struct dynbuf* pdb)
{
    data_string_to_buf(pds, &psec->provider_id, pdb);
    data_blob_to_buf(pds, &psec->state, pdb);
}

NB_DEF NB_Error
data_search_event_cookie_from_binary(data_util_state* pds, data_search_event_cookie* psec, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_string_from_binary(pds, &psec->provider_id, pdata, pdatalen);

    err = err ? err : data_blob_from_binary(pds, &psec->state, pdata, pdatalen);

    return err;
}

/*! @} */
