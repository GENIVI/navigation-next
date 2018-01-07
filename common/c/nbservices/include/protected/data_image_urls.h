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

    @file     data_image_urls.h
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

#ifndef DATA_IMAGE_URLS_H
#define DATA_IMAGE_URLS_H

/*! @{ */

#include "datautil.h"
#include "data_string.h"

typedef struct data_image_urls_
{
    /* Child Elements */

    /* Attributes */
    data_string    image_url;           /*!< URL to fetch the image for the coupon. */
    data_string    small_thumb_url;     /*!< URL to fetch a small thimb of the image. */
    data_string    medium_thumb_url;    /*!< URL to fetch a medium thimb of the image. */
    data_string    large_thumb_url;     /*!< URL to fetch a large thimb of the image. */

} data_image_urls;

NB_Error    data_image_urls_init(data_util_state* state, data_image_urls* imageUrls);
void        data_image_urls_free(data_util_state* state, data_image_urls* imageUrls);

NB_Error    data_image_urls_from_tps(data_util_state* state, data_image_urls* imageUrls, tpselt tpsElement);
tpselt      data_image_urls_to_tps(data_util_state* state, data_image_urls* imageUrls);

boolean     data_image_urls_equal(data_util_state* state, data_image_urls* left, data_image_urls* right);
NB_Error    data_image_urls_copy(data_util_state* state, data_image_urls* dest, data_image_urls* src);

uint32      data_image_urls_get_tps_size(data_util_state* state, data_image_urls* imageUrls);
void        data_image_urls_to_buf(data_util_state* state, data_image_urls* imageUrls, struct dynbuf *buffer);
NB_Error    data_image_urls_from_binary(data_util_state* state, data_image_urls* imageUrls, byte** ppdata, size_t* pdatalen);

/*! @} */

#endif
