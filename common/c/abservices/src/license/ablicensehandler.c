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

    @file     ablicensehandler.h
    @defgroup abhandler   Handler
*/
/*
    See file description in header file.

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


#include "ablicensehandler.h"
#include "ablicenseparametersprivate.h"
#include "ablicenseinformationprivate.h"
#include "nbcontextprotected.h"
#include "nbutilityprotected.h"
#include "abqalog.h"

static void LicenseHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp);
static void LicenseHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total);

static void LicenseHandlerLogReply(AB_LicenseHandler* pThis, AB_LicenseInformation* information);

struct LicenseHandler_Query
{
    NB_NetworkQuery              networkQuery;
    AB_LicenseHandler*           pThis;
};

struct AB_LicenseHandler
{
    NB_Context*                         context;
    NB_RequestHandlerCallback           callback;
    nb_boolean                          inProgress;
    struct LicenseHandler_Query  query;
    tpselt                              tpsReply;
    AB_LicenseParameters*        parameters;
};

/* See header file for description. */ 
AB_DEF NB_Error
AB_LicenseHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, AB_LicenseHandler** handler)
{
    AB_LicenseHandler* pThis = 0;

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
    pThis->query.networkQuery.action = LicenseHandler_NetworkCallback;
    pThis->query.networkQuery.progress = LicenseHandler_NetworkProgressCallback;
    pThis->query.networkQuery.qflags = TN_QF_DEFLATE;
    pThis->query.networkQuery.failFlags = TN_FF_CLOSE_ANY;
    nsl_strlcpy(pThis->query.networkQuery.target, NB_ContextGetTargetMapping(pThis->context, "license"), sizeof(pThis->query.networkQuery.target));
    pThis->query.networkQuery.targetlen = -1;

    *handler = pThis;

    return NE_OK;
}

/* See header file for description. */ 
AB_DEF NB_Error
AB_LicenseHandlerDestroy(AB_LicenseHandler* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    if (pThis->inProgress)
    {
        AB_LicenseHandlerCancelRequest(pThis);
    }

    te_dealloc(pThis->query.networkQuery.query);

    if (pThis->parameters)
    {
        (void)AB_LicenseParametersDestroy(pThis->parameters);
    }

    nsl_free(pThis);

    return NE_OK;
}

/* See header file for description. */ 
AB_DEF NB_Error
AB_LicenseHandlerStartRequest(AB_LicenseHandler* pThis, AB_LicenseParameters* parameters)
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

    pThis->query.networkQuery.query = AB_LicenseParametersToTPSQuery(parameters);
    if (!pThis->query.networkQuery.query)
    {
        return NE_NOMEM;
    }

    if (pThis->parameters)
    {
        (void)AB_LicenseParametersDestroy(pThis->parameters);
    }

    err = AB_LicenseParametersClone(parameters, &pThis->parameters);
    if (!err)
    {
        pThis->inProgress = TRUE;

        CSL_NetworkRequest(NB_ContextGetNetwork(pThis->context), &pThis->query.networkQuery);
    }
    

    return err;
}

/* See header file for description. */ 
AB_DEF NB_Error
AB_LicenseHandlerCancelRequest(AB_LicenseHandler* pThis)
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

/* See header file for description. */ 
AB_DEF nb_boolean
AB_LicenseHandlerIsRequestInProgress(AB_LicenseHandler* pThis)
{
    if (pThis)
    {
        return pThis->inProgress;
    }

    return 0;
}

/* See header file for description. */ 
AB_DEF NB_Error
AB_LicenseHandlerGetLicenseInformation(AB_LicenseHandler* pThis, AB_LicenseInformation** information)
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

    err = AB_LicenseInformationCreate(pThis->context, pThis->tpsReply, information);

    // iterate through reply only if QA logging enabled
    if (err == NE_OK && NB_ContextGetQaLog(pThis->context) != NULL)
    {
        LicenseHandlerLogReply(pThis, *information);
    }

    return err;
}

void
LicenseHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp)
{
    AB_LicenseHandler* pThis = ((struct LicenseHandler_Query*)query)->pThis;
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
LicenseHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total)
{
    if (!up)
    {
        AB_LicenseHandler* pThis = ((struct LicenseHandler_Query*) query)->pThis;

        if (pThis->callback.callback)
        {
            int percent = (cur * 100) / total;
            (pThis->callback.callback)(pThis, NB_NetworkRequestStatus_Progress, NE_OK, 0, percent, pThis->callback.callbackData);
        }
    }
}

void
LicenseHandlerLogReply(AB_LicenseHandler* pThis, AB_LicenseInformation* information)
{
    const char *requestId = 0;
    int licenseCount = 0;
    int i = 0;

    if (!pThis || !information)
    {
        return;
    }

    (void)AB_LicenseInformationGetRequestId(information, &requestId);

    if (AB_LicenseInformationGetLicenseCount(information, &licenseCount) == NE_OK)
    {
        for (i = 0; i < licenseCount; i++)
        {
            const char* key = 0;
            const char* productId = 0;
            const char* licenseRequestId = 0;
            AB_LicenseStatus detailStatusCode = AB_LicenseStatus_Success;

            if (AB_LicenseInformationGetLicenseDetail(information, i, &key, &productId, &licenseRequestId, &detailStatusCode) == NE_OK)
            {
                AB_QaLogLicenseReply(pThis->context, key, productId, requestId, detailStatusCode);
            }
        }
    }
}

/*! @} */
