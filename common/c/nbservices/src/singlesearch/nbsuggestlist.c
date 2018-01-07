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

    @file     nbsuggestlist.c
*/
/*
    (C) Copyright 2014 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "nbsuggestlist.h"
#include "nbcontextprotected.h"
#include "nbsuggestmatchprivate.h"
#include "nbsuggestlistprivate.h"
#include "nbpersistentdataprotected.h"
#include "data_suggest_list.h"
#include "data_pair.h"

/*! @{ */

/*! Definition of 'NB_SuggestMatch' */
struct NB_SuggestList
{
    NB_Context* context;                /*!< Pointer to current context */
    data_suggest_list dataSuggestList;  /*!< 'data_suggest_list' object */
};

// Declaration of local functions ...............................................................

static NB_Error CreateSuggestList(NB_Context* context, NB_SuggestList** suggestList);

// Public functions .............................................................................

/* See header file for description */
/*
NB_DEF NB_Error
NB_SuggestListCreateBySerializedData(NB_Context* context,
                                     const uint8* data,
                                     nb_size dataSize,
                                     NB_SuggestList** suggestList)
{
    NB_Error result = NE_OK;
    NB_SuggestList* pThis = NULL;
    nb_boolean ok = FALSE;

    if (!context || !data || (dataSize <= 0) || !suggestList)
    {
        return NE_INVAL;
    }

    result = CreateSuggestList(context, &pThis);
    if (result != NE_OK)
    {
        return result;
    }

    result = DeserializeToSuggestList(pThis, data, dataSize);

    if (result != NE_OK)
    {
        // Destroy the above created 'NB_SuggestMatch' object
        NB_SuggestListDestroy(pThis);
        pThis = NULL;

        return result;
    }

    *suggestList = pThis;

    return result;
}
*/

/* See header file for description */
NB_DEF NB_Error
NB_SuggestListCreateByCopy(NB_SuggestList* sourceSuggestList,
                           NB_SuggestList** suggestList)
{
    if (!sourceSuggestList || !suggestList)
    {
        return NE_INVAL;
    }

    return NB_SuggestListCreateByDataSuggestList(sourceSuggestList->context,
                                                 &sourceSuggestList->dataSuggestList,
                                                 suggestList);
}

/* See header file for description */
NB_DEF void
NB_SuggestListDestroy(NB_SuggestList* suggestList)
{
    data_util_state* state = NULL;

    if (!suggestList)
    {
        return;
    }

    state = NB_ContextGetDataState(suggestList->context);

    // Free the 'data_suggest_list' object.
    data_suggest_list_free(state, &suggestList->dataSuggestList);

    nsl_free(suggestList);
}

/* See header file for description */
NB_DEF void
NB_SuggestListDataDestroy(NB_SuggestListData* suggestListData)
{
    if (!suggestListData)
    {
        return;
    }

    if (suggestListData->matchCount > 0 && suggestListData->suggestMatchData)
    {
        nsl_free(suggestListData->suggestMatchData);

        suggestListData->suggestMatchData = NULL;
        suggestListData->matchCount = 0;
    }
}

/* See header file for description */
NB_DEF NB_Error
NB_SuggestListGetDataToDisplay(NB_SuggestList* suggestList,
                               NB_SuggestListData* dataToDisplay)
{
    NB_Context* context = NULL;
    data_suggest_list* dataSuggestList = NULL;
    NB_SuggestMatch* suggestMatch = NULL;
    NB_SuggestMatchData* suggestMatchData = NULL;
    int countMatches = 0, i = 0;
    NB_Error err = NE_OK;

    if (!suggestList || !dataToDisplay || !suggestList->context)
    {
        return NE_INVAL;
    }

    context = suggestList->context;
    dataSuggestList = &suggestList->dataSuggestList;

    countMatches = CSL_VectorGetLength(dataSuggestList->vec_suggest_matches);
    if (countMatches == 0)
    {
        return NE_INVAL;
    }

    suggestMatchData = (NB_SuggestMatchData*) nsl_malloc(sizeof(NB_SuggestMatchData) * countMatches);
    if (suggestMatchData == NULL)
    {
        return NE_NOMEM;
    }

    nsl_memset(suggestMatchData, 0, sizeof(NB_SuggestMatchData) * countMatches);

    dataToDisplay->matchCount = countMatches;

    for (i = 0; i < countMatches; ++i)
    {
        data_suggest_match* dataSuggestMatch = (data_suggest_match*) CSL_VectorGetPointer(dataSuggestList->vec_suggest_matches, i);
        if (!dataSuggestMatch)
        {
            continue;
        }

        err = NB_SuggestMatchCreateByDataSuggestMatch(context, dataSuggestMatch, &suggestMatch);
        if (err != NE_OK)
        {
            NB_SuggestMatchDestroy(suggestMatch);
            nsl_free(suggestMatchData);
            suggestMatchData = NULL;
            break;
        }

        err = NB_SuggestMatchGetDataToDisplay(suggestMatch, &suggestMatchData[i]);
        if (err != NE_OK)
        {
            nsl_free(suggestMatchData);
            suggestMatchData = NULL;
            break;
        }
    }

    dataToDisplay->suggestMatchData = suggestMatchData;

    return err;
}

// Private functions ............................................................................

/* See 'nbsuggestlistprivate.h' for description */
NB_Error
NB_SuggestListCreateByDataSuggestList(NB_Context* context,
                                      data_suggest_list* dataSuggestList,
                                      NB_SuggestList** suggestList)
{
    NB_Error result = NE_OK;
    data_util_state* state = NULL;
    NB_SuggestList* pThis = NULL;

    if (!context || !dataSuggestList || !suggestList)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(context);

    result = CreateSuggestList(context, &pThis);
    if (result != NE_OK)
    {
        return result;
    }

    result = data_suggest_list_copy(state,
                                    &(pThis->dataSuggestList),
                                    dataSuggestList);
    if (result != NE_OK)
    {
        // Destroy the above created 'NB_SuggestList' object
        NB_SuggestListDestroy(pThis);
        pThis = NULL;

        return result;
    }

    *suggestList = pThis;

    return result;
}

/*! Create a new instance of a 'NB_SuggestList' object

    Use this function to create a new instance of a 'NB_SuggestList' object.
    Call NB_SuggestListDestroy() to destroy the object.

    @return NE_OK if success
    @see NB_SuggestListDestroy
*/
NB_Error
CreateSuggestList(
    NB_Context* context,                    /*!< Pointer to current context */
    NB_SuggestList** suggestList            /*!< On success, returns pointer to 'NB_SuggestList' object */
    )
{
    NB_Error result = NE_OK;
    data_util_state* state = NULL;
    NB_SuggestList* pThis = NULL;

    if (!context || !suggestList)
    {
        return NE_INVAL;
    }

    state = NB_ContextGetDataState(context);

    pThis = nsl_malloc(sizeof(NB_SuggestList));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;

    // Initialize the 'data_suggest_list' object.
    result = data_suggest_list_init(state, &pThis->dataSuggestList);
    if (result != NE_OK)
    {
        NB_SuggestListDestroy(pThis);
        return result;
    }

    *suggestList = pThis;

    return result;
}

/*! @} */

