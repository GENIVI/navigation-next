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

    @file     data_suggest_list.c
*/
/*
    See file description in header file.

    (C) Copyright 2014 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

/*! @{ */

#include "data_suggest_list.h"

NB_Error data_suggest_list_init(data_util_state* state, data_suggest_list* suggestList)
{
    NB_Error err = NE_OK;

    DATA_MEM_ZERO(suggestList, data_suggest_list);

    DATA_VEC_ALLOC(err, suggestList->vec_suggest_matches, data_suggest_match);
    DATA_INIT(state, err, &suggestList->name, data_string);

    return err;
}

void data_suggest_list_free(data_util_state* state, data_suggest_list* suggestList)
{
    DATA_VEC_FREE(state, suggestList->vec_suggest_matches, data_suggest_list);
    DATA_FREE(state, &suggestList->name, data_string);
}

NB_Error data_suggest_list_from_tps(data_util_state* state, data_suggest_list* suggestList, tpselt tpsElement)
{
    NB_Error result = NE_OK;
    tpselt ce = NULL;
    int iter = 0;

    if (tpsElement == NULL)
    {
        result = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(state, result, suggestList, data_suggest_list);
    if (result != NE_OK)
    {
        goto errexit;
    }

    result = data_string_from_tps_attr(state, &suggestList->name, tpsElement, "name");
    if (result != NE_OK)
    {
        goto errexit;
    }

    while ((ce = te_nextchild(tpsElement, &iter)) != NULL)
    {
        if (nsl_strcmp(te_getname(ce), "suggest-match") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(state, result, ce, suggestList->vec_suggest_matches, data_suggest_match);
        }

        if (result != NE_OK)
        {
            goto errexit;
        }
    }

errexit:

    if (result != NE_OK)
    {
        DATA_FREE(state, suggestList, data_suggest_list);
    }

    return result;
}

boolean data_suggest_list_equal(data_util_state* state, data_suggest_list* suggestList1, data_suggest_list* suggestList2)
{
    boolean result = TRUE;

    DATA_VEC_EQUAL(state, result, suggestList1->vec_suggest_matches, suggestList2->vec_suggest_matches, data_suggest_match);
    DATA_EQUAL(state, result, &suggestList1->name, &suggestList2->name, data_string);

    return result;
}

NB_Error data_suggest_list_copy(data_util_state* state, data_suggest_list* destinationSuggestList, data_suggest_list* sourceSuggestList)
{
    NB_Error result = NE_OK;

    DATA_REINIT(state, result, destinationSuggestList, data_suggest_list);

    DATA_VEC_COPY(state, result, destinationSuggestList->vec_suggest_matches, sourceSuggestList->vec_suggest_matches, data_suggest_match);
    DATA_COPY(state, result, &destinationSuggestList->name, &sourceSuggestList->name, data_string);

    return result;
}

/*! @} */

