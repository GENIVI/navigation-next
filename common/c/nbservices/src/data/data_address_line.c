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

    @file   data_address_line.c
*/
/*
    See file description in header file.

    (C) Copyright 2013 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

/*! @{ */

#include "data_address_line.h"

NB_Error
data_address_line_init(data_util_state* state, data_address_line* addressLine)
{
    NB_Error error = NE_OK;

    DATA_MEM_ZERO(addressLine, data_address_line);

    DATA_INIT(state, error, &(addressLine->text), data_string);

    return error;
}

void
data_address_line_free(data_util_state* state, data_address_line* addressLine)
{
    DATA_FREE(state,  &(addressLine->text), data_string);
}

NB_Error
data_address_line_from_tps(data_util_state* state, data_address_line* addressLine, tpselt tpsElement)
{
    NB_Error error = NE_OK;

    if (tpsElement == NULL)
    {
        error = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(state, error, addressLine, data_address_line);
    if (error != NE_OK)
    {
        goto errexit;
    }

    if (te_getattrc(tpsElement, "text") != NULL)
    {
        error = error ? error : data_string_from_tps_attr(state, &(addressLine->text), tpsElement, "text");
    }

errexit:

    if (error != NE_OK)
    {
        DATA_FREE(state, addressLine, data_address_line);
    }

    return error;
}

boolean
data_address_line_equal(data_util_state* state, data_address_line* addressLine1, data_address_line* addressLine2)
{
    int result = TRUE;

    DATA_EQUAL(state, result, &(addressLine1->text), &(addressLine2->text), data_string);

    return (boolean) result;
}

NB_Error
data_address_line_copy(data_util_state* state, data_address_line* destinationAddressLine, data_address_line* sourceAddressLine)
{
    NB_Error error = NE_OK;

    DATA_REINIT(state, error, destinationAddressLine, data_address_line);
    DATA_COPY(state, error, &(destinationAddressLine->text), &(sourceAddressLine->text), data_string);

    if (error != NE_OK)
    {
        DATA_FREE(state, destinationAddressLine, data_address_line);
    }

    return error;
}

uint32
data_address_line_get_tps_size(data_util_state* state, data_address_line* addressLine)
{
    uint32 size = 0;

    size += data_string_get_tps_size(state, &(addressLine->text));

    return size;
}

void
data_address_line_to_buf(data_util_state* state, data_address_line* addressLine, struct dynbuf* buf)
{
    data_string_to_buf(state, &(addressLine->text), buf);
}

NB_Error
data_address_line_from_binary(data_util_state* state, data_address_line* addressLine, byte** data, size_t* dataSize)
{
    NB_Error error = NE_OK;

    error = error ? error : data_string_from_binary(state, &(addressLine->text), data, dataSize);

    return error;
}

/*! @} */
