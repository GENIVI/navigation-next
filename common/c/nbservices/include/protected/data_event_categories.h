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

    @file     data_event_categories.h

    Interface to create Event-Categories TPS element.

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
#ifndef DATA_EVENT_CATEGORIES_
#define DATA_EVENT_CATEGORIES_

#include "nbexp.h"
#include "datautil.h"
#include "data_string.h"

typedef struct data_event_categories_ {
    /* Child Elements */
    boolean gold_category;              /*!< If present, the client suports
                                          gold events */
    boolean poi_category;               /*!< If present, the client supports
                                          POI events */
    boolean route_tracking_category;    /*!< If present, the client supports
                                          route trakcing events */
    boolean gps_probes_category;        /*!< If present, the client supports
                                          gps probes events */
    boolean feedback_category;          /*!< If present, the client supports
                                          feedback events */
    boolean app_errors_category;        /*!< If present, the client supports
                                          application errors events */
    boolean wifi_probes_category;       /*!< If present, the client supports
                                          wifi probes events */
    boolean session_category;
    boolean settings_category;
    boolean share_category;
    boolean user_actions_category;
    boolean transaction_category;
    boolean map_category;
    boolean coupon_category;            /*!< If present, the client supports
                                          coupon action events */
    boolean nav_invocation_category;    /*!< If present, the client supports
                                          nav-invocation events */


    /* Attributes */
} data_event_categories;

NB_Error data_event_categories_init(data_util_state* pds,
            data_event_categories* pec);
void data_event_categories_free(data_util_state* pds,
            data_event_categories* pec);

tpselt data_event_categories_to_tps(data_util_state* pds,
            data_event_categories* pec);

boolean data_event_categories_equal(data_util_state* pds,
            data_event_categories* pec1, data_event_categories* pec2);

NB_Error data_event_categories_copy(data_util_state* pds,
            data_event_categories* pec_dest, data_event_categories* aep_src);

uint32 data_event_categories_get_tps_size(data_util_state* pds,
            data_event_categories* pec);

#endif
/*! @} */

