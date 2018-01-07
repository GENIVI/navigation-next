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

    @file     GpsLocationRequest.cpp
    @date     02/27/2010
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
    GpsLocationRequest class implementation
    This class makes a geocode (address to coordinates) request to navigation servers.

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



#include "lsabpalgps.h"
#include "gpslocationrequest.h"
#include "locationcontext.h"
#include "wifiprobes.h"

extern "C" {
  #include "palclock.h"
  #include <nbqalog.h>
}

// This is the timer for low-level (PAL) GPS request.
// It never changes, and is selected experimentally, just as "a quite long timer"
#define NBABPAL_GPS_TIMEOUT            600000

// This is the timer for warm-up fix only. It functions on LocationKit level, and it is in seconds
#define NBWARMUP_GPS_TIMEOUT     180 // 3 minutes

// This is the timer for cold
#define NB_GPS_COLD_TIMEOUT      60000    // 1 minute

GpsLocationRequest::GpsLocationRequest(NB_LocationContext* locationContext, AbpalGps* gps)
    : LocationRequest (locationContext),
      m_gps(gps),
      m_state(NBPGS_Undefined),
      m_isInProgress (FALSE),
      m_gpsCold (TRUE)
{
    nsl_memset(&m_gpsLocation, 0, sizeof(m_gpsLocation));
}

GpsLocationRequest::~GpsLocationRequest()
{
    CancelLocationRequest();
    PAL_TimerCancel(m_locationContext->m_PAL_Instance,
            StaticColdGPSTimerCallback, (void*)this);
}

void GpsLocationRequest::GpsInitialized(void)
{
    m_state = NBPGS_Initialized;
}

int GpsLocationRequest::GetSpecificTimeout(NB_LS_FixType fixType)
{
    if (m_locationContext->locationConfig.warmUpFix)
    {
        return NBWARMUP_GPS_TIMEOUT;
    }
    else
    {
        switch (fixType)
        {
        case NBFixType_Fast:
            return m_locationContext->fastSGPSFixTimeout;
        case NBFixType_Normal:
            return m_locationContext->normalSGPSFixTimeout;
        case NBFixType_Accurate:
        default:
            return m_locationContext->accurateSGPSFixTimeout;
        }
    }
}

bool GpsLocationRequest::IsCachedLocationValid(NB_LS_FixType fixType)
{
    int maxAge = 0;
    switch (fixType)
    {
        case NBFixType_Fast:
            maxAge = m_locationContext->maxFastSGPSAge;
            break;
        case NBFixType_Normal:
            maxAge = m_locationContext->maxNormalSGPSAge;
            break;
        case NBFixType_Accurate:
            // In this case, cached results are never valid
            maxAge = 0;
            break;
    }

    return IsLastFixValid(maxAge);
}

NB_Error GpsLocationRequest::RequestStart(NB_LS_LocationCallback callback, void* applicationData, NB_LS_FixType fixType)
{
    if (!m_locationContext->m_abpalGps->IsGPSEnabled())
    {
        return NEGPS_GENERAL;
    }

    if ((fixType != NBFixType_Accurate) && m_gpsCold)
    {
        DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]GPS is cold. Asking a background fix"));
        // Return "GPS Cold" status, but schedule a background GPS request
        ScheduleRequest(callback, applicationData, NBFixType_Accurate);
        return NEGPS_GENERAL;
    }

    return ScheduleRequest(callback, applicationData, fixType);
}

NB_Error GpsLocationRequest::StartLocationRequest()
{
    if (m_state == NBPGS_Undefined)
    {
        return NEGPS_GENERAL;
    }
    nsl_memset(&m_gpsLocation, 0, sizeof(m_gpsLocation));

    ABPAL_GpsCriteria palGpsCriteria = { 0 };

    if (m_locationContext->locationConfig.warmUpFix)
    {
        m_gps->InitGpsCriteria(palGpsCriteria, NBABPAL_GPS_TIMEOUT);
    }
    else
    {
        // Apply request-specific settings for maxAge
        int maxAge = ABPAL_GPS_FIX_MINIMAL_MAXAGE;

        switch (m_fixType)
        {
        case NBFixType_Fast:
            maxAge = m_locationContext->maxFastSGPSAge;
            break;
        case NBFixType_Normal:
            maxAge = m_locationContext->maxNormalSGPSAge;
            break;
        case NBFixType_Accurate:
            // there is no special maxAge for Accurate fix
            maxAge = m_locationContext->maxNormalSGPSAge;
            break;
        }
        m_gps->InitGpsCriteria(palGpsCriteria, NBABPAL_GPS_TIMEOUT, maxAge);
    }

    NB_Error err = NE_OK;
    if ( ABPAL_GpsGetLocation(m_gps->GpsContext(),
          &palGpsCriteria, StaticGpsLocationCallback, this) == PAL_Ok )
    {
        m_isInProgress = TRUE;
        m_state = NBPGS_Oneshot;
    }
    else
    {
        err = NEGPS_GENERAL;
    }

    return err;
}

bool GpsLocationRequest::IsInProgress()
{
    return m_isInProgress;
}

void GpsLocationRequest::CancelLocationRequest()
{
    ABPAL_GpsCancelGetLocation(m_gps->GpsContext());
    m_isInProgress = FALSE;
    m_state = NBPGS_Initialized;
}

NB_LS_Location* GpsLocationRequest::RetrieveLocation()
{
    return &m_gpsLocation;
}

NB_Error GpsLocationRequest::GetState( NB_LS_LocationState * state )
{
    if (state == NULL)
    {
        return NE_BADDATA;
    }

    *state = m_state;
    return NE_OK;
}

void GpsLocationRequest::StaticGpsLocationCallback(const void* userData, PAL_Error error, const ABPAL_GpsLocation* location, const char* errorInfoXml)
{
    GpsLocationRequest* request = (GpsLocationRequest *)userData;
    if (request != NULL )
    {
        request->GpsLocationCallback(error, location, errorInfoXml);
    }
}

void GpsLocationRequest::GpsLocationCallback(PAL_Error error, const ABPAL_GpsLocation* location, const char* errorInfoXml)
{
    NB_Error locationError = NEGPS_GENERAL;
    m_isInProgress = FALSE;
    m_state = NBPGS_Initialized;

    if (error == PAL_Ok && location != NULL && location->valid != 0) // At least something should be valid in the location
    {
        m_gps->ConvertToNBLocation(m_gpsLocation, *location);

        // Add fresh GPS fix to Wi-Fi probes, and QA-log it
        NB_GpsLocation nbGps;
        nsl_memset(&nbGps, 0, sizeof(NB_GpsLocation));
        m_gps->ConvertToNbGpsLocation(*location, nbGps);
        WifiProbesAddWifiProbe(m_locationContext, &nbGps);
        NB_QaLogGPSFix(m_locationContext->internalContext, &nbGps);
        locationError = NE_OK;

        // Declare GPS warm, and restart the timer
        m_gpsCold = FALSE;
        PAL_TimerCancel(m_locationContext->m_PAL_Instance,
                StaticColdGPSTimerCallback, (void*)this);
        PAL_TimerSet(m_locationContext->m_PAL_Instance,
            NB_GPS_COLD_TIMEOUT,
            StaticColdGPSTimerCallback, (void*)this);
    }
    else if (location != NULL && location->valid == 0)
    {
        locationError = NESERVERX_INVALID_COORD_ERROR;
    }
    else if (error == PAL_ErrGpsTimeout)
    {
        locationError = NEGPS_TIMEOUT;
    }

    LocationCallback(locationError);
}

void GpsLocationRequest::LocationCallback(NB_Error error)
{
    if (error == NE_OK)
    {
        UpdateCache(*RetrieveLocation());
        NB_LS_Location location = *RetrieveLocation();
        m_queue.NotifyAndRemoveAll(NE_OK, &location);
    }
    else
    {
        // Lower-level error or timeout. Let's try again. Our internal timer remains ON.
        NB_Error err = StartLocationRequest();
        if (err != NE_OK)
        {
            // Request failed to re-start. Sorry for everyone.
            m_queue.NotifyAndRemoveAll(err, NULL);
        }
    }

    if( (m_queue.Count() == 0) && (m_timeout != 0) )
    {
        // Request is over
        PAL_TimerCancel(m_locationContext->m_PAL_Instance,
            StaticRequestTimerCallback, (void*)this);
        m_timeout = 0;
    }

    DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]GPS Loc. callback: status=%d", error));
}

void GpsLocationRequest::StaticColdGPSTimerCallback (PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    if (reason == PTCBR_TimerFired) // Currently, no need to react anyhow on PTCBR_TimerCancelled
    {
        GpsLocationRequest* thisRequest = (GpsLocationRequest*)userData;
        thisRequest->ColdGPSTimerCallback();
    }
}

void GpsLocationRequest::ColdGPSTimerCallback ()
{
    m_gpsCold = TRUE;
    DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]GPS is cold now", 1));
}

void GpsLocationRequest::CancelOutstandingRequests()
{
    m_queue.NotifyAndRemoveAll(NEGPS_GENERAL, NULL);

    CancelLocationRequest();
    if (m_timeout != 0)
    {
        PAL_TimerCancel(m_locationContext->m_PAL_Instance,
            StaticRequestTimerCallback, (void*)this);
        m_timeout = 0;
    }
}

/*! @} */
