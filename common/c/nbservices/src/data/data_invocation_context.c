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

 @file     data_invocation_context.c
 */
/*
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

#include "data_invocation_context.h"

NB_Error data_invocation_context_init(data_util_state* pds, data_invocation_context* pdic)
{
    NB_Error err = NE_OK;

    DATA_INIT(pds, err, &pdic->input_source, data_string);
    DATA_INIT(pds, err, &pdic->invocation_method, data_string);
    DATA_INIT(pds, err, &pdic->screen_id, data_string);
    DATA_INIT(pds, err, &pdic->source_module, data_string);

    return err;
}

void data_invocation_context_free(data_util_state* pds, data_invocation_context* pdic)
{
    DATA_FREE(pds, &pdic->input_source, data_string);
    DATA_FREE(pds, &pdic->invocation_method, data_string);
    DATA_FREE(pds, &pdic->screen_id, data_string);
    DATA_FREE(pds, &pdic->source_module, data_string);
}

tpselt data_invocation_context_to_tps(data_util_state* pds, data_invocation_context* pdic)
{
    tpselt te = NULL;

    te = te_new("invocation-context");
    if (te == NULL)
    {
        goto errexit;
    }

    if (!te_setattrc(te, "input-source",
                     data_string_get(pds, &pdic->input_source)))
    {
        goto errexit;
    }

    if (!te_setattrc(te, "invocation-method",
                     data_string_get(pds, &pdic->invocation_method)))
    {
        goto errexit;
    }

    if (!te_setattrc(te, "screen-id",
                     data_string_get(pds, &pdic->screen_id)))
    {
        goto errexit;
    }

    if (!te_setattrc(te, "source-module",
                     data_string_get(pds, &pdic->source_module)))
    {
        goto errexit;
    }

    return te;

errexit:

    te_dealloc(te);
    return NULL;
}

boolean data_invocation_context_equal(data_util_state* pds, data_invocation_context* pdic1, data_invocation_context* pdic2)
{
    int ret = TRUE;

    DATA_EQUAL(pds, ret, &pdic1->input_source, &pdic2->input_source, data_string);
    DATA_EQUAL(pds, ret, &pdic1->invocation_method, &pdic2->invocation_method, data_string);
    DATA_EQUAL(pds, ret, &pdic1->screen_id, &pdic2->screen_id, data_string);
    DATA_EQUAL(pds, ret, &pdic1->source_module, &pdic2->source_module, data_string);

    return (boolean)ret;
}

NB_Error data_invocation_context_copy(data_util_state* pds, data_invocation_context* pdic_dest, data_invocation_context* pdic_src)
{
    NB_Error err = NE_OK;

    DATA_COPY(pds, err, &pdic_dest->input_source, &pdic_src->input_source, data_string);
    DATA_COPY(pds, err, &pdic_dest->invocation_method, &pdic_src->invocation_method, data_string);
    DATA_COPY(pds, err, &pdic_dest->screen_id, &pdic_src->screen_id, data_string);
    DATA_COPY(pds, err, &pdic_dest->source_module, &pdic_src->source_module, data_string);

    return err;
}

uint32 data_invocation_context_get_tps_size(data_util_state* pds, data_invocation_context* pdic)
{
    uint32 size = 0;

    size += data_string_get_tps_size(pds, &pdic->input_source);
    size += data_string_get_tps_size(pds, &pdic->invocation_method);
    size += data_string_get_tps_size(pds, &pdic->screen_id);
    size += data_string_get_tps_size(pds, &pdic->source_module);

    return size;
}

void data_invocation_context_to_buf(data_util_state* pds, data_invocation_context* pdic, struct dynbuf* pdb)
{
    data_string_to_buf(pds, &pdic->input_source, pdb);
    data_string_to_buf(pds, &pdic->invocation_method, pdb);
    data_string_to_buf(pds, &pdic->screen_id, pdb);
    data_string_to_buf(pds, &pdic->source_module, pdb);
}

NB_Error data_invocation_context_from_binary(data_util_state* pds, data_invocation_context* pdic, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    do {
        err = data_string_from_binary(pds, &pdic->input_source, pdata, pdatalen);
        if (err != NE_OK)
        {
            break;
        }

        err = data_string_from_binary(pds, &pdic->invocation_method, pdata, pdatalen);
        if (err != NE_OK)
        {
            break;
        }

        err = data_string_from_binary(pds, &pdic->screen_id, pdata, pdatalen);
        if (err != NE_OK)
        {
            break;
        }

        err = data_string_from_binary(pds, &pdic->source_module, pdata, pdatalen);
        if (err != NE_OK)
        {
            break;
        }

    } while (0);

    return err;
}

/*! @} */
