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

    @file     nbstorerequestinformation.c
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
#include "nbstorerequestinformationprivate.h"
#include "nbmobilecouponsprivate.h"
#include "data_coupon_reply.h"
#include "data_coupon_query.h"

struct NB_StoreRequestInformation
{
    NB_Context*        context;
    data_coupon_reply  reply;
    data_coupon_query  query;
};


NB_DEF NB_Error
NB_StoreRequestInformationCreate(NB_Context* context, tpselt reply, data_coupon_query* query,
                                 NB_StoreRequestInformation** information)
{
    NB_StoreRequestInformation* pThis = NULL;
    NB_Error err = NE_OK;

    if (context == NULL || reply == NULL || query == NULL || information == NULL)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(NB_StoreRequestInformation));
    if (pThis == NULL)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(NB_StoreRequestInformation));

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
        NB_StoreRequestInformationDestroy(pThis);
    }

    return err;
}

NB_DEF NB_Error
NB_StoreRequestInformationGetResultCount(NB_StoreRequestInformation* information, int32* count)
{
    data_store_response* response = NULL;
    if (information == NULL || information->reply.store_response == NULL ||
        count == NULL)
    {
        return NE_INVAL;
    }

    response = information->reply.store_response;
    *count = CSL_VectorGetLength(response->stores);

    return NE_OK;
}

NB_DEF NB_Error 
NB_StoreRequestInformationHasMoreResults(NB_StoreRequestInformation* information, nb_boolean* moreResults)
{
    if (information == NULL || information->reply.store_response == NULL ||
        moreResults == NULL)
    {
        return NE_INVAL;
    }

    *moreResults = !information->reply.store_response->iter_result.exhausted;

    return NE_OK;
}

NB_DEF NB_Error
NB_StoreRequestInformationGetStore(NB_StoreRequestInformation* information, int32 index, NB_Store** store)
{
    data_store_response* response = NULL;
    data_store_data* tmpStore = NULL;

    if (information == NULL || information->reply.store_response == NULL ||
        store == NULL)
    {
        return NE_INVAL;
    }

    response = information->reply.store_response;
    if (index < 0 || index >= CSL_VectorGetLength(response->stores))
    {
        return NE_RANGE;
    }

    tmpStore = (data_store_data*)CSL_VectorGetPointer(response->stores, index);
    if (tmpStore == NULL)
    {
        return NE_NOENT;
    }

    return CreateStore(information->context, store, tmpStore);
}

NB_DEF NB_Error
NB_StoreRequestInformationFreeStoreData(NB_Store* store)
{
    if (store == NULL)
    {
        return NE_INVAL;
    }

    DestroyStore(store);
    return NE_OK;
}

NB_DEF NB_Error
NB_StoreRequestInformationDestroy(NB_StoreRequestInformation* information)
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
NB_StoreRequestInformationGetIterationState(NB_StoreRequestInformation* information)
{
    if (information == NULL || information->reply.store_response == NULL)
    {
        return NULL;
    }

    return &information->reply.store_response->iter_result.state;
}

data_coupon_query*
NB_StoreRequestInformationGetQuery(NB_StoreRequestInformation* information)
{
    if (information == NULL)
    {
        return NULL;
    }

    return &information->query;
}

/*! @} */
