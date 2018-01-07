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

    @file     data_arrival_event.c
*/
/*
    See file description in header file.

    (C) Copyright 2004 - 2009 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */
#include "data_arrival_event.h"

NB_Error
data_arrival_event_init(data_util_state* dataState, data_arrival_event* arrivalEvent)
{
    NB_Error err = NE_OK;

    DATA_INIT(dataState, err, &arrivalEvent->analytics_event_place, data_analytics_event_place);

    arrivalEvent->session_id = 0;
    arrivalEvent->route_request_event_id = 0;
    DATA_INIT(dataState, err, &arrivalEvent->route_id, data_blob);

    return err;
}

void
data_arrival_event_free(data_util_state* dataState, data_arrival_event* arrivalEvent)
{
    DATA_FREE(dataState, &arrivalEvent->analytics_event_place, data_analytics_event_place);
    DATA_FREE(dataState, &arrivalEvent->route_id, data_blob);
}

tpselt
data_arrival_event_to_tps(data_util_state* dataState, data_arrival_event* arrivalEvent)
{
    tpselt te = NULL;
    tpselt ce = NULL;

    te = te_new("arrival-event");

    if (te == NULL)
    {
        goto errexit;
    }

    ce = data_analytics_event_place_to_tps(dataState, &arrivalEvent->analytics_event_place);
    if ((ce != NULL) && te_attach(te, ce))
    {
        ce = NULL;
    }
    else
    {
        goto errexit;
    }

    if (!te_setattru(te, "nav-session-id", arrivalEvent->session_id))
    {
        goto errexit;
    }
    if (!te_setattru(te, "route-request-event-id", arrivalEvent->route_request_event_id))
    {
        goto errexit;
    }
    if (!te_setattr(te, "route-id", (const char*)arrivalEvent->route_id.data, arrivalEvent->route_id.size))
    {
        goto errexit;
    }

    return te;

errexit:

    te_dealloc(te);
    return NULL;
}

boolean
data_arrival_event_equal(data_util_state* dataState,
                         data_arrival_event* arrivalEvent1,
                         data_arrival_event* arrivalEvent2)
{
    return (boolean) (data_analytics_event_place_equal(dataState,
            &arrivalEvent1->analytics_event_place, &arrivalEvent2->analytics_event_place) &&
            arrivalEvent1->session_id == arrivalEvent2->session_id &&
            arrivalEvent1->route_request_event_id == arrivalEvent2->route_request_event_id &&
            data_blob_equal(dataState, &arrivalEvent1->route_id, &arrivalEvent2->route_id));
}

NB_Error
data_arrival_event_copy(data_util_state* dataState,
                        data_arrival_event* arrivalEventDest,
                        data_arrival_event* arrivalEventSrc)
{
    NB_Error err = NE_OK;

    DATA_REINIT(dataState, err, arrivalEventDest, data_arrival_event);

    DATA_COPY(dataState, err, &arrivalEventDest->analytics_event_place,
            &arrivalEventSrc->analytics_event_place, data_analytics_event_place);

    arrivalEventDest->session_id = arrivalEventSrc->session_id;
    arrivalEventDest->route_request_event_id = arrivalEventSrc->route_request_event_id;
    DATA_COPY(dataState, err, &arrivalEventDest->route_id,
            &arrivalEventSrc->route_id, data_blob);

    return err;
}

uint32
data_arrival_event_get_tps_size(data_util_state* dataState, data_arrival_event* arrivalEvent)
{
    uint32 size = 0;
    size = data_analytics_event_place_get_tps_size(dataState,
            &arrivalEvent->analytics_event_place);
    size += sizeof(arrivalEvent->session_id);
    size += sizeof(arrivalEvent->route_request_event_id);
    size += arrivalEvent->route_id.size;
    return size;
}

void
data_arrival_event_to_buf(data_util_state* dataState,
                          data_arrival_event* arrivalEvent,
                          struct dynbuf* buffer)
{
    data_analytics_event_place_to_buf(dataState, &arrivalEvent->analytics_event_place, buffer);

    dbufcat(buffer, (const byte*) &arrivalEvent->session_id,
            sizeof(arrivalEvent->session_id));
    dbufcat(buffer, (const byte*) &arrivalEvent->route_request_event_id,
            sizeof(arrivalEvent->route_request_event_id));
    data_blob_to_buf(dataState, &arrivalEvent->route_id, buffer);
}

NB_Error
data_arrival_event_from_binary(data_util_state* dataState,
                               data_arrival_event* arrivalEvent,
                               byte** pdata, size_t* pdatalen)
{
    NB_Error err = data_analytics_event_place_from_binary(dataState, &arrivalEvent->analytics_event_place, pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(dataState, &arrivalEvent->session_id,
                                              pdata, pdatalen);
    err = err ? err : data_uint32_from_binary(dataState, &arrivalEvent->route_request_event_id,
                                              pdata, pdatalen);
    err = err ? err : data_blob_from_binary(dataState, &arrivalEvent->route_id,
                                              pdata, pdatalen);
    return err;
}

/*! @} */

