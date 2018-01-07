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

    @file     cellinfo.cpp
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

#include "cellinfo.h"
#include "palstdlib.h"
#include "cellinfolistener.h"
#include "systemmodelistener.h"
#include "palradiolistener.h"

#define INVALID_HRESULT   -1
#define RIL_RESULT_FAILED  0

namespace nimpal
{
    namespace radio
    {
        static void CALLBACK RilCallback(DWORD dwCode, HRESULT hrCmdID, const void* lpData, DWORD cbData, DWORD dwParam);

        static const WCHAR RIL_DLL_NAME[] = L"ril.dll";
        static const WCHAR RIL_INITIALIZE_FUNCTION[] = L"RIL_Initialize";
        static const WCHAR RIL_DEINITIALIZE_FUNCTION[] = L"RIL_Deinitialize";
        static const WCHAR RIL_GETCELLTOWERINFO_FUNCTION[] = L"RIL_GetCellTowerInfo";
        static const WCHAR RIL_GETCURRENTSYSTEMTYPE_FUNCTION[] = L"RIL_GetCurrentSystemType";

        CellInfo::CellInfo() :
            m_cellInfoListener(NULL),
            m_systemModeListener(NULL),
            m_cellInfoRequest(INVALID_HRESULT),
            m_systemTypeRequest(INVALID_HRESULT),
            m_hRilDll(NULL),
            m_hRil(NULL),
            m_RIL_Initialize(NULL),
            m_RIL_Deinitialize(NULL),
            m_RIL_GetCellTowerInfo(NULL),
            m_RIL_GetCurrentSystemType(NULL)
        {
            InitializeCriticalSection(&m_Lock);
        }

        CellInfo::~CellInfo()
        {
            DeleteCriticalSection(&m_Lock);

            if(m_hRil != NULL)
            {
                m_RIL_Deinitialize(m_hRil);
            }

            m_RIL_Initialize = NULL;
            m_RIL_Deinitialize = NULL;
            m_RIL_GetCellTowerInfo = NULL;
            m_RIL_GetCurrentSystemType = NULL;

            if (m_hRilDll != NULL)
            {
                FreeLibrary(m_hRilDll);
            }
        }

        PAL_Error CellInfo::Initialize()
        {
            PAL_Error err = PAL_Failed;
            m_hRilDll = LoadLibrary( RIL_DLL_NAME );
            m_hRil = NULL;

            if( m_hRilDll )
            {
                m_RIL_Initialize = (PFRIL_Initialize)GetProcAddress( m_hRilDll, RIL_INITIALIZE_FUNCTION );
                m_RIL_Deinitialize = (PFRIL_Deinitialize)GetProcAddress( m_hRilDll, RIL_DEINITIALIZE_FUNCTION );
                m_RIL_GetCellTowerInfo = (PFRIL_GetCellTowerInfo)GetProcAddress( m_hRilDll, RIL_GETCELLTOWERINFO_FUNCTION );
                m_RIL_GetCurrentSystemType = (PFRIL_GetCurrentSystemType)GetProcAddress( m_hRilDll, RIL_GETCURRENTSYSTEMTYPE_FUNCTION );

                err = m_RIL_Initialize( 1, RilCallback, NULL, 0, (DWORD)this, &m_hRil ) == S_OK ?
                        PAL_Ok : PAL_ErrRadioNoInit;
            }
            else
            {
                err = PAL_ErrRadioLibraryNotLoad;
            }
            return err;
        }

        void CellInfo::GetCellInfo(CellInfoListener* listener)
        {
            Lock();
            // if m_cellInfoRequest == INVALID_HRESULT we do not have pending request now
            if (m_cellInfoRequest == INVALID_HRESULT)
            {
                m_cellInfoRequest = m_RIL_GetCellTowerInfo(m_hRil);
            }

            delete m_cellInfoListener;
            m_cellInfoListener = listener;
            Unlock();

            if (m_cellInfoRequest < 0)
            {
                HandleCallback(RIL_RESULT_FAILED, m_cellInfoRequest, NULL, 0);
            }
        }

        void CellInfo::GetNetworkType(SystemModeListener* listener)
        {
            Lock();
            // if m_systemTypeRequest == INVALID_HRESULT we do not have pending request now
            if (m_systemTypeRequest == INVALID_HRESULT)
            {
                m_systemTypeRequest = m_RIL_GetCurrentSystemType(m_hRil);
            }

            delete m_systemModeListener;
            m_systemModeListener = listener;
            Unlock();

            if (m_systemTypeRequest < 0)
            {
                HandleCallback(RIL_RESULT_FAILED, m_systemTypeRequest, NULL, 0);
            }
        }

        void CellInfo::HandleCallback(DWORD dwCode, HRESULT hrCmdID, const void* lpData, DWORD cbData)
        {
            if (hrCmdID == m_cellInfoRequest)
            {
                Lock();
                if (m_cellInfoListener != NULL)
                {
                    m_cellInfoListener->Callback(dwCode == RIL_RESULT_OK ? PAL_Ok : PAL_Failed,
                                                 (const RILCELLTOWERINFO*)lpData);
                    m_cellInfoListener = NULL; // object will be destroyed asynchronously, do not use it.
                }
                m_cellInfoRequest = INVALID_HRESULT;
                Unlock();
            }
            else if (hrCmdID == m_systemTypeRequest)
            {
                Lock();
                if (m_systemModeListener != NULL)
                {
                    m_systemModeListener->Callback(dwCode == RIL_RESULT_OK ? PAL_Ok : PAL_Failed,
                                                   lpData != NULL ? *((int32*)lpData) : 0);
                    m_systemModeListener = NULL; // object will be destroyed asynchronously, do not use it.
                }
                m_systemTypeRequest = INVALID_HRESULT;
                Unlock();
            }
        }

        void CALLBACK RilCallback(DWORD dwCode, HRESULT hrCmdID, const void* lpData, DWORD cbData, DWORD dwParam)
        {
            ((CellInfo*)dwParam)->HandleCallback(dwCode, hrCmdID, lpData, cbData);
        }
    }
}

