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

    @file     data_wifi_probe.c
    
    Implemention for Wifi-Probe TPS element.
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

#include "data_wifi_probe.h"

NB_Error
data_wifi_probe_init(data_util_state* pds, data_wifi_probe* pwp)
{
    NB_Error err = NE_OK;

    DATA_INIT(pds, err, &pwp->wifi_capabilities, data_wifi_capabilities);

    DATA_INIT(pds, err, &pwp->mac_address, data_string);
    DATA_INIT(pds, err, &pwp->ssid, data_string);
    pwp->frequency = 0;
    pwp->signal_strength = 0;
    pwp->time_delta = 0;

    return err;
}

void
data_wifi_probe_free(data_util_state* pds, data_wifi_probe* pwp)
{
    DATA_FREE(pds, &pwp->wifi_capabilities, data_wifi_capabilities);

    DATA_FREE(pds, &pwp->mac_address, data_string);
    DATA_FREE(pds, &pwp->ssid, data_string);
}

tpselt
data_wifi_probe_to_tps(data_util_state* pds, data_wifi_probe* pwp)
{
    tpselt tpsElement = NULL;

    tpsElement = te_new("wifi-probe");

    if (tpsElement == NULL)
    {
        goto errexit;
    }


    DATA_TO_TPS(pds, errexit, tpsElement, &pwp->wifi_capabilities, data_wifi_capabilities);


    if (!te_setattrc(tpsElement, "mac-address", data_string_get(pds, &pwp->mac_address)))
    {
        goto errexit;
    }

    if (!te_setattrc(tpsElement, "ssid", data_string_get(pds, &pwp->ssid)))
    {
        goto errexit;
    }

    if (!te_setattru(tpsElement, "frequency", pwp->frequency))
    {
        goto errexit;
    }

    if (!te_setattru(tpsElement, "signal-strength", pwp->signal_strength))
    {
        goto errexit;
    }

    if (!te_setattru(tpsElement, "time-delta", pwp->time_delta))
    {
        goto errexit;
    }

    return tpsElement;

errexit:

    te_dealloc(tpsElement);
    return NULL;
}

boolean
data_wifi_probe_equal(data_util_state* pds, data_wifi_probe* pwp1, data_wifi_probe* pwp2)
{
    int ret = TRUE;

    ret = data_wifi_capabilities_equal(pds, &pwp1->wifi_capabilities, &pwp2->wifi_capabilities) &&
          data_string_equal(pds, &pwp1->mac_address, &pwp2->mac_address) &&
          data_string_equal(pds, &pwp1->ssid, &pwp2->ssid) &&
          (pwp1->frequency == pwp2->frequency) &&
          (pwp1->signal_strength == pwp2->signal_strength) &&
          (pwp1->time_delta == pwp2->time_delta);

    return (boolean) ret;
}

NB_Error
data_wifi_probe_copy(data_util_state* pds, data_wifi_probe* pwp_dest, data_wifi_probe* pwp_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, pwp_dest, data_wifi_probe);

    DATA_COPY(pds, err, &pwp_dest->wifi_capabilities, &pwp_src->wifi_capabilities, data_wifi_capabilities);

    DATA_COPY(pds, err, &pwp_dest->mac_address, &pwp_src->mac_address, data_string);
    DATA_COPY(pds, err, &pwp_dest->ssid, &pwp_src->ssid, data_string);
    pwp_dest->frequency = pwp_src->frequency;
    pwp_dest->signal_strength = pwp_src->signal_strength;
    pwp_dest->time_delta = pwp_src->time_delta;

    return err;
}

uint32
data_wifi_probe_get_tps_size(data_util_state* pds, data_wifi_probe* pwp)
{
    uint32 size = 0;

    size += data_wifi_capabilities_get_tps_size(pds, &pwp->wifi_capabilities);

    size += data_string_get_tps_size(pds, &pwp->mac_address);
    size += data_string_get_tps_size(pds, &pwp->ssid);
    size += sizeof(pwp->frequency);
    size += sizeof(pwp->signal_strength);
    size += sizeof(pwp->time_delta);

    return size;
}

void
data_wifi_probe_to_buf(data_util_state* pds, data_wifi_probe* pwp, struct dynbuf* pdb)
{
    data_wifi_capabilities_to_buf(pds, &pwp->wifi_capabilities, pdb);

    data_string_to_buf(pds, &pwp->mac_address, pdb);
    data_string_to_buf(pds, &pwp->ssid, pdb);
    dbufcat(pdb, (const byte*)&pwp->frequency, sizeof(pwp->frequency));
    dbufcat(pdb, (const byte*)&pwp->signal_strength, sizeof(pwp->signal_strength));
    dbufcat(pdb, (const byte*)&pwp->time_delta, sizeof(pwp->time_delta));
}

NB_Error
data_wifi_probe_from_binary(data_util_state* pds, data_wifi_probe* pwp, byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;

    err = err ? err : data_wifi_capabilities_from_binary(pds,
            &pwp->wifi_capabilities, pdata, pdatalen);

    err = err ? err : data_string_from_binary(pds, &pwp->mac_address,
            pdata, pdatalen);

    err = err ? err : data_string_from_binary(pds, &pwp->ssid,
            pdata, pdatalen);

    err = err ? err : data_uint32_from_binary(pds, &pwp->frequency,
            pdata, pdatalen);

    err = err ? err : data_uint32_from_binary(pds, &pwp->signal_strength,
            pdata, pdatalen);

    err = err ? err : data_uint32_from_binary(pds, &pwp->time_delta,
            pdata, pdatalen);

    return err;
}

NB_Error
data_wifi_probe_from_radio_wifi_info(data_util_state* pds, data_wifi_probe* pwp,
                                     PAL_RadioCompleteWifiInfo wifiInfo, uint32 timeDelta)
{
    NB_Error err = NE_OK;

    char macAddr[18] = {0};

    nsl_sprintf(macAddr, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
                wifiInfo.macAddress[0],
                wifiInfo.macAddress[1],
                wifiInfo.macAddress[2],
                wifiInfo.macAddress[3],
                wifiInfo.macAddress[4],
                wifiInfo.macAddress[5]);

    data_string_set(pds, &pwp->mac_address, macAddr);

    err = data_string_set(pds, &pwp->ssid, (const char *)wifiInfo.ssid);

    pwp->frequency = wifiInfo.frequency / 1000;
    pwp->signal_strength = wifiInfo.signalStrength > 0 ? wifiInfo.signalStrength : -wifiInfo.signalStrength;
    pwp->time_delta = timeDelta > 0 ? timeDelta : 0xffffffff    /* maximum unsigned int value */
                                                      + timeDelta;    

    if(wifiInfo.isPrivacy)
    {
        data_wifi_capabilities_add_pair(pds, &pwp->wifi_capabilities, WIFI_KEY_AUTHENTICATION,
            WIFI_CAPABILITIES_AUTHENTICATION_WEP);
    }
    else
    {
        data_wifi_capabilities_add_pair(pds, &pwp->wifi_capabilities, WIFI_KEY_AUTHENTICATION,
            WIFI_CAPABILITIES_AUTHENTICATION_OPEN);
    }

    return err;
}

/*! @} */