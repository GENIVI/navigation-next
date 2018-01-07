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

    @file     nbreversegeocodehandler.c
    @defgroup nbhandler Handler
*/
/*
    See file description in header file.

    (C) Copyright 2004 - 2009 by Networks In Motion, Inc.                

    The information contained herein is confidential, proprietary 
    to Networks In Motion, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */


#include "nbreversegeocodehandler.h"
#include "nbreversegeocodeinformationprivate.h"
#include "nbreversegeocodeparametersprivate.h"
#include "nbcontextprotected.h"
#include "cslnetwork.h"
#include "data_reverse_geocode_reply.h"
#include "nbutilityprotected.h"


struct ReverseGeocodeHandler_Query
{
	NB_NetworkQuery                     networkQuery;
	NB_ReverseGeocodeHandler*           pThis;
};


struct NB_ReverseGeocodeHandler
{
    NB_Context*                         context;
    NB_RequestHandlerCallback           callback;
    boolean                             inProgress;
    struct ReverseGeocodeHandler_Query  query;
    tpselt                              tpsReply;
};


static void ReverseGeocodeHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp);
static void ReverseGeocodeHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total);


NB_DEF NB_Error
NB_ReverseGeocodeHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, NB_ReverseGeocodeHandler** handler)
{
	NB_ReverseGeocodeHandler* pThis = 0;
    
    if (!context || !handler)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(*pThis));
	if (pThis == NULL)
    {
		return NE_NOMEM;
    }

	nsl_memset(pThis, 0, sizeof(*pThis));

	pThis->context = context;
    if (callback)
    {
        pThis->callback.callback = callback->callback;
        pThis->callback.callbackData = callback->callbackData;
    }

    pThis->query.pThis = pThis;
    pThis->query.networkQuery.action = ReverseGeocodeHandler_NetworkCallback;
	pThis->query.networkQuery.progress = ReverseGeocodeHandler_NetworkProgressCallback;
	pThis->query.networkQuery.qflags = TN_QF_DEFLATE;
	pThis->query.networkQuery.failFlags = TN_FF_CLOSE_ANY;
    nsl_strlcpy(pThis->query.networkQuery.target, NB_ContextGetTargetMapping(pThis->context, "reverse-geocode"), sizeof(pThis->query.networkQuery.target));
    pThis->query.networkQuery.targetlen = -1;

    *handler = pThis;

	return NE_OK;
}


NB_DEF NB_Error
NB_ReverseGeocodeHandlerDestroy(NB_ReverseGeocodeHandler* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    if (pThis->inProgress)
    {
        NB_ReverseGeocodeHandlerCancelRequest(pThis);
    }

    te_dealloc(pThis->query.networkQuery.query);
	nsl_free(pThis);

    return NE_OK;
}



NB_DEF NB_Error
NB_ReverseGeocodeHandlerStartRequest(NB_ReverseGeocodeHandler* pThis, NB_ReverseGeocodeParameters* parameters)
{
    NB_Error err = NE_OK;

    if (!pThis || !parameters)
    {
        return NE_INVAL;
    }

    if (pThis->inProgress)
    {
        return NE_BUSY;
    }

    te_dealloc(pThis->query.networkQuery.query);

    pThis->query.networkQuery.query = NB_ReverseGeocodeParametersToTPSQuery(parameters);
    if (!pThis->query.networkQuery.query)
    {
        return NE_NOMEM;
    }

    pThis->inProgress = TRUE;

    CSL_NetworkRequest(NB_ContextGetNetwork(pThis->context), &pThis->query.networkQuery);

    return err;
}


NB_DEF NB_Error
NB_ReverseGeocodeHandlerCancelRequest(NB_ReverseGeocodeHandler* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    if (!pThis->inProgress)
    {
        return NE_UNEXPECTED;
    }

	CSL_NetworkCancel(NB_ContextGetNetwork(pThis->context), &pThis->query.networkQuery);

    return NE_OK;
}


NB_DEF nb_boolean
NB_ReverseGeocodeHandlerIsRequestInProgress(NB_ReverseGeocodeHandler* pThis)
{
    if (pThis)
    {
        return pThis->inProgress;
    }

    return 0;
}


NB_DEF NB_Error
NB_ReverseGeocodeHandlerGetReverseGeocodeInformation(NB_ReverseGeocodeHandler* pThis, NB_ReverseGeocodeInformation** information)
{
    if (!pThis || !information)
    {
        return NE_INVAL;
    }

    if (!pThis->tpsReply)
    {
        return NE_UNEXPECTED;
    }

    return NB_ReverseGeocodeInformationCreate(pThis->context, pThis->tpsReply, information);
}


void
ReverseGeocodeHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp)
{
	NB_ReverseGeocodeHandler* pThis = ((struct ReverseGeocodeHandler_Query*)query)->pThis;
	NB_NetworkRequestStatus status;
	uint32 replyerr = NE_OK;

	replyerr = GetReplyErrorCode(resp);

    status = TranslateNetStatus(resp->status);

	pThis->inProgress = FALSE;
	query->query = NULL;

    if (pThis->callback.callback != NULL)
    {
        // Make reply available for duration of callback
        pThis->tpsReply = resp->reply;
        (pThis->callback.callback)(pThis, status, replyerr, FALSE, 100, pThis->callback.callbackData);
        pThis->tpsReply = 0;
    }
}


void
ReverseGeocodeHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total)
{
	NB_ReverseGeocodeHandler* pThis = ((struct ReverseGeocodeHandler_Query*) query)->pThis;

    if (pThis->callback.callback)
    {
		int percent = (cur * 100) / total;
        (pThis->callback.callback)(pThis, NB_NetworkRequestStatus_Progress, NE_OK, up, percent, pThis->callback.callbackData);
	}
}


/*! @} */
