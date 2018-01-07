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

    @file     nbmobilecouponsprivate.h
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

#ifndef NBMOBILECOUPONSPRIVATE_H
#define NBMOBILECOUPONSPRIVATE_H

/*!
    @addtogroup nbmobilecoupons
    @{
*/

#include "nberror.h"
#include "nbcontextprotected.h"
#include "nbiterationcommand.h"
#include "datautil.h"
#include "data_iter_command.h"
#include "data_position.h"
#include "data_store_coupon_data.h"
#include "data_search_filter.h"
#include "nbmobilecoupons.h"
#include "cslnetwork.h"


#define COUPONS_SERVLET  "coupons"

// TPS string constants
#define COUPON_AVAILABILITY_LOCAL        "local"
#define COUPON_AVAILABILITY_NATIONAL     "national"
#define COUPON_AVAILABILITY_ECOMMERCE    "ecommerce"

#define COUPON_ACQUISITION_IN_STORE      "in store"
#define COUPON_ACQUISITION_PRINTOUT      "printout"
#define COUPON_ACQUISITION_SHIPPED       "shipped"

#define COUPON_DISCOUNT_TYPE_NONE        "none"
#define COUPON_DISCOUNT_TYPE_AMOUNT      "amount"
#define COUPON_DISCOUNT_TYPE_PERCENTAGE  "percentage"

#define POSITION_VARIANT_POINT           "point"

#define COUPON_DISPLAYABLE_IMAGE_ID_NONE "none"

/*! Sets iteration command value

@param state Data util state
@param iterCommand Iteration command structure
@param command Iteration command value
@return NB_Error
*/
NB_Error SetIterationCommand(data_util_state* state, data_iter_command* iterCommand, NB_IterationCommand command);


/*! Copy mobile coupon.

@param context The NB context
@param coupon The mobile coupon
@param couponData The TPS mobile coupon structure
@return NB_Error
*/
NB_Error GetMobileCoupon(NB_Context* context, NB_Coupon* coupon, const data_coupon_data* couponData);


/*! Creates new store object.

@param context The NB context
@param store The store
@param storeData The TPS store structure
@return NB_Error
*/
NB_Error CreateStore(NB_Context* context, NB_Store** store, const data_store_data* storeData);


/*! Destroys store object.

@param store The store
*/
void DestroyStore(NB_Store* store);


/*! Creates a new store coupons object.

@param context The NB context
@param storeCoupons The store coupons
@param storeCouponsData The TPS store coupons structure
@return NB_Error
*/
NB_Error CreateStoreCoupon(NB_Context* context, NB_StoreCoupons** storeCoupons, const data_store_coupon_data* storeCouponsData);


/*! Destroys store coupon object.

@param storeCoupon The store coupon
*/
void DestroyStoreCoupon(NB_StoreCoupons* storeCoupon);


/*! Returns error code (server or network error).

@param context The NB context
@param response The network response
@return NB_Error
*/
NB_Error GetErrorCode(NB_Context* context, NB_NetworkResponse* response);


/*! Sets TPS position structure.

@param context The NB context
@param position The TPS position structure
@param searchRegion The search region
@return NB_Error
*/
NB_Error SetPositionFromSearchRegion(NB_Context* context, data_position* position, const NB_CouponSearchRegion* searchRegion);

/*! @} */

#endif
