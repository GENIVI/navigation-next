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

   @file     data_feedback_event.h
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, Inc. is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef DATA_FEEDBACK_EVENT_
#define DATA_FEEDBACK_EVENT_

#include "nbexp.h"
#include "datautil.h"
#include "data_place.h"
#include "data_origin.h"
#include "data_destination.h"
#include "data_string.h"
#include "data_text.h"
#include "data_issue_type.h"


typedef struct data_feedback_event_ {
    /* Child Elements */
    data_place*         ppoi_place;
    data_origin*        pnav_origin;
    data_destination*   pnav_destination;
    data_text           entered_text;
    CSL_Vector*         vec_issue_type;

    /* Attributes */
    data_string         screen_id;
    uint32              nav_session_id;
    uint32              route_request_event_id;
    uint32              search_query_event_id;
    data_string         provider_id;
    data_string         poi_id;
} data_feedback_event;

NB_Error data_feedback_event_init(data_util_state* pds,
            data_feedback_event* pfe);

void data_feedback_event_free(data_util_state* pds,
            data_feedback_event* pfe);

tpselt data_feedback_event_to_tps(data_util_state* pds,
            data_feedback_event* pfe);

boolean data_feedback_event_equal(data_util_state* pds,
            data_feedback_event* pfe1, data_feedback_event* pfe2);

NB_Error data_feedback_event_copy(data_util_state* pds,
            data_feedback_event* dst,
            data_feedback_event* src);

NB_Error data_feedback_event_add_issue_type(data_util_state* pds,
            data_feedback_event* pfe, const char* val);

uint32  data_feedback_event_get_tps_size(data_util_state* pds,
            data_feedback_event* pfe);

void    data_feedback_event_to_buf(data_util_state* pds,
            data_feedback_event* pfe,
            struct dynbuf* pdb);

NB_Error    data_feedback_event_from_binary(data_util_state* pds,
            data_feedback_event* pfe,
            byte** pdata, size_t* pdatalen);

#endif

/*! @} */
