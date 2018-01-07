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

    @file     data_store_request.c
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

#include "data_store_request.h"
#include "nbmobilecouponsprivate.h"
#include "data_pair.h"

NB_Error
data_store_request_init(data_util_state* state, data_store_request* request)
{
    NB_Error err = NE_OK;
    DATA_MEM_ZERO(request, data_store_request);

    err = err ? err : data_iter_command_init(state, &request->iter_command);
    err = err ? err : data_position_init(state, &request->position);
    err = err ? err : data_search_filter_init(state, &request->search_filter);

    return err;
}

void
data_store_request_free(data_util_state* state, data_store_request* request)
{
    data_iter_command_free(state, &request->iter_command);
    data_position_free(state, &request->position);
    data_search_filter_free(state, &request->search_filter);
}

tpselt
data_store_request_to_tps(data_util_state* state, data_store_request* request)
{
    tpselt tpsElement = te_new("store-request");

    if (tpsElement == NULL)
    {
        goto errexit;
    }

    DATA_TO_TPS(state, errexit, tpsElement, &request->iter_command, data_iter_command);

    if ( NULL != request->search_filter.vec_pairs &&
         0 != CSL_VectorGetLength(  request->search_filter.vec_pairs ) )
    {
        DATA_TO_TPS(state, errexit, tpsElement, &request->search_filter, data_search_filter);
    }

    DATA_TO_TPS(state, errexit, tpsElement, &request->position, data_position);

    return tpsElement;

errexit:

    te_dealloc(tpsElement);
    return NULL;
}

NB_Error
data_store_request_copy(data_util_state* state, data_store_request* dest, data_store_request* src)
{
    NB_Error err = NE_OK;

    data_store_request_free(state, dest);
    err = err ? err : data_store_request_init(state, dest);

    err = err ? err : data_iter_command_copy(state, &dest->iter_command, &src->iter_command);
    err = err ? err : data_position_copy(state, &dest->position, &src->position);
    err = err ? err : data_search_filter_copy(state, &dest->search_filter, &src->search_filter);

    return err;
}

/*! @} */
