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

    @file     tcpconnection.h
    @date     03/02/2012
    @defgroup PAL_NET PAL Network I/O Functions

    @brief    Class for PAL HTTP network connections.

    Qt QNX implementation for PAL HTTP network functions.
*/
/*
    (C) Copyright 2012 by TeleCommunication Systems, Inc.

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
#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H
#include <string>
#include <netconnection.h>
#include "bq.h"

namespace nimpal
{
    namespace network
    {
        const int INVALID_SOCKET = -1;
        const int SELECT_ERROR = -1;
        const unsigned int READ_BUFFER_SIZE = 4096;

        class TcpConnection : public NetConnection
        {
        public:
            TcpConnection(PAL_NetConnectionProtocol protocol, PAL_Instance* pal);
            /* See description in netconnection.h */
            virtual void Destroy();

            virtual PAL_Error Connect(const char* pHostName, uint16 port, uint32 localIpAddr=0);
            virtual PAL_Error Send(const byte* pBuffer, int count);
            virtual PAL_Error Close();
            virtual PAL_Error Send(const byte* pBuffer,
                                   uint32      count,
                                   const char* pVerb,
                                   const char* pObject,
                                   const char* pAcceptType,
                                   const char* pAdditionalHeaders,
                                   void*       pRequestData);
            
        protected:

            virtual ~TcpConnection();

            virtual PAL_Error AddDataToSendQ(const byte* pBuffer, int count);


            PAL_Error CreateSocketConnection(void);
            void      DestroySocketConnection(void);

            virtual int CreateSocket(void);
            virtual PAL_Error BindSocket(void);
            virtual PAL_Error ConnectSocket(void);
            virtual void DestroySocket(void);

            virtual PAL_Error SetSocketNonBlocking(void);
            virtual nb_boolean IsSocketReadable(void);
            virtual nb_boolean IsSocketWritable(void);
            virtual PAL_NetConnectionStatus ReadSocket(void);
            virtual PAL_NetConnectionStatus WriteSocket(void);

            static void* WorkerThreadFunc(void* user_data);
            void WakeWorkerThread(void);
            void SetWorkerThreadDone(void);
            PAL_Error ResolveHostName(void);
            int GetSocket(void) { return m_socket; };
            PAL_Error TranslateSocketError(int socketError);

        private:
            //! disable copy
            TcpConnection(const TcpConnection& old);

            //! disable assignment
            TcpConnection& operator= (const TcpConnection& old);

            struct bq       m_sendQ;
            pthread_mutex_t m_sendQMutex;

            pthread_t       m_workerThread;
            nb_boolean      m_workerThreadStop;
            nb_boolean      m_workerThreadRunning;
            int             m_pipe[2];

        protected:
            int             m_socket;
            struct sockaddr_in m_socketAddr;
        };
    }
}

#endif
/*! @} */
