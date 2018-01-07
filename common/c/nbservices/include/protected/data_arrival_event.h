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

    @file     data_arrival_event.h

    Interface to create Arrival-Event TPS element.

*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */
#ifndef DATA_ARRIVAL_EVENT_
#define DATA_ARRIVAL_EVENT_

#include "nbexp.h"
#include "datautil.h"
#include "dynbuf.h"
#include "data_analytics_event_place.h"

typedef struct data_arrival_event_ {
    /* Child Elements */
    data_analytics_event_place analytics_event_place;  /*!< The place is arrived. */

    /* Attributes */
    uint32                     session_id;             /*!< Unique ID for this nav. session. */
    uint32                     route_request_event_id; /*!< Rotue request event Id.  */
    data_blob                  route_id;               /*!< Route Id.*/
} data_arrival_event;

NB_Error data_arrival_event_init(data_util_state* dataState, data_arrival_event* arrivalEvent);

void data_arrival_event_free(data_util_state* dataState, data_arrival_event* arrivalEvent);

tpselt data_arrival_event_to_tps(data_util_state* dataState, data_arrival_event* arrivalEvent);

boolean data_arrival_event_equal(data_util_state* dataState,
                                 data_arrival_event* arrivalEvent1,
                                 data_arrival_event* arrivalEvent2);

NB_Error data_arrival_event_copy(data_util_state* dataState,
                                 data_arrival_event* arrivalEventDest,
                                 data_arrival_event* arrivalEventSrc);

uint32 data_arrival_event_get_tps_size(data_util_state* dataState, data_arrival_event* arrivalEvent);

void data_arrival_event_to_buf(data_util_state* dataState,
                               data_arrival_event* arrivalEvent,
                               struct dynbuf* buffer);

NB_Error data_arrival_event_from_binary(data_util_state* dataState,
                                        data_arrival_event* arrivalEvent,
                                        byte** pdata, size_t* pdatalen);

#endif
/*! @} */

