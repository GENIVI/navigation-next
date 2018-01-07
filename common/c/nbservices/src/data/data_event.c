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
 * data_event.c: created 2007/11/06 by Tom Phan.
 */

#include "data_event.h"

NB_Error
data_event_init(data_util_state* pds, data_event* pdat)
{
    NB_Error err = NE_OK;

    DATA_MEM_ZERO(pdat, data_event);

    DATA_INIT(pds, err, &pdat->content, data_event_content);

    DATA_VEC_ALLOC(err, pdat->vec_performances, data_event_performance);

    return err;
}

void
data_event_free(data_util_state* pds, data_event* pdat)
{
    DATA_FREE(pds, &pdat->content, data_event_content);

    DATA_VEC_FREE(pds, pdat->vec_performances, data_event_performance);
}

NB_Error
data_event_from_tps(data_util_state* pds, data_event* pdat, tpselt te)
{
    NB_Error err = NE_OK;
    boolean has_event_content = FALSE;
    tpselt    ce;
    int iter;

    if (te == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(pds, err, pdat, data_event);

    iter = 0;

    while ((ce = te_nextchild(te, &iter)) != NULL) {

        if (nsl_strcmp(te_getname(ce), "event-performance") == 0) {

            DATA_FROM_TPS_ADD_TO_VEC(pds, err, ce, pdat->vec_performances, data_event_performance);
        }
        else if (nsl_strcmp(te_getname(ce), "event-content") == 0) {

            has_event_content = TRUE;
            DATA_FROM_TPS(pds, err, ce, &pdat->content, data_event_content);
        }

        if (err != NE_OK)
            goto errexit;
    }

    if (err == NE_OK)
        err = has_event_content ? NE_OK : NE_INVAL;
    
errexit:

    if (err != NE_OK) {

        DEBUGLOG(LOG_SS_DATA, LOG_SEV_INFO, ("data_event (error_code: %d)", err));
        data_event_free(pds, pdat);
    }

    return err;
}


boolean
data_event_equal(data_util_state* pds, data_event* lhs, data_event* rhs)
{
    int ret = TRUE;

    DATA_EQUAL(pds, ret, &lhs->content, &rhs->content, data_event_content);

    DATA_VEC_EQUAL(pds, ret, lhs->vec_performances, rhs->vec_performances, data_event_performance);

    return (boolean) ret;
}


NB_Error
data_event_copy(data_util_state* pds, data_event* dst, data_event* src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, dst, data_event);

    DATA_COPY(pds, err, &dst->content, &src->content, data_event_content);

    DATA_VEC_COPY(pds, err, dst->vec_performances, src->vec_performances, data_event_performance);

    return err;
}

uint32   data_event_get_tps_size(data_util_state* pds, data_event* pdat)
{
    uint32 size = 0;
    int i = 0;
    int length = 0;

    size += data_event_content_get_tps_size(pds, &pdat->content);

    length = CSL_VectorGetLength(pdat->vec_performances);
    size += sizeof(length);
    for(i = 0; i < length; i++)
    {
        size += data_event_performance_get_tps_size(pds, CSL_VectorGetPointer(pdat->vec_performances, i));
    }

    return size;
}

void     data_event_to_buf(data_util_state* pds, data_event* pdat, struct dynbuf* pdb)
{
    int len = 0;

    data_event_content_to_buf(pds, &pdat->content, pdb);

    len = CSL_VectorGetLength(pdat->vec_performances);
    dbufcat(pdb, (const byte*)&len, sizeof(len));
    if (len  > 0)
    {
        int i = 0;

        for (i = 0; i < len; i++)
        {
            data_event_performance* pair =
                CSL_VectorGetPointer(pdat->vec_performances, i);

            data_event_performance_to_buf(pds, pair, pdb);
        }
    }
}

NB_Error data_event_from_binary(data_util_state* pds, data_event* pdat, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;
    int len = 0;

    err = err ? err : data_event_content_from_binary(pds, &pdat->content, pdata, pdatalen);

    err = err ? err : data_uint32_from_binary(pds, (uint32*)&len, pdata, pdatalen);
    if (err == NE_OK && len > 0)
    {
        int i = 0;
        
        for (i = 0; i < len ; i++)
        {
            data_event_performance pair = {0};

            if (*pdatalen >= sizeof(pair))
            {
                err = err ? err : data_event_performance_init(pds, &pair);
                err = err ? err : data_event_performance_from_binary(pds, &pair, pdata, pdatalen);

                if(err)
                {
                    break;
                }

                if (!CSL_VectorAppend(pdat->vec_performances, &pair))
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

    return err;
}
