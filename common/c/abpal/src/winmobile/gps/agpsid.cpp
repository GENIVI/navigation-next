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

@file agpsid.cpp
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

#include "palstdlib.h"
#include "agpsid.h"
#include <Winsock2.h>
#include <snapi.h>
#include "gpsapiaku4.h"
#include "gpsconfignames.h"

const uint32 DEFAULT_FIX_RATE_FIX_COUNT = 65536;
const uint32 DEFAULT_FIX_RATE_INTERVAL_SECONDS = 1;
const uint16 DEFAULT_QOS_ACCURACY_METERS = 200;
const uint16 DEFAULT_QOS_PERFORMANCE_SECONDS = 16;

const uint32 QUERY_RESPONSE_TIMEOUT_SECONDS = DEFAULT_QOS_PERFORMANCE_SECONDS * 4;

AGpsId::AGpsId(GpsContext* gpsContext, std::map<std::string, std::wstring> parameters) :
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
	GetGpsContext()->WriteGpsDebugLogMessage("AGpsId constructor");	

	InitializeCriticalSection(&m_fixParametersLock);
}

AGpsId::~AGpsId()
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

	GetGpsContext()->WriteGpsDebugLogMessage("AGpsId destructor");
}




PAL_Error AGpsId::CreateInstance()
{
	GetGpsContext()->WriteGpsDebugLogMessage("AGpsId::CreateInstance()");

	PAL_Error error = PAL_ErrNoInit;
	DWORD gpsError = AGPS_ERR_FATAL_ERROR;

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
	if (gpsError != AGPS_ERR_SUCCESS)
	{
		goto exit;
	}

	gpsError = OpenDevice();

	char* errorXml = "<?xml version=\"1.0\"?><result>TODO</result>";
	if (gpsError != AGPS_ERR_SUCCESS)
	{
		/// @todo Add error information to xml
		errorXml = "<?xml version=\"1.0\"?><result>TODO</result>";
		goto exit;
	}

	/// @todo notify callback through context

	error = PAL_Ok;

exit:
	if (gpsError != AGPS_ERR_SUCCESS)
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

PAL_Error AGpsId::DestroyInstance()
{
	GetGpsContext()->WriteGpsDebugLogMessage("AGpsId::DestroyInstance()");

	StopThread();

    CloseDevice();

	// Unload the Dll
	if (m_gpsapiDll)
	{
		FreeLibrary(m_gpsapiDll);
		m_gpsapiDll = NULL;
		m_pGpsOpenDevice = NULL;
		m_pGpsGetPosition = NULL;
		m_pGpsGetDeviceState = NULL;
		m_pGpsCloseDevice = NULL;      
		m_pGpsGetDeviceParam = NULL;
		m_pGpsSetDeviceParam = NULL;  
	}

	return PAL_Ok;
}

DWORD AGpsId::LoadDriver()
{

	if (!m_gpsapiDll)
	{
		m_gpsapiDll = LoadLibrary(L"gpsapi.dll");
	}

	if (m_gpsapiDll)
	{
		// Get all the procs
		m_pGpsOpenDevice = (AGPSID_OpenDevice)GetProcAddress(m_gpsapiDll, L"GPSOpenDevice");
		m_pGpsGetPosition = (AGPSID_GetPosition)GetProcAddress(m_gpsapiDll, L"GPSGetPosition");
		m_pGpsGetDeviceState = (AGPSID_GetDeviceState)GetProcAddress(m_gpsapiDll, L"GPSGetDeviceState");
		m_pGpsCloseDevice = (AGPSID_CloseDevice)GetProcAddress(m_gpsapiDll, L"GPSCloseDevice");
		m_pGpsGetDeviceParam = (AGPSID_GetDeviceParam)GetProcAddress(m_gpsapiDll, L"GPSGetDeviceParam");
		m_pGpsSetDeviceParam = (AGPSID_SetDeviceParam)GetProcAddress(m_gpsapiDll, L"GPSSetDeviceParam");

		if (!m_pGpsOpenDevice ||!m_pGpsGetPosition || !m_pGpsGetDeviceState || !m_pGpsCloseDevice)
		{
			return GetLastError();          
		}

	}

	return 0;
}

DWORD AGpsId::RestartDevice(bool& rGpsServiceInitialized, bool& rFixQueryPending, uint32& rFixCount)
{
	CloseDevice();

	rGpsServiceInitialized = FALSE;
	rFixQueryPending = FALSE;
	rFixCount = 0;

	return OpenDevice();
}

DWORD AGpsId::OpenDevice()
{
	DWORD gpsError = AGPS_ERR_SUCCESS;
	DWORD openFlags = wcscmp(m_parameters[GPS_CONFIG_DEVICE_NAME].c_str(), AGPS_DEVICE_LGE_GW825) != 0 ? AGPS_DATA_FLAGS_HARDWARE_OFF : 0;

	CloseDevice();

	GetGpsContext()->WriteGpsDebugLogMessage("AGpsId::OpenDevice(): calling GpsOpenDevice");
	m_gpsDevice = m_pGpsOpenDevice(m_gpsidNewLocationAvailable, m_gpsidDeviceStateChanged, NULL, openFlags);
	GetGpsContext()->WriteGpsDebugLogMessage("AGpsId::OpenDevice(): back from GpsOpenDevice");

	if (!m_gpsDevice)
	{
		return GetLastError();
	}

	/* Device/Chipset specific implementations

		HTC: Uses data structure as shown in AGPS_SERVER_CONFIG to pass PDE params
		Samsung Omnia: Uses data structure as shown in SGPS_SERVER_CONFIG to pass PDE params
		LG Monaco (GW825): As an HSPA device, it uses the SUPL server instead of a PDE (which is strictly for CDMA devices).
		SUPL support is set within the HSPA device and therefore we should not try to define an IP address, opening the device should be sufficient		   
	*/

	if(wcscmp(m_parameters[GPS_CONFIG_DEVICE_NAME].c_str(), AGPS_DEVICE_OMNIA) == 0)
	{
		SGPS_SERVER_CONFIG sgpsServerConfig = { 0 };
		std::string address;
		address.resize(m_parameters[GPS_CONFIG_PDE_ADDRESS].length());
		wcstombs(&address[0], m_parameters[GPS_CONFIG_PDE_ADDRESS].c_str(), address.length());

		GetGpsContext()->WriteGpsDebugLogMessage("AGpsId::OpenDevice(): selecting SAMSUNG device");
		sgpsServerConfig.gpsOneLock = 0;
		sgpsServerConfig.PDE_IP_Address = inet_addr(address.c_str());
		sgpsServerConfig.PDE_IP_Port = _wtoi(m_parameters[GPS_CONFIG_PDE_PORT].c_str());

		if (sgpsServerConfig.PDE_IP_Address)
		{
			GetGpsContext()->WriteGpsDebugLogMessage("AGpsId::OpenDevice(): calling GpsSetDeviceParam(GPS_CONFIG_INIT)");
			gpsError = m_pGpsSetDeviceParam(m_gpsDevice, AGPS_CONFIG_CUSTOM, (BYTE *)&sgpsServerConfig, sizeof(SGPS_SERVER_CONFIG));
			GetGpsContext()->WriteGpsDebugLogMessage("AGpsId::OpenDevice(): GpsSetDeviceParam(GPS_CONFIG_INIT) returned %x", gpsError);
		}
	}
	else if(wcscmp(m_parameters[GPS_CONFIG_DEVICE_NAME].c_str(), AGPS_DEVICE_LGE_GW825) != 0)
	{
		AGPS_SERVER_CONFIG gpsServerConfig = { 0 };
		struct sockaddr_in m_SockStorage;
		DWORD dwGpsLock = GPS_LOCK_OFF;
		std::string address;
		address.resize(m_parameters[GPS_CONFIG_PDE_ADDRESS].length());
		wcstombs(&address[0], m_parameters[GPS_CONFIG_PDE_ADDRESS].c_str(), address.length());

		GetGpsContext()->WriteGpsDebugLogMessage("AGpsId::OpenDevice(): selecting standard device");
		
		m_SockStorage.sin_addr.S_un.S_addr = inet_addr(address.c_str());
		m_SockStorage.sin_port = (u_short)_wtoi(m_parameters[GPS_CONFIG_PDE_PORT].c_str());
		nsl_memcpy(&(gpsServerConfig.GPSServerAddr), &m_SockStorage, sizeof(SOCKADDR_STORAGE));
		gpsServerConfig.hGPSDevice = m_gpsDevice;
		gpsServerConfig.dwValidFields = AGPS_VALID_SERVER_IPADDRESS;
		gpsServerConfig.dwFlags=0;

		gpsError = m_pGpsSetDeviceParam(m_gpsDevice, AGPS_CONFIG_CUSTOM_WRITE_GPS_LOCK, (BYTE*)&dwGpsLock,sizeof(dwGpsLock));

		if (m_SockStorage.sin_addr.S_un.S_addr && gpsError == AGPS_ERR_SUCCESS)
		{
			GetGpsContext()->WriteGpsDebugLogMessage("AGpsId::OpenDevice(): calling GpsSetDeviceParam(GPS_CONFIG_INIT)");
			gpsError = m_pGpsSetDeviceParam(m_gpsDevice, AGPS_CONFIG_CUSTOM_LOCAL_PDE, (BYTE *)&gpsServerConfig, sizeof(AGPS_SERVER_CONFIG));
			GetGpsContext()->WriteGpsDebugLogMessage("AGpsId::OpenDevice(): GpsSetDeviceParam(GPS_CONFIG_INIT) returned %x", gpsError);
		}
	}

	return gpsError;
}

void AGpsId::CloseDevice()
{
	DWORD gpsError;

	if (m_gpsDevice)
	{
		//stop any fixes
		gpsError = m_pGpsSetDeviceParam(m_gpsDevice, AGPS_STOP_FIX, NULL, 0);

		//clear out device params
		if(wcscmp(m_parameters[GPS_CONFIG_DEVICE_NAME].c_str(), AGPS_DEVICE_OMNIA) == 0)
		{
			SGPS_SERVER_CONFIG sgpsServerConfig = { 0 };

			GetGpsContext()->WriteGpsDebugLogMessage("AGpsId::CloseDevice(): closing SAMSUNG device");
			sgpsServerConfig.gpsOneLock = 0;
			sgpsServerConfig.PDE_IP_Address = 0;
			sgpsServerConfig.PDE_IP_Port = 1;

			GetGpsContext()->WriteGpsDebugLogMessage("AGpsId::CloseDevice(): calling GpsSetDeviceParam(GPS_CONFIG_INIT)");
			gpsError = m_pGpsSetDeviceParam(m_gpsDevice, AGPS_CONFIG_CUSTOM, (BYTE *)&sgpsServerConfig, sizeof(SGPS_SERVER_CONFIG));
			GetGpsContext()->WriteGpsDebugLogMessage("AGpsId::CloseDevice(): GpsSetDeviceParam(GPS_CONFIG_INIT) returned %x", gpsError);
		}
		else if(wcscmp(m_parameters[GPS_CONFIG_DEVICE_NAME].c_str(), AGPS_DEVICE_LGE_GW825) != 0)
		{
			AGPS_SERVER_CONFIG gpsServerConfig = { 0 };
			struct sockaddr_in m_SockStorage;

			GetGpsContext()->WriteGpsDebugLogMessage("AGpsId::CloseDevice(): closing standard device");

			m_SockStorage.sin_addr.S_un.S_addr = 0;
			m_SockStorage.sin_port = 1;
			nsl_memcpy(&(gpsServerConfig.GPSServerAddr), &m_SockStorage, sizeof(SOCKADDR_STORAGE));
			gpsServerConfig.hGPSDevice = m_gpsDevice;
			gpsServerConfig.dwValidFields = AGPS_VALID_SERVER_IPADDRESS;
			gpsServerConfig.dwFlags=0;

			GetGpsContext()->WriteGpsDebugLogMessage("AGpsId::CloseDevice(): calling GpsSetDeviceParam(GPS_CONFIG_INIT)");
			gpsError = m_pGpsSetDeviceParam(m_gpsDevice, AGPS_CONFIG_CUSTOM_LOCAL_PDE, (BYTE *)&gpsServerConfig, sizeof(AGPS_SERVER_CONFIG));
			GetGpsContext()->WriteGpsDebugLogMessage("AGpsId::CloseDevice(): GpsSetDeviceParam(GPS_CONFIG_INIT) returned %x", gpsError);
			
		}

		GetGpsContext()->WriteGpsDebugLogMessage("AGpsId::CloseDevice(): calling GpsCloseDevice()");
		m_pGpsCloseDevice(m_gpsDevice);
		m_gpsDevice = NULL;
	}
}

void AGpsId::UpdateFixRequestParameters(bool listenersPresent, uint32 lowestCommonInterval, uint32 lowestCommonAccuracy)
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

void AGpsId::RequestNextFix(bool& rFixQueryPending, uint32& rFixCount)
{
	LockFixParameters();

	if (m_fixInterval)
	{
		DWORD gpsError = AGPS_ERR_SUCCESS;

		// for first fix use AGPS_FIX_MODE_MSA, otherwise AGPS_FIX_MODE_MSB
		if (rFixCount == 0)
		{
			gpsError = gpsError ? gpsError : SetMode(GetRoamingGpsMode(AGPS_FIX_MODE_MSA));
			gpsError = gpsError ? gpsError : SetFixRate(DEFAULT_FIX_RATE_FIX_COUNT, m_fixInterval);
			gpsError = gpsError ? gpsError : SetQOS(m_fixAccuracy, DEFAULT_QOS_PERFORMANCE_SECONDS);

			GetGpsContext()->WriteGpsDebugLogMessage("  SetMode(MSA), SetQOS(%d, %d), SetFixRate(%d, %d) result: %X",
				m_fixAccuracy, DEFAULT_QOS_PERFORMANCE_SECONDS, DEFAULT_FIX_RATE_FIX_COUNT, m_fixInterval, gpsError);
		}
		else if (rFixCount == 3) //switch to MS-B after three fixes, either due to retry or because it's a tracking session
		{
			if(wcscmp(m_parameters["testmodes"].c_str(), L"forcemsa") == 0)
				gpsError = gpsError ? gpsError : SetMode(AGPS_FIX_MODE_MSA);
			else
				gpsError = gpsError ? gpsError : SetMode(GetRoamingGpsMode(AGPS_FIX_MODE_MSB));

			GetGpsContext()->WriteGpsDebugLogMessage("  SetMode(MSB) result: %X", gpsError);
		}

		gpsError = gpsError ? gpsError : m_pGpsSetDeviceParam(m_gpsDevice, AGPS_QUERY_FIX, NULL, 0);

		GetGpsContext()->WriteGpsDebugLogMessage("  RequestNextFix result: %X", gpsError);

		if (gpsError == AGPS_ERR_SUCCESS)
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

DWORD AGpsId::AcquisitionThreadProc()
{
	GetGpsContext()->WriteGpsDebugLogMessage("Entering AGPS acquisition thread");

	bool gpsServiceInitialized = FALSE;
	bool fixQueryPending = FALSE;
	uint32 fixCount = 0;

	AGPS_POSITION gpsPosition = { 0 };
	gpsPosition.dwSize = sizeof(gpsPosition);
	gpsPosition.dwVersion = AGPS_VERSION_2;

	AGPS_DEVICE gpsDevice = { 0 };
	gpsDevice.dwSize = sizeof(gpsDevice);
	gpsDevice.dwVersion = AGPS_VERSION_2;

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

			result = m_pGpsGetPosition(m_gpsDevice, &gpsPosition, 2000, 0);
			GetGpsContext()->WriteGpsDebugLogMessage("  New position event: GpsGetPosition result: %X, (%f, %f) valid fields %X", result, gpsPosition.dblLatitude, gpsPosition.dblLongitude, gpsPosition.dwValidFields);

			switch (result)
			{
			case AGPS_ERR_SUCCESS:
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

			case AGPS_ERR_TRACKING_SESSION_EXPIRED:
				{
					DWORD gpsError = AGPS_ERR_SUCCESS;

					// simply re-programming the session after it expires doesn't appear to work... so trying close & re-open GPS
					// gpsError = gpsError ? gpsError : SetFixRate(DEFAULT_FIX_RATE_FIX_COUNT, m_fixInterval);
					// gpsError = gpsError ? gpsError : SetQOS(m_fixAccuracy, DEFAULT_QOS_PERFORMANCE_SECONDS);

					gpsError = RestartDevice(gpsServiceInitialized, fixQueryPending, fixCount);

					GetGpsContext()->WriteGpsDebugLogMessage("  Close & re-Open device result: %X", gpsError);
					break;
				}    
			case AGPS_ERR_GPSONE:
				GetGpsContext()->WriteGpsDebugLogMessage("  GpsGetPosition GPS_ERR_GPSONE dwFixError: %X", gpsPosition.dwFixError);

				switch (gpsPosition.dwFixError)
				{
				case AGPS_ERR_INSUFFICIENT_SAT_SIGNAL:
				case AGPS_ERR_POSITION_INFO_NOT_AVAILABLE:
				case AGPS_ERR_GETPOS_TIMEOUT:
				case AGPS_ERR_NETWORK_UNREACHABLE:
				case AGPS_ERR_DEVICE_BUSY:
				case ERROR_CANCELLED:
					GetGpsContext()->NotifyLocationProviderStateChange("", PAL_ErrGpsInformationUnavailable, "");
					RequestNextFix(fixQueryPending, fixCount);
					break;

				default:
					GetGpsContext()->NotifyLocationProviderStateChange("", PAL_ErrGpsGeneralFailure, "");
					break;
				}
				break;

			default:
				GetGpsContext()->NotifyLocationProviderStateChange("", PAL_ErrGpsGeneralFailure, "");
				break;
			}
			break;

		case WAIT_OBJECT_0 + 1:     // device state change
			if (!gpsServiceInitialized)
			{
				result = m_pGpsGetDeviceState(&gpsDevice);			
				GetGpsContext()->WriteGpsDebugLogMessage("  Device state event: GpsGetDeviceState result: %X device state: %X service state: %X", result, gpsDevice.dwDeviceState, gpsDevice.dwServiceState);

				switch (result)
				{
				case AGPS_ERR_SUCCESS:
					if (gpsServiceInitialized)
					{
						/// @todo Report device state xml via device state callback
						GetGpsContext()->NotifyLocationProviderStateChange("", PAL_Ok, "");
					}
					else
					{
						if (gpsDevice.dwServiceState && gpsDevice.dwDeviceState)
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
					GetGpsContext()->NotifyLocationProviderStateChange("", PAL_ErrGpsGeneralFailure, "");
					break;                        
				}
			}
			break;

		case WAIT_OBJECT_0 + 2:     // fix parameters changed
			GetGpsContext()->WriteGpsDebugLogMessage("  FixRequestParameterChanged event: (gpsInitialized %d, fixQueryPending: %d)", gpsServiceInitialized, fixQueryPending);
			if (gpsServiceInitialized && !fixQueryPending)
			{
				//reset fixCount when the parameters change so that it tries to get MS-A fixes for one shot fixes and the start of tracking sessions
				fixCount = 0;
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
				if (RestartDevice(gpsServiceInitialized, fixQueryPending, fixCount) != AGPS_ERR_SUCCESS)
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

DWORD AGpsId::SetMode(DWORD mode)
{
	DWORD gpsError = m_pGpsSetDeviceParam (m_gpsDevice, AGPS_CONFIG_FIX_MODE, (BYTE *)&mode, sizeof(mode));
	return gpsError;
}

DWORD AGpsId::SetQOS(DWORD dwAccuracy, DWORD dwPerformance)
{
	AGPS_QOS qos = {0};
	qos.dwAccuracy = dwAccuracy;
	qos.dwPerformance = dwPerformance;
	DWORD gpsError = m_pGpsSetDeviceParam(m_gpsDevice, AGPS_CONFIG_QOS, (BYTE *)&qos, sizeof(AGPS_QOS));
	return gpsError;
}

DWORD AGpsId::SetFixRate(DWORD wNumberOfFixes, DWORD wDelayBetweenFixes)
{
	AGPS_FIX_RATE rate = {0};
	rate.dwNumFixes = wNumberOfFixes;             
	rate.dwTimeBetweenFixes = wDelayBetweenFixes; 
	DWORD gpsError = m_pGpsSetDeviceParam(m_gpsDevice, AGPS_CONFIG_FIX_RATE, (BYTE *)&rate, sizeof(AGPS_FIX_RATE));
	return gpsError;
}

void AGpsId::TranslateGpsPosition(const AGPS_POSITION &rGpsPosition, Location& rLocation)
{
	rLocation.Reset();

	if (rGpsPosition.dwValidFields & AGPS_VALID_UTC_TIME)
	{
		rLocation.gpsTime = Location::SystemTimeToGpsTime(&rGpsPosition.stUTCTime);
	}

	if (rGpsPosition.dwValidFields & AGPS_VALID_LATITUDE) 
	{
		// convert to units of 180/2^25 degrees, WGS-84 ellipsoid 
		rLocation.latitude = rGpsPosition.dblLatitude / Location::LATITUDE_CONSTANT;
		rLocation.valid |= PGV_Latitude;
	}

	if (rGpsPosition.dwValidFields & AGPS_VALID_LONGITUDE)
	{
		// convert to units of 180/2^25 degrees, WGS-84 ellipsoid 
		rLocation.longitude = rGpsPosition.dblLongitude / Location::LONGITUDE_CONSTANT;
		rLocation.valid |= PGV_Longitude;
	}

	if (rGpsPosition.dwValidFields & AGPS_VALID_HEADING)
	{
		// convert heading to units of  360/2^10 degrees 
		rLocation.heading = rGpsPosition.flHeading / Location::HEADING_CONSTANT;
		rLocation.valid |= PGV_Heading;
	}

	if (rGpsPosition.dwValidFields & AGPS_VALID_SPEED)
	{
		rLocation.horizontalVelocity = rGpsPosition.flSpeed * Location::SPEED_CONSTANT;
		rLocation.valid |= PGV_HorizontalVelocity;
	}

	if (rGpsPosition.dwValidFields & AGPS_VALID_SATELLITES_IN_VIEW)
	{
		rLocation.numberOfSatellites = (uint16)rGpsPosition.dwSatelliteCount;
	}

    if (rGpsPosition.dwValidFields & AGPS_VALID_ALTITUDE_WRT_SEA_LEVEL)
    {
        rLocation.altitude = rGpsPosition.flAltitudeWRTSeaLevel;
        rLocation.valid |= PGV_Altitude;
    }

	rLocation.Validate(); 

	rLocation.status = PAL_Ok;
}

//if the device is roaming, use the specified gps mode for roaming if one has been specified
AGPS_FIX_MODE AGpsId::GetRoamingGpsMode(AGPS_FIX_MODE requestedMode)
{
	AGPS_FIX_MODE mode = requestedMode;

	if(wcscmp(m_parameters[GPS_CONFIG_ROAMING_MODES].c_str(), AGPS_ROAMING_MODE_ALL) != 0 && m_parameters[GPS_CONFIG_ROAMING_MODES].length() > 0)
	{
		DWORD dwData;

		//check roaming bit in registry. I don't like having to check this here, but if we passed from app we'd have to reinit gps each time
		RegistryGetDWORD(SN_PHONEROAMING_ROOT,SN_PHONEROAMING_PATH,SN_PHONEROAMING_VALUE,
			(DWORD*)&dwData);

		if(dwData & SN_PHONEROAMING_BITMASK)
		{

			const wchar_t* roamString = m_parameters[GPS_CONFIG_ROAMING_MODES].c_str();
			
			if(wcscmp(roamString, AGPS_ROAMING_MODE_MSA) == 0)
				mode = AGPS_FIX_MODE_MSA;
			else if(wcscmp(roamString, AGPS_ROAMING_MODE_MSB) == 0)
				mode = AGPS_FIX_MODE_MSB;
			else if(wcscmp(roamString, AGPS_ROAMING_MODE_MSS) == 0)
				mode = AGPS_FIX_MODE_MSS;

		}
	}

	return mode;
}
/*! @} */
