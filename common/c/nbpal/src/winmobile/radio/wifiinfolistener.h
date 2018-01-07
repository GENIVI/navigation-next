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

    @file     wifiinfolistener.h
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

#ifndef WIFIINFOLISTENER_H
#define WIFIINFOLISTENER_H

#include "palradio.h"
#include "palradiolistener.h"

namespace nimpal
{
    namespace radio
    {
        /*! Class for Wifi information listener */
        class WifiInfoListener : public PalRadioListener
        {
        public:
            WifiInfoListener(PAL_RadioWifiNetworkInfoCallback userCallback, void* userData);
            virtual ~WifiInfoListener();
            /*! This function schedules calling user callback from the PAL thread. */
            void Callback(PAL_Error status, PAL_RadioWifiInfo* wifiInfo);

        protected:
            /*! PAL thread caller. User callback actually is called here. */
            virtual void PalThreadCallerFunc();

        private:
            void* m_userData;
            PAL_RadioWifiNetworkInfoCallback m_userCallback;
            PAL_Error m_status;
            PAL_RadioWifiInfo* m_wifiInfo;
        };

        /*! Class for complete Wifi information listener */
        class AllWifisInfoListener : public PalRadioListener
        {
        public:
            AllWifisInfoListener(PAL_RadioAllWifiNetworksInfoCallback userCallback, void* userData);
            virtual ~AllWifisInfoListener();
            /*! This function schedules calling user callback from the PAL thread. */
            void Callback(PAL_Error status, PAL_RadioCompleteWifiInfo** wifiInfo, unsigned long wifisInfoCount);
            /*! Return true if "callback" is the same as internal "m_userCallback", false - otherwise. */
            bool CheckCallback(PAL_RadioAllWifiNetworksInfoCallback callback);

        protected:
            /*! PAL thread caller. User callback actually is called here. */
            virtual void PalThreadCallerFunc();

        private:
            void* m_userData;
            PAL_RadioAllWifiNetworksInfoCallback m_userCallback;
            PAL_Error m_status;
            PAL_RadioCompleteWifiInfo* m_wifiInfo;
            unsigned long m_wifisInfoCount;
        };
    }
}

#endif
