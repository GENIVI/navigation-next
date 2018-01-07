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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2004 by Networks In Motion, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to Networks In Motion, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/

/*-
 * data_result_style.c: created 2007/05/29 by Mike Gilbert.
 */

#include "data_result_style.h"
#include "data_pair.h"

NB_Error
data_result_style_init(data_util_state* pds, data_result_style* prs)
{
    NB_Error err = NE_OK;

    data_string_init(pds, &prs->key);

    return err;
}

void        
data_result_style_free(data_util_state* pds, data_result_style* prs)
{
    data_string_free(pds, &prs->key);
}

tpselt
data_result_style_to_tps(data_util_state* pds, data_result_style* prs)
{
    tpselt te;

    te = te_new("result-style");

    if (te == NULL)
        goto errexit;

    if (!te_setattrc(te, "key", data_string_get(pds, &prs->key)))
        goto errexit;

    return te;

errexit:

    te_dealloc(te);
    return NULL;
}

NB_Error
data_result_style_from_tps(data_util_state* pds, data_result_style* pdat, tpselt te)
{
    NB_Error err = NE_OK;

    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(pds, err, pdat, data_result_style);

    if (err != NE_OK)
        return err;

    err = err ? err : data_string_from_tps_attr(pds, &pdat->key, te, "key");

errexit:
    if (err != NE_OK)
        DATA_FREE(pds, pdat, data_result_style);
    return err;
}

boolean
data_result_style_equal(data_util_state* pds, data_result_style* lhs, data_result_style* rhs)
{
    int ret = TRUE;

    ret = ret && data_string_equal(pds, &lhs->key, &rhs->key);

    return (boolean) ret;
}

NB_Error
data_result_style_copy(data_util_state* pds, data_result_style* prs_dest, data_result_style* prs_src)
{
    NB_Error err = NE_OK;

    data_result_style_free(pds, prs_dest);

    err = err ? err : data_result_style_init(pds, prs_dest);

    err = err ? err : data_string_copy(pds, &prs_dest->key, &prs_src->key);

    return err;
}

uint32
data_result_style_get_tps_size(data_util_state* pds, data_result_style* prs)
{
    return data_string_get_tps_size(pds, &prs->key);
}

void
data_result_style_to_buf(data_util_state* pds,
            data_result_style* prs,
            struct dynbuf* pdb)
{
    data_string_to_buf(pds, &prs->key, pdb);
}

NB_Error
data_result_style_from_binary(data_util_state* pds,
            data_result_style* prs,
            byte** pdata, size_t* pdatalen)
{
    return data_string_from_binary(pds, &prs->key, pdata, pdatalen);
}

