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

    @file   data_hours_of_operation_for_search.c
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

#include "data_hours_of_operation_for_search.h"
#include "data_time_range.h"

NB_Error
data_hours_of_operation_for_search_init(data_util_state* state, data_hours_of_operation_for_search* hoursOfOperation)
{
    NB_Error error = NE_OK;

    DATA_MEM_ZERO(hoursOfOperation, data_hours_of_operation_for_search);

    hoursOfOperation->hasUtcOffset = FALSE;
    hoursOfOperation->utc_offset = 0;
    DATA_VEC_ALLOC(error, hoursOfOperation->vector_time_range, data_time_range);

    return error;
}

void
data_hours_of_operation_for_search_free(data_util_state* state, data_hours_of_operation_for_search* hoursOfOperation)
{
    hoursOfOperation->hasUtcOffset = FALSE;
    hoursOfOperation->utc_offset = 0;
    DATA_VEC_FREE(state, hoursOfOperation->vector_time_range, data_time_range);
}

NB_Error
data_hours_of_operation_for_search_from_tps(data_util_state* state, data_hours_of_operation_for_search* hoursOfOperation, tpselt tpsElement)
{
    NB_Error error = NE_OK;
    tpselt childTpsElement = NULL;
    int iterator = 0;
    int utcOffsetResult = 0;
    char* utcOffsetString = NULL;
    size_t utcOffsetSize = 0;

    if (tpsElement == NULL)
    {
        error = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(state, error, hoursOfOperation, data_hours_of_operation_for_search);
    if (error != NE_OK)
    {
        goto errexit;
    }

    utcOffsetResult = te_getattr(tpsElement, "utc-offset", &utcOffsetString, &utcOffsetSize);
    if ((utcOffsetResult != 0) &&
        utcOffsetString &&
        (utcOffsetSize > 0))
    {
        hoursOfOperation->hasUtcOffset = TRUE;
        hoursOfOperation->utc_offset = nsl_atoi(utcOffsetString);
    }
    else
    {
        hoursOfOperation->hasUtcOffset = FALSE;
    }

    while ((childTpsElement = te_nextchild(tpsElement, &iterator)) != NULL)
    {
        if (nsl_strcmp(te_getname(childTpsElement), "time-range") == 0)
        {
            DATA_FROM_TPS_ADD_TO_VEC(state, error, childTpsElement, hoursOfOperation->vector_time_range, data_time_range);
        }

        if (error != NE_OK)
        {
            goto errexit;
        }
    }

errexit:

    if (error != NE_OK)
    {
        DATA_FREE(state, hoursOfOperation, data_hours_of_operation_for_search);
    }

    return error;
}

boolean
data_hours_of_operation_for_search_equal(data_util_state* state, data_hours_of_operation_for_search* hoursOfOperation1, data_hours_of_operation_for_search* hoursOfOperation2)
{
    int result = TRUE;

    if (hoursOfOperation1->hasUtcOffset && hoursOfOperation2->hasUtcOffset)
    {
        if (hoursOfOperation1->utc_offset != hoursOfOperation2->utc_offset)
        {
            return FALSE;
        }
    }
    else if (hoursOfOperation1->hasUtcOffset || hoursOfOperation2->hasUtcOffset)
    {
        return FALSE;
    }

    DATA_VEC_EQUAL(state, result, hoursOfOperation1->vector_time_range, hoursOfOperation2->vector_time_range, data_time_range);

    return (boolean) result;
}

NB_Error
data_hours_of_operation_for_search_copy(data_util_state* state, data_hours_of_operation_for_search* destinationHoursOfOperation, data_hours_of_operation_for_search* sourceHoursOfOperation)
{
    NB_Error error = NE_OK;

    DATA_REINIT(state, error, destinationHoursOfOperation, data_hours_of_operation_for_search);
    DATA_VEC_COPY(state, error, destinationHoursOfOperation->vector_time_range, sourceHoursOfOperation->vector_time_range, data_time_range);

    if (error == NE_OK)
    {
        destinationHoursOfOperation->hasUtcOffset = sourceHoursOfOperation->hasUtcOffset;
        destinationHoursOfOperation->utc_offset = sourceHoursOfOperation->utc_offset;
    }
    else
    {
        DATA_FREE(state, destinationHoursOfOperation, data_hours_of_operation_for_search);
    }

    return error;
}

/*! @} */
