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

    @file ablicenseinformation.h
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

#ifndef AB_LICENSEINFORMATION_H
#define AB_LICENSEINFORMATION_H

#include "abexp.h"
#include "nberror.h"
#include "paltypes.h"
#include "nbplace.h"
#include "csltypes.h"

#include "abauthenticationinformation.h"

/*!
    @addtogroup ablicenseinformation
    @{
*/

typedef struct AB_LicenseInformation AB_LicenseInformation;

typedef enum {
   AB_LicenseStatus_Success,
   AB_LicenseStatus_DoesNotExist,
   AB_LicenseStatus_HasExpired,
   AB_LicenseStatus_AuthenticationFailed,
   AB_LicenseStatus_XmlParserError,
   AB_LicenseStatus_SystemError,
   AB_LicenseStatus_InvalidParametners
}AB_LicenseStatus;

typedef struct {
    char*      description;             /*!< Pointer to promo code description*/
    char*      value;					  /*!< Pointer to value of promo code */
}AB_PromotionCode;

/*! Destroy a previously created information object.

@param information Address of a valid AB_LicenseInformation object.
@returns NB_Error
*/
AB_DEC NB_Error
AB_LicenseInformationDestroy(AB_LicenseInformation* information);

/*! Returns the status code

@param information Address of a valid AB_LicenseInformation object.
@param statusCode Status Code to be returned.
@returns NB_Error
*/
AB_DEC NB_Error AB_LicenseInformationGetStatus(
    AB_LicenseInformation* information, 
    AB_LicenseStatus* statusCode);

/*! Returns the request id correlates to the license request.

@param information Address of a valid AB_LicenseInformation object.
@param request id to be returned.  The caller needs not worry about freeing the data.  Destroys the information object or invokes AB_LicenseHandlerStartRequest invalidates the returned data.
@returns NB_Error
*/
AB_DEC NB_Error AB_LicenseInformationGetRequestId(
    AB_LicenseInformation* information,
    const char** requestId);

/*! Returns the transaction id for the license request.

@param information Address of a valid AB_LicenseInformation object.
@param transactionId Transaction Id to be returned.  The caller needs not worry about freeing the data.  Destroys the information object or invokes AB_LicenseHandlerStartRequest invalidates the returned data.
@returns NB_Error
*/
AB_DEC NB_Error AB_LicenseInformationGetTransactionId(
    AB_LicenseInformation* information,
    const char** transactionId);

/*! Returns the number of license returned for the request

@param information Address of a valid AB_LicenseInformation object.
@param licensceCount License count to be returned.
@returns NB_Error
*/
AB_DEC NB_Error AB_LicenseInformationGetLicenseCount(
    AB_LicenseInformation* information,
    int *licenseCount);

/*! Returns the license details

@param information Address of a valid AB_LicenseInformation object.
@param licenseIndex Index to returned licenses.
@param key The old, existing or new key for the license to be returned.  The caller needs not worry about freeing the data.  Destroys the information object or invokes AB_LicenseHandlerStartRequest invalidates the returned data.
@param productId The product Id for the license to be returned.  The caller needs not worry about freeing the data.  Destroys the information object or invokes AB_LicenseHandlerStartRequest invalidates the returned data.
@param licenseRequestId Id for the license requested to be returned.  The caller needs not worry about freeing the data.  Destroys the information object or invokes AB_LicenseHandlerStartRequest invalidates the returned data.
@param statusCode Status Code to be returned
@returns NB_Error
*/
AB_DEC NB_Error AB_LicenseInformationGetLicenseDetail(
    AB_LicenseInformation* information,
    int licenseIndex,
    const char** key,
    const char** productId,
    const char** licenseRequestId,
    AB_LicenseStatus *statusCode);

/*! Returns the license error desription

@param information Address of a valid AB_LicenseInformation object.
@param error description of the license error.
@returns NB_Error
*/
AB_DEC NB_Error AB_LicenseInformationGetError(
    AB_LicenseInformation* information, 
    const char** error);

/*! Returns the license message detail

Populates the given AB_MessageDetail structure with the license message detail returned, if any.
The AB_LicenseInformationFreeMessageDetail() function should be called when done to free all
allocated strings referenced in the structure.

@param information Address of a valid AB_LicenseInformation object.
@param messageDetail pointer to AB_MessageDetail structure that will be populated with message detail
@returns NB_Error, NE_OK if message detail available, NE_NOENT if not
*/
AB_DEC NB_Error AB_LicenseInformationGetMessageDetail(
    AB_LicenseInformation* information, 
    AB_MessageDetail* messageDetail);

/*! Frees all strings in given license message detail structure

@param messageDetail pointer to AB_MessageDetail structure to free all strings for
@returns NB_Error
*/
AB_DEC NB_Error
AB_LicenseInformationFreeMessageDetail(AB_MessageDetail* messageDetail);

/*! Get license message text 

This function requests the license message text and formatting info.
The message info will be returned via the specified callback.

@param information Address of a valid AB_LicenseInformation object.
@param messageTextCallback Pointer to AB_MessageTextCallback callback function to be called with message info
@param userData Pointer to user data that will be included in the messageTextCallback function call
@returns NB_Error
*/
AB_DEC NB_Error
AB_LicenseInformationGetMessageText(
    AB_LicenseInformation* information,
    AB_MessageTextCallback messageTextCallback,
    void* userData);

/*! Get license promotion code

This function requests the license promotion code and description information.

@param information Address of a valid AB_LicenseInformation object.
@param promotion code structure. The caller of this method will free this structure.
@returns NB_Error
*/
AB_DEC NB_Error
AB_LicenseInformationGetPromoCode(
    AB_LicenseInformation* information,
    AB_PromotionCode **promoCode);

/*! Destroy promo code

This function destroys the members of AB_PromoCode data structure.

@param messageDetail Pointer to AB_PromoCode
@returns NB_Error
*/
AB_DEC NB_Error
AB_LicenseInformationDestroyPromoCode(
    AB_PromotionCode* promoCode);


/*! Check if response contains license validation 
 
@return NB_Error
*/

AB_DEC nb_boolean AB_LicenseInformationIsValidationResponse ();

/*! @} */

#endif	// AB_LICENSEINFORMATION_H
