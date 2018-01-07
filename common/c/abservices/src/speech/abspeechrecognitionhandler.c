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

    @file     abdatastorehandler.c
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


#include "abspeechrecognitionhandler.h"
#include "abspeechrecognitionparametersprivate.h"
#include "abspeechrecognitioninformationprivate.h"
#include "nbcontextprotected.h"
#include "cslnetwork.h"
#include "nbutilityprotected.h"
#include "abqalog.h"


struct SpeechRecognitionHandler_Query
{
	NB_NetworkQuery                 networkQuery;
	AB_SpeechRecognitionHandler*    pThis;
};


struct AB_SpeechRecognitionHandler
{
    NB_Context*                             context;
    NB_RequestHandlerCallback               callback;
    nb_boolean                              inProgress;
    struct SpeechRecognitionHandler_Query   query;
    tpselt                                  tpsReply;
};


static void SpeechRecognitionHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp);
static void SpeechRecognitionHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total);


AB_DEF NB_Error
AB_SpeechRecognitionHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, AB_SpeechRecognitionHandler** handler)
{
	AB_SpeechRecognitionHandler* pThis = 0;
    
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
    pThis->query.networkQuery.action = SpeechRecognitionHandler_NetworkCallback;
	pThis->query.networkQuery.progress = SpeechRecognitionHandler_NetworkProgressCallback;
	pThis->query.networkQuery.qflags = TN_QF_DEFLATE;
	pThis->query.networkQuery.failFlags = TN_FF_CLOSE_ANY;
    nsl_strlcpy(pThis->query.networkQuery.target, NB_ContextGetTargetMapping(pThis->context, "asr-reco"), sizeof(pThis->query.networkQuery.target));
    pThis->query.networkQuery.targetlen = -1;
    
    *handler = pThis;

	return NE_OK;
}


AB_DEF NB_Error
AB_SpeechRecognitionHandlerDestroy(AB_SpeechRecognitionHandler* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    if (pThis->inProgress)
    {
        AB_SpeechRecognitionHandlerCancelRequest(pThis);
    }

    te_dealloc(pThis->query.networkQuery.query);
	nsl_free(pThis);

    return NE_OK;
}


AB_DEF NB_Error
AB_SpeechRecognitionHandlerStartRequest(AB_SpeechRecognitionHandler* pThis, AB_SpeechRecognitionParameters* parameters)
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

    if (pThis->query.networkQuery.query)
    {
	    te_dealloc(pThis->query.networkQuery.query);
    }

    pThis->query.networkQuery.query = AB_SpeechRecognitionParametersToTPSQuery(parameters);
    if (!pThis->query.networkQuery.query)
    {
        return NE_NOMEM;
    }
    pThis->inProgress = TRUE;
    
    CSL_NetworkRequest(NB_ContextGetNetwork(pThis->context), &pThis->query.networkQuery);

    return err;
}


AB_DEF NB_Error
AB_SpeechRecognitionHandlerCancelRequest(AB_SpeechRecognitionHandler* pThis)
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
AB_SpeechRecognitionHandlerIsRequestInProgress(AB_SpeechRecognitionHandler* pThis)
{
    if (pThis)
    {
        return pThis->inProgress;
    }

    return 0;
}


AB_DEF NB_Error
AB_SpeechRecognitionHandlerGetInformation(AB_SpeechRecognitionHandler* pThis, AB_SpeechRecognitionInformation** information)
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
    
    err = AB_SpeechRecognitionInformationCreate(pThis->context, pThis->tpsReply, information);

    if (err == NE_OK && NB_ContextGetQaLog(pThis->context) != NULL)
    {
        AB_SpeechRecognitionResults results = { 0 };
        if (AB_SpeechRecognitionInformationGetRecognizeResults(*information, &results) == NE_OK)
        {
            if (results.lastFieldName && *results.lastFieldName)
            {
                AB_QaLogSpeechRecognitionAmbiguousReply(pThis->context, results.lastFieldName, results.completionCode, results.locationMatchCount + results.placeMatchCount, results.interactionID);
            }

            if (results.locationMatchCount)
            {
                NB_Location location = { {0} };
                uint32 n = 0;

                for (n = 0; n < results.locationMatchCount; n++)
                {
                    if (AB_SpeechRecognitionInformationGetRecognizedLocation(*information, n, &location) == NE_OK)
                    {
                        AB_QaLogSpeechRecognitionRecognizedLocationReply(pThis->context, &location, results.interactionID);
                    }
                }
            }

            if (results.placeMatchCount)
            {
                NB_Place place = { {0} };
                uint32 n = 0;

                for (n = 0; n < results.placeMatchCount; n++)
                {
                    if (AB_SpeechRecognitionInformationGetRecognizedPlace(*information, n, &place, 0) == NE_OK)
                    {
                        AB_QaLogSpeechRecognitionRecognizedPlaceReply(pThis->context, &place, results.interactionID);
                    }
                }
            }
        }
    }
    
    return err;
}


void
SpeechRecognitionHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp)
{
	AB_SpeechRecognitionHandler* pThis = ((struct SpeechRecognitionHandler_Query*)query)->pThis;
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


void
SpeechRecognitionHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total)
{
	if (!up)
    {
    	AB_SpeechRecognitionHandler* pThis = ((struct SpeechRecognitionHandler_Query*) query)->pThis;

        if (pThis->callback.callback)
        {
    		int percent = (cur * 100) / total;
            (pThis->callback.callback)(pThis, NB_NetworkRequestStatus_Progress, NE_OK, 0, percent, pThis->callback.callbackData);
		}
	}
}


/*! @} */
