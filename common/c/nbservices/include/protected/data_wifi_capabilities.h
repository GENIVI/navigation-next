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

    @file     data_wifi_capabilities.h

    Interface to create Wifi-Capabilities TPS element.
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

#ifndef DATA_WIFI_CAPABILITIES__H
#define DATA_WIFI_CAPABILITIES__H

#include "datautil.h"
#include "data_gps.h"
#include "palradio.h"
#include "data_pair.h"

/* Keys */
#define WIFI_KEY_CIPHER         "cipher"
#define WIFI_KEY_MANAGEMENT     "key-management"
#define WIFI_KEY_AUTHENTICATION "authentication"
#define WIFI_KEY_PHYSICAL_LAYER "physical-layer"

/* Ciphers */
#define WIFI_CAPABILITIES_CIPHER_CCMP    "CCMP"
#define WIFI_CAPABILITIES_CIPHER_TKIP    "TKIP"
#define WIFI_CAPABILITIES_CIPHER_WEP104  "WEP104"
#define WIFI_CAPABILITIES_CIPHER_WEP140  "WEP140"

/* Key management */
#define WIFI_CAPABILITIES_KEY_MANAGEMENT_WPA_EAP    "WPA_EAP"
#define WIFI_CAPABILITIES_KEY_MANAGEMENT_WPA_PSK    "WPA_PSK"
#define WIFI_CAPABILITIES_KEY_MANAGEMENT_IEEE8201X  "IEEE8021x"

/* Authentication */
#define WIFI_CAPABILITIES_AUTHENTICATION_OPEN              "OPEN"
#define WIFI_CAPABILITIES_AUTHENTICATION_SHARED            "SHARED"
#define WIFI_CAPABILITIES_AUTHENTICATION_LEAP              "LEAP"
#define WIFI_CAPABILITIES_AUTHENTICATION_WEP               "WEP"
#define WIFI_CAPABILITIES_AUTHENTICATION_WPA_PSK           "WPA_PSK"
#define WIFI_CAPABILITIES_AUTHENTICATION_WPA2_PSK          "WPA2_PSK"
#define WIFI_CAPABILITIES_AUTHENTICATION_WPA_ENTERPRISE    "WPA_ENTERPRISE"
#define WIFI_CAPABILITIES_AUTHENTICATION_WPA2_ENTERPRISE   "WPA2_ENTERPRISE"
#define WIFI_CAPABILITIES_AUTHENTICATION_WPS               "WPS"
#define WIFI_CAPABILITIES_AUTHENTICATION_DYNAMIC_WEP       "DYNAMICWEP"

/* Physical Layer */
#define WIFI_CAPABILITIES_PHYSICAL_LAYER_802_11A      "802.11a"
#define WIFI_CAPABILITIES_PHYSICAL_LAYER_802_11B      "802.11b"
#define WIFI_CAPABILITIES_PHYSICAL_LAYER_802_11G      "802.11g"
#define WIFI_CAPABILITIES_PHYSICAL_LAYER_802_11N      "802.11n"

typedef struct data_wifi_capabilities_
{
    ///* Child Elements */
    struct CSL_Vector*   vec_pairs;

    ///* Attributes */

} data_wifi_capabilities;

NB_Error data_wifi_capabilities_init(data_util_state* pds,
            data_wifi_capabilities* pwc);

void data_wifi_capabilities_free(data_util_state* pds,
            data_wifi_capabilities* pwc);

tpselt data_wifi_capabilities_to_tps(data_util_state* pds,
            data_wifi_capabilities* pwc);

boolean data_wifi_capabilities_equal(data_util_state* pds,
            data_wifi_capabilities* pwc1, data_wifi_capabilities* pwc2);

NB_Error data_wifi_capabilities_copy(data_util_state* pds,
            data_wifi_capabilities* pwc_dest, data_wifi_capabilities* pwc_src);

uint32 data_wifi_capabilities_get_tps_size(data_util_state* pds,
            data_wifi_capabilities* pwc);

void data_wifi_capabilities_to_buf(data_util_state* pds,
            data_wifi_capabilities* pwc,
            struct dynbuf* pdb);

NB_Error data_wifi_capabilities_from_binary(data_util_state* pds,
            data_wifi_capabilities* pwc,
            byte** pdata, size_t* pdatalen);

NB_Error data_wifi_capabilities_add_pair(data_util_state* pds,
            data_wifi_capabilities* pwc,
            const char* key,
            const char* value);

#endif // DATA_WIFI_CAPABILITIES__H

/*! @} */