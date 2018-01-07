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

    @file     NBWifiProbes.cpp
    @date     05/05/2011
    @defgroup LOCATIONSERVICES_GROUP

*/
/*
    NBWifiProbes
    Helper functions for Wi-Fi probes.

    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */



#include "wifiprobes.h"

extern "C" {
  #include "nbanalytics.h"
  #include <nbwifiprobes.h>
  #include "nbcontextaccess.h"
}

#define NB_ANALYTICS_PROCESS_TIMEOUT_INITIAL  100
#define NB_ANALYTICS_PROCESS_TIMEOUT          50000

NB_Error WifiProbesCreate(NB_LocationContext* locationContext)
{
    NB_Error nbErr = NE_OK;

    if (locationContext->locationConfig.collectWiFiProbes)
    {
        nbErr = NB_WifiProbesCreate(locationContext->internalContext,
                                    locationContext->m_PAL_RadioContext,
                                    &locationContext->m_wifiProbes);
        if (nbErr == NE_OK)
        {
            NB_ContextSetWifiProbesNoOwnershipTransfer(locationContext->internalContext, locationContext->m_wifiProbes);
        }
    }

    if (nbErr == NE_NOSUPPORT)
    {
        return NE_NOSUPPORT;
    }

    return nbErr == NE_OK ? NE_OK : NEGPS_GENERAL;
}

static void AnalyticsProcessCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    NB_LocationContext* locationContext = (NB_LocationContext*)userData;

    if (reason == PTCBR_TimerFired)
    {
        if (locationContext->locationConfig.collectWiFiProbes)
        {
            NB_AnalyticsProcess(locationContext->internalContext);

            PAL_TimerSet(pal,
                         NB_ANALYTICS_PROCESS_TIMEOUT,
                         AnalyticsProcessCallback,
                         (void*) locationContext);
        }
    }
}

NB_Error WifiProbesStartAnalyticsTimer(NB_LocationContext* locationContext)
{
    PAL_Error palErr = PAL_Ok;

    if (locationContext->locationConfig.collectWiFiProbes)
    {
        palErr = PAL_TimerSet(locationContext->m_PAL_Instance,
                                   NB_ANALYTICS_PROCESS_TIMEOUT_INITIAL,
                                   AnalyticsProcessCallback,
                                   locationContext);
    }

    return palErr == PAL_Ok ? NE_OK : NEGPS_GENERAL;
}

NB_Error WifiProbesStopAnalyticsTimer(NB_LocationContext* locationContext)
{
    PAL_Error palErr = PAL_Ok;

    palErr = PAL_TimerCancel(locationContext->m_PAL_Instance,
                             AnalyticsProcessCallback,
                             locationContext);

    return palErr == PAL_Ok ? NE_OK : NEGPS_GENERAL;
}

NB_Error WifiProbesAddWifiProbe(NB_LocationContext* locationContext, NB_GpsLocation* location)
{
    NB_Error nbErr = NE_OK;

    if (locationContext && locationContext->locationConfig.collectWiFiProbes)
    {
        /* Server handles horizontalUncertaintyAlongAxis and horizontalUncertaintyAlongPerpendicular as
           axises of ellipse (a and b). It calculates accuracy radius as sqrt(a*a + b*b).
           Since we get only one uncertainity value from GPS and put it to horizontalUncertaintyAlongAxis
           and horizontalUncertaintyAlongPerpendicular, we will set one of them to 0 in order to calculate
           accuracy correctly on the server side.*/
        location->horizontalUncertaintyAlongAxis = 0;

        nbErr = NB_WifiProbesAddWifiProbe(locationContext->internalContext, location);
    }

    return nbErr == NE_OK ? NE_OK : NEGPS_GENERAL;
}

/*! @} */
