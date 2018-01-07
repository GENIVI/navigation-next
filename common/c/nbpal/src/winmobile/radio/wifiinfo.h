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

    @file     wifiinfo.h
    @date     5/05/2010
    @defgroup PAL_RADIO PAL Radio Functions

    @brief    Class for PAL Wifi information.

    Windows implementation for PAL Wifi information.
*/
/*
(C) Copyright 2010 by TeleCommunication Systems, Inc.

The information contained herein is confidential, proprietary 
to TeleCommunication Systems, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of TeleCommunication Systems is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#ifndef WIFIINFO_H
#define WIFIINFO_H

#include <vector>
#include <windows.h>
#include <ntddndis.h>
#include <pm.h>
#include "paltimer.h"
#include "palradio.h"

namespace nimpal
{
    namespace radio
    {
        class WifiInfoListener;
        class AllWifisInfoListener;

        class WifiInfo
        {
        public:
            enum CbThreadEvents
            {
                EventCallbackWifiInfo = 0,
                EventCallbackAllWifisInfo,
                EventExit,
                EventThreadStarted,
                EventLast
            };

            WifiInfo(PAL_Instance* pal);
            ~WifiInfo();
            PAL_Error Initialize();
            /*! Get Wifi info for the AP with the best signal strength request. Request is asynch. Result will be returned in callback. */
            void GetWifiInfo(WifiInfoListener* listener);
            /*! Get Wifi info for all APs request. Request is asynch. Result will be returned in callback. */
            void GetAllWifisInfo(AllWifisInfoListener* listener);
            /*! Cancel Wifi info request. */
            void CancelWifiInfo();
            /*! Cancel Wifi info for all APs request. */
            void CancelAllWifisInfo(PAL_RadioAllWifiNetworksInfoCallback callback);
            /*! Start APs info acquirement. */
            void GetAPsInfo();
            HANDLE* GetEventArray();
            HANDLE  GetEvent(int idx);

        private:
            /*! Get adapter name. */
            PAL_Error FindDeviceName();
            /*! Start new wifi scan. */
            bool InitNewScan(bool force);
            /*! Enumerate APs found. */
            bool EnumerateAPs();
            /*! Get info for AP with the best signal strength. */
            void GetAPInfo(NDIS_WLAN_BSSID_EX* ap, unsigned long count);
            /*! Get info for all APs. */
            void GetAllAPsInfo(NDIS_WLAN_BSSID_EX* ap, unsigned long count);
            /*! Notify listener of single Wifi AP info. */
            void NotifyListenerSingleWiFi(PAL_Error status, PAL_RadioWifiInfo* wifiInfo);
            /*! Notify all listeners of all Wifi APs info. */
            void NotifyListenerAllWiFis(PAL_Error status, PAL_RadioCompleteWifiInfo** allWifisInfo, unsigned long count);

            void Lock() { EnterCriticalSection(&m_Lock); }
            void Unlock() { LeaveCriticalSection(&m_Lock); }

            /*! Set Wifi device power state. */
            bool SetWifiPowerState(CEDEVICE_POWER_STATE powerState, bool force = false);
            /*! Get device power driver name. */
            void FindDeviceDriverKey();
            PAL_Error GetAPsInfoImpl();
            /*! Notify all listeners with error. */
            void NotifyWithError(PAL_Error status);

            static void StaticScanWaitTimerCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);
            void ScanWaitTimerCallback();
            static void StaticWifiOffTimerCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);
            void WifiOffTimerCallback();

        private:
            WifiInfoListener*  m_wifiInfoListener;
            std::vector<AllWifisInfoListener*>  *m_allWifisInfoListeners;

            PAL_Instance*  m_pal;
            HANDLE   m_cbThread;
            DWORD    m_cbThreadId;
            HANDLE   m_eventArr[EventLast];
            CRITICAL_SECTION m_Lock;

            wchar_t*        m_pDeviceDriverKey;
            wchar_t*        m_pDeviceName;
            unsigned char*  m_pIoCtlBuffer;
            unsigned long   m_bufferLength;

            bool                  m_waitScanComplete;
            bool                  m_needPowerOffWifi;
            CEDEVICE_POWER_STATE  m_prevPowerState;
        };
    }
}

#endif
