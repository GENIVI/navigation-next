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

@file lpsgpsid.cpp
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

#include <regext.h>
#include <snapi.h>

#include "lpsgpsid.h"
#include "gpsconfignames.h"
#include "location.h"
#include "palstdlib.h"
#include "palclock.h"

#include "LBSDriver.h"

const uint16 DEFAULT_FIX_RATE_FIX_COUNT = 14401;
const uint16 DEFAULT_FIX_RATE_INTERVAL_SECONDS = 2;
const uint16 DEFAULT_QOS_ACCURACY_METERS = 50;
const uint16 DEFAULT_QOS_PERFORMANCE_SECONDS = 16;

const uint32 QUERY_RESPONSE_TIMEOUT_SECONDS = DEFAULT_QOS_PERFORMANCE_SECONDS * 4;


LpsGpsId::LpsGpsId(GpsContext* gpsContext, std::map<std::string, std::wstring> parameters) :
    LocationProvider(gpsContext),
    m_gpsDevice(0),
    m_gpsapiDll(0),
    m_gpsidNewLocationAvailable(0),
    m_gpsidDeviceStateChanged(0),
    m_gpsidFixRequestParametersChanged(0),
    m_pGpsOpenDevice(0),
    m_pGpsGetPosition(0),
    m_pGpsGetDeviceState(0),
    m_pGpsCloseDevice(0),
    m_pGpsGetDeviceParam(0),
    m_pGpsSetDeviceParam(0),
    m_parameters(parameters),
    m_fixInterval(0),
    m_fixAccuracy(0),
    m_inactivityTimeout(INFINITE)
{
    GetGpsContext()->WriteGpsDebugLogMessage("LpsGpsId constructor");

    InitializeCriticalSection(&m_fixParametersLock);
}

LpsGpsId::~LpsGpsId()
{
    if (m_gpsidNewLocationAvailable)
    {
        CloseHandle(m_gpsidNewLocationAvailable);
    }

    if (m_gpsidDeviceStateChanged)
    {
        CloseHandle(m_gpsidDeviceStateChanged);
    }

    if (m_gpsidFixRequestParametersChanged)
    {
        CloseHandle(m_gpsidFixRequestParametersChanged);
    }

    DeleteCriticalSection(&m_fixParametersLock);

    GetGpsContext()->WriteGpsDebugLogMessage("LpsGpsId destructor");
}

void LpsGpsId::UpdateFixRequestParameters(bool listenersPresent, uint32 lowestCommonInterval, uint32 lowestCommonAccuracy)
{
    LockFixParameters();
    
    if (listenersPresent)
    {
        m_fixInterval = (uint16)(lowestCommonInterval ? lowestCommonInterval / 1000 : DEFAULT_FIX_RATE_INTERVAL_SECONDS);
        m_fixAccuracy = (uint16)(lowestCommonAccuracy ? lowestCommonAccuracy : DEFAULT_QOS_ACCURACY_METERS);
        m_inactivityTimeout = QUERY_RESPONSE_TIMEOUT_SECONDS * 1000;
    }
    else
    {
        m_fixInterval = 0;
        m_fixAccuracy = 0;
        m_inactivityTimeout = INFINITE;
    }
    
    UnlockFixParameters();

    GetGpsContext()->WriteGpsDebugLogMessage("  UpdateFixRequestParameters listenersPresent: %d LC interval: %d LC accuracy: %d", listenersPresent, lowestCommonInterval, lowestCommonAccuracy);

    SetEvent(m_gpsidFixRequestParametersChanged);
}

void LpsGpsId::RequestNextFix(bool& rFixQueryPending, uint32& rFixCount)
{
    LockFixParameters();

    if (m_fixInterval)
    {
        DWORD gpsError = GPS_ERR_SUCCESS;

        // for first fix use GPS_FIX_MODE_MSA, otherwise GPS_FIX_MODE_MSB
        if (rFixCount == 0)
        {
            gpsError = gpsError ? gpsError : SetMode(GetRoamingGpsMode(GPS_FIX_MODE_MSA));
            gpsError = gpsError ? gpsError : SetFixRate(DEFAULT_FIX_RATE_FIX_COUNT, m_fixInterval);
            gpsError = gpsError ? gpsError : SetQOS(m_fixAccuracy, DEFAULT_QOS_PERFORMANCE_SECONDS);

            GetGpsContext()->WriteGpsDebugLogMessage("  SetMode(MSA), SetQOS(%d, %d), SetFixRate(%d, %d) result: %X",
                m_fixAccuracy, DEFAULT_QOS_PERFORMANCE_SECONDS, DEFAULT_FIX_RATE_FIX_COUNT, m_fixInterval, gpsError);
        }
        else if (rFixCount == 1)
        {
            gpsError = gpsError ? gpsError : SetMode(GPS_FIX_MODE_MSB);
            
            GetGpsContext()->WriteGpsDebugLogMessage("  SetMode(MSB) result: %X", gpsError);
        }
        
        gpsError = gpsError ? gpsError : m_pGpsSetDeviceParam(m_gpsDevice, GPS_QUERY_FIX, NULL, 0);

        GetGpsContext()->WriteGpsDebugLogMessage("  RequestNextFix result: %X", gpsError);

        if (gpsError == GPS_ERR_SUCCESS)
        {
            rFixQueryPending = TRUE;
        }
    }
    else
    {
        rFixCount = 0;
    }

    UnlockFixParameters();
}

DWORD LpsGpsId::RestartDevice(bool& rGpsServiceInitialized, bool& rFixQueryPending, uint32& rFixCount)
{
    CloseDevice();

    rGpsServiceInitialized = FALSE;
    rFixQueryPending = FALSE;
    rFixCount = 0;

    return OpenDevice();
}

DWORD LpsGpsId::AcquisitionThreadProc()
{
    GetGpsContext()->WriteGpsDebugLogMessage("Entering LPS GPS acquisition thread");

    bool gpsServiceInitialized = FALSE;
    bool fixQueryPending = FALSE;
    uint32 fixCount = 0;
    
    GPS_POSITION gpsPosition = { 0 };
    gpsPosition.dwSize = sizeof(gpsPosition);
    gpsPosition.dwVersion = GPS_VERSION_2;

    GPS_DEVICE gpsDevice = { 0 };
    gpsDevice.dwSize = sizeof(gpsDevice);
    gpsDevice.dwVersion = GPS_VERSION_2;

    Location lastLocationFix;
    
    HANDLE gpsEvents[] = {
        m_gpsidNewLocationAvailable,
        m_gpsidDeviceStateChanged,
        m_gpsidFixRequestParametersChanged,
        GetStopEvent()
    };
    DWORD eventCount = sizeof(gpsEvents) / sizeof(gpsEvents[0]);

    bool stopThread = FALSE;
    
    do
    {
        DWORD result = WaitForMultipleObjects(eventCount, gpsEvents, FALSE, m_inactivityTimeout);
        switch (result)
        {
            case WAIT_OBJECT_0:         // new position
                fixQueryPending = FALSE;

                result = m_pGpsGetPosition(m_gpsDevice, &gpsPosition, 0, 0);
                GetGpsContext()->WriteGpsDebugLogMessage("  New position event: GpsGetPosition result: %X, (%f, %f) valid fields %X", result, gpsPosition.dblLatitude, gpsPosition.dblLongitude, gpsPosition.dwValidFields);
    
                switch (result)
                {
                case GPS_ERR_SUCCESS:
                    fixCount++;
                    TranslateGpsPosition(gpsPosition, lastLocationFix);
                    if (lastLocationFix.IsValid())
                    {
                        UpdateLocation(lastLocationFix);
                    }
                    else
                    {
                        GetGpsContext()->NotifyLocationProviderStateChange("", PAL_ErrGpsInformationUnavailable, "");
                    }
                    RequestNextFix(fixQueryPending, fixCount);
                    break;

                case GPS_ERR_TRACKING_SESSION_EXPIRED:
                {
                    DWORD gpsError = GPS_ERR_SUCCESS;

                    // simply re-programming the session after it expires doesn't appear to work... so trying close & re-open GPS
                    // gpsError = gpsError ? gpsError : SetFixRate(DEFAULT_FIX_RATE_FIX_COUNT, m_fixInterval);
                    // gpsError = gpsError ? gpsError : SetQOS(m_fixAccuracy, DEFAULT_QOS_PERFORMANCE_SECONDS);

                    gpsError = RestartDevice(gpsServiceInitialized, fixQueryPending, fixCount);

                    GetGpsContext()->WriteGpsDebugLogMessage("  Close & re-Open device result: %X", gpsError);
                    break;
                }    
                case GPS_ERR_GPSONE:
                    GetGpsContext()->WriteGpsDebugLogMessage("  GpsGetPosition GPS_ERR_GPSONE dwFixError: %X", gpsPosition.dwFixError);
                    
                    switch (gpsPosition.dwFixError)
                    {
                    case GPS_ERR_INSUFFICIENT_SAT_SIGNAL:
                    case GPS_ERR_POSITION_INFO_NOT_AVAILABLE:
                    case GPS_ERR_GETPOS_TIMEOUT:
                    case GPS_ERR_NETWORK_UNREACHABLE:
                    case GPS_ERR_DEVICE_BUSY:
                    case ERROR_CANCELLED:
                        GetGpsContext()->NotifyLocationProviderStateChange("", PAL_ErrGpsInformationUnavailable, "");
                        RequestNextFix(fixQueryPending, fixCount);
                        break;

                    case ERROR_ACCESS_DENIED:
                        GetGpsContext()->NotifyLocationProviderStateChange("", PAL_ErrGpsLocationDisabled, "");
                        RequestNextFix(fixQueryPending, fixCount);
                        break;

                    default:
                        GetGpsContext()->NotifyLocationProviderStateChange("", PAL_ErrGpsGeneralFailure, "");
                        break;
                    }
                    break;

                default:
                    GetGpsContext()->NotifyLocationProviderStateChange("", TranslateGpsError(result), "");
                    break;
                }
                break;
                
            case WAIT_OBJECT_0 + 1:     // device state change
                if (!gpsServiceInitialized)
                {
                    result = m_pGpsGetDeviceState(m_gpsDevice, &gpsDevice);			
                    GetGpsContext()->WriteGpsDebugLogMessage("  Device state event: GpsGetDeviceState result: %X device state: %X service state: %X", result, gpsDevice.dwDeviceState, gpsDevice.dwServiceState);

                    switch (result)
                    {
                    case GPS_ERR_SUCCESS:
                        if (gpsServiceInitialized)
                        {
                            /// @todo Report device state xml via device state callback
                            GetGpsContext()->NotifyLocationProviderStateChange("", PAL_Ok, "");
                        }
                        else
                        {
                            if (gpsDevice.dwServiceState == GPS_LBSDRIVER_STATE_ON)
                            {
                                gpsServiceInitialized = TRUE;
                                fixCount = 0;
                                fixQueryPending = FALSE;
                                
                                GetGpsContext()->NotifyLocationProviderInitialized(PAL_Ok, "");
                                SetEvent(m_gpsidFixRequestParametersChanged);
                            }
                        }
                        break;

                    default:
                        GetGpsContext()->NotifyLocationProviderStateChange("", TranslateGpsError(result), "");
                        break;
                    }
                }
                break;
                
            case WAIT_OBJECT_0 + 2:     // fix parameters changed
                GetGpsContext()->WriteGpsDebugLogMessage("  FixRequestParameterChanged event: (gpsInitialized %d, fixQueryPending: %d)", gpsServiceInitialized, fixQueryPending);
                if (gpsServiceInitialized && !fixQueryPending && m_inactivityTimeout != INFINITE)
                {
                    RequestNextFix(fixQueryPending, fixCount);
                }
                break;
                

            case WAIT_TIMEOUT:          // maximum polling inactivity elapsed
                GetGpsContext()->WriteGpsDebugLogMessage("  Timeout event: (%d msec timeout, gpsInitialized %d, fixQueryPending: %d)", m_inactivityTimeout, gpsServiceInitialized, fixQueryPending);

                GetGpsContext()->NotifyLocationProviderStateChange("", PAL_ErrGpsTimeout, "");
                
                if (gpsServiceInitialized && !fixQueryPending)
                {
                    RequestNextFix(fixQueryPending, fixCount);
                }
                else
                {
                    if (RestartDevice(gpsServiceInitialized, fixQueryPending, fixCount) != GPS_ERR_SUCCESS)
                    {
                        GetGpsContext()->NotifyLocationProviderStateChange("", PAL_ErrGpsGeneralFailure, "");
                    }
                }
                break;
            
            case WAIT_OBJECT_0 + 3:     // stop event
                GetGpsContext()->WriteGpsDebugLogMessage("  Stop thread event");
                stopThread = TRUE;
                break;

            default:
                ASSERT(0);
                break;
        }
    } while (!stopThread);

    GetGpsContext()->WriteGpsDebugLogMessage("Exiting acquisition thread");

    return 0;
}

PAL_Error LpsGpsId::CreateInstance()
{
    GetGpsContext()->WriteGpsDebugLogMessage("LpsGpsId::CreateInstance()");

    PAL_Error error = PAL_ErrNoInit;
    DWORD gpsError = GPS_ERR_FATAL_ERROR;

    m_gpsidNewLocationAvailable = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!m_gpsidNewLocationAvailable)
    {
        goto exit;
    }

    m_gpsidDeviceStateChanged = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!m_gpsidDeviceStateChanged)
    {
        goto exit;
    }

    m_gpsidFixRequestParametersChanged = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!m_gpsidFixRequestParametersChanged)
    {
        goto exit;
    }

    gpsError = LoadDriver();
    if (gpsError != GPS_ERR_SUCCESS)
    {
        goto exit;
    }
    
    gpsError = OpenDevice();
    
    char* errorXml = "<?xml version=\"1.0\"?><result>TODO</result>";
    if (gpsError != GPS_ERR_SUCCESS)
    {
        /// @todo Add error information to xml
        errorXml = "<?xml version=\"1.0\"?><result>TODO</result>";
        goto exit;
    }

    /// @todo notify callback through context

    error = PAL_Ok;

exit:
    if (gpsError != GPS_ERR_SUCCESS)
    {
        if (m_gpsidNewLocationAvailable)
        {
            CloseHandle(m_gpsidNewLocationAvailable);
            m_gpsidNewLocationAvailable = NULL;
        }
        if (m_gpsidDeviceStateChanged)
        {
            CloseHandle(m_gpsidDeviceStateChanged);
            m_gpsidDeviceStateChanged = NULL;
        }
        if (m_gpsidFixRequestParametersChanged)
        {
            CloseHandle(m_gpsidFixRequestParametersChanged);
            m_gpsidFixRequestParametersChanged = NULL;
        }

        /// @todo gps to pal error
        error = PAL_ErrGpsGeneralFailure;
    }

    return error;
}

PAL_Error LpsGpsId::DestroyInstance()
{
    GetGpsContext()->WriteGpsDebugLogMessage("LpsGpsId::DestroyInstance()");
    
    StopThread();

    CloseDevice();
    
    UnloadDriver();

    return PAL_Ok;
}

DWORD LpsGpsId::LoadDriver()
{
    if (!m_gpsapiDll)
    {
        m_gpsapiDll = LoadLibrary(L"lbsdriver.dll");
    }

    if (!m_gpsapiDll)
    {
        return GetLastError();
    }

    m_pGpsOpenDevice = (LPSGPSID_OpenDevice)GetProcAddress(m_gpsapiDll, L"GPSOpenDevice");
    m_pGpsGetPosition = (LPSGPSID_GetPosition)GetProcAddress(m_gpsapiDll, L"GPSGetPosition");
    m_pGpsGetDeviceState = (LPSGPSID_GetDeviceState)GetProcAddress(m_gpsapiDll, L"GPSGetDeviceState");
    m_pGpsCloseDevice = (LPSGPSID_CloseDevice)GetProcAddress(m_gpsapiDll, L"GPSCloseDevice");
    m_pGpsGetDeviceParam = (LPSGPSID_GetDeviceParam)GetProcAddress(m_gpsapiDll, L"GPSGetDeviceParam");
    m_pGpsSetDeviceParam = (LPSGPSID_SetDeviceParam)GetProcAddress(m_gpsapiDll, L"GPSSetDeviceParam");

    if (!m_pGpsOpenDevice ||!m_pGpsGetPosition || !m_pGpsGetDeviceState || !m_pGpsCloseDevice || !m_pGpsGetDeviceParam || !m_pGpsSetDeviceParam)
    {
        return GetLastError();
    }

    return GPS_ERR_SUCCESS;
}

void LpsGpsId::UnloadDriver()
{
    if (m_gpsapiDll)
    {
        // @todo HACK: don't free lpsdriver.dll to preserve authentication across multiple client create/destroys
        // FreeLibrary(m_gpsapiDll);
        m_gpsapiDll = NULL;
        m_pGpsOpenDevice = NULL;
        m_pGpsGetPosition = NULL;
        m_pGpsGetDeviceState = NULL;
        m_pGpsCloseDevice = NULL;   
        m_pGpsGetDeviceParam = NULL;
        m_pGpsSetDeviceParam = NULL;  
    }
}

DWORD LpsGpsId::OpenDevice()
{
    DWORD gpsError = GPS_ERR_SUCCESS;
    GPS_INIT gpsInit = { 0 };

    CloseDevice();

    GetGpsContext()->WriteGpsDebugLogMessage("LpsGpsId::OpenDevice(): calling GpsOpenDevice");
    m_gpsDevice = m_pGpsOpenDevice(m_gpsidNewLocationAvailable, m_gpsidDeviceStateChanged, NULL, NULL);
    GetGpsContext()->WriteGpsDebugLogMessage("LpsGpsId::OpenDevice(): back from GpsOpenDevice");

    if (!m_gpsDevice)
    {
        return GetLastError();
    }

    gpsInit.dwClientID = _wtoi(m_parameters[GPS_CONFIG_CLIENT_ID].c_str());
    gpsInit.szClientName = m_parameters[GPS_CONFIG_CLIENT_NAME].c_str();
    gpsInit.szClientPWD = m_parameters[GPS_CONFIG_CLIENT_PASSWORD].c_str();
    gpsInit.szSMSPrefix = m_parameters[GPS_CONFIG_SMS_PREFIX].c_str();

    if (gpsInit.dwClientID && *gpsInit.szClientName && *gpsInit.szClientPWD && *gpsInit.szSMSPrefix)
    {
        GetGpsContext()->WriteGpsDebugLogMessage("LpsGpsId::OpenDevice(): calling GpsSetDeviceParam(GPS_CONFIG_INIT)");
        gpsError = m_pGpsSetDeviceParam(m_gpsDevice, GPS_CONFIG_INIT, (BYTE*)&gpsInit, sizeof(GPS_INIT));
        GetGpsContext()->WriteGpsDebugLogMessage("LpsGpsId::OpenDevice(): GpsSetDeviceParam(GPS_CONFIG_INIT) returned %x", gpsError);
    }

    return gpsError;
}

void LpsGpsId::CloseDevice()
{
    if (m_gpsDevice)
    {
        GetGpsContext()->WriteGpsDebugLogMessage("LpsGpsId::CloseDevice(): calling GpsCloseDevice()");
        m_pGpsCloseDevice(m_gpsDevice);
        m_gpsDevice = NULL;
    }
}

DWORD LpsGpsId::SetMode(DWORD mode)
{
    DWORD gpsError = m_pGpsSetDeviceParam (m_gpsDevice, GPS_CONFIG_FIX_MODE, (BYTE *)&mode, sizeof(mode));
    return gpsError;
}

DWORD LpsGpsId::SetQOS(DWORD dwAccuracy, DWORD dwPerformance)
{
    GPS_QOS qos = {0};
    qos.dwAccuracy = dwAccuracy;
    qos.dwPerformance = dwPerformance;
    DWORD gpsError = m_pGpsSetDeviceParam(m_gpsDevice, GPS_CONFIG_QOS, (BYTE *)&qos, sizeof(GPS_QOS));
    return gpsError;
}

DWORD LpsGpsId::SetFixRate(WORD wNumberOfFixes, WORD wDelayBetweenFixes)
{
    GPS_FIX_RATE rate = {0};
    rate.dwNumFixes = wNumberOfFixes;             
    rate.dwTimeBetweenFixes = wDelayBetweenFixes; 
    DWORD gpsError = m_pGpsSetDeviceParam(m_gpsDevice, GPS_CONFIG_FIX_RATE, (BYTE *)&rate, sizeof(GPS_FIX_RATE));
    return gpsError;
}

void LpsGpsId::TranslateGpsPosition(const GPS_POSITION &rGpsPosition, Location& rLocation)
{
    rLocation.Reset();

    if (rGpsPosition.dwValidFields & GPS_VALID_UTC_TIME)
    {
        rLocation.gpsTime = Location::SystemTimeToGpsTime(&rGpsPosition.stUTCTime);
    }

    if (rGpsPosition.dwValidFields & GPS_VALID_LATITUDE) 
    {
        // convert to units of 180/2^25 degrees, WGS-84 ellipsoid 
        rLocation.latitude = rGpsPosition.dblLatitude / Location::LATITUDE_CONSTANT;
        rLocation.valid |= PGV_Latitude;
    }

    if (rGpsPosition.dwValidFields & GPS_VALID_LONGITUDE)
    {
        // convert to units of 180/2^25 degrees, WGS-84 ellipsoid 
        rLocation.longitude = rGpsPosition.dblLongitude / Location::LONGITUDE_CONSTANT;
        rLocation.valid |= PGV_Longitude;
    }

    if (rGpsPosition.dwValidFields & GPS_VALID_HEADING)
    {
        // convert heading to units of  360/2^10 degrees 
        rLocation.heading = rGpsPosition.flHeading / Location::HEADING_CONSTANT;
        rLocation.valid |= PGV_Heading;
    }

    if (rGpsPosition.dwValidFields & GPS_VALID_SPEED)
    {
        rLocation.horizontalVelocity = rGpsPosition.flSpeed * Location::SPEED_CONSTANT;
        rLocation.valid |= PGV_HorizontalVelocity;
    }

    if (rGpsPosition.dwValidFields & GPS_VALID_SATELLITES_IN_VIEW)
    {
        rLocation.numberOfSatellites = (uint16)rGpsPosition.dwSatelliteCount;
        rLocation.valid |= PGV_SatelliteCount;
    }

    if (rGpsPosition.dwValidFields & GPS_VALID_ALTITUDE_WRT_SEA_LEVEL)
    {
        rLocation.altitude = rGpsPosition.flAltitudeWRTSeaLevel;
        rLocation.valid |= PGV_Altitude;
    }

    if (rGpsPosition.dwValidFields & GPS_VALID_HORIZONTAL_DILUTION_OF_PRECISION)
    {
        rLocation.horizontalUncertaintyAlongAxis = rGpsPosition.flHorizontalDilutionOfPrecision * GPS_DOP_VALUE;
        rLocation.horizontalUncertaintyAlongPerpendicular = rGpsPosition.flHorizontalDilutionOfPrecision * GPS_DOP_VALUE;
        rLocation.valid |= PGV_AxisUncertainty | PGV_PerpendicularUncertainty;
    }

    if (rGpsPosition.dwValidFields & GPS_VALID_VERTICAL_DILUTION_OF_PRECISION)
    {
        rLocation.verticalUncertainty = rGpsPosition.flVerticalDilutionOfPrecision * GPS_DOP_VALUE;
        rLocation.valid |= PGV_VerticalUncertainty;
    }

    rLocation.Validate(); 

    rLocation.SetFixTime(PAL_ClockGetGPSTime());

    rLocation.status = PAL_Ok;
}

PAL_Error LpsGpsId::TranslateGpsError(DWORD gpsError)
{
    PAL_Error error = PAL_ErrGpsGeneralFailure;

    switch (gpsError)
    {
        case GPS_ERR_AUTHENTICATION_FAIL:
        case GPS_ERR_FAILED_AUTHENTICATION:
            error = PAL_ErrGpsRejected;
            break;

        case GPS_ERR_DEVICE_NOT_OPEN:
            error = PAL_ErrGpsInvalidState;
            break;

        case GPS_ERR_FAILED_EULA_DECLINED:
        case GPS_ERR_FAILED_FTT_CANCELLED:
            error = PAL_ErrGpsPrivacyRefused;
            break;

        case GPS_ERR_PENDING_SERVICE_AUTHORIZATION:
            error = PAL_ErrGpsPendingServiceAuthorization;
            break;

        case GPS_ERR_NOT_PROVISIONED:
            error = PAL_ErrGpsNotProvisioned;
            break;

        case GPS_ERR_FAILED_LOCATION_DISABLED:
            error = PAL_ErrGpsLocationDisabled;
            break;
    }

    return error;
}

GPS_MODE LpsGpsId::GetRoamingGpsMode(GPS_MODE requestedMode)
{
    GPS_MODE mode = requestedMode;

    if(wcscmp(m_parameters[GPS_CONFIG_ROAMING_MODES].c_str(), LPSGPS_ROAMING_MODE_ALL) != 0 && m_parameters[GPS_CONFIG_ROAMING_MODES].length() > 0)
    {
        DWORD dwData = 0;

        // check roaming bit in registry
        RegistryGetDWORD(SN_PHONEROAMING_ROOT, SN_PHONEROAMING_PATH, SN_PHONEROAMING_VALUE, (DWORD*)&dwData);

        if(dwData & SN_PHONEROAMING_BITMASK || GetGpsContext()->IsFakeRoamingMode())
        {
            const wchar_t* roamString = m_parameters[GPS_CONFIG_ROAMING_MODES].c_str();

            if(wcscmp(roamString, LPSGPS_ROAMING_MODE_MSA) == 0)
            {
                mode = GPS_FIX_MODE_MSA;
            }
            else if(wcscmp(roamString, LPSGPS_ROAMING_MODE_MSB) == 0)
            {
                mode = GPS_FIX_MODE_MSB;
            }
            else if(wcscmp(roamString, LPSGPS_ROAMING_MODE_MSS) == 0)
            {
                mode = GPS_FIX_MODE_MSS;
            }
        }
    }

    return mode;
}

/*! @} */
