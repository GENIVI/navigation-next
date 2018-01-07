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

    @file     nbcategorylistinformation.c
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
#include "nbcategorylistinformationprivate.h"
#include "data_coupon_reply.h"
#include "data_coupon_sub_category.h"
#include "data_coupon_category.h"
#include "data_coupon_broad_category.h"

struct NB_CategoryListInformation
{
    NB_Context*        context;
    data_coupon_reply  reply;
};


NB_DEF NB_Error
NB_CategoryListInformationCreate(NB_Context* context, tpselt reply, NB_CategoryListInformation** information)
{
    NB_CategoryListInformation* pThis = NULL;
    NB_Error err = NE_OK;

    if (context == NULL || reply == NULL || information == NULL)
    {
        return NE_INVAL;
    }

    pThis = nsl_malloc(sizeof(NB_CategoryListInformation));
    if (pThis == NULL)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(NB_CategoryListInformation));

    err = data_coupon_reply_init(NB_ContextGetDataState(context), &pThis->reply);
    err = err ? err : data_coupon_reply_from_tps(NB_ContextGetDataState(context),
                                                 &pThis->reply, reply);
    if (err == NE_OK)
    {
        pThis->context = context;
        *information = pThis;
    }
    else
    {
        NB_CategoryListInformationDestroy(pThis);
    }

    return err;
}

NB_DEF NB_Error
NB_CategoryListInformationGetBroadCategoryItem(
                NB_CategoryListInformation* information,
                int32 index,
                char const** broadCategory )
{
    NB_Error err = NE_OK;
    data_category_list_response* response = NULL;
    data_coupon_broad_category* item = NULL;

    if ( information == NULL || information->reply.category_list_response == NULL ||
         broadCategory == NULL || *broadCategory != NULL)
    {
        return NE_INVAL;
    }

    response = information->reply.category_list_response;
    if ( index < 0 || index >= CSL_VectorGetLength( response->coupon_broad_categories ) )
    {
        return NE_RANGE;
    }

    item = ( data_coupon_broad_category* )CSL_VectorGetPointer( response->coupon_broad_categories, index );
    *broadCategory = data_string_get(NB_ContextGetDataState(information->context), &item->broad_category_text);

    return err;
}

NB_DEF NB_Error
NB_CategoryListInformationGetBroadCategoryCount(NB_CategoryListInformation* information, int32* count)
{
     if ( information == NULL || information->reply.category_list_response == NULL ||
         count == NULL )
    {
        return NE_INVAL;
    }

    *count = CSL_VectorGetLength( information->reply.category_list_response->coupon_broad_categories );
    return NE_OK;
}


NB_DEF NB_Error
NB_CategoryListInformationGetCategoryItem(
                NB_CategoryListInformation* information,
                int32 index,
                char const** category )
{
    data_category_list_response* response = NULL;
    data_coupon_category* item = NULL;


    if ( information == NULL || information->reply.category_list_response == NULL ||
         category == NULL || *category != NULL )
    {
        return NE_INVAL;
    }

    response = information->reply.category_list_response;
    if ( index < 0 || index >= CSL_VectorGetLength( response->coupon_categories ) )
    {
        return NE_RANGE;
    }

    item = ( data_coupon_category* )CSL_VectorGetPointer( response->coupon_categories, index );
    *category = data_string_get(NB_ContextGetDataState(information->context), &item->category_text);

    return NE_OK;
}


NB_DEF NB_Error
NB_CategoryListInformationGetCategoryCount(NB_CategoryListInformation* information,  int32* count )
{
    if ( information == NULL || information->reply.category_list_response == NULL ||
         count == NULL )
    {
        return NE_INVAL;
    }

    *count = CSL_VectorGetLength( information->reply.category_list_response->coupon_categories );
    return NE_OK;
}

NB_DEF NB_Error
NB_CategoryListInformationGetSubCategoryItem(
                NB_CategoryListInformation* information,
                int32 index,
                char const** subCategory )
{
    data_category_list_response* response = NULL;
    data_coupon_sub_category* item = NULL;

    if ( information == NULL || information->reply.category_list_response == NULL ||
         subCategory == NULL || *subCategory != NULL)
    {
        return NE_INVAL;
    }

    response = information->reply.category_list_response;
    if ( index < 0 || index >= CSL_VectorGetLength( response->coupon_sub_categories ) )
    {
        return NE_RANGE;
    }

    item = ( data_coupon_sub_category* )CSL_VectorGetPointer( response->coupon_sub_categories, index );
    *subCategory = data_string_get(NB_ContextGetDataState(information->context), &item->sub_category_text);

    return NE_OK;
}

NB_DEF NB_Error
NB_CategoryListInformationGetSubCategoryCount(NB_CategoryListInformation* information, int32* count )
{
    if ( information == NULL || information->reply.category_list_response == NULL ||
         count == NULL )
    {
        return NE_INVAL;
    }

    *count = CSL_VectorGetLength( information->reply.category_list_response->coupon_sub_categories );
    return NE_OK;
}

NB_DEF NB_Error
NB_CategoryListInformationDestroy(NB_CategoryListInformation* information)
{
    if (information == NULL)
    {
        return NE_INVAL;
    }

    data_coupon_reply_free(NB_ContextGetDataState(information->context), &information->reply);
    nsl_free(information);

    return NE_OK;
}

/*! @} */
