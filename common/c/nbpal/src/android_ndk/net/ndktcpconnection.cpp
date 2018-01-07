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

    @file     NDKTcpConnection.cpp
    @date     09/27/2012
    @defgroup PAL_NET PAL Network I/O Functions

    Platform-independent network I/O API.

    JAVA implementation for TcpConnection class.
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

/* all the file's code goes here */

//#include "palstdlib.h"
#include "ndktcpconnection.h"
#include "paldebuglog.h"
#include "palnetjni.h"

using namespace nimpal::network;

namespace nimpal
{
    namespace network
    {
        NDKTcpConnection::NDKTcpConnection(PAL_NetConnectionProtocol protocol, PAL_Instance* pal) :
            NetConnection(protocol, pal), m_handleID(0), m_jobject(NULL){}

        NDKTcpConnection::~NDKTcpConnection()
        {
            Close();
        }

        /* See description in netconnection.h */
        void NDKTcpConnection::Destroy()
        {
            delete this;
        }

        PAL_Error NDKTcpConnection::Connect(const char* pHostName, uint16 port, uint32 localIpAddr)
        {
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::Connect: %s %u\n", pHostName, port));
            if (nsl_strempty(pHostName))
            {
                return PAL_ErrNoData;
            }

            if (m_hostName != pHostName) // prevent copying to itself
            {
                nsl_strncpy(m_hostName, pHostName, HOSTNAME_MAX_LEN);
                m_hostName[HOSTNAME_MAX_LEN-1] = '\0';  // always terminates
            }

            m_port = port;
            m_localIpAddr = localIpAddr;

            if (PAL_Ok != CreateSocketConnection())
            {
                SetStatus(PNCS_Failed);
                return PAL_ErrNetGeneralFailure;
            }

            return PAL_Ok;
        }

        PAL_Error NDKTcpConnection::Send(const byte* pBuffer, int count)
        {
            if(!pBuffer || count <= 0 || !m_jobject)
            {
                return PAL_Failed;
            }

            if(PalNetJNI::Send(m_jobject, (char*)pBuffer, count))
            {
                return PAL_Ok;
            }

            return PAL_Failed;
        }

        PAL_Error NDKTcpConnection::Send(const byte* pBuffer,
                                       uint32      count,
                                       const char* pVerb,
                                       const char* pObject,
                                       const char* pAcceptType,
                                       const char* pAdditionalHeaders,
                                       void*       pRequestData)
        {
            return PAL_ErrUnsupported;
        }

        PAL_Error NDKTcpConnection::Close()
        {
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::Close\n"));
            PalNetJNI::Close(m_jobject, m_handleID);
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::Close SUCCESS\n"));
            SetStatus(PNCS_Closed);
            m_jobject = NULL;
            return PAL_Ok;
        }

        PAL_Error NDKTcpConnection::CreateSocketConnection(void)
        {
            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::CreateSocketConnection\n"));

            PalNetJNI::SetPalInstance(m_pal);
            m_jobject = PalNetJNI::CreateSocket((void*)this, m_handleID);
            if(!m_jobject)
            {
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("TcpConnection::CreateSocketConnection FAILED. Can't create.\n"));
                return PAL_ErrNetGeneralFailure;
            }
            SetStatus(PNCS_Created);

            if(!PalNetJNI::Connect(m_jobject, m_hostName, m_port))
            {
                DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_MAJOR, ("TcpConnection::CreateSocketConnection FAILED. Can't connect\n"));
                return PAL_ErrNetGeneralFailure;
            }

            DEBUGLOG(LOG_SS_NB_NET, LOG_SEV_INFO, ("TcpConnection::CreateSocketConnection SUCCESS\n"));
            return PAL_Ok;
        }

        void NDKTcpConnection::SetTcpStatus(PAL_NetConnectionStatus status)
        {
            this->SetStatus(status);
        }

        void NDKTcpConnection::NotifyTcpDataReceived(const byte* pBytes, unsigned int count)
        {
            this->NotifyDataReceived(pBytes, count);
        }
    }   //network
}   //nimpal
/*! @} */
