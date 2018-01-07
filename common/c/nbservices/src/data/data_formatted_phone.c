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

    @file   data_formatted_phone.c
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

#include "data_formatted_phone.h"

NB_Error
data_formatted_phone_init(data_util_state* state, data_formatted_phone* formattedPhone)
{
    NB_Error error = NE_OK;

    DATA_MEM_ZERO(formattedPhone, data_formatted_phone);

    DATA_INIT(state, error, &(formattedPhone->text), data_string);

    return error;
}

void
data_formatted_phone_free(data_util_state* state, data_formatted_phone* formattedPhone)
{
    DATA_FREE(state, &(formattedPhone->text), data_string);
}

NB_Error
data_formatted_phone_from_tps(data_util_state* state, data_formatted_phone* formattedPhone, tpselt tpsElement)
{
    NB_Error error = NE_OK;

    if (tpsElement == NULL)
    {
        error = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(state, error, formattedPhone, data_formatted_phone);

    data_string_from_tps_attr(state, &(formattedPhone->text), tpsElement, "text");

errexit:

    if (error != NE_OK)
    {
        DATA_FREE(state, formattedPhone, data_formatted_phone);
    }

    return error;
}

boolean
data_formatted_phone_equal(data_util_state* state, data_formatted_phone* formattedPhone1, data_formatted_phone* formattedPhone2)
{
    int result = TRUE;

    DATA_EQUAL(state, result, &(formattedPhone1->text), &(formattedPhone2->text), data_string);

    return (boolean) result;
}

NB_Error
data_formatted_phone_copy(data_util_state* state, data_formatted_phone* destinationFormattedPhone, data_formatted_phone* sourceFormattedPhone)
{
    NB_Error error = NE_OK;

    DATA_REINIT(state, error, destinationFormattedPhone, data_formatted_phone);
    DATA_COPY(state, error, &(destinationFormattedPhone->text), &(sourceFormattedPhone->text), data_string);

    if (error != NE_OK)
    {
        DATA_FREE(state, destinationFormattedPhone, data_formatted_phone);
    }

    return error;
}

uint32
data_formatted_phone_get_tps_size(data_util_state* state, data_formatted_phone* formattedPhone)
{
    uint32 size = 0;

    size += data_string_get_tps_size(state, &(formattedPhone->text));

    return size;
}

void
data_formatted_phone_to_buf(data_util_state* state, data_formatted_phone* formattedPhone, struct dynbuf* buf)
{
    data_string_to_buf(state, &(formattedPhone->text), buf);
}

NB_Error
data_formatted_phone_from_binary(data_util_state* state, data_formatted_phone* formattedPhone, byte** data, size_t* dataSize)
{
    NB_Error error = NE_OK;

    error = error ? error : data_string_from_binary(state, &(formattedPhone->text), data, dataSize);

    return error;
}

/*! @} */
