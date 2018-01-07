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

    @file     data_image_urls.c
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

#include "data_image_urls.h"

NB_Error
data_image_urls_init(data_util_state* state, data_image_urls* imageUrls)
{
    NB_Error err = NE_OK;
    DATA_MEM_ZERO(imageUrls, data_image_urls);

    err = err ? err : data_string_init(state, &imageUrls->image_url);
    err = err ? err : data_string_init(state, &imageUrls->small_thumb_url);
    err = err ? err : data_string_init(state, &imageUrls->medium_thumb_url);
    err = err ? err : data_string_init(state, &imageUrls->large_thumb_url);

    return err;
}

void
data_image_urls_free(data_util_state* state, data_image_urls* imageUrls)
{
    data_string_free(state, &imageUrls->image_url);
    data_string_free(state, &imageUrls->small_thumb_url);
    data_string_free(state, &imageUrls->medium_thumb_url);
    data_string_free(state, &imageUrls->large_thumb_url);
}

NB_Error
data_image_urls_from_tps(data_util_state* state, data_image_urls* imageUrls, tpselt tpsElement)
{
    NB_Error err = NE_OK;
    if (tpsElement == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    data_image_urls_free(state, imageUrls);
    err = data_image_urls_init(state, imageUrls);
    if (err != NE_OK)
    {
        return err;
    }

    err = data_string_from_tps_attr(state, &imageUrls->image_url, tpsElement, "image-url");
    err = err ? err : data_string_from_tps_attr(state, &imageUrls->small_thumb_url, tpsElement, "small-thumb-url");
    err = err ? err : data_string_from_tps_attr(state, &imageUrls->medium_thumb_url, tpsElement, "medium-thumb-url");
    err = err ? err : data_string_from_tps_attr(state, &imageUrls->large_thumb_url, tpsElement, "large-thumb-url");


errexit:
    if (err != NE_OK)
    {
        data_image_urls_free(state, imageUrls);
    }
    return err;
}

tpselt
data_image_urls_to_tps(data_util_state* state, data_image_urls* imageUrls)
{
    tpselt tpsElement = te_new("image-urls");
    if (tpsElement == NULL)
    {
        goto errexit;
    }

    if (!te_setattrc(tpsElement, "image-url", data_string_get(state, &imageUrls->image_url)))
    {
        goto errexit;
    }

    if (!te_setattrc(tpsElement, "small-thumb-url", data_string_get(state, &imageUrls->small_thumb_url)))
    {
        goto errexit;
    }

    if (!te_setattrc(tpsElement, "medium-thumb-url", data_string_get(state, &imageUrls->medium_thumb_url)))
    {
        goto errexit;
    }

    if (!te_setattrc(tpsElement, "large-thumb-url", data_string_get(state, &imageUrls->large_thumb_url)))
    {
        goto errexit;
    }

    return tpsElement;

errexit:
    te_dealloc(tpsElement);
    return NULL;
}

boolean
data_image_urls_equal(data_util_state* state, data_image_urls* left, data_image_urls* right)
{
    return (boolean) ( data_string_equal(state, &left->image_url, &right->image_url) &&
                       data_string_equal(state, &left->small_thumb_url, &right->small_thumb_url) &&
                       data_string_equal(state, &left->medium_thumb_url, &right->medium_thumb_url) &&
                       data_string_equal(state, &left->large_thumb_url, &right->large_thumb_url) );
}


NB_Error
data_image_urls_copy(data_util_state* state, data_image_urls* dest, data_image_urls* src)
{
    NB_Error err = NE_OK;

    data_image_urls_free(state, dest);
    err = err ? err : data_image_urls_init(state, dest);

    err = err ? err : data_string_copy(state, &dest->image_url, &src->image_url);
    err = err ? err : data_string_copy(state, &dest->small_thumb_url, &src->small_thumb_url);
    err = err ? err : data_string_copy(state, &dest->medium_thumb_url, &src->medium_thumb_url);
    err = err ? err : data_string_copy(state, &dest->large_thumb_url, &src->large_thumb_url);    

    return err;
}

uint32
data_image_urls_get_tps_size(data_util_state* state, data_image_urls* imageUrls)
{
    uint32 size = 0;
    
    size += data_string_get_tps_size(state, &imageUrls->image_url);
    size += data_string_get_tps_size(state, &imageUrls->small_thumb_url);
    size += data_string_get_tps_size(state, &imageUrls->medium_thumb_url);
    size += data_string_get_tps_size(state, &imageUrls->large_thumb_url);

    return size;
}

void
data_image_urls_to_buf(data_util_state* state, data_image_urls* imageUrls, struct dynbuf *buffer)
{
    data_string_to_buf(state, &imageUrls->image_url, buffer);
    data_string_to_buf(state, &imageUrls->small_thumb_url, buffer);
    data_string_to_buf(state, &imageUrls->medium_thumb_url, buffer);
    data_string_to_buf(state, &imageUrls->large_thumb_url, buffer);
}

NB_Error
data_image_urls_from_binary(data_util_state* state, data_image_urls* imageUrls, byte** ppdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_string_from_binary(state, &imageUrls->image_url, ppdata, pdatalen);
    err = err ? err : data_string_from_binary(state, &imageUrls->small_thumb_url, ppdata, pdatalen);
    err = err ? err : data_string_from_binary(state, &imageUrls->medium_thumb_url, ppdata, pdatalen);
    err = err ? err : data_string_from_binary(state, &imageUrls->large_thumb_url, ppdata, pdatalen);

    return err;
}

/*! @} */
