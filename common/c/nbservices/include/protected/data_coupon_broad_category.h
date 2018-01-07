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

    @file     data_coupon_broad_category.h
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

#ifndef DATA_BROAD_CATEGORY_H
#define DATA_BROAD_CATEGORY_H

/*! @{ */

#include "datautil.h"
#include "data_string.h"

typedef struct data_coupon_broad_category_
{
    /* Child Elements */

    /* Attributes */
    data_string  broad_category_text; /*!< A broad category for coupons and deals. */

} data_coupon_broad_category;

NB_Error    data_coupon_broad_category_init(data_util_state* state, data_coupon_broad_category* broadCategory);
void        data_coupon_broad_category_free(data_util_state* state, data_coupon_broad_category* broadCategory);

NB_Error    data_coupon_broad_category_from_tps(data_util_state* state, data_coupon_broad_category* broadCategory, tpselt tpsElement);
tpselt      data_coupon_broad_category_to_tps(data_util_state* state, data_coupon_broad_category* broadCategory);

boolean     data_coupon_broad_category_equal(data_util_state* state, data_coupon_broad_category* left, data_coupon_broad_category* right);
NB_Error    data_coupon_broad_category_copy(data_util_state* state, data_coupon_broad_category* dest, data_coupon_broad_category* src);

uint32      data_coupon_broad_category_get_tps_size(data_util_state* state, data_coupon_broad_category* broadCategory);
void        data_coupon_broad_category_to_buf(data_util_state* state, data_coupon_broad_category* broadCategory, struct dynbuf *buffer);
NB_Error    data_coupon_broad_category_from_binary(data_util_state* state, data_coupon_broad_category* broadCategory, byte** ppdata, size_t* pdatalen);

/*! @} */

#endif
