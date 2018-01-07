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

    @file     tlsconnection.cpp
    @date     2010/06/10
    @defgroup PAL_NET PAL Network I/O Functions

    Windows Mobile implementation of a TLS-secured socket connection
*/
/*
    (C) Copyright 2010 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunications Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunications Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#include "tlsconnection.h"
#include "palwindow.h"
#include <sslsock.h>

namespace nimpal
{
    namespace network
    {
        const char COMMON_NAME_PARAM[] = "CN=";
        const char SEPARATORS[] = ";, ";

        /*! Thread starter structure.

	        This structure is used to pass parameters into thread starter function.
        */
        struct tclosure
        {
            /*! Pointer to TLSConnection class. */
            TLSConnection *pthis;

            /*! Pointer to thread strater function. */
            ThreadStarterFunction threadStarterFunction;
        };

        //////////////////////////////////////////////////////////////////////////
        // constructor/destructor
        //////////////////////////////////////////////////////////////////////////

        TLSConnection::TLSConnection(PAL_NetConnectionProtocol protocol) :
            WSAConnection(protocol)
        {
            InitializeCriticalSection(&m_tlsLock);
            InitializeSecureChannel();
        }

        TLSConnection::~TLSConnection()
        {
            DeleteCriticalSection(&m_tlsLock);
            DestroySecureChannel();
        }

        //////////////////////////////////////////////////////////////////////////
        // protected functions
        //////////////////////////////////////////////////////////////////////////

        /*! Create socket. 

            @return created socket.
        */
        SOCKET TLSConnection::WCSocketCreate()
        {
            SOCKET sock;

            sock = socket(m_Family, SOCK_STREAM, IPPROTO_TCP);
            if (sock != INVALID_SOCKET && !SetupSecureSocket(sock))
            {
                closesocket(sock);
                return (INVALID_SOCKET);
            }
            return (sock);
        }

        /*! Handle socket event.

            @return true if event handled, false otherwise.
        */
        boolean TLSConnection::WCOnEvent(WSAEVENT ev)
        {
            if (ev == m_dataEvent)
            {
                OutputFull();
                return (TRUE);
            }
            return (FALSE);
        }

        /*! Socket connected callback.
        */
        void TLSConnection::OnConnect()
        {
            if (StartThread(&TLSConnection::ReaderThread) != NULL)
            {
                WSAConnection::OnConnect();
            }
            else
            {
                SetStatus(PNCS_Failed);
            }
        }

        //////////////////////////////////////////////////////////////////////////
        // private functions
        //////////////////////////////////////////////////////////////////////////

        /*! Initialize secure channel library. 

            A note about our use of schannel.dll: This appears to be the closest thing to a built-in way to
            parse X509 on Windows Mobile. If this is not available, we won't have access to any X509 details
            and server certificate validation will fail.
        */
        void TLSConnection::InitializeSecureChannel()
        {
            m_secureChannel = LoadLibrary(TEXT("schannel.dll"));
            if (m_secureChannel != NULL)
            {
                fn_sslCrackCertificate = (SSL_CRACK_CERTIFICATE_FN)GetProcAddress(m_secureChannel, SSL_CRACK_CERTIFICATE_NAME);
                fn_sslFreeCertificate = (SSL_FREE_CERTIFICATE_FN)GetProcAddress(m_secureChannel, SSL_FREE_CERTIFICATE_NAME);
                if (fn_sslCrackCertificate == NULL || fn_sslFreeCertificate == NULL)
                {
                    DestroySecureChannel();
                }
            }
        }

        /*! Unload secure channel library. 
        */
        void TLSConnection::DestroySecureChannel()
        {
            if (m_secureChannel != NULL)
            {
                FreeLibrary(m_secureChannel);
                m_secureChannel = NULL;
            }
            fn_sslCrackCertificate = NULL;
            fn_sslFreeCertificate = NULL;
        }

        /*! Check if secure channel library initialized.

            @return true if initialized, false otherwise.
        */
        boolean TLSConnection::HaveSecureChannel()
        {
            return (m_secureChannel != NULL);
        }

        /*! Setup socket socket.

            @return 0 if setup is OK, 1 in case of error.
        */
        int TLSConnection::SetupSecureSocket(SOCKET sock)
        {
            DWORD dw = SO_SEC_SSL;
            if (setsockopt(sock, SOL_SOCKET, SO_SECURE, (const char *)&dw, sizeof(dw)) != 0)
            {
                return (0);
            }

            SSLVALIDATECERTHOOK sslValidate = { 0 };
            sslValidate.HookFunc = &SslValidateStatic;
            sslValidate.pvArg = this;
            if (WSAIoctl(sock, SO_SSL_SET_VALIDATE_CERT_HOOK, &sslValidate, sizeof(sslValidate), NULL, 0, NULL, NULL, NULL) != 0)
            {
                return (0);
            }
            /*
             * NB: There is a SO_SSL_SET_PEERNAME control code, but using it makes it impossible for SslValidate
             * to distinguish between a trust error and a hostname error.
             */

            return (1);
        }

        /*! Check if data block is ready to read from the socket. 

            Determine whether there's data available to be read on the secure socket.
            Routinely blocking on recv is unacceptable because then another thread can't
            call send. This method is supposed to block until recv will succeed without
            blocking. Returns 0 if no data is available; 1 if recv can be expected to
            succeed; and -1 on error.
            
            :KLUDGE: This select-based implementation is far from perfect. It's probably
            possible that it will indicate success prematurely if it's still sensing the
            underlying socket state and not the decrypted data which is ready. Winsock
            doesn't provide a higher-level API to ask for that information, so we're left
            with this. Fortunately, it's not a disaster if it fires too soon; even if recv
            blocks, as long as some data is in the pipeline, it won't wait forever.
            
            NB: The case which really has to be avoided is where some unsent data is
            required by the server before it sends more data and we block on recv before
            getting that data out. In this case, nothing can be done and the socket has to
            be abandoned. Other than the beginning of the connection where this is taken
            care of, our query/reply protocol shouldn't run into this issue. This test
            provides some protection in addition to the use of OutputFull.

           @return number of sockets ready to read.
        */
        int TLSConnection::DataBlockReady(void)
        {
            fd_set sockets[1];

            sockets[0].fd_count = 1;
            sockets[0].fd_array[0] = m_Sock;
            return (select(0, sockets, NULL, NULL, NULL));
        }

        /*! Reader thread entry point. 
        */
        void TLSConnection::ReaderThread(void)
        {
            char buf[READ_BUFFER_SIZE] = {0};
            int result = 0;

            while (GetStatus() == PNCS_Connected)
            {
                // check if secure socket has data to read
                result = DataBlockReady();
                if (result == 0)    // Timeout or other interruption; no data available
                {
                    continue;
                }
                else if (result == -1)
                {
wserr:              int err = WSAGetLastError();
                    WCOnError(err);
                    break;
                }
                else if (GetStatus() != PNCS_Connected || m_Sock == INVALID_SOCKET)
                {
                    break;
                }

                EnterCriticalSection(&m_tlsLock);
                result = recv(m_Sock, buf, sizeof(buf), 0);
                LeaveCriticalSection(&m_tlsLock);
                if (result == SOCKET_ERROR)
                {
                    goto wserr;
                }
                else if (result > 0)
                {
                    NotifyDataReceived((byte *)buf, result);
                }
            }
        }

        /*! Send all data blocks. 
        */
        void TLSConnection::OutputFull(void)
        {
            int result = 1;

            /**
             * By entering the critical section here, we prevent recv attempts while we
             * have a SendQ. Winsock Secure sockets don't support overlapped I/O (i.e.
             * waiting on recv in one thread while calling send in another), so we want to
             * avoid a situation where we accidently wait for data from the server while
             * [part of] a critical message is still in the queue. Waiting for this
             * blocking operation may slow down an unrelated read operation, but it
             * ensures proper functionality in all cases.
             */
            EnterCriticalSection(&m_tlsLock);
            while (result == 1 && GetStatus() == PNCS_Connected && m_Sock != INVALID_SOCKET)
            {
                result = OutputBlock();
            }
            LeaveCriticalSection(&m_tlsLock);
        }

        /*! Perform a blocking send operation. 

            @return -1 if an error occurred (connection state may have changed)
            @return  0 if the entire output buffer has been sent
            @return  1 if there's more to be sent
        */
        int TLSConnection::OutputBlock(void)
        {
            size_t bytesToSend = 0;
            int result = 0;

            Lock();
            bytesToSend = m_SendQ.w - m_SendQ.r;
            if (bytesToSend > 0)
            {
                EnterCriticalSection(&m_tlsLock);
                int bytesSent = send(m_Sock, m_SendQ.r, bytesToSend, 0);
                LeaveCriticalSection(&m_tlsLock);
                if (bytesSent > 0)
                {
                    NotifyDataSent(0, bytesSent);
                    m_SendQ.r += bytesSent;
                    bqshrink(&m_SendQ);
                    bytesToSend -= bytesSent;
                }
                else if (bytesSent == SOCKET_ERROR)
                {
                    int err = WSAGetLastError();
                    WCOnError(err);
                    result = -1;
                    goto out;
                }
            }
            result = bytesToSend > 0;
out:        Unlock();
            return (result);
        }

        /*! Start thread. 

            @return thread handler
        */
        HANDLE TLSConnection::StartThread(ThreadStarterFunction threadStarterFunction)
        {
            struct tclosure *threadParams;
            HANDLE threadHandle;

            threadParams = new struct tclosure;
            threadParams->pthis = this;
            threadParams->threadStarterFunction = threadStarterFunction;
            AddRef();
            threadHandle = CreateThread(NULL, 0,
                                       (LPTHREAD_START_ROUTINE)&TLSConnection::ThreadStarterProc,
                                       threadParams, 0, NULL);
            if (threadHandle == NULL)
            {
                delete threadParams;
                Release();
            }
            return (threadHandle);
        }

        /*! Thread entry point. 

            @return thread exit code
        */
        DWORD TLSConnection::ThreadStarterProc(LPVOID pParameter)
        {
            struct tclosure *threadParams = static_cast<struct tclosure *>(pParameter);
            TLSConnection *pThis = threadParams->pthis;

            (pThis->*(threadParams->threadStarterFunction))();
            pThis->Release();
            delete threadParams;
            return (DWORD)1;
        }

        //////////////////////////////////////////////////////////////////////////
        // TLS server certificate validation methods
        //////////////////////////////////////////////////////////////////////////

        /*! Get TLS trust override flag value .

            @return trust override flag value
        */
        int TLSConnection::tlsOverride(int flags)
        {
            return (m_tlsConfig.override & flags);
        }

        /*! Win API SSL response callback. 

            @return SSL error code
        */
        int TLSConnection::SslValidateStatic(DWORD dwType, LPVOID pvArg, DWORD dwChainLen, LPBLOB pCertChain, DWORD dwFlags)
        {
            TLSConnection* tc = static_cast<TLSConnection *>(pvArg);

            return (tc->SslValidate(dwType, dwChainLen, pCertChain, dwFlags));
        }

        /*! SSL response callback. 

            @return SSL error code
        */
        int TLSConnection::SslValidate(DWORD dwType, DWORD dwChainLen, LPBLOB pCertChain, DWORD dwFlags)
        {

            if (dwType != SSL_CERT_X509 || dwChainLen != 1)
            {
                return (SSL_ERR_BAD_DATA);
            }
            if (!tlsOverride(PAL_TLS_OVERRIDE_TRUST_ROOT) && (dwFlags & SSL_CERT_FLAG_ISSUER_UNKNOWN) != 0)
            {
                return (SSL_ERR_CERT_UNKNOWN);
            }
            return (SslCrackValidate(pCertChain));
        }

        /*! Validate SSL response.

            @return SSL error code
        */
        int TLSConnection::SslCrackValidate(LPBLOB pCertChain)
        {
            X509Certificate* certificate = NULL;
            int result = 0;

            if (!HaveSecureChannel())
            {
                return (SSL_ERR_CERT_UNKNOWN);
            }
            if (!(*fn_sslCrackCertificate)(pCertChain->pBlobData, pCertChain->cbSize, TRUE, &certificate))
            {
                return (SSL_ERR_BAD_DATA);
            }
            result = SslValidateCertificate(certificate);
            (*fn_sslFreeCertificate)(certificate);
            return (result);
        }

        /*! Validate SSL certificate. 

            @return SSL certificate error code
        */
        int TLSConnection::SslValidateCertificate(X509Certificate* certificate)
        {
            if (!tlsOverride(PAL_TLS_OVERRIDE_EXPIRY) && !ValidateExpiry(certificate))
            {
                return (SSL_ERR_CERT_EXPIRED);
            }
            if (!tlsOverride(PAL_TLS_OVERRIDE_HOST) && !ValidateSubject(certificate))
            {
                return (SSL_ERR_CERT_UNKNOWN);
            }
            return (SSL_ERR_OKAY);
        }

        /*! Validate whether certificate expired. 

            @return false if certificate expired, true otherwise
        */
        boolean TLSConnection::ValidateExpiry(X509Certificate* certificate)
        {
            FILETIME now = {0};

            GetCurrentFT(&now);
            return (CompareFileTime(&certificate->ValidFrom, &now) < 0 && CompareFileTime(&now, &certificate->ValidUntil) < 0);
        }

        /*! Validate certificate's subject. 

            @return true if subject is valid, false otherwise
        */
        boolean TLSConnection::ValidateSubject(X509Certificate *certificate)
        {
            const char *commonName = NULL;
            size_t commonNameLength = 0;

            commonName = getCN(certificate->pszSubject, &commonNameLength);
            return (commonName != NULL && commonNameLength == strlen(m_tlsConfig.name) &&
                    strncmp(commonName, m_tlsConfig.name, commonNameLength) == 0);
        }

        /*! Get common name from certificate.

           Parse the CN out of a certificate subject field. This function expects the string to be in the loosely-defined
           format as currently returned by schannel.dll. The existing algorithm is non-strict and will work with several variations

           @return a pointer to the name and stores its length in lenp or NULL if it can't be found
        */
        const char *TLSConnection::getCN(const char *subject, size_t *lenp)
        {
            char *subStr = NULL;
            int commonNameParamLength = strlen(COMMON_NAME_PARAM);

            subStr = strstr(subject, COMMON_NAME_PARAM);
            if (subStr == NULL)
            {
                return NULL;
            }
            if (subStr != subject && strchr(SEPARATORS, subStr[-1]) == '\0') // Ensure we didn't find the middle of another value
            {
                return (getCN(subStr + commonNameParamLength, lenp));
            }
            subStr += commonNameParamLength;
            *lenp = strcspn(subStr, SEPARATORS);
            return (subStr);
        }
    }
}
