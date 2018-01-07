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

@file gpsid.cpp
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

#include <string>
#include "windows.h"
#include "gpsid.h"
#include "palclock.h"
#include "location.h"
#include "notificationthread.h"

#include "gpsapi.h"

const DWORD INACTIVITY_TIMEOUT_MSEC = 2 * 60 * 1000;

GpsId::GpsId(GpsContext* gpsContext) :
    LocationProvider(gpsContext),
    m_gpsDevice(0),
    m_gpsapiDll(0),
    m_gpsidNewLocationAvailable(0),
    m_gpsidDeviceStateChanged(0),
    m_pGpsOpenDevice(0),
    m_pGpsGetPosition(0),
    m_pGpsGetDeviceState(0),
    m_pGpsCloseDevice(0)
{
    GetGpsContext()->WriteGpsDebugLogMessage("GpsId constructor");
}


GpsId::~GpsId()
{
    if (m_gpsidNewLocationAvailable)
    {
        CloseHandle(m_gpsidNewLocationAvailable);
    }

    if (m_gpsidDeviceStateChanged)
    {
        CloseHandle(m_gpsidDeviceStateChanged);
    }
    
    GetGpsContext()->WriteGpsDebugLogMessage("GpsId destructor");
}


DWORD GpsId::AcquisitionThreadProc()
{
    GetGpsContext()->WriteGpsDebugLogMessage("Entering GPSID acquisition thread");

    nb_boolean initialized = FALSE;
    nb_boolean stopThread = FALSE;

    GPS_POSITION gpsPosition = { 0 };
    gpsPosition.dwSize = sizeof(gpsPosition);
    gpsPosition.dwVersion = GPS_VERSION_1;

    GPS_DEVICE gpsDevice = { 0 };
    gpsDevice.dwSize = sizeof(gpsDevice);
    gpsDevice.dwVersion = GPS_VERSION_1;

    Location lastLocationFix;

    HANDLE gpsEvents[] = {
        m_gpsidNewLocationAvailable,
        m_gpsidDeviceStateChanged,
        GetStopEvent()
    };
    DWORD eventCount = sizeof(gpsEvents) / sizeof(gpsEvents[0]);

    do
    {
        DWORD result = WaitForMultipleObjects(eventCount, gpsEvents, FALSE, INACTIVITY_TIMEOUT_MSEC);
        switch (result)
        {
            case WAIT_OBJECT_0:
                if (!initialized) 
                {
                    // If we haven't received m_gpsidDeviceStateChanged, but received m_gpsidNewLocationAvailable - 
                    // we consider driver was initialized externally, e.g. by other NBI client or VZ Navigator 
                    result = m_pGpsGetDeviceState(&gpsDevice);
                    GetGpsContext()->WriteGpsDebugLogMessage("  Device state event: GpsGetDeviceState result: %X device state: %X service state: %X", result, gpsDevice.dwDeviceState, gpsDevice.dwServiceState);
                    if( gpsDevice.dwDeviceState && gpsDevice.dwServiceState)
                    {
                        initialized = TRUE;
                        GetGpsContext()->NotifyLocationProviderInitialized(PAL_Ok, "");
                    }
                }

                result = m_pGpsGetPosition(m_gpsDevice, &gpsPosition, 1000, 0);
                GetGpsContext()->WriteGpsDebugLogMessage("  New position event: GpsGetPosition result: %X, (%f, %f) valid fields %X", result, gpsPosition.dblLatitude, gpsPosition.dblLongitude, gpsPosition.dwValidFields);

                if (result != ERROR_SUCCESS)
                {
                    GetGpsContext()->NotifyLocationProviderStateChange("", PAL_ErrGpsGeneralFailure, "");
                }
                else
                {
                    if (gpsPosition.dwValidFields != 0)
                    {
                        TranslateGpsPosition(gpsPosition, lastLocationFix);
                        UpdateLocation(lastLocationFix);
                    }
                    else
                    {
                        GetGpsContext()->NotifyLocationProviderStateChange("", PAL_ErrGpsInformationUnavailable, "");
                    }
                }
                break;

            case WAIT_OBJECT_0 + 1:
                result = m_pGpsGetDeviceState(&gpsDevice);
                GetGpsContext()->WriteGpsDebugLogMessage("  Device state event: GpsGetDeviceState result: %X device state: %X service state: %X", result, gpsDevice.dwDeviceState, gpsDevice.dwServiceState);

                if (result != ERROR_SUCCESS)
                {
                    GetGpsContext()->NotifyLocationProviderStateChange("", PAL_ErrGpsInformationUnavailable, "");
                }
                else
                {
                    if (!initialized && gpsDevice.dwDeviceState && gpsDevice.dwServiceState)
                    {
                        initialized = TRUE;
                        GetGpsContext()->NotifyLocationProviderInitialized(PAL_Ok, "");
                    }
                }
                break;

            case WAIT_OBJECT_0 + 2:
                GetGpsContext()->WriteGpsDebugLogMessage("  Stop thread event");
                stopThread = TRUE;
                break;

            case WAIT_TIMEOUT:
                GetGpsContext()->WriteGpsDebugLogMessage("  Timeout event: (%d msec timeout, initialized: %u)", INACTIVITY_TIMEOUT_MSEC, initialized);
                GetGpsContext()->NotifyLocationProviderStateChange("", PAL_ErrGpsTimeout, "");
                break;

            default:
                ASSERT(0);
                break;
        }
    } while (!stopThread);

    GetGpsContext()->WriteGpsDebugLogMessage("Exiting acquisition thread");

    return 0;
}

PAL_Error GpsId::CreateInstance()
{
    GetGpsContext()->WriteGpsDebugLogMessage("GpsId::CreateInstance()");

    HRESULT hr = E_FAIL;
    DWORD winerr = 0;
    PAL_Error error = PAL_ErrNoInit;

    m_gpsidNewLocationAvailable = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!m_gpsidNewLocationAvailable)
    {
        winerr = GetLastError();
        hr = HRESULT_FROM_WIN32(winerr);
        goto exit;
    }

    m_gpsidDeviceStateChanged = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!m_gpsidDeviceStateChanged)
    {
        winerr = GetLastError();
        hr = HRESULT_FROM_WIN32(winerr);
        goto exit;
    }

    hr = LoadGpsIdDll();

    char* errorXml = "<?xml version=\"1.0\"?><result>TODO</result>";
    if (FAILED(hr))
    {
        /// @todo Add error information to xml
        errorXml = "<?xml version=\"1.0\"?><result>TODO</result>";
        goto exit;
    }

    /// @todo notify callback through context

    error = PAL_Ok;
    hr = S_OK;

exit:
    if (FAILED(hr))
    {
        error = PAL_ErrNoInit;
    }

    return error;
}

PAL_Error GpsId::DestroyInstance()
{
    GetGpsContext()->WriteGpsDebugLogMessage("GpsId::DestroyInstance()");

    StopThread();
    
    if (m_gpsDevice)
    {
        m_pGpsCloseDevice(m_gpsDevice);
        m_gpsDevice = NULL;
    }
    
    // Unload the Dll
    if (m_gpsapiDll)
    {
        FreeLibrary(m_gpsapiDll);
        m_gpsapiDll = NULL;
        m_pGpsOpenDevice = NULL;
        m_pGpsGetPosition = NULL;
        m_pGpsGetDeviceState = NULL;
        m_pGpsCloseDevice = NULL;       
    }

    return PAL_Ok;
}


HRESULT GpsId::LoadGpsIdDll()
{
    HRESULT hr = E_FAIL;
    DWORD winerr = 0;

    if (!m_gpsapiDll)
    {
        m_gpsapiDll = LoadLibrary(L"gpsapi.dll");
    }
    
    if (m_gpsapiDll)
    {
        // Get all the procs
        m_pGpsOpenDevice = (GPSID_OpenDevice)GetProcAddress(m_gpsapiDll, L"GPSOpenDevice");
        m_pGpsGetPosition = (GPSID_GetPosition)GetProcAddress(m_gpsapiDll, L"GPSGetPosition");
        m_pGpsGetDeviceState = (GPSID_GetDeviceState)GetProcAddress(m_gpsapiDll, L"GPSGetDeviceState");
        m_pGpsCloseDevice = (GPSID_CloseDevice)GetProcAddress(m_gpsapiDll, L"GPSCloseDevice");

        if (!m_pGpsOpenDevice ||!m_pGpsGetPosition || !m_pGpsGetDeviceState || !m_pGpsCloseDevice)
        {
            winerr = GetLastError();
            hr = HRESULT_FROM_WIN32(winerr);
            return hr;           
        }

        m_gpsDevice = m_pGpsOpenDevice(m_gpsidNewLocationAvailable, m_gpsidDeviceStateChanged, NULL, NULL);
        if (!m_gpsDevice)
        {
            winerr = GetLastError();
            hr = HRESULT_FROM_WIN32(winerr);
            return hr;
        }   
    }

    return S_OK;
}

void GpsId::TranslateGpsPosition(const GPS_POSITION& rGpsPosition, Location& rLocation)
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

/*! @} */
