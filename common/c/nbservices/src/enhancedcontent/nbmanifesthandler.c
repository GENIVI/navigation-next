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

    @file nbmanifesthandler.c
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#include "nbmanifesthandler.h"
#include "nbmanifestparameters.h"
#include "nbmanifestinformation.h"
#include "nbcontextprotected.h"
#include "cslnetwork.h"
#include "nbutilityprotected.h"


struct ManifestHandler_Query
{
    NB_NetworkQuery                 networkQuery;
    NB_ManifestHandler*             pThis;
};


struct NB_ManifestHandler
{
    NB_Context*                     context;
    NB_RequestHandlerCallback       callback;
    nb_boolean                      inProgress;
    struct ManifestHandler_Query    query;
    tpselt                          tpsReply;
};


static void ManifestHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* response);
static void ManifestHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total);


NB_Error
NB_ManifestHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, NB_ManifestHandler** handler)
{
    NB_ManifestHandler* pThis = 0;
    
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
    pThis->query.networkQuery.action = ManifestHandler_NetworkCallback;
    pThis->query.networkQuery.progress = ManifestHandler_NetworkProgressCallback;
    pThis->query.networkQuery.qflags = TN_QF_DEFLATE;
    pThis->query.networkQuery.failFlags = TN_FF_CLOSE_ANY;
    nsl_strlcpy(pThis->query.networkQuery.target, NB_ContextGetTargetMapping(pThis->context, "metadata"), sizeof(pThis->query.networkQuery.target));
    pThis->query.networkQuery.targetlen = -1;
    
    *handler = pThis;

    return NE_OK;
}


NB_Error
NB_ManifestHandlerDestroy(NB_ManifestHandler* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    if (pThis->inProgress)
    {
        NB_ManifestHandlerCancelRequest(pThis);
    }

    te_dealloc(pThis->query.networkQuery.query);
    nsl_free(pThis);

    return NE_OK;
}


NB_Error
NB_ManifestHandlerStartRequest(NB_ManifestHandler* pThis, NB_ManifestParameters* parameters)
{
    NB_Error result = NE_OK;

    if (!pThis || !parameters)
    {
        return NE_INVAL;
    }

    if (pThis->inProgress)
    {
        return NE_BUSY;
    }

    te_dealloc(pThis->query.networkQuery.query);

    pThis->query.networkQuery.query = NB_ManifestParametersToTPSQuery(parameters);
    if (!pThis->query.networkQuery.query)
    {
        return NE_NOMEM;
    }
    pThis->inProgress = TRUE;

    CSL_NetworkRequest(NB_ContextGetNetwork(pThis->context), &pThis->query.networkQuery);

    return result;
}


NB_Error
NB_ManifestHandlerCancelRequest(NB_ManifestHandler* pThis)
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


nb_boolean
NB_ManifestHandlerIsRequestInProgress(NB_ManifestHandler* pThis)
{
    if (pThis)
    {
        return pThis->inProgress;
    }

    return 0;
}


NB_Error
NB_ManifestHandlerGetInformation(NB_ManifestHandler* pThis, NB_ManifestInformation** information)
{
    NB_Error result = NE_OK;

    if (!pThis || !information)
    {
        return NE_INVAL;
    }

    if (!pThis->tpsReply)
    {
        return NE_UNEXPECTED;
    }

    result = NB_ManifestInformationCreate(pThis->context, pThis->tpsReply, information);

    return result;
}


void
ManifestHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* response)
{
    NB_ManifestHandler* pThis = ((struct ManifestHandler_Query*)query)->pThis;
    NB_NetworkRequestStatus status;

    uint32 replyError = NE_OK;

    replyError = GetReplyErrorCode(response);

    status = TranslateNetStatus(response->status);

    pThis->inProgress = 0;
    query->query = NULL;

    if (pThis->callback.callback != NULL)
    {
        // Make reply available for duration of callback
        pThis->tpsReply = response->reply;
        (pThis->callback.callback)(pThis, status, replyError, 0, 100, pThis->callback.callbackData);
        pThis->tpsReply = 0;
    }
}


void
ManifestHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total)
{
    if (!up)
    {
        NB_ManifestHandler* pThis = ((struct ManifestHandler_Query*) query)->pThis;

        if (pThis->callback.callback)
        {
            int percent = (cur * 100) / total;
            (pThis->callback.callback)(pThis, NB_NetworkRequestStatus_Progress, NE_OK, 0, percent, pThis->callback.callbackData);
        }
    }
}


/*! @} */
