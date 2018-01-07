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

    @file     data_reveal_review_event.c
*/
/*
    See file description in header file.

 (C) Copyright 2012 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */
#include "data_reveal_review_event.h"

NB_Error
data_reveal_review_event_init(data_util_state* dataState, data_reveal_review_event* revealReviewEvent)
{
    NB_Error err = NE_OK;

    DATA_INIT(dataState, err, &revealReviewEvent->analytics_event_place, data_analytics_event_place);

    return err;
}

void
data_reveal_review_event_free(data_util_state* dataState, data_reveal_review_event* revealReviewEvent)
{
    DATA_FREE(dataState, &revealReviewEvent->analytics_event_place, data_analytics_event_place);
}

tpselt
data_reveal_review_event_to_tps(data_util_state* dataState, data_reveal_review_event* revealReviewEvent)
{
    tpselt te = NULL;
    tpselt ce = NULL;

    te = te_new("reveal-review-event");

    if (te == NULL)
    {
        goto errexit;
    }

    ce = data_analytics_event_place_to_tps(dataState, &revealReviewEvent->analytics_event_place);
    if ((ce != NULL) && te_attach(te, ce))
    {
        ce = NULL;
    }
    else
    {
        goto errexit;
    }

    return te;

errexit:

    te_dealloc(te);
    return NULL;
}

boolean
data_reveal_review_event_equal(data_util_state* dataState,
                               data_reveal_review_event* revealReviewEvent1,
                               data_reveal_review_event* revealReviewEvent2)
{
    return (boolean) data_analytics_event_place_equal(dataState,
            &revealReviewEvent1->analytics_event_place, &revealReviewEvent2->analytics_event_place);
}

NB_Error
data_reveal_review_event_copy(data_util_state* dataState,
                              data_reveal_review_event* revealReviewEventDest,
                              data_reveal_review_event* revealReviewEventSrc)
{
    NB_Error err = NE_OK;

    DATA_REINIT(dataState, err, revealReviewEventDest, data_reveal_review_event);

    DATA_COPY(dataState, err, &revealReviewEventDest->analytics_event_place,
            &revealReviewEventSrc->analytics_event_place, data_analytics_event_place);

    return err;
}

uint32
data_reveal_review_event_get_tps_size(data_util_state* dataState, data_reveal_review_event* revealReviewEvent)
{
    return data_analytics_event_place_get_tps_size(dataState,
            &revealReviewEvent->analytics_event_place);
}

void
data_reveal_review_event_to_buf(data_util_state* dataState,
                                data_reveal_review_event* revealReviewEvent,
                                struct dynbuf* buffer)
{
    data_analytics_event_place_to_buf(dataState, &revealReviewEvent->analytics_event_place, buffer);
}

NB_Error
data_reveal_review_event_from_binary(data_util_state* dataState,
                                     data_reveal_review_event* revealReviewEvent,
                                     byte** pdata, size_t* pdatalen)
{
    return data_analytics_event_place_from_binary(dataState,
            &revealReviewEvent->analytics_event_place, pdata, pdatalen);
}

/*! @} */

