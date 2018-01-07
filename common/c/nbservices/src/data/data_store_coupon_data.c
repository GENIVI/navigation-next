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

    @file     data_store_coupon_data.c
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

#include "data_store_coupon_data.h"

NB_Error
data_store_coupon_data_init(data_util_state* state, data_store_coupon_data* storeCouponData)
{
    NB_Error err = NE_OK;
    DATA_MEM_ZERO(storeCouponData, data_store_coupon_data);

    err = err ? err : data_store_data_init(state, &storeCouponData->store_data);
    DATA_VEC_ALLOC(err, storeCouponData->coupon_data, data_coupon_data);

    return err;
}

void
data_store_coupon_data_free(data_util_state* state, data_store_coupon_data* storeCouponData)
{
    data_store_data_free(state, &storeCouponData->store_data);
    DATA_VEC_FREE(state, storeCouponData->coupon_data, data_coupon_data);
}

NB_Error
data_store_coupon_data_from_tps(data_util_state* state, data_store_coupon_data* storeCouponData, tpselt tpsElement)
{
    NB_Error err = NE_OK;
    tpselt childElement;
    int iteration = 0;
    if (tpsElement == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    data_store_coupon_data_free(state, storeCouponData);
    err = data_store_coupon_data_init(state, storeCouponData);
    if (err != NE_OK)
    {
        return err;
    }

    while (err == NE_OK && (childElement = te_nextchild(tpsElement, &iteration)) != NULL)
    {
        if (nsl_strcmp(te_getname(childElement), "store-data") == 0)
        {
            err = err ? err : data_store_data_from_tps(state, &storeCouponData->store_data, childElement);
        }
        else if (nsl_strcmp(te_getname(childElement), "coupon-data") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(state, err, childElement, storeCouponData->coupon_data, data_coupon_data);
        }
    }

errexit:
    if (err != NE_OK)
    {
        data_store_coupon_data_free(state, storeCouponData);
    }
    return err;
}

boolean
data_store_coupon_data_equal(data_util_state* state, data_store_coupon_data* left, data_store_coupon_data* right)
{
    int ret = TRUE;
    DATA_VEC_EQUAL(state, ret, left->coupon_data, right->coupon_data, data_coupon_data);

    return (boolean) ( ret == TRUE && data_store_data_equal(state, &left->store_data, &right->store_data) );
}

NB_Error
data_store_coupon_data_copy(data_util_state* state, data_store_coupon_data* dest, data_store_coupon_data* src)
{
    NB_Error err = NE_OK;

    data_store_coupon_data_free(state, dest);
    err = err ? err : data_store_coupon_data_init(state, dest);

    err = err ? err : data_store_data_copy(state, &dest->store_data, &src->store_data);
    DATA_VEC_COPY(state, err, dest->coupon_data, src->coupon_data, data_coupon_data);

    return err;
}

/*! @} */
