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
 * data_event_performance.c: created 2007/11/06 by Tom Phan.
 */

#include "data_event_performance.h"

static void
data_event_performance_clear(data_util_state* pds, data_event_performance* pdat)
{
    pdat->all_day = FALSE;
    pdat->indeterminate_time = FALSE;
    pdat->bargain_price = FALSE;
    pdat->start_time = 0;
    pdat->end_time = 0;
}

NB_Error
data_event_performance_init(data_util_state* pds, data_event_performance* pdat)
{
    NB_Error err = NE_OK;

    DATA_MEM_ZERO(pdat, data_event_performance);

    DATA_VEC_ALLOC(err, pdat->vec_pairs, data_pair);

    data_event_performance_clear(pds, pdat);

    return err;
}

void
data_event_performance_free(data_util_state* pds, data_event_performance* pdat)
{
    DATA_VEC_FREE(pds, pdat->vec_pairs, data_pair);

    data_event_performance_clear(pds, pdat);
}

NB_Error
data_event_performance_from_tps(data_util_state* pds, data_event_performance* pdat, tpselt te)
{
    NB_Error err = NE_OK;
    tpselt    ce;
    int iter;
    int utc_offset = 0;
    
    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(pds, err, pdat, data_event_performance);

    if (err != NE_OK)
        return err;
    
    iter = 0;

    while ((ce = te_nextchild(te, &iter)) != NULL) {

        if (nsl_strcmp(te_getname(ce), "pair") == 0) {

            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, pdat->vec_pairs, data_pair);
        }
        else if (nsl_strcmp(te_getname(ce), "all-day") == 0) {

            pdat->all_day = TRUE;
        }
        else if (nsl_strcmp(te_getname(ce), "indeterminate-time") == 0) {

            pdat->indeterminate_time = TRUE;
        }
        else if (nsl_strcmp(te_getname(ce), "bargain-price") == 0) {

            pdat->bargain_price = TRUE;
        }

        if (err != NE_OK)
            goto errexit;
    }

    utc_offset = te_getattratoi(te, "utc-offset");
    pdat->start_time = te_getattru(te, "start-time") + utc_offset;
    pdat->end_time = te_getattru(te, "end-time") + utc_offset; 


    DEBUGLOG(LOG_SS_DATA, LOG_SEV_INFO, ("data_event_performance_from_tps (start_time: %d, end_time: %d, utc_offset: %d)", pdat->start_time, pdat->end_time, utc_offset));

    //if (pdat->start_time == 0 || pdat->end_time == 0) {
    //    DEBUGLOG(LOG_SS_DATA, LOG_SEV_INFO, ("data_event_performance_from_tps failed (start_time: %d, end_time: %d)", pdat->start_time, pdat->end_time));
    //    err = NE_INVAL;
    //}

errexit:
    if (err != NE_OK) {

        DEBUGLOG(LOG_SS_DATA, LOG_SEV_INFO, ("data_event_performance_from_tps failed (error_code: %d)", err));
        data_event_performance_free(pds, pdat);
    }
    return err;
}

boolean
data_event_performance_equal(data_util_state* pds, data_event_performance* lhs, data_event_performance* rhs)
{
    int ret = TRUE;

    ret = ret && lhs->all_day == rhs->all_day;
    ret = ret && lhs->indeterminate_time == rhs->indeterminate_time;
    ret = ret && lhs->bargain_price == rhs->bargain_price;
    ret = ret && lhs->start_time == rhs->start_time;
    ret = ret && lhs->end_time == rhs->end_time;
    DATA_VEC_EQUAL(pds, ret, lhs->vec_pairs, rhs->vec_pairs, data_pair);

    return (boolean) ret;
}

NB_Error
data_event_performance_copy(data_util_state* pds, data_event_performance* dst, data_event_performance* src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, dst, data_event_performance);

    DATA_VEC_COPY(pds, err, dst->vec_pairs, src->vec_pairs, data_pair);

    if (err == NE_OK) {
        dst->all_day = src->all_day;
        dst->indeterminate_time = src->indeterminate_time;
        dst->bargain_price = src->bargain_price;
        dst->start_time = src->start_time;
        dst->end_time = src->end_time;
    }

    return err;
}

uint32   data_event_performance_get_tps_size(data_util_state* pds, data_event_performance* pdat)
{
    uint32 size = 0;
    int i = 0;
    int length = 0;

    size += sizeof(pdat->all_day);
    size += sizeof(pdat->indeterminate_time);
    size += sizeof(pdat->bargain_price);

    length = CSL_VectorGetLength(pdat->vec_pairs);
    size += sizeof(length);
    for (i = 0; i < length; i++)
    {
        size += data_pair_get_tps_size(pds, CSL_VectorGetPointer(pdat->vec_pairs, i));
    }

    size += sizeof(pdat->start_time);
    size += sizeof(pdat->end_time);

    return size;
}

void     data_event_performance_to_buf(data_util_state* pds, data_event_performance* pdat, struct dynbuf* pdb)
{
    int len = 0;

    dbufcat(pdb, (const byte*)&pdat->all_day, sizeof(pdat->all_day));
    dbufcat(pdb, (const byte*)&pdat->indeterminate_time, sizeof(pdat->indeterminate_time));
    dbufcat(pdb, (const byte*)&pdat->bargain_price, sizeof(pdat->bargain_price));

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

    dbufcat(pdb, (const byte*)&pdat->start_time, sizeof(pdat->start_time));
    dbufcat(pdb, (const byte*)&pdat->end_time, sizeof(pdat->end_time));
}

NB_Error data_event_performance_from_binary(data_util_state* pds, data_event_performance* pdat, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;
    int len = 0;

    err = err ? err : data_boolean_from_binary(pds, &pdat->all_day, pdata, pdatalen);
    err = err ? err : data_boolean_from_binary(pds, &pdat->indeterminate_time, pdata, pdatalen);
    err = err ? err : data_boolean_from_binary(pds, &pdat->bargain_price, pdata, pdatalen);

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

    err = err ? err : data_uint32_from_binary(pds, &pdat->start_time, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(pds, &pdat->end_time, pdata, pdatalen);

    return err;
}
