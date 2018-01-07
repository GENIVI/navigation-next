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

    @file     wifiinfo.cpp
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

#include <windows.h>
#include "wifiinfo.h"
#include <iphlpapi.h>
#pragma comment(lib,"iphlpapi.lib")
#include <winioctl.h>
#include <nuiouser.h>
#include <snapi.h>
#include "palstdlib.h"
#include "wifiinfolistener.h"

#define MAX_KEY_LENGTH 255
#define MAC_LENGTH 6
// 16384 bytes buffer should be enough to get BSSID info. The 4096 bytes is not enough,
// DeviceIoControl() returns error.
#define IOCTLBUFFER_LENGTH 16384

// We cannot read APs right after Wifi power on because Wifi scan takes some time. We start the timer
// to wait for wifi scan complete.
#define SCAN_WAIT_TIMEOUT 3000 // in milliseconds

// Wifi scan takes some time even after Wifi power on. 
#define BSSID_LIST_SCAN_TIMEOUT 500 // in milliseconds

// If user has Wifi off on his mobile device we should power it on, scan for networks and then power off.
// User can make often location requests. To avoid often power ON/OFF switches and scanning we will do the following:
// - if wifi is not ON, turn it ON and remember current state (it might be not only OFF state).
// - do the scanning
// - set power state to "low power state" in order to save battery
// - if user does not request wifi info for WIFI_ACTIVE_TIME ms, return original power state back.
#define WIFI_ACTIVE_TIME 60000 // in milliseconds

namespace nimpal
{
    namespace radio
    {

        DWORD WINAPI WifiThreadMain(LPVOID lpParam)
        { 
            WifiInfo* wifiInfo;
            bool exit = false;
            DWORD dwRet = 0;

            wifiInfo = (WifiInfo*)lpParam;
            if (wifiInfo == NULL)
            {
                exit = true;
            }
            else
            {
                SetEvent(wifiInfo->GetEvent(WifiInfo::EventThreadStarted));
            }

            while (!exit)
            {
                dwRet = MsgWaitForMultipleObjectsEx(WifiInfo::EventLast, wifiInfo->GetEventArray(), INFINITE, QS_TIMER, 0);
                if (WifiInfo::EventCallbackWifiInfo == dwRet - WAIT_OBJECT_0)
                {
                    wifiInfo->GetAPsInfo();
                }
                else if (WifiInfo::EventCallbackAllWifisInfo == dwRet - WAIT_OBJECT_0)
                {
                    wifiInfo->GetAPsInfo();
                }
                else if (WifiInfo::EventLast == dwRet - WAIT_OBJECT_0)
                {
                    // We need to read messages from the thread async message queue.
                    // Otherwise PAL_TimerSet() will not work from other thread.
                    // PAL_TimerSet() uses Win API SetTimer() which posts the WM_TIMER message
                    // to the current thread async message queue.
                    MSG msg;
                    while (PeekMessage(&msg, NULL,  0, 0, PM_REMOVE))
                    {} 
                }
                else if (WifiInfo::EventExit == dwRet - WAIT_OBJECT_0)
                {
                    exit = true;
                }
            }

            return 0; 
        }

        WifiInfo::WifiInfo(PAL_Instance* pal) :
            m_wifiInfoListener(NULL),
            m_allWifisInfoListeners(NULL),
            m_pal(pal),
            m_pIoCtlBuffer(NULL),
            m_pDeviceDriverKey(NULL),
            m_pDeviceName(NULL),
            m_cbThread(NULL),
            m_waitScanComplete(false),
            m_needPowerOffWifi(false),
            m_prevPowerState(PwrDeviceUnspecified)
        {
            m_bufferLength = IOCTLBUFFER_LENGTH + sizeof(NDISUIO_QUERY_OID);
            m_pIoCtlBuffer = (unsigned char*)nsl_malloc(m_bufferLength);
            m_allWifisInfoListeners = new std::vector<AllWifisInfoListener*>;
        }

        WifiInfo::~WifiInfo()
        {
            if (m_cbThread != NULL)
            {
                SetEvent(m_eventArr[EventExit]);
                WaitForSingleObject(m_cbThread, INFINITE);
                CloseHandle(m_cbThread);
                m_cbThread = NULL;

                DeleteCriticalSection(&m_Lock);

                for (int i = 0; i < EventLast; ++i)
                {
                    if (m_eventArr[i] != NULL)
                    {
                        CloseHandle(m_eventArr[i]);
                    }
                }
            }

            PAL_TimerCancel(m_pal, StaticScanWaitTimerCallback, this);
            PAL_TimerCancel(m_pal, StaticWifiOffTimerCallback, this);
            SetWifiPowerState(D4);

            if (m_wifiInfoListener != NULL)
            {
                delete m_wifiInfoListener;
                m_wifiInfoListener = NULL;
            }

            if (m_allWifisInfoListeners != NULL)
            {
                int count = m_allWifisInfoListeners->size();
                for (int index = 0; index < count; ++index)
                {
                    delete (AllWifisInfoListener*)m_allWifisInfoListeners->at(index);
                }
                delete m_allWifisInfoListeners;
                m_allWifisInfoListeners = NULL;
            }

            if (m_pIoCtlBuffer != NULL)
            {
                nsl_free(m_pIoCtlBuffer);
                m_pIoCtlBuffer = NULL;
            }
            if (m_pDeviceDriverKey != NULL)
            {
                nsl_free(m_pDeviceDriverKey);
                m_pDeviceDriverKey = NULL;
            }
            if (m_pDeviceName != NULL)
            {
                nsl_free(m_pDeviceName);
                m_pDeviceName = NULL;
            }
        }

        PAL_Error WifiInfo::Initialize()
        {
            FindDeviceDriverKey();
            SetWifiPowerState(D0, false);
            PAL_Error err = FindDeviceName();
            SetWifiPowerState(D4);

            if (err == PAL_Ok)
            {
                m_cbThread = CreateThread(NULL, 0, WifiThreadMain, this, 0, &m_cbThreadId);
                if (m_cbThread != NULL)
                {
                    InitializeCriticalSection(&m_Lock);
                    int i = 0;
                    for (i = 0; i < EventLast; ++i)
                    {
                        m_eventArr[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
                        if (m_eventArr[i] == NULL)
                        {
                            break;
                        }
                    }
                    if (i == EventLast)
                    {
                        WaitForSingleObject(m_eventArr[EventThreadStarted], INFINITE);
                        err = PAL_Ok;
                    }
                }
                else
                {
                    err = PAL_Failed;
                }
            }

            return err;
        }

        void WifiInfo::GetWifiInfo(WifiInfoListener* listener)
        {
            Lock();
            delete m_wifiInfoListener;
            m_wifiInfoListener = listener;
            Unlock();
            SetEvent(m_eventArr[EventCallbackWifiInfo]);
        }

        void WifiInfo::GetAllWifisInfo(AllWifisInfoListener* listener)
        {
            if (m_allWifisInfoListeners != NULL)
            {
                Lock();
                m_allWifisInfoListeners->push_back(listener);
                Unlock();
                // send event if this is the first listener added. If there are other listeners in
                // the m_allWifisInfoListeners, we've already sent event.
                if (m_allWifisInfoListeners->size() == 1)
                {
                    SetEvent(m_eventArr[EventCallbackAllWifisInfo]);
                }
            }
        }

        void WifiInfo::CancelWifiInfo()
        {
            Lock();
            delete m_wifiInfoListener;
            m_wifiInfoListener = NULL;
            Unlock();
        }

        void WifiInfo::CancelAllWifisInfo(PAL_RadioAllWifiNetworksInfoCallback callback)
        {
            if (m_allWifisInfoListeners != NULL)
            {
                Lock();
                std::vector<AllWifisInfoListener*> *l  = m_allWifisInfoListeners;
                std::vector<AllWifisInfoListener*>::iterator current;

                for (current = l->begin(); current != l->end(); current++)
                {
                    AllWifisInfoListener *listener = *current;
                    if ( listener->CheckCallback(callback) )
                    {
                        current = l->erase(current);
                        delete listener;
                        current--;
                    }
                }
                Unlock();
            }
        }

        PAL_Error WifiInfo::FindDeviceName()
        {
            // If we already found the device, do nothing
            if (m_pDeviceName != NULL)
            {
                return PAL_Ok;
            }

            // Enumerate all network adapters
            PIP_ADAPTER_INFO pAdapterInfo;
            PIP_ADAPTER_INFO pAdapter = 0;

            ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
            pAdapterInfo = (IP_ADAPTER_INFO *)nsl_malloc(sizeof(IP_ADAPTER_INFO));
            if (pAdapterInfo == NULL)
            {
                return PAL_ErrNoMem;
            }
            if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
            {
                nsl_free(pAdapterInfo);
                pAdapterInfo = (IP_ADAPTER_INFO *)nsl_malloc(ulOutBufLen);
                if (pAdapterInfo == NULL)
                {
                    return PAL_ErrNoMem;
                }
            }

            if (NO_ERROR == GetAdaptersInfo(pAdapterInfo, &ulOutBufLen))
            {
                pAdapter = pAdapterInfo;
                while (pAdapter != NULL)
                {
                    // only consider Ethernet adapters
                    if (pAdapter->Type == MIB_IF_TYPE_ETHERNET)
                    {
                        // Open the NDIS driver in order to make a IOCTL_NDISUIO_NIC_STATISTICS ioctl.
                        HANDLE ndisAccess = CreateFile(NDISUIO_DEVICE_NAME, 0, 0, 0,
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
                                INVALID_HANDLE_VALUE);
                        if (ndisAccess && ndisAccess != INVALID_HANDLE_VALUE)
                        {
                            int len = 0;
                            while (pAdapter->AdapterName[len])
                            {
                                ++len;
                            }
                            wchar_t* unicodeName = (wchar_t*)nsl_malloc((len+1) * sizeof(wchar_t));
                            for (int c = 0; c < len; ++c)
                            {
                                unicodeName[c] = (wchar_t)pAdapter->AdapterName[c];
                            }
                            unicodeName[len] = 0;

                            bool success = false;
                            NIC_STATISTICS nicStatistics = {0};
                            nicStatistics.ptcDeviceName = unicodeName;
                            nicStatistics.Size = sizeof(NIC_STATISTICS);
                            DWORD dwBytesWritten = 0;
                            success = (DeviceIoControl(ndisAccess, IOCTL_NDISUIO_NIC_STATISTICS,
                                0, 0, &nicStatistics, sizeof(NIC_STATISTICS),
                                &dwBytesWritten, 0) == TRUE) ? true : false;

                            // Only consider WIFI adapters
                            success = (success && (nicStatistics.PhysicalMediaType == NdisPhysicalMediumWirelessLan));
                            CloseHandle(ndisAccess);

                            if (success)
                            {
                                m_pDeviceName = unicodeName;
                                break;
                            }
                            else
                            {
                                nsl_free(unicodeName);
                            }
                        }
                    }

                    // Iterate to next adapter
                    pAdapter = pAdapter->Next;
                }
            }
            if (pAdapterInfo != NULL)
            {
                nsl_free(pAdapterInfo);
            }
            return PAL_Ok;
        }

        bool WifiInfo::InitNewScan(bool force)
        {
            if (m_pDeviceName == NULL)
            {
                return false;
            }
            if (!SetWifiPowerState(D0, force))
            {
                return false;
            }

            HANDLE ndisAccess = CreateFile(NDISUIO_DEVICE_NAME, GENERIC_READ, 
                FILE_SHARE_READ | FILE_SHARE_WRITE, 
                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
            if (ndisAccess == NULL || ndisAccess == INVALID_HANDLE_VALUE)
            {
                return false;
            }

            NDISUIO_QUERY_OID* query = (NDISUIO_QUERY_OID*)m_pIoCtlBuffer;
            query->Oid = OID_802_11_BSSID_LIST_SCAN;
            query->ptcDeviceName = m_pDeviceName;

            DWORD dwBytesWritten = 0;

            bool success = false;
            HANDLE devCtlEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
            if (devCtlEvent)
            {
                OVERLAPPED ov = {0};
                ov.hEvent = devCtlEvent;

                success = (DeviceIoControl(ndisAccess, IOCTL_NDISUIO_SET_OID_VALUE,
                                           (LPVOID)query, m_bufferLength, NULL, 0,
                                           &dwBytesWritten, &ov) == TRUE);

                if (success)
                {
                    // It seems DeviceIoControl doesn't support "FILE_FLAG_OVERLAPPED".
                    // This means we shouldn't care about result of the WaitForSingleObject.
                    WaitForSingleObject(devCtlEvent, BSSID_LIST_SCAN_TIMEOUT);
                }

                CloseHandle(devCtlEvent);
            }
            CloseHandle(ndisAccess);
            return success;
        }

        bool WifiInfo::EnumerateAPs()
        {
            if (m_pDeviceName == NULL)
            {
                return false;
            }
            HANDLE ndisAccess = CreateFile(NDISUIO_DEVICE_NAME, GENERIC_READ, 
                FILE_SHARE_READ | FILE_SHARE_WRITE, 
                NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
            if (ndisAccess == NULL || ndisAccess==INVALID_HANDLE_VALUE)
            {
                return false;
            }

            // clean m_pIoCtlBuffer before request
            memset(m_pIoCtlBuffer, 0, m_bufferLength);

            NDISUIO_QUERY_OID* query = (NDISUIO_QUERY_OID*)m_pIoCtlBuffer;
            query->Oid = OID_802_11_BSSID_LIST;
            query->ptcDeviceName = m_pDeviceName;
            DWORD dwBytesWritten = 0;
            bool success = (DeviceIoControl(ndisAccess, IOCTL_NDISUIO_QUERY_OID_VALUE,
                (LPVOID)query, m_bufferLength, (LPVOID)query, m_bufferLength,
                &dwBytesWritten, 0) == TRUE) ? true : false;

            CloseHandle(ndisAccess);
            return success;
        }

        void WifiInfo::GetAPsInfo()
        {
            PAL_Error status = InitNewScan(false) ? PAL_Ok : PAL_ErrRadioBssidScanFailed;
            if (status == PAL_Ok)
            {
                if (m_needPowerOffWifi)
                {
                    PAL_TimerCancel(m_pal, StaticWifiOffTimerCallback, this);
                    PAL_TimerSet(m_pal, WIFI_ACTIVE_TIME, StaticWifiOffTimerCallback, this);
                }
                if (m_waitScanComplete)
                {
                    // we cannot read APs right now because Wifi scan takes some time. It looks like
                    // there is no callback for this, so we start the timer.
                    PAL_TimerSet(m_pal, SCAN_WAIT_TIMEOUT, StaticScanWaitTimerCallback, this);
                }
                else
                {
                    status = GetAPsInfoImpl();
                }
            }
            if (status != PAL_Ok)
            {
                NotifyWithError(status);
            }
            SetWifiPowerState(D1);
        }

        PAL_Error WifiInfo::GetAPsInfoImpl()
        {
            PAL_Error status = EnumerateAPs() ? PAL_Ok : PAL_ErrRadioWlanNotFound;
            if (status == PAL_Ok)
            {
                NDIS_802_11_BSSID_LIST_EX* list;
                NDISUIO_QUERY_OID* query = (NDISUIO_QUERY_OID*)m_pIoCtlBuffer;
                list = (NDIS_802_11_BSSID_LIST_EX*)(&query->Data[0]);
                unsigned long count = list->NumberOfItems;
                NDIS_WLAN_BSSID_EX* ap = &list->Bssid[0];

                if (count == 0 || ap == NULL)
                {
                    status = PAL_Failed;
                }
                else
                {
                    if (m_allWifisInfoListeners != NULL && m_allWifisInfoListeners->size() > 0)
                    {
                        GetAllAPsInfo(ap, count);
                    }
                    else if (m_wifiInfoListener != NULL)
                    {
                        GetAPInfo(ap, count);
                    }
                }
            }

            return status;
        }

        void WifiInfo::GetAllAPsInfo(NDIS_WLAN_BSSID_EX* ap, unsigned long count)
        {
            PAL_RadioCompleteWifiInfo* allWifisInfo = new PAL_RadioCompleteWifiInfo[count];
            if (allWifisInfo == NULL)
            {
                NotifyWithError(PAL_ErrRadioWlanNotFound);
                return;
            }

            unsigned int actualCount = 0;
            // sometimes we can get some garbage in the ap, so we do additional checks here:
            // - address value in the ap has to be bound to 4 bytes(ARM limitation)
            // - ssid and mac should be allocated correctly
            for(unsigned long i = 0; (i < count) && (((unsigned int)ap & 0x03) == 0); i++)
            {
                unsigned char* ssid = NULL;
                if (ap->Ssid.SsidLength < NDIS_802_11_LENGTH_SSID)
                {
                    ssid = new unsigned char[ap->Ssid.SsidLength + 1];
                }
                unsigned char* mac = new unsigned char[MAC_LENGTH];

                if (ssid != NULL && mac != NULL)
                {
                    allWifisInfo[i].signalStrength = (int16)(ap->Rssi & 0xffff);

                    allWifisInfo[i].frequency = ap->Configuration.DSConfig;

                    nsl_memcpy(ssid, ap->Ssid.Ssid, ap->Ssid.SsidLength);
                    ssid[ap->Ssid.SsidLength] = 0;
                    allWifisInfo[i].ssid = ssid;

                    nsl_memcpy(mac, ap->MacAddress, MAC_LENGTH);
                    allWifisInfo[i].macAddress = mac;

                    allWifisInfo[i].isPrivacy = (nb_boolean)ap->Privacy;
                    actualCount++;
                }
                else
                {
                    delete[] ssid;
                    delete[] mac;
                }

                ap = (NDIS_WLAN_BSSID_EX*)((unsigned char*)ap + ap->Length);
            }

            if (actualCount != 0)
            {
                NotifyListenerAllWiFis(PAL_Ok, &allWifisInfo, actualCount);
            }
            else
            {
                NotifyWithError(PAL_Failed);
            }

            for(unsigned long i = 0; i < count; i++)
            {
                delete[] allWifisInfo[i].ssid;
                delete[] allWifisInfo[i].macAddress;
            }
            delete[] allWifisInfo;
        }

        void WifiInfo::GetAPInfo(NDIS_WLAN_BSSID_EX* ap, unsigned long count)
        {

            PAL_RadioWifiInfo wifiInfo = { 0 };
            unsigned char mac[MAC_LENGTH] = { 0 };

            wifiInfo.signalStrength = (int16)(ap->Rssi & 0xffff);
            nsl_memcpy(mac, ap->MacAddress, MAC_LENGTH);

            // looking for AP with the best RSSI
            while (0 < count--)
            {
                if (ap->Rssi > wifiInfo.signalStrength)
                {
                    wifiInfo.signalStrength = (int16)(ap->Rssi & 0xffff);
                    nsl_memcpy(mac, ap->MacAddress, MAC_LENGTH);
                }
                ap = (NDIS_WLAN_BSSID_EX*)((unsigned char*)ap + ap->Length);
            }
            wifiInfo.macAddress = mac;
            NotifyListenerSingleWiFi(PAL_Ok, &wifiInfo);
        }

        void WifiInfo::NotifyListenerSingleWiFi(PAL_Error status, PAL_RadioWifiInfo* wifiInfo)
        {
            // data might be changed in other thread
            Lock();
            WifiInfoListener* listener = m_wifiInfoListener;
            m_wifiInfoListener = NULL;
            Unlock();

            if (listener != NULL)
            {
                listener->Callback(status, wifiInfo);
            }
        }

        void WifiInfo::NotifyListenerAllWiFis(PAL_Error status, PAL_RadioCompleteWifiInfo** allWifisInfo, unsigned long count)
        {
            if (m_allWifisInfoListeners == NULL)
            {
                return;
            }

            // data might be changed in other thread
            Lock();
            std::vector<AllWifisInfoListener*> *listeners = new std::vector<AllWifisInfoListener*>(*m_allWifisInfoListeners);
            m_allWifisInfoListeners->clear();
            Unlock();

            int len = listeners->size();
            for (int index = 0; index < len; ++index)
            {
                listeners->at(index)->Callback(status, allWifisInfo, count);
            }
            delete listeners;
        }

        HANDLE* WifiInfo::GetEventArray()
        {
            return m_eventArr;
        }

        HANDLE WifiInfo::GetEvent(int idx)
        {
            if (idx < 0 || idx >= EventLast)
            {
                return NULL;
            }

            return m_eventArr[idx];
        }

        void WifiInfo::FindDeviceDriverKey()
        {
            HKEY hKey;
            m_pDeviceDriverKey = NULL;
            if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Control\\Power\\State",
                              0, KEY_READ, &hKey) == ERROR_SUCCESS )
            {
                wchar_t* keyName = (wchar_t*)nsl_malloc(MAX_KEY_LENGTH * sizeof(wchar_t));
                if (keyName != NULL)
                {
                    DWORD keyNameLength, retCode;
                    int i = 0;
                    do
                    {
                        keyNameLength = MAX_KEY_LENGTH;
                        retCode = RegEnumValue(hKey, i, keyName, &keyNameLength, 
                             NULL, NULL, NULL, NULL);

                        if (wcsstr(keyName, PMCLASS_NDIS_MINIPORT) != NULL)
                        {
                            m_pDeviceDriverKey = keyName;
                            break;
                        }

                        ++i;
                    } while (retCode != ERROR_NO_MORE_ITEMS);
                }

                if (m_pDeviceDriverKey == NULL)
                {
                    nsl_free(keyName);
                }

                RegCloseKey(hKey);
            }
        }

        bool WifiInfo::SetWifiPowerState(CEDEVICE_POWER_STATE powerState, bool force)
        {
            m_waitScanComplete = false;
            bool res = false;
            CEDEVICE_POWER_STATE curPwState = PwrDeviceUnspecified;
            DWORD retCode = GetDevicePower(m_pDeviceDriverKey, POWER_NAME, &curPwState);
            if (retCode != ERROR_SUCCESS)
            {
                return false;
            }

            if (curPwState != powerState)
            {
                if (powerState == D0) // full power state
                {
                    // do not set full power if we are in low power state. Scan can be performed in low power state.
                    if (force && curPwState != D1)
                    {
                        m_needPowerOffWifi = true;
                        m_prevPowerState = curPwState;
                        res = SetDevicePower(m_pDeviceDriverKey, POWER_NAME, D0) == ERROR_SUCCESS;
                        m_waitScanComplete = res;
                    }

                }
                else if (powerState == D1) // low power state
                {
                    if (m_needPowerOffWifi)
                    {
                        res = SetDevicePower(m_pDeviceDriverKey, POWER_NAME, D1) == ERROR_SUCCESS;
                    }
                }
                else if (powerState == D4) // power off state
                {
                    // power off actually means "set original power state back" here.
                    // if we forced wifi power on, we should return original state back.
                    if (m_needPowerOffWifi && m_prevPowerState != PwrDeviceUnspecified)
                    {
                        res = SetDevicePower(m_pDeviceDriverKey, POWER_NAME, m_prevPowerState) == ERROR_SUCCESS;
                        m_prevPowerState = PwrDeviceUnspecified;
                        m_needPowerOffWifi = false;
                    }
                }
            }
            else
            { // we are already in this power state
                res = true;
            }
            return res;
        }

        void WifiInfo::StaticScanWaitTimerCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
        {
            if (reason == PTCBR_TimerFired && userData != NULL)
            {
                WifiInfo* me = (WifiInfo*)userData;
                me->ScanWaitTimerCallback();
            }
        }

        void WifiInfo::ScanWaitTimerCallback()
        {
            PAL_Error status = GetAPsInfoImpl();
            if (status != PAL_Ok)
            {
                NotifyWithError(status);
            }
            SetWifiPowerState(D1);
        }

        void WifiInfo::StaticWifiOffTimerCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
        {
            if (reason == PTCBR_TimerFired && userData != NULL)
            {
                WifiInfo* me = (WifiInfo*)userData;
                me->WifiOffTimerCallback();
            }
        }

        void WifiInfo::WifiOffTimerCallback()
        {
            SetWifiPowerState(D4);
        }

        void WifiInfo::NotifyWithError(PAL_Error status)
        {
            if (m_allWifisInfoListeners != NULL && m_allWifisInfoListeners->size() > 0)
            {
                NotifyListenerAllWiFis(status, NULL, 0);
            }
            else if (m_wifiInfoListener != NULL)
            {
                NotifyListenerSingleWiFi(status, NULL);
            }
        }
    }
}
