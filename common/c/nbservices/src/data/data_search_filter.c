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
 * data_search_filter.c: created 2007/05/29 by Mike Gilbert.
 */

#include "data_search_filter.h"
#include "data_result_style.h"
#include "data_pair.h"

NB_Error
data_search_filter_init(data_util_state* pds, data_search_filter* psf)
{
    NB_Error err = NE_OK;

    DATA_MEM_ZERO(psf, data_search_filter);

    err = data_result_style_init(pds, &psf->result_style);

    return err;
}

void
data_search_filter_free(data_util_state* pds, data_search_filter* psf)
{
    int i, len;

    if (psf->vec_pairs) {

        len = CSL_VectorGetLength(psf->vec_pairs);

        for (i = 0; i < len; i++)
            data_pair_free(pds, (data_pair*)CSL_VectorGetPointer(psf->vec_pairs, i));

        CSL_VectorDealloc(psf->vec_pairs);
    }

    data_result_style_free(pds, &psf->result_style);

    data_search_filter_init(pds, psf);
}

NB_Error
data_search_filter_from_tps(data_util_state* pds, data_search_filter* pdat, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt  ce;
    int iter;

    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    DATA_VEC_ALLOC(err, pdat->vec_pairs, data_pair);

    if (err != NE_OK)
        return err;

    iter = 0;

    while (err == NE_OK && (ce = te_nextchild(te, &iter)) != NULL) {

        if (nsl_strcmp(te_getname(ce), "pair") == 0) {

            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, pdat->vec_pairs, data_pair);
        }
        else if (nsl_strcmp(te_getname(ce), "result-style") == 0) {

            DATA_FROM_TPS(pds, err, ce, &pdat->result_style, data_result_style);
        }

        if (err != NE_OK)
            goto errexit;
    }

errexit:
    if (err != NE_OK)
        DATA_FREE(pds, pdat, data_search_filter);

    return err;
}

tpselt
data_search_filter_to_tps(data_util_state* pds, data_search_filter* psf)
{
    tpselt te;
    tpselt ce = NULL;
    int i, len;

    te = te_new("search-filter");

    if (te == NULL)
        goto errexit;

    if (psf->vec_pairs) {

        len = CSL_VectorGetLength(psf->vec_pairs);

        for (i = 0; i < len; i++) {

            if ((ce = data_pair_to_tps(pds, (data_pair*)CSL_VectorGetPointer(psf->vec_pairs, i))) != NULL && te_attach(te, ce))
                ce = NULL;
            else
                goto errexit;
        }
    }

    if ((ce = data_result_style_to_tps(pds, &psf->result_style)) != NULL && te_attach(te, ce))
        ce = NULL;
    else
        goto errexit;

    return te;

errexit:

    te_dealloc(te);
    return NULL;
}

boolean
data_search_filter_equal(data_util_state* pds, data_search_filter* lhs, data_search_filter* rhs) 
{
    int ret = TRUE;

    DATA_VEC_EQUAL(pds, ret, lhs->vec_pairs, rhs->vec_pairs, data_pair);

    ret = ret && data_result_style_equal(pds, &lhs->result_style, &rhs->result_style);

    return (boolean) ret;
}

NB_Error
data_search_filter_copy(data_util_state* pds, data_search_filter* psf_dest, data_search_filter* psf_src)
{
    NB_Error err = NE_OK;
    int i, len;
    data_pair dp;

    err = err ? err : data_pair_init(pds, &dp);
    data_search_filter_free(pds, psf_dest);

    err = err ? err : data_search_filter_init(pds, psf_dest);

    if (psf_src->vec_pairs)
    {

        len = CSL_VectorGetLength(psf_src->vec_pairs);

        if (len > 0 && psf_dest->vec_pairs == NULL)
        {
            psf_dest->vec_pairs = CSL_VectorAlloc(sizeof(data_pair));

            if (psf_dest->vec_pairs == NULL)
                err = NE_NOMEM;
        }

        for (i = 0; i < len; i++) {

            err = err ? err : data_pair_copy(pds, &dp, (data_pair*)CSL_VectorGetPointer(psf_src->vec_pairs, i));
            err = err ? err : CSL_VectorAppend(psf_dest->vec_pairs, &dp) ? NE_OK : NE_NOMEM;

            if (err)
                data_pair_free(pds, &dp);
            else
                err = data_pair_init(pds, &dp);
        }
    }
    err = err ? err : data_result_style_copy(pds, &psf_dest->result_style, &psf_src->result_style);

    return err;
}

uint32
data_search_filter_get_tps_size(data_util_state* pds, data_search_filter* psf)
{
    int i, len;
    uint32 size = 0;

    if (psf->vec_pairs) {
        len = CSL_VectorGetLength(psf->vec_pairs);
        for (i = 0; i < len; i++) {
            data_pair* pd = (data_pair*)CSL_VectorGetPointer(psf->vec_pairs, i);
            size += data_pair_get_tps_size(pds, pd);
        }
    }

    size += data_result_style_get_tps_size(pds, &psf->result_style);

    return size;
}

void
data_search_filter_to_buf(data_util_state* pds,
            data_search_filter* psf,
            struct dynbuf* pdb)
{
    int len = 0;

    if (psf->vec_pairs)
    {
        int i;

        len = CSL_VectorGetLength(psf->vec_pairs);
        dbufcat(pdb, (const byte*)&len, sizeof(len));

        for (i = 0; i < len; i++) {
            data_pair* pd = (data_pair*)CSL_VectorGetPointer(psf->vec_pairs, i);
            data_pair_to_buf(pds, pd, pdb);
        }
    }
    else
    {
        dbufcat(pdb, (const byte*)&len, sizeof(len));
    }

    data_result_style_to_buf(pds, &psf->result_style, pdb);
}

NB_Error
data_search_filter_from_binary(data_util_state* pds,
            data_search_filter* psf,
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
        int i=0;
        if(psf->vec_pairs == NULL)
        {
            psf->vec_pairs = CSL_VectorAlloc(sizeof(data_pair));
        }

        if(psf->vec_pairs == NULL)
        {
            return NE_NOMEM;
        }

        for (i=0; i<len; i++)
        {
            data_pair pair;

            err = data_pair_init(pds, &pair);
            if (err)
            {
                break;
            }

            err = data_pair_from_binary(pds, &pair, pdata, pdatalen);
            if (err)
            {
                data_pair_free(pds, &pair);
                break;
            }

            if (!CSL_VectorAppend(psf->vec_pairs, &pair))
            {
                data_pair_free(pds, &pair);
                err = NE_NOMEM;
                break;
            }
        }
    }

    err = err ? err : data_result_style_from_binary(pds, &psf->result_style,
                                                    pdata, pdatalen);
    return err;
}
