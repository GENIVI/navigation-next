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

    @file     data_load_image_response.c
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

#include "data_load_image_response.h"

NB_Error
data_load_image_response_init(data_util_state* state,
                               data_load_image_response* response)
{
    NB_Error err = NE_OK;
    DATA_MEM_ZERO(response, data_load_image_response);

    err = err ? err : data_string_init(state, &response->image_url);
    err = err ? err : data_blob_init(state, &response->image_bytes);

    return err;
}

void
data_load_image_response_free(data_util_state* state,
                               data_load_image_response* response)
{
    data_string_free(state, &response->image_url);
    data_blob_free(state, &response->image_bytes);
}

NB_Error
data_load_image_response_from_tps(data_util_state* state,
                                   data_load_image_response* response,
                                   tpselt tpsElement)
{
    NB_Error err = NE_OK;

    if (tpsElement == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    data_load_image_response_free(state, response);
    err = data_load_image_response_init(state, response);
    if (err != NE_OK)
    {
        return err;
    }

    err = data_string_from_tps_attr(state, &response->image_url, tpsElement, "image-url");
    err = err ? err : data_blob_from_tps_attr(state, &response->image_bytes, tpsElement, "image-bytes");


errexit:
    if (err != NE_OK)
    {
        data_load_image_response_free(state, response);
    }
    return err;
}

/*! @} */
