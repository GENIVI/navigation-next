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

    @file ablicenseparameters.h
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

#ifndef AB_LICENSEPARAMETERS_H
#define AB_LICENSEPARAMETERS_H

#include "abexp.h"
#include "nbcontext.h"

/*!
    @addtogroup ablicenseparameters
    @{
*/

typedef struct AB_LicenseParameters AB_LicenseParameters;

typedef enum {
    AB_LicenseAction_Create,
    AB_LicenseAction_Update,
    AB_LicenseAction_List,
    AB_LicenseAction_Delete,
    AB_LicenseAction_Validate
} AB_LicenseAction;

/*! Create parameters for a license operation.

@param context Address of current NB_Context.
@param parameters On success, a newly created AB_AuthenitcationParameters object; NULL otherwise.  A valid object must be destroyed using AB_LicenseParametersDestroy().
@returns NB_Error
*/
AB_DEC NB_Error AB_LicenseParametersCreate(NB_Context* context, AB_LicenseParameters** parameters);

/*! Destroy a previously created parameters object.

@param parameters Address of a valid AB_LicenseParameters object.
@returns NB_Error
*/
AB_DEC NB_Error AB_LicenseParametersDestroy(AB_LicenseParameters* parameters);

/*! Set up parameters for a license request.

@param parameters Address of a valid AB_LicenseParameters object.
@param requestId The request Id that will be returned back to user in the corresponding AB_LicenseInformation object.
@param transactionId The transaction Id that will be returned back to user in the corresponding AB_LicensInformation object.  The transactionId is optional.
@param vendorName The vendor object defined in the database.
@param coutry Country of purchase, default is USA.
@param subscriberKey Key provided by the user.  It will be different depending each deployment.  i.e. for Verizon this will be the user MDN.  For Gokivo RIM it is the device PIN.
@param action Action for the license request.
@returns NB_Error
*/
AB_DEC NB_Error AB_LicenseParametersSetRequest(
    AB_LicenseParameters* parameters,
    const char* requestId,
    const char* transactionId,
    const char* vendorName,
    const char* vendorUserId,
    const char* country,
    const char* subscriberKey,
    AB_LicenseAction action);

/*! Create a license request object and attach to the parameters object.

@param parameters Address of a valid AB_LicenseParameters object.
@param key The old, existing or new license key.
@param productId The product Id of the license.
@param licRequestId  Id for this license request object to be returned in the corresponding AB_LicenseInformation object.
@returns NB_Error
*/
AB_DEC NB_Error AB_LicenseParametersCreateLicense(
    AB_LicenseParameters* parameters,
    const char* key,
    const char* productId,
    const char* licRequestId,
    int *licIndex);

/*! Set parameter values for a license request.

@param parameters Address of a valid AB_LicenseParameters object.
@param licenseIndex Index of the license request object to be updated.
@param bundleName Name of predefined bundle in the database.
@param bundleType Subscription type for the bundle.
@returns NB_Error
NOTE: previous AB_LicenseParametersAddBundle with AB_BundleType is deprecated
*/
AB_DEC NB_Error AB_LicenseParametersAddBundle(
    AB_LicenseParameters* parameters,
    int licenseIndex,
    const char* bundleName,
    const char* bundleType);

/*! Set promo code parameter values for a license request.

@param parameters Address of a valid AB_LicenseParameters object.
@param licenseIndex Index of the license request object to be updated.
@param bundleIndex Index of the bundle object to be updated.
@param promocode Promotion code for the bundle.
@returns NB_Error
*/
AB_DEC NB_Error AB_LicenseParametersAddBundlePromoCode(
    AB_LicenseParameters* parameters,
    int licenseIndex,
    int bundleIndex,
    const char* promoCode);


/*! Set language for message in reply.

@param parameters Address of a valid AB_LicenseParameters object.
@param language Language for message in reply.
@returns NB_Error
*/
AB_DEC NB_Error AB_LicenseParametersSetLanguage(
    AB_LicenseParameters* parameters,
    const char* language);

/*! @} */

#endif	// AB_LICENSEPARAMETERS_H

