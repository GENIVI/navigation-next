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

@file locationlistener.cpp
*/
/*
(C) Copyright 2014 by TeleCommunication Systems, Inc.                

The information contained herein is confidential, proprietary 
to TeleCommunication Systems, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of TeleCommunication Systems is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*!
    @addtogroup abpalgpswinmobile
    @{
*/

#include "gpscontext.h"
#include "location.h"
#include "locationlistener.h"
#include "palclock.h"
#include "windows.h"


uint16 LocationListener::m_nextTrackingId = 0;


LocationListener::LocationListener(GpsContext* gpsContext, ABPAL_GpsLocationCallback callback, const void* callbackData) :
    m_gpsContext(gpsContext),
    m_callback(callback),
    m_callbackData(callbackData),
    m_oneShot(FALSE),
    m_maxAge(0),
    m_interval(0),
    m_accuracy(0),
    m_timeout(0),
    m_requestTime(0),
    m_suspended(false)
{
    m_trackingInfo.Id = ++m_nextTrackingId;
}

LocationListener::LocationListener(GpsContext* gpsContext, const ABPAL_GpsTrackingInfo* trackingInfo) :
    m_gpsContext(gpsContext),
    m_callback(0),
    m_callbackData(0),
    m_oneShot(FALSE),
    m_maxAge(0),
    m_interval(0),
    m_accuracy(0),
    m_timeout(0),
    m_requestTime(0),
    m_suspended(false)
{
    m_trackingInfo = *trackingInfo;
}

LocationListener::~LocationListener()
{
}

bool LocationListener::IsSameListener(const LocationListener& other) const
{
    return (m_trackingInfo.Id == other.m_trackingInfo.Id) || (m_callback == other.m_callback && m_callbackData == other.m_callbackData);
}


void LocationListener::Initialize(const ABPAL_GpsCriteria* criteria, nb_boolean oneShot)
{
    m_oneShot = oneShot;
    m_maxAge = criteria->desiredMaxAge;
    m_interval = criteria->desiredInterval;
    m_accuracy = criteria->desiredAccuracy;

    if (oneShot && criteria->desiredTimeout)
    {
        m_timeout = PAL_ClockGetGPSTime() + (criteria->desiredTimeout * 1000);
    }
}

uint32 LocationListener::Notify(const Location& location)
{
    uint32 millisecondsToNextFix = m_interval;

    if (!m_suspended)
    {
        bool invoked = FALSE;

        // Send valid fixes only if they haven't been sent before and they aren't too old
        if (location.IsValid() && !m_lastFix.IsSameFix(location) && !HasMaxAgeExpired(location.FixTime()))
        {
            (m_callback)(m_callbackData, PAL_Ok, &location, location.GetErrorInfoXml().c_str());
            invoked = TRUE;

            m_lastFix = location;

            if (m_oneShot)
            {
                millisecondsToNextFix = INFINITE;
            }
        }

        // Check if one shot request has timed out
        if (!invoked && m_oneShot && HasTimeoutExpired())
        {
            millisecondsToNextFix = INFINITE;
        }
    }

    return millisecondsToNextFix;
}

nb_boolean LocationListener::HasTimeoutExpired() const
{
    return m_timeout != 0 && PAL_ClockGetGPSTime() >= m_timeout;
}

nb_boolean LocationListener::HasMaxAgeExpired(nb_gpsTime fixTime) const
{
    // Max age 0 means fixes are always expired
    if (m_maxAge == 0)
    {
        return true;
    }

    // Using fake gps means fixes can't expire
    if (m_gpsContext->IsFakeGpsEnabled())
    {
        return false;
    }

    // Return true if time is later than fix time + max age
    return PAL_ClockGetGPSTime() > fixTime + m_maxAge;
}

/*! @} */
