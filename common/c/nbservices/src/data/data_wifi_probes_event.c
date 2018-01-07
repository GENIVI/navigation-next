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

    @file     data_wifi_probes_event.c
    
    Implemention for Wifi-Probes-Event TPS element.
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

#include "data_wifi_probes_event.h"
#include "data_wifi_probe.h"

NB_Error
data_wifi_probes_event_init(data_util_state* pds, data_wifi_probes_event* pwpe)
{
    NB_Error err = NE_OK;

    DATA_INIT(pds, err, &pwpe->gps, data_gps);

    DATA_VEC_ALLOC(err, pwpe->vec_wifi_probes, data_wifi_probe);

    return err;
}

void
data_wifi_probes_event_free(data_util_state* pds, data_wifi_probes_event* pwpe)
{
    DATA_VEC_FREE(pds, pwpe->vec_wifi_probes, data_wifi_probe);

    DATA_FREE(pds, &pwpe->gps, data_gps);
}

tpselt
data_wifi_probes_event_to_tps(data_util_state* pds, data_wifi_probes_event* pwpe)
{
    tpselt tpsElement = NULL;

    tpsElement = te_new("wifi-probes-event");

    if (tpsElement == NULL)
    {
        goto errexit;
    }

    DATA_TO_TPS(pds, errexit, tpsElement, &pwpe->gps, data_gps);

    DATA_VEC_TO_TPS(pds, errexit, tpsElement, pwpe->vec_wifi_probes, data_wifi_probe);

    return tpsElement;
 
errexit:

    te_dealloc(tpsElement);
    return NULL;
}

boolean
data_wifi_probes_event_equal(data_util_state* pds, data_wifi_probes_event* pwpe1, data_wifi_probes_event* pwpe2)
{
    int ret = TRUE;

    DATA_EQUAL(pds, ret, &pwpe1->gps, &pwpe2->gps, data_gps);

    DATA_VEC_EQUAL(pds, ret, pwpe1->vec_wifi_probes, pwpe2->vec_wifi_probes, data_wifi_probe);

    return (boolean) ret;
}

NB_Error
data_wifi_probes_event_copy(data_util_state* pds, data_wifi_probes_event* pwpe_dest,
                            data_wifi_probes_event* pwpe_src)
{
    NB_Error err = NE_OK;

    DATA_REINIT(pds, err, pwpe_dest, data_wifi_probes_event);

    DATA_COPY(pds, err, &pwpe_dest->gps, &pwpe_src->gps, data_gps);

    DATA_VEC_COPY(pds, err, pwpe_dest->vec_wifi_probes, pwpe_src->vec_wifi_probes, data_wifi_probe);

    return err;
}

uint32
data_wifi_probes_event_get_tps_size(data_util_state* pds, data_wifi_probes_event* pwpe)
{
    uint32 size = 0;
    int i = 0;
    int length = 0;

    size += data_gps_get_tps_size(pds, &pwpe->gps);

    length = CSL_VectorGetLength(pwpe->vec_wifi_probes);

    for(i = 0; i < length; i++)
    {
        size += data_wifi_probe_get_tps_size(pds, CSL_VectorGetPointer(pwpe->vec_wifi_probes, i));
    }

    return size;
}

void
data_wifi_probes_event_to_buf(data_util_state* pds, data_wifi_probes_event* pwpe, struct dynbuf* pdb)
{
    uint32 len = CSL_VectorGetLength(pwpe->vec_wifi_probes);

    dbufcat(pdb, pwpe->gps.packed, GPS_PACKED_LEN);        

    if (len  > 0)
    {
        uint32 i=0;

        dbufcat(pdb, (const byte*)&len, sizeof(len));

        for (i = 0; i < len; i++)
        {
            data_wifi_probe* wifi_probe =
                CSL_VectorGetPointer(pwpe->vec_wifi_probes, i);

            data_wifi_probe_to_buf(pds, wifi_probe, pdb);
        }
    }
    else
    {
        len = 0;
        dbufcat(pdb, (const byte*)&len, sizeof(len));
    }
}

NB_Error
data_wifi_probes_event_from_binary(data_util_state* pds, data_wifi_probes_event* pwpe,
                                   byte** pdata, size_t* pdatalen)
{
    NB_Error err = NE_OK;
    uint32 len = 0;

    if (*pdatalen >= GPS_PACKED_LEN)
    {
        nsl_memcpy(pwpe->gps.packed, *pdata, GPS_PACKED_LEN);

        *pdata += GPS_PACKED_LEN;
        *pdatalen -= GPS_PACKED_LEN;
    }
    else
    {
        err = NE_BADDATA;
    }

    err = err ? err : data_uint32_from_binary(pds, &len, pdata, pdatalen);

    if (err == NE_OK && len > 0)
    {
        uint32 i;

        for (i = 0; i < len ; i++)
        {
            data_wifi_probe wifi_probe;

            if (*pdatalen >= sizeof(wifi_probe))
            {
                err = err ? err : data_wifi_probe_init(pds, &wifi_probe);
                err = err ? err : data_wifi_probe_from_binary(pds, &wifi_probe, pdata, pdatalen);

                if(err)
                {
                    break;
                }

                if (!CSL_VectorAppend(pwpe->vec_wifi_probes, &wifi_probe))
                {
                    err = NE_NOMEM;
                    break;
                }
            }
            else
            {
                err = NE_BADDATA;
                break;
            }
        }
    }

    return err;
}

NB_Error
data_wifi_probes_event_add_wifis(data_util_state* pds,
                                 data_wifi_probes_event* pwpe,
                                 NB_GpsLocation* location,
                                 PAL_RadioCompleteWifiInfo** wifiInfo,
                                 uint32 count,
                                 uint32 timeDela)
{
    NB_Error err = NE_OK;
    uint32 i;

    data_gps_from_gpsfix(pds, &pwpe->gps, location);

    for(i = 0; i < count; i++)
    {
        data_wifi_probe wifi_probe;
        err = err ? err : data_wifi_probe_init(pds, &wifi_probe);
        err = err ? err : data_wifi_probe_from_radio_wifi_info(pds, &wifi_probe, (*wifiInfo)[i], timeDela);

        if(err)
        {
            break;
        }

        if (!CSL_VectorAppend(pwpe->vec_wifi_probes, &wifi_probe))
        {
            err = NE_NOMEM;
            break;
        }
    }

    return err;
}

/*! @} */