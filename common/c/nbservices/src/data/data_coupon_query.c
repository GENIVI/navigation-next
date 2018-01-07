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

    @file     data_coupon_query.c
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

#include "data_coupon_query.h"


#define REQUEST_COPY(STATE, ERR, DEST_PTR, SRC_PTR, DATA_TYPE)         \
    do {                                                               \
        DEST_PTR = nsl_malloc(sizeof(DATA_TYPE));                      \
        ERR = (DEST_PTR != NULL) ? NE_OK : NE_NOMEM;                   \
        ERR = ERR ? ERR : DATA_TYPE##_init(STATE, DEST_PTR);           \
        ERR = ERR ? ERR : DATA_TYPE##_copy(STATE, DEST_PTR, SRC_PTR);  \
    } while (0)


NB_Error
data_coupon_query_init(data_util_state* state, data_coupon_query* couponQuery)
{
    DATA_MEM_ZERO(couponQuery, data_coupon_query);
    return NE_OK;
}

void
data_coupon_query_free(data_util_state* state, data_coupon_query* couponQuery)
{
    DATA_PTR_FREEIF(state, couponQuery->store_request, data_store_request);
    DATA_PTR_FREEIF(state, couponQuery->coupon_request, data_coupon_request);
    DATA_PTR_FREEIF(state, couponQuery->load_image_request, data_load_image_request);
    DATA_PTR_FREEIF(state, couponQuery->category_list_request, data_category_list_request);
}

tpselt
data_coupon_query_to_tps(data_util_state* state, data_coupon_query* couponQuery)
{
    tpselt tpsElement = te_new("coupon-query");
    if (tpsElement == NULL)
    {
        goto errexit;
    }

    if (couponQuery->store_request != NULL)
    {
        DATA_TO_TPS(state, errexit, tpsElement, couponQuery->store_request, data_store_request);
    }
    else if (couponQuery->coupon_request != NULL)
    {
        DATA_TO_TPS(state, errexit, tpsElement, couponQuery->coupon_request, data_coupon_request);
    }
    else if (couponQuery->load_image_request != NULL)
    {
        DATA_TO_TPS(state, errexit, tpsElement, couponQuery->load_image_request, data_load_image_request);
    }
    else if (couponQuery->category_list_request != NULL)
    {
        DATA_TO_TPS(state, errexit, tpsElement, couponQuery->category_list_request, data_category_list_request);
    }
    else
    {
        goto errexit;
    }

    return tpsElement;

errexit:

    te_dealloc(tpsElement);
    return NULL;
}

NB_Error
data_coupon_query_copy(data_util_state* state, data_coupon_query* dest, data_coupon_query* src)
{
    NB_Error err = NE_OK;

    data_coupon_query_free(state, dest);
    err = err ? err : data_coupon_query_init(state, dest);

    if (src->store_request != NULL)
    {
        REQUEST_COPY(state, err, dest->store_request, src->store_request, data_store_request);
    }
    else if (src->coupon_request != NULL)
    {
        REQUEST_COPY(state, err, dest->coupon_request, src->coupon_request, data_coupon_request);
    }
    else if (src->load_image_request != NULL)
    {
        REQUEST_COPY(state, err, dest->load_image_request, src->load_image_request, data_load_image_request);
    }
    else if (src->category_list_request != NULL)
    {
        REQUEST_COPY(state, err, dest->category_list_request, src->category_list_request, data_category_list_request);
    }
    return err;
}

/*! @} */
