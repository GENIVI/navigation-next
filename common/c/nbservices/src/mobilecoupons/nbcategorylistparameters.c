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

    @file     nbcategorylistparameters.c
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
#include "nbmobilecouponsprivate.h"
#include "nbcategorylistparametersprivate.h"
#include "data_coupon_query.h"
#include "data_category_list_request.h"

struct NB_CategoryListParameters
{
    NB_Context*          context;
    data_coupon_query    wrapperQuery;
};

NB_DEF NB_Error
NB_CategoryListParametersCreate( NB_Context* context,
                                 const NB_CouponSearchRegion* position,
                                 const char* broadCategoryName,
                                 const char* categoryName,
                                 NB_CategoryListParameters** parameters)
{
    NB_CategoryListParameters* pThis = NULL;
    data_category_list_request* request = NULL;
    NB_Error err = NE_OK;

    if ( context == NULL || parameters == NULL )
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(NB_CategoryListParameters));
    if (pThis == NULL)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(NB_CategoryListParameters));

    pThis->context = context;

    err = err ? err : data_coupon_query_init(NB_ContextGetDataState(pThis->context), &pThis->wrapperQuery);

    if (err == NE_OK)
    {
        request = nsl_malloc(sizeof(data_category_list_request));
        err = (request != NULL) ? NE_OK : NE_NOMEM;
        pThis->wrapperQuery.category_list_request = request;
    }

    err = err ? err : data_category_list_request_init(NB_ContextGetDataState(pThis->context), request);
    if ( position != NULL )
    {
        err = err ? err : SetPositionFromSearchRegion( pThis->context, &request->position, position );
    }

    if ( broadCategoryName != NULL )
    {
        err = err ? err : data_string_set(NB_ContextGetDataState(pThis->context), 
                                &request->broad_category_name, broadCategoryName);
    }

    if ( categoryName != NULL )
    {
        err = err ? err : data_string_set(NB_ContextGetDataState(pThis->context), 
                                &request->category_name, categoryName);
    }

    if (err == NE_OK)
    {
        *parameters = pThis;
    }
    else
    {
        NB_CategoryListParametersDestroy(pThis);
    }

    return err;
}

NB_DEF NB_Error
NB_CategoryListParametersDestroy(NB_CategoryListParameters* parameters)
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
NB_CategoryListParametersToTPSQuery(NB_CategoryListParameters* parameters)
{
    if (parameters != NULL)
    {
        return data_coupon_query_to_tps(NB_ContextGetDataState(parameters->context), &parameters->wrapperQuery);
    }
    return NULL;
}

/*! @} */
