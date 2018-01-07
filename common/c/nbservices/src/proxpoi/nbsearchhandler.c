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

    @file     nbsearchhandler.c
    @defgroup nbhandler Handler

    Handler objects provide server communication functionality
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.                

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#include "paltypes.h"
#include "cslnetwork.h"
#include "nbgpstypes.h"
#include "nbsearchhandler.h"
#include "nbsearchparametersprivate.h"
#include "nbsearchinformationprivate.h"
#include "nbcontextprotected.h"
#include "nbanalyticsprotected.h"
#include "nbutility.h"
#include "nbqalog.h"

// Local Structures ..............................................................................

struct SearchHandlerNetQuery 
{
	NB_NetworkQuery	    net_query;
	NB_SearchHandler*	handler;
};

struct NB_SearchHandler
{
	NB_Context*                         context;

    /*! Actual Network Query for the download */
	struct SearchHandlerNetQuery        netQuery;		            

	boolean			                    inProgress;
    int				                    uploadPercent;
    int				                    downloadPercent;
	NB_RequestHandlerCallbackFunction   handlerCallback;
	void*			                    handlerCallbackUserData;

    /*! The following fields is used temporarily to hold previous event id to prevent subsequent calls to the analytic engine */
    uint32                              analyticsEventId;
    nb_boolean                          hasValidAnalyticsEventId;

    /*! this field is only valid during the call to the Request callback with the request status */
    NB_SearchInformation*               information;      

    /*! this field is used to temporarily hold the previous search query while the request is pending */
    data_proxpoi_query                  last_query;

    /* source of the current search */
    NB_SearchSource                     searchSource;

    /* input method from parameters for analytics */
    NB_SearchInputMethod                inputMethod;
};


// Local Functions ...............................................................................

static void SearchNetProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total);
static void SearchNetCallback(NB_NetworkQuery* query, NB_NetworkResponse* response);


// Public Functions ..............................................................................

// See description in header file
NB_DEF NB_Error 
NB_SearchHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, NB_SearchHandler** handler)
{
	NB_SearchHandler* pThis = NULL;

    if (handler == NULL)
    {
        return NE_INVAL;
    }

	if ((pThis = nsl_malloc(sizeof(*pThis))) == NULL)
    {
        *handler = NULL;
        return NE_NOMEM;
    }

	nsl_memset(pThis, 0, sizeof(*pThis));

	pThis->context = context;
    pThis->handlerCallback = callback->callback;
    pThis->handlerCallbackUserData = callback->callbackData;

	pThis->netQuery.net_query.query = NULL;

    data_proxpoi_query_init(NB_ContextGetDataState(context), &pThis->last_query);

    *handler = pThis;
    return NE_OK;
}

// See description in header file
NB_DEF NB_Error 
NB_SearchHandlerDestroy(NB_SearchHandler* handler)
{
	NB_Error err = NE_OK;
	NB_SearchHandler* pThis = handler;

    /* Cancel any pending network requests */
    if (NB_SearchHandlerIsRequestInProgress(pThis))
    {
        NB_SearchHandlerCancelRequest(pThis);
    }

    data_proxpoi_query_free(NB_ContextGetDataState(handler->context), &pThis->last_query);

	nsl_free(pThis);

    return err;
}

// See description in header file
NB_DEF NB_Error 
NB_SearchHandlerStartRequest(NB_SearchHandler* handler, NB_SearchParameters* parameters)
{
    NB_Error error = NE_OK;

    if (!handler || !parameters)
    {
        return NE_INVAL;
    }

	if (NB_SearchHandlerIsRequestInProgress(handler))
    {
        return NE_BUSY;
    }

    error = NB_SearchParametersGetSource(parameters, &handler->searchSource);
    if (error != NE_OK)
    {
        return error;
    }

    // Copy analytics event id from parameters
    handler->analyticsEventId = 0;
    handler->hasValidAnalyticsEventId = FALSE;
    if (NB_SearchParametersHasAnalyticsEventId(parameters))
    {
        handler->analyticsEventId = NB_SearchParametersGetAnalyticsEventId(parameters);
        handler->hasValidAnalyticsEventId = TRUE;
    }

    // Copy input method from parameters for analytics
    handler->inputMethod = NB_SearchParametersGetSearchInputMethod(parameters);

    error = NB_SearchParametersCopyQuery(parameters, &handler->last_query);

    if (error != NE_OK)
        return error;

    te_dealloc(handler->netQuery.net_query.query);

	handler->netQuery.net_query.query = NULL;

	handler->netQuery.net_query.action = SearchNetCallback;
	handler->netQuery.net_query.progress = SearchNetProgressCallback;
	handler->netQuery.net_query.qflags = TN_QF_DEFLATE;
	handler->netQuery.net_query.failFlags = TN_FF_CLOSE_ANY;

    nsl_strlcpy(handler->netQuery.net_query.target, NB_ContextGetTargetMapping(handler->context, "proxpoi"), sizeof(handler->netQuery.net_query.target));
	handler->netQuery.net_query.targetlen = -1;

    handler->netQuery.handler = handler;

	handler->netQuery.net_query.query = NB_SearchParametersToTPSQuery(parameters);

	if (handler->netQuery.net_query.query == NULL)
    {
        return NE_NOMEM;
    }

    handler->inProgress = TRUE;
    handler->uploadPercent = 0;
    handler->downloadPercent = 0;

	CSL_NetworkRequest(NB_ContextGetNetwork(handler->context), &handler->netQuery.net_query);

    return NE_OK;
}

// See description in header file
NB_DEF NB_Error 
NB_SearchHandlerCancelRequest(NB_SearchHandler* handler)
{
    if (!handler)
    {
        return NE_INVAL;
    }

    if (!handler->inProgress)
    {
        return NE_UNEXPECTED;
    }

	CSL_NetworkCancel(NB_ContextGetNetwork(handler->context), &handler->netQuery.net_query);

	return NE_OK;
}

// See description in header file
NB_DEF nb_boolean 
NB_SearchHandlerIsRequestInProgress(NB_SearchHandler* handler)
{
    return handler->inProgress;
}

// See description in header file
NB_DEF NB_Error 
NB_SearchHandlerGetSearchInformation(NB_SearchHandler* handler, NB_SearchInformation** searchInformation)
{
    *searchInformation = handler->information;

    if (handler->information == NULL)
    {
        return NE_UNEXPECTED;
    }

    handler->information = NULL;

    return NE_OK;
}

// Local Functions ...............................................................................

/*! Callback for network progress.

    @return None
*/
void 
SearchNetProgressCallback(NB_NetworkQuery* query, 
                          boolean up,               /*!< TRUE if upload is in progress, FALSE for downloads. */
                          uint32 cur, 
                          uint32 total)
{
	NB_SearchHandler* handler = ((struct SearchHandlerNetQuery*) query)->handler;
    int percent;

	if (up) 
    {
		percent = handler->uploadPercent = (cur * 100) / total;
    }
    else
    {
		percent = handler->downloadPercent = (cur * 100) / total;
	}

	if (handler->handlerCallback != NULL) 
    {
        // Forward the callback to the API user
        (handler->handlerCallback)(handler, NB_NetworkRequestStatus_Progress, NE_OK, up, percent, handler->handlerCallbackUserData);
    }
}


/*! Callback for network completion.

    @return None
*/
void 
SearchNetCallback(NB_NetworkQuery* query, NB_NetworkResponse* response)
{
	NB_Error error = NE_OK;
	NB_SearchHandler* handler = ((struct SearchHandlerNetQuery*) query)->handler;
	
    NB_RequestStatus status = NB_NetworkRequestStatus_Success;
	uint32 replyerr = NE_OK;

    uint32 analyticsEventId = 0;

	replyerr = GetReplyErrorCode(response);

	status = TranslateNetStatus(response->status);

	if (status == NB_NetworkRequestStatus_Success && response->reply != NULL) 
    {
        error = NB_SearchInformationCreateFromTPSReply(response->reply, &handler->last_query, handler->context, &handler->information);

        if (error != NE_OK) 
        {
            status = NB_NetworkRequestStatus_Failed;
    	}       
        else
        {
            // We need to report this search event to the analytics for this search event.
            // 1.   On the first result sets, the analytic function gets called and saves the event id
            //      in the information object.
            // 2.   Subsequent search calls, the event id is temporarily stored in the NB_SearchParameters
            // 3.   The parameters is passed to NB_SearchHandlerStartRequest and transfers that information
            //      into the NB_SearchHandler structure to be passed to this callback function.
            if ( (handler->searchSource != NB_SS_Carousel)  &&  // do not record analytics event for carousel search
                (handler->searchSource != NB_SS_Navigation) )  // do not record analytics event for Navigation search
            {
                NB_Error analyticsError = NE_OK;
                NB_LatitudeLongitude point = { 0 };
                NB_SearchInputMethod inputMethod = handler->inputMethod;
                NB_SearchEventCookie searchEventCookie = { 0 };

                data_proxpoi_query* searchQuery = NB_SearchInformationGetQuery(handler->information);

                point.latitude = searchQuery->position.point.lat;
                point.longitude = searchQuery->position.point.lon;

                // If this is not the first search result set, and user hasn't specified an
                // input method, default the search input method to More
                if (handler->hasValidAnalyticsEventId && inputMethod == NB_SIM_Undefined)
                {
                    inputMethod = NB_SIM_More;
                }

                NB_SearchInformationGetSearchEventCookie(handler->information, &searchEventCookie);

                // @todo: Remove it because it's an empty func only returning NOT_SUPPORTED.
                //analyticsError= NB_AnalyticsAddSearchQueryEventProtected(
                //    handler->context,
                //    &searchQuery->filter,
                //    searchQuery->scheme,
                //    &point,
                //    inputMethod,
                //    &searchEventCookie,
                //    &analyticsEventId);

                if (analyticsError == NE_OK)
                {
                    NB_SearchInformationSetAnalyticsEventId(handler->information, analyticsEventId);
                }
                else
                {
                    NB_QaLogAppState(handler->context, "Error adding search query event");
                }
            }
        }
    }

    handler->inProgress = FALSE;
    handler->downloadPercent = 100;

	/* clear the query since it is now invalid */
	query->query = NULL;

	if (handler->handlerCallback != NULL) 
    {
        // Forward the callback to the API user
        (handler->handlerCallback)(handler, 
                                   status, 
                                   replyerr ? replyerr : error, 
                                   FALSE, 
                                   handler->downloadPercent, 
                                   handler->handlerCallbackUserData);
    }

    /* clear the query since it is no longer needed now that the request is complete */
    data_proxpoi_query_free(NB_ContextGetDataState(handler->context), &handler->last_query);

    if (handler->information != NULL) 
    {
        NB_SearchInformationDestroy(handler->information);
        handler->information = NULL;
    }
}
