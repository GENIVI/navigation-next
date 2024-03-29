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

    @file   data_hours_of_operation_for_search.h
*/
/*
    (C) Copyright 2013 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.
---------------------------------------------------------------------------*/

#ifndef DATA_HOURS_OF_OPERATION_FOR_SEARCH_H
#define DATA_HOURS_OF_OPERATION_FOR_SEARCH_H

/*! @{ */

#include "datautil.h"
#include "vec.h"

typedef struct data_hours_of_operation_for_search_
{
    /* Child Elements */
    CSL_Vector* vector_time_range;      /*!< Vector of time range */

    /* Attributes */
    boolean hasUtcOffset;               /*!< Is the UTC offset present? */
    int utc_offset;                     /*!< Offset in seconds from UTC of POI local time, can be negative. */

} data_hours_of_operation_for_search;

NB_Error data_hours_of_operation_for_search_init(data_util_state* state, data_hours_of_operation_for_search* hoursOfOperation);
void data_hours_of_operation_for_search_free(data_util_state* state, data_hours_of_operation_for_search* hoursOfOperation);

NB_Error data_hours_of_operation_for_search_from_tps(data_util_state* state, data_hours_of_operation_for_search* hoursOfOperation, tpselt tpsElement);

boolean data_hours_of_operation_for_search_equal(data_util_state* state, data_hours_of_operation_for_search* hoursOfOperation1, data_hours_of_operation_for_search* hoursOfOperation2);
NB_Error data_hours_of_operation_for_search_copy(data_util_state* state, data_hours_of_operation_for_search* destinationHoursOfOperation, data_hours_of_operation_for_search* sourceHoursOfOperation);

/*! @} */

#endif
