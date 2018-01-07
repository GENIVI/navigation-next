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

    @file abauthenticationparameters.h
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

#ifndef AB_AUTHENTICATIONPARAMETERS_H
#define AB_AUTHENTICATIONPARAMETERS_H

#include "abexp.h"
#include "nbcontext.h"

/*!
    @addtogroup abauthenticationparameters
    @{
*/


typedef enum 
{
    AB_Authentication_Bind_License,
    AB_Authentication_Add_Feature,
    AB_Authentication_List_Feature,
    AB_Authentication_Remove_Feature,
    AB_Authentication_Save_Optin_Response
} AB_Authentication_RequestType;

typedef struct AB_AuthenticationParameters AB_AuthenticationParameters;

/*! Create parameters for a authentication operation

@param context Address of current NB_Context
@param parameters On success, a newly created AB_AuthenitcationParameters object; NULL otherwise.  A valid object must be destroyed using AB_AuthenticationParametersDestroy().
@returns NB_Error
*/
AB_DEC NB_Error AB_AuthenticationParametersCreate(NB_Context* context, AB_AuthenticationParameters** parameters);

/*! Destroy a previously created parameters object

@param parameters Address of a valid AB_AuthenticationParameters object
@returns NB_Error
*/
AB_DEC NB_Error AB_AuthenticationParametersDestroy(AB_AuthenticationParameters* parameters);


/*! Sets the confirm Action for query

@param parameters Address of a valid AB_AuthenticationParameters object
@param messageConfirm message confirm for query 
@returns NB_Error
*/
AB_DEC NB_Error AB_AuthenticationParametersSetConfirmAction(AB_AuthenticationParameters* parameters, const char* messageConfirm);

/*! Sets the confirm Action for query

@param parameters Address of a valid AB_AuthenticationParameters object
@param confirmId Confirm ID to be set
@returns NB_Error
*/
AB_DEC NB_Error AB_AuthenticationParametersSetConfirmID(AB_AuthenticationParameters* parameters, const char* confirmId);

/*! Sets the confirm Action for query

@param parameters Address of a valid AB_AuthenticationParameters object
@param featureName Feature name to be set for add feature request
@returns NB_Error
*/
AB_DEC NB_Error AB_AuthenticationParametersSetAddFeatureName(AB_AuthenticationParameters* parameters, const char* featureName);


/*! Sets the confirm Action for query

@param parameters Address of a valid AB_AuthenticationParameters object
@param featureName Feature Name to be set for remove feature request
@returns NB_Error
*/
AB_DEC NB_Error AB_AuthenticationParametersSetRemoveFeatureName(AB_AuthenticationParameters* parameters, const char* featureName);

/*! Sets the confirm Action for query

@param parameters Address of a valid AB_AuthenticationParameters object
@param licenseKey License Key to be set for query 

@returns NB_Error
*/
AB_DEC NB_Error AB_AuthenticationParametersSetLicenseKey(AB_AuthenticationParameters* parameters, const char* licenseKey);

/*! Sets the confirm Action for query

@param parameters Address of a valid AB_AuthenticationParameters object
@param authType Auth Type to be set
@param confirmAction Confirm Action to be set
@param confirmId Confirm ID in message confirm to be set
@returns NB_Error
*/
AB_DEC NB_Error AB_AuthenticationParametersSetOptin(AB_AuthenticationParameters* parameters, const char* authType, const char* confirmAction, const char* confirmId);

/*! Sets the confirm Action for query

@param parameters Address of a valid AB_AuthenticationParameters object
@param request request operation to be set
@returns NB_Error
*/
AB_DEC NB_Error AB_AuthenticationParametersSetRequest(AB_AuthenticationParameters* parameters, AB_Authentication_RequestType request);

/*! Sets the want license message information in query

@param parameters Address of a valid AB_AuthenticationParameters object
@param timestamp timestamp to be added to data_want_license_message. This timestamp is obtained from AB_AuthenticationInformationGetClientStoredMessageDetail.
The timestamp is passed in want license message so that server will send us only the recent client stored message with ts > timestamp.

This API is deprecated for V6 onwards. Use AB_AuthenticationParametersSetWantPurchaseMessage

@returns NB_Error
*/

AB_DEC NB_Error AB_AuthenticationParametersSetWantLicenseMessage(AB_AuthenticationParameters* paramaters, nb_boolean licenseMessageFlag, uint32 timestamp);

/*! Set the language for authentication

@param parameters Address of a valid AB_AuthenticationParameters object
@param language specify language to get the messages in the authenticatoin query
@returns NB_Error
*/

AB_DEC NB_Error AB_AuthenticationParametersSetLanguage(AB_AuthenticationParameters* paramaters, const char* language);

/*! Sets the want purchase message information in query

@param parameters Address of a valid AB_AuthenticationParameters object
@param timestamp timestamp to be added to want-purchase-message element in auth query. This timestamp is obtained from AB_AuthenticationInformationGetClientStoredMessageDetail.
The timestamp is passed in want purchase message so that server will send us only the recent client stored message with ts > timestamp.
@returns NB_Error
*/

AB_DEC NB_Error AB_AuthenticationParametersSetWantPurchaseMessage(AB_AuthenticationParameters* paramaters, nb_boolean purchaseMessageFlag, uint32 timestamp);

/*! Sets the want marketing message information in query

@param parameters Address of a valid AB_AuthenticationParameters object
@param want marketing message boolean value, if true adds want-marketing-message element in the want-purchase-message element
@returns NB_Error
*/
AB_DEC NB_Error AB_AuthenticationParametersSetWantMarketingMessage(AB_AuthenticationParameters* pThis, nb_boolean wantMarkettingMessageFlag);


/*! Set the pin message infromation in query
 @param pThis - Address of a valid AB_AuthenticationParameters object
 @param wantPinMessageFlag - boolean value, if true adds want-marketing-message element in the want-purchase-message element
 @returns NB_Error
 */
AB_DEC NB_Error AB_AuthenticationParametersSetWantPinMessage(AB_AuthenticationParameters* pThis, nb_boolean wantPinMessageFlag);


/*! Sets the want extended marketing message information in query

 @param parameters Address of a valid AB_AuthenticationParameters object
 @param want extended marketing message boolean value, if true adds want-extended-marketing-message element in the want-purchase-message element
 @returns NB_Error
 */
AB_DEC NB_Error AB_AuthenticationParametersSetWantExtendedMarketingMessage(AB_AuthenticationParameters* pThis, nb_boolean wantExtendedMarkettingMessageFlag);

/*! Sets the want region element in query

@param parameters Address of a valid AB_AuthenticationParameters object
@param region boolean value, if true adds want-region element in query. If want-region element is present in the query, list-features returns region information in the reply. Part of V6
@returns NB_Error
*/

AB_DEC NB_Error AB_AuthenticationParametersSetWantRegion(AB_AuthenticationParameters* paramaters, nb_boolean region_info);

/*! Set the transactionId for authentication.

 @param parameters Address of a valid AB_AuthenticationParameters object
 @param transactionId Provide transactionId when you want to do a restore of an existing transaction/license.
 @returns NB_Error
 */

AB_DEC NB_Error AB_AuthenticationParametersSetTransactionId(AB_AuthenticationParameters* paramaters, const char* transactionId);

/*! Adds the want promo options element in query. Call this method to get dynamic promocode information in price options.
 
 @param parameters Address of a valid AB_AuthenticationParameters object
 @param wantPromoOptions boolean value, if true adds want-promo-options element in query. If want-promo-options element is present in the query, list-features returns dynamic promo code information in reply.
 @returns NB_Error
 */

AB_DEC NB_Error AB_AuthenticationParametersSetWantPromoOptions(AB_AuthenticationParameters* parameters, nb_boolean wantPromoOptions);

/*! Adds the want product description element in query. Call this method to get formatted text of product description in client stored message.

@param parameters Address of a valid AB_AuthenticationParameters object
@param wantProductDescription boolean value, if true adds want-product-description element in query. If want-product-description element is present in the query, server shall return product-description element with bundle description. The description shall be provided in multiple formatted text under bundle-option.
@returns NB_Error
*/

AB_DEC NB_Error AB_AuthenticationParametersSetWantProductDescription(AB_AuthenticationParameters* parameters, nb_boolean wantProductDescription);

/*! @} */

#endif	// AB_AUTHENTICATIONPARAMETERS_H

