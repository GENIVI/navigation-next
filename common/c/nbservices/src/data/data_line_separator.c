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

    @file   data_line_separator.c
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

#include "data_line_separator.h"

NB_Error
data_line_separator_init(data_util_state* state, data_line_separator* lineSeparator)
{
    NB_Error error = NE_OK;

    DATA_MEM_ZERO(lineSeparator, data_line_separator);

    DATA_INIT(state, error, &(lineSeparator->separator), data_string);

    return error;
}

void
data_line_separator_free(data_util_state* state, data_line_separator* lineSeparator)
{
    DATA_FREE(state,  &(lineSeparator->separator), data_string);
}

NB_Error
data_line_separator_from_tps(data_util_state* state, data_line_separator* lineSeparator, tpselt tpsElement)
{
    NB_Error error = NE_OK;

    if (tpsElement == NULL)
    {
        error = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(state, error, lineSeparator, data_line_separator);

    if (error != NE_OK)
    {
        goto errexit;
    }

    if (te_getattrc(tpsElement, "separator") != NULL)
    {
        error = error ? error : data_string_from_tps_attr(state, &(lineSeparator->separator), tpsElement, "separator");
    }

errexit:

    if (error != NE_OK)
    {
        DATA_FREE(state, lineSeparator, data_line_separator);
    }

    return error;
}

boolean
data_line_separator_equal(data_util_state* state, data_line_separator* lineSeparator1, data_line_separator* lineSeparator2)
{
    int result = TRUE;

    DATA_EQUAL(state, result, &(lineSeparator1->separator), &(lineSeparator2->separator), data_string);

    return (boolean) result;
}

NB_Error
data_line_separator_copy(data_util_state* state, data_line_separator* destinationLineSeparator, data_line_separator* sourceLineSeparator)
{
    NB_Error error = NE_OK;

    DATA_REINIT(state, error, destinationLineSeparator, data_line_separator);
    DATA_COPY(state, error, &(destinationLineSeparator->separator), &(sourceLineSeparator->separator), data_string);

    if (error != NE_OK)
    {
        DATA_FREE(state, destinationLineSeparator, data_line_separator);
    }

    return error;
}

uint32
data_line_separator_get_tps_size(data_util_state* state, data_line_separator* lineSeparator)
{
    uint32 size = 0;

    size += data_string_get_tps_size(state, &(lineSeparator->separator));

    return size;
}

void
data_line_separator_to_buf(data_util_state* state, data_line_separator* lineSeparator, struct dynbuf* buf)
{
    data_string_to_buf(state, &(lineSeparator->separator), buf);
}

NB_Error
data_line_separator_from_binary(data_util_state* state, data_line_separator* lineSeparator, byte** data, size_t* dataSize)
{
    NB_Error error = NE_OK;

    error = error ? error : data_string_from_binary(state, &(lineSeparator->separator), data, dataSize);

    return error;
}

/*! @} */
