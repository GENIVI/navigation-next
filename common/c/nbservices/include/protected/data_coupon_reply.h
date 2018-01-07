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

    @file     data_coupon_reply.h
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

#ifndef DATA_COUPON_REPLY_H
#define DATA_COUPON_REPLY_H

/*! @{ */

#include "datautil.h"

#include "data_coupon_error.h"
#include "data_store_response.h"
#include "data_coupon_response.h"
#include "data_load_image_response.h"
#include "data_load_image_error.h"
#include "data_category_list_response.h"

/* This top level query can contain only 1 sub element.
   Structure owns the request objects. Objects's memory will be released in data_coupon_reply_free().*/
typedef struct data_coupon_reply_
{
    /* Child Elements */

    data_store_response*          store_response;         /*<! Consolidated coupon search (stores + coupons). */
    data_coupon_response*         coupon_response;        /*<! Simple coupon search (coupons). */
    data_coupon_error*            coupon_error;           /*<! Contains error data if something went wrong in using the mobile coupon API. */
    data_load_image_error*        load_image_error;       /*<! Contains error data if something went wrong in loading an image. */
    data_load_image_response*     load_image_response;    /*<! Contains the image binary data for a load-image-request. */
    data_category_list_response*  category_list_response; /*<! Contains the list of categories requested. */

    /* Attributes */

} data_coupon_reply;

NB_Error    data_coupon_reply_init(data_util_state* state, data_coupon_reply* couponReply);
void        data_coupon_reply_free(data_util_state* state, data_coupon_reply* couponReply);

NB_Error    data_coupon_reply_from_tps(data_util_state* state, data_coupon_reply* couponReply, tpselt tpsElement);

NB_Error    data_coupon_reply_from_tps_error(data_util_state* state, NB_Error* serverError, tpselt tpsElement);

/*! @} */

#endif
