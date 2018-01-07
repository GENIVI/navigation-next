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
 * data_event_content.c: created 2007/11/06 by Tom Phan.
 */

#include "data_event_content.h"

NB_Error
data_event_content_init(data_util_state* pds, data_event_content* pdat)
{
    NB_Error err = NE_OK;

    DATA_MEM_ZERO(pdat, data_event_content);

    DATA_INIT(pds, err, &pdat->url, data_url);

    DATA_INIT(pds, err, &pdat->rating, data_rating);

    DATA_VEC_ALLOC(err, pdat->vec_pairs, data_pair);


    DATA_INIT(pds, err, &pdat->name, data_string);

    DATA_INIT(pds, err, &pdat->formated_text, data_formatted_text);

    return err;
}

void
data_event_content_free(data_util_state* pds, data_event_content* pdat)
{
    data_string_free(pds, &pdat->name);

    DATA_VEC_FREE(pds, pdat->vec_pairs, data_pair);

    data_rating_free(pds, &pdat->rating);


    data_url_free(pds, &pdat->url);

    DATA_FREE(pds, &pdat->formated_text, data_formatted_text);
}

NB_Error
data_event_content_from_tps(data_util_state* pds, data_event_content* pdat, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt    ce;
    int iter;

    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(pds, err, pdat, data_event_content);

    err = err ? err : data_string_from_tps_attr(pds, &pdat->name, te, "name");
    
    if (err != NE_OK)
        goto errexit;

    iter = 0;

    while ((ce = te_nextchild(te, &iter)) != NULL) {

        if (nsl_strcmp(te_getname(ce), "pair") == 0) {

            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, pdat->vec_pairs, data_pair);
        }
        else if (nsl_strcmp(te_getname(ce), "url") == 0) {

            DATA_FROM_TPS(pds, err, ce, &pdat->url, data_url);
        }
        else if (nsl_strcmp(te_getname(ce), "rating") == 0) {

            DATA_FROM_TPS(pds, err, ce, &pdat->rating, data_rating);
        }
        else if (nsl_strcmp(te_getname(ce), "formatted-text") == 0) {

            DATA_FROM_TPS(pds, err, ce, &pdat->formated_text, data_formatted_text);
        }

        if (err != NE_OK)
            goto errexit;
    }

errexit:

    if (err != NE_OK) {

        DEBUGLOG(LOG_SS_DATA, LOG_SEV_INFO, ("data_event_content (error_code: %d)", err));
        DATA_FREE(pds, pdat, data_event_content);
    }

    return err;
}

boolean
data_event_content_equal(data_util_state* pds, data_event_content* lhs, data_event_content* rhs)
{
    int ret = TRUE;

    DATA_EQUAL(pds, ret, &lhs->url, &rhs->url, data_url);
    
    DATA_EQUAL(pds, ret, &lhs->rating, &rhs->rating, data_rating);

    DATA_VEC_EQUAL(pds, ret, lhs->vec_pairs, rhs->vec_pairs, data_pair);

    DATA_EQUAL(pds, ret, &lhs->name, &rhs->name, data_string);

    return (boolean) ret;
}

NB_Error
data_event_content_copy(data_util_state* pds, data_event_content* dst, data_event_content* src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, dst, data_event_content);

    DATA_COPY(pds, err, &dst->url, &src->url, data_url);

    DATA_COPY(pds, err, &dst->rating, &src->rating, data_rating);

    DATA_VEC_COPY(pds, err, dst->vec_pairs, src->vec_pairs, data_pair);

    DATA_COPY(pds, err, &dst->name, &src->name, data_string);

    return err;
}

uint32   data_event_content_get_tps_size(data_util_state* pds, data_event_content* pdat)
{
    uint32 size = 0;
    int i = 0;
    int length = 0;

    size += data_url_get_tps_size(pds, &pdat->url);
    size += data_rating_get_tps_size(pds, &pdat->rating);

    length = CSL_VectorGetLength(pdat->vec_pairs);
    size += sizeof(length);
    for (i = 0; i < length; i++)
    {
        size += data_pair_get_tps_size(pds, CSL_VectorGetPointer(pdat->vec_pairs, i));
    }

    size += data_formatted_text_get_tps_size(pds, &pdat->formated_text);
    size += data_string_get_tps_size(pds, &pdat->name);

    return size;
}

void     data_event_content_to_buf(data_util_state* pds, data_event_content* pdat, struct dynbuf* pdb)
{
    int len = 0;

    data_url_to_buf(pds, &pdat->url, pdb);
    data_rating_to_buf(pds, &pdat->rating, pdb);

    len = CSL_VectorGetLength(pdat->vec_pairs);
    dbufcat(pdb, (const byte*)&len, sizeof(len));
    if (len  > 0)
    {
        int i = 0;

        for (i = 0; i < len; i++)
        {
            data_pair* pair =
                CSL_VectorGetPointer(pdat->vec_pairs, i);

            data_pair_to_buf(pds, pair, pdb);
        }
    }

    data_formatted_text_to_buf(pds, &pdat->formated_text, pdb);
    data_string_to_buf(pds, &pdat->name, pdb);
}

NB_Error data_event_content_from_binary(data_util_state* pds, data_event_content* pdat, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;
    int len = 0;

    err = err ? err : data_url_from_binary(pds, &pdat->url, pdata, pdatalen);
    err = err ? err : data_rating_from_binary(pds, &pdat->rating, pdata, pdatalen);

    err = err ? err : data_uint32_from_binary(pds, (uint32*)&len, pdata, pdatalen);
    if (err == NE_OK && len > 0)
    {
        int i;

        for (i = 0; i < len ; i++)
        {
            data_pair pair;

            if (*pdatalen >= sizeof(pair))
            {
                err = err ? err : data_pair_init(pds, &pair);
                err = err ? err : data_pair_from_binary(pds, &pair, pdata, pdatalen);

                if(err)
                {
                    break;
                }

                if (!CSL_VectorAppend(pdat->vec_pairs, &pair))
                {
                    err = NE_NOMEM;
                    break;
                }
            }
            else
            {
                err = NE_BADDATA;
                break;
            }
        }
    }

    err = err ? err : data_formatted_text_from_binary(pds, &pdat->formated_text, pdata, pdatalen);
    err = err ? err : data_string_from_binary(pds, &pdat->name, pdata, pdatalen);

    return err;
}

