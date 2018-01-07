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

    @file     data_coupon_sub_category.c
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

/*! @{ */

#include "data_coupon_sub_category.h"

NB_Error
data_coupon_sub_category_init(data_util_state* state, data_coupon_sub_category* subCategory)
{
    DATA_MEM_ZERO(subCategory, data_coupon_sub_category);
    return data_string_init(state, &subCategory->sub_category_text);
}

void
data_coupon_sub_category_free(data_util_state* state, data_coupon_sub_category* subCategory)
{
    data_string_free(state, &subCategory->sub_category_text);
}

NB_Error
data_coupon_sub_category_from_tps(data_util_state* state, data_coupon_sub_category* subCategory, tpselt tpsElement)
{
    NB_Error err = NE_OK;
    if (tpsElement == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    data_coupon_sub_category_free(state, subCategory);
    err = data_coupon_sub_category_init(state, subCategory);
    if (err != NE_OK)
    {
        return err;
    }

    err = err ? err : data_string_from_tps_attr(state, &subCategory->sub_category_text, tpsElement, "coupon-sub-category-text");

errexit:
    if (err != NE_OK)
    {
        data_coupon_sub_category_free(state, subCategory);
    }
    return err;
}

tpselt
data_coupon_sub_category_to_tps(data_util_state* state, data_coupon_sub_category* subCategory)
{
    tpselt tpsElement = te_new("coupon-sub-category");
    if (tpsElement == NULL)
    {
        goto errexit;
    }

    if (!te_setattrc(tpsElement, "coupon-sub-category-text", data_string_get(state, &subCategory->sub_category_text)))
    {
        goto errexit;
    }

    return tpsElement;

errexit:
    te_dealloc(tpsElement);
    return NULL;
}

boolean
data_coupon_sub_category_equal(data_util_state* state, data_coupon_sub_category* left, data_coupon_sub_category* right)
{
    return (boolean) data_string_equal(state, &left->sub_category_text, &right->sub_category_text);
}

NB_Error
data_coupon_sub_category_copy(data_util_state* state, data_coupon_sub_category* dest, data_coupon_sub_category* src)
{
    NB_Error err = NE_OK;
    data_coupon_sub_category_free(state, dest);
    err = err ? err : data_coupon_sub_category_init(state, dest);

    err = err ? err : data_string_copy(state, &dest->sub_category_text, &src->sub_category_text);

    return err;
}

uint32
data_coupon_sub_category_get_tps_size(data_util_state* state, data_coupon_sub_category* subCategory)
{
    return data_string_get_tps_size(state, &subCategory->sub_category_text);
}

void
data_coupon_sub_category_to_buf(data_util_state* state, data_coupon_sub_category* subCategory, struct dynbuf *buffer)
{
    data_string_to_buf(state, &subCategory->sub_category_text, buffer);
}

NB_Error
data_coupon_sub_category_from_binary(data_util_state* state, data_coupon_sub_category* subCategory, byte** ppdata, size_t* pdatalen)
{
    return data_string_from_binary(state, &subCategory->sub_category_text, ppdata, pdatalen);
}

/*! @} */
