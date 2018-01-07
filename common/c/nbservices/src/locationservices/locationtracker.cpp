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

    @file     LocationTracker.cpp
    @date     05/05/2011
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
    LocationTracker class declaration. Tracking implementation

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
#include "locationrequest.h"
#include "locationtracker.h"
#include "gpslocationtracker.h"
#include "networklocationtracker.h"
#include "emulocationtracker.h"
//#include "NBCoreCrossPlatform.h"
#include "statemanager.h"

LocationTracker::LocationTracker(NB_LocationContext* locationContext, NB_Context* nbContext)
    : m_locationContext(locationContext),
      m_gpsTracker(NULL),
      m_networkTracker(NULL),
      m_emuTracker(NULL),
      m_gotGpsFix(FALSE),
      m_callback(NULL),
      m_userData(NULL)
{
    nsl_memset(&m_gpsTrackingLocation, 0, sizeof(m_gpsTrackingLocation));
    nsl_memset(&m_networkTrackingLocation, 0, sizeof(m_networkTrackingLocation));
    nsl_memset(&m_emuTrackingLocation, 0, sizeof(m_emuTrackingLocation));

    if (m_locationContext)
    {
        m_emuTracker = new EmuLocationTracker(m_locationContext);
        m_gpsTracker = new GpsLocationTracker(m_locationContext);
        m_networkTracker = new NetworkLocationTracker(m_locationContext, nbContext);
    }
}

LocationTracker::~LocationTracker(void)
{
    delete m_networkTracker;
    delete m_gpsTracker;
    delete m_emuTracker;
}

NB_LS_LocationState LocationTracker::GetState(void)
{
    // Emulation mode
    if (m_locationContext->locationConfig.emulationMode)
    {
        return m_emuTracker->GetState();
    }

    // Non-emulation mode
    NB_LS_LocationState state = NBPGS_Undefined;

    if(m_gpsTracker && m_networkTracker)
    {
        state = m_gpsTracker->GetState();
        if (state != NBPGS_Tracking_GPS)
        {
            // GPS is turned off by user, or there is some other issue
            state = m_networkTracker->GetState();
        }
    }

    return state;
}

void LocationTracker::GpsInitialized(void)
{
    m_gpsTracker->GpsInitialized();
}

void LocationTracker::StaticGpsTrackerCallback(void* userData, NB_LocationContext* context,
        const NB_LS_Location* location, NB_Error error)
{
    LocationTracker* me = (LocationTracker*)userData;
    if (me != NULL)
    {
        me->GpsTrackerCallback(location, error);
    }
}

void LocationTracker::GpsTrackerCallback(const NB_LS_Location* location, NB_Error error)
{
    if (error == NE_OK && location->valid != 0)
    {
        m_gotGpsFix = TRUE;

        if (m_networkTracker != NULL)
        {
            m_networkTracker->EndTracking();
            if (m_locationContext)
            {
                m_locationContext->m_stateManager->SetTrackerState(NBPGS_Tracking_GPS);
            }
        }

        LocationRequest::CopyLocation(m_gpsTrackingLocation, *location);

        if (m_callback)
        {
            m_callback(m_userData, m_locationContext, &m_gpsTrackingLocation, error);
        }
    }
    else if (error == NEGPS_TIMEOUT)
    {
        // GPS tracking failed. Start network tracking until we get valid GPS fix. We do it on
        // NEGPS_TIMEOUT only to give GPS tracker the time to back to normal state.
        if (m_networkTracker != NULL)
        {
            m_networkTracker->BeginTracking((NB_LS_LocationCallback)StaticNetworkTrackerCallback, this);
            if (m_locationContext)
            {
                m_locationContext->m_stateManager->SetTrackerState(NBPGS_Tracking_Network);
            }
        }
    }
}

void LocationTracker::StaticNetworkTrackerCallback(void* userData, NB_LocationContext* context,
        const NB_LS_Location* location, NB_Error error)
{
    LocationTracker* me = (LocationTracker*)userData;
    if (me != NULL && !me->m_gotGpsFix)
    {
        me->NetworkTrackerCallback(location, error);
    }
}

void LocationTracker::NetworkTrackerCallback(const NB_LS_Location* location, NB_Error error)
{
    if (error == NE_OK && location->valid != 0)
    {
        LocationRequest::CopyLocation(m_networkTrackingLocation, *location);
        m_callback(m_userData, m_locationContext, &m_networkTrackingLocation, error);
    }
    else if (LocationRequest::IsIdenError(error))
    {
        m_callback(m_userData, m_locationContext, NULL, error);
    }
}

void LocationTracker::EmuTrackerCallback(const NB_LS_Location* location, NB_Error error)
{
    if (error == NE_OK && location->valid != 0)
    {
        LocationRequest::CopyLocation(m_emuTrackingLocation, *location);
        m_callback(m_userData, m_locationContext, &m_emuTrackingLocation, error);
    }
}

void LocationTracker::StaticEmuTrackerCallback(void* userData, NB_LocationContext* context,
        const NB_LS_Location* location, NB_Error error)
{
    LocationTracker* me = (LocationTracker*)userData;
    if (me != NULL)
    {
        me->EmuTrackerCallback(location, error);
    }
}

NB_Error LocationTracker::BeginTracking(NB_LS_LocationCallback callback, void* userData,
                                            NB_LocationTrackingInfo** trackingInfo)
{
    if (callback == NULL || trackingInfo == NULL)
    {
        return NE_BADDATA;
    }

    if (GetState() == NBPGS_Undefined)
    {
        return NEGPS_GENERAL;
    }

    m_callback = callback;
    m_userData = userData;

    NB_Error gpsRetStatus = NEGPS_GENERAL;
    NB_Error netRetStatus = NEGPS_GENERAL;

    if (m_locationContext->locationConfig.emulationMode)
    {
        gpsRetStatus = m_emuTracker->BeginTracking((NB_LS_LocationCallback)StaticEmuTrackerCallback, this, trackingInfo);

        if (gpsRetStatus == NE_OK)
        {
            m_locationContext->m_stateManager->SetTrackerState(NBPGS_Tracking_GPS); // we consider emulation mode as GPS emulator
        }
    }
    else
    {
        if (m_gpsTracker->GetState() != NBPGS_Undefined)
        {
            gpsRetStatus = m_gpsTracker->BeginTracking((NB_LS_LocationCallback)StaticGpsTrackerCallback, this, trackingInfo);
        }

        // This flag is introduced specially for the case when GPS tracking callback is called
        // synchronously from m_gpsTracker->BeginTracking (ABPAL could do it if it has a fix in the cache).
        // In this case we do not need to start network tracking.
        if (m_gotGpsFix == FALSE)
        {
            // Start network tracker at the same time. While GPS is cold, we will get network fixes
            netRetStatus = m_networkTracker->BeginTracking((NB_LS_LocationCallback)StaticNetworkTrackerCallback, this);
        }
        if (netRetStatus == NE_OK)
        {
            m_locationContext->m_stateManager->SetTrackerState(NBPGS_Tracking_Network);
        }
        else if (gpsRetStatus == NE_OK)
        {
            m_locationContext->m_stateManager->SetTrackerState(NBPGS_Tracking_GPS);
        }
    }

    return ((gpsRetStatus == NE_OK) || (netRetStatus == NE_OK)) ? NE_OK : NEGPS_GENERAL;
}

NB_Error LocationTracker::EndTracking(NB_LocationTrackingInfo* trackingInfo)
{
    if (GetState() == NBPGS_Undefined)
    {
        return NEGPS_GENERAL;
    }

    if (m_locationContext->locationConfig.emulationMode)
    {
        return m_emuTracker->EndTracking(trackingInfo);
    }

    m_gotGpsFix = FALSE;

    NB_Error retStatus = m_networkTracker->EndTracking();

    if (m_gpsTracker->GetState() != NBPGS_Undefined)
    {
        retStatus = m_gpsTracker->EndTracking(trackingInfo);
    }

    // If no other trackers work in parallel, TrackerManager will report resetting the state of LocationKit to Initialized

    return retStatus;
}

/*! @} */
