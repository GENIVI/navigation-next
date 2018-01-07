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

@file gpscontext.cpp
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
#include "locationlistener.h"
#include "locationprovider.h"
#include "locationproviderfactory.h"
#include "notificationthread.h"


#define GPS_KEYNAME_DRIVER              L"System\\CurrentControlSet\\GPS Intermediate Driver\\Drivers"
#define	GPS_VALUENAME_CURRENT_DRIVER    L"CurrentDriver"
#define	GPS_VALUENAME_INTERFACE_TYPE    L"InterfaceType"
#define GPS_VALUE_FAKEGPS               L"File"

GpsContext::GpsContext(PAL_Instance* pal):
    m_pPalInstance(pal),
    m_locationProvider(0),
    m_notifyThread(0),
    m_fakeGpsEnabled(false),
    m_initializedCallback(0),
    m_deviceStateCallback(0),
    m_callbackUserData(0)
{
    CheckFakeGps();
}

GpsContext::~GpsContext()
{
    if (m_locationProvider)
    {
        m_locationProvider->Destroy();
    }

    if (m_notifyThread)
    {
        m_notifyThread->Destroy();
    }
}

PAL_Error GpsContext::Initialize(const ABPAL_GpsConfig* config, uint32 configCount, ABPAL_GpsInitializeCallback* userInitializedCallback, ABPAL_GpsDeviceStateCallback* userCallbackDeviceState, const void* userData)
{
    m_notifyThread = new NotificationThread(this, ListenersChangedCallback);
    if (!m_notifyThread)
    {
        return PAL_ErrNoMem;
    }

    PAL_Error error = m_notifyThread->Create();
    error = error ? error : m_notifyThread->StartThread();
    if (error)
    {
        return error;
    }

    m_locationProvider = LocationProviderFactory::Create(this, config, configCount);
    if (!m_locationProvider)
    {
        return PAL_ErrNoMem;
    }

    m_initializedCallback = userInitializedCallback;
    m_deviceStateCallback = userCallbackDeviceState;
    m_callbackUserData = userData;

    error = error ? error : m_locationProvider->Create();
    error = error ? error : m_locationProvider->StartThread();
    return error;
}

PAL_Error GpsContext::AddListener(const ABPAL_GpsCriteria* criteria, ABPAL_GpsLocationCallback* userCallback, const void* userData, ABPAL_GpsTrackingInfo* trackingInfo)
{
    if (!m_notifyThread)
    {
        return PAL_ErrNoInit;
    }

    LocationListener* listener = new LocationListener(this, userCallback, userData);
    listener->Initialize(criteria, FALSE);
    m_notifyThread->AddListener(listener);
    trackingInfo->Id = listener->GetTrackingId();
    
    return PAL_Ok;
}

PAL_Error GpsContext::RemoveListener(const ABPAL_GpsTrackingInfo* trackingInfo)
{
    LocationListener* listener = new LocationListener(this, trackingInfo);
    if (!listener)
    {
        return PAL_ErrNoMem;
    }

    if (!m_notifyThread)
    {
        return PAL_ErrNoInit;
    }

    return m_notifyThread->RemoveListener(listener);
}

PAL_Error GpsContext::SuspendListener(const ABPAL_GpsTrackingInfo* trackingInfo)
{
    LocationListener* listener = new LocationListener(this, trackingInfo);
    if (!listener)
    {
        return PAL_ErrNoMem;
    }

    if (!m_notifyThread)
    {
        return PAL_ErrNoInit;
    }

    return m_notifyThread->SuspendListener(listener);
}

PAL_Error GpsContext::ResumeListener(const ABPAL_GpsTrackingInfo* trackingInfo)
{
    LocationListener* listener = new LocationListener(this, trackingInfo);
    if (!listener)
    {
        return PAL_ErrNoMem;
    }

    if (!m_notifyThread)
    {
        return PAL_ErrNoInit;
    }

    return m_notifyThread->ResumeListener(listener);
}


PAL_Error GpsContext::GetLocation(const ABPAL_GpsCriteria* criteria, ABPAL_GpsLocationCallback* userCallback, const void* userData)
{
    LocationListener* listener = new LocationListener(this, userCallback, userData);
    listener->Initialize(criteria, TRUE);

    if (!m_notifyThread)
    {
        return PAL_ErrNoInit;
    }

    return m_notifyThread->AddListener(listener);
}

PAL_Error GpsContext::CancelGetLocation()
{
     if (!m_notifyThread)
    {
        return PAL_ErrNoInit;
    }

    return m_notifyThread->RemoveOneShotListeners();
}

PAL_Error GpsContext::SetCurrentLocation(const Location& location)
{
    if (!m_notifyThread)
    {
        return PAL_ErrNoInit;
    }

    m_notifyThread->SetLocation(location);
    return PAL_Ok;
}

void GpsContext::CheckFakeGps()
{
    m_fakeGpsEnabled = false;

    HKEY driverKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, GPS_KEYNAME_DRIVER, 0, 0, &driverKey) == ERROR_SUCCESS)
    {
        wchar_t currentDriver[200] = { 0 };
        DWORD size = sizeof(currentDriver) / sizeof(currentDriver[0]);

        if (RegQueryValueEx(driverKey, GPS_VALUENAME_CURRENT_DRIVER, NULL, NULL, (LPBYTE)currentDriver, &size) == ERROR_SUCCESS)
        {
            HKEY anotherKey;
            if (RegOpenKeyEx(driverKey, currentDriver, 0, 0, &anotherKey) == ERROR_SUCCESS)
            {
                wchar_t gpsInterface[10] = { 0 };
                DWORD size = sizeof(gpsInterface) / sizeof(gpsInterface[0]);

                if (RegQueryValueEx(anotherKey, GPS_VALUENAME_INTERFACE_TYPE, NULL, NULL, (LPBYTE)gpsInterface, &size) == ERROR_SUCCESS)
                {
                    m_fakeGpsEnabled = wcscmp(gpsInterface, GPS_VALUE_FAKEGPS) == 0;
                }

                RegCloseKey(anotherKey);
            }
        }
        RegCloseKey(driverKey);
    }
}

void GpsContext::NotifyLocationProviderInitialized(PAL_Error error, const char* errorInfoXml)
{
    WriteGpsDebugLogMessage("  >>> NotifyLocationProviderInitialized (callback: %X, err: %X)", m_initializedCallback, error);
    if (m_initializedCallback)
    {
        (m_initializedCallback)(m_callbackUserData, error, errorInfoXml);
    }
}

void GpsContext::NotifyLocationProviderStateChange(const char* stateChangeXml, PAL_Error error,  const char* errorInfoXml)
{
    WriteGpsDebugLogMessage("  >>> NotifyLocationProviderStateChange (callback: %X, err: %X)", m_deviceStateCallback, error);
    if (m_deviceStateCallback)
    {
        (m_deviceStateCallback)(m_callbackUserData, stateChangeXml, error, errorInfoXml);
    }
}

void GpsContext::ListenersChangedCallback(GpsContext* gpsContext, bool listenersPresent, uint32 lowestCommonInterval, uint32 lowestCommonAccuracy)
{
    if (gpsContext && gpsContext->m_locationProvider)
    {
        gpsContext->m_locationProvider->UpdateFixRequestParameters(listenersPresent, lowestCommonInterval, lowestCommonAccuracy);
    }
}

#ifdef GPS_DEBUG_LOG

#include "palfile.h"
#include "palclock.h"

void GpsContext::WriteGpsDebugLogMessage(const char* format, ...)
{
    const char* LOG_FILE_NAME = "gpslog.txt";
    
    static boolean logFileExistanceChecked = FALSE;
    static boolean logDebugMessagesToFile = FALSE;

    // check if file exists first time thru, and set log debug messages flag if it does
    if (!logFileExistanceChecked)
    {
        logDebugMessagesToFile = (PAL_FileExists(m_pPalInstance, LOG_FILE_NAME) == PAL_Ok);
        logFileExistanceChecked = TRUE;
    }
    
    // log messages only if file existed on startup
    if (logDebugMessagesToFile)
    {
        PAL_File* file = 0;
        PAL_ClockDateTime dateTime = {0};
        SYSTEM_POWER_STATUS_EX power = {0};
        uint32 written = 0;
        char dateTimeStamp[80] = {0};
        char powerStatus[20] = {0};

        char buffer[256] = {0};
        va_list args;

        (void)PAL_FileOpen(m_pPalInstance, LOG_FILE_NAME, PFM_Append, &file);
        (void)PAL_ClockGetDateTime(&dateTime);

        if (GetSystemPowerStatusEx(&power, FALSE))
        {
            if (power.BatteryFlag & 1)
            {
                strcpy(buffer, "[HIGH]");
            }
            else if (power.BatteryFlag & 2)
            {
                strcpy(buffer, "[LOW ]");
            }
            else if (power.BatteryFlag & 4)
            {
                strcpy(buffer, "[CRIT]");
            }
            sprintf(powerStatus, "%d%% %s", power.BatteryLifePercent, buffer);
        }

        sprintf(dateTimeStamp, "%02d/%02d/%02d %02d:%02d:%02d 0x%08X %s ",
            dateTime.month, dateTime.day, dateTime.year % 100, dateTime.hour, dateTime.minute, dateTime.second,
            GetCurrentThreadId(), powerStatus);
        (void)PAL_FileWrite(file, (const uint8 *)dateTimeStamp, strlen(dateTimeStamp) * sizeof(char), &written);

        va_start(args, format);
        vsprintf(buffer, format, args);
        strcat(buffer, "\r\n");

        (void)PAL_FileWrite(file, (const uint8 *)buffer, strlen(buffer) * sizeof(char), &written);
        (void)PAL_FileClose(file);
    }
}

bool GpsContext::IsFakeRoamingMode()
{
    const char* ROAMING_FILE_NAME = "roaming.txt";
    return (PAL_FileExists(m_pPalInstance, ROAMING_FILE_NAME) == PAL_Ok);
}

#else

void GpsContext::WriteGpsDebugLogMessage(const char* format, ...)
{
}

bool GpsContext::IsFakeRoamingMode()
{
    return false;
}

#endif

/*! @} */


