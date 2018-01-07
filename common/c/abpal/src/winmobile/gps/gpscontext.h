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

@file gpscontext.h      
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

#include "abpalgps.h"
#include "pal.h"
#include "palerror.h"

class GpsConfig;
class LocationProvider;
class NotificationThread;
class Location;

#define GPS_DEBUG_LOG

class GpsContext
{
public:
    GpsContext(PAL_Instance* pPal);
    ~GpsContext();

    PAL_Error               Initialize(const ABPAL_GpsConfig* config, uint32 configCount, ABPAL_GpsInitializeCallback* userInitializedCallback, ABPAL_GpsDeviceStateCallback* userCallbackDeviceState, const void* userData);

    PAL_Error               AddListener(const ABPAL_GpsCriteria* criteria, ABPAL_GpsLocationCallback* userCallback, const void* userData, ABPAL_GpsTrackingInfo* trackingInfo);
    PAL_Error               RemoveListener(const ABPAL_GpsTrackingInfo* trackingInfo);
    PAL_Error               SuspendListener(const ABPAL_GpsTrackingInfo* trackingInfo);
    PAL_Error               ResumeListener(const ABPAL_GpsTrackingInfo* trackingInfo);

    PAL_Error               GetLocation(const ABPAL_GpsCriteria* criteria, ABPAL_GpsLocationCallback* userCallback, const void* userData);
    PAL_Error               CancelGetLocation();

    PAL_Error               SetCurrentLocation(const Location& location);

    bool                    IsFakeGpsEnabled() const { return m_fakeGpsEnabled; }
    bool                    IsFakeRoamingMode();

    void                    NotifyLocationProviderInitialized(PAL_Error error, const char* errorInfoXml);
    void                    NotifyLocationProviderStateChange(const char* stateChangeXml, PAL_Error error,  const char* errorInfoXml);

    void                    WriteGpsDebugLogMessage(const char* format, ...);

private:
    void                    CheckFakeGps();

    static void             ListenersChangedCallback(GpsContext* gpsContext, bool listenersPresent, uint32 lowestCommonInterval, uint32 lowestCommonAccuracy);

    PAL_Instance*           m_pPalInstance;
    LocationProvider*       m_locationProvider;
    NotificationThread*     m_notifyThread;
    bool                    m_fakeGpsEnabled;

    ABPAL_GpsInitializeCallback*    m_initializedCallback;
    ABPAL_GpsDeviceStateCallback*   m_deviceStateCallback;
    const void*                     m_callbackUserData;
};


/*! @} */
