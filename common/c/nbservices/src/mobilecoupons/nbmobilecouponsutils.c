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

    @file     nbmobilecouponsutils.h
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

#include "nbmobilecouponsprivate.h"
#include "data_coupon_reply.h"
#include "data_coupon_category.h"
#include "data_coupon_broad_category.h"
#include "data_coupon_sub_category.h"
#include "data_coupon_availability.h"
#include "data_coupon_acquisition.h"
#include "data_pair.h"
#include "nbutilityprotected.h"
#include "tpselt.h"
#include "transformutility.h"

typedef enum
{
    CategoriesArray = 0,
    BroadCategoriesArray,
    SubCategoriesArray,
    AvailabilityArray,
    AcquisitionArray
} ArrayType;

static NB_Error CreateStringsArray(CSL_Vector* stringVector, char*** array, uint32* number, ArrayType arrType, data_util_state* state);
static void DestroyStringsArray(char** stringArray, uint32 num);

NB_Error
SetIterationCommand(data_util_state* state, data_iter_command* iterCommand, NB_IterationCommand command)
{
    NB_Error error = NE_OK;

    switch (command) 
    {
    case NB_IterationCommand_Start:
        error = data_string_set(state, &iterCommand->command, "start");
        break;
    case NB_IterationCommand_Next:
        error = data_string_set(state, &iterCommand->command, "next");
        break;
    case NB_IterationCommand_Previous:
        error = data_string_set(state, &iterCommand->command, "prev");
        break;
    default:
        error = NE_INVAL;
    }

    return error;
}

NB_Error
GetMobileCoupon(NB_Context* context, NB_Coupon* coupon, const data_coupon_data* couponData)
{
    data_util_state* state = NULL;
    NB_Error err = NE_OK;
    uint32 descriptionLength = 0;

    if (context == NULL || coupon == NULL || couponData == NULL)
    {
        return NE_INVAL;
    }
    nsl_memset(coupon, 0, sizeof(NB_Coupon));

    state = NB_ContextGetDataState(context);
    nsl_strlcpy(coupon->title, data_string_get(state, &couponData->title), NB_COUPON_TITLE_LEN);

    descriptionLength = (uint32)nsl_strlen(data_string_get(state, &couponData->description));
    coupon->description = (char*)nsl_malloc((descriptionLength + 1) * sizeof(char));
    if (!coupon->description)
    {
        return NE_NOMEM;
    }
    nsl_strlcpy(coupon->description, data_string_get(state, &couponData->description), descriptionLength + 1);

    nsl_strlcpy(coupon->conditions, data_string_get(state, &couponData->conditions), NB_COUPON_CONDITIONS_LEN);
    nsl_strlcpy(coupon->id, data_string_get(state, &couponData->id), NB_COUPON_ID_LEN);
    nsl_strlcpy(coupon->currency, data_string_get(state, &couponData->currency), NB_COUPON_CURRENCY_LEN);
    nsl_strlcpy(coupon->couponCode, data_string_get(state, &couponData->coupon_code), NB_COUPON_CODE_LEN);
    nsl_strlcpy(coupon->dealUrl, data_string_get(state, &couponData->deal_url), NB_COUPON_DEAL_URL_LEN);
    nsl_strlcpy(coupon->imageUrl, data_string_get(state, &couponData->image_urls.image_url), NB_COUPON_IMAGE_URL_LEN);
    nsl_strlcpy(coupon->smallThumbUrl, data_string_get(state, &couponData->image_urls.small_thumb_url), NB_COUPON_IMAGE_URL_LEN);
    nsl_strlcpy(coupon->mediumThumbUrl, data_string_get(state, &couponData->image_urls.medium_thumb_url), NB_COUPON_IMAGE_URL_LEN);
    nsl_strlcpy(coupon->largeThumbUrl, data_string_get(state, &couponData->image_urls.large_thumb_url), NB_COUPON_IMAGE_URL_LEN);

    coupon->startDate      = couponData->start_date;
    coupon->expirationDate = couponData->expiration_date;
    coupon->discountType   = couponData->discount_type;
    coupon->discountValue  = couponData->discount_value;
    coupon->buyValue       = couponData->buy_value;
    coupon->listValue      = couponData->list_value;

    err = err ? err : CreateStringsArray(couponData->availability, &coupon->availability, &coupon->availabilityCount, AvailabilityArray, state);
    err = err ? err : CreateStringsArray(couponData->acquisition, &coupon->acquisition, &coupon->acquisitionCount, AcquisitionArray, state);

    if (err != NE_OK)
    {
        goto errexit;
    }

    return err;

errexit:
    nsl_free(coupon->description);
    DestroyStringsArray(coupon->availability, coupon->availabilityCount);
    DestroyStringsArray(coupon->acquisition, coupon->acquisitionCount);
    return err;
}

NB_Error
CreateStore(NB_Context* context, NB_Store** store, const data_store_data* storeData)
{
    NB_Error err = NE_OK;
    NB_Store* tmpStore = NULL;
    data_util_state* state = NULL;

    if (context == NULL || store == NULL || storeData == NULL)
    {
        return NE_INVAL;
    }

    tmpStore = (NB_Store*)nsl_malloc(sizeof(NB_Store));
    if (tmpStore == NULL)
    {
        err = NE_NOMEM;
        goto errexit;
    }
    nsl_memset(tmpStore, 0, sizeof(NB_Store));

    state = NB_ContextGetDataState(context);
    tmpStore->count = storeData->coupon_count;
    nsl_strncpy(tmpStore->name, data_string_get(state, &storeData->retailer_name),
                NB_STORE_NAME_LEN - 1);
    nsl_strncpy(tmpStore->id, data_string_get(state, &storeData->retailer_id),
                NB_STORE_ID_LEN - 1);
    nsl_strncpy(tmpStore->imageUrl, data_string_get(state, &storeData->image_urls.image_url), NB_COUPON_IMAGE_URL_LEN - 1);
    nsl_strncpy(tmpStore->smallThumbUrl, data_string_get(state, &storeData->image_urls.small_thumb_url), NB_COUPON_IMAGE_URL_LEN - 1);
    nsl_strncpy(tmpStore->mediumThumbUrl, data_string_get(state, &storeData->image_urls.medium_thumb_url), NB_COUPON_IMAGE_URL_LEN - 1);
    nsl_strncpy(tmpStore->largeThumbUrl, data_string_get(state, &storeData->image_urls.large_thumb_url), NB_COUPON_IMAGE_URL_LEN - 1);

    err = SetNIMPlaceFromPlace(&tmpStore->place, state, (data_place*)&storeData->place);

    err = err ? err : CreateStringsArray(storeData->categories, &tmpStore->categories, &tmpStore->categoriesNumber, CategoriesArray, state);
    err = err ? err : CreateStringsArray(storeData->broad_categories, &tmpStore->broadCategories, &tmpStore->broadCategoriesNumber, BroadCategoriesArray, state);
    err = err ? err : CreateStringsArray(storeData->sub_categories, &tmpStore->subCategories, &tmpStore->subCategoriesNumber, SubCategoriesArray, state);

errexit:
    if (err != NE_OK)
    {
        DestroyStore(tmpStore);
        *store = NULL;
    }
    else
    {
        *store = tmpStore;
    }

    return err;
}

void
DestroyStore(NB_Store* store)
{
    if (store == NULL)
    {
        return;
    }

    DestroyStringsArray(store->categories, store->categoriesNumber);
    DestroyStringsArray(store->broadCategories, store->broadCategoriesNumber);
    DestroyStringsArray(store->subCategories, store->subCategoriesNumber);
    nsl_free(store);
}

NB_Error
CreateStoreCoupon(NB_Context* context, NB_StoreCoupons** storeCoupons, const data_store_coupon_data* storeCouponsData)
{
    NB_Error err = NE_OK;
    NB_StoreCoupons* tmpStoreCoupons = NULL;
    int couponsCount = 0, index = 0;
    data_util_state* state = NULL;

    if (context == NULL || storeCoupons == NULL || storeCouponsData == NULL)
    {
        return NE_INVAL;
    }

    tmpStoreCoupons = (NB_StoreCoupons*)nsl_malloc(sizeof(NB_StoreCoupons));
    if (tmpStoreCoupons == NULL)
    {
        err = NE_NOMEM;
        goto errexit;
    }
    nsl_memset(tmpStoreCoupons, 0, sizeof(NB_StoreCoupons));

    couponsCount = CSL_VectorGetLength(storeCouponsData->coupon_data);
    if (couponsCount > 0)
    {
        tmpStoreCoupons->coupons = (NB_Coupon*)nsl_malloc(sizeof(NB_Coupon) * couponsCount);
        if (tmpStoreCoupons->coupons == NULL)
        {
            err = NE_NOMEM;
            goto errexit;
        }
        nsl_memset(tmpStoreCoupons->coupons, 0, sizeof(NB_Coupon) * couponsCount);
    }

    // copy array of coupons
    tmpStoreCoupons->couponsNumber = couponsCount;
    for (index = 0; (err == NE_OK) && (index < couponsCount); ++index)
    {
        err = GetMobileCoupon(context, &(tmpStoreCoupons->coupons[index]),
                              CSL_VectorGetPointer(storeCouponsData->coupon_data, index));
    }

    // copy store
    state = NB_ContextGetDataState(context);
    tmpStoreCoupons->store.count = storeCouponsData->store_data.coupon_count;
    nsl_strncpy(tmpStoreCoupons->store.name, data_string_get(state,
                &storeCouponsData->store_data.retailer_name), NB_STORE_NAME_LEN - 1);
    nsl_strncpy(tmpStoreCoupons->store.id, data_string_get(state,
                &storeCouponsData->store_data.retailer_id), NB_STORE_ID_LEN - 1);
    err = SetNIMPlaceFromPlace(&tmpStoreCoupons->store.place, state,
                               (data_place*)&storeCouponsData->store_data.place);
    nsl_strncpy(tmpStoreCoupons->store.imageUrl, data_string_get(state,
                &storeCouponsData->store_data.image_urls.image_url), NB_COUPON_IMAGE_URL_LEN - 1);
    nsl_strncpy(tmpStoreCoupons->store.smallThumbUrl, data_string_get(state,
                &storeCouponsData->store_data.image_urls.small_thumb_url), NB_COUPON_IMAGE_URL_LEN - 1);
    nsl_strncpy(tmpStoreCoupons->store.mediumThumbUrl, data_string_get(state,
                &storeCouponsData->store_data.image_urls.medium_thumb_url), NB_COUPON_IMAGE_URL_LEN - 1);
    nsl_strncpy(tmpStoreCoupons->store.largeThumbUrl, data_string_get(state,
                &storeCouponsData->store_data.image_urls.large_thumb_url), NB_COUPON_IMAGE_URL_LEN - 1);

    err = err ? err : CreateStringsArray(storeCouponsData->store_data.categories,
                                            &tmpStoreCoupons->store.categories,
                                            &tmpStoreCoupons->store.categoriesNumber,
                                            CategoriesArray,
                                            state);
    err = err ? err : CreateStringsArray(storeCouponsData->store_data.broad_categories,
                                            &tmpStoreCoupons->store.broadCategories,
                                            &tmpStoreCoupons->store.broadCategoriesNumber,
                                            BroadCategoriesArray,
                                            state);
    err = err ? err : CreateStringsArray(storeCouponsData->store_data.sub_categories,
                                            &tmpStoreCoupons->store.subCategories,
                                            &tmpStoreCoupons->store.subCategoriesNumber,
                                            SubCategoriesArray,
                                            state);

errexit:
    if (err != NE_OK)
    {
        DestroyStoreCoupon(tmpStoreCoupons);
        *storeCoupons = NULL;
    }
    else
    {
        *storeCoupons = tmpStoreCoupons;
    }

    return err;
}

void
DestroyStoreCoupon(NB_StoreCoupons* storeCoupon)
{
    if (storeCoupon != NULL)
    {
        uint32 i = 0;
        DestroyStringsArray(storeCoupon->store.categories, storeCoupon->store.categoriesNumber);
        DestroyStringsArray(storeCoupon->store.broadCategories, storeCoupon->store.broadCategoriesNumber);
        DestroyStringsArray(storeCoupon->store.subCategories, storeCoupon->store.subCategoriesNumber);
 
        for ( i = 0; i < storeCoupon->couponsNumber; i++ )
        {
            nsl_free(storeCoupon->coupons[i].description);
            DestroyStringsArray(storeCoupon->coupons[i].acquisition, storeCoupon->coupons[i].acquisitionCount);
            DestroyStringsArray(storeCoupon->coupons[i].availability, storeCoupon->coupons[i].availabilityCount);
        }

        nsl_free(storeCoupon->coupons);
        nsl_free(storeCoupon);
    }
}

NB_Error
GetErrorCode(NB_Context* context, NB_NetworkResponse* response)
{
    uint32 replyError = 0;
    NB_Error serverError = NE_OK;
    if (context == NULL || response == NULL || response->reply == NULL)
    {
        return NE_OK;
    }

    replyError = GetReplyErrorCode(response);
    data_coupon_reply_from_tps_error(NB_ContextGetDataState(context), &serverError, response->reply);

    return (serverError != NE_OK) ? serverError : replyError;
}

NB_Error
SetPositionFromSearchRegion(NB_Context* context, data_position* position, const NB_CouponSearchRegion* searchRegion)
{
    NB_Error err = NE_OK;
    if (context == NULL || position == NULL || searchRegion == NULL)
    {
        return NE_INVAL;
    }

    switch (searchRegion->type)
    {
    case NB_CSRT_Center:
        {
            err = IsLatitudeLongitudeValid((NB_LatitudeLongitude*)&searchRegion->center) ? NE_OK : NE_INVAL;
            err = err ? err : data_string_set(NB_ContextGetDataState(context), &position->variant, POSITION_VARIANT_POINT);

            position->point.lat = searchRegion->center.latitude;
            position->point.lon = searchRegion->center.longitude;
        }
        break;

    case NB_CSRT_BoundingBox:
        {
            err = (IsLatitudeLongitudeValid((NB_LatitudeLongitude*)&searchRegion->boundingBox.topLeft) &&
                   IsLatitudeLongitudeValid((NB_LatitudeLongitude*)&searchRegion->boundingBox.bottomRight)) ? NE_OK : NE_INVAL;

            position->boxValid = TRUE;
            position->boundingBox.topLeft.lat = searchRegion->boundingBox.topLeft.latitude;
            position->boundingBox.topLeft.lon = searchRegion->boundingBox.topLeft.longitude;
            position->boundingBox.bottomRight.lat = searchRegion->boundingBox.bottomRight.latitude;
            position->boundingBox.bottomRight.lon = searchRegion->boundingBox.bottomRight.longitude;
        }
        break;

    default:
        err = NE_INVAL;
    }

    return err;
}

NB_Error
CreateStringsArray(CSL_Vector* stringVector, char*** array, uint32* number, ArrayType arrType, data_util_state* state)
{
    NB_Error err = NE_OK;
    uint32 num = 0, i = 0;
    char** arr = NULL;

    if (stringVector == NULL)
    {
        // no such categories, just skip it
        goto errexit;
    }

    num = CSL_VectorGetLength(stringVector);
    if (num == 0)
    {
        // no such categories, just skip it
        goto errexit;
    }

    arr = (char**)nsl_malloc(sizeof(char*) * num);
    if (arr == NULL)
    {
        err = NE_NOMEM;
        goto errexit;
    }
    nsl_memset(arr, 0, sizeof(char*) * num);

    for (i = 0; i < num && err == NE_OK; ++i)
    {
        const char* str = NULL;
        void* item = CSL_VectorGetPointer(stringVector, i);

        switch (arrType)
        {
        case CategoriesArray:
            str = data_string_get(state, &((data_coupon_category*)item)->category_text);
            break;
        case BroadCategoriesArray:
            str = data_string_get(state, &((data_coupon_broad_category*)item)->broad_category_text);
            break;
        case SubCategoriesArray:
            str = data_string_get(state, &((data_coupon_sub_category*)item)->sub_category_text);
            break;
        case AvailabilityArray:
             str = data_string_get(state, &((data_coupon_availability*)item)->availability_text);
            break;
        case AcquisitionArray:
             str = data_string_get(state, &((data_coupon_acquisition*)item)->acquisition_text);
            break;
        default:
            err = NE_INVAL;
            goto errexit;
        }

        arr[i] = nsl_strdup(str);
        if (arr[i] == NULL)
        {
            err = NE_NOMEM;
        }
    }


errexit:
    if (err == NE_OK)
    {
        *array = arr;
        *number = num;
    }
    else
    {
        DestroyStringsArray(arr, num);
    }
    return err;
}

void
DestroyStringsArray(char** stringArray, uint32 num)
{
    uint32 i = 0;
    if (stringArray == NULL)
    {
        return;
    }

    while (i < num && stringArray[i] != NULL)
    {
        nsl_free(stringArray[i]);
        ++i;
    }
    nsl_free(stringArray);
}

/*! @} */
