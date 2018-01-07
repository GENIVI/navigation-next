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

    @file     EmuLocationTracker.cpp
    @date     05/05/2011
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
    EmuLocationTracker class implementation

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



extern "C" {
  #include "paltimer.h"
  #include "nblocationtypes.h"
  #include "nberror.h"
}

#include "emulocationtracker.h"
#include "emulocationrequest.h"

const int EmuLocationTracker::TRACKING_FIXES_INTERVAL_CONSTANT = 2000; // in milliseconds

struct NB_LocationTrackingInfo : public Base
{
    int trackingId;
};

EmuLocationTracker::EmuLocationTracker(NB_LocationContext* locationContext)
    : m_locationContext(locationContext),
      m_inProgress(FALSE),
      m_callback(NULL),
      m_userData(NULL),
      m_state(NBPGS_Undefined)
{
    m_dummyTrackingInfo = new NB_LocationTrackingInfo;

    nsl_memset(&m_location, 0, sizeof(m_location));
    if (m_locationContext->m_emuLocationRequest)
    {
        m_state = NBPGS_Initialized;
    }
}

EmuLocationTracker::~EmuLocationTracker(void)
{
    EndTracking(m_dummyTrackingInfo);

    delete m_dummyTrackingInfo;
}

bool EmuLocationTracker::IsInProgress(void)
{
    return m_inProgress;
}

NB_LS_LocationState EmuLocationTracker::GetState(void)
{
    return m_state;
}

NB_Error EmuLocationTracker::BeginTracking(NB_LS_LocationCallback callback, void* userData,
                                               NB_LocationTrackingInfo** trackingInfo)
{
    if (m_state == NBPGS_Undefined)
    {
        return NE_NOTINIT;
    }

    if (m_inProgress)
    {
        return NE_BUSY;
    }
    m_callback = callback;
    m_userData = userData;

    nsl_memset(&m_location, 0, sizeof(m_location));

    NB_Error err = m_locationContext->m_emuLocationRequest->RequestStart
        ((NB_LS_LocationCallback)Static_EmuRequestCallback, this, NBFixType_Normal);

    if (err == NE_OK)
    {
        m_inProgress = TRUE;
        m_state = NBPGS_Tracking_GPS;

        if (trackingInfo)
        {
            *trackingInfo = m_dummyTrackingInfo;
        }
    }

    return err;
}

NB_Error EmuLocationTracker::EndTracking(NB_LocationTrackingInfo* trackingInfo)
{
    if (m_state == NBPGS_Tracking_GPS)
    {
        PAL_TimerCancel(m_locationContext->m_PAL_Instance,
                    StaticPALTimerCallback, (void*)this);

        m_locationContext->m_emuLocationRequest->RequestCancel((NB_LS_LocationCallback)Static_EmuRequestCallback, this);
        m_inProgress = FALSE;
        m_state = NBPGS_Initialized;
    }

    return NE_OK;
}

void EmuLocationTracker::Static_EmuRequestCallback (void* appData, NB_LocationContext* context,
                                                       const NB_LS_Location* location, NB_Error error)
{
    EmuLocationTracker* me = (EmuLocationTracker*)appData;
    me->EmuRequestCallback(context, location, error);
}

void EmuLocationTracker::EmuRequestCallback(NB_LocationContext* context, const NB_LS_Location* location,
                                               NB_Error error)
{
    if (error == NE_OK)
    {
        LocationRequest::CopyLocation(m_location, *location);
    }

    if ((error == NE_OK) || (error == NE_ERROR_INVALID_LOCATION)) // Track could contain both valid and invalid
    {                                                               // locations at different periods of time
        PAL_TimerSet(m_locationContext->m_PAL_Instance,
            TRACKING_FIXES_INTERVAL_CONSTANT, StaticPALTimerCallback, (void*)this);
    }

    if (m_callback)
    {
        m_callback(m_userData, m_locationContext, m_location.valid ? &m_location : NULL, error);
    }

}

void EmuLocationTracker::StaticPALTimerCallback (PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    EmuLocationTracker* me = (EmuLocationTracker*)userData;

    if (reason == PTCBR_TimerFired)
    {
        me->TimerCallbackResult();
    }
}

void EmuLocationTracker::TimerCallbackResult()
{
    nsl_memset(&m_location, 0, sizeof(m_location));
    m_locationContext->m_emuLocationRequest->RequestStart
        ((NB_LS_LocationCallback)Static_EmuRequestCallback, this, NBFixType_Normal);
}

/*! @} */
