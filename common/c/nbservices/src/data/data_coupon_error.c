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

    @file     data_coupon_error.c
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

#include "data_coupon_error.h"

NB_Error
data_coupon_error_init(data_util_state* state, data_coupon_error* couponError)
{
    DATA_MEM_ZERO(couponError, data_coupon_error);

    couponError->code = 0;
    return NE_OK;
}

void
data_coupon_error_free(data_util_state* state, data_coupon_error* couponError)
{
    couponError->code = 0;
}

NB_Error
data_coupon_error_from_tps(data_util_state* state, data_coupon_error* couponError, tpselt tpsElement)
{
    NB_Error err = NE_OK;
    if (tpsElement == NULL)
    {
        return NE_INVAL;
    }

    data_coupon_error_free(state, couponError);
    err = data_coupon_error_init(state, couponError);
    if (err != NE_OK)
    {
        return err;
    }

    couponError->code = te_getattru(tpsElement, "code");
    return NE_OK;
}

tpselt
data_coupon_error_to_tps(data_util_state* state, data_coupon_error* couponError)
{
    tpselt tpsElement;
    tpsElement = te_new("coupon-error");

    if (tpsElement == NULL)
    {
        goto errexit;
    }
    if (!te_setattru(tpsElement, "code", couponError->code))
    {
        goto errexit;
    }

    return tpsElement;

errexit:

    te_dealloc(tpsElement);
    return NULL;
}

boolean
data_coupon_error_equal(data_util_state* state, data_coupon_error* left, data_coupon_error* right)
{
    return (boolean)(left->code == right->code);
}

NB_Error
data_coupon_error_copy(data_util_state* state, data_coupon_error* dest, data_coupon_error* src)
{
    NB_Error err = NE_OK;

    data_coupon_error_free(state, dest);
    err = data_coupon_error_init(state, dest);

    dest->code = src->code;
    return err;
}

/*! @} */
