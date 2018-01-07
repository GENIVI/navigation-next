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

    @file     data_coupon_broad_category.c
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

#include "data_coupon_broad_category.h"

NB_Error
data_coupon_broad_category_init(data_util_state* state, data_coupon_broad_category* broadCategory)
{
    NB_Error err = NE_OK;
    DATA_MEM_ZERO(broadCategory, data_coupon_broad_category);
    err = err ? err : data_string_init(state, &broadCategory->broad_category_text);
    return err;
}

void
data_coupon_broad_category_free(data_util_state* state, data_coupon_broad_category* broadCategory)
{
    data_string_free(state, &broadCategory->broad_category_text);
}

NB_Error
data_coupon_broad_category_from_tps(data_util_state* state, data_coupon_broad_category* broadCategory, tpselt tpsElement)
{
    NB_Error err = NE_OK;
    if (tpsElement == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    data_coupon_broad_category_free(state, broadCategory);
    err = data_coupon_broad_category_init(state, broadCategory);
    if (err != NE_OK)
    {
        return err;
    }

    err = err ? err : data_string_from_tps_attr(state, &broadCategory->broad_category_text, tpsElement, "coupon-broad-category-text");

errexit:
    if (err != NE_OK)
    {
        data_coupon_broad_category_free(state, broadCategory);
    }
    return err;
}

tpselt
data_coupon_broad_category_to_tps(data_util_state* state, data_coupon_broad_category* broadCategory)
{
    tpselt tpsElement = te_new("coupon-broad-category");
    if (tpsElement == NULL)
    {
        goto errexit;
    }

    if (!te_setattrc(tpsElement, "coupon-broad-category-text", data_string_get(state, &broadCategory->broad_category_text)))
    {
        goto errexit;
    }

    return tpsElement;

errexit:
    te_dealloc(tpsElement);
    return NULL;
}

boolean
data_coupon_broad_category_equal(data_util_state* state, data_coupon_broad_category* left, data_coupon_broad_category* right)
{
    return (boolean) data_string_equal(state, &left->broad_category_text, &right->broad_category_text);
}

NB_Error
data_coupon_broad_category_copy(data_util_state* state, data_coupon_broad_category* dest, data_coupon_broad_category* src)
{
    NB_Error err = NE_OK;
    data_coupon_broad_category_free(state, dest);
    err = err ? err : data_coupon_broad_category_init(state, dest);

    err = err ? err : data_string_copy(state, &dest->broad_category_text, &src->broad_category_text);

    return err;
}

uint32
data_coupon_broad_category_get_tps_size(data_util_state* state, data_coupon_broad_category* broadCategory)
{
    return data_string_get_tps_size(state, &broadCategory->broad_category_text);
}

void
data_coupon_broad_category_to_buf(data_util_state* state, data_coupon_broad_category* broadCategory, struct dynbuf *buffer)
{
    data_string_to_buf(state, &broadCategory->broad_category_text, buffer);
}

NB_Error
data_coupon_broad_category_from_binary(data_util_state* state, data_coupon_broad_category* broadCategory, byte** ppdata, size_t* pdatalen)
{
    return data_string_from_binary(state, &broadCategory->broad_category_text, ppdata, pdatalen);
}

/*! @} */
