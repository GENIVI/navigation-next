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
#include "nbtraffichandler.h"
#include "nbtrafficparametersprivate.h"
#include "nbtrafficinformationprivate.h"
#include "cslnetwork.h"
#include "nbutility.h"

typedef struct TrafficHandler_NetQuery
{
    NB_NetworkQuery	net_query;
    NB_TrafficHandler*	pThis;
} TrafficHandler_NetQuery;

struct NB_TrafficHandler 
{
  NB_Context*               context;
  NB_RequestHandlerCallback callback;
  TrafficHandler_NetQuery	netquery;  
  tpselt                    tpsReply;
  boolean                   download_inprogress;
  int                       download_percent;
};

static void TrafficHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp);
static void TrafficHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total);

NB_DEF NB_Error 
NB_TrafficHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, NB_TrafficHandler** handler)
{
    NB_TrafficHandler* pThis = NULL;

    data_util_state* dataState = NULL; 

    if (!context)
    {
        return NE_INVAL;
    }

    if ((pThis = nsl_malloc(sizeof(NB_TrafficHandler))) == NULL)
    {
        return NE_NOMEM;
    }

    nsl_memset(pThis, 0, sizeof(NB_TrafficHandler));

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
   
    pThis->netquery.net_query.action = TrafficHandler_NetworkCallback;
    pThis->netquery.net_query.progress = TrafficHandler_NetworkProgressCallback;
    pThis->netquery.net_query.qflags = TN_QF_DEFLATE;    
    pThis->netquery.net_query.failFlags = TN_FF_NOCLOSE;
    nsl_strlcpy(pThis->netquery.net_query.target, NB_ContextGetTargetMapping(pThis->context, "traffic-notify"), sizeof(pThis->netquery.net_query.target));
    pThis->netquery.net_query.targetlen = -1;    
    pThis->netquery.pThis = pThis;

    (*handler) = pThis;

    return NE_OK;
}

NB_DEF NB_Error 
NB_TrafficHandlerDestroy(NB_TrafficHandler* handler)
{
    if (handler)
    {        
        if (NB_TrafficHandlerIsRequestInProgress(handler))
        {
            NB_TrafficHandlerCancelRequest(handler);
        }

        nsl_free(handler);
        return NE_OK;
    }
    return NE_INVAL;
}

NB_DEF NB_Error 
NB_TrafficHandlerStartRequest(NB_TrafficHandler* handler, NB_TrafficParameters* parameters)
{
    NB_Error err = NE_OK;

    if (!handler || !parameters)
    {
        return NE_INVAL;
    }

    if (!NB_TrafficParametersIsIdentifierValid(parameters))
    {
        return NE_INVAL;
    }

    if (NB_TrafficHandlerIsRequestInProgress(handler))
    {
        return NE_BUSY;
    }

    te_dealloc(handler->netquery.net_query.query);

    handler->netquery.net_query.query = NB_TrafficParametersToTPSQuery(parameters);

    if (handler->netquery.net_query.query == NULL)
        return NE_NOMEM;

    handler->download_inprogress = TRUE;    
    handler->download_percent = 0;
       
    CSL_NetworkRequest( NB_ContextGetNetwork(handler->context), &handler->netquery.net_query);

    err = NE_OK;

    return err;
}

NB_DEF NB_Error 
NB_TrafficHandlerCancelRequest(NB_TrafficHandler* handler)
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
NB_TrafficHandlerIsRequestInProgress(NB_TrafficHandler* handler)
{ 
    if (handler)
    {
        return handler->download_inprogress;
    }
    return 0;
}

NB_DEF NB_Error 
NB_TrafficHandlerGetTrafficInformation(NB_TrafficHandler* handler, NB_TrafficInformation** trafficInformation)
{
    NB_Error err = NE_OK;
    if (!handler || !trafficInformation)
        return NE_INVAL;

    err = NB_TrafficInformationCreateFromTPSReply(handler->context, handler->tpsReply, trafficInformation);

    return err;
}

static void TrafficHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp)
{
    NB_Error err = NE_OK;
    NB_TrafficHandler* handler = ((TrafficHandler_NetQuery*)query)->pThis;    
    NB_NetworkRequestStatus status = NB_NetworkRequestStatus_Failed;    
    uint32 replyerr = NE_OK;

    replyerr = GetReplyErrorCode(resp);

    handler->download_inprogress = FALSE;
    handler->download_percent = 100;    

    status = TranslateNetStatus(resp->status);

    if (handler->callback.callback != NULL)
    {        
        handler->tpsReply = resp->reply;                
        (handler->callback.callback)(handler, status, replyerr ? replyerr : err, FALSE, 100, handler->callback.callbackData);
        handler->tpsReply = 0;
    }
    
    query->query = NULL;
}

static void TrafficHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total)
{
    NB_TrafficHandler* pThis = ((struct TrafficHandler_NetQuery*) query)->pThis;

    if (pThis->callback.callback)
    {
        int percent = (cur * 100) / total;        
        (pThis->callback.callback)(pThis, NB_NetworkRequestStatus_Progress, NE_OK, up, percent, pThis->callback.callbackData);    
    }
}
