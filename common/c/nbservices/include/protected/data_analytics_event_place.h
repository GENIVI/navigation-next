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

    @file     data_analytics_event_place.h

    Interface to create Analytics-Event-Place TPS element.

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

#ifndef DATA_ANALYTICS_EVENT_PLACE_
#define DATA_ANALYTICS_EVENT_PLACE_

#include "nbexp.h"
#include "datautil.h"
#include "dynbuf.h"
#include "data_string.h"
#include "data_golden_cookie.h"
#include "data_place_event_cookie.h"

typedef struct data_analytics_event_place_ {
    /* Child Elements */
    boolean gps_based;          /*!< boolean used to indicate GPS based place */
    boolean premium_placement;  /*!< If present, indicates a premium placement
                                  AD */
    boolean enhanced_poi;       /*!< If present, indicates an enhanced POI */
    data_golden_cookie golden_cookie; /*!< present if the event on the place is
                                        billable*/
    data_place_event_cookie place_event_cookie; /*!< If present, contains information
                                                     to be sent back to the server
                                                     when reporting an event*/

    /* Attributes */
    data_string id;     /*!< unique identifier associated with this place*/
    uint32 search_query_event_id; /*!< event id associated with a search */
    boolean search_query_event_id_present; /*!< whether event id is present */
    uint32 index;                           /*!< zero-based index of the place in search results */
    data_string origin;                     /*!< origin of the action performed on the place */
} data_analytics_event_place;

NB_Error data_analytics_event_place_init(data_util_state* pds,
            data_analytics_event_place* paep);
void data_analytics_event_place_free(data_util_state* pds,
            data_analytics_event_place* paep);

tpselt data_analytics_event_place_to_tps(data_util_state* pds,
            data_analytics_event_place* paep);

boolean data_analytics_event_place_equal(data_util_state* pds,
            data_analytics_event_place* paep1,
            data_analytics_event_place* paep2);

NB_Error data_analytics_event_place_copy(data_util_state* pds,
        data_analytics_event_place* paep_dest,
        data_analytics_event_place* aep_src);

uint32 data_analytics_event_place_get_tps_size(data_util_state* pds,
            data_analytics_event_place* paep);

void data_analytics_event_place_to_buf(data_util_state* pds,
            data_analytics_event_place* paep,
            struct dynbuf* pdb);

NB_Error data_analytics_event_place_from_binary(data_util_state* pds,
            data_analytics_event_place* paep,
            byte** pdata, size_t* pdatalen);

#endif
/*! @} */

