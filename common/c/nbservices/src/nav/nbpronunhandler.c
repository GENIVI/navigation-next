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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#include "nbcontext.h"
#include "nbcontextprotected.h"
#include "nbpronuninformation.h"
#include "nbpronuninformationprivate.h"
#include "nbpronunhandler.h"
#include "cslnetwork.h"
#include "data_pronun_query.h"
#include "data_pronun_reply.h"
#include "nbutility.h"
#include "nbpronunparameters.h"
#include "nbpronunparametersprivate.h"
#include "nbqalog.h"

typedef struct PronunHandler_NetQuery
{
    NB_NetworkQuery	net_query;
    NB_PronunHandler*	pThis;
} PronunHandler_NetQuery;

struct NB_PronunHandler
{
  NB_Context*               context;
  NB_PronunParameters*      parameters;
  NB_RequestHandlerCallback callback;
  PronunHandler_NetQuery	netquery;   
  boolean                   download_inprogress;
  int                       download_percent;  
  tpselt                    tpsReply;  
};

static void NB_PronunHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp);
static void NB_PronunHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total);

NB_DEF NB_Error 
NB_PronunHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, NB_PronunHandler** handler)
{
    NB_PronunHandler* pThis = NULL;

    data_util_state* dataState = NULL; 

    if (!context)
    {
        return NE_INVAL;
    }

    if ((pThis = nsl_malloc(sizeof(NB_PronunHandler))) == NULL)
    {
        return NE_NOMEM;
    }

    nsl_memset(pThis, 0, sizeof(NB_PronunHandler));

    dataState = NB_ContextGetDataState(context);

    if (!dataState)
    {
        nsl_free(pThis);
        return NE_INVAL;
    }

    pThis->context = context;

    if (callback)
    {
        pThis->callback.callback = callback->callback;
        pThis->callback.callbackData = callback->callbackData;
    }

    pThis->download_inprogress = FALSE;
    pThis->download_percent = 0;

    pThis->netquery.net_query.action = NB_PronunHandler_NetworkCallback;
    pThis->netquery.net_query.progress = NB_PronunHandler_NetworkProgressCallback;
    pThis->netquery.net_query.qflags = TN_QF_DEFLATE;
    pThis->netquery.net_query.failFlags = TN_FF_CLOSE_ANY;
    nsl_strlcpy(pThis->netquery.net_query.target, NB_ContextGetTargetMapping(pThis->context, "pronun"), sizeof(pThis->netquery.net_query.target));
    pThis->netquery.net_query.targetlen = -1;
    pThis->netquery.pThis = pThis;
    pThis->parameters = NULL;

    (*handler) = pThis;

    return NE_OK;
}

NB_DEF NB_Error
NB_PronunHandlerDestroy(NB_PronunHandler* handler)
{
    if (handler)
    {
        if (NB_PronunHandlerIsRequestInProgress(handler))
        {
            NB_PronunHandlerCancelRequest(handler);
        }
        if (handler->parameters)
        {
            NB_PronunParametersDestroy(handler->parameters);
        }
        nsl_free(handler);
        return NE_OK;
    }
    return NE_INVAL;
}

NB_DEF NB_Error 
NB_PronunHandlerStartRequest(NB_PronunHandler* handler, NB_PronunParameters* parameters)
{
    NB_Error err = NE_OK;

    if (!handler || !parameters)
    {
        return NE_INVAL;
    }

    if (NB_PronunHandlerIsRequestInProgress(handler))
    {
        return NE_BUSY;
    }

    te_dealloc(handler->netquery.net_query.query);

    handler->netquery.net_query.query = NB_PronunParametersToTPSQuery(parameters);

    if (handler->netquery.net_query.query == NULL)
        return NE_NOMEM;

    handler->download_inprogress = TRUE;    
    handler->download_percent = 0;

    if (handler->parameters)
    {
        NB_PronunParametersDestroy(handler->parameters);
    }
           
    CSL_NetworkRequest( NB_ContextGetNetwork(handler->context), &handler->netquery.net_query);

    err = NE_OK;

    return err;
}

NB_DEF NB_Error 
NB_PronunHandlerCancelRequest(NB_PronunHandler* handler)
{
    if (!handler)
    {
        return NE_INVAL;
    }

    if (!handler->download_inprogress)
    {
        return NE_UNEXPECTED;
    }

    CSL_NetworkCancel(NB_ContextGetNetwork(handler->context), &handler->netquery.net_query);

    return NE_OK;
}

NB_DEF nb_boolean 
NB_PronunHandlerIsRequestInProgress(NB_PronunHandler* handler)
{ 
    if (handler)
    {
        return handler->download_inprogress;
    }
    return 0;
}

NB_DEF NB_Error 
NB_PronunHandlerGetInformation(NB_PronunHandler* handler, NB_PronunInformation** pronunInformation)
{
    NB_Error err = NE_OK;
    if (!handler)
        return NE_INVAL;

    err = NB_PronunInformationCreateFromTPSReply(handler->context, handler->tpsReply, handler->parameters, pronunInformation);
    
    return err;
}

static void NB_PronunHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp)
{
    NB_Error err = NE_OK;
    NB_PronunHandler* handler = ((PronunHandler_NetQuery*)query)->pThis;
    NB_NetworkRequestStatus status = NB_NetworkRequestStatus_Failed;    
    uint32 replyerr = NE_OK;

    replyerr = GetReplyErrorCode(resp);

    handler->download_inprogress = FALSE;
    handler->download_percent = 100;
    
    status = TranslateNetStatus(resp->status);
    
    if (status == NB_NetworkRequestStatus_Success) 
    {
      
    }
   
    if (handler->callback.callback != NULL)
    {        
        handler->tpsReply = resp->reply;                
        (handler->callback.callback)(handler, status, replyerr ? replyerr : err, FALSE, 100, handler->callback.callbackData);
        handler->tpsReply = 0;
    }

    query->query = NULL;
}

static void NB_PronunHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total)
{
    NB_PronunHandler* handler = ((struct PronunHandler_NetQuery*) query)->pThis;

    if (handler->callback.callback)
    {
        int percent = (cur * 100) / total;        
        (handler->callback.callback)(handler, NB_NetworkRequestStatus_Progress, NE_OK, up, percent, handler->callback.callbackData);    
    }
}
