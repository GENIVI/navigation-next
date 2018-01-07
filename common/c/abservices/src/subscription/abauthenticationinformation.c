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

    @file     abauthenticationinformation.c
    @defgroup authenticationInfo   AuthenticationInformation
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

#include "abauthenticationinformation.h"
#include "abauthenticationparametersprivate.h"
#include "nbcontextprotected.h"
#include "cslnetwork.h"
#include "data_auth_reply.h"
#include "abauthenticationinformationprivate.h"
/*! @{ */

static void FreeMessageDetail(AB_MessageDetail* message);
static NB_Error ConvertDataMessageToMessageDetail(data_message* pdm, AB_MessageDetail* pMsg);
static void SetBundleToABBundle(data_util_state* pds, data_bundle_option* bundle, AB_BundleOption* bundleOption);
static void SetPriceToABPrice(data_util_state* pds, data_price_option* price, AB_PriceOption* priceOption);

struct AB_AuthenticationInformation
{
    NB_Context*                     context;
    data_auth_reply                 reply;
    char                            empty[2];
    char                            zero_length[1];
};

/* See header file for description. */ 
AB_DEF NB_Error
AB_AuthenticationInformationCreate(NB_Context* context, tpselt reply, AB_AuthenticationInformation** information)
{
    AB_AuthenticationInformation* pThis = 0;
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

    err = data_auth_reply_from_tps(NB_ContextGetDataState(context), &pThis->reply, reply);
    pThis->empty[0] = ' ';

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
AB_AuthenticationInformationDestroy(AB_AuthenticationInformation* pThis)
{
    if (!pThis)
    {
        return NE_INVAL;
    }

    data_auth_reply_free(NB_ContextGetDataState(pThis->context), &pThis->reply);

    nsl_free(pThis);

    return NE_OK;
}

/* See header file for description. */
AB_DEF NB_Error
AB_AuthenticationInformationFeatureCount(AB_AuthenticationInformation* pThis, int* featureCount)
{
    if (!pThis || !featureCount)
    {
        return NE_INVAL;
    }

    *featureCount = data_auth_reply_num_feature(NB_ContextGetDataState(pThis->context), &pThis->reply);

    return NE_OK;
}

/* See header file for description. */
AB_DEF NB_Error
AB_AuthenticationInformationGetStatusCode(AB_AuthenticationInformation* information, int32* statusCode)
{
    if(!information)
    {
        return NE_INVAL;
    }

    *statusCode = information->reply.status_code;

    return NE_OK;
}

/* See header file for description. */
AB_DEF NB_Error AB_AuthenticationInformationGetFeatureResult(AB_AuthenticationInformation* pThis, int index, AB_FeatureDetail** featureDetail)
{
    NB_Error err = NE_OK;
    data_feature* dataFeatureSrc = NULL;
    AB_FeatureDetail* feature = NULL;

    if(!pThis)
    {
        return NE_INVAL;
    }

    if (pThis->reply.vec_feature)
    {
        int l = 0;
        l = CSL_VectorGetLength(pThis->reply.vec_feature);

        if (index > l)
        {
            return NE_RANGE;
        }
        dataFeatureSrc = (data_feature*)CSL_VectorGetPointer(pThis->reply.vec_feature, index);            
    }

    if (NULL == dataFeatureSrc)
    {
        return NE_BADDATA;
    }

    feature = nsl_malloc(sizeof(*feature));

    if(feature == NULL)
    {
        return NE_NOMEM;
    }
    nsl_memset(feature, 0, sizeof(*feature));

    feature->endDate = dataFeatureSrc->endDate;
    feature->name = nsl_strdup(dataFeatureSrc->name); 
    feature->type = nsl_strdup(dataFeatureSrc->type); 
    feature->region = nsl_strdup(dataFeatureSrc->region);

    *featureDetail = feature;
   
    return err;
}

/* See header file for description. */
AB_DEF void
AB_AuthenticationInformationGetFeatureResultDestroy(AB_FeatureDetail* featureDetail)
{
    if(featureDetail == NULL)
    {
        return;
    }

    if(featureDetail->name)
    {
        nsl_free(featureDetail->name);
    }

    if(featureDetail->type)
    {
        nsl_free(featureDetail->type);
    }

    if (featureDetail->region)
    {
        nsl_free(featureDetail->region);
    }

    nsl_free(featureDetail);
}

/* See header file for description. */
AB_DEF NB_Error
AB_AuthenticationInformationGetClientStoredMessageDetail(AB_AuthenticationInformation* pThis, AB_ClientStoredMessageDetail** pMsg)
{
    AB_ClientStoredMessageDetail* clientMessage = NULL;

    if (!pThis)
    {
        return NE_INVAL;
    }

    if (!pThis->reply.client_stored_message_valid)
    {
        return NE_NOENT;
    }

    clientMessage = nsl_malloc(sizeof(AB_ClientStoredMessageDetail));

    if (clientMessage == NULL)
    {
        return NE_NOMEM;
    }

    nsl_memset(clientMessage, 0, sizeof(*clientMessage));

    if (pThis->reply.client_stored_message.language)
    {
        clientMessage->language = nsl_strdup(pThis->reply.client_stored_message.language);
    }

    if (pThis->reply.client_stored_message.type)
    {
        clientMessage->type = nsl_strdup(pThis->reply.client_stored_message.type);
    }

    clientMessage->timestamp = pThis->reply.client_stored_message.timestamp;

    if (pThis->reply.client_stored_message.accept_text)
    {
        clientMessage->accept_text = nsl_strdup(pThis->reply.client_stored_message.accept_text);
    }

    if (pThis->reply.client_stored_message.decline_text)
    {
        clientMessage->decline_text = nsl_strdup(pThis->reply.client_stored_message.decline_text);
    }

    if (pThis->reply.client_stored_message.option_text)
    {
        clientMessage->option_text = nsl_strdup(pThis->reply.client_stored_message.option_text);
    }

    if (pThis->reply.client_stored_message.purchase_option.vec_bundle_option)
    {
        clientMessage->bundle_count = CSL_VectorGetLength(pThis->reply.client_stored_message.purchase_option.vec_bundle_option);
    }
    
    *pMsg = clientMessage;

    return NE_OK;
}

/* See header file for description. */
AB_DEF void
AB_AuthenticationInformationDestroyClientStoredMessageDetail(AB_ClientStoredMessageDetail* clientMessage)
{
    if (!clientMessage)
    {
        return;
    }

    if (clientMessage->language)
    {
        nsl_free(clientMessage->language);
    }
    clientMessage->language = NULL;

    if (clientMessage->type)
    {
        nsl_free(clientMessage->type);
    }
    clientMessage->type = NULL;
    
    if (clientMessage->accept_text)
    {
        nsl_free(clientMessage->accept_text);
    }
    clientMessage->accept_text = NULL;

    if (clientMessage->decline_text)
    {
        nsl_free(clientMessage->decline_text);
    }
    clientMessage->decline_text = NULL;

    if (clientMessage->option_text)
    {
        nsl_free(clientMessage->option_text);
    }
    clientMessage->option_text = NULL;

    nsl_free(clientMessage);

    clientMessage = NULL;
}

/* See header file for description. */ 
AB_DEF NB_Error
AB_AuthenticationInformationGetMessageCount(AB_AuthenticationInformation* pThis, int* messageCount)
{
    if (!pThis || !messageCount)
    {
        return NE_INVAL;
    }

    *messageCount = data_auth_reply_num_message(NB_ContextGetDataState(pThis->context), &pThis->reply);

    return NE_OK;
}

AB_DEF NB_Error
AB_AuthenticationInformationGetMessageDetail (AB_AuthenticationInformation* pThis, int index, AB_MessageDetail** pMsg)
{
    NB_Error err = NE_OK;
    data_message* message = NULL;
    AB_MessageDetail* messageDetail = NULL;
    int l;

    if(!pThis)
    {
        return NE_INVAL;
    }

    if (pThis->reply.vec_message)
    {        
        l = CSL_VectorGetLength(pThis->reply.vec_message);

        if(index > l)
        {
            return NE_RANGE;
        }
        message = (data_message*)CSL_VectorGetPointer(pThis->reply.vec_message, index);            
        if(NULL == message)
        {
            return NE_BADDATA;
        }
    }

    if (message != NULL)
    {
        messageDetail = nsl_malloc(sizeof(*messageDetail));

        if(messageDetail == NULL)
        {
            return NE_NOMEM;
        }
        nsl_memset(messageDetail, 0, sizeof(*messageDetail));
    
        err = ConvertDataMessageToMessageDetail (message, messageDetail);
    }

    *pMsg = messageDetail;
   
    return err;
}

AB_DEF NB_Error
AB_AuthenticationInformationGetMessageText(AB_AuthenticationInformation* pThis, int index, AB_MessageTextCallback messageTextCallback, void* userData)
{
    NB_Error err = NE_OK;

    data_message* dm = NULL;
    data_formatted_text fmtText = {0};

    if (!pThis || !messageTextCallback)
    {
        return NE_INVAL;
    }

    if (pThis->reply.vec_message)
    {
        int l = 0;
        l = CSL_VectorGetLength(pThis->reply.vec_message);

        if(index > l)
        {
            return NE_RANGE;
        }

        dm = (data_message*)CSL_VectorGetPointer(pThis->reply.vec_message, index);            
        if(NULL == dm)
        {
            return NE_BADDATA;
        }

        fmtText = dm->formatted_text;

        err = AB_AuthenticationInformationGetFormattedText(pThis, &fmtText, messageTextCallback, userData);
    }

    return err;
}

AB_DEF NB_Error
AB_AuthenticationInformationGetClientStoredMessageText(AB_AuthenticationInformation* pThis, AB_MessageTextCallback messageTextCallback, void* userData)
{
    data_formatted_text fmtText = {0};

    if (!pThis || !messageTextCallback)
    {
        return NE_INVAL;
    }

    fmtText = pThis->reply.client_stored_message.formatted_text;

    return AB_AuthenticationInformationGetFormattedText (pThis, &fmtText, messageTextCallback, userData);    
}

NB_Error
AB_AuthenticationInformationGetFormattedText(AB_AuthenticationInformation* pThis, data_formatted_text* fmtText, AB_MessageTextCallback messageTextCallback, void* userData)
{
    NB_Error err = NE_OK;

    data_format_element* pfe = 0;
    NB_Font font = NB_Font_Normal;
    nb_color color  = MAKE_NB_COLOR(0,0,0);

    int i = 0;
    int len = 0;    

    if (!pThis || !messageTextCallback || !fmtText)
    {
        return NE_INVAL;
    }
    
    len = CSL_VectorGetLength(fmtText->vec_format_elements);

    for (i=0; i<len; i++)
    {
        pfe = (data_format_element*) CSL_VectorGetPointer(fmtText->vec_format_elements, i);

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
                    messageTextCallback(font, color, pThis->empty, TRUE, NULL, NULL, userData);
                    break;
                case NB_FormatElement_Link:
                    if (pfe->link.href != NULL && pfe->link.text != NULL)
                    {
                        // @todo (BUG 56476)- Test formatted text data to see how we are appending the URL, we may need to keep 
                        // zero length string or change to empty string
                        messageTextCallback (font, color, pThis->zero_length, FALSE, pfe->link.href, pfe->link.text, userData);
                    }
                    break;                    
                default:
                    break;
            }
        }
    }

    return err;

}

AB_DEF void 
AB_AuthenticationInformationDestroyMessageDetail(AB_MessageDetail* messageDetail)
{
    if (messageDetail)
    {
        FreeMessageDetail(messageDetail);
        nsl_free(messageDetail);
    }
}

NB_Error
ConvertDataMessageToMessageDetail(data_message* pdm, AB_MessageDetail* pMsg)
{
    NB_Error err = NE_OK;

    nsl_memset(pMsg, 0, sizeof(*pMsg));

    pMsg->id = nsl_strdup(pdm->id); 
    pMsg->title = nsl_strdup(pdm->title);
    pMsg->type = nsl_strdup(pdm->type);
    pMsg->language = nsl_strdup(pdm->language);
    pMsg->accept_text = nsl_strdup(pdm->accept_text);
    pMsg->center_text = nsl_strdup(pdm->center_text);
    pMsg->decline_text = nsl_strdup(pdm->decline_text);

    if (pdm->url.value)     /* prevent passing NULL to nsl_strdup */
    {
        pMsg->url = nsl_strdup(pdm->url.value);
    }

    if (pMsg->id == NULL || pMsg->title == NULL ||pMsg->type == NULL || pMsg->language == NULL ||
            pMsg->accept_text == NULL || pMsg->center_text == NULL || pMsg->decline_text == NULL)
    {
        err = NE_NOMEM;
        goto errexit;
    }

    pMsg->confirm = pdm->confirm;    
    pMsg->time = pdm->time;

errexit:
    if (err != NE_OK)
    {
        FreeMessageDetail(pMsg);
        nsl_memset(pMsg, 0, sizeof(*pMsg));
    }

    return err;
}

void
FreeMessageDetail(AB_MessageDetail* message)
{
    if (message->id)
    {
        nsl_free(message->id);
    }
    message->id = NULL;

    if (message->title)
    {
        nsl_free(message->title);
    }
    message->title = NULL;

    if (message->type)
    {
        nsl_free(message->type);
    }
    message->type = NULL;

    if (message->language)
    {
        nsl_free(message->language);
    }
    message->language = NULL;

    if (message->accept_text)
    {
        nsl_free(message->accept_text);
    }
    message->accept_text = NULL;

    if (message->center_text)
    {
        nsl_free(message->center_text);
    }
    message->center_text = NULL;

    if (message->decline_text)
    {
        nsl_free(message->decline_text);
    }
    message->decline_text = NULL;

    if (message->url)
    {
        nsl_free(message->url);
    }
    message->url = NULL;
}

AB_DEF NB_Error
AB_AuthenticationInformationGetPurchaseBundle(AB_AuthenticationInformation* pThis, int index, AB_BundleOption* bundleOption)
{
    NB_Error err = NE_OK;
    int len =0;
    data_bundle_option* bundle = NULL;
    data_util_state* pds = NULL;

    if (!pThis || index < 0)
    {
        return NE_INVAL;
    }

    len = CSL_VectorGetLength(pThis->reply.client_stored_message.purchase_option.vec_bundle_option);

    if (index >= len)
    {
        return NE_INVAL;
    }

    bundle = (data_bundle_option*)CSL_VectorGetPointer(pThis->reply.client_stored_message.purchase_option.vec_bundle_option, index);

    pds = NB_ContextGetDataState(pThis->context);

    SetBundleToABBundle(pds, bundle, bundleOption);

    return err;
}

void
SetBundleToABBundle(data_util_state* pds, data_bundle_option* bundle, AB_BundleOption* bundleOption)
{
    bundleOption->description = data_string_get(pds, &bundle->description);
    bundleOption->name = data_string_get(pds, &bundle->name);
    bundleOption->title = data_string_get(pds, &bundle->title);
    bundleOption->feature_codes = data_string_get(pds, &bundle->feature_list.feature_code);
    bundleOption->region = data_string_get(pds, &bundle->region);
    bundleOption->token_id = bundle->token_id;

    bundleOption->price_option_count = CSL_VectorGetLength(bundle->price_list.vec_price_option);
}

AB_DEF NB_Error
AB_AuthenticationInformationGetPurchaseBundlePricing(AB_AuthenticationInformation* pThis, int bundleIndex, int index, AB_PriceOption* priceOption)
{
    int len = 0;
    data_price_option* price = NULL;
    data_bundle_option* bundle = NULL;
    data_util_state* pds = NULL;

    if (!pThis || index <0)
    {
        return NE_INVAL;
    }

    bundle = (data_bundle_option*)CSL_VectorGetPointer(pThis->reply.client_stored_message.purchase_option.vec_bundle_option, bundleIndex);

    if (!bundle)
    {
        return NE_INVAL;
    }

    len = CSL_VectorGetLength(bundle->price_list.vec_price_option);

    if (index >= len)
    {
        return NE_INVAL;
    }

    pds = NB_ContextGetDataState(pThis->context);

    price = (data_price_option*)CSL_VectorGetPointer(bundle->price_list.vec_price_option, index);

    SetPriceToABPrice(pds, price, priceOption);

    return NE_OK;
}

void
SetPriceToABPrice(data_util_state* pds, data_price_option* price, AB_PriceOption* priceOption)
{
    priceOption->data = data_string_get(pds, &price->data);
    priceOption->type = data_string_get(pds, &price->type);

    priceOption->enddate = price->endDate;
    priceOption->is_recommended_option = price->is_recommended_option;
    priceOption->has_promocode_offering = price->hasPromoCodeOffering;
}

AB_DEF NB_Error
AB_AuthenticationInformationGetSubscribedMessage(AB_AuthenticationInformation* pThis, AB_SubscribedMessage* subscribedMessage)
{
    data_util_state* pds = NULL;

    if (!pThis || !subscribedMessage)
    {
        return NE_INVAL;
    }

    pds = NB_ContextGetDataState(pThis->context);

    if (pThis->reply.subscribed_message_valid)
    {
        subscribedMessage->language = data_string_get(pds, &pThis->reply.subscribed_message.language);
        subscribedMessage->ts = pThis->reply.subscribed_message.timestamp;
        subscribedMessage->maptileAccessToken = data_string_get(pds, &pThis->reply.subscribed_message.maptile_access_token.token);

        subscribedMessage->bundle_count = CSL_VectorGetLength(pThis->reply.subscribed_message.purchase_option.vec_bundle_option);
    }

    return NE_OK;
}

AB_DEF NB_Error
AB_AuthenticationInformationGetPinMessage(AB_AuthenticationInformation* pThis, AB_PinMessage* pinMessage)
{
    data_util_state* pds = NULL;
    if(!pThis || !pinMessage)
    {
        return NE_INVAL;
    }
    
    pds = NB_ContextGetDataState(pThis->context);
    
    if(pThis->reply.pin_message_valid)
    {
        pinMessage->pinMessage = data_string_get(pds, &pThis->reply.pin_message.pin);
    }
    
    return NE_OK;
    
}

AB_DEF NB_Error
AB_AuthenticationInformationGetPinMessageText(AB_AuthenticationInformation* pThis, AB_MessageTextCallback messageTextCallback, void* userData)
{
    data_util_state* dataState = 0;
    data_formatted_text fmtText = {0};
    
    if (!pThis || !messageTextCallback)
    {
        return NE_INVAL;
    }
    
    dataState = NB_ContextGetDataState(pThis->context);
    
    fmtText = pThis->reply.pin_message.formatted_text;
    
    return AB_AuthenticationInformationGetFormattedText (pThis, &fmtText, messageTextCallback, userData);
}

AB_DEF NB_Error
AB_AuthenticationInformationGetSubscribedBundle(AB_AuthenticationInformation* pThis, int index, AB_BundleOption* bundleOption)
{
    data_util_state* pds = NULL;
    data_bundle_option* bundle = NULL;
    int len =0;

    if (!pThis || index < 0 || !pThis->reply.subscribed_message_valid)
    {
        return NE_INVAL;
    }

    pds = NB_ContextGetDataState(pThis->context);

    len = CSL_VectorGetLength(pThis->reply.subscribed_message.purchase_option.vec_bundle_option);

    if (index >= len)
    {
        return NE_INVAL;
    }
    
    bundle = (data_bundle_option*)CSL_VectorGetPointer(pThis->reply.subscribed_message.purchase_option.vec_bundle_option, index);
    
    SetBundleToABBundle(pds, bundle, bundleOption);

    return NE_OK;
}

AB_DEF NB_Error
AB_AuthenticationInformationGetSubscribedBundlePricing(AB_AuthenticationInformation* pThis, int bundleIndex, int index, AB_PriceOption* priceOption)
{
    data_util_state* pds = NULL;
    data_bundle_option* bundle = NULL;
    data_price_option* price = NULL;
    int len = 0;

    if (!pThis || bundleIndex < 0 || index <0)
    {
        return NE_INVAL;
    }

    pds = NB_ContextGetDataState(pThis->context);

    bundle = (data_bundle_option*)CSL_VectorGetPointer(pThis->reply.subscribed_message.purchase_option.vec_bundle_option, bundleIndex);

    if (!bundle)
    {
        return NE_INVAL;
    }

    len = CSL_VectorGetLength(bundle->price_list.vec_price_option);
    if (index >= len)
    {
        return NE_INVAL;
    }

    price =(data_price_option*)CSL_VectorGetPointer(bundle->price_list.vec_price_option, index);

    SetPriceToABPrice(pds, price, priceOption);

    return NE_OK;
}

AB_DEF NB_Error
AB_AuthenticationInformationGetSubscribedMessageText(AB_AuthenticationInformation* pThis, AB_MessageTextCallback messageTextCallback, void* userData)
{
    data_formatted_text fmtText = {0};

    if (!pThis || !messageTextCallback)
    {
        return NE_INVAL;
    }

    fmtText = pThis->reply.subscribed_message.formatted_text;

    return AB_AuthenticationInformationGetFormattedText (pThis, &fmtText, messageTextCallback, userData);
}

AB_DEF NB_Error
AB_AuthenticationInformationGetPurchaseBundleOptionText(AB_AuthenticationInformation* pThis, AB_MessageTextCallback messageTextCallback, void* userData, int index)
{
    NB_Error err = NE_OK;

    data_bundle_option* pdo = NULL;
    data_formatted_text fmtText = {0};

    if (!pThis || !messageTextCallback)
    {
        return NE_INVAL;
    }

    if (pThis->reply.client_stored_message.purchase_option.vec_bundle_option)
    {
        int l = 0;
        l = CSL_VectorGetLength(pThis->reply.client_stored_message.purchase_option.vec_bundle_option);

        if (index > l)
        {
            return NE_RANGE;
        }

        pdo = (data_bundle_option*)CSL_VectorGetPointer(pThis->reply.client_stored_message.purchase_option.vec_bundle_option, index);
        if (NULL == pdo)
        {
            return NE_BADDATA;
        }

        fmtText = pdo->formatted_text;

        err = AB_AuthenticationInformationGetFormattedText(pThis, &fmtText, messageTextCallback, userData);
    }

    return err;

}

AB_DEF NB_Error
AB_AuthenticationInformationGetPurchaseBundlePriceOptionText(AB_AuthenticationInformation* pThis, AB_MessageTextCallback messageTextCallback, void* userData, int bundleIndex, int priceIndex)
{
    NB_Error err = NE_OK;
    data_bundle_option* bundle = NULL;
    data_price_option* price = NULL;
    data_formatted_text fmtText = {0};
    int len = 0;

    if (!pThis || bundleIndex < 0 || priceIndex < 0 || !messageTextCallback)
    {
        return NE_INVAL;
    }

    bundle = (data_bundle_option*)CSL_VectorGetPointer(pThis->reply.client_stored_message.purchase_option.vec_bundle_option, bundleIndex);

    if (!bundle)
    {
        return NE_INVAL;
    }

    len = CSL_VectorGetLength(bundle->price_list.vec_price_option);

    if (priceIndex >= len)
    {
        return NE_RANGE;
    }

    price =(data_price_option*)CSL_VectorGetPointer(bundle->price_list.vec_price_option, priceIndex);

    if(NULL == price)
    {
        return NE_BADDATA;
    }

    fmtText = price->recommended_option.formatted_text;

    err = AB_AuthenticationInformationGetFormattedText(pThis, &fmtText, messageTextCallback, userData);

    return err;

}

AB_DEF NB_Error
AB_AuthenticationInformationGetProductDescriptionText(AB_AuthenticationInformation* pThis, int index, AB_MessageTextCallback messageTextCallback, void* userData)
{
    data_formatted_text* fmtText = NULL;
    int len = 0;

    if (!pThis || !messageTextCallback)
    {
        return NE_INVAL;
    }

    len = CSL_VectorGetLength(pThis->reply.client_stored_message.product_description.vec_formatted_text);
    if (index < 0 || index >= len)
    {
        return NE_RANGE;
    }

    fmtText = (data_formatted_text*)CSL_VectorGetPointer(pThis->reply.client_stored_message.product_description.vec_formatted_text, index);

    return AB_AuthenticationInformationGetFormattedText(pThis, fmtText, messageTextCallback, userData);
}

AB_DEF NB_Error
AB_AuthenticationInformationGetAvailableExtAppFeatures(AB_AuthenticationInformation* pThis, int32* appMask)
{
    if (!pThis || !appMask)
    {
        return NE_INVAL;
    }

    *appMask = data_auth_reply_extapp_features(NB_ContextGetDataState(pThis->context), &pThis->reply);

    return NE_OK;
}

/*! @} */
