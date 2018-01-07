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

    @file     wifiinfolistener.cpp
    @date     5/06/2010
    @defgroup PAL_RADIO PAL Radio Functions

    @brief    Classes for PAL Wifi information listeners.

    Windows implementation for PAL Wifi information listeners.
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

#include "wifiinfolistener.h"
#include "windef.h"
#include "palstdlib.h"

#define MAC_LENGTH 6

namespace nimpal
{
    namespace radio
    {
        WifiInfoListener::WifiInfoListener(PAL_RadioWifiNetworkInfoCallback userCallback,
            void* userData) : m_userData(userData), m_userCallback(userCallback),
            m_status(PAL_ErrNoInit), m_wifiInfo(NULL)
        {
        }

        WifiInfoListener::~WifiInfoListener()
        {
            if (m_wifiInfo != NULL)
            {
                delete[] m_wifiInfo->macAddress;
                delete m_wifiInfo;
            }
        }

        void WifiInfoListener::Callback(PAL_Error status, PAL_RadioWifiInfo* wifiInfo)
        {
            m_status = status;
            if (wifiInfo != NULL)
            {
                m_wifiInfo = new PAL_RadioWifiInfo;
                unsigned char* mac = new unsigned char[MAC_LENGTH];
                nsl_memcpy(mac, wifiInfo->macAddress, MAC_LENGTH);
                m_wifiInfo->macAddress = mac;
                m_wifiInfo->signalStrength = wifiInfo->signalStrength;
            }
            CallInPalThread(this);
        }

        void WifiInfoListener::PalThreadCallerFunc()
        {
            if (m_userCallback)
            {
                m_userCallback(m_status, m_wifiInfo, m_userData);
            }
            DestroyListenerAsync();
        }

        AllWifisInfoListener::AllWifisInfoListener(PAL_RadioAllWifiNetworksInfoCallback userCallback,
            void* userData) : m_userData(userData), m_userCallback(userCallback), m_status(PAL_ErrNoInit),
            m_wifiInfo(NULL), m_wifisInfoCount(0)
        {
        }

        AllWifisInfoListener::~AllWifisInfoListener()
        {
            if (m_wifiInfo != NULL)
            {
                for(unsigned long i = 0; i < m_wifisInfoCount; i++)
                {
                    delete[] m_wifiInfo[i].ssid;
                    delete[] m_wifiInfo[i].macAddress;
                }
                delete[] m_wifiInfo;
            }
        }

        void AllWifisInfoListener::Callback(PAL_Error status, PAL_RadioCompleteWifiInfo** wifiInfo, unsigned long wifisInfoCount)
        {
            m_status = status;
            m_wifisInfoCount = wifisInfoCount;
            if (wifiInfo != NULL)
            {
                m_wifiInfo = new PAL_RadioCompleteWifiInfo[wifisInfoCount];
                for(unsigned long i = 0; i < wifisInfoCount; i++)
                {
                    m_wifiInfo[i].signalStrength = (*wifiInfo)[i].signalStrength;
                    m_wifiInfo[i].frequency = (*wifiInfo)[i].frequency;
                    m_wifiInfo[i].isPrivacy = (*wifiInfo)[i].isPrivacy;

                    int ssidLen = nsl_strlen((const char*)(*wifiInfo)[i].ssid);
                    unsigned char* ssid = new unsigned char[ssidLen + 1];
                    nsl_strcpy((char*)ssid, (const char*)(*wifiInfo)[i].ssid);
                    m_wifiInfo[i].ssid = ssid;

                    unsigned char* mac = new unsigned char[MAC_LENGTH];
                    nsl_memcpy(mac, (*wifiInfo)[i].macAddress, MAC_LENGTH);
                    m_wifiInfo[i].macAddress = mac;
                }
            }
            CallInPalThread(this);
        }

        void AllWifisInfoListener::PalThreadCallerFunc()
        {
            if (m_userCallback)
            {
                m_userCallback(m_status, &m_wifiInfo, m_wifisInfoCount, m_userData);
            }
            DestroyListenerAsync();
        }

        bool AllWifisInfoListener::CheckCallback(PAL_RadioAllWifiNetworksInfoCallback callback)
        {
            return m_userCallback == callback;
        }
    }
}
