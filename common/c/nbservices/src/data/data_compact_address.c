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

    @file   data_compact_address.c
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

#include "data_compact_address.h"
#include "data_address_line.h"

static NB_Error data_compact_address_set_separator(data_util_state* state, data_compact_address* compactAddress, const char* sperator);
static NB_Error data_compact_address_add_line(data_util_state* state, data_compact_address* compactAddress, const char* line);

NB_Error
data_compact_address_init(data_util_state* state, data_compact_address* compactAddress)
{
    NB_Error error = NE_OK;

    DATA_MEM_ZERO(compactAddress, data_compact_address);

    compactAddress->hasLineSeparator = FALSE;
    DATA_INIT(state, error, &(compactAddress->line_separator), data_line_separator);
    DATA_VEC_ALLOC(error, compactAddress->vector_address_line, data_address_line);

    return error;
}

void
data_compact_address_free(data_util_state* state, data_compact_address* compactAddress)
{
    compactAddress->hasLineSeparator = FALSE;
    DATA_FREE(state, &(compactAddress->line_separator), data_line_separator);
    DATA_VEC_FREE(state, compactAddress->vector_address_line, data_address_line);
}

NB_Error
data_compact_address_from_tps(data_util_state* state, data_compact_address* compactAddress, tpselt tpsElement)
{
    NB_Error error = NE_OK;
    tpselt childTpsElement = NULL;
    int iterator = 0;

    if (tpsElement == NULL)
    {
        error = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(state, error, compactAddress, data_compact_address);
    if (error != NE_OK)
    {
        goto errexit;
    }

    while ((childTpsElement = te_nextchild(tpsElement, &iterator)) != NULL)
    {
        if (nsl_strcmp(te_getname(childTpsElement), "line-separator") == 0)
        {
            DATA_FROM_TPS(state, error, childTpsElement, &(compactAddress->line_separator), data_line_separator);
            compactAddress->hasLineSeparator = (error == NE_OK) ? TRUE : FALSE;
        }
        else if (nsl_strcmp(te_getname(childTpsElement), "address-line") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(state, error, childTpsElement, compactAddress->vector_address_line, data_address_line);
        }

        if (error != NE_OK)
        {
            goto errexit;
        }
    }

errexit:

    if (error != NE_OK)
    {
        DATA_FREE(state, compactAddress, data_compact_address);
    }

    return error;
}

boolean
data_compact_address_equal(data_util_state* state, data_compact_address* compactAddress1, data_compact_address* compactAddress2)
{
    int result = TRUE;

    if (compactAddress1->hasLineSeparator && compactAddress2->hasLineSeparator)
    {
        DATA_EQUAL(state, result, &(compactAddress1->line_separator), &(compactAddress2->line_separator), data_line_separator);
    }
    else if (compactAddress1->hasLineSeparator || compactAddress2->hasLineSeparator)
    {
        return FALSE;
    }

    DATA_VEC_EQUAL(state, result, compactAddress1->vector_address_line, compactAddress2->vector_address_line, data_address_line);

    return (boolean) result;
}

NB_Error
data_compact_address_copy(data_util_state* state, data_compact_address* destinationCompactAddress, data_compact_address* sourceCompactAddress)
{
    NB_Error error = NE_OK;

    DATA_REINIT(state, error, destinationCompactAddress, data_compact_address);
    DATA_COPY(state, error, &(destinationCompactAddress->line_separator), &(sourceCompactAddress->line_separator), data_line_separator);
    DATA_VEC_COPY(state, error, destinationCompactAddress->vector_address_line, sourceCompactAddress->vector_address_line, data_address_line);

    if (error == NE_OK)
    {
        destinationCompactAddress->hasLineSeparator = sourceCompactAddress->hasLineSeparator;
    }
    else
    {
        DATA_FREE(state, destinationCompactAddress, data_compact_address);
    }

    return error;
}

uint32
data_compact_address_get_tps_size(data_util_state* state, data_compact_address* compactAddress)
{
    uint32 size = 0;

    size += sizeof(compactAddress->hasLineSeparator);
    size += data_line_separator_get_tps_size(state, &(compactAddress->line_separator));
    DATA_VEC_GET_TPS_SIZE(state, size, compactAddress->vector_address_line, data_address_line);

    return size;
}

void
data_compact_address_to_buf(data_util_state* state, data_compact_address* compactAddress, struct dynbuf* buf)
{
    dbufcat(buf, (const byte*) (&(compactAddress->hasLineSeparator)), sizeof(compactAddress->hasLineSeparator));
    data_line_separator_to_buf(state, &(compactAddress->line_separator), buf);
    DATA_VEC_TO_BUF(state, buf, compactAddress->vector_address_line, data_address_line);
}

NB_Error
data_compact_address_from_binary(data_util_state* state, data_compact_address* compactAddress, byte** data, size_t* dataSize)
{
    NB_Error error = NE_OK;

    error = error ? error : data_boolean_from_binary(state, &(compactAddress->hasLineSeparator), data, dataSize);
    error = error ? error : data_line_separator_from_binary(state, &(compactAddress->line_separator), data, dataSize);
    DATA_VEC_FROM_BINARY(state, error, data, dataSize, compactAddress->vector_address_line, data_address_line);

    return error;
}

NB_Error
data_compact_address_from_nimlocation(data_util_state* state, data_compact_address* compactAddress, const NB_Location* location)
{
    NB_Error error = NE_OK;
    int lineCount = 0;

    if ((!state) ||
        (!compactAddress) ||
        (!location))
    {
        return NE_INVAL;
    }

    DATA_REINIT(state, error, compactAddress, data_compact_address);

    compactAddress->hasLineSeparator = location->compactAddress.hasLineSeparator;
    if (compactAddress->hasLineSeparator)
    {
        error = error ? error : data_compact_address_set_separator(state, compactAddress, location->compactAddress.lineSeparator);
    }

    lineCount = location->compactAddress.lineCount;
    if (lineCount > 0)
    {
        if ((--lineCount) >= 0)
        {
            error = error ? error : data_compact_address_add_line(state, compactAddress, location->compactAddress.addressLine1);
        }
        if ((--lineCount) >= 0)
        {
            error = error ? error : data_compact_address_add_line(state, compactAddress, location->compactAddress.addressLine2);
        }
    }

    if (error != NE_OK)
    {
        DATA_FREE(state, compactAddress, data_compact_address);
    }

    return error;
}

NB_Error
data_compact_address_to_nimlocation(data_util_state* state, data_compact_address* compactAddress, NB_Location* location)
{
    nb_boolean hasLineSeparator = FALSE;
    int i = 0;
    int lineCount = 0;
    CSL_Vector* addressLineVector = NULL;
    data_address_line* addressLine = NULL;

    if ((!state) || (!compactAddress) || (!location))
    {
        return NE_INVAL;
    }

    hasLineSeparator = (compactAddress->hasLineSeparator) ? TRUE : FALSE;
    location->compactAddress.hasLineSeparator = hasLineSeparator;
    if (hasLineSeparator)
    {
        data_string_get_copy(state,
                             &(compactAddress->line_separator.separator),
                             location->compactAddress.lineSeparator,
                             sizeof(location->compactAddress.lineSeparator));
    }

    location->compactAddress.lineCount = 0;
    addressLineVector = compactAddress->vector_address_line;
    if (addressLineVector && ((lineCount = CSL_VectorGetLength(addressLineVector)) > 0))
    {
        lineCount = (lineCount <= NB_COMPACT_ADDRESS_MAX_ADDRESS_LINE_COUNT) ?
                    lineCount :
                    NB_COMPACT_ADDRESS_MAX_ADDRESS_LINE_COUNT;
        location->compactAddress.lineCount = lineCount;

        if ((--lineCount) >= 0)
        {
            addressLine = (data_address_line*) CSL_VectorGetPointer(addressLineVector, i++);
            if (addressLine)
            {
                data_string_get_copy(state,
                                     &(addressLine->text),
                                     location->compactAddress.addressLine1,
                                     sizeof(location->compactAddress.addressLine1));
            }
        }

        if ((--lineCount) >= 0)
        {
            addressLine = (data_address_line*) CSL_VectorGetPointer(addressLineVector, i++);
            if (addressLine)
            {
                data_string_get_copy(state,
                                     &(addressLine->text),
                                     location->compactAddress.addressLine2,
                                     sizeof(location->compactAddress.addressLine2));
            }
        }
    }

    return NE_OK;
}

NB_Error
data_compact_address_set_separator(data_util_state* state, data_compact_address* compactAddress, const char* sperator)
{
    NB_Error error = NE_OK;

    if ((!state) ||
        (!compactAddress) ||
        (!sperator))
    {
        return NE_INVAL;
    }

    error = data_string_set(state, &(compactAddress->line_separator.separator), sperator);
    if (error == NE_OK)
    {
        compactAddress->hasLineSeparator = TRUE;
    }

    return error;
}

NB_Error
data_compact_address_add_line(data_util_state* state, data_compact_address* compactAddress, const char* lineText)
{
    NB_Error error = NE_OK;
    data_address_line addressLine = {0};

    if ((!state) ||
        (!compactAddress) ||
        (!lineText) ||
        (!(compactAddress->vector_address_line)))
    {
        return NE_INVAL;
    }

    DATA_INIT(state, error, &addressLine, data_address_line);

    error = error ? error : data_string_set(state, &(addressLine.text), lineText);
    error = error ? error : (CSL_VectorAppend(compactAddress->vector_address_line, &addressLine) ? NE_OK : NE_NOMEM);

    if (error != NE_OK)
    {
        DATA_FREE(state, &addressLine, data_address_line);
    }

    return error;
}

/*! @} */
