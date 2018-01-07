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

    @file     nbwifiprobes.c
    @defgroup nbwifiprobes
*/
/*
    (C) Copyright 2005 - 2009 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "nbcontextaccess.h"
#include "nbanalyticsprotected.h"
#include "palclock.h"
#include "paltimer.h"
#include "nbspatial.h"
#include "nbwifiprobes.h"
#include "nbqalog.h"


/*! @{ */

#define NB_ANALYTICS_PROCESS_TIMEOUT_INITIAL  100
#define NB_ANALYTICS_PROCESS_TIMEOUT          50000

struct NB_WifiProbes
{
    /* Pointer to PAL_RadioContext. It is owned by client code. */
    PAL_RadioContext*           radioContext;

    /* wifi probes gps */
    NB_GpsLocation*             lastWifiLocation;
    NB_GpsLocation*             currentWifiLocation;

    /* Count of started automatic analytics processes. */
    uint16                      automaticProcessCount;
};

/* local functions */
static void AllWifiNetworksInfoCallback(PAL_Error status,
        PAL_RadioCompleteWifiInfo** wifiInfo, unsigned long count,
        void* userData);

NB_DEF NB_Error
NB_WifiProbesCreate(NB_Context* context, PAL_RadioContext* radioContext, NB_WifiProbes** wifiProbes)
{
    NB_WifiProbes* pThis = 0;

    if (!context || !radioContext || !wifiProbes)
    {
        return NE_INVAL;
    }

    if ((pThis = nsl_malloc(sizeof(*pThis))) == NULL)
    {
        return NE_NOMEM;
    }
    nsl_memset(pThis, 0, sizeof(*pThis));

    pThis->radioContext = radioContext;

    *wifiProbes = pThis;

    return NE_OK;
}

NB_DEF NB_Error
NB_WifiProbesDestroy(NB_WifiProbes* pThis)
{
    NB_Error error = NE_OK;

    if (pThis)
    {
        if(pThis->radioContext != NULL)
        {
            PAL_RadioCancelAllWifiNetworksInfo(pThis->radioContext, AllWifiNetworksInfoCallback);
            pThis->radioContext = NULL;
        }

        if(pThis->lastWifiLocation != NULL)
        {
            nsl_free(pThis->lastWifiLocation);
        }

        if(pThis->currentWifiLocation != NULL)
        {
            nsl_free(pThis->currentWifiLocation);
        }

        nsl_free(pThis);
    }

    return error;
}

nb_boolean
NB_WifiProbesIsGpsGoodForWifiProbe(NB_WifiProbes* pThis,
        const NB_AnalyticsWifiConfig* config, NB_GpsLocation* location)
{
    uint32 max_speed = 0;
    uint32 collection_interval = 0;
    uint32 max_horizontal_uncertainty = 0;
    uint32 minimum_distance_delta = 0;
    double distance = 0.0;

    if (!pThis || !config || !location)
    {
        return FALSE;
    }

    if(!((location->valid & NGV_Latitude) &&
         (location->valid & NGV_Longitude)))
    {
        return FALSE;
    }

    max_speed = config->max_speed;
    if((location->valid & NGV_HorizontalVelocity) &&
       (location->horizontalVelocity > max_speed))
    {
        return FALSE;
    }

    max_horizontal_uncertainty =
        config->max_horizontal_uncertainty;
    if (location->valid & NGV_AxisUncertainty &&
        (location->valid & NGV_PerpendicularUncertainty &&
         (location->horizontalUncertaintyAlongAxis > max_horizontal_uncertainty ||
          location->horizontalUncertaintyAlongPerpendicular > max_horizontal_uncertainty)))
    {
        return FALSE;
    }


    if (!pThis->lastWifiLocation)
    {
        return TRUE;   /* first wifi probe, not too soon */
    }

    collection_interval = config->collection_interval;
    if((location->gpsTime - pThis->lastWifiLocation->gpsTime) < collection_interval)
    {
        return FALSE;
    }

    minimum_distance_delta =
        config->minimum_distance_delta;
    distance = NB_SpatialGetLineOfSightDistance(pThis->lastWifiLocation->latitude,
                                                       pThis->lastWifiLocation->longitude,
                                                       location->latitude,
                                                       location->longitude,
                                                       NULL);
    if (distance < minimum_distance_delta)
    {
        return FALSE;
    }

    return TRUE;
}

void
AllWifiNetworksInfoCallback(PAL_Error status, PAL_RadioCompleteWifiInfo** wifiInfo, unsigned long count, void* userData)
{
    NB_Error err = NE_OK;
    NB_Context* context = (NB_Context*)userData;

    NB_WifiProbes* pThis = NB_ContextGetWifiProbes(context);

    nb_gpsTime wifiTime = PAL_ClockGetGPSTime();
    uint32 timeDelta = 0;

    if(context == NULL || pThis == NULL || wifiInfo == NULL ||
       pThis->currentWifiLocation == NULL)
    {
        return;        
    }
    
    timeDelta = (wifiTime - pThis->currentWifiLocation->gpsTime) * 1000;

    if(count > 0)
    {
        unsigned int i = 0;
        for (i = 0; i < count; ++i)
        {
            NB_QaLogNbiWifiProbes(context,
                                  pThis->currentWifiLocation->latitude,
                                  pThis->currentWifiLocation->longitude,
                                  pThis->currentWifiLocation->gpsTime,
                                  (unsigned char)(count & 0xFF),
                                  (*wifiInfo)[i].macAddress,
                                  (*wifiInfo)[i].ssid,
                                  (*wifiInfo)[i].signalStrength,
                                  timeDelta);
        }

        // @todo: Remove it because it's an empty func only returning NOT_SUPPORTED.
        //err = NB_AnalyticsAddWifiProbes(context, pThis->currentWifiLocation, timeDelta, wifiInfo, (uint32)count);

        if (!err)
        {
            /*  event is added to eventsTailQs */
            if(pThis->lastWifiLocation != NULL)
            {
                nsl_free(pThis->lastWifiLocation);
            }

            pThis->lastWifiLocation = pThis->currentWifiLocation;

            pThis->currentWifiLocation = NULL;

            return;
        }
    }

    if(pThis->currentWifiLocation != NULL)
    {
        nsl_free(pThis->currentWifiLocation);
    }

    pThis->currentWifiLocation = NULL;
}

NB_DEF NB_Error
NB_WifiProbesAddWifiProbe(NB_Context* context, NB_GpsLocation* location)
{
    NB_WifiProbes* pThis = NB_ContextGetWifiProbes(context);

    PAL_RadioContext* radioContext = NULL;

    const NB_AnalyticsWifiConfig* config = NB_AnalyticsGetWifiConfig(context);

    if (!pThis || !config ||!location) /* analytics has not been created */
    {
        return NE_INVAL;
    }

    radioContext = pThis->radioContext;
    if (!radioContext)
    {
        return NE_INVAL;
    }

    if (!NB_AnalyticsIsWifiProbesAllowed(context))
    {
        return NE_OK;   /* wifi probes is not allowed */
    }

    if (!NB_WifiProbesIsGpsGoodForWifiProbe(pThis, config, location))
    {
        return NE_OK;   /* bad GPS, just drop */
    }

    if(pThis->currentWifiLocation == NULL)
    {
        if ((pThis->currentWifiLocation = nsl_malloc(sizeof(*pThis->currentWifiLocation))) == NULL)
        {
            return NE_NOMEM;
        }
        nsl_memcpy(pThis->currentWifiLocation, location, sizeof(*pThis->currentWifiLocation));

        PAL_RadioGetAllWifiNetworksInfo(radioContext, AllWifiNetworksInfoCallback, context);
    }
    else
    {
        nsl_memcpy(pThis->currentWifiLocation, location, sizeof(*pThis->currentWifiLocation));
    }

    return NE_OK;
}

/* @} */
