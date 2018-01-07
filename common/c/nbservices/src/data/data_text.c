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

   @file     data_text.c
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

#include "data_text.h"

NB_Error
data_text_init(data_util_state* pds, data_text* pt)
{
    NB_Error err = NE_OK;

    DATA_MEM_ZERO(pt, data_text);

    DATA_INIT(pds, err, &pt->data, data_string);

    return err;
}

void
data_text_free(data_util_state* pds, data_text* pt)
{
    DATA_FREE(pds, &pt->data, data_string);
}

NB_Error
data_text_from_tps(data_util_state* pds, data_text* pt, tpselt te)
{
    NB_Error err = NE_OK;

    if (te == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(pds, err, pt, data_text);

    err = err ? err : data_string_from_tps_attr(pds, &pt->data, te, "data");

errexit:
    if (err != NE_OK)
    {
        DATA_FREE(pds, pt, data_text);
    }
    return err;
}

tpselt
data_text_to_tps(data_util_state* pds, data_text* pt)
{
    tpselt te = NULL;

    te = te_new("text");

    if (te == NULL)
    {
        goto errexit;
    }

    if (!te_setattrc(te, "data", data_string_get(pds, &pt->data)))
    {
        goto errexit;
    }

    return te;

errexit:

    te_dealloc(te);
    return NULL;
}


boolean
data_text_equal(data_util_state* pds, data_text* pt1, data_text* pt2)
{
    int ret = TRUE;

    DATA_EQUAL(pds, ret, &pt1->data, &pt2->data, data_string);

    return (boolean) ret;
}

NB_Error
data_text_copy(data_util_state* pds, data_text* pt_dest, data_text* pt_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, pt_dest, data_text);

    DATA_COPY(pds, err, &pt_dest->data, &pt_src->data, data_string);

    return err;
}

uint32
data_text_get_tps_size(data_util_state* pds, data_text* pt)
{
    uint32 size = 0;

    size += data_string_get_tps_size(pds, &pt->data);

    return size;
}


void
data_text_to_buf(data_util_state* pds, data_text* pt, struct dynbuf* pdb)
{
    data_string_to_buf(pds, &pt->data, pdb);
}

NB_Error
data_text_from_binary(data_util_state* pds, data_text* pt, byte** pdata, size_t* pdatalen)
{
    return data_string_from_binary(pds, &pt->data, pdata, pdatalen);
}

/*! @} */
