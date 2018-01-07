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

    @file     systemmodelistener.cpp
    @date     4/30/2010
    @defgroup PAL_RADIO PAL Radio Functions

    @brief    Class for PAL System mode listener.

    Windows implementation for PAL System mode listener.
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

#include "systemmodelistener.h"
#include "riltypes.h"

namespace nimpal
{
    namespace radio
    {
        SystemModeListener::SystemModeListener(PAL_RadioSystemModeCallback userCallback,
            void* userData) : m_userData(userData), m_userCallback(userCallback),
            m_status(PAL_ErrNoInit), m_networkType(RIL_SYSTEMTYPE_NONE)
        {
        }

        void SystemModeListener::Callback(PAL_Error status, int32 networkType)
        {
            m_status = status;
            m_networkType = networkType;
            CallInPalThread(this);
        }

        void SystemModeListener::PalThreadCallerFunc()
        {
            if (m_userCallback)
            {
                PAL_RadioSystemMode type = PRSM_NoService;
                if ( m_networkType & (RIL_SYSTEMTYPE_GSM | RIL_SYSTEMTYPE_GPRS | RIL_SYSTEMTYPE_EDGE) )
                {
                    type = PRSM_GSM;
                }
                else if ( m_networkType & (RIL_SYSTEMTYPE_1XRTTPACKET  |
                                           RIL_SYSTEMTYPE_1XEVDOPACKET |
                                           RIL_SYSTEMTYPE_1XEVDVPACKET) )
                {
                    type = PRSM_CDMA;
                }
                m_userCallback(m_status, type, m_userData);
            }
            DestroyListenerAsync();
        }
    }
}
