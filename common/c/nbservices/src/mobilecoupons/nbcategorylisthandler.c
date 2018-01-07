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

    @file     nbcategorylisthandler.c
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems, Inc. is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#include "nbcategorylisthandler.h"
#include "nbcategorylistparametersprivate.h"
#include "nbcategorylistinformationprivate.h"
#include "nbmobilecouponsprivate.h"
#include "nbcontextprotected.h"
#include "cslnetwork.h"
#include "nbutilityprotected.h"

typedef struct CategoryListHandler_Query
{
    NB_NetworkQuery                  networkQuery;
    NB_CategoryListHandler*          pThis;
} CategoryListHandler_Query;


struct NB_CategoryListHandler
{
    NB_Context*                        context;
    NB_RequestHandlerCallback          callback;
    boolean                            inProgress;
    CategoryListHandler_Query          query;
    tpselt                             tpsReply;
};

static void CategoryListHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* response);
static void CategoryListHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 have, uint32 total);

NB_DEF NB_Error
NB_CategoryListHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback,
                                     NB_CategoryListHandler** handler)
{
    NB_CategoryListHandler* pThis = NULL;

    if (context == NULL || handler == NULL)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(NB_CategoryListHandler));
    if (pThis == NULL)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(NB_CategoryListHandler));

    pThis->context = context;
    if (callback != NULL)
    {
        pThis->callback.callback = callback->callback;
        pThis->callback.callbackData = callback->callbackData;
    }

    pThis->query.pThis = pThis;
    pThis->query.networkQuery.action = CategoryListHandler_NetworkCallback;
    pThis->query.networkQuery.progress = CategoryListHandler_NetworkProgressCallback;
    pThis->query.networkQuery.qflags = TN_QF_DEFLATE;
    pThis->query.networkQuery.failFlags = TN_FF_CLOSE_ANY;
    nsl_strlcpy(pThis->query.networkQuery.target, NB_ContextGetTargetMapping(pThis->context, COUPONS_SERVLET), sizeof(pThis->query.networkQuery.target));
    pThis->query.networkQuery.targetlen = -1;

    *handler = pThis;

    return NE_OK;
}

NB_DEF NB_Error
NB_CategoryListHandlerDestroy(NB_CategoryListHandler* handler)
{
    if (handler == NULL)
    {
        return NE_INVAL;
    }

    if (handler->inProgress)
    {
        NB_CategoryListHandlerCancelRequest(handler);
    }

    te_dealloc(handler->query.networkQuery.query);
    nsl_free(handler);
    return NE_OK;
}

NB_DEF NB_Error
NB_CategoryListHandlerStartRequest(NB_CategoryListHandler* handler,
                                           NB_CategoryListParameters* parameters)
{
    NB_Error err = NE_OK;
    if (handler == NULL || parameters == NULL)
    {
        return NE_INVAL;
    }
    if (handler->inProgress)
    {
        return NE_BUSY;
    }

    te_dealloc(handler->query.networkQuery.query);

    handler->query.networkQuery.query = NB_CategoryListParametersToTPSQuery(parameters);
    if (handler->query.networkQuery.query == NULL)
    {
        return NE_NOMEM;
    }

    handler->inProgress = TRUE;
    CSL_NetworkRequest(NB_ContextGetNetwork(handler->context), &handler->query.networkQuery);

    return err;
}

NB_DEF NB_Error
NB_CategoryListHandlerCancelRequest(NB_CategoryListHandler* handler)
{
    if (handler == NULL)
    {
        return NE_INVAL;
    }
    if (!handler->inProgress)
    {
        return NE_UNEXPECTED;
    }

    CSL_NetworkCancel(NB_ContextGetNetwork(handler->context), &handler->query.networkQuery);

    return NE_OK;
}

NB_DEF nb_boolean
NB_CategoryListHandlerIsRequestInProgress(NB_CategoryListHandler* handler)
{
    if (handler != NULL)
    {
        return handler->inProgress;
    }
    return FALSE;
}

NB_DEF NB_Error
NB_CategoryListHandlerGetInformation(NB_CategoryListHandler* handler,
                                             NB_CategoryListInformation** information)
{
    if (handler == NULL || information == NULL)
    {
        return NE_INVAL;
    }
    if (handler->tpsReply == NULL)
    {
        return NE_UNEXPECTED;
    }

    return NB_CategoryListInformationCreate(handler->context, handler->tpsReply, information);
}

void
CategoryListHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* response)
{
    NB_CategoryListHandler* pThis = NULL;
    NB_NetworkRequestStatus status = NB_NetworkRequestStatus_Failed;
    NB_Error replyError = NE_OK;

    if (query == NULL ||
        (pThis = ((CategoryListHandler_Query*)query)->pThis) == NULL)
    {
        return;
    }
    if (response != NULL)
    {
        replyError = GetErrorCode(pThis->context, response);
        status = TranslateNetStatus(response->status);
    }

    pThis->inProgress = FALSE;
    query->query = NULL;

    if (pThis->callback.callback != NULL)
    {
        // Make reply available for duration of callback
        pThis->tpsReply = (response != NULL) ? response->reply : NULL;
        (pThis->callback.callback)(pThis, status, replyError, FALSE, 100, pThis->callback.callbackData);
        pThis->tpsReply = NULL;
    }
}

void
CategoryListHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 have, uint32 total)
{
    NB_CategoryListHandler* pThis = (query != NULL) ? ((CategoryListHandler_Query*)query)->pThis : NULL;

    if (pThis != NULL && pThis->callback.callback != NULL)
    {
        int percent = (have * 100) / total;
        (pThis->callback.callback)(pThis, NB_NetworkRequestStatus_Progress, NE_OK, up, percent, pThis->callback.callbackData);
    }
}

/*! @} */
