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

   @file     data_asr_event.c
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, Inc. is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "data_asr_event.h"


NB_Error
data_asr_event_init(data_util_state* pds, data_asr_event* pfe)
{
    NB_Error err = NE_OK;

    DATA_MEM_ZERO(pfe, data_asr_event);

    DATA_INIT(pds, err, &pfe->asr_screen, data_string);
    DATA_INIT(pds, err, &pfe->session_id, data_string);
    DATA_INIT(pds, err, &pfe->asr_text, data_string);
    DATA_INIT(pds, err, &pfe->provider_session_id, data_string);
    DATA_INIT(pds, err, &pfe->user_language, data_string);

    if (err)
    {
        DATA_FREE(pds, pfe, data_asr_event);
    }

    return err;
}

void
data_asr_event_free(data_util_state* pds, data_asr_event* pfe)
{
    DATA_FREE(pds, &pfe->asr_screen, data_string);
    DATA_FREE(pds, &pfe->session_id, data_string);
    DATA_FREE(pds, &pfe->asr_text, data_string);
    DATA_FREE(pds, &pfe->provider_session_id, data_string);
    DATA_FREE(pds, &pfe->user_language, data_string);
}

NB_Error
data_asr_event_copy(data_util_state* pds, data_asr_event* dst, data_asr_event* src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, dst, data_asr_event);

    DATA_COPY(pds, err, &dst->asr_screen, &src->asr_screen, data_string);
    DATA_COPY(pds, err, &dst->session_id, &src->session_id, data_string);
    DATA_COPY(pds, err, &dst->asr_text, &src->asr_text, data_string);
    DATA_COPY(pds, err, &dst->provider_session_id, &src->provider_session_id, data_string);
    DATA_COPY(pds, err, &dst->user_language, &src->user_language, data_string);

    return err;
}

boolean
data_asr_event_equal(data_util_state* pds, data_asr_event* pfe1, data_asr_event* pfe2)
{
    int ret = TRUE;

    DATA_EQUAL(pds, ret, &pfe1->asr_screen, &pfe2->asr_screen, data_string);
    DATA_EQUAL(pds, ret, &pfe1->session_id, &pfe2->session_id, data_string);
    DATA_EQUAL(pds, ret, &pfe1->asr_text, &pfe2->asr_text, data_string);
    DATA_EQUAL(pds, ret, &pfe1->provider_session_id, &pfe2->provider_session_id, data_string);
    DATA_EQUAL(pds, ret, &pfe1->user_language, &pfe2->user_language, data_string);

    return (boolean)ret;
}

tpselt
data_asr_event_to_tps(data_util_state* pds, data_asr_event* pfe)
{
    tpselt te = NULL;

    te = te_new("asr-event");

    if (te == NULL)
    {
        goto errexit;
    }

    if (!te_setattrc(te, "asr-screen", data_string_get(pds, &pfe->asr_screen)))
    {
        goto errexit;
    }

    if (!te_setattrc(te, "session-id", data_string_get(pds, &pfe->session_id)))
    {
        goto errexit;
    }

    if (!te_setattrc(te, "asr-text", data_string_get(pds, &pfe->asr_text)))
    {
        goto errexit;
    }

    if (!te_setattrc(te, "provider-session-id", data_string_get(pds, &pfe->provider_session_id)))
    {
        goto errexit;
    }

    if (!te_setattrc(te, "user-lang", data_string_get(pds, &pfe->user_language)))
    {
        goto errexit;
    }

    return te;

errexit:

    te_dealloc(te);

    return NULL;
}

uint32
data_asr_event_get_tps_size(data_util_state* pds, data_asr_event* pfe)
{
    uint32 size = 0;

    size += data_string_get_tps_size(pds, &pfe->asr_screen);
    size += data_string_get_tps_size(pds, &pfe->session_id);
    size += data_string_get_tps_size(pds, &pfe->asr_text);
    size += data_string_get_tps_size(pds, &pfe->provider_session_id);
    size += data_string_get_tps_size(pds, &pfe->user_language);

    return size;
}

void
data_asr_event_to_buf(data_util_state* pds, data_asr_event* pfe, struct dynbuf* pdb)
{
    data_string_to_buf(pds, &pfe->asr_screen, pdb);
    data_string_to_buf(pds, &pfe->session_id, pdb);
    data_string_to_buf(pds, &pfe->asr_text, pdb);
    data_string_to_buf(pds, &pfe->provider_session_id, pdb);
    data_string_to_buf(pds, &pfe->user_language, pdb);
}

NB_Error
data_asr_event_from_binary(data_util_state* pds, data_asr_event* pfe, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_string_from_binary(pds, &pfe->asr_screen, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pfe->session_id, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pfe->asr_text, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pfe->provider_session_id, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pfe->user_language, pdata, pdatalen);

    return err;
}

/*! @} */
