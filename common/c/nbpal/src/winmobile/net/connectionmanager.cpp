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

    @file     connectionmanager.cpp
    @date     1/5/2009
    @defgroup PAL_NET PAL Network I/O Functions

    Platform-independent network I/O API.

    Windows implementation of ConnectionManager base class.
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

#ifdef WINCE
#include <objbase.h>
#include <initguid.h>
#include <connmgr.h>
#include <connmgr_status.h>
#include "connectionmanager.h"
#include "palwindow.h"

namespace nimpal
{
    namespace network
    {
        //////////////////////////////////////////////////////////////////////////
        // static class members & functions
        //////////////////////////////////////////////////////////////////////////

        #define NIM_CONNMGR_TIMEOUT            60 * 1000        // milliseconds
        #define NIM_CONNMGR_DISC_IMMEDIATE     0                // Immediate disconnection
        #define NIM_CONNMGR_DISC_REGVALUE      1                // Use registry value

        ConnectionManager* ConnectionManager::m_instance = 0;

        ConnectionManager::ConnectionManager()
                :m_ConnMgr(0), m_windowMsgId(0)
        {
            // Set all registered events array to ZERO
            memset(m_ErrorEventArray, 0, sizeof(m_ErrorEventArray));
 
            // Inialize critical section
            InitializeCriticalSection(&m_Lock);
        }

        void ConnectionManager::ConnectionStatusCallback(WPARAM wParam, LPARAM lParam)
        {
            if (m_instance)
            {
                switch(wParam)
                {
                    case CONNMGR_STATUS_CONNECTED:
                    case CONNMGR_STATUS_WAITINGCONNECTION:
                         break;

                    default:
                     // Connection is not available
                     if (m_instance)
                     {
                         m_instance->SignalConnectionErrorEvents();
                     }
                     break;
                }
            }
        }


        boolean ConnectionManager::InitializeConnectionManager()
        {
            if (m_instance == NULL)
            {
                m_instance = new ConnectionManager();
            }
            else
            {
                // Already initialized
                return TRUE;
            }

            m_instance->m_windowMsgId = RegisterWindowMessage(CONNMGR_STATUS_CHANGE_NOTIFICATION_MSG);
            //Register message id with PALWindow
            PALWindow::Instance()->PALWindowRegisterMessageCallback(ConnectionStatusCallback,
                                                                     m_instance->m_windowMsgId);
            return TRUE;
        }

        boolean ConnectionManager::RegisterForConnectionErrorEvent(boolean enable, HANDLE errorEvent)
        {
            boolean success = FALSE;
            if(m_instance)
            {
                m_instance->Lock();
                for (uint32 i=0; i< CONNECTION_MANAGER_MAXIMUM_CONNECTIONS; i++)
                {
                    if (TRUE == enable)
                    {
                        if (m_instance->m_ErrorEventArray[i] == errorEvent)
                        {
                            // Event already registered
                            success = TRUE;
                            break;
                        }
                        else if (m_instance->m_ErrorEventArray[i] == 0)
                        {
                            // Got a empty slot for event
                            m_instance->m_ErrorEventArray[i] = errorEvent;
                            success = TRUE;
                            break;
                        }
                    }
                    else
                    {
                        if (m_instance->m_ErrorEventArray[i] == errorEvent)
                        {
                            // Unregister. Empty the event slot
                            m_instance->m_ErrorEventArray[i] = 0;
                            success = TRUE;
                            break;
                        }
                    }
                }
                m_instance->Unlock();
            }
            return success;
        }

        boolean ConnectionManager::EstablishConnection()
        {
            if (!m_instance)
            {
                //Connection manager not initialized
                return FALSE;
            }

            boolean connMgrOk = TRUE;
            HRESULT hr = E_FAIL;
            DWORD status = CONNMGR_STATUS_UNKNOWN;
            GUID guid = {0};
            DWORD index = 0;

            CONNMGR_CONNECTIONINFO connMgrInfo;
            memset(&connMgrInfo, 0, sizeof(CONNMGR_CONNECTIONINFO));

            // Establish connection with Connection Manager
            if (m_instance->m_ConnMgr != NULL)
            {
                if (S_OK != ConnMgrConnectionStatus(m_instance->m_ConnMgr, &status))
                {
                    // Failed to get connection status
                    return FALSE;
                }
            }


            if (status != CONNMGR_STATUS_CONNECTED)
            {
                connMgrInfo.cbSize = sizeof(CONNMGR_CONNECTIONINFO);

                // Indicate valid pointer params
                connMgrInfo.dwParams = CONNMGR_PARAM_GUIDDESTNET;

                // Set to highest data priority
                connMgrInfo.dwPriority = CONNMGR_PRIORITY_HIPRIBKGND;

                // Proxies supported
                connMgrInfo.dwFlags = CONNMGR_FLAG_PROXY_HTTP | CONNMGR_FLAG_PROXY_WAP | CONNMGR_FLAG_PROXY_SOCKS4 | CONNMGR_FLAG_PROXY_SOCKS5;

                // Shareable
                connMgrInfo.bExclusive = FALSE;
                connMgrInfo.bDisabled = FALSE;

                // Determine and set appropriate network destination guid
                hr =  ConnMgrMapURL(_T("http://nimone.com"), &guid, &index);
                if(SUCCEEDED(hr))
                {
                    connMgrInfo.guidDestNet = guid;
                }
                else
                {
                    connMgrInfo.guidDestNet = IID_DestNetInternet;
                }

                // Set parameters for receiving connection status notification
                connMgrInfo.hWnd = PALWindow::Instance()->GetPALWindow();
                connMgrInfo.uMsg = m_instance->m_windowMsgId;
                connMgrInfo.lParam = 0;

                if (ConnMgrEstablishConnectionSync(&connMgrInfo, &m_instance->m_ConnMgr, NIM_CONNMGR_TIMEOUT, &status) == S_OK)
                {
                    if (status != CONNMGR_STATUS_CONNECTED)
                    {
                        connMgrOk = FALSE;
                    }
                }
            }
            return connMgrOk;
        }

        void ConnectionManager::SignalConnectionErrorEvents()
        {
            // Signal all connection objects about error on connection.
            Lock();
            for (uint32 i=0; i< CONNECTION_MANAGER_MAXIMUM_CONNECTIONS; i++)
            {
                if (m_ErrorEventArray[i] != 0)
                {
                    // Signal event
                    SetEvent(m_ErrorEventArray[i]);
                }
            }
            Unlock();
        }

        void ConnectionManager::CleanupConnectionManager()
        {
            // Free connection manager
            if (m_instance && m_instance->m_ConnMgr)
            {
                ConnMgrReleaseConnection(m_instance->m_ConnMgr, NIM_CONNMGR_DISC_IMMEDIATE);
                m_instance->m_ConnMgr = NULL;
            }
        }
    }
}
#endif
