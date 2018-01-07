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

    @file     data_store_data.c
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

#include "data_store_data.h"
#include "data_coupon_broad_category.h"
#include "data_coupon_sub_category.h"
#include "data_coupon_category.h"

#define DATA_VEC_FROM_ARRAY(PDS, ERR, VEC, ARR, ARR_SIZE, DATA_TYPE)        \
    do {                                                                    \
        if (ERR == NE_OK) {                                                 \
           int n = 0;                                                       \
           DATA_TYPE tmp;                                                   \
           for (n=0; n<ARR_SIZE && ERR == NE_OK; n++) {                     \
               ERR = ERR ? ERR : DATA_TYPE##_init(PDS, &tmp);               \
               ERR = ERR ? ERR : data_string_set(PDS, &tmp, CSL_VectorGetPointer(VECSRC, n)); \
               ERR = ERR ? ERR : CSL_VectorAppend(VECDST, &tmp) ? NE_OK : NE_NOMEM; \
               if (ERR)                                                     \
                   DATA_TYPE##_free(PDS, &tmp);                             \
            }                                                               \
        }                                                                   \
    } while (0)


NB_Error
data_store_data_init(data_util_state* state, data_store_data* storeData)
{
    NB_Error err = NE_OK;
    DATA_MEM_ZERO(storeData, data_store_data);

    err = err ? err : data_place_init(state, &storeData->place);

    err = err ? err : data_string_init(state, &storeData->retailer_name);
    err = err ? err : data_string_init(state, &storeData->retailer_id);
    storeData->coupon_count = 0;

    DATA_VEC_ALLOC(err, storeData->broad_categories, data_coupon_broad_category);
    DATA_VEC_ALLOC(err, storeData->sub_categories, data_coupon_sub_category);
    DATA_VEC_ALLOC(err, storeData->categories, data_coupon_category);
    err = err ? err : data_image_urls_init( state, &storeData->image_urls );

    return err;
}

void
data_store_data_free(data_util_state* state, data_store_data* storeData)
{
    data_place_free(state, &storeData->place);

    data_string_free(state, &storeData->retailer_name);
    data_string_free(state, &storeData->retailer_id);
    storeData->coupon_count = 0;

    DATA_VEC_FREE(state, storeData->broad_categories, data_coupon_broad_category);
    DATA_VEC_FREE(state, storeData->sub_categories, data_coupon_sub_category);
    DATA_VEC_FREE(state, storeData->categories, data_coupon_category);
    data_image_urls_free( state, &storeData->image_urls);
}

NB_Error
data_store_data_from_tps(data_util_state* state, data_store_data* storeData, tpselt tpsElement)
{
    NB_Error err = NE_OK;
    tpselt childElement;
    int iteration = 0;
    if (tpsElement == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    data_store_data_free(state, storeData);
    err = data_store_data_init(state, storeData);
    if (err != NE_OK)
    {
        return err;
    }

    err = err ? err : data_string_from_tps_attr(state, &storeData->retailer_name, tpsElement, "retailer-name");
    err = err ? err : data_string_from_tps_attr(state, &storeData->retailer_id, tpsElement, "retailer-id");
    storeData->coupon_count = te_getattru( tpsElement, "coupon-count");

    while (err == NE_OK && (childElement = te_nextchild(tpsElement, &iteration)) != NULL)
    {
        if (nsl_strcmp(te_getname(childElement), "place") == 0)
        {
            err = err ? err : data_place_from_tps(state, &storeData->place, childElement);
        }
        else if (nsl_strcmp(te_getname(childElement), "coupon-broad-category") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(state, err, childElement, storeData->broad_categories, data_coupon_broad_category);
        }
        else if (nsl_strcmp(te_getname(childElement), "coupon-sub-category") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(state, err, childElement, storeData->sub_categories, data_coupon_sub_category);
        }
        else if (nsl_strcmp(te_getname(childElement), "coupon-category") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(state, err, childElement, storeData->categories, data_coupon_category);
        }
        else if ( childElement && ( nsl_stricmp( te_getname( childElement ), "image-urls" ) == 0 )  )
        {
            DATA_FROM_TPS( state, err, childElement, &storeData->image_urls, data_image_urls );
        }
 
    }

errexit:
    if (err != NE_OK)
    {
        data_store_data_free(state, storeData);
    }
    return err;
}

tpselt
data_store_data_to_tps(data_util_state* state, data_store_data* storeData)
{
    tpselt tpsElement;
    tpselt childElement = NULL;

    tpsElement = te_new("store-data");
    if (tpsElement == NULL)
    {
        goto errexit;
    }

    if (!te_setattrc(tpsElement, "retailer-name", data_string_get(state, &storeData->retailer_name)))
    {
        goto errexit;
    }
    if (!te_setattrc(tpsElement, "retailer-id", data_string_get(state, &storeData->retailer_id)))
    {
        goto errexit;
    }
    if (!te_setattru(tpsElement, "coupon-count", storeData->coupon_count))
    {
        goto errexit;
    }

    // this field should be optional, so check if it is valid
    if ( storeData->place.location.point.lat != INVALID_LATITUDE && storeData->place.location.point.lon != INVALID_LONGITUDE )
    {
        if ((childElement = data_place_to_tps(state, &storeData->place)) != NULL && te_attach(tpsElement, childElement))
        {
            childElement = NULL;
        }
        else
        {
            goto errexit;
        }
    }

    if (storeData->broad_categories != NULL)
    {
        DATA_VEC_TO_TPS(state, errexit, tpsElement, storeData->broad_categories, data_coupon_broad_category);
    }
    if (storeData->sub_categories != NULL)
    {
        DATA_VEC_TO_TPS(state, errexit, tpsElement, storeData->sub_categories, data_coupon_sub_category);
    }
    if (storeData->categories != NULL)
    {
        DATA_VEC_TO_TPS(state, errexit, tpsElement, storeData->categories, data_coupon_category);
    }

    DATA_TO_TPS( state, errexit, tpsElement, &storeData->image_urls, data_image_urls );

    return tpsElement;

errexit:

    te_dealloc(tpsElement);
    te_dealloc(childElement);
    return NULL;
}

boolean
data_store_data_equal(data_util_state* state, data_store_data* left, data_store_data* right)
{
    int ret = TRUE;
    DATA_VEC_EQUAL(state, ret, left->broad_categories, right->broad_categories, data_coupon_broad_category);
    DATA_VEC_EQUAL(state, ret, left->sub_categories, right->sub_categories, data_coupon_sub_category);
    DATA_VEC_EQUAL(state, ret, left->categories, right->categories, data_coupon_category);

    return (boolean) ( ret == TRUE &&
                       data_string_equal(state, &left->retailer_name, &right->retailer_name) &&
                       data_string_equal(state, &left->retailer_id, &right->retailer_id) &&
                       left->coupon_count == right->coupon_count &&
                       data_place_equal(state, &left->place, &right->place) &&
                       data_image_urls_equal(state, &left->image_urls, &right->image_urls) );
}

NB_Error
data_store_data_copy(data_util_state* state, data_store_data* dest, data_store_data* src)
{
    NB_Error err = NE_OK;

    data_store_data_free(state, dest);
    err = err ? err : data_store_data_init(state, dest);

    err = err ? err : data_string_copy(state, &dest->retailer_name, &src->retailer_name);
    err = err ? err : data_string_copy(state, &dest->retailer_id, &src->retailer_id);
    dest->coupon_count = src->coupon_count;
    err = err ? err : data_place_copy(state, &dest->place, &src->place);

    DATA_VEC_COPY(state, err, dest->broad_categories, src->broad_categories, data_coupon_broad_category);
    DATA_VEC_COPY(state, err, dest->sub_categories, src->sub_categories, data_coupon_sub_category);
    DATA_VEC_COPY(state, err, dest->categories, src->categories, data_coupon_category);
    err = err ? err : data_image_urls_copy(state, &dest->image_urls, &src->image_urls);

    return err;
}

NB_Error
data_store_data_from_nbstore(data_util_state* state, data_store_data* storeData, const NB_Store* store)
{
    NB_Error err = NE_OK;
    uint32 i = 0;
    if (store == NULL)
    {
        return NE_INVAL;
    }

    data_store_data_free(state, storeData);
    err = err ? err : data_store_data_init(state, storeData);

    err = err ? err : data_place_from_nimplace(state, &storeData->place, &store->place);
    err = err ? err : data_string_set(state, &storeData->retailer_id, store->id);
    err = err ? err : data_string_set(state, &storeData->retailer_name, store->name);
    storeData->coupon_count = store->count;

    for ( i = 0; i < store->broadCategoriesNumber && err == NE_OK; i++ )
    {
        data_coupon_broad_category tmp = {0};
        err = err ? err : data_coupon_broad_category_init( state, &tmp );
        err = err ? err : data_string_set( state, &tmp.broad_category_text, store->broadCategories[i] );
        err = err ? err : CSL_VectorAppend( storeData->broad_categories, &tmp) ? NE_OK : NE_NOMEM;
        if ( err )
        {
           data_coupon_broad_category_free(state, &tmp);
        }
        
    }

    for ( i = 0; i < store->categoriesNumber && err == NE_OK; i++ )
    {
        data_coupon_category tmp = {0};
        err = err ? err : data_coupon_category_init( state, &tmp );
        err = err ? err : data_string_set( state, &tmp.category_text, store->categories[i] );
        err = err ? err : CSL_VectorAppend( storeData->categories, &tmp) ? NE_OK : NE_NOMEM;
        if ( err )
        {
           data_coupon_category_free(state, &tmp);
        }
        
    }    

    for ( i = 0; i < store->subCategoriesNumber && err == NE_OK; i++ )
    {
        data_coupon_sub_category tmp = {0};
        err = err ? err : data_coupon_sub_category_init( state, &tmp );
        err = err ? err : data_string_set( state, &tmp.sub_category_text, store->subCategories[i] );
        err = err ? err : CSL_VectorAppend( storeData->sub_categories, &tmp) ? NE_OK : NE_NOMEM;
        if ( err )
        {
           data_coupon_sub_category_free(state, &tmp);
        }
        
    }

    err = err ? err : data_string_set(state, &storeData->image_urls.image_url, store->imageUrl);
    err = err ? err : data_string_set(state, &storeData->image_urls.small_thumb_url, store->smallThumbUrl);
    err = err ? err : data_string_set(state, &storeData->image_urls.medium_thumb_url, store->mediumThumbUrl);
    err = err ? err : data_string_set(state, &storeData->image_urls.large_thumb_url, store->largeThumbUrl);  

    return err;
}

uint32
data_store_data_get_tps_size(data_util_state* state, data_store_data* storeData)
{
    uint32 size = 0;

    size += data_place_get_tps_size(state, &storeData->place);

    size += data_string_get_tps_size(state, &storeData->retailer_name);
    size += data_string_get_tps_size(state, &storeData->retailer_id);
    size +=sizeof(storeData->coupon_count);

    DATA_VEC_GET_TPS_SIZE(state, size, storeData->broad_categories, data_coupon_broad_category);
    DATA_VEC_GET_TPS_SIZE(state, size, storeData->sub_categories, data_coupon_sub_category);
    DATA_VEC_GET_TPS_SIZE(state, size, storeData->categories, data_coupon_category);
    size += data_image_urls_get_tps_size( state, &storeData->image_urls );

    return size;
}

void
data_store_data_to_buf(data_util_state* state, data_store_data* storeData, struct dynbuf* buffer)
{
    data_place_to_buf(state, &storeData->place, buffer);

    data_string_to_buf(state, &storeData->retailer_name, buffer);
    data_string_to_buf(state, &storeData->retailer_id, buffer);
    dbufcat(buffer, (const byte*) &storeData->coupon_count, sizeof(storeData->coupon_count));

    DATA_VEC_TO_BUF(state, buffer, storeData->broad_categories, data_coupon_broad_category);
    DATA_VEC_TO_BUF(state, buffer, storeData->sub_categories, data_coupon_sub_category);
    DATA_VEC_TO_BUF(state, buffer, storeData->categories, data_coupon_category);
    data_image_urls_to_buf(state, &storeData->image_urls, buffer);
}

NB_Error
data_store_data_from_binary(data_util_state* state, data_store_data* storeData, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_place_from_binary(state, &storeData->place, pdata, pdatalen);

    err = err ? err : data_string_from_binary(state, &storeData->retailer_name, pdata, pdatalen);
    err = err ? err : data_string_from_binary(state, &storeData->retailer_id, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(state, &storeData->coupon_count, pdata, pdatalen);

    DATA_VEC_FROM_BINARY(state, err, pdata, pdatalen, storeData->broad_categories, data_coupon_broad_category);
    DATA_VEC_FROM_BINARY(state, err, pdata, pdatalen, storeData->sub_categories, data_coupon_sub_category);
    DATA_VEC_FROM_BINARY(state, err, pdata, pdatalen, storeData->categories, data_coupon_category);
    err = err ? err : data_image_urls_from_binary(state, &storeData->image_urls, pdata, pdatalen);

    return err;
}


/*! @} */
