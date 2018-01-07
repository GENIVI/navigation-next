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

    @file     nbcouponrequestinformation.c
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

/*! @{ */

#include "nbcontextprotected.h"
#include "nbcouponrequestinformationprivate.h"
#include "nbmobilecouponsprivate.h"
#include "data_coupon_reply.h"
#include "data_coupon_query.h"

struct NB_CouponRequestInformation
{
    NB_Context*        context;
    data_coupon_reply  reply;
    data_coupon_query  query;
};


NB_DEF NB_Error
NB_CouponRequestInformationCreate(NB_Context* context, tpselt reply, data_coupon_query* query,
                                  NB_CouponRequestInformation** information)
{
    NB_CouponRequestInformation* pThis = NULL;
    NB_Error err = NE_OK;

    if (context == NULL || reply == NULL || query == NULL || information == NULL)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(NB_CouponRequestInformation));
    if (pThis == NULL)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(NB_CouponRequestInformation));

    err = data_coupon_reply_init(NB_ContextGetDataState(context), &pThis->reply);
    err = err ? err : data_coupon_reply_from_tps(NB_ContextGetDataState(context), &pThis->reply, reply);

    err = err ? err : data_coupon_query_copy(NB_ContextGetDataState(context), &pThis->query, query);

    if (err == NE_OK)
    {
        pThis->context = context;
        *information = pThis;
    }
    else
    {
        NB_CouponRequestInformationDestroy(pThis);
    }

    return err;
}

NB_DEF NB_Error
NB_CouponRequestInformationGetResultCount(NB_CouponRequestInformation* information, int32* count)
{
    data_coupon_response* response = NULL;
    if (information == NULL || information->reply.coupon_response == NULL ||
        count == NULL)
    {
        return NE_INVAL;
    }

    response = information->reply.coupon_response;
    *count = CSL_VectorGetLength(response->storeCoupons);

    return NE_OK;
}

NB_DEF NB_Error 
NB_CouponRequestInformationHasMoreResults(NB_CouponRequestInformation* information, nb_boolean* moreResults)
{
    if (information == NULL || information->reply.coupon_response == NULL ||
        moreResults == NULL)
    {
        return NE_INVAL;
    }

    *moreResults = !information->reply.coupon_response->iter_result.exhausted;

    return NE_OK;
}

NB_DEF NB_Error
NB_CouponRequestInformationGetStoreCoupon( NB_CouponRequestInformation* information, 
                                           int32 index, NB_StoreCoupons** storeCoupon )
{
    data_coupon_response* response = NULL;
    data_store_coupon_data* tmpCoupon = NULL;

    if (information == NULL || information->reply.coupon_response == NULL ||
        storeCoupon == NULL)
    {
        return NE_INVAL;
    }

    response = information->reply.coupon_response;
    if (index < 0 || index >= CSL_VectorGetLength(response->storeCoupons))
    {
        return NE_RANGE;
    }

    tmpCoupon = (data_store_coupon_data*)CSL_VectorGetPointer(response->storeCoupons, index);
    if (tmpCoupon == NULL)
    {
        return NE_NOENT;
    }

    return CreateStoreCoupon( information->context, storeCoupon, tmpCoupon );
}

NB_DEF NB_Error
NB_CouponRequestInformationFreeStoreCouponData(NB_StoreCoupons* storeCoupon)
{
    if (storeCoupon == NULL)
    {
        return NE_INVAL;
    }

    DestroyStoreCoupon(storeCoupon);
    return NE_OK;
}

NB_DEF NB_Error
NB_CouponRequestInformationDestroy(NB_CouponRequestInformation* information)
{
    if (information == NULL)
    {
        return NE_INVAL;
    }

    data_coupon_reply_free(NB_ContextGetDataState(information->context), &information->reply);
    data_coupon_query_free(NB_ContextGetDataState(information->context), &information->query);
    nsl_free(information);

    return NE_OK;
}

data_blob*
NB_CouponRequestInformationGetIterationState(NB_CouponRequestInformation* information)
{
    if (information == NULL || information->reply.coupon_response == NULL)
    {
        return NULL;
    }

    return &information->reply.coupon_response->iter_result.state;
}

data_coupon_query*
NB_CouponRequestInformationGetQuery(NB_CouponRequestInformation* information)
{
    if (information == NULL)
    {
        return NULL;
    }

    return &information->query;
}

/*! @} */
