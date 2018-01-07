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

    @file   data_time_range.c
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

#include "data_time_range.h"

NB_Error
data_time_range_init(data_util_state* state, data_time_range* timeRange)
{
    DATA_MEM_ZERO(timeRange, data_time_range);

    timeRange->day_of_week = 0;
    timeRange->start_time = 0;
    timeRange->end_time = 0;

    return NE_OK;
}

void
data_time_range_free(data_util_state* state, data_time_range* timeRange)
{
    timeRange->day_of_week = 0;
    timeRange->start_time = 0;
    timeRange->end_time = 0;
}

NB_Error
data_time_range_from_tps(data_util_state* state, data_time_range* timeRange, tpselt tpsElement)
{
    NB_Error error = NE_OK;

    if (tpsElement == NULL)
    {
        error = NE_INVAL;
        goto errexit;
    }

    DATA_REINIT(state, error, timeRange, data_time_range);
    if (error != NE_OK)
    {
        goto errexit;
    }

    timeRange->day_of_week = te_getattru(tpsElement, "day-of-week");
    timeRange->start_time = te_getattru(tpsElement, "start-time");
    timeRange->end_time = te_getattru(tpsElement, "end-time");

errexit:

    if (error != NE_OK)
    {
        DATA_FREE(state, timeRange, data_time_range);
    }

    return error;
}

boolean
data_time_range_equal(data_util_state* state, data_time_range* timeRange1, data_time_range* timeRange2)
{
    return (boolean) ((timeRange1->day_of_week == timeRange2->day_of_week) &&
                      (timeRange1->start_time == timeRange2->start_time) &&
                      (timeRange1->end_time == timeRange2->end_time));
}

NB_Error
data_time_range_copy(data_util_state* state, data_time_range* destinationTimeRange, data_time_range* sourceTimeRange)
{
    NB_Error error = NE_OK;

    DATA_REINIT(state, error, destinationTimeRange, data_time_range);

    if (error == NE_OK)
    {
        destinationTimeRange->day_of_week = sourceTimeRange->day_of_week;
        destinationTimeRange->start_time = sourceTimeRange->start_time;
        destinationTimeRange->end_time = sourceTimeRange->end_time;
    }
    else
    {
        DATA_FREE(state, destinationTimeRange, data_time_range);
    }

    return error;
}

/*! @} */
