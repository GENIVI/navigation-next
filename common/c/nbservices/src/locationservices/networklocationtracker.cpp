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

    @file     NetworkLocationTracker.cpp
    @date     05/05/2011
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
    NetworkLocationTracker class declaration. Network tracking implementation

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
}

#include "networklocationtracker.h"
#include "networklocationrequest.h"

NetworkLocationTracker::NetworkLocationTracker(NB_LocationContext* locationContext, NB_Context* nbContext)
    : m_locationContext(locationContext),
      m_networkRequest(NULL),
      m_inProgress(FALSE),
      m_callback(NULL),
      m_userData(NULL)
{
    nsl_memset(&m_networkLocation, 0, sizeof(m_networkLocation));

    if (m_locationContext)
    {
        m_networkRequest = new NetworkLocationRequest(m_locationContext, nbContext);
    }
}

NetworkLocationTracker::~NetworkLocationTracker(void)
{
    EndTracking();

    delete m_networkRequest;
    m_networkRequest = NULL;
}

NB_LS_LocationState NetworkLocationTracker::GetState(void)
{
    return m_inProgress ? NBPGS_Tracking_Network : NBPGS_Initialized;
}

NB_Error NetworkLocationTracker::BeginTracking(NB_LS_LocationCallback callback, void* userData)
{
    NB_Error err = NEGPS_GENERAL;

    if (m_inProgress)
    {
        return NE_BUSY;
    }
    m_callback = callback;
    m_userData = userData;

    nsl_memset(&m_networkLocation, 0, sizeof(m_networkLocation));

    // Cancel "in progress" network location request
    if (m_networkRequest)
    {
        m_networkRequest->CancelLocationRequest();
        err = m_networkRequest->RequestStart((NB_LS_LocationCallback)Static_NetworkRequestCallback, this, NBFixType_Normal);
    }

    m_inProgress = (err == NE_OK);
    return err;
}

NB_Error NetworkLocationTracker::EndTracking()
{
    if (m_inProgress)
    {
        m_inProgress = FALSE;

        if (m_locationContext)
        {
            PAL_TimerCancel(m_locationContext->m_PAL_Instance, StaticPALTimerNetworkCallback, this);
        }

        m_networkRequest->RequestCancel((NB_LS_LocationCallback)Static_NetworkRequestCallback, this);
    }

    return NE_OK;
}

NB_LS_Location* NetworkLocationTracker::RetrieveTrackingLocation()
{
    NB_LS_Location* loc = NULL;

    if (m_networkLocation.valid != 0)
    {
        loc = &m_networkLocation;
    }
    return loc;
}

void NetworkLocationTracker::Static_NetworkRequestCallback(void* appData, NB_LocationContext* context, const NB_LS_Location* location, NB_Error error)
{
    NetworkLocationTracker* me = static_cast<NetworkLocationTracker*>(appData);
    me->NetworkRequestCallback(context, location, error);
}

void NetworkLocationTracker::NetworkRequestCallback(NB_LocationContext* context, const NB_LS_Location* location, NB_Error error)
{
    if (LocationRequest::IsIdenError(error))
    {
        // These errors are critical, we can't do tracking no more
        NetworkTrackingRequestComplete(error);
    }
    else
    {
        // TODO: when wifi support will be added, move cell ID and wifi tracking timers logic
        // into the NetworkLocationRequest
        int timeout = m_locationContext->cellIDRequestTimer;
        if (error == NE_OK)
        {
            LocationRequest::CopyLocation(m_networkLocation, *location);
            NetworkTrackingRequestComplete(NE_OK);
        }

        PAL_TimerSet(m_locationContext->m_PAL_Instance, timeout*1000, StaticPALTimerNetworkCallback, this);
    }
}

void NetworkLocationTracker::NetworkTrackingRequestComplete(NB_Error error)
{
    if (m_callback)
    {
        m_callback(m_userData, m_locationContext, (error == NE_OK) ? RetrieveTrackingLocation() : NULL, error);
    }
}

void NetworkLocationTracker::StaticPALTimerNetworkCallback (PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    NetworkLocationTracker* me = static_cast<NetworkLocationTracker*>(userData);

    if (reason == PTCBR_TimerFired)
    {
        me->NetworkTimerCallbackResult();
    }
}

void NetworkLocationTracker::NetworkTimerCallbackResult()
{
    nsl_memset(&m_networkLocation, 0, sizeof(m_networkLocation));

    NB_Error err = m_networkRequest->RequestStart((NB_LS_LocationCallback)Static_NetworkRequestCallback, this, NBFixType_Normal);
    m_inProgress = (m_inProgress || err == NE_OK);
}

/*! @} */
