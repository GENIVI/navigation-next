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

    @file     LocationRequest.cpp
    @date     05/05/2011
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
    LocationRequest class implementation

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
  #include "palclock.h"
  #include <nbqalog.h>
}

#include "locationrequest.h"

LocationRequest::LocationRequest(NB_LocationContext* locationContext)
    : m_queue(locationContext),
      m_locationContext (locationContext),
      m_timeout(0),
      m_lastFixTimestamp(0),
      m_fixType(NBFixType_Normal)
{
    nsl_memset(&m_lastFixLocation, 0, sizeof(m_lastFixLocation));
}

LocationRequest::~LocationRequest(void)
{
    PAL_TimerCancel(m_locationContext->m_PAL_Instance,
        StaticRequestTimerCallback, (void*)this);
}

void LocationRequest::SetTimeout(int timeout)
{
    PAL_TimerCancel(m_locationContext->m_PAL_Instance,
        StaticRequestTimerCallback, (void*)this);

    m_timeout = timeout;

    if (timeout != 0)
    {
        PAL_TimerSet(m_locationContext->m_PAL_Instance,
            timeout*1000, // our time settings are in seconds
            StaticRequestTimerCallback, (void*)this);
    }
}

// alesnov:
// Since SGPS request deals differently with the cache, I made RequestStart virtual (earlier, it had common non-virtual code)
// and split it in two. In fact, the proper final resolution is to move all dealings with cache to request level.
// Eventually the GPS and network requests will have nothing in common regarding the cache.
NB_Error LocationRequest::RequestStart(NB_LS_LocationCallback callback, void* applicationData, NB_LS_FixType fixType)
{
    NB_Error err = NE_OK;

    bool cacheResult = CheckCache(callback, applicationData, fixType);

    if (!cacheResult)
    {
        // Nothing useful in the cache.
        err = ScheduleRequest(callback, applicationData, fixType);
    }

    return err;
}

bool LocationRequest::CheckCache(NB_LS_LocationCallback callback, void* applicationData, NB_LS_FixType fixType)
{
    NB_LS_Location fixFromCache = {0};


    if (IsCachedLocationValid(fixType))
    {
        CopyLocation(fixFromCache, m_lastFixLocation);
    }


    if (fixFromCache.valid)
    {
        // This means, we really got something valid from cache.
        // Schedule timer callback, so reply will look "as from a normal request"
        NB_QaLogAppState(m_locationContext->internalContext, "Got a fix from cache");
        CachedLocationReply* newReply =
            (CachedLocationReply*)nsl_malloc(sizeof(CachedLocationReply));
        newReply->callback = callback;
        newReply->userData = applicationData;
        CopyLocation(newReply->location, fixFromCache);
        newReply->context = m_locationContext;
        PAL_TimerSet(m_locationContext->m_PAL_Instance,
            0, StaticCacheCallback, (void*)newReply);
        return TRUE;
    }

    return FALSE;
}

NB_Error LocationRequest::ScheduleRequest(NB_LS_LocationCallback callback, void* applicationData, NB_LS_FixType fixType)
{
    NB_Error err = NE_OK;

    // Select timeout by req type
    int timeout = GetSpecificTimeout(fixType);

    // Queue manager will add listener, with replyTime = PAL_ClockGetUnixTime() + timeout;
    err = m_queue.AddListener(callback, applicationData, timeout);

    if (err == NE_OK)
    {
        if (m_timeout == 0)
        {
            // Request is not yet started
            m_fixType = fixType;
            err = StartLocationRequest();
            if (err == NE_OK)
            {
                SetTimeout(timeout);
            }
        }
        // Request is alredy running.
        // If timeout for a fresh request is less or equal than current timeout, let's reset the timeout.
        else if ((uint32)timeout <=
            (m_queue.GetMinimalReplyTime() - PAL_ClockGetGPSTime()))
        {
            // Reset the timeout
            SetTimeout(timeout);
        }
    }

    return err;
}

NB_Error LocationRequest::RequestInProgress(bool* inProgress)
{
    *inProgress = IsInProgress();

    return NE_OK;
}


NB_Error LocationRequest::RequestCancel(NB_LS_LocationCallback callback, void* appData)
{
    // Silently remove all the listeners
    m_queue.RemoveListeners(callback, appData);

    // Is anyone left here?
    if( (m_queue.Count() == 0))
    {
        CancelLocationRequest();
        if (m_timeout != 0)
        {
            PAL_TimerCancel(m_locationContext->m_PAL_Instance,
                StaticRequestTimerCallback, (void*)this);
            m_timeout = 0;
        }
    }
    else
    {
        // There are still listeners. Need to review the timeout
        NB_LS_LocalTime minReplyTime = m_queue.GetMinimalReplyTime();
        int timeout = minReplyTime - PAL_ClockGetGPSTime();
        SetTimeout( (timeout >= 0) ? timeout : NBTIME_LAG_VALUE );
    }

    return NE_OK;
}

bool LocationRequest::IsCachedLocationValid(NB_LS_FixType fixType)
{
    return FALSE;
}

void LocationRequest::LocationCallback(NB_Error error)
{
    if (error == NE_OK)
    {
        UpdateCache(*RetrieveLocation());
    }

    if (error != NEGPS_TIMEOUT)
    {
        // Either success, or non-timeout error (serious error). Notify everyone on status

        NB_LS_Location location = *RetrieveLocation();

        m_queue.NotifyAndRemoveAll(error, (error == NE_OK) ? &location : NULL);
    }
    else
    {
        // Lower-level timeout. Just restart the request. Our internal timer remains ON.
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

    DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]Location callback: status=%d", error));
}

void LocationRequest::StaticCacheCallback (PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    if (reason == PTCBR_TimerFired) // Currently, no need to react anyhow on PTCBR_TimerCancelled
    {
        CachedLocationReply* reply = (CachedLocationReply*)userData;
        reply->callback(reply->userData, reply->context, &(reply->location), NE_OK);
        nsl_free(reply);
    }
}

void LocationRequest::StaticRequestTimerCallback (PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    LocationRequest* thisRequest = (LocationRequest*)userData;

    if (reason == PTCBR_TimerFired) // Currently, no need to react anyhow on PTCBR_TimerCancelled
    {
        thisRequest->TimerCallbackResult();
    }
}

void LocationRequest::TimerCallbackResult()
{
    // Notify and remove everyone who's replyTime is suitable
    m_queue.NotifyTimeout();
    // Is anyone left here?
    if( (m_queue.Count() == 0))
    {
        CancelLocationRequest();
        if (m_timeout != 0)
        {
            PAL_TimerCancel(m_locationContext->m_PAL_Instance,
                StaticRequestTimerCallback, (void*)this);
            m_timeout = 0;
        }
    }
    else
    {
        // There are still listeners. Need to review the timeout
        NB_LS_LocalTime minReplyTime = m_queue.GetMinimalReplyTime();
        int timeout = minReplyTime - PAL_ClockGetGPSTime();
        SetTimeout((timeout >= 0) ? timeout : NBTIME_LAG_VALUE );
    }
}

void LocationRequest::CopyLocation(NB_LS_Location& dest, const NB_LS_Location& src)
{
        dest.altitude = src.altitude;
        dest.gpsTime  = src.gpsTime;
        dest.heading  = src.heading;
        dest.horizontalUncertaintyAlongAxis          = src.horizontalUncertaintyAlongAxis;
        dest.horizontalUncertaintyAlongPerpendicular = src.horizontalUncertaintyAlongPerpendicular;
        dest.horizontalUncertaintyAngleOfAxis        = src.horizontalUncertaintyAngleOfAxis;
        dest.horizontalVelocity                      = src.horizontalVelocity;
        dest.latitude             = src.latitude;
        dest.longitude            = src.longitude;
        dest.numberOfSatellites   = src.numberOfSatellites;
        dest.status               = src.status;
        dest.utcOffset            = src.utcOffset;
        dest.valid                = src.valid;
        dest.verticalUncertainty  = src.verticalUncertainty;
        dest.verticalVelocity     = src.verticalVelocity;
}

bool LocationRequest::IsLastFixValid(int maxAge)
{
    if (m_lastFixTimestamp == 0)
    {
        return FALSE;
    }
    return PAL_ClockGetGPSTime() < m_lastFixTimestamp + maxAge;
}

void LocationRequest::UpdateCache(const NB_LS_Location& newLocation)
{

    CopyLocation(m_lastFixLocation, newLocation);
    m_lastFixTimestamp = PAL_ClockGetGPSTime();

}

bool LocationRequest::IsIdenError(NB_Error err)
{
    if ((err == NESERVERX_DAILY_REQUEST_LIMIT_ERROR) ||  // Daily request limit exhausted
        (err == NESERVERX_INVALID_CREDENTIAL_ERROR) ||  // API key invalid
        (err == NESERVERX_DAILY_USER_LIMIT_ERROR))    // Daily user limit exhausted
    {
        return TRUE;
    }

    return FALSE;
}

/*! @} */
