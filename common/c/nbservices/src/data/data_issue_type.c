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

   @file     data_issue_type.c
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

#include "data_issue_type.h"

NB_Error
data_issue_type_init(data_util_state* pds, data_issue_type* pt)
{
    NB_Error err = NE_OK;

    DATA_MEM_ZERO(pt, data_issue_type);

    DATA_INIT(pds, err, &pt->type, data_string);

    return err;
}

void
data_issue_type_free(data_util_state* pds, data_issue_type* pt)
{
    DATA_FREE(pds, &pt->type, data_string);
}

NB_Error
data_issue_type_from_tps(data_util_state* pds, data_issue_type* pt, tpselt te)
{
    NB_Error err = NE_OK;

    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(pds, err, pt, data_issue_type);

    err = err ? err : data_string_from_tps_attr(pds, &pt->type, te, "type");

errexit:
    if (err != NE_OK)
        DATA_FREE(pds, pt, data_issue_type);
    return err;
}

tpselt
data_issue_type_to_tps(data_util_state* pds, data_issue_type* pt)
{
    tpselt te = NULL;

    te = te_new("issue-type");

    if (te == NULL)
    {
        goto errexit;
    }

    if (!te_setattrc(te, "type", data_string_get(pds, &pt->type)))
    {
        goto errexit;
    }

    return te;

errexit:

    te_dealloc(te);
    return NULL;
}


boolean
data_issue_type_equal(data_util_state* pds, data_issue_type* pt1, data_issue_type* pt2)
{
    int ret = TRUE;

    DATA_EQUAL(pds, ret, &pt1->type, &pt2->type, data_string);

    return (boolean) ret;
}

NB_Error
data_issue_type_copy(data_util_state* pds, data_issue_type* pt_dest, data_issue_type* pt_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, pt_dest, data_issue_type);

    DATA_COPY(pds, err, &pt_dest->type, &pt_src->type, data_string);

    return err;
}

uint32
data_issue_type_get_tps_size(data_util_state* pds, data_issue_type* pt)
{
    uint32 size = 0;

    size += data_string_get_tps_size(pds, &pt->type);

    return size;
}

void
data_issue_type_to_buf(data_util_state* pds, data_issue_type* pt, struct dynbuf* pdb)
{
    data_string_to_buf(pds, &pt->type, pdb);
}

NB_Error
data_issue_type_from_binary(data_util_state* pds, data_issue_type* pt, byte** pdata, size_t* pdatalen)
{
    return data_string_from_binary(pds, &pt->type, pdata, pdatalen);
}

/*! @} */
