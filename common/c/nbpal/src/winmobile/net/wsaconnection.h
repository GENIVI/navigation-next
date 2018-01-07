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

    @file     wsaconnection.h
    @date     2010/06/10
    @defgroup PAL_NET PAL Network I/O Functions

    Intermediate support class for protocols based on Windows Mobile
    WSA sockets
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.
    (C) Copyright 2010 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunications Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunications Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef WSACONNECTION_H
#define WSACONNECTION_H

#include "netconnection.h"
#include "winsock2.h"


namespace nimpal
{
    namespace network
    {
        /**
         * Windows Socket connection class
         *
         * This class provides a general infrastructure for WSA-based
         * connections. It is an abstract class that requires a
         * protocol-specific implementation.
         */
        class WSAConnection : public NetConnection
        {
        public:
            static boolean InitializeWindowsSockets();
            static void CleanupWindowsSockets();

            virtual boolean Connect(const char* pHostName, uint16 port);
            virtual boolean Send(const byte* pBuffer, int count);
            virtual boolean Close();

        private:
            static DWORD _stdcall WorkerThreadProc(LPVOID pParameter);


        public:
            WSAConnection(PAL_Instance* pal, PAL_NetConnectionProtocol protocol);
            virtual ~WSAConnection();

        protected:
            /**
             * The interface for protocol implementations comprises these methods
             */

            virtual SOCKET WCSocketCreate() = 0;
            virtual HANDLE WCSendEvent();

            virtual void WCWorkerEvents(WSAEVENT *, int *);
            virtual boolean WCOnEvent(WSAEVENT) = 0;

            static const size_t READ_BUFFER_SIZE = 4096;

        protected:
            static boolean  m_WinsockInitialized;
            PAL_Error TranslateWinsockError(int wsaError);
            void WCOnError(int wsaError);

            virtual void OnConnect();
            virtual void OnError(int error);

            void WorkerThread(void);
            int SocketCreate();
            void SocketDestroy();

            SOCKET           m_Sock;
            int              m_Family;

            struct bq        m_SendQ;

            HANDLE           m_dataEvent;
            HANDLE           m_exitEvent;
            HANDLE           m_networkErrorEvent;
        };
    }
}

#endif
