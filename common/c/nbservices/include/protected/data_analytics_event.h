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

    @file     data_analytics_event.h

    Interface to create Analytics-Event TPS element.

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
#ifndef DATA_ANALYTICS_EVENT_
#define DATA_ANALYTICS_EVENT_

#include "nbexp.h"
#include "datautil.h"
#include "dynbuf.h"
#include "data_search_query_event.h"
#include "data_search_detail_event.h"
#include "data_map_event.h"
#include "data_place_message_event.h"
#include "data_call_event.h"
#include "data_add_favorites_event.h"
#include "data_route_request_event.h"
#include "data_route_reply_event.h"
#include "data_route_state_event.h"
#include "data_gps_probes_event.h"
#include "data_impression_event.h"
#include "data_facebook_update_event.h"
#include "data_wifi_probes_event.h"
#include "data_coupon_analytics_action_event.h"
#include "data_nav_invocation_event.h"
#include "data_feedback_event.h"
#include "data_app_error_event.h"
#include "data_arrival_event.h"
#include "data_web_url_event.h"
#include "data_asr_event.h"
#include "data_reveal_review_event.h"
#include "data_reveal_description_event.h"
#include "data_poiext_event.h"
#include "data_invocation_context.h"

typedef enum analytics_event_type_{
    /*Whenever needs to add a new events please appened it in the end of the enum dont insert it in between.*/
    analytics_event_none = 0,

    analytics_event_search_query,
    analytics_event_search_detail,
    analytics_event_map,
    analytics_event_place_message,
    analytics_event_call,
    analytics_event_add_favorites,
    analytics_event_route_request,
    analytics_event_route_reply,
    analytics_event_route_state,
    analytics_event_gps_probes,
    analytics_event_impression,
    analytics_event_facebook_update,
    analytics_event_wifi_probes,
    analytics_event_coupon_analytics_action,
    analytics_event_nav_invocation,
    analytics_event_feedback,
    analytics_event_app_error,
    analytics_event_arrival,
    analytics_event_web_url,
    analytics_event_asr,
    analytics_event_reveal_review,
    analytics_event_reveal_description,
    analytics_event_poiext,
    analytics_event_max
} analytics_event_type;

typedef struct data_analytics_event_ {
    /* Child Elements */
    union {
        data_search_query_event     search_query_event;
        data_search_detail_event    search_detail_event;
        data_map_event              map_event;
        data_place_message_event    place_message_event;
        data_call_event             call_event;
        data_arrival_event          arrival_event;
        data_web_url_event          web_url_event;
        data_add_favorites_event    add_favorites_event;
        data_route_request_event    route_request_event;
        data_route_reply_event      route_reply_event;
        data_route_state_event      route_state_event;
        data_gps_probes_event       gps_probes_event;
        data_impression_event       impression_event;
        data_facebook_update_event  facebook_update_event;
        data_wifi_probes_event      wifi_probes_event;
        data_coupon_analytics_action_event  coupon_analytics_action_event;
        data_nav_invocation_event   nav_invocation_event;
        data_feedback_event         feedback_event;
        data_app_error_event       app_error_event;
        data_asr_event              asr_event;
        data_reveal_review_event    reveal_review_event;
        data_reveal_description_event reveal_description_event;
        data_poiext_event           poiext_event;
    } event;

    data_invocation_context         invocation_context;

    /* Attributes */
    data_string active_session;
    uint32  id;     /*!< Unique ID for this event within this session. */
    uint32  ts;     /*!< The timestamp for the event represented as GPS time. */
    uint32  session_id; /*!< Unique session id for this client. */

    /* internal state */
    analytics_event_type event_type; /* type of event stored in union event. */
} data_analytics_event;

NB_Error data_analytics_event_init(data_util_state* pds,
            data_analytics_event* pae);

void data_analytics_event_free(data_util_state* pds, data_analytics_event* pae);

tpselt data_analytics_event_to_tps(data_util_state* pds,
            data_analytics_event* pae);

boolean data_analytics_event_equal(data_util_state* pds,
        data_analytics_event* pae1, data_analytics_event* pae2);

NB_Error data_analytics_event_copy(data_util_state* pds,
        data_analytics_event* pae_dest, data_analytics_event* pae_src);

uint32 data_analytics_event_get_tps_size(data_util_state* pds,
        data_analytics_event* pae);

NB_Error data_analytics_event_set_event_type(data_util_state* pds,
        data_analytics_event* pae, analytics_event_type event_type);

void data_analytics_event_to_buf(data_util_state* pds,
            data_analytics_event* pae,
            struct dynbuf* pdb);

NB_Error data_analytics_event_from_binary(data_util_state* pds,
            data_analytics_event* pae,
            byte** pdata, size_t* pdatalen);

#endif
/*! @} */

