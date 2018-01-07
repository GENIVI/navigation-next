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

    @file     data_error_category.c
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

#include "data_error_category.h"

NB_Error
data_error_category_init(data_util_state* pds, data_error_category* errorCategory)
{
    NB_Error err = NE_OK;
    err = data_string_init(pds, &errorCategory->type);
    return err;
}

void
data_error_category_free(data_util_state* pds, data_error_category* errorCategory)
{
    data_string_free(pds, &errorCategory->type);
}

NB_Error
data_error_category_from_tps(data_util_state* pds, data_error_category* errorCategory, tpselt te)
{
    NB_Error err = NE_OK;
    if (te == NULL)
    {
        err = NE_INVAL;
        goto errexit;
    }

    data_error_category_free(pds, errorCategory);
    err = data_error_category_init(pds, errorCategory);
    if (err != NE_OK)
    {
        return err;
    }

    err = data_string_from_tps_attr(pds, &errorCategory->type, te, "type");

errexit:
    if (err != NE_OK)
    {
        data_error_category_free(pds, errorCategory);
    }

    return err;
}

tpselt
data_error_category_to_tps(data_util_state* pds, data_error_category* errorCategory)
{
    tpselt te = te_new("error-category");
    if (te == NULL)
    {
        goto errexit;
    }

    if (!te_setattrc(te, "type", data_string_get(pds, &errorCategory->type)))
    {
        goto errexit;
    }

    return te;

errexit:
    te_dealloc(te);
    return NULL;
}

boolean
data_error_category_equal(data_util_state* pds, data_error_category* left, data_error_category* right)
{
    return (boolean) data_string_equal(pds, &left->type, &right->type);
}

NB_Error
data_error_category_copy(data_util_state* pds, data_error_category* dest, data_error_category* src)
{
    NB_Error err = NE_OK;
    data_error_category_free(pds, dest);
    err = data_error_category_init(pds, dest);

    err = err ? err : data_string_copy(pds, &dest->type, &src->type);

    return err;
}

uint32
data_error_category_get_tps_size(data_util_state* pds, data_error_category* errorCategory)
{
    return data_string_get_tps_size(pds, &errorCategory->type);
}

void
data_error_category_to_buf(data_util_state* pds, data_error_category* errorCategory, struct dynbuf *buffer)
{
    data_string_to_buf(pds, &errorCategory->type, buffer);
}

NB_Error
data_error_category_from_binary(data_util_state* pds, data_error_category* errorCategory, byte** ppdata, size_t* pdatalen)
{
    return data_string_from_binary(pds, &errorCategory->type, ppdata, pdatalen);
}

/*! @} */
