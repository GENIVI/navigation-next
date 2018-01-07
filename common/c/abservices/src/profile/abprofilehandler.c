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

    @file abprofilehandler.c

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


/*! @{ */


#include "abprofilehandler.h"
#include "abprofileparametersprivate.h"
#include "abprofileinformationprivate.h"
#include "nbcontextprotected.h"
#include "cslnetwork.h"
#include "nbutilityprotected.h"
#include "abqalog.h"


struct ProfileHandler_Query
{
	NB_NetworkQuery                 networkQuery;
	AB_ProfileHandler*              pThis;
};


struct AB_ProfileHandler
{
    NB_Context*                     context;
    NB_RequestHandlerCallback       callback;
    nb_boolean                      inProgress;
    struct ProfileHandler_Query     query;
    tpselt                          tpsReply;

    data_profile_query              currentQuery;
};


static void ProfileHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp);
static void ProfileHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total);


AB_DEF NB_Error
AB_ProfileHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, AB_ProfileHandler** handler)
{
	AB_ProfileHandler* pThis = 0;
    
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
    pThis->query.networkQuery.action = ProfileHandler_NetworkCallback;
	pThis->query.networkQuery.progress = ProfileHandler_NetworkProgressCallback;
	pThis->query.networkQuery.qflags = TN_QF_DEFLATE;
	pThis->query.networkQuery.failFlags = TN_FF_CLOSE_ANY;
    nsl_strlcpy(pThis->query.networkQuery.target, NB_ContextGetTargetMapping(pThis->context, "profile"), sizeof(pThis->query.networkQuery.target));
    pThis->query.networkQuery.targetlen = -1;

    data_profile_query_init(NB_ContextGetDataState(pThis->context), &pThis->currentQuery);

    *handler = pThis;

	return NE_OK;
}


AB_DEF NB_Error
AB_ProfileHandlerDestroy(AB_ProfileHandler* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    if (pThis->inProgress)
    {
        AB_ProfileHandlerCancelRequest(pThis);
    }

    te_dealloc(pThis->query.networkQuery.query);

    data_profile_query_free(NB_ContextGetDataState(pThis->context), &pThis->currentQuery);

    nsl_free(pThis);

    return NE_OK;
}


AB_DEF NB_Error
AB_ProfileHandlerStartRequest(AB_ProfileHandler* pThis, AB_ProfileParameters* parameters)
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

    pThis->query.networkQuery.query = AB_ProfileParametersToTPSQuery(parameters);
    if (!pThis->query.networkQuery.query)
    {
        return NE_NOMEM;
    }
    pThis->inProgress = TRUE;

    // copy query only if QA logging enabled
    if (NB_ContextGetQaLog(pThis->context) != NULL)
    {
        data_profile_query_copy(
            NB_ContextGetDataState(pThis->context),
            &pThis->currentQuery,
            AB_ProfileParametersGetProfileQuery(parameters));
    }

    CSL_NetworkRequest(NB_ContextGetNetwork(pThis->context), &pThis->query.networkQuery);

    return err;
}


AB_DEF NB_Error
AB_ProfileHandlerCancelRequest(AB_ProfileHandler* pThis)
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


AB_DEF nb_boolean
AB_ProfileHandlerIsRequestInProgress(AB_ProfileHandler* pThis)
{
    if (pThis)
    {
        return pThis->inProgress;
    }

    return 0;
}


AB_DEF NB_Error
AB_ProfileHandlerGetInformation(AB_ProfileHandler* pThis, AB_ProfileInformation** information)
{
    NB_Error err = NE_OK;

    if (!pThis || !information)
    {
        return NE_INVAL;
    }
    
    if (!pThis->tpsReply)
    {
        return NE_UNEXPECTED;
    }
    
    err = AB_ProfileInformationCreate(pThis->context, pThis->tpsReply, information);

    // log results only if QA logging enabled
    if (err == NE_OK && NB_ContextGetQaLog(pThis->context) != NULL)
    {
        data_util_state* dataState = NB_ContextGetDataState(pThis->context);

        const char* getKey = "";
        const char* setKey = "";
        const char* setValue = "";
        const char* returnedValue = "";

        uint32 errorCode = 0;
        const char* errorText = 0;

        getKey = data_string_get(dataState, &pThis->currentQuery.getValue.key);
        setKey = data_string_get(dataState, &pThis->currentQuery.setValue.key);

        if (pThis->currentQuery.isDefault)
        {
            setValue = "<default>";
        }
        else
        {
            setValue = data_string_get(dataState, &pThis->currentQuery.setValue.value);
        }

        (void)AB_ProfileInformationGetStringValue(*information, getKey, &returnedValue);
        errorCode = AB_ProfileInformationGetError(*information, &errorText);

        AB_QaLogProfileSetting(pThis->context, getKey, setKey, setValue, returnedValue, errorCode, errorText? errorText : "");
    }

    return err;
}


void
ProfileHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp)
{
	AB_ProfileHandler* pThis = ((struct ProfileHandler_Query*)query)->pThis;
	NB_NetworkRequestStatus status;
	
	uint32 replyerr = NE_OK;

	replyerr = GetReplyErrorCode(resp);

	status = TranslateNetStatus(resp->status);

	pThis->inProgress = 0;
	query->query = NULL;

    if (pThis->callback.callback != NULL)
    {
        // Make reply available for duration of callback
        pThis->tpsReply = resp->reply;
        (pThis->callback.callback)(pThis, status, replyerr, 0, 100, pThis->callback.callbackData);
        pThis->tpsReply = 0;
    }
}


void ProfileHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total)
{
	if (!up)
    {
    	AB_ProfileHandler* pThis = ((struct ProfileHandler_Query*) query)->pThis;

        if (pThis->callback.callback)
        {
    		int percent = (cur * 100) / total;
            (pThis->callback.callback)(pThis, NB_NetworkRequestStatus_Progress, NE_OK, 0, percent, pThis->callback.callbackData);
		}
	}
}


/*! @} */
