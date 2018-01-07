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

    @file     data_coupon_reply.c
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

#include "data_coupon_reply.h"


#define DATA_CREATE_FROM_TPS(STATE, ERR, PTR, ELEM, DATA_TYPE)         \
    do {                                                               \
        PTR = nsl_malloc(sizeof(DATA_TYPE));                           \
        ERR = (PTR != NULL) ? NE_OK : NE_NOMEM;                        \
        ERR = ERR ? ERR : DATA_TYPE##_init(STATE, PTR);                \
        ERR = ERR ? ERR : DATA_TYPE##_from_tps(STATE, PTR, ELEM);      \
    } while (0)


NB_Error
data_coupon_reply_init(data_util_state* state, data_coupon_reply* couponReply)
{
    DATA_MEM_ZERO(couponReply, data_coupon_reply);
    return NE_OK;
}

void
data_coupon_reply_free(data_util_state* state, data_coupon_reply* couponReply)
{
    DATA_PTR_FREEIF(state, couponReply->store_response, data_store_response);
    DATA_PTR_FREEIF(state, couponReply->coupon_response, data_coupon_response);
    DATA_PTR_FREEIF(state, couponReply->load_image_error, data_load_image_error);
    DATA_PTR_FREEIF(state, couponReply->load_image_response, data_load_image_response);
    DATA_PTR_FREEIF(state, couponReply->category_list_response, data_category_list_response);
}

NB_Error
data_coupon_reply_from_tps(data_util_state* state, data_coupon_reply* couponReply, tpselt tpsElement)
{
    NB_Error err = NE_OK;
    tpselt childElement;
    int iteration = 0;
    if (tpsElement == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    data_coupon_reply_free(state, couponReply);
    err = data_coupon_reply_init(state, couponReply);
    if (err != NE_OK)
    {
        return err;
    }

    while (err == NE_OK && (childElement = te_nextchild(tpsElement, &iteration)) != NULL)
    {
        if (nsl_strcmp(te_getname(childElement), "store-response") == 0)
        {
            DATA_CREATE_FROM_TPS(state, err, couponReply->store_response, childElement, data_store_response);
        }
        else if (nsl_strcmp(te_getname(childElement), "coupon-response") == 0)
        {
            DATA_CREATE_FROM_TPS(state, err, couponReply->coupon_response, childElement, data_coupon_response);
        }
        else if (nsl_strcmp(te_getname(childElement), "coupon-error") == 0)
        {
            DATA_CREATE_FROM_TPS(state, err, couponReply->coupon_error, childElement, data_coupon_error);
        }
        else if (nsl_strcmp(te_getname(childElement), "load-image-error") == 0)
        {
            DATA_CREATE_FROM_TPS(state, err, couponReply->load_image_error, childElement, data_load_image_error);
        }
        else if (nsl_strcmp(te_getname(childElement), "load-image-response") == 0)
        {
            DATA_CREATE_FROM_TPS(state, err, couponReply->load_image_response, childElement, data_load_image_response);
        }
        else if (nsl_strcmp(te_getname(childElement), "category-list-response") == 0)
        {
            DATA_CREATE_FROM_TPS(state, err, couponReply->category_list_response, childElement, data_category_list_response);
        }
    }

errexit:
    if (err != NE_OK)
    {
        data_coupon_reply_free(state, couponReply);
    }
    return err;
}

NB_Error
data_coupon_reply_from_tps_error(data_util_state* state, NB_Error* serverError, tpselt tpsElement)
{
    NB_Error err = NE_OK;
    tpselt childElement;
    int iteration = 0;
    data_coupon_error couponError = {0};
    data_load_image_error imageError = {0};

    if (tpsElement == NULL || serverError == NULL)
    {
        return NE_INVAL;
    }

    *serverError = NE_OK;
    err = data_coupon_error_init(state, &couponError);
    err = err ? err : data_load_image_error_init(state, &imageError);

    while (err == NE_OK && (childElement = te_nextchild(tpsElement, &iteration)) != NULL)
    {
        if (nsl_strcmp(te_getname(childElement), "coupon-error") == 0)
        {
            err = err ? err : data_coupon_error_from_tps(state, &couponError, childElement);
            *serverError = (err == NE_OK) ? couponError.code : NE_OK;
        }
        if (nsl_strcmp(te_getname(childElement), "load-image-error") == 0)
        {
            err = err ? err : data_load_image_error_from_tps(state, &imageError, childElement);
            *serverError = (err == NE_OK) ? imageError.code : NE_OK;
        }
    }

    return err;
}

/*! @} */
