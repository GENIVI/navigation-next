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

    @file     cellinfolistener.cpp
    @date     4/28/2010
    @defgroup PAL_RADIO PAL Radio Functions

    @brief    Class for PAL Cell information listener.

    Windows implementation for PAL Cell information listener.
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

#include "cellinfolistener.h"

namespace nimpal
{
    namespace radio
    {
        CellInfoListener::CellInfoListener(void* userData) :
            m_userData(userData), m_status(PAL_ErrNoInit)
        {
        }

        GSMCellInfoListener::GSMCellInfoListener(PAL_RadioGSMNetworkInfoCallback userCallback,
            void* userData) : CellInfoListener(userData), m_userCallback(userCallback)
        {
            memset(&m_gsmInfo, 0, sizeof(PAL_RadioGSMInfo));
        }

        void GSMCellInfoListener::Callback(PAL_Error status, const RILCELLTOWERINFO* cellInfo)
        {
            m_status = status;
            if (cellInfo != NULL)
            {
                m_gsmInfo.cellId = cellInfo->dwCellID;
                m_gsmInfo.locationAreaCode = cellInfo->dwLocationAreaCode;
                m_gsmInfo.mobileCountryCode = cellInfo->dwMobileCountryCode;
                m_gsmInfo.mobileNetworkCode = cellInfo->dwMobileNetworkCode;
                m_gsmInfo.signalStrength = (uint16)(cellInfo->dwRxLevel & 0xffff);
            }
            CallInPalThread(this);
        }

        void GSMCellInfoListener::PalThreadCallerFunc()
        {
            if (m_userCallback)
            {
                m_userCallback(m_status, &m_gsmInfo, m_userData);
            }
            DestroyListenerAsync();
        }

        CDMACellInfoListener::CDMACellInfoListener(PAL_RadioCDMANetworkInfoCallback userCallback,
            void* userData) : CellInfoListener(userData), m_userCallback(userCallback)
        {
            memset(&m_cdmaInfo, 0, sizeof(PAL_RadioCDMAInfo));
        }

        void CDMACellInfoListener::Callback(PAL_Error status, const RILCELLTOWERINFO* cellInfo)
        {
            m_status = status;
            if (cellInfo != NULL)
            {
                m_cdmaInfo.cellId = cellInfo->dwCellID;
                m_cdmaInfo.networkId = cellInfo->dwLocationAreaCode;
                m_cdmaInfo.signalStrength = (uint16)(cellInfo->dwRxLevel & 0xffff);
                m_cdmaInfo.systemId = cellInfo->dwMobileNetworkCode;
            }
            CallInPalThread(this);
        }

        void CDMACellInfoListener::PalThreadCallerFunc()
        {
            if (m_userCallback)
            {
                m_userCallback(m_status, &m_cdmaInfo, m_userData);
            }
            DestroyListenerAsync();
        }
    }
}
