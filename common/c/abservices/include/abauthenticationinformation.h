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

    @file abauthenticationinformation.h
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

#ifndef AB_AUTHENTICATIONINFORMATION_H
#define AB_AUTHENTICATIONINFORMATION_H

#include "abexp.h"
#include "nberror.h"
#include "paltypes.h"
#include "nbplace.h"
#include "csltypes.h"
#include "nbextappcontent.h"

/*!
    @addtogroup abauthenticationinformation
    @{
*/

typedef struct AB_AuthenticationInformation AB_AuthenticationInformation;

/*! Message Details from Server. */
typedef struct 
{
    char*               id;             /*!< Message ID */
    char*               title;          /*!< Message title */
    char*               type;           /*!< Message type */
    char*               language;       /*!< Message language */
    char*               accept_text;    /*!< Display text for accept */
    char*               center_text;    /*!< Display text for center */
    char*               decline_text;   /*!< Display txt for decline */
    char*               url;            /*!< Message url */    
    nb_boolean          confirm;        /*!< Confirmation required */
    uint32              time;           /*!< Download time for comparisons */
} AB_MessageDetail;

typedef struct
{
    char*       name;
    char*       type;
    uint32      endDate;
    char*       region;         /*!<delimited list of country codes supported for feature*/
}AB_FeatureDetail;

/*! Client stored Message Details from Server. */
typedef struct
{    
    char*               type;           /*!< Message type */
    char*               language;       /*!< Message language */
    uint32              timestamp;        /*!< Timestamp of message */
    char*               accept_text;      /*!< Accept text*/
    char*               decline_text;     /*!< Decline text*/
    char*               option_text;      /*!< Optional center text*/
    int                 bundle_count;     /*!< Number of purchase option bundles available*/
}AB_ClientStoredMessageDetail;

/*! Bundle option */
typedef struct
{
    const char*      title;             /*!< Pointer to display name of bundle*/
    const char*      description;       /*!< Pointer to bundle description */
    const char*      name;              /*!< Pointer to bundle id*/
    const char*      feature_codes;     /*!< Pointer to delimited feature ids in the bundle */
    int              price_option_count; /*!< number of price options in the bundle */
    const char*      region;            /*!< delimited string of country codes supported in the bundle*/
    int              token_id;       /*!< id of token to use for the bundle */
}AB_BundleOption;

/* Price option*/
typedef struct
{
    nb_boolean          is_recommended_option; /*!< Recommended option, if true set as default option*/
    const char*         data;               /*!< Pointer to display text for price option */
    const char*         type;               /*!< Pointer to type of price option */
    uint32              enddate;             /*!< bundle end date for this pricing option*/
    nb_boolean          has_promocode_offering; /*! <If TRUE, dynamic promotion available */
}AB_PriceOption;

/* Subscribed message */
typedef struct
{
    int              bundle_count;       /*!< number of bundles subscribed to */
    const char*      language;           /*!< Pointer to language for subscribed message */
    uint32           ts;                 /*!< message timestamp */
    const char*      maptileAccessToken; /*!< Pointer to the map tile access for subscribed message */
}AB_SubscribedMessage;

typedef struct
{
    const char* pinMessage;              /*!< Pin Message from server when successful purchase */
}AB_PinMessage;

/*! Callback function signature for function passed into AB_AuthenticationInformationGetMessageText().

    This is the function signature required of the callback function required for
    AB_AuthenticationInformationGetMessageText() to get the message formatted text.

    @return a NB_Error code, indicating success or failure

    @see AB_AuthenticationInformationGetMessageText
*/
typedef NB_Error (*AB_MessageTextCallback)(
    NB_Font font,                               /*!< Font to be used for the message text */
    nb_color color,                             /*!< Color to be used for the message text */
    const char* text,                           /*!< Text of message */
    nb_boolean newLine,                         /*!< Indicates if new line should follow message text */
    const char* href,							/*!< anchor for the specified URL */
    const char* href_text,						/*!< text associated with URL */
    void* userData                              /*!< User data provided at AB_ServerMessageEnumMessageText call  */
);

/*! returns the status code

@param information Address of a valid AB_AuthenticationInformation object
@param statusCode Status Code to be returned
@returns NB_Error
*/
AB_DEC NB_Error AB_AuthenticationInformationGetStatusCode(AB_AuthenticationInformation* information, int32* statusCode);

/*! returns the Number of feature

@param information Address of a valid AB_AuthenticationInformation object
@param featureCount Feature Count from server
@returns NB_Error
*/
AB_DEC NB_Error AB_AuthenticationInformationFeatureCount(AB_AuthenticationInformation* information, int* featureCount);

/*! returns the info about a feature

@param information Address of a valid AB_AuthenticationInformation object
@param index Index of the vector for which client needs data
@param featureDetail On success, a newly created AB_FeatureDetail object with copied data; NULL otherwise.  A valid object must be destroyed using AB_AuthenticationInformationgetGetFeatureResultDestroy().

@returns NB_Error

@see AB_AuthenticationInformationgetGetFeatureResultDestroy
*/
AB_DEC NB_Error AB_AuthenticationInformationGetFeatureResult(AB_AuthenticationInformation* information, int index, AB_FeatureDetail** featureDetail);

/*! Destroy the AB_AuthenticationInformation object

@param information Address of a valid AB_AuthenticationInformation object
@returns NB_Error
*/
AB_DEC NB_Error AB_AuthenticationInformationDestroy(AB_AuthenticationInformation* information);

/*! Destroy feature result details

This function destroy the member of AB_FeatureDetail data structure.

@param featureDetail Pointer to AB_FeatureDetail
@returns NB_Error
*/
AB_DEC void AB_AuthenticationInformationGetFeatureResultDestroy(AB_FeatureDetail* featureDetail);

/*! Get client stored message details

This function returns the AB_ClientStoredMessageDetail data structure. AB_ClientStoredMesageDetail should be deleted by caller

@param information An AB_AuthenticationInformation object
@param messageDetail On success, a newly created AB_ClientStoredMessageDetail object with copied data; NULL otherwise.  
A valid object must be destroyed using AB_AuthenticationInformationDestroyClientStoredMessageDetail().

@see AB_AuthenticationInformationDestroyClientStoredMessageDetail

@returns NB_Error
*/
AB_DEC NB_Error AB_AuthenticationInformationGetClientStoredMessageDetail(AB_AuthenticationInformation* information, AB_ClientStoredMessageDetail** clientMessage);

/*! Destroy client stored message details

This function destroys the members of AB_ClientStoredMessageDetail data structure.

@param message Pointer to AB_ClientStoredMessageDetail
@returns NB_Error
*/
AB_DEC void AB_AuthenticationInformationDestroyClientStoredMessageDetail(AB_ClientStoredMessageDetail* clientMessage);

/*! returns the Number of messages

@param information Address of a valid AB_AuthenticationInformation object
@param messageCount Message Count from server
@returns NB_Error
*/
AB_DEC NB_Error AB_AuthenticationInformationGetMessageCount(AB_AuthenticationInformation* information, int* messageCount);

/*! returns the info about a message

@param information Address of a valid AB_AuthenticationInformation object
@param index Index of the vector for which client needs data
@param messageDetail On success, a newly created AB_MessageDetail object with copied data; NULL otherwise.  
A valid object must be destroyed using AB_AuthenticationInformationgetDestroyMessageDetail().

@returns NB_Error

@see AB_AuthenticationInformationgetDestroyMessageDetail
*/
AB_DEC NB_Error AB_AuthenticationInformationGetMessageDetail(AB_AuthenticationInformation* information, int index, AB_MessageDetail** featureDetail);


/*! Destroy message details

This function destroys the members of AB_MessageDetail data structure.

@param messageDetail Pointer to AB_MessageDetail
@returns NB_Error
*/
AB_DEC void AB_AuthenticationInformationDestroyMessageDetail(AB_MessageDetail* messageDetail);


/*! Requests message text and formatting callback for specfied message ID

This function requests the message text and formatting info for a specified message ID.
The message info will be returned via the specified callback.

@param information An AB_AuthenticationInformation object
@param index  index of message to retrieve
@param messageTextCallback Pointer to AB_MessageTextCallback callback function to be called with message info
@param userData Pointer to user data that will be included in the messageTextCallback function call
@returns NB_Error
*/
AB_DEC NB_Error AB_AuthenticationInformationGetMessageText(AB_AuthenticationInformation* information, int index, AB_MessageTextCallback messageTextCallback, void* userData);


/*! Requests message text and formatting callback for client stored message

This function requests the message text and formatting info for client stored message.
The message info will be returned via the specified callback.

@param information An AB_AuthenticationInformation object
@param index  index of message to retrieve
@param messageTextCallback Pointer to AB_MessageTextCallback callback function to be called with message info
@param userData Pointer to user data that will be included in the messageTextCallback function call
@returns NB_Error
*/
AB_DEC NB_Error AB_AuthenticationInformationGetClientStoredMessageText(AB_AuthenticationInformation* information, AB_MessageTextCallback messageTextCallback, void* userData);

/*! Get the purchase bundle by index 

Call this function to get the bundle information for the bundles in client stored message. 

@see AB_AuthenticationInformationGetClientStoredMessageDetail to get the number of bundles in a client stored message

@param information - An AB_AuthenticationInformation object
@param index -  index of purchase bundle to retrieve
@param bundleOption - AB_BundleOption object to store bundle information
@returns NB_Error
*/
AB_DEC NB_Error AB_AuthenticationInformationGetPurchaseBundle(AB_AuthenticationInformation* information, int index, AB_BundleOption* bundleOption);

/*! Get the purchase bundle pricing

Call this function to get the pricing information in a bundle. You can get the number of pricing options in a bundle
by calling AB_AuthenticationInformationGetPurchaseBundle

@param information - An AB_AuthenticationInformation object
@param bundleIndex -  index of purchase bundle
@param index - index of the price option within a bundle
@param priceOption - AB_PriceOption object to store pricing information
@returns NB_Error

@see AB_AuthenticationInformationGetPurchaseBundle

*/
AB_DEC NB_Error AB_AuthenticationInformationGetPurchaseBundlePricing(AB_AuthenticationInformation* information, int bundleIndex, int index, AB_PriceOption* priceOption);

/*! Get the subscribed message

@param information - An AB_AuthenticationInformation object
@param subscribedMessage - AB_SubscribedMessage structure to store subscription information
@returns NB_Error
*/
AB_DEC NB_Error AB_AuthenticationInformationGetSubscribedMessage(AB_AuthenticationInformation* information, AB_SubscribedMessage* subscribedMessage);


/*! Get the pin attribute message after success purchase
 @param pThis - An AB_AuthenticationInformation object
 @param pinMessage - the Pin code from server
 @return NB_Error
 */
AB_DEC NB_Error AB_AuthenticationInformationGetPinMessage(AB_AuthenticationInformation* pThis, AB_PinMessage* pinMessage);


/*! Get the pin message format text
 @param pThis - An AB_AuthenticationInformation object
 @param messageTextCallback Pointer to AB_MessageTextCallback callback function to be called with message info
 @param userData Pointer to user data that will be included in the messageTextCallback function call
 @returns NB_Error
 */
AB_DEC NB_Error AB_AuthenticationInformationGetPinMessageText(AB_AuthenticationInformation* pThis, AB_MessageTextCallback messageTextCallback, void* userData);


/*! Get subscribed bundle information

Call this function to get the subscribed bundle information The number of subscribed bundles is available
by calling AB_AuthenticationInformationGetSubscribedMessage

@param information - An AB_AuthenticationInformation object
@param index -  index of subscribed bundle to retrieve
@param bundleOption - AB_BundleOption object to store bundle information
@returns NB_Error

@see AB_AuthenticationInformationGetSubscribedMessage
*/
AB_DEC NB_Error AB_AuthenticationInformationGetSubscribedBundle(AB_AuthenticationInformation* information, int index, AB_BundleOption* bundleOption);

/*! Get the subscribed bundle pricing

Call this function to get the pricing information for a bundle. The number of price options can be obtained by calling
AB_AuthenticationInformationGetSubscribedBundle

@param information - An AB_AuthenticationInformation object
@param bundleIndex -  index of subscribed bundle
@param index - index of the price option within a bundle
@param priceOption - AB_PriceOption object to store pricing information
@returns NB_Error

@see AB_AuthenticationInformationGetSubscribedBundle
*/
AB_DEC NB_Error AB_AuthenticationInformationGetSubscribedBundlePricing(AB_AuthenticationInformation* information, int bundleIndex, int index, AB_PriceOption* priceOption);

/*! Get subscribed message text 

This function requests the message text and formatting info for subscribed message.
The message info will be returned via the specified callback.

@param information An AB_AuthenticationInformation object
@param index  index of message to retrieve
@param messageTextCallback Pointer to AB_MessageTextCallback callback function to be called with message info
@param userData Pointer to user data that will be included in the messageTextCallback function call
@returns NB_Error
*/
AB_DEC NB_Error AB_AuthenticationInformationGetSubscribedMessageText(AB_AuthenticationInformation* information, AB_MessageTextCallback messageTextCallback, void* userData);

/*! Get purchase bundle option formatted text

This function requests the message text and formatting info for puchase bundle option.
The message info will be returned via the specified callback.

@param information An AB_AuthenticationInformation object
@param messageTextCallback Pointer to AB_MessageTextCallback callback function to be called with message info
@param userData Pointer to user data that will be included in the messageTextCallback function call
@param index index of bundle to retrieve.
@returns NB_Error
*/
AB_DEC NB_Error AB_AuthenticationInformationGetPurchaseBundleOptionText(AB_AuthenticationInformation* information, AB_MessageTextCallback messageTextCallback, void* userData, int index);

/*! Get purchase bundle price option formatted text

This function requests the message text and formatting info for puchase bundle price option.
The message info will be returned via the specified callback.

@param information An AB_AuthenticationInformation object
@param messageTextCallback Pointer to AB_MessageTextCallback callback function to be called with message info
@param userData Pointer to user data that will be included in the messageTextCallback function call
@param bundleIndex index of bundle option to retrieve.
@param priceIndex index of price option to retrieve.
@returns NB_Error
*/
AB_DEC NB_Error AB_AuthenticationInformationGetPurchaseBundlePriceOptionText(AB_AuthenticationInformation* information, AB_MessageTextCallback messageTextCallback, void* userData, int bundleIndex, int priceIndex);


/*! Get product description formatted text

This function requests the message text and formatting info for product description.
The message info will be returned via the specified callback.

@param information An AB_AuthenticationInformation object
@param index Index of formatted text to retrieve
@param messageTextCallback Pointer to AB_MessageTextCallback callback function to be called with message info
@param userData Pointer to user data that will be included in the messageTextCallback function call
@returns NB_Error
*/
AB_DEC NB_Error AB_AuthenticationInformationGetProductDescriptionText(AB_AuthenticationInformation* information, int index, AB_MessageTextCallback messageTextCallback, void* userData);


/*! Get available 3rd party apps mask

This function returns mask of available 3rd party apps

@param information An AB_AuthenticationInformation object
@param appMask A pointer to int32 to be initialized with mask of available 3rd party apps from NB_ThirdPartyApp
@returns NB_Error
*/
AB_DEC NB_Error AB_AuthenticationInformationGetAvailableExtAppFeatures(AB_AuthenticationInformation* information, int32* appMask);

/*! @} */

#endif	// AB_AUTHENTICATIONINFORMATION_H
