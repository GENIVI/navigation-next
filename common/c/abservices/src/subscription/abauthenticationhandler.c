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

    @file     abauthenticationhandler.h
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


#include "abauthenticationhandler.h"
#include "abauthenticationparametersprivate.h"
#include "abauthenticationinformationprivate.h"
#include "nbcontextprotected.h"
#include "nbutilityprotected.h"
#include "cslqarecorddefinitions.h"
#include "abqalog.h"

static void AuthenticationHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp);
static void AuthenticationHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total);

static void AuthenticationHandlerLogReply(AB_AuthenticationHandler* pThis, AB_AuthenticationInformation* information);

struct AuthenticationHandler_Query
{
    NB_NetworkQuery                     networkQuery;
    AB_AuthenticationHandler*           pThis;
};

struct AB_AuthenticationHandler
{
    NB_Context*                         context;
    NB_RequestHandlerCallback           callback;
    nb_boolean                          inProgress;
    struct AuthenticationHandler_Query  query;
    tpselt                              tpsReply;
    AB_AuthenticationParameters*        parameters;
};

/* See header file for description. */ 
AB_DEF NB_Error
AB_AuthenticationHandlerCreate(NB_Context* context, NB_RequestHandlerCallback* callback, AB_AuthenticationHandler** handler)
{
    AB_AuthenticationHandler* pThis = 0;

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
    pThis->query.networkQuery.action = AuthenticationHandler_NetworkCallback;
    pThis->query.networkQuery.progress = AuthenticationHandler_NetworkProgressCallback;
    pThis->query.networkQuery.qflags = TN_QF_DEFLATE;
    pThis->query.networkQuery.failFlags = TN_FF_CLOSE_TIMEOUT;
    nsl_strlcpy(pThis->query.networkQuery.target, NB_ContextGetTargetMapping(pThis->context, "auth"), sizeof(pThis->query.networkQuery.target));
    pThis->query.networkQuery.targetlen = -1;

    *handler = pThis;

    return NE_OK;
}

/* See header file for description. */ 
AB_DEF NB_Error
AB_AuthenticationHandlerDestroy(AB_AuthenticationHandler* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    if (pThis->inProgress)
    {
        AB_AuthenticationHandlerCancelRequest(pThis);
    }

    te_dealloc(pThis->query.networkQuery.query);

    if (pThis->parameters)
    {
        (void)AB_AuthenticationParametersDestroy(pThis->parameters);
    }

    nsl_free(pThis);

    return NE_OK;
}

/* See header file for description. */ 
AB_DEF NB_Error
AB_AuthenticationHandlerStartRequest(AB_AuthenticationHandler* pThis, AB_AuthenticationParameters* parameters)
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

    pThis->query.networkQuery.query = AB_AuthenticationParametersToTPSQuery(parameters);
    if (!pThis->query.networkQuery.query)
    {
        return NE_NOMEM;
    }

    if (pThis->parameters)
    {
        (void)AB_AuthenticationParametersDestroy(pThis->parameters);
    }

    err = AB_AuthenticationParametersClone(parameters, &pThis->parameters);
    if (!err)
    {
        pThis->inProgress = TRUE;

        CSL_NetworkRequest(NB_ContextGetNetwork(pThis->context), &pThis->query.networkQuery);
    }
    

    return err;
}

/* See header file for description. */ 
AB_DEF NB_Error
AB_AuthenticationHandlerCancelRequest(AB_AuthenticationHandler* pThis)
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
AB_AuthenticationHandlerIsRequestInProgress(AB_AuthenticationHandler* pThis)
{
    if (pThis)
    {
        return pThis->inProgress;
    }

    return 0;
}

/* See header file for description. */ 
AB_DEF NB_Error
AB_AuthenticationHandlerGetAuthenticationInformation(AB_AuthenticationHandler* pThis, AB_AuthenticationInformation** information)
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

    err = AB_AuthenticationInformationCreate(pThis->context, pThis->tpsReply, information);

    // iterate through reply only if QA logging enabled
    if (err == NE_OK && NB_ContextGetQaLog(pThis->context) != NULL)
    {
        AuthenticationHandlerLogReply(pThis, *information);
    }

    return err;
}

void
AuthenticationHandler_NetworkCallback(NB_NetworkQuery* query, NB_NetworkResponse* resp)
{
    AB_AuthenticationHandler* pThis = ((struct AuthenticationHandler_Query*)query)->pThis;
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
AuthenticationHandler_NetworkProgressCallback(NB_NetworkQuery* query, boolean up, uint32 cur, uint32 total)
{
    if (!up)
    {
        AB_AuthenticationHandler* pThis = ((struct AuthenticationHandler_Query*) query)->pThis;

        if (pThis->callback.callback)
        {
            int percent = (cur * 100) / total;
            (pThis->callback.callback)(pThis, NB_NetworkRequestStatus_Progress, NE_OK, 0, percent, pThis->callback.callbackData);
        }
    }
}

void
AuthenticationHandlerLogReply(AB_AuthenticationHandler* pThis, AB_AuthenticationInformation* information)
{
    AB_SubscribedMessage subscribedMessage = { 0 };
    AB_ClientStoredMessageDetail* clientMessageDetail = 0;
    int availableBundleCount = 0;
    uint32 availableBundleTimeStamp = 0;

    char pricingOptions[AUTHENTICATION_PRICE_OPTIONS_LENGTH] = { 0 };

    int32 status = 0;

    int bundleIndex = 0;
    int priceOptionIndex = 0;

    if (!pThis || !information)
    {
        return;
    }

    (void)AB_AuthenticationInformationGetStatusCode(information, &status);

    // log bundles currently subscribed to
    if (AB_AuthenticationInformationGetSubscribedMessage(information, &subscribedMessage) == NE_OK)
    {
        for (bundleIndex = 0; bundleIndex < subscribedMessage.bundle_count; bundleIndex++)
        {
            AB_BundleOption bundleOption = { 0 };

            if (AB_AuthenticationInformationGetSubscribedBundle(information, bundleIndex, &bundleOption) == NE_OK)
            {
                pricingOptions [0] = '\0';
                for (priceOptionIndex = 0; priceOptionIndex < bundleOption.price_option_count; priceOptionIndex++)
                {
                    AB_PriceOption priceOption = { 0 };
                    if (AB_AuthenticationInformationGetSubscribedBundlePricing(information, bundleIndex, priceOptionIndex, &priceOption) == NE_OK)
                    {
                        nsl_strlcat(pricingOptions, priceOption.type, AUTHENTICATION_PRICE_OPTIONS_LENGTH);
                        nsl_strlcat(pricingOptions, " ", AUTHENTICATION_PRICE_OPTIONS_LENGTH);
                    }
                }

                AB_QaLogAuthenticationReply(pThis->context, AB_QLBS_Subscribed, subscribedMessage.ts, bundleOption.token_id, bundleOption.name, bundleOption.region, bundleOption.feature_codes, pricingOptions, status);
            }
        }
    }
    if (subscribedMessage.bundle_count == 0)
    {
        AB_QaLogAuthenticationReply(pThis->context, AB_QLBS_Subscribed, subscribedMessage.ts, 0, "", "", "", "", status);
    }

    // log bundles available for purchase
    if (AB_AuthenticationInformationGetClientStoredMessageDetail(information, &clientMessageDetail) == NE_OK)
    {
        availableBundleCount = clientMessageDetail->bundle_count;
        availableBundleTimeStamp = clientMessageDetail->timestamp;

        for (bundleIndex = 0; bundleIndex < availableBundleCount; bundleIndex++)
        {
            AB_BundleOption bundleOption = { 0 };

            if (AB_AuthenticationInformationGetPurchaseBundle(information, bundleIndex, &bundleOption) == NE_OK)
            {
                pricingOptions [0] = '\0';
                for (priceOptionIndex = 0; priceOptionIndex < bundleOption.price_option_count; priceOptionIndex++)
                {
                    AB_PriceOption priceOption = { 0 };
                    if (AB_AuthenticationInformationGetPurchaseBundlePricing(information, bundleIndex, priceOptionIndex, &priceOption) == NE_OK)
                    {
                        nsl_strlcat(pricingOptions, priceOption.type, AUTHENTICATION_PRICE_OPTIONS_LENGTH);
                        nsl_strlcat(pricingOptions, " ", AUTHENTICATION_PRICE_OPTIONS_LENGTH);
                    }
                }

                AB_QaLogAuthenticationReply(pThis->context, AB_QLBS_Available, clientMessageDetail->timestamp, bundleOption.token_id, bundleOption.name, bundleOption.region, bundleOption.feature_codes, pricingOptions, status);
            }
        }
        (void)AB_AuthenticationInformationDestroyClientStoredMessageDetail(clientMessageDetail);
    }
    if (availableBundleCount == 0)
    {
        AB_QaLogAuthenticationReply(pThis->context, AB_QLBS_Available, availableBundleTimeStamp, 0, "", "", "", "", status);
    }
}

/*! @} */
