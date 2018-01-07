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

    @file     data_proxy_api_key.c
    
    Implemention for wifi TPS element for the Location servlet. 
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

#include "data_proxy_api_key.h"

NB_Error
data_proxy_api_key_init(data_util_state* pds, data_proxy_api_key* pak)
{
    return data_string_init(pds, &pak->apiKey);
}

void
data_proxy_api_key_free(data_util_state* pds, data_proxy_api_key* pak)
{
    data_string_free(pds, &pak->apiKey);
}

tpselt
data_proxy_api_key_to_tps(data_util_state* pds, data_proxy_api_key* pak)
{
    tpselt te = NULL;

    if ( nsl_strcmp(  data_string_get(pds, &pak->apiKey), "" ) == 0 )
    {
        return NULL;
    }

    te = te_new("proxy-api-key");

    if (!te) 
    {
        goto errexit;
    }

    if (!te_setattrc(te, "api-key", data_string_get(pds, &pak->apiKey)))
    {
        goto errexit;
    }
    
    return te;

errexit:
    te_dealloc(te);
    return NULL;
}

boolean data_proxy_api_key_equal(data_util_state* pds, data_proxy_api_key* pak1, data_proxy_api_key* pak2)
{
    return (boolean)(data_string_equal(pds, &pak1->apiKey, &pak2->apiKey));
}

NB_Error
data_proxy_api_key_copy(data_util_state* pds, data_proxy_api_key* pak_dest, data_proxy_api_key* pak_src)
{
    return data_string_copy(pds, &pak_dest->apiKey, &pak_src->apiKey);
}

/*! @} */
