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

    @file     data_wifi_probes_config.h

    Interface to create wifi-probes-config TPS element.

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
#ifndef DATA_WIFI_PROBES_CONFIG_
#define DATA_WIFI_PROBES_CONFIG_

#include "nbexp.h"
#include "datautil.h"

typedef struct data_wifi_probes_config_ {
    /* Child Elements */

    /* Attributes */
    uint32  max_horizontal_uncertainty; /*!< Maximum horizontal uncertainty in meters allowed for the GPS fix
                                          associated with a set of Wi-Fi probes. */

    uint32  max_speed;                  /*!< Maximum allowed device speed in m/sec as reported in the GPS fix.
                                          Fixes with speed higher than this number shall be skipped. */

    uint32  collection_interval;        /*!< Minimum time in seconds allowed between two consecutive Wi-Fi probe
                                          collections. */

    uint32  minimum_distance_delta;     /*!< Minimum distance in meters from previous Wi-Fi probe collection point.
                                          If the distance is less than this number, then skip the collection until
                                          the next time slot. */

    uint32  priority;                   /*!< The priority of this category of events relative to other categories
                                          of events.  Valid range is 0 to 10, where 10 is the highest priority.
                                          Priority 0 means that no events of in this category shall be collected,
                                          and all events cached on the client within this category shall be deleted
                                          without being uploaded to the server. */

} data_wifi_probes_config;

NB_Error    data_wifi_probes_config_init(data_util_state* pds, data_wifi_probes_config* pwpc);
void        data_wifi_probes_config_free(data_util_state* pds, data_wifi_probes_config* pwpc);

NB_Error	data_wifi_probes_config_from_tps(data_util_state* pds, data_wifi_probes_config* pwpc, tpselt te);

boolean     data_wifi_probes_config_equal(data_util_state* pds, data_wifi_probes_config* pwpc1,
                data_wifi_probes_config* pwpc2);

NB_Error    data_wifi_probes_config_copy(data_util_state* pds, data_wifi_probes_config* pwpc_dest,
                data_wifi_probes_config* pwpc_src);

#endif
/*! @} */

