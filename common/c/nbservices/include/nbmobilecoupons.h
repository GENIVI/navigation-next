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

    @file     nbmobilecoupons.h
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

#ifndef NBMOBILECOUPONS_H
#define NBMOBILECOUPONS_H

#include "paltypes.h"
#include "nblocation.h"
#include "nbsearchparameters.h"

/*!
    @addtogroup nbmobilecoupons
    @{
*/
// TODO: need to implement dynamic strings
#define NB_COUPON_TITLE_LEN          200
#define NB_COUPON_CONDITIONS_LEN     256
#define NB_COUPON_ID_LEN             200
#define NB_COUPON_CURRENCY_LEN       10
#define NB_COUPON_CODE_LEN           200
#define NB_COUPON_DEAL_URL_LEN       1000
#define NB_COUPON_IMAGE_URL_LEN      1000

#define NB_STORE_NAME_LEN            200
#define NB_STORE_ID_LEN              200


/*! Types of mobile coupons discount threshold types.

@see NB_StoreCouponParametersCreate
*/
typedef enum
{
    NB_CDTT_None = 0,       /*!< No discount. */
    NB_CDTT_Amount,         /*!< Discount threshold in absolute value. */
    NB_CDTT_Percentage      /*!< Discount threshold in percents. */

} NB_CouponDiscountThresholdType;

/*! Information of mobile coupon.

    @see NB_StoreCouponParametersCreate
    @see NB_SimpleCouponInformationGetCoupons
*/
typedef struct
{
    char**         availability;                            /*!< Where the coupon is available. */
    uint32         availabilityCount;                       /*!< Availability elements count. */
    char**         acquisition;                             /*!< Indicates how the user will get the deal. */
    uint32         acquisitionCount;                        /*!< Acquisition elements count. */
    char           title[NB_COUPON_TITLE_LEN];              /*!< Title of coupon (short description). */
    char*          description;                             /*!< Long description of coupon. User frees this field manually */
    char           conditions[NB_COUPON_CONDITIONS_LEN];    /*!< Long description of coupon. */
    char           id[NB_COUPON_ID_LEN];                    /*!< Unique ID of the coupon (provided by CSP). */
    uint32         startDate;                               /*!< Date from which this coupon is valid (in seconds since the GPS epoch). */
    uint32         expirationDate;                          /*!< Date for which this forecast is valid (in seconds since the GPS epoch). */
    char           currency[NB_COUPON_ID_LEN];
    NB_CouponDiscountThresholdType
                   discountType;                            /*!< Discount type: can be 'percentage' or 'amount' or 'none'. */
    double         discountValue;                           /*!< It's unit is in discount-type (eg: 20 % , or $ 20 off). This value is 0 if discount-type is 'none'. */
    double         buyValue;                                /*!< This is the money the user will be paying for the deal. */
    double         listValue;                               /*!< This is the normal price that the user would pay without the deal. */
    char           couponCode[NB_COUPON_CODE_LEN];          /*!< If the coupon has a code that can be redeemed by typing it in, this value exists. */
    char           dealUrl[NB_COUPON_DEAL_URL_LEN];         /*!< If the coupon has an external URL for a mobile site that can be loaded, then this field is set (clippable coupons will not have this, only displayable coupons will). */
    char           imageUrl[NB_COUPON_IMAGE_URL_LEN];       /*!< URL to fetch the image for the coupon. */
    char           smallThumbUrl[NB_COUPON_IMAGE_URL_LEN];  /*!< URL to fetch a small thimb of the image. */
    char           mediumThumbUrl[NB_COUPON_IMAGE_URL_LEN]; /*!< URL to fetch a medium thimb of the image. */
    char           largeThumbUrl[NB_COUPON_IMAGE_URL_LEN];  /*!< URL to fetch a large thimb of the image. */

} NB_Coupon;

/*! Information of retailer's store.

    @see NB_StoreCouponParametersCreate
*/
typedef struct
{
    NB_Place  place;                                  /*!< Street address of retailer; may contain lat/lon information as well. */
    char      name[NB_STORE_NAME_LEN];                /*!< Name of retailer (eg: Ralphs). */
    char      id[NB_STORE_ID_LEN];                    /*!< Unique ID for retailer, on CSP system. */
    uint32    count;                                  /*!< Number of coupons found in the store. */
    char      imageUrl[NB_COUPON_IMAGE_URL_LEN];      /*!< URL to fetch the image for the coupon. */
    char      smallThumbUrl[NB_COUPON_IMAGE_URL_LEN]; /*!< URL to fetch a small thimb of the image. */
    char      mediumThumbUrl[NB_COUPON_IMAGE_URL_LEN];/*!< URL to fetch a medium thimb of the image. */
    char      largeThumbUrl[NB_COUPON_IMAGE_URL_LEN]; /*!< URL to fetch a large thimb of the image. */

    char**    broadCategories;           /*!< Array of broad categories. */
    uint32    broadCategoriesNumber;     /*!< Number of broad categories. */
    char**    categories;                /*!< Array of categories. */
    uint32    categoriesNumber;          /*!< Number of categories. */
    char**    subCategories;             /*!< Array of sub-categories. */
    uint32    subCategoriesNumber;       /*!< Number of sub-categories. */

} NB_Store;

/*! This structure contains information about store and associated coupons.

    @see NB_StoreCouponInformationGetStoreCoupons
*/
typedef struct
{
    NB_Store      store;           /*!< This is the store that the coupons are associated with. */
    NB_Coupon*    coupons;         /*!< Array of coupons associated with the store. */
    uint32        couponsNumber;   /*!< Number of coupons in array. */
} NB_StoreCoupons;

/*! Types of mobile coupon search region.

    @see NB_CouponSearchRegion
*/
typedef enum
{
    NB_CSRT_None = 0,
    NB_CSRT_Center,      /*!< Use the center point for the search. */
    NB_CSRT_BoundingBox  /*!< Use the bounding box for the search. */

} NB_CouponSearchRegionType;

/*! This structure describes mobile coupon search region.
*/
typedef struct
{
    NB_CouponSearchRegionType  type;         /*!< Type of mobile coupon search region. */
    NB_LatitudeLongitude       center;       /*!< Center point for search. Valid if type is set to 'NB_MCSRT_Center'. */
    NB_BoundingBox             boundingBox;  /*!< Bounding box for search. Valid if type is set to 'NB_MCSRT_BoundingBox'. */

} NB_CouponSearchRegion;

/*! @} */

#endif
