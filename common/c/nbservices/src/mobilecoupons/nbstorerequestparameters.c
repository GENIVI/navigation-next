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

    @file     nbstorerequestparameters.c
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
#include "nbstorerequestparametersprivate.h"
#include "nbstorerequestinformationprivate.h"
#include "data_store_request.h"
#include "data_coupon_query.h"
#include "nbsearchfilterprivate.h"
#include "nbmobilecouponsprivate.h"

struct NB_StoreRequestParameters
{
    NB_Context*          context;
    data_coupon_query    wrapperQuery;
};

NB_DEF NB_Error
NB_StoreRequestParametersCreate(NB_Context* context,
                                const NB_CouponSearchRegion* searchRegion,
                                uint32 sliceSize,
                                NB_StoreRequestParameters** parameters)
{
    NB_StoreRequestParameters* pThis = NULL;
    data_store_request* request = NULL;
    NB_Error err = NE_OK;

    if (context == NULL || searchRegion == NULL || parameters == NULL)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(NB_StoreRequestParameters));
    if (pThis == NULL)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(NB_StoreRequestParameters));

    pThis->context = context;

    err = err ? err : data_coupon_query_init(NB_ContextGetDataState(pThis->context), &pThis->wrapperQuery);
    if (err == NE_OK)
    {
        request = nsl_malloc(sizeof(data_store_request));
        if (!request)
        {
            NB_StoreRequestParametersDestroy(pThis);
            return NE_NOMEM;
        }
        pThis->wrapperQuery.store_request = request;
    }

    err = err ? err : data_store_request_init(NB_ContextGetDataState(pThis->context), request);
    err = err ? err : SetPositionFromSearchRegion(pThis->context, &request->position, searchRegion);

    err = err ? err : SetIterationCommand(NB_ContextGetDataState(pThis->context), &request->iter_command, NB_IterationCommand_Start);
    request->iter_command.number = sliceSize;

    if (err == NE_OK)
    {
        *parameters = pThis;
    }
    else
    {
        NB_StoreRequestParametersDestroy(pThis);
    }

    return err;
}

NB_DEF NB_Error
NB_StoreRequestParametersSetSearchFilter(NB_StoreRequestParameters* parameters,
                                         NB_SearchFilter *filter)
{
    if ( parameters == NULL || filter == NULL )
    {
        return NE_INVAL;
    }

    return NB_SearchFilterSetFilter( filter, &parameters->wrapperQuery.store_request->search_filter.vec_pairs );
}

NB_DEF NB_Error
NB_StoreRequestParametersCreateIteration(NB_Context* context,
                                         NB_StoreRequestInformation* information,
                                         NB_IterationCommand iterationCommand,
                                         NB_StoreRequestParameters** parameters)
{
    NB_StoreRequestParameters* pThis = NULL;
    data_store_request* request = NULL;
    data_coupon_query* previousQuery = NB_StoreRequestInformationGetQuery(information);
    NB_Error err = NE_OK;

    if (context == NULL || information == NULL || parameters == NULL || previousQuery == NULL)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(NB_StoreRequestParameters));
    if (pThis == NULL)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(NB_StoreRequestParameters));

    pThis->context = context;

    err = err ? err : data_coupon_query_init(NB_ContextGetDataState(pThis->context), &pThis->wrapperQuery);
    err = err ? err : data_coupon_query_copy(NB_ContextGetDataState(pThis->context), &pThis->wrapperQuery, previousQuery);
    request = pThis->wrapperQuery.store_request;
    if (err == NE_OK && request != NULL)
    {
        err = err ? err : SetIterationCommand(NB_ContextGetDataState(pThis->context), &request->iter_command, iterationCommand);
        // if this is not the first iteration, copy state from the server reply
        if (iterationCommand != NB_IterationCommand_Start) 
        {
            err = err ? err : data_blob_copy(NB_ContextGetDataState(pThis->context), 
                                             &request->iter_command.state, 
                                             NB_StoreRequestInformationGetIterationState(information));
        }
    }

    if (err == NE_OK)
    {
        *parameters = pThis;
    }
    else
    {
        NB_StoreRequestParametersDestroy(pThis);
    }

    return err;
}

NB_DEF NB_Error
NB_StoreRequestParametersDestroy(NB_StoreRequestParameters* parameters)
{
    if (parameters == NULL)
    {
        return NE_INVAL;
    }

    data_coupon_query_free(NB_ContextGetDataState(parameters->context), &parameters->wrapperQuery);
    nsl_free(parameters);

    return NE_OK;
}

tpselt
NB_StoreRequestParametersToTPSQuery(NB_StoreRequestParameters* parameters)
{
    if (parameters != NULL)
    {
        return data_coupon_query_to_tps(NB_ContextGetDataState(parameters->context), &parameters->wrapperQuery);
    }
    return NULL;
}

NB_Error
NB_StoreRequestParametersCopyQuery(NB_StoreRequestParameters* parameters, data_coupon_query* query)
{
    if (parameters == NULL || query == NULL)
    {
        return NE_INVAL;
    }

    return data_coupon_query_copy(NB_ContextGetDataState(parameters->context), query, &parameters->wrapperQuery);
}

/*! @} */
