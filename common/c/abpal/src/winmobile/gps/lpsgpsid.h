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

@file lpsgpsid.h
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

#include <map>
#include <string>
#include "locationprovider.h"
#include "lbsdriver.h"

typedef struct _GPS_POSITION GPS_POSITION;
typedef struct _GPS_DEVICE GPS_DEVICE;

// Needed to load procs dynamically
typedef HANDLE	(WINAPI *LPSGPSID_OpenDevice)(HANDLE, HANDLE, const WCHAR *, DWORD);
typedef DWORD	(WINAPI *LPSGPSID_GetPosition)(HANDLE, GPS_POSITION *, DWORD, DWORD);
typedef DWORD	(WINAPI *LPSGPSID_GetDeviceState)(HANDLE, GPS_DEVICE *);
typedef DWORD	(WINAPI *LPSGPSID_CloseDevice)(HANDLE);
typedef DWORD	(WINAPI *LPSGPSID_GetDeviceParam)(HANDLE, DWORD , BYTE* , DWORD*);
typedef DWORD	(WINAPI *LPSGPSID_SetDeviceParam)(HANDLE, DWORD , BYTE* , DWORD);

#define LPSGPS_ROAMING_MODE_ALL				L"all"
#define LPSGPS_ROAMING_MODE_MSA				L"MSA"
#define LPSGPS_ROAMING_MODE_MSB				L"MSB"
#define LPSGPS_ROAMING_MODE_MSS				L"MSS"

class LpsGpsId : public LocationProvider
{
public:
    LpsGpsId(GpsContext* gpsContext, std::map<std::string, std::wstring> parameters);
    virtual ~LpsGpsId();

    /* Update listeners requested parameters */
    virtual void UpdateFixRequestParameters(bool listenersPresent, uint32 lowestCommonInterval, uint32 lowestCommonAccuracy);

protected:    
    virtual PAL_Error       CreateInstance();
    virtual PAL_Error       DestroyInstance();

    HANDLE                  m_gpsDevice;
    HMODULE                 m_gpsapiDll;

    HANDLE                  m_gpsidNewLocationAvailable;
    HANDLE                  m_gpsidDeviceStateChanged;
    HANDLE                  m_gpsidFixRequestParametersChanged;

    LPSGPSID_OpenDevice     m_pGpsOpenDevice;
    LPSGPSID_GetPosition    m_pGpsGetPosition;
    LPSGPSID_GetDeviceState m_pGpsGetDeviceState;
    LPSGPSID_CloseDevice    m_pGpsCloseDevice;   
    LPSGPSID_GetDeviceParam m_pGpsGetDeviceParam;
    LPSGPSID_SetDeviceParam m_pGpsSetDeviceParam;
    
    CRITICAL_SECTION        m_fixParametersLock;
    uint16                  m_fixInterval;
    uint16                  m_fixAccuracy;
    DWORD                   m_inactivityTimeout;

private:
    DWORD                   LoadDriver();
    void                    UnloadDriver();
    
    DWORD                   OpenDevice();
    void                    CloseDevice();
    
    void                    LockFixParameters() { EnterCriticalSection(&m_fixParametersLock); };
    void                    UnlockFixParameters() { LeaveCriticalSection(&m_fixParametersLock); };

    void                    RequestNextFix(bool& rFixQueryPending, uint32& rFixCount);
    DWORD                   RestartDevice(bool& rGpsServiceInitialized, bool& rFixQueryPending, uint32& rFixCount);
    
    DWORD                   SetMode(DWORD mode);
    DWORD                   SetQOS(DWORD dwAccuracy, DWORD dwPerformance);
    DWORD                   SetFixRate(WORD wNumberOfFixes, WORD wDelayBetweenFixes);
    
    void                    TranslateGpsPosition(const GPS_POSITION& rGpsPosition, Location& rLocation);
    PAL_Error               TranslateGpsError(DWORD gpsError);

    GPS_MODE                GetRoamingGpsMode(GPS_MODE requestedMode);

    DWORD                   AcquisitionThreadProc();
    
    std::map<std::string, std::wstring> m_parameters;
};


/*! @} */
