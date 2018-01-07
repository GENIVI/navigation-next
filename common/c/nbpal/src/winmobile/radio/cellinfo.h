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

    @file     cellinfo.h
    @date     4/27/2010
    @defgroup PAL_RADIO PAL Radio Functions

    @brief    Class for PAL Cell information.

    Windows implementation for PAL Cell information.
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

#ifndef CELLINFO_H
#define CELLINFO_H

#include <windows.h>
#include "palradio.h"
#include "riltypes.h"

namespace nimpal
{
    namespace radio
    {
        class CellInfoListener;
        class SystemModeListener;

        /*! Class to provide Cell information
        Get cell information using Windows Mobile Radio Interface Layer(RIL).
        */
        class CellInfo
        {
        public:
            CellInfo();
            ~CellInfo();
            PAL_Error Initialize();
            /*! Get Cell ID info request. Request is asynch. Result will be returned in callback. */
            void GetCellInfo(CellInfoListener* listener);
            /*! Get network type request. Request is asynch. Result will be returned in callback. */
            void GetNetworkType(SystemModeListener* listener);
            /*! RIL interface callback. */
            void HandleCallback(DWORD dwCode, HRESULT hrCmdID, const void* lpData, DWORD cbData);

        private:
            void Lock() { EnterCriticalSection(&m_Lock); }
            void Unlock() { LeaveCriticalSection(&m_Lock); }

        private:
            CellInfoListener*     m_cellInfoListener;
            SystemModeListener*   m_systemModeListener;

            HRESULT  m_cellInfoRequest;
            HRESULT  m_systemTypeRequest;

            CRITICAL_SECTION m_Lock;

            // We need to load "ril.dll" using LoadLibrary() because Window Mobile SDK doesn't contain library file
            // for it. Also we use our own dummy "ril.dll" on WM emulator(which doesn't contain this library at all).
            HMODULE  m_hRilDll;
            HRIL     m_hRil;

            PFRIL_Initialize            m_RIL_Initialize;
            PFRIL_Deinitialize          m_RIL_Deinitialize;
            PFRIL_GetCellTowerInfo      m_RIL_GetCellTowerInfo;
            PFRIL_GetCurrentSystemType  m_RIL_GetCurrentSystemType;
        };
    }
}

#endif
