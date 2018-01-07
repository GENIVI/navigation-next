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

    @file     data_coupon_data.c
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

#include "data_coupon_data.h"
#include "data_coupon_acquisition.h"
#include "data_coupon_availability.h"
#include "nbmobilecouponsprivate.h"

NB_Error
data_coupon_data_init(data_util_state* state, data_coupon_data* couponData)
{
    NB_Error err = NE_OK;
    DATA_MEM_ZERO(couponData, data_coupon_data);

    DATA_VEC_ALLOC(err, couponData->availability, data_coupon_availability);
    DATA_VEC_ALLOC(err, couponData->acquisition, data_coupon_acquisition);

    err = err ? err : data_string_init(state, &couponData->title);
    err = err ? err : data_string_init(state, &couponData->description);
    err = err ? err : data_string_init(state, &couponData->conditions);
    err = err ? err : data_string_init(state, &couponData->id);
    err = err ? err : data_string_init(state, &couponData->currency);
    err = err ? err : data_string_init(state, &couponData->coupon_code);
    err = err ? err : data_string_init(state, &couponData->deal_url);

    couponData->start_date = 0;
    couponData->expiration_date = 0;
    couponData->discount_type = NB_CDTT_None;
    couponData->discount_value = 0.0;
    couponData->buy_value = 0.0;
    couponData->list_value = 0.0;

    err = err ? err : data_image_urls_init( state, &couponData->image_urls );

    return err;
}

void
data_coupon_data_free(data_util_state* state, data_coupon_data* couponData)
{
    if (couponData)
    {
        DATA_VEC_FREE(state, couponData->availability, data_coupon_availability);
        DATA_VEC_FREE(state, couponData->acquisition, data_coupon_acquisition);

        data_string_free(state, &couponData->title);
        data_string_free(state, &couponData->description);
        data_string_free(state, &couponData->conditions);
        data_string_free(state, &couponData->id);
        data_string_free(state, &couponData->currency);
        data_string_free(state, &couponData->coupon_code);
        data_string_free(state, &couponData->deal_url);

        couponData->start_date = 0;
        couponData->expiration_date = 0;
        couponData->discount_type = NB_CDTT_None;
        couponData->discount_value = 0.0;
        couponData->buy_value = 0.0;
        couponData->list_value = 0.0;

        data_image_urls_free( state, &couponData->image_urls);
    }
}

NB_Error
data_coupon_data_from_tps(data_util_state* state, data_coupon_data* couponData, tpselt tpsElement)
{
    NB_Error err = NE_OK;
    tpselt ce = NULL;
    int iterator = 0;

    if (tpsElement == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    data_coupon_data_free(state, couponData);
    err = data_coupon_data_init(state, couponData);
    if (err != NE_OK)
    {
        return err;
    }

    while ( ( ce = te_nextchild( tpsElement, &iterator ) ) != NULL )
    {
        if ( ce && ( nsl_stricmp( te_getname( ce ), "coupon-availability" ) == 0 ) )
        {
            if ( couponData->availability == NULL )
            {
                DATA_VEC_ALLOC( err, couponData->availability, data_coupon_availability );
            }

            DATA_FROM_TPS_ADD_TO_VEC( state, err, ce, couponData->availability, data_coupon_availability );

            if ( err != NE_OK )
            {
                goto errexit;
            }
        }
        else if ( ce && ( nsl_stricmp( te_getname( ce ), "coupon-acquisition" ) == 0 ) )
        {
            if ( couponData->acquisition == NULL )
            {
                DATA_VEC_ALLOC( err, couponData->acquisition, data_coupon_acquisition );
            }

            DATA_FROM_TPS_ADD_TO_VEC( state, err, ce, couponData->acquisition, data_coupon_acquisition );

            if ( err != NE_OK )
            {
                goto errexit;
            }
        }
        else if ( ce && ( nsl_stricmp( te_getname( ce ), "image-urls" ) == 0 )  )
        {
            DATA_FROM_TPS( state, err, ce, &couponData->image_urls, data_image_urls );
        }
    }

    err = err ? err : data_string_from_tps_attr(state, &couponData->title, tpsElement, "title");
    err = err ? err : data_string_from_tps_attr(state, &couponData->description, tpsElement, "description");
    err = err ? err : data_string_from_tps_attr(state, &couponData->conditions, tpsElement, "conditions");
    err = err ? err : data_string_from_tps_attr(state, &couponData->id, tpsElement, "id");
    couponData->start_date = te_getattru(tpsElement, "start-date");
    couponData->expiration_date = te_getattru(tpsElement, "expiration-date");
    err = err ? err : data_string_from_tps_attr(state, &couponData->currency, tpsElement, "currency");
    couponData->discount_type = ( NB_CouponDiscountThresholdType )te_getattru(tpsElement, "discount-type");
    ( void )te_getattrd(tpsElement, "discount-value", &couponData->discount_value);
    ( void )te_getattrd(tpsElement, "buy-value", &couponData->buy_value);
    ( void )te_getattrd(tpsElement, "list-value", &couponData->list_value);

    err = err ? err : data_string_from_tps_attr(state, &couponData->coupon_code, tpsElement, "coupon-code");
    err = err ? err : data_string_from_tps_attr(state, &couponData->deal_url, tpsElement, "deal-url");

errexit:
    if (err != NE_OK)
    {
        data_coupon_data_free(state, couponData);
    }
    return err;
}

tpselt
data_coupon_data_to_tps(data_util_state* state, data_coupon_data* couponData)
{
    tpselt tpsElement = te_new("coupon-data");
    if (tpsElement == NULL)
    {
        goto errexit;
    }

    DATA_VEC_TO_TPS( state, errexit, tpsElement, couponData->availability, data_coupon_availability );
    DATA_VEC_TO_TPS( state, errexit, tpsElement, couponData->acquisition, data_coupon_acquisition );

    if (!te_setattrc(tpsElement, "title", data_string_get(state, &couponData->title)))
    {
        goto errexit;
    }

    if (!te_setattrc(tpsElement, "description", data_string_get(state, &couponData->description)))
    {
        goto errexit;
    }

    if (!te_setattrc(tpsElement, "conditions", data_string_get(state, &couponData->conditions)))
    {
        goto errexit;
    }

    if (!te_setattrc(tpsElement, "id", data_string_get(state, &couponData->id)))
    {
        goto errexit;
    }

    if (!te_setattru(tpsElement, "start-date", couponData->start_date))
    {
        goto errexit;
    }

    if (!te_setattru(tpsElement, "expiration-date", couponData->expiration_date))
    {
        goto errexit;
    }

    if (!te_setattrc(tpsElement, "currency", data_string_get(state, &couponData->currency)))
    {
        goto errexit;
    }

    if (!te_setattru(tpsElement, "discount-type", ( uint32 )couponData->discount_type))
    {
        goto errexit;
    }

    if (!te_setattrd(tpsElement, "discount-value", couponData->discount_value))
    {
        goto errexit;
    }

    if (!te_setattrd(tpsElement, "buy-value", couponData->buy_value))
    {
        goto errexit;
    }

    if (!te_setattrd(tpsElement, "list-value", couponData->list_value))
    {
        goto errexit;
    }

    if (!te_setattrc(tpsElement, "coupon-code", data_string_get(state, &couponData->coupon_code)))
    {
        goto errexit;
    }

    if (!te_setattrc(tpsElement, "deal-url", data_string_get(state, &couponData->deal_url)))
    {
        goto errexit;
    }

    DATA_TO_TPS( state, errexit, tpsElement, &couponData->image_urls, data_image_urls );

    return tpsElement;

errexit:

    te_dealloc(tpsElement);
    return NULL;
}

boolean
data_coupon_data_equal(data_util_state* state, data_coupon_data* left, data_coupon_data* right)
{
    int ret = TRUE;

    DATA_VEC_EQUAL(state, ret, left->availability, right->availability, data_coupon_availability);
    DATA_VEC_EQUAL(state, ret, left->acquisition, right->acquisition, data_coupon_acquisition);

    return (boolean) ( ret == TRUE &&
                       data_string_equal(state, &left->title, &right->title) &&
                       data_string_equal(state, &left->description, &right->description) &&
                       data_string_equal(state, &left->conditions, &right->conditions) &&
                       data_string_equal(state, &left->id, &right->id) &&
                       left->start_date == right->start_date &&
                       left->expiration_date == right->expiration_date &&
                       data_string_equal(state, &left->currency, &right->currency) &&
                       left->discount_type == right->discount_type &&
                       left->discount_value == right->discount_value &&
                       left->buy_value == right->buy_value &&
                       left->list_value == right->list_value &&
                       data_string_equal(state, &left->coupon_code, &right->coupon_code) &&
                       data_string_equal(state, &left->deal_url, &right->deal_url) &&
                       data_image_urls_equal(state, &left->image_urls, &right->image_urls) );
}

NB_Error
data_coupon_data_copy(data_util_state* state, data_coupon_data* dest, data_coupon_data* src)
{
    NB_Error err = NE_OK;

    data_coupon_data_free(state, dest);
    err = err ? err : data_coupon_data_init(state, dest);

    DATA_VEC_COPY(state, err, dest->availability, src->availability, data_coupon_availability);
    DATA_VEC_COPY(state, err, dest->acquisition, src->acquisition, data_coupon_acquisition);
    err = err ? err : data_string_copy(state, &dest->title, &src->title);
    err = err ? err : data_string_copy(state, &dest->description, &src->description);
    err = err ? err : data_string_copy(state, &dest->conditions, &src->conditions);
    err = err ? err : data_string_copy(state, &dest->id, &src->id);
    dest->start_date = src->start_date;
    dest->expiration_date = src->expiration_date;
    err = err ? err : data_string_copy(state, &dest->currency, &src->currency);
    dest->discount_type = src->discount_type;
    dest->discount_value = src->discount_value;
    dest->buy_value = src->buy_value;
    dest->list_value = src->list_value;
    err = err ? err : data_string_copy(state, &dest->coupon_code, &src->coupon_code);
    err = err ? err : data_string_copy(state, &dest->deal_url, &src->deal_url);
    err = err ? err : data_image_urls_copy(state, &dest->image_urls, &src->image_urls);

    return err;
}

NB_Error
data_coupon_data_from_nbcoupon(data_util_state* state, data_coupon_data* couponData, const NB_Coupon* coupon)
{
    NB_Error err = NE_OK;
    uint32 i = 0;

    if (coupon == NULL)
    {
        return NE_INVAL;
    }

    data_coupon_data_free(state, couponData);
    err = err ? err : data_coupon_data_init(state, couponData);

    if ( couponData->availability == NULL && coupon->availabilityCount > 0 )
    {
        DATA_VEC_ALLOC( err, couponData->availability, data_coupon_availability );
    }

    for ( i = 0; i < coupon->availabilityCount && err == NE_OK; i++ )
    {
        data_coupon_availability tmp = {0};
        err = data_coupon_availability_init( state, &tmp );
        err = err ? err : data_string_set( state, &tmp.availability_text, coupon->availability[i] );

        if ( err == NE_OK)
        {
            if ( !CSL_VectorAppend( couponData->availability, &tmp ) )
            {
                err = NE_NOMEM;
            }
        }

        if ( err != NE_OK )
        {
            data_coupon_availability_free( state, &tmp );
        }
    }

    if ( couponData->acquisition == NULL && coupon->acquisitionCount > 0 )
    {
        DATA_VEC_ALLOC( err, couponData->acquisition, data_coupon_acquisition );
    }

    for ( i = 0; i < coupon->acquisitionCount && err == NE_OK; i++ )
    {
        data_coupon_acquisition tmp = {0};
        err = data_coupon_acquisition_init( state, &tmp );
        err = err ? err : data_string_set( state, &tmp.acquisition_text, coupon->acquisition[i] );

        if ( err == NE_OK)
        {
            if ( !CSL_VectorAppend( couponData->acquisition, &tmp ) )
            {
                err = NE_NOMEM;
            }
        }

        if ( err != NE_OK )
        {
            data_coupon_acquisition_free( state, &tmp );
        }
    }

    err = err ? err : data_string_set(state, &couponData->title, coupon->title);
    err = err ? err : data_string_set(state, &couponData->description, coupon->description);
    err = err ? err : data_string_set(state, &couponData->conditions, coupon->conditions);
    err = err ? err : data_string_set(state, &couponData->id, coupon->id);
    couponData->start_date = coupon->startDate;
    couponData->expiration_date = coupon->expirationDate;
    err = err ? err : data_string_set(state, &couponData->currency, coupon->currency);
    couponData->discount_type = coupon->discountType;
    couponData->discount_value = coupon->discountValue;
    couponData->buy_value = coupon->buyValue;
    couponData->list_value = coupon->listValue;
    err = err ? err : data_string_set(state, &couponData->coupon_code, coupon->couponCode);
    err = err ? err : data_string_set(state, &couponData->deal_url, coupon->dealUrl);
    err = err ? err : data_string_set(state, &couponData->image_urls.image_url, coupon->imageUrl);
    err = err ? err : data_string_set(state, &couponData->image_urls.small_thumb_url, coupon->smallThumbUrl);
    err = err ? err : data_string_set(state, &couponData->image_urls.medium_thumb_url, coupon->mediumThumbUrl);
    err = err ? err : data_string_set(state, &couponData->image_urls.large_thumb_url, coupon->largeThumbUrl);  

    return err;
}

uint32
data_coupon_data_get_tps_size(data_util_state* state, data_coupon_data* couponData)
{
    uint32 size = 0;

    DATA_VEC_GET_TPS_SIZE(state, size, couponData->availability, data_coupon_availability);
    DATA_VEC_GET_TPS_SIZE(state, size, couponData->acquisition, data_coupon_acquisition);
    size += data_string_get_tps_size(state, &couponData->title);
    size += data_string_get_tps_size(state, &couponData->description);
    size += data_string_get_tps_size(state, &couponData->conditions);
    size += data_string_get_tps_size(state, &couponData->id);
    size += sizeof(couponData->start_date);
    size += sizeof(couponData->expiration_date);
    size += data_string_get_tps_size(state, &couponData->currency);
    size += sizeof(couponData->discount_type);
    size += sizeof(couponData->discount_value);
    size += sizeof(couponData->buy_value);
    size += sizeof(couponData->list_value);
    size += data_string_get_tps_size(state, &couponData->coupon_code);
    size += data_string_get_tps_size(state, &couponData->deal_url);
    size += data_image_urls_get_tps_size( state, &couponData->image_urls );

    return size;
}

void
data_coupon_data_to_buf(data_util_state* state, data_coupon_data* couponData, struct dynbuf* buffer)
{
    uint32 type = 0;
    DATA_VEC_TO_BUF(state, buffer, couponData->availability, data_coupon_availability);
    DATA_VEC_TO_BUF(state, buffer, couponData->acquisition, data_coupon_acquisition);

    data_string_to_buf(state, &couponData->title, buffer);
    data_string_to_buf(state, &couponData->description, buffer);
    data_string_to_buf(state, &couponData->conditions, buffer);
    data_string_to_buf(state, &couponData->id, buffer);
    dbufcat(buffer, (const byte*) &couponData->start_date, sizeof(couponData->start_date));
    dbufcat(buffer, (const byte*) &couponData->expiration_date, sizeof(couponData->expiration_date));
    data_string_to_buf(state, &couponData->currency, buffer);
    type = (uint32)couponData->discount_type;
    dbufcat(buffer, (const byte*) &type, sizeof( type ));
    dbufcat(buffer, (const byte*) &couponData->discount_value, sizeof(couponData->discount_value));
    dbufcat(buffer, (const byte*) &couponData->buy_value, sizeof(couponData->buy_value));
    dbufcat(buffer, (const byte*) &couponData->list_value, sizeof(couponData->list_value));
    data_string_to_buf(state, &couponData->coupon_code, buffer);
    data_string_to_buf(state, &couponData->deal_url, buffer);
    data_image_urls_to_buf(state, &couponData->image_urls, buffer);
}

NB_Error
data_coupon_data_from_binary(data_util_state* state, data_coupon_data* couponData, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;
    uint32 type = 0;

    DATA_VEC_FROM_BINARY( state, err, pdata, pdatalen, couponData->availability, data_coupon_availability);
    DATA_VEC_FROM_BINARY( state, err, pdata, pdatalen, couponData->acquisition, data_coupon_acquisition);

    err = err ? err : data_string_from_binary(state, &couponData->title, pdata, pdatalen);
    err = err ? err : data_string_from_binary(state, &couponData->description, pdata, pdatalen);
    err = err ? err : data_string_from_binary(state, &couponData->conditions, pdata, pdatalen);
    err = err ? err : data_string_from_binary(state, &couponData->id, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(state, &couponData->start_date, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(state, &couponData->expiration_date, pdata, pdatalen);
    err = err ? err : data_string_from_binary(state, &couponData->currency, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(state, &type, pdata, pdatalen);
    if (!err)
    {
        couponData->discount_type = (NB_CouponDiscountThresholdType)type;
    }
    err = err ? err : data_double_from_binary(state, &couponData->discount_value, pdata, pdatalen);
    err = err ? err : data_double_from_binary(state, &couponData->buy_value, pdata, pdatalen);
    err = err ? err : data_double_from_binary(state, &couponData->list_value, pdata, pdatalen);
    err = err ? err : data_string_from_binary(state, &couponData->coupon_code, pdata, pdatalen);
    err = err ? err : data_string_from_binary(state, &couponData->deal_url, pdata, pdatalen);
    err = err ? err : data_image_urls_from_binary(state, &couponData->image_urls, pdata, pdatalen);

    return err;
}

/*! @} */
