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

    @file     nbsinglesearchhandler.c
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "nbsinglesearchhandler.h"
#include "nbsinglesearchinformationprivate.h"
#include "nbsinglesearchparametersprivate.h"
#include "nbcontextprotected.h"
#include "nbutilityprotected.h"
#include "nbanalyticsprotected.h"
#include "nbqalog.h"
#include "cslnetwork.h"

/*! @{ */

// Constants .........................................................................................................


// Local types .......................................................................................................

/*! The single search handler */
struct NB_SingleSearchHandler
{
    NB_NetworkQuery                 networkQuery;                   /*!< The network query. This element must be the first member
                                                                         of NB_SingleSearchHandler. Because the pointer of this
                                                                         element needs to be casted to NB_SingleSearchHandler in the
                                                                         callback function of NB_NetworkQuery. */
    NB_SingleSearchInformation*     information;                    /*!< Single search result */
    NB_Context*                     context;                        /*!< Pointer to current context */
    NB_RequestHandlerCallback       handlerCallback;                /*!< Request handler callback for single search */
    NB_SearchSource                 source;                         /*!< Source of current single search */
    NB_SearchInputMethod            inputMethod;                    /*!< Input method for analytics */
    uint32                          analyticsEventId;               /*!< Previous analytics event id */
    nb_boolean                      isAnalyticsEventIdValid;        /*!< Is previous analytics event id valid? */
    nb_boolean                      inProgress;                     /*!< Is single search request in progress? */
    data_search_query               lastQuery;                      /*!< The last query */
};


// Declaration of local functions ....................................................................................

static void SingleSearchNetworkActionCallback(NB_NetworkQuery* networkQuery, NB_NetworkResponse* networkResponse);
static void SingleSearchNetworkProgressCallback(NB_NetworkQuery* networkQuery, boolean up, uint32 have, uint32 total);


// Public functions ..................................................................................................

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchHandlerCreate(NB_Context* context,
                             NB_RequestHandlerCallback* callback,
                             NB_SingleSearchHandler** handler)
{
    NB_Error result = NE_OK;
    NB_SingleSearchHandler* pThis = NULL;

    if (!context || !callback || !handler)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    result = data_search_query_init(NB_ContextGetDataState(context), &(pThis->lastQuery));
    if (result != NE_OK)
    {
        NB_SingleSearchHandlerDestroy(pThis);
        return result;
    }

    pThis->information = NULL;
    pThis->context = context;
    pThis->handlerCallback.callback = callback->callback;
    pThis->handlerCallback.callbackData = callback->callbackData;
    pThis->source = NB_SS_None;
    pThis->inputMethod = NB_SIM_Undefined;
    pThis->analyticsEventId = 0;
    pThis->isAnalyticsEventIdValid = FALSE;
    pThis->inProgress = FALSE;

    *handler = pThis;

    return result;
}

/* See header file for description */
NB_DEF void
NB_SingleSearchHandlerDestroy(NB_SingleSearchHandler* handler)
{
    if (!handler)
    {
        return;
    }

    if (NB_SingleSearchHandlerIsRequestInProgress(handler))
    {
        NB_SingleSearchHandlerCancelRequest(handler);
    }

    data_search_query_free(NB_ContextGetDataState(handler->context), &(handler->lastQuery));

    nsl_free(handler);
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchHandlerStartRequest(NB_SingleSearchHandler* handler,
                                   NB_SingleSearchParameters* parameters)
{
    NB_Error result = NE_OK;

    if (!handler || !parameters)
    {
        return NE_INVAL;
    }

    // Check if request is in progress
    if (NB_SingleSearchHandlerIsRequestInProgress(handler))
    {
        return NE_BUSY;
    }

    // Get the search source
    result = NB_SingleSearchParametersGetSource(parameters, &(handler->source));
    if (result != NE_OK)
    {
        return result;
    }

    // Get the input method
    result = NB_SingleSearchParametersGetSearchInputMethod(parameters, &(handler->inputMethod));
    if (result != NE_OK)
    {
        return result;
    }

    // Copy the analytics event id
    handler->isAnalyticsEventIdValid = (nb_boolean) (NB_SingleSearchParametersGetAnalyticsEventId(parameters, &(handler->analyticsEventId)) == NE_OK);

    // Copy the last query
    result = NB_SingleSearchParametersCopyQuery(parameters, &(handler->lastQuery));
    if (result != NE_OK)
    {
        return result;
    }

    // Clear the network query
    if (handler->networkQuery.query)
    {
        te_dealloc(handler->networkQuery.query);
        handler->networkQuery.query = NULL;
    }

    handler->networkQuery.action = SingleSearchNetworkActionCallback;
    handler->networkQuery.progress = SingleSearchNetworkProgressCallback;
    handler->networkQuery.qflags = TN_QF_DEFLATE;
    handler->networkQuery.failFlags = TN_FF_CLOSE_ANY;
    nsl_strlcpy(handler->networkQuery.target, NB_ContextGetTargetMapping(handler->context, "search"), sizeof(handler->networkQuery.target));
    handler->networkQuery.targetlen = -1;
    handler->networkQuery.query = NB_SingleSearchParametersToTPSQuery(parameters);
    if (!(handler->networkQuery.query))
    {
        return NE_NOMEM;
    }

    handler->inProgress = TRUE;

    CSL_NetworkRequest(NB_ContextGetNetwork(handler->context), &(handler->networkQuery));

    return result;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchHandlerCancelRequest(NB_SingleSearchHandler* handler)
{
    if (!handler)
    {
        return NE_INVAL;
    }

    if (!(handler->inProgress))
    {
        return NE_UNEXPECTED;
    }

    CSL_NetworkCancel(NB_ContextGetNetwork(handler->context), &(handler->networkQuery));

    return NE_OK;
}

/* See header file for description */
NB_DEF nb_boolean
NB_SingleSearchHandlerIsRequestInProgress(NB_SingleSearchHandler* handler)
{
    if (!handler)
    {
        return FALSE;
    }

    return handler->inProgress;
}

/* See header file for description */
NB_DEF NB_Error
NB_SingleSearchHandlerGetInformation(NB_SingleSearchHandler* handler,
                                     NB_SingleSearchInformation** information)
{
    if (!handler || !information)
    {
        return NE_INVAL;
    }

    if (!(handler->information))
    {
        return NE_UNEXPECTED;
    }

    *information = handler->information;
    handler->information = NULL;

    return NE_OK;
}


// Private functions .................................................................................................


// Definition of local functions .....................................................................................

/*! Callback function for network request completion

    @return None
*/
void
SingleSearchNetworkActionCallback(NB_NetworkQuery* networkQuery,                /*!< The network query */
                                  NB_NetworkResponse* networkResponse           /*!< The network response */
                                  )
{
    NB_Error replyError = NE_OK;
    NB_Error result = NE_OK;
    NB_RequestStatus status = NB_NetworkRequestStatus_Success;
    NB_SingleSearchHandler* handler = (NB_SingleSearchHandler*) networkQuery;

    status = TranslateNetStatus(networkResponse->status);

    if (status == NB_NetworkRequestStatus_Success && networkResponse->reply)
    {
        result = NB_SingleSearchInformationCreateFromTPSReply(networkResponse->reply, &(handler->lastQuery), handler->context, handler->source, &(handler->information));
        if (result == NE_OK)
        {
            // Report this search event to the analytics
            // 1. On the first result sets, the analytics function gets called and saves the event id in the NB_SingleSearchInformation object.
            // 2. Subsequent search calls, the event id is temporarily stored in the NB_SingleSearchParameters object.
            // 3. The NB_SingleSearchParameters object is passed to the NB_SingleSearchHandlerStartRequest function and transfers the event id into
            //    the NB_SingleSearchHandler object to be passed to this callback function.

            uint32 analyticsEventId = 0;
            NB_Error analyticsError = NE_OK;
            NB_SearchInputMethod inputMethod = handler->inputMethod;
            NB_LatitudeLongitude point = {0};
            data_search_query* searchQuery = NB_SingleSearchInformationGetQuery(handler->information);

            point.latitude = searchQuery->position.point.lat;
            point.longitude = searchQuery->position.point.lon;

            // If this is not the first search result set and user has not specified an input method, set the search input method to NB_SIM_More.
            if (handler->isAnalyticsEventIdValid && inputMethod == NB_SIM_Undefined)
            {
                inputMethod = NB_SIM_More;
            }

            // @todo: Remove it because it's an empty func only returning NOT_SUPPORTED.
            // @todo: Check if setting the search event cookie NULL is right. Because there is no search event cookie in single search.
            //analyticsError = analyticsError ? analyticsError :
            //    NB_AnalyticsAddSearchQueryEventProtected(handler->context,
            //                                             &(searchQuery->search_filter),
            //                                             searchQuery->scheme,
            //                                             &point,
            //                                             inputMethod,
            //                                             NULL,
            //                                             &analyticsEventId);
            analyticsError = analyticsError ? analyticsError :
                NB_SingleSearchInformationSetAnalyticsEventId(handler->information, analyticsEventId);
            if (analyticsError != NE_OK)
            {
                NB_QaLogAppState(handler->context, "Error adding search query event");
            }
        }
        else
        {
            status = NB_NetworkRequestStatus_Failed;
        }
    }

    // Clear the last query since it is no longer needed now that the request is complete
    data_search_query_free(NB_ContextGetDataState(handler->context), &(handler->lastQuery));

    handler->inProgress = FALSE;

    // Clear the query since it is now invalid
    networkQuery->query = NULL;

    replyError = (NB_Error) GetReplyErrorCode(networkResponse);
    if (handler->handlerCallback.callback)
    {
        // Forward the single search result to user
        (handler->handlerCallback.callback)(handler, status, replyError ? replyError : result, FALSE, 100, handler->handlerCallback.callbackData);
    }

    // Clear the single search result
    if (handler->information)
    {
        NB_SingleSearchInformationDestroy(handler->information);
        handler->information = NULL;
    }
}

/*! Callback function for network request in progress

    @return None
*/
void
SingleSearchNetworkProgressCallback(NB_NetworkQuery* networkQuery,              /*!< The network query */
                                    boolean up,                                 /*!< TRUE if upload is in progress. FALSE for download. */
                                    uint32 have,                                /*!< Current completed */
                                    uint32 total                                /*!< Total */
                                    )
{
    if (!up)
    {
        NB_SingleSearchHandler* handler = (NB_SingleSearchHandler*) networkQuery;
        if (handler->handlerCallback.callback)
        {
            int percent = (have * 100) / total;
            (handler->handlerCallback.callback)(handler, NB_NetworkRequestStatus_Progress, NE_OK, FALSE, percent, handler->handlerCallback.callbackData);
        }
    }
}

/*! @} */
