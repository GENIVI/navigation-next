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

    @file     httpconnection.h
    @date     3/18/2009
    @defgroup PAL_NET PAL Network I/O Functions

    @brief    Class for PAL HTTP network connections.

    Linux implementation for PAL HTTP network functions.
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

/*! @{ */

/* all the file's code goes here */
#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H
#include <string>
#include <deque>
#include <pthread.h>
#include <netinet/in.h>
#include "netconnection.h"
#include "httprequest.h"
#include "httpresponse.h"

using namespace std;

namespace nimpal
{
    namespace network
    {
        const unsigned int HTTP_READ_BUFFER_SIZE = 4096;

        typedef enum
               {
                  HTTP_Ok,
                  HTTP_ErrReadAgain,
                  HTTP_ErrWriteAgain,
                  HTTP_ErrReadFailure,
                  HTTP_ErrWriteFailure,
               }HTTP_Error;

        class HttpConnection : public NetConnection
        {
        public:
            HttpConnection(PAL_NetConnectionProtocol protocol, PAL_Instance* pal);

            /* See description in netconnection.h */
            virtual void Destroy();
            virtual PAL_Error Connect(const char* pHostName, uint16 port, struct sockaddr* localIpAddr=NULL);
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

            /*! Destructor of HTTP connection

                This function is private. User should call function Destroy to destroy this object.
            */
            virtual ~HttpConnection();

        private:
        
            PAL_Error  ResolveHostName(void);
            PAL_Error  EnsureSocketConnection(void);
            PAL_Error  EstablishSocketConnection(void);
            void       DestroySocketConnection(void);
            void       WaitForNextAction(int* flag);
            PAL_Error  SetFdNonBlocking(int fd);
            HTTP_Error ReadData(int fd, void *buf, unsigned int* count);
            HTTP_Error WriteData(int fd, const void *buf, unsigned int* count);
            void       SendRequest(int* nextActionFlag);
            void       ReceiveResponse(int* nextActionFlag);
            void       FinishRequest(int* nextActionFlag);
            void       NotifyErrorForAllRequests(PAL_Error error);
            PAL_Error  AddDataToSendQ(HttpRequest* pRequest);
            void       WakeWorkerThread();

            static  void*      WorkerThreadFunc(void* user_data);

            HttpRequest*       GetNextRequest(void);

            /*! Callback function for worker thread to exit

                This function is called in CCC thread when worker thread exits.

                @return None
            */
            static void
            WorkerThreadExitCallback(PAL_Instance* pal,             /*!< A PAL instance */
                                     HttpConnection* connection     /*!< A HTTP connection object */
                                     );

            std::deque<HttpRequest*> m_requestQueue;
            pthread_mutex_t    m_requestQueueMutex;

            PAL_Instance*      m_pal;                       /*!< A PAL instance */
            HttpRequest*       m_currentRequest;
            HttpResponse*      m_currentResponse;
            bool               m_statusNotified;

            pthread_t          m_workerThread;

            bool               m_workerThreadStop;          /*!< CCC thread set this flag to ask HTTP thread to stop */
            bool               m_willBeDestroyed;           /*!< Flag to specified this object will be destroyed */
            int                m_pipe[2];
            int                m_socket;
            int                m_socketFamily;
            int                m_socketType;
            int                m_socketProtocol;
            struct sockaddr_storage m_socketAddr;
            byte               m_readBuffer[HTTP_READ_BUFFER_SIZE];
        };
    }
}

#endif

/*! @} */
