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

    @file     GpsLocationTracker.cpp
    @date     06/01/2010
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
    GpsLocationTracker class declaration. Gps tracking implementation

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
#include "gpslocationtracker.h"
#include "gpslocationrequest.h"
#include "wifiprobes.h"

// WM ABPAL returns valid GPS fixes only. So we need inactivity interval to understand GPS is not OK.
const int GpsLocationTracker::GPS_TRACKING_INACTIVITY_INTERVAL = 30000;    // Timeout in milliseconds

GpsLocationTracker::GpsLocationTracker(NB_LocationContext* locationContext)
    : m_locationContext(locationContext),
      m_callback(NULL),
      m_userData(NULL),
      m_inProgress(FALSE),
      m_fixReceived(FALSE),
      m_state(NBPGS_Undefined)
{
    nsl_memset(&m_trackingLocation, 0, sizeof(m_trackingLocation));

    if (locationContext->m_abpalGps->IsGPSInitialized())
    {
        m_state = NBPGS_Initialized;
    }
}

GpsLocationTracker::~GpsLocationTracker(void)
{
    PAL_TimerCancel(m_locationContext->m_PAL_Instance,
                StaticInactivityCallback, this);
    PAL_TimerCancel(m_locationContext->m_PAL_Instance,
                StaticNewRequestCallback, this);
}

bool GpsLocationTracker::IsInProgress(void)
{
    return m_inProgress;
}

NB_LS_LocationState GpsLocationTracker::GetState(void)
{
    return m_state;
}

void GpsLocationTracker::GpsInitialized(void)
{
    m_state = NBPGS_Initialized;
}

NB_Error GpsLocationTracker::BeginTracking(NB_LS_LocationCallback userCallback, void* userData,
                                                      NB_LocationTrackingInfo** trackingInfo)
{
    if ((m_state == NBPGS_Undefined) ||
        (!m_locationContext->m_abpalGps->IsGPSEnabled()))
    {
        return NEGPS_GENERAL;
    }
    if (trackingInfo == NULL)
    {
        return NE_BADDATA;
    }

    m_callback = userCallback;
    m_userData = userData;

    NB_Error err = NEGPS_GENERAL;
    nsl_memset(&m_trackingLocation, 0, sizeof(m_trackingLocation));

    err = m_locationContext->m_gpsLocationRequest->RequestStart
        (StaticGpsTrackingCallback, (void*)this, NBFixType_Accurate);

    if (err == NE_OK)
    {
        m_inProgress = TRUE;
        m_state = NBPGS_Tracking_GPS;
        m_fixReceived = FALSE;
        *trackingInfo = (NB_LocationTrackingInfo*)this;

        PAL_TimerSet(m_locationContext->m_PAL_Instance,
            GPS_TRACKING_INACTIVITY_INTERVAL, StaticInactivityCallback, this);

    }

    return err;
}

NB_Error GpsLocationTracker::EndTracking(NB_LocationTrackingInfo* trackingInfo)
{
    if (m_state == NBPGS_Undefined)
    {
        return NEGPS_GENERAL;
    }
    if (trackingInfo == NULL)
    {
        return NE_BADDATA;
    }

    NB_Error err = NE_OK;

    err = m_locationContext->m_gpsLocationRequest->RequestCancel
        (StaticGpsTrackingCallback, (void*)this);

    PAL_TimerCancel(m_locationContext->m_PAL_Instance,
            StaticInactivityCallback, this);
    PAL_TimerCancel(m_locationContext->m_PAL_Instance,
            StaticNewRequestCallback, this);

    m_state = NBPGS_Initialized;
    m_inProgress = FALSE;
    return err;
}

void GpsLocationTracker::StaticGpsTrackingCallback(void* userData, NB_LocationContext* context,
                                             const NB_LS_Location* location, NB_Error error)
{
    GpsLocationTracker* me = (GpsLocationTracker*)userData;
    if (me != NULL)
    {
        me->GpsTrackingCallback(error, location);
    }
}

void GpsLocationTracker::GpsTrackingCallback(NB_Error error, const NB_LS_Location* location)
{
    if (m_callback != NULL)
    {
        if (error == NE_OK &&
            location != NULL &&
            location->valid != 0)
        {
            m_trackingLocation = *location;
            m_fixReceived = TRUE;

            // Go ahead for a new request, load up the timer
            PAL_TimerSet(m_locationContext->m_PAL_Instance,
                AbpalGps::TRACKING_FIXES_INTERVAL_CONSTANT, StaticNewRequestCallback, this);

            // Notify user on a fresh GPS fix
            m_callback(m_userData, m_locationContext, &m_trackingLocation, error);
        }
        else if (error == NE_GPS_TIMEOUT)
        {
            // If it's just a timeout - start one more request right now
            error = m_locationContext->m_gpsLocationRequest->RequestStart
                (StaticGpsTrackingCallback, (void*)this, NBFixType_Accurate);
        }

        if (error != NE_OK)
        {
            // Try once again after some time
            PAL_TimerSet(m_locationContext->m_PAL_Instance,
                AbpalGps::TRACKING_FIXES_INTERVAL_CONSTANT, StaticNewRequestCallback, this);
        }
    }
}

void GpsLocationTracker::StaticInactivityCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    GpsLocationTracker* me = (GpsLocationTracker*)userData;
    if (me != NULL && reason == PTCBR_TimerFired)
    {
        me->InactivityCallback();
    }
}

void GpsLocationTracker::InactivityCallback()
{
    if (!m_fixReceived && m_callback)
    {
        // Notify user on GPS inactivity
        m_callback(m_userData, m_locationContext, NULL, NEGPS_TIMEOUT);
    }
    m_fixReceived = FALSE;
    PAL_TimerSet(m_locationContext->m_PAL_Instance,
        GPS_TRACKING_INACTIVITY_INTERVAL, StaticInactivityCallback, this);
}

void GpsLocationTracker::StaticNewRequestCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    GpsLocationTracker* me = (GpsLocationTracker*)userData;
    if (me != NULL && reason == PTCBR_TimerFired)
    {
        me->NewRequestCallback();
    }
}

void GpsLocationTracker::NewRequestCallback()
{
    NB_Error err = m_locationContext->m_gpsLocationRequest->RequestStart
                (StaticGpsTrackingCallback, (void*)this, NBFixType_Accurate);

    if (err != NE_OK)
    {
        // Try once again after some time
        PAL_TimerSet(m_locationContext->m_PAL_Instance,
            AbpalGps::TRACKING_FIXES_INTERVAL_CONSTANT, StaticNewRequestCallback, this);
    }
}

/*! @} */
