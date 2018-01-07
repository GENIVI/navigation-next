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

    @file     data_coupon_data.h
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.      
---------------------------------------------------------------------------*/

#ifndef DATA_COUPON_DATA_H
#define DATA_COUPON_DATA_H

/*! @{ */

#include "datautil.h"
#include "data_string.h"
#include "data_blob.h"
#include "nbmobilecoupons.h"
#include "data_image_urls.h"

typedef struct data_coupon_data_
{
    /* Child Elements */
    CSL_Vector*    availability;        /*!< Where the coupon is available. Example values are 'local', 'national', 'ecommerce' */
    CSL_Vector*    acquisition;         /*!< Indicates how the user will get the deal. Example values are: 'in store', 'printout', 'shipped'. */
    data_image_urls                     /*!< If present, this element contains links to deal image and thumbnails. */
                   image_urls;

    /* Attributes */
    data_string    title;               /*<! Title of coupon (short description). */
    data_string    description;         /*!< Long description of coupon. */
    data_string    conditions;          /*!< Conditions (if any) for the deal. */
    data_string    id;                  /*!< Unique ID of the coupon (provided by CSP). */
    uint32         start_date;          /*!< Date from which this coupon is valid (in seconds since the GPS epoch). */
    uint32         expiration_date;     /*!< Date until which this coupon is valid (in seconds since the GPS epoch). */
    data_string    currency;            /*!< Specifies currency units of the price.  Example: USD for US dollars.
                                             If the provider doesn't specify the value, use USD. */
    NB_CouponDiscountThresholdType
                   discount_type;       /*!< Discount type: 1 - percantage; 2 - amount*/

    double         discount_value;      /*!< This is the value of the discount on the coupon. It's unit is in discount-type (eg: 20 % , or $ 20 off). */
    double         buy_value;           /*!< This is the money the user will be paying for the deal. */
    double         list_value;          /*!< This is the normal price that the user would pay without the deal. */

    data_string    coupon_code;         /*!< If the coupon has a code that can be redeemed by typing it in, this value exists. */

    data_string    deal_url;            /*!< If the coupon has an external URL for a mobile site that can be loaded, then this field is set (clippable
                                             coupons will not have this, only displayable coupons will). */

} data_coupon_data;

NB_Error    data_coupon_data_init(data_util_state* state, data_coupon_data* couponData);
void        data_coupon_data_free(data_util_state* state, data_coupon_data* couponData);

NB_Error    data_coupon_data_from_tps(data_util_state* state, data_coupon_data* couponData, tpselt tpsElement);
tpselt      data_coupon_data_to_tps(data_util_state* state, data_coupon_data* couponData);

boolean     data_coupon_data_equal(data_util_state* state, data_coupon_data* left, data_coupon_data* right);
NB_Error    data_coupon_data_copy(data_util_state* state, data_coupon_data* dest, data_coupon_data* src);

NB_Error    data_coupon_data_from_nbcoupon(data_util_state* state, data_coupon_data* couponData, const NB_Coupon* coupon);

uint32   data_coupon_data_get_tps_size(data_util_state* pds, data_coupon_data* couponData);
void     data_coupon_data_to_buf(data_util_state* pds, data_coupon_data* couponData, struct dynbuf* pdb);
NB_Error data_coupon_data_from_binary(data_util_state* pds, data_coupon_data* couponData, byte** pdata, size_t* pdatalen);


/*! @} */

#endif
