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

    @file     data_coupon_acquisition.c
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

#include "data_coupon_acquisition.h"

NB_Error
data_coupon_acquisition_init(data_util_state* state, data_coupon_acquisition* acquisition)
{
    NB_Error err = NE_OK;
    DATA_MEM_ZERO(acquisition, data_coupon_acquisition);
    err = err ? err : data_string_init(state, &acquisition->acquisition_text);
    return err;
}

void
data_coupon_acquisition_free(data_util_state* state, data_coupon_acquisition* acquisition)
{
    if ( acquisition )
    {
        data_string_free(state, &acquisition->acquisition_text);
    }
}

NB_Error
data_coupon_acquisition_from_tps(data_util_state* state, data_coupon_acquisition* acquisition, tpselt tpsElement)
{
    NB_Error err = NE_OK;
    if (tpsElement == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    data_coupon_acquisition_free(state, acquisition);
    err = data_coupon_acquisition_init(state, acquisition);
    if (err != NE_OK)
    {
        return err;
    }

    err = err ? err : data_string_from_tps_attr(state, &acquisition->acquisition_text, tpsElement, "acquisition-text");

errexit:
    if (err != NE_OK)
    {
        data_coupon_acquisition_free(state, acquisition);
    }
    return err;
}

tpselt
data_coupon_acquisition_to_tps(data_util_state* state, data_coupon_acquisition* acquisition)
{
    tpselt tpsElement = te_new("coupon-acquisition");
    if (tpsElement == NULL)
    {
        goto errexit;
    }

    if (!te_setattrc(tpsElement, "acquisition-text", data_string_get(state, &acquisition->acquisition_text)))
    {
        goto errexit;
    }

    return tpsElement;

errexit:
    te_dealloc(tpsElement);
    return NULL;
}

boolean
data_coupon_acquisition_equal(data_util_state* state, data_coupon_acquisition* left, data_coupon_acquisition* right)
{
    return (boolean) data_string_equal(state, &left->acquisition_text, &right->acquisition_text);
}

NB_Error
data_coupon_acquisition_copy(data_util_state* state, data_coupon_acquisition* dest, data_coupon_acquisition* src)
{
    NB_Error err = NE_OK;
    data_coupon_acquisition_free(state, dest);
    err = err ? err : data_coupon_acquisition_init(state, dest);

    err = err ? err : data_string_copy(state, &dest->acquisition_text, &src->acquisition_text);

    return err;
}

uint32
data_coupon_acquisition_get_tps_size(data_util_state* state, data_coupon_acquisition* acquisition)
{
    return data_string_get_tps_size(state, &acquisition->acquisition_text);
}

void
data_coupon_acquisition_to_buf(data_util_state* state, data_coupon_acquisition* acquisition, struct dynbuf *buffer)
{
    data_string_to_buf(state, &acquisition->acquisition_text, buffer);
}

NB_Error
data_coupon_acquisition_from_binary(data_util_state* state, data_coupon_acquisition* acquisition, byte** ppdata, size_t* pdatalen)
{
    return data_string_from_binary(state, &acquisition->acquisition_text, ppdata, pdatalen);
}


/*! @} */
