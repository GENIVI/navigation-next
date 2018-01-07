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

@file locationlistener.h
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


#pragma once

#include "paltypes.h"
#include "abpalgps.h"
#include "location.h"

class GpsContext;

class LocationListener
{
public:
    LocationListener(GpsContext* gpsContext, ABPAL_GpsLocationCallback* callback, const void* callbackData);
    LocationListener(GpsContext* gpsContext, const ABPAL_GpsTrackingInfo* trackingInfo);
    ~LocationListener();

    void Initialize(const ABPAL_GpsCriteria* criteria, nb_boolean oneShot);

    /*! Notify the listener of a new fix
    @return The number of milliseconds until another fix is desired.  If INFINITE, no further fixes are desired
    */
    uint32                      Notify(const Location& location);

    bool                        IsOneShot() const { return m_oneShot == TRUE; }
    uint16                      GetTrackingId() const { return m_trackingInfo.Id; }

    /*! Function to test if two LocationListeners are the same
    @return true if the same tracking id or callback/callback data pair; false otherwise
    */
    bool                        IsSameListener(const LocationListener& other) const;
    
    bool                        IsSuspended() const { return m_suspended; }

    void                        Suspend() { m_suspended = true; }
    void                        Resume() { m_suspended = false; }

    uint32                      GetFixInterval() const { return m_interval; }
    uint32                      GetFixAccuracy() const { return m_accuracy; }
    
private:
    nb_boolean                  HasTimeoutExpired() const;
    nb_boolean                  HasMaxAgeExpired(nb_gpsTime fixTime) const;

    GpsContext*                 m_gpsContext;
    ABPAL_GpsTrackingInfo       m_trackingInfo;
    nb_boolean                  m_oneShot;

    ABPAL_GpsLocationCallback*  m_callback;         /*!< Callback to invoke when a fix is received */
    const void*                 m_callbackData;     /*!< Caller-supplied callback data */

    bool                        m_suspended;
    uint32                      m_maxAge;           /*!< Maximum age of fix, in seconds */
    nb_gpsTime                  m_timeout;          /*!< Single shot timeout time */
    uint32                      m_interval;         /*!< Fix interval, in seconds */
    uint32                      m_accuracy;         /*!< Desired accuracy, in meters */
    nb_gpsTime                  m_requestTime;      /*!< Time that the initial request was made */
    Location                    m_lastFix;          /*!< Last fix sent to the callback */

    static uint16               m_nextTrackingId;
};


/*! @} */
