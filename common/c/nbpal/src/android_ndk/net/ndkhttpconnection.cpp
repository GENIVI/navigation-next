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

    @file     ndkhttpconnection.cpp
    @date     10/12/2013
    @defgroup PAL_NET PAL Network I/O Functions

    Platform-independent network I/O API.

    JAVA implementation for HttpConnection class.
*/
/*
    See file description in header file.

    (C) Copyright 2013 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "ndkhttpconnection.h"
#include "paldebuglog.h"
#include "palnetjni.h"
#include <string>

namespace nimpal
{
    namespace network
    {

        NDKHttpConnection::NDKHttpConnection(PAL_NetConnectionProtocol protocol, PAL_Instance* pal) :
                   NetConnection(protocol, pal), m_pal(pal), m_jobject(NULL), m_handleID(0)
        {
            m_isSecurityConnection = (protocol == PNCP_HTTPS) ? true : false;
            m_status = PNCS_Initialized;
        }

        NDKHttpConnection::~NDKHttpConnection()
        {
            Close();
        }

        PAL_Error NDKHttpConnection::Close()
        {
            PalNetJNI::HttpClose(m_jobject, m_handleID);
            SetStatus(PNCS_Closed);
            m_jobject = NULL;
            return PAL_Ok;
        }

        /* See description in netconnection.h */
        void NDKHttpConnection::Destroy()
        {
            // Remove all pending callbacks of this connection.
            StopAllCallbacks();
            if (IsInCallback())
            {
                TaskId taskId = 0;
                PAL_EventTaskQueueAdd(m_pal,
                                      (PAL_TaskQueueCallback) DelayDestroy,
                                      (void*) this,
                                      &taskId);
            }
            else
            {
                delete this;
            }
        }

        PAL_Error NDKHttpConnection::Connect(const char* pHostName, uint16 port, uint32 localIpAddr)
        {
            if (nsl_strempty(pHostName) || port == 0)
            {
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("HttpConnection::Connect pHostName is empty! \n"));
                return PAL_ErrNoData;
            }

            m_jobject = PalNetJNI::HttpCreate((void*)this, m_handleID, m_isSecurityConnection);
            if( !m_jobject)
            {
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("HttpConnection::Connect FAILED. Can't create.\n"));
                return PAL_ErrNetGeneralFailure;
            }

            if(!PalNetJNI::HttpConnect(m_jobject, std::string(pHostName), port, m_isSecurityConnection))
            {
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("HttpConnection::Connect FAILED. Can't connect\n"));
                return PAL_ErrNetGeneralFailure;
            }

            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("HttpConnection::Connect SUCCESS\n"));
            SetStatus(PNCS_Connected);
            return PAL_Ok;
        }

        PAL_Error NDKHttpConnection::Send(const byte* pBuffer, int count)
        {
            return Send(pBuffer, count, NULL, NULL, NULL, NULL, NULL);
        }

        PAL_Error NDKHttpConnection::Send(const byte* pBuffer,
                                       uint32      count,
                                       const char* pVerb,
                                       const char* pObject,
                                       const char* pAcceptType,
                                       const char* pAdditionalHeaders,
                                       void*       pRequestData)
        {
            if(!PalNetJNI::HttpSend(m_jobject, (char*)pBuffer,count,pVerb,pObject,
                    pAcceptType,pAdditionalHeaders,pRequestData))
            {
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("HttpConnection::Send FAILED. Can't Send\n"));
                return PAL_Failed;
            }
            return PAL_Ok;
        }

        void NDKHttpConnection::DelayDestroy(PAL_Instance* pal, NDKHttpConnection* connection)
        {
            if (!connection)
            {
                return;
            }
            connection->Destroy();
        }

        void NDKHttpConnection::onHttpNetworkStatus(PAL_NetConnectionStatus status)
        {
            SetStatus(status);
        }

        void NDKHttpConnection::onHttpDataReceived(void* pRequestData, PAL_Error errorCode, const byte* pBytes, unsigned int count)
        {
            NotifyHttpDataReceived(pRequestData, errorCode, pBytes, count);
        }

        void NDKHttpConnection::onHttpResponseStatus(void* pRequestData, unsigned int status)
        {
            NotifyHttpResponseStatus(pRequestData, status);
        }

        void NDKHttpConnection::onHttpResponseHeaders(void* pRequestData, char* responseHeaders)
        {
            NotifyHttpResponseHeaders(pRequestData, responseHeaders);
        }
    }
}

/*! @} */
