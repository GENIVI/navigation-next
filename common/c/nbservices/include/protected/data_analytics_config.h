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

    @file     data_analytics_config.h

    Interface to create Analytics-Config TPS element.

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
#ifndef DATA_ANALYTICS_CONFIG_
#define DATA_ANALYTICS_CONFIG_

#include "nbexp.h"
#include "datautil.h"
#include "data_gold_events_config.h"
#include "data_poi_events_config.h"
#include "data_route_tracking_config.h"
#include "data_gps_probes_config.h"
#include "data_wifi_probes_config.h"
#include "data_coupon_events_config.h"
#include "data_nav_invocation_config.h"
#include "data_feedback_config.h"
#include "data_app_errors_config.h"
#include "data_asr_event_config.h"
#include "data_poiext_event_config.h"
#include "data_user_actions_events_config.h"
#include "data_session_config.h"
#include "data_settings_events_config.h"
#include "data_share_events_config.h"
#include "data_transaction_events_config.h"
#include "data_map_events_config.h"

typedef struct data_analytics_config_ {
    /* Child Elements */
    data_gold_events_config                 gold_events_config;
    data_poi_events_config                  poi_events_config;
    data_route_tracking_config              route_tracking_config;
    data_gps_probes_config                  gps_probes_config;
    data_feedback_config                    feedback_config;
    data_app_errors_config                  app_errors_config;
    data_wifi_probes_config                 wifi_probes_config;
    data_session_config                     session_config;
    data_settings_events_config             settings_events_config;
    data_share_events_config                share_events_config;
    data_user_actions_events_config         user_actions_events_config;
    data_transaction_events_config          transaction_events_config;
    data_map_events_config                  map_events_config;
    data_coupon_events_config               coupon_events_config;
    data_nav_invocation_config              nav_invocation_config;
    data_asr_event_config                   asr_event_config;
    data_poiext_event_config                poiext_event_config;

    /* Attributes */
    uint32 max_hold_time;   /*!< the maximum amount of time (in seconds) that
                              collected data can reside on the local device
                              before the client attempts to upload to the data
                              to the server. A value of 0 meanus that the
                              client shall never automatically establish a
                              connection to the server to upload the data, but
                              shall instead wait for some other activity to
                              establish the connection before uploading the
                              data. */

    uint32 max_data_age;    /*!< Number of minutes the client should hold
                              data before deleting. A value of 0 mean never
                              delete */

    uint32 max_request_size;    /*!< Maximum size, in bytes, for an
                                  analytics-event-query. A value of 0 means
                                  that there is no imposed limit. */

} data_analytics_config;

NB_Error    data_analytics_config_init(data_util_state* pds, data_analytics_config* pac);
void        data_analytics_config_free(data_util_state* pds, data_analytics_config* pac);

NB_Error    data_analytics_config_from_tps(data_util_state* pds, data_analytics_config* pac, tpselt te);

boolean     data_analytics_config_equal(data_util_state* pds, data_analytics_config* pac1, data_analytics_config* pac2);

NB_Error    data_analytics_config_copy(data_util_state* pds, data_analytics_config* pac_dest, data_analytics_config* pac_src);

#endif
/*! @} */

