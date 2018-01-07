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

    @file     ablicenseinformation.c
    @defgroup licenseInfo   LicenseInformation
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

#include "ablicenseinformation.h"
#include "ablicenseparametersprivate.h"
#include "nbcontextprotected.h"
#include "cslnetwork.h"
#include "data_license_reply.h"
#include "ablicenseinformationprivate.h"
/*! @{ */

struct AB_LicenseInformation
{
    NB_Context*                     context;
    data_license_reply              reply;    
};

/* See header file for description. */ 
AB_DEF NB_Error
AB_LicenseInformationCreate(NB_Context* context, tpselt reply, AB_LicenseInformation** information)
{
    AB_LicenseInformation* pThis = 0;
    NB_Error err = NE_OK;
    
    if (!context || !reply || !information)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(*pThis));
    if (!pThis)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    err = data_license_reply_from_tps(NB_ContextGetDataState(context), &pThis->reply, reply);    

    if (!err)
    {
        pThis->context = context;
        *information = pThis;
    }
    else
    {
        nsl_free(pThis);
    }

    return err;
}

/* See header file for description. */ 
AB_DEF NB_Error
AB_LicenseInformationDestroy(AB_LicenseInformation* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    data_license_reply_free(NB_ContextGetDataState(pThis->context), &pThis->reply);

    nsl_free(pThis);        

    return NE_OK;
}

static AB_LicenseStatus
ab_getStatusCode(const char* str)
{
    AB_LicenseStatus sc = AB_LicenseStatus_Success;

    if (nsl_strcmp(str, "-1") == 0) 
    {
        sc = AB_LicenseStatus_DoesNotExist;
    } 
    else if (nsl_strcmp(str, "-2") == 0) 
    {
        sc = AB_LicenseStatus_HasExpired;
    } 
    else if (nsl_strcmp(str, "-3") == 0) 
    {
        sc = AB_LicenseStatus_AuthenticationFailed;
    } 
    else if (nsl_strcmp(str, "-4") == 0) 
    {
        sc = AB_LicenseStatus_XmlParserError;
    } 
    else if (nsl_strcmp(str, "-5") == 0) 
    {
        sc = AB_LicenseStatus_SystemError;
    } 
    else if (nsl_strcmp(str, "-10") == 0) 
    {
        sc = AB_LicenseStatus_InvalidParametners;
    }

    return sc;
}

/* See header file for description. */ 
AB_DEF NB_Error
AB_LicenseInformationGetStatus(
    AB_LicenseInformation* pThis,
    AB_LicenseStatus *statusCode)
{
    NB_Error err = NE_OK;
    data_util_state* pds = NB_ContextGetDataState(pThis->context);
    data_license_reply* reply = &pThis->reply;
    const char *error = data_string_get(pds, &reply->response.license_response.error.code);
    *statusCode = ab_getStatusCode(error);
    return err;
}

/* See header file for description. */ 
AB_DEF NB_Error
AB_LicenseInformationGetRequestId(
    AB_LicenseInformation* pThis,
    const char** requestId)
{
    NB_Error err = NE_OK;
    data_util_state* pds = NB_ContextGetDataState(pThis->context);
    data_license_reply* reply = &pThis->reply;

    if (requestId == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    *requestId = data_string_get(pds, &reply->response.requestid);

errexit:
    return err;
}


/* See header file for description. */ 
AB_DEF NB_Error
AB_LicenseInformationGetTransactionId(
    AB_LicenseInformation* pThis,
    const char** transactionId)
{
    NB_Error err = NE_OK;
    data_util_state* pds = NB_ContextGetDataState(pThis->context);
    data_license_reply* reply = &pThis->reply;

    if (transactionId == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    *transactionId = data_string_get(pds, &reply->response.license_response.transactionid);

errexit:
    return err;
}



/* See header file for description. */ 
AB_DEF NB_Error
AB_LicenseInformationGetLicenseCount(
    AB_LicenseInformation* pThis,
    int *licenseCount)
{
    NB_Error err = NE_OK;
    data_license_reply* reply = &pThis->reply;

    if (licenseCount == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    *licenseCount = CSL_VectorGetLength(reply->response.license_response.vec_license);

errexit:
    return err;
}


/* See header file for description. */ 
AB_DEF NB_Error
AB_LicenseInformationGetLicenseDetail(
    AB_LicenseInformation* pThis,
    int licenseIndex,
    const char** key,
    const char** productId,
    const char** licenseRequestId,
    AB_LicenseStatus *statusCode)
{
    NB_Error err = NE_OK;
    data_util_state* pds = NB_ContextGetDataState(pThis->context);
    data_license_reply* reply = &pThis->reply;
    data_license* license = (data_license*)CSL_VectorGetPointer(reply->response.license_response.vec_license, licenseIndex);

    if (license == NULL) {
        err = NE_BADDATA;
        goto errexit;
    }

    if (key != NULL) {
        *key = data_string_get(pds, &license->key);
    }

    if (productId != NULL) {
        *productId = data_string_get(pds, &license->productid);
    }

    if (licenseRequestId == NULL || statusCode == NULL) {
        err = NE_INVAL;
        goto errexit;
    }

    *licenseRequestId = data_string_get(pds, &license->licrequestid);
    *statusCode = ab_getStatusCode(data_string_get(pds, &license->error.code));

errexit:
    return err;
}

/* See header file for description */
AB_DEF NB_Error 
AB_LicenseInformationGetError(
    AB_LicenseInformation* pThis, 
    const char** error)
{
    data_util_state* pds = NB_ContextGetDataState(pThis->context);
    data_license_reply* reply = &pThis->reply;
    
    if (error)
    {
        *error = data_string_get(pds, &reply->response.license_response.error.description);
    }

    return NE_OK;
}

/* See header file for description */
AB_DEF NB_Error AB_LicenseInformationGetMessageDetail(
    AB_LicenseInformation* pThis, 
    AB_MessageDetail* messageDetail)
{
    NB_Error err = NE_OK;
    data_message* message = NULL;

    if (!pThis || !messageDetail)
    {
        return NE_INVAL;
    }

    nsl_memset(messageDetail, 0, sizeof(*messageDetail));

    if (pThis->reply.response.license_response.hasMessage)
    {
        message = &(pThis->reply.response.license_response.message);

        messageDetail->confirm = message->confirm;
        messageDetail->time = message->time;

        messageDetail->id = nsl_strdup(message->id); 
        messageDetail->title = nsl_strdup(message->title);
        messageDetail->type = nsl_strdup(message->type);
        messageDetail->language = nsl_strdup(message->language);
        messageDetail->accept_text = nsl_strdup(message->accept_text);
        messageDetail->center_text = nsl_strdup(message->center_text);
        messageDetail->decline_text = nsl_strdup(message->decline_text);

        if (message->url.value)     /* prevent passing NULL to nsl_strdup */
        {
            messageDetail->url = nsl_strdup(message->url.value);
        }

        if (messageDetail->id == NULL || messageDetail->title == NULL
            || messageDetail->type == NULL || messageDetail->language == NULL
            || messageDetail->accept_text == NULL || messageDetail->center_text == NULL
            || messageDetail->decline_text == NULL)
        {
            err = NE_NOMEM;
            AB_LicenseInformationFreeMessageDetail(messageDetail);
        }
    }

    return err;
}

/* See header file for description */
AB_DEF NB_Error AB_LicenseInformationFreeMessageDetail(AB_MessageDetail* messageDetail)
{
    if (!messageDetail)
    {
        return NE_INVAL;
    }

    if (messageDetail->id)
    {
        nsl_free(messageDetail->id);
    }

    if (messageDetail->title)
    {
        nsl_free(messageDetail->title);
    }

    if (messageDetail->type)
    {
        nsl_free(messageDetail->type);
    }

    if (messageDetail->language)
    {
        nsl_free(messageDetail->language);
    }

    if (messageDetail->accept_text)
    {
        nsl_free(messageDetail->accept_text);
    }

    if (messageDetail->center_text)
    {
        nsl_free(messageDetail->center_text);
    }

    if (messageDetail->decline_text)
    {
        nsl_free(messageDetail->decline_text);
    }

    if (messageDetail->url)
    {
        nsl_free(messageDetail->url);
    }

    nsl_memset(messageDetail, 0 ,sizeof(*messageDetail));

    return NE_OK;
}

/* See header file for description */
AB_DEF NB_Error AB_LicenseInformationGetMessageText(AB_LicenseInformation* pThis, AB_MessageTextCallback messageTextCallback, void* userData)
{
    NB_Error err = NE_OK;

    data_message* pm = 0;
    data_format_element* pfe = 0;
    NB_Font font = NB_Font_Normal;
    nb_color color  = MAKE_NB_COLOR(0,0,0);

    char empty[] = { ' ', '\0' };
    char zerolength[] = {'\0'};
    int i = 0;
    int len = 0;

    if (!pThis || !messageTextCallback)
    {
        return NE_INVAL;
    }

    pm = &pThis->reply.response.license_response.message;

    len = CSL_VectorGetLength(pm->formatted_text.vec_format_elements);

    for (i=0; i<len; i++)
    {
        pfe = (data_format_element*) CSL_VectorGetPointer(pm->formatted_text.vec_format_elements, i);

        if (pfe != NULL)
        {
            switch (pfe->type)
            {
            case NB_FormatElement_Font:
                err = data_format_element_get_font( pfe, &font );
                break;
            case NB_FormatElement_Color:
                color = pfe->color;
                break;
            case NB_FormatElement_Text:
                if (pfe->text != NULL)
                {  
                    messageTextCallback(font, color, pfe->text, FALSE, NULL, NULL, userData);
                }
                break;
            case NB_FormatElement_NewLine:
                messageTextCallback(font, color, empty, TRUE, NULL, NULL, userData);
                break;
            case NB_FormatElement_Link:
                if (pfe->link.href != NULL && pfe->link.text != NULL)
                {
                    messageTextCallback (font, color, zerolength, FALSE, pfe->link.href, pfe->link.text, userData);
                }
                break;
            default:
                break;
            }
        }
    }

    return err;
}

/* See header file for description */
AB_DEF NB_Error AB_LicenseInformationGetPromoCode(AB_LicenseInformation* pThis, AB_PromotionCode** pPromoCode)
{
    NB_Error err = NE_OK;
    data_license_reply* reply = &pThis->reply;
    data_promo_code* dataPromoCodeSrc = NULL;
    AB_PromotionCode* promotionCode = NULL;

    if(!pThis)
    {
        return NE_INVAL;
    }

    if (pThis->reply.response.license_response.hasPromoCode)
    {
        dataPromoCodeSrc = &(reply->response.license_response.promo_code);
    }
    if (dataPromoCodeSrc == NULL)
    {
        return NE_BADDATA;
    }

    promotionCode = nsl_malloc(sizeof(*promotionCode));

    if (promotionCode == NULL)
    {
        return NE_NOMEM;
    }
    nsl_memset(promotionCode, 0, sizeof(*promotionCode));

    if (dataPromoCodeSrc->description)
    {
        promotionCode->description = nsl_strdup(dataPromoCodeSrc->description);
    }

    if (dataPromoCodeSrc->value)
    {
        promotionCode->value= nsl_strdup(dataPromoCodeSrc->value);
    }

    *pPromoCode = promotionCode;

    return err;
}

AB_DEF NB_Error AB_LicenseInformationDestroyPromoCode(AB_PromotionCode* promoCode)
{
    if (promoCode)
    {
        if (promoCode->value)
        {
            nsl_free(promoCode->value);
        }
        promoCode->value = NULL;

        if (promoCode->description)
        {
            nsl_free(promoCode->description);
        }
        promoCode->description = NULL;

        nsl_free(promoCode);
    }

    return NE_OK;
}

AB_DEF nb_boolean AB_LicenseInformationIsValidationResponse (AB_LicenseInformation* pThis)
{
    if (pThis->reply.response.license_response.licenseValidationResponse)
    {
        return TRUE;
    }
    
    return FALSE;
}

/*! @} */


