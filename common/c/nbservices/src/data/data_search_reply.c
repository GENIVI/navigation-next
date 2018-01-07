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

    @file     data_search_reply.c
*/
/*
    See file description in header file.

    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

/*! @{ */

#include "data_search_reply.h"
#include "data_proxmatch.h"
#include "data_suggest_match.h"
#include "data_suggest_list.h"
#include "data_file.h"

NB_Error
data_search_reply_init(data_util_state* state, data_search_reply* searchReply)
{
    NB_Error result = NE_OK;

    DATA_MEM_ZERO(searchReply, data_search_reply);

    DATA_INIT(state, result, &(searchReply->iter_result), data_iter_result);
    DATA_INIT(state, result, &(searchReply->fuel_pricesummary), data_fuel_pricesummary);

    DATA_VEC_ALLOC(result, searchReply->vector_proxmatch, data_proxmatch);
    DATA_VEC_ALLOC(result, searchReply->vector_suggest_match, data_suggest_match);
    DATA_VEC_ALLOC(result, searchReply->vector_suggest_list, data_suggest_list);
    DATA_VEC_ALLOC(result, searchReply->vector_file, data_file);

    return result;
}

void
data_search_reply_free(data_util_state* state, data_search_reply* searchReply)
{
    DATA_FREE(state, &(searchReply->iter_result), data_iter_result);
    DATA_FREE(state, &(searchReply->fuel_pricesummary), data_fuel_pricesummary);

    DATA_VEC_FREE(state, searchReply->vector_proxmatch, data_proxmatch);
    DATA_VEC_FREE(state, searchReply->vector_suggest_match, data_suggest_match);
    DATA_VEC_FREE(state, searchReply->vector_suggest_list, data_suggest_list);
    DATA_VEC_FREE(state, searchReply->vector_file, data_file);
}

NB_Error
data_search_reply_from_tps(data_util_state* state, data_search_reply* searchReply, tpselt tpsElement)
{
    NB_Error result = NE_OK;
    tpselt ce = NULL;
    int iter = 0;

    if (tpsElement == NULL)
    {
        result = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(state, result, searchReply, data_search_reply);

    if (result != NE_OK)
    {
        goto errexit;
    }

    while ((ce = te_nextchild(tpsElement, &iter)) != NULL)
    {
        if (nsl_strcmp(te_getname(ce), "proxmatch") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(state, result, ce, searchReply->vector_proxmatch, data_proxmatch);
        }
        else if (nsl_strcmp(te_getname(ce), "suggest-match") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(state, result, ce, searchReply->vector_suggest_match, data_suggest_match);
        }
        else if (nsl_strcmp(te_getname(ce), "suggest-list") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(state, result, ce, searchReply->vector_suggest_list, data_suggest_list);
        }
        else if (nsl_strcmp(te_getname(ce), "file") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(state, result, ce, searchReply->vector_file, data_file);
        }
        else if (nsl_strcmp(te_getname(ce), "iter-result") == 0)
        {
            DATA_FROM_TPS(state, result, ce, &(searchReply->iter_result), data_iter_result);
        }
        else if (nsl_strcmp(te_getname(ce), "fuel-price-summary") == 0)
        {
            DATA_FROM_TPS(state, result, ce, &(searchReply->fuel_pricesummary), data_fuel_pricesummary);
        }

        if (result != NE_OK)
        {
            goto errexit;
        }
    }

errexit:

    if (result != NE_OK)
    {
        DATA_FREE(state, searchReply, data_search_reply);
    }

    return result;
}

boolean
data_search_reply_equal(data_util_state* state, data_search_reply* searchReply1, data_search_reply* searchReply2)
{
    int result = TRUE;

    DATA_VEC_EQUAL(state, result, searchReply1->vector_proxmatch, searchReply2->vector_proxmatch, data_proxmatch);
    DATA_VEC_EQUAL(state, result, searchReply1->vector_suggest_match, searchReply2->vector_suggest_match, data_suggest_match);
    DATA_VEC_EQUAL(state, result, searchReply1->vector_suggest_list, searchReply2->vector_suggest_list, data_suggest_list);
    DATA_VEC_EQUAL(state, result, searchReply1->vector_file, searchReply2->vector_file, data_file);

    DATA_EQUAL(state, result, &(searchReply1->iter_result), &(searchReply2->iter_result), data_iter_result);
    DATA_EQUAL(state, result, &(searchReply1->fuel_pricesummary), &(searchReply2->fuel_pricesummary), data_fuel_pricesummary);

    return (boolean) result;
}

NB_Error
data_search_reply_copy(data_util_state* state, data_search_reply* destinationSearchReply, data_search_reply* sourceSearchReply)
{
    NB_Error result = NE_OK;

    DATA_REINIT(state, result, destinationSearchReply, data_search_reply);

    DATA_VEC_COPY(state, result, destinationSearchReply->vector_proxmatch, sourceSearchReply->vector_proxmatch, data_proxmatch);
    DATA_VEC_COPY(state, result, destinationSearchReply->vector_suggest_match, sourceSearchReply->vector_suggest_match, data_suggest_match);
    DATA_VEC_COPY(state, result, destinationSearchReply->vector_suggest_list, sourceSearchReply->vector_suggest_list, data_suggest_list);
    DATA_VEC_COPY(state, result, destinationSearchReply->vector_file, sourceSearchReply->vector_file, data_file);

    DATA_COPY(state, result, &(destinationSearchReply->iter_result), &(sourceSearchReply->iter_result), data_iter_result);
    DATA_COPY(state, result, &(destinationSearchReply->fuel_pricesummary), &(sourceSearchReply->fuel_pricesummary), data_fuel_pricesummary);

    return result;
}

/*! @} */
