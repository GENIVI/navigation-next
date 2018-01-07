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

 @file     data_extapp_content.c
 */
/*
 (C) Copyright 2012 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunications Systems, Inc. is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 ---------------------------------------------------------------------------*/

/*! @{ */

#include "data_extapp_content.h"
#include "data_pair.h"


NB_Error
data_extapp_content_init(data_util_state* pds, data_extapp_content* peac)
{
    NB_Error err = NE_OK;

    DATA_MEM_ZERO(peac, data_extapp_content);

    DATA_VEC_ALLOC(err, peac->vec_pairs, data_pair);
    DATA_INIT(pds, err, &peac->name, data_string);

    if (err)
    {
        DATA_VEC_FREE(pds, peac->vec_pairs, data_pair);
        DATA_FREE(pds, &peac->name, data_string);
    }

    return err;
}

void
data_extapp_content_free(data_util_state* pds, data_extapp_content* peac)
{
    DATA_VEC_FREE(pds, peac->vec_pairs, data_pair);
    DATA_FREE(pds, &peac->name, data_string);
}

tpselt
data_extapp_content_to_tps(data_util_state* pds, data_extapp_content* peac)
{
    tpselt te = NULL;

    te = te_new("extapp-content");
    if (te == NULL)
    {
        goto errexit;
    }

    if (peac->vec_pairs)
    {
        int n = 0;
        int l = 0;

        l = CSL_VectorGetLength(peac->vec_pairs);

        for (n = 0; n < l; n++)
        {
            data_pair* pair = CSL_VectorGetPointer(peac->vec_pairs, n);

            DATA_TO_TPS(pds, errexit, te, pair, data_pair);
        }
    }

    if (!te_setattrc(te, "name", data_string_get(pds, &peac->name)))
    {
        goto errexit;
    }

    return te;

errexit:

    if (te)
    {
        te_dealloc(te);
    }

    return NULL;
}

NB_Error
data_extapp_content_from_tps(data_util_state* pds, data_extapp_content* peac, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt ce = NULL;
    int iter = 0;

    if (te == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(pds, err, peac, data_extapp_content);

    while ((ce = te_nextchild(te, &iter)) != NULL)
    {
        if (nsl_strcmp(te_getname(ce), "pair") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, peac->vec_pairs, data_pair);
        }

        if (err != NE_OK)
        {
            goto errexit;
        }
    }

    err = err ? err : data_string_from_tps_attr(pds, &peac->name, te, "name");
    if (err != NE_OK)
    {
        goto errexit;
    }

errexit:
    if (err != NE_OK)
    {
        DATA_FREE(pds, peac, data_extapp_content);
    }

    return err;
}

boolean
data_extapp_content_equal(data_util_state* pds, data_extapp_content* peac1, data_extapp_content* peac2)
{
    int ret = TRUE;

    DATA_VEC_EQUAL(pds, ret, peac1->vec_pairs, peac2->vec_pairs, data_pair);
    DATA_EQUAL(pds, ret, &peac1->name, &peac2->name, data_string);

    return (boolean)ret;
}

NB_Error
data_extapp_content_copy(data_util_state* pds, data_extapp_content* peac_dest, data_extapp_content* peac_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, peac_dest, data_extapp_content);

    DATA_VEC_COPY(pds, err, peac_dest->vec_pairs, peac_src->vec_pairs, data_pair);
    DATA_COPY(pds, err, &peac_dest->name, &peac_src->name, data_string);

    return err;
}

/*! @} */