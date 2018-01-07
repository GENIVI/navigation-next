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

    @file   data_extended_address.c
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

#include "data_extended_address.h"
#include "data_address_line.h"

static NB_Error data_extended_address_add_line(data_util_state* state, data_extended_address* extendedAddress, const char* lineText);

NB_Error
data_extended_address_init(data_util_state* state, data_extended_address* extendedAddress)
{
    NB_Error error = NE_OK;

    DATA_MEM_ZERO(extendedAddress, data_extended_address);

    DATA_VEC_ALLOC(error, extendedAddress->vector_address_line, data_address_line);

    return error;
}

void
data_extended_address_free(data_util_state* state, data_extended_address* extendedAddress)
{
    DATA_VEC_FREE(state, extendedAddress->vector_address_line, data_address_line);
}

NB_Error
data_extended_address_from_tps(data_util_state* state, data_extended_address* extendedAddress, tpselt tpsElement)
{
    NB_Error error = NE_OK;
    tpselt childTpsElement = NULL;
    int iterator = 0;

    if (tpsElement == NULL)
    {
        error = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(state, error, extendedAddress, data_extended_address);
    if (error != NE_OK)
    {
        goto errexit;
    }

    while ((childTpsElement = te_nextchild(tpsElement, &iterator)) != NULL)
    {
        if (nsl_strcmp(te_getname(childTpsElement), "address-line") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(state, error, childTpsElement, extendedAddress->vector_address_line, data_address_line);
        }

        if (error != NE_OK)
        {
            goto errexit;
        }
    }

errexit:

    if (error != NE_OK)
    {
        DATA_FREE(state, extendedAddress, data_extended_address);
    }

    return error;
}

boolean
data_extended_address_equal(data_util_state* state, data_extended_address* extendedAddress1, data_extended_address* extendedAddress2)
{
    int result = TRUE;

    DATA_VEC_EQUAL(state, result, extendedAddress1->vector_address_line, extendedAddress2->vector_address_line, data_address_line);

    return (boolean) result;
}

NB_Error
data_extended_address_copy(data_util_state* state, data_extended_address* destinationExtendedAddress, data_extended_address* sourceExtendedAddress)
{
    NB_Error error = NE_OK;

    DATA_REINIT(state, error, destinationExtendedAddress, data_extended_address);
    DATA_VEC_COPY(state, error, destinationExtendedAddress->vector_address_line, sourceExtendedAddress->vector_address_line, data_address_line);

    if (error != NE_OK)
    {
        DATA_FREE(state, destinationExtendedAddress, data_extended_address);
    }

    return error;
}

uint32
data_extended_address_get_tps_size(data_util_state* state, data_extended_address* extendedAddress)
{
    uint32 size = 0;

    DATA_VEC_GET_TPS_SIZE(state, size, extendedAddress->vector_address_line, data_address_line);

    return size;
}

void
data_extended_address_to_buf(data_util_state* state, data_extended_address* extendedAddress, struct dynbuf* buf)
{
    DATA_VEC_TO_BUF(state, buf, extendedAddress->vector_address_line, data_address_line);
}

NB_Error
data_extended_address_from_binary(data_util_state* state, data_extended_address* extendedAddress, byte** data, size_t* dataSize)
{
    NB_Error error = NE_OK;

    DATA_VEC_FROM_BINARY(state, error, data, dataSize, extendedAddress->vector_address_line, data_address_line);

    return error;
}

NB_Error
data_extended_address_from_nimlocation(data_util_state* state, data_extended_address* extendedAddress, const NB_Location* location)
{
    NB_Error error = NE_OK;
    int lineCount = 0;

    if ((!state) ||
        (!extendedAddress) ||
        (!location))
    {
        return NE_INVAL;
    }

    DATA_REINIT(state, error, extendedAddress, data_extended_address);

    lineCount = location->extendedAddress.lineCount;
    if (lineCount > 0)
    {
        if ((--lineCount) >= 0)
        {
            error = error ? error : data_extended_address_add_line(state, extendedAddress, location->extendedAddress.addressLine1);
        }
        if ((--lineCount) >= 0)
        {
            error = error ? error : data_extended_address_add_line(state, extendedAddress, location->extendedAddress.addressLine2);
        }
        if ((--lineCount) >= 0)
        {
            error = error ? error : data_extended_address_add_line(state, extendedAddress, location->extendedAddress.addressLine3);
        }
        if ((--lineCount) >= 0)
        {
            error = error ? error : data_extended_address_add_line(state, extendedAddress, location->extendedAddress.addressLine4);
        }
        if ((--lineCount) >= 0)
        {
            error = error ? error : data_extended_address_add_line(state, extendedAddress, location->extendedAddress.addressLine5);
        }
    }

    if (error != NE_OK)
    {
        DATA_FREE(state, extendedAddress, data_extended_address);
    }

    return error;
}

NB_Error
data_extended_address_to_nimlocation(data_util_state* state, data_extended_address* extendedAddress, NB_Location* location)
{
    int i = 0;
    int lineCount = 0;
    CSL_Vector* addressLineVector = NULL;
    data_address_line* addressLine = NULL;

    if ((!state) || (!extendedAddress) || (!location))
    {
        return NE_INVAL;
    }

    location->extendedAddress.lineCount = 0;
    addressLineVector = extendedAddress->vector_address_line;
    if (addressLineVector && ((lineCount = CSL_VectorGetLength(addressLineVector)) > 0))
    {
        lineCount = (lineCount <= NB_EXTENDED_ADDRESS_MAX_ADDRESS_LINE_COUNT) ?
                    lineCount :
                    NB_EXTENDED_ADDRESS_MAX_ADDRESS_LINE_COUNT;
        location->extendedAddress.lineCount = lineCount;

        if ((--lineCount) >= 0)
        {
            addressLine = (data_address_line*) CSL_VectorGetPointer(addressLineVector, i++);
            if (addressLine)
            {
                data_string_get_copy(state,
                                     &(addressLine->text),
                                     location->extendedAddress.addressLine1,
                                     sizeof(location->extendedAddress.addressLine1));
            }
        }

        if ((--lineCount) >= 0)
        {
            addressLine = (data_address_line*) CSL_VectorGetPointer(addressLineVector, i++);
            if (addressLine)
            {
                data_string_get_copy(state,
                                     &(addressLine->text),
                                     location->extendedAddress.addressLine2,
                                     sizeof(location->extendedAddress.addressLine2));
            }
        }

        if ((--lineCount) >= 0)
        {
            addressLine = (data_address_line*) CSL_VectorGetPointer(addressLineVector, i++);
            if (addressLine)
            {
                data_string_get_copy(state,
                                     &(addressLine->text),
                                     location->extendedAddress.addressLine3,
                                     sizeof(location->extendedAddress.addressLine3));
            }
        }

        if ((--lineCount) >= 0)
        {
            addressLine = (data_address_line*) CSL_VectorGetPointer(addressLineVector, i++);
            if (addressLine)
            {
                data_string_get_copy(state,
                                     &(addressLine->text),
                                     location->extendedAddress.addressLine4,
                                     sizeof(location->extendedAddress.addressLine4));
            }
        }

        if ((--lineCount) >= 0)
        {
            addressLine = (data_address_line*) CSL_VectorGetPointer(addressLineVector, i++);
            if (addressLine)
            {
                data_string_get_copy(state,
                                     &(addressLine->text),
                                     location->extendedAddress.addressLine5,
                                     sizeof(location->extendedAddress.addressLine5));
            }
        }
    }

    return NE_OK;
}

NB_Error
data_extended_address_add_line(data_util_state* state, data_extended_address* extendedAddress, const char* lineText)
{
    NB_Error error = NE_OK;
    data_address_line addressLine = {0};

    if ((!state) ||
        (!extendedAddress) ||
        (!lineText) ||
        (!(extendedAddress->vector_address_line)))
    {
        return NE_INVAL;
    }

    DATA_INIT(state, error, &addressLine, data_address_line);

    error = error ? error : data_string_set(state, &(addressLine.text), lineText);
    error = error ? error : (CSL_VectorAppend(extendedAddress->vector_address_line, &addressLine) ? NE_OK : NE_NOMEM);

    if (error != NE_OK)
    {
        DATA_FREE(state, &addressLine, data_address_line);
    }

    return error;
}

/*! @} */
