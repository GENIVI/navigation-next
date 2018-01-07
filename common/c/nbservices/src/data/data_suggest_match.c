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

    @file     data_suggest_match.c
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

#include "data_pair.h"
#include "data_suggest_match.h"

NB_Error
data_suggest_match_init(data_util_state* state, data_suggest_match* suggestMatch)
{
    NB_Error result = NE_OK;

    DATA_MEM_ZERO(suggestMatch, data_suggest_match);

    DATA_VEC_ALLOC(result, suggestMatch->vec_pairs, data_pair);
    DATA_INIT(state, result, &(suggestMatch->search_filter), data_search_filter);
    DATA_INIT(state, result, &(suggestMatch->line1), data_string);
    DATA_INIT(state, result, &(suggestMatch->line2), data_string);
    DATA_INIT(state, result, &(suggestMatch->line3), data_string);
    DATA_INIT(state, result, &(suggestMatch->match_type), data_string);

    return result;
}

void
data_suggest_match_free(data_util_state* state, data_suggest_match* suggestMatch)
{
    DATA_VEC_FREE(state, suggestMatch->vec_pairs, data_pair);
    DATA_FREE(state, &(suggestMatch->search_filter), data_search_filter);
    DATA_FREE(state, &(suggestMatch->line1), data_string);
    DATA_FREE(state, &(suggestMatch->line2), data_string);
    DATA_FREE(state, &(suggestMatch->line3), data_string);
    DATA_FREE(state, &(suggestMatch->match_type), data_string);
}

NB_Error
data_suggest_match_from_tps(data_util_state* state, data_suggest_match* suggestMatch, tpselt tpsElement)
{
    NB_Error result = NE_OK;
    tpselt ce = NULL;
    int iter = 0;

    if (tpsElement == NULL)
    {
        result = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(state, result, suggestMatch, data_suggest_match);

    // Ignore the returned error because the attributes may not return
    te_getattrf(tpsElement,  "distance", &(suggestMatch->distance));
    data_string_from_tps_attr(state, &(suggestMatch->line1), tpsElement, "line1");
    data_string_from_tps_attr(state, &(suggestMatch->line2), tpsElement, "line2");
    data_string_from_tps_attr(state, &(suggestMatch->line3), tpsElement, "line3");
    data_string_from_tps_attr(state, &(suggestMatch->match_type), tpsElement, "match-type");

    if (result != NE_OK)
    {
        goto errexit;
    }

    while ((ce = te_nextchild(tpsElement, &iter)) != NULL)
    {
        if (nsl_strcmp(te_getname(ce), "pair") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(state, result, ce, suggestMatch->vec_pairs, data_pair);
        }
        else if (nsl_strcmp(te_getname(ce), "search-filter") == 0)
        {
            DATA_FROM_TPS(state, result, ce, &(suggestMatch->search_filter), data_search_filter);
        }

        if (result != NE_OK)
        {
            goto errexit;
        }
    }

errexit:

    if (result != NE_OK)
    {
        DATA_FREE(state, suggestMatch, data_suggest_match);
    }

    return result;
}

boolean
data_suggest_match_equal(data_util_state* state, data_suggest_match* suggestMatch1, data_suggest_match* suggestMatch2)
{
    int result = TRUE;

    DATA_VEC_EQUAL(state, result, suggestMatch1->vec_pairs, suggestMatch2->vec_pairs, data_pair);
    DATA_EQUAL(state, result, &(suggestMatch1->search_filter), &(suggestMatch2->search_filter), data_search_filter);
    DATA_EQUAL(state, result, &(suggestMatch1->line1), &(suggestMatch2->line1), data_string);
    DATA_EQUAL(state, result, &(suggestMatch1->line2), &(suggestMatch2->line2), data_string);
    DATA_EQUAL(state, result, &(suggestMatch1->line3), &(suggestMatch2->line3), data_string);
    DATA_EQUAL(state, result, &(suggestMatch1->match_type), &(suggestMatch2->match_type), data_string);

    return (boolean) result;
}

NB_Error
data_suggest_match_copy(data_util_state* state, data_suggest_match* destinationSuggestMatch, data_suggest_match* sourceSuggestMatch)
{
    NB_Error result = NE_OK;

    DATA_REINIT(state, result, destinationSuggestMatch, data_suggest_match);

    DATA_VEC_COPY(state, result, destinationSuggestMatch->vec_pairs, sourceSuggestMatch->vec_pairs, data_pair);
    DATA_COPY(state, result, &(destinationSuggestMatch->search_filter), &(sourceSuggestMatch->search_filter), data_search_filter);
    destinationSuggestMatch->distance = sourceSuggestMatch->distance;
    DATA_COPY(state, result, &(destinationSuggestMatch->line1), &(sourceSuggestMatch->line1), data_string);
    DATA_COPY(state, result, &(destinationSuggestMatch->line2), &(sourceSuggestMatch->line2), data_string);
    DATA_COPY(state, result, &(destinationSuggestMatch->line3), &(sourceSuggestMatch->line3), data_string);
    DATA_COPY(state, result, &(destinationSuggestMatch->match_type), &(sourceSuggestMatch->match_type), data_string);

    return result;
}

/*! @} */
