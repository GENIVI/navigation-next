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

    @file     data_wifi_probe.h

    Interface to create Wifi-Probe TPS element.
*/
/*
    (C) Copyright 2010 by Networks In Motion, Inc.                
                                                                  
    The information contained herein is confidential, proprietary 
    to Networks In Motion, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#ifndef DATA_WIFI_PROBE__H
#define DATA_WIFI_PROBE__H

#include "datautil.h"
#include "data_string.h"
#include "data_wifi_capabilities.h"
#include "palradio.h"

typedef struct data_wifi_probe_
{
    ///* Child Elements */
    data_wifi_capabilities wifi_capabilities;

    ///* Attributes */
    data_string   mac_address;                /*!< A MAC-48 identifier, which consists of 6 bytes of binary data. */

    data_string   ssid;                       /*!< The Service Set ID of the network (the network name). */

    uint32        frequency;                  /*!< Frequency in MHz of the channel over which the client is
                                                   communicating with the access point. */

    uint32        signal_strength;            /*!< Received signal strength in dBm. Note: The value passed should
                                                   be taken as negative since TPS does not support signed ints. */

    uint32        time_delta;                 /*!< Difference in milliseconds between the time this network was
                                                   found and the associated GPS fix timestamp. This is needed
                                                   to calculate the error radius. */

} data_wifi_probe;

NB_Error data_wifi_probe_init(data_util_state* pds,
            data_wifi_probe* pwp);

void data_wifi_probe_free(data_util_state* pds,
            data_wifi_probe* pwp);

tpselt data_wifi_probe_to_tps(data_util_state* pds,
            data_wifi_probe* pwp);

boolean data_wifi_probe_equal(data_util_state* pds,
            data_wifi_probe* pwp1, data_wifi_probe* pwp2);

NB_Error data_wifi_probe_copy(data_util_state* pds,
            data_wifi_probe* pwp_dest, data_wifi_probe* pwp_src);

uint32 data_wifi_probe_get_tps_size(data_util_state* pds,
            data_wifi_probe* pwp);

void data_wifi_probe_to_buf(data_util_state* pds,
            data_wifi_probe* pwp,
            struct dynbuf* pdb);

NB_Error data_wifi_probe_from_binary(data_util_state* pds,
            data_wifi_probe* pwp,
            byte** pdata, size_t* pdatalen);

NB_Error data_wifi_probe_from_radio_wifi_info(data_util_state* pds,
            data_wifi_probe* pwp,
            PAL_RadioCompleteWifiInfo wifiInfo,
            uint32 timeDelta);

#endif // DATA_WIFI_PROBE__H

/*! @} */