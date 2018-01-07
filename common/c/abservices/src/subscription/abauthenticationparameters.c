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

    @file     abauthenticationparameters.c
    @defgroup abparams   Parameters

    This API is used to create Parameters objects.  Parameters objects are used to create and
    configure Handler objects to retrieve information from the server
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


#include "abauthenticationparameters.h"
#include "abauthenticationparametersprivate.h"
#include "nbcontextprotected.h"
#include "data_auth_query.h"
#include "data_auth_reply.h"
#include "abqalog.h"

/*! @{ */

static void AuthenticationParametersLogRequest(AB_AuthenticationParameters* pThis);

struct AB_AuthenticationParameters
{
    NB_Context*           context;
    data_auth_query       query;
};


/* See header file for description. */
AB_DEF NB_Error
AB_AuthenticationParametersCreate(NB_Context* context, AB_AuthenticationParameters** parameters)
{
    AB_AuthenticationParameters* pThis = 0;
    NB_Error err = NE_OK;

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }

    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->context = context;

    if (!err)
    {
        *parameters = pThis;
    }
    else
    {
        nsl_free(pThis);
    }

    return err;
}

/* See header file for description. */
AB_DEF NB_Error
AB_AuthenticationParametersDestroy(AB_AuthenticationParameters* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    data_auth_query_free(NB_ContextGetDataState(pThis->context), &pThis->query);
    nsl_free(pThis);

    return NE_OK;
}

/* See header file for description. */
AB_DEF tpselt
AB_AuthenticationParametersToTPSQuery(AB_AuthenticationParameters* pThis)
{
    if (pThis)
    {
        // process request for logging if QA logging enabled
        if (NB_ContextGetQaLog(pThis->context) != NULL)
        {
            AuthenticationParametersLogRequest(pThis);
        }

        return data_auth_query_to_tps(NB_ContextGetDataState(pThis->context), &pThis->query);
    }

    return 0;
}

/* See header file for description. */
AB_DEF NB_Error AB_AuthenticationParametersClone(AB_AuthenticationParameters* pThis, AB_AuthenticationParameters** clone)
{
    NB_Error err = NE_OK;
    AB_AuthenticationParameters* pClone = 0;

    if (!pThis || !clone)
    {
        return NE_INVAL;
    }
    *clone = 0;

    pClone = nsl_malloc(sizeof(*pClone));
    if (!pClone)
    {
        return NE_NOMEM;
    }
    nsl_memset(pClone, 0, sizeof(*pClone));

    err = data_auth_query_copy(NB_ContextGetDataState(pThis->context), &pClone->query, &pThis->query);
    if (!err)
    {
        pClone->context = pThis->context;
        *clone = pClone;
    }
    else
    {
        nsl_free(pClone);
    }

    return err;
}

/* See header file for description. */
AB_DEF NB_Error AB_AuthenticationParametersSetConfirmAction(AB_AuthenticationParameters* pThis, const char* confirmAction)
{
    NB_Error err = NE_OK;

    if (!pThis)
    {
        return NE_INVAL;
    }
    
    err = data_string_set(NB_ContextGetDataState(pThis->context), &pThis->query.optinResponse.message_confirm.confirm_action, confirmAction);

    return err;
}

/* See header file for description. */ 
AB_DEF NB_Error AB_AuthenticationParametersSetConfirmID(AB_AuthenticationParameters* pThis, const char* confirmId)
{
    NB_Error err = NE_OK;
    if (!pThis)
    {
        return NE_INVAL;
    }

    err = data_string_set(NB_ContextGetDataState(pThis->context), &pThis->query.optinResponse.message_confirm.id, confirmId);

    return err;
}

/* See header file for description. */ 
AB_DEF NB_Error AB_AuthenticationParametersSetAddFeatureName(AB_AuthenticationParameters* pThis, const char* featureName)
{
    NB_Error err = NE_OK;
    if (!pThis)
    {
        return NE_INVAL;
    }

    err = data_string_set(NB_ContextGetDataState(pThis->context), &pThis->query.addFeature.featureName, featureName);

    return err;
}

/* See header file for description. */
AB_DEF NB_Error AB_AuthenticationParametersSetRemoveFeatureName(AB_AuthenticationParameters* pThis, const char* featureName)
{
    NB_Error err = NE_OK;
    if (!pThis)
    {
        return NE_INVAL;
    }

    err = data_string_set(NB_ContextGetDataState(pThis->context), &pThis->query.removeFeature.featureName, featureName);

    return err;
}

/* See header file for description. */
AB_DEF NB_Error AB_AuthenticationParametersSetLicenseKey(AB_AuthenticationParameters* pThis, const char* licenseKey)
{
    NB_Error err = NE_OK;
    if (!pThis)
    {
        return NE_INVAL;
    }

    err = data_string_set(NB_ContextGetDataState(pThis->context), &pThis->query.bindLicense.licenseKey, licenseKey);

    return err;
}

/* See header file for description. */
AB_DEF NB_Error
AB_AuthenticationParametersSetOptin(AB_AuthenticationParameters* pThis, const char* optinType, const char* confirmAction, const char* confirmId)
{
    NB_Error err = NE_OK;
    data_util_state* dataState = NULL;

    dataState = NB_ContextGetDataState(pThis->context);

    err = err ? err : data_string_set(dataState, &pThis->query.optinResponse.message_type, optinType);
    err = err ? err : data_string_set(dataState, &pThis->query.optinResponse.message_confirm.confirm_action, confirmAction);
    err = err ? err : data_string_set(dataState, &pThis->query.optinResponse.message_confirm.id, confirmId);

    return err;
}

/* See header file for description. */
AB_DEF NB_Error AB_AuthenticationParametersSetRequest(AB_AuthenticationParameters* pThis, AB_Authentication_RequestType request)
{
    NB_Error err = NE_OK;
    if (!pThis)
    {
        return NE_INVAL;
    }

    pThis->query.request = (data_auth_request) request;

    return err;
}

/* See header file for description. */
AB_DEF NB_Error
AB_AuthenticationParametersSetWantLicenseMessage(AB_AuthenticationParameters* pThis, nb_boolean licenseMessageFlag, uint32 timestamp)
{
    NB_Error err = NE_OK;

    if (!pThis)
    {
        return NE_INVAL;
    }

    pThis->query.license_msg_valid = licenseMessageFlag;
    pThis->query.want_license_msg.timestamp = timestamp;

    return err;
}

/* See header file for description. */
AB_DEF NB_Error
AB_AuthenticationParametersSetLanguage(AB_AuthenticationParameters* pThis, const char* language)
{
    NB_Error err = NE_OK;
    data_util_state* dataState = NULL;

    if (!pThis)
    {
        return NE_INVAL;
    }

    dataState = NB_ContextGetDataState(pThis->context);

    err = err ? err : data_string_set(dataState, &pThis->query.language, language);

    return err;
}

/* See header file for description. */
AB_DEF NB_Error
AB_AuthenticationParametersSetWantPurchaseMessage(AB_AuthenticationParameters* pThis, nb_boolean purchaseMessageFlag, uint32 timestamp)
{
    NB_Error err = NE_OK;

    if (!pThis)
    {
        return NE_INVAL;
    }

    pThis->query.purchase_msg = purchaseMessageFlag;
    pThis->query.purchase_msg_ts = timestamp;

    return err;
}

/* See header file for description. */
AB_DEF NB_Error
AB_AuthenticationParametersSetWantMarketingMessage(AB_AuthenticationParameters* pThis, nb_boolean wantMarkettingMessageFlag)
{
    NB_Error err = NE_OK;

    if (!pThis)
    {
        return NE_INVAL;
    }

    pThis->query.want_marketing_msg = wantMarkettingMessageFlag;

    return err;
}

AB_DEF NB_Error
AB_AuthenticationParametersSetWantPinMessage(AB_AuthenticationParameters* pThis, nb_boolean wantPinMessageFlag)
{
    NB_Error err = NE_OK;

    if (!pThis)
    {
        return NE_INVAL;
    }

    pThis->query.want_pin_msg = wantPinMessageFlag;

    return err;
}

/* See header file for description. */
AB_DEF NB_Error
AB_AuthenticationParametersSetWantExtendedMarketingMessage(AB_AuthenticationParameters* pThis, nb_boolean wantExtendedMarkettingMessageFlag)
{
    NB_Error err = NE_OK;

    if (!pThis)
    {
        return NE_INVAL;
    }

    pThis->query.want_extended_marketing_msg = wantExtendedMarkettingMessageFlag;

    return err;
}


/* See header file for description. */
AB_DEF NB_Error
AB_AuthenticationParametersSetWantRegion(AB_AuthenticationParameters* pThis, nb_boolean region_info)
{
    NB_Error err = NE_OK;

    if (!pThis)
    {
        return NE_INVAL;
    }

    pThis->query.region_info = region_info;

    return err;
}

/* See header file for description */
AB_DEF NB_Error
AB_AuthenticationParametersSetTransactionId(AB_AuthenticationParameters* pThis, const char* transactionId)
{
    NB_Error err = NE_OK;
    data_util_state* dataState = NULL;

    if (!pThis)
        return NE_INVAL;

    dataState = NB_ContextGetDataState(pThis->context);

    err = err ? err : data_string_set(dataState, &pThis->query.transactionid, transactionId);

    return err;
}

/*See header file for description */

AB_DEF NB_Error 
AB_AuthenticationParametersSetWantPromoOptions(AB_AuthenticationParameters* pThis, nb_boolean wantPromoOptions)
{
    NB_Error result = NE_OK;
    
    if (!pThis)
    {
        return NE_INVAL;
    }
    
    pThis->query.want_promo_options = wantPromoOptions;
    
    return result;
}

void
AuthenticationParametersLogRequest(AB_AuthenticationParameters* pThis)
{
    data_util_state* dataState = NULL;
    const char* requestParameter = 0;
    uint32 timeStamp = 0;
    const char* language = 0;

    if (!pThis)
    {
        return;
    }

    dataState = NB_ContextGetDataState(pThis->context);

    switch (pThis->query.request)
    {
        case dar_bind_license:
            requestParameter = data_string_get(dataState, &pThis->query.bindLicense.licenseKey);
            break;
        case dar_list_feature:
            if (pThis->query.purchase_msg)
            {
                requestParameter = "WantPurchase";
            }
            else if (pThis->query.license_msg_valid)
            {
                requestParameter = "WantLicense";
            }
            break;
        default:
            requestParameter = "";
            break;
    }

    if (pThis->query.purchase_msg)
    {
        timeStamp = pThis->query.purchase_msg_ts;
    }
    else if (pThis->query.license_msg_valid)
    {
        timeStamp = pThis->query.want_license_msg.timestamp;
    }

    language = data_string_get(dataState, &pThis->query.language);

    AB_QaLogAuthenticationRequest(pThis->context, pThis->query.request, requestParameter, timeStamp, language);
}

/* See header file for description. */
AB_DEF NB_Error
AB_AuthenticationParametersSetWantProductDescription(AB_AuthenticationParameters* pThis, nb_boolean wantProductDescriptionFlag)
{
    NB_Error err = NE_OK;

    if (!pThis)
    {
        return NE_INVAL;
    }

    pThis->query.want_product_description = wantProductDescriptionFlag;

    return err;
}
/*! @} */
