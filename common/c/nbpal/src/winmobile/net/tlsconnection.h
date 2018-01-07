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

    @file     tlsconnection.h
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

#ifndef TLSCONNECTION_H
#define TLSCONNECTION_H

#ifdef WINCE

#include "wsaconnection.h"

#include <wincrypt.h>
#include <schnlsp.h>

namespace nimpal
{
    namespace network
    {
        class TLSConnection;

        /*! Thread starter function prototype */
        typedef void (TLSConnection::*ThreadStarterFunction)(void);

        class TLSConnection : public WSAConnection
        {
        public:
            TLSConnection(PAL_NetConnectionProtocol protocol);
            virtual ~TLSConnection();

        protected:
            /* See source file for description. */
            virtual SOCKET WCSocketCreate();
            /* See source file for description. */
            virtual boolean WCOnEvent(WSAEVENT);

            /* See source file for description. */
            virtual void OnConnect();

        private:
            /* See source file for description. */
            int SetupSecureSocket(SOCKET);

            /* See source file for description. */
            int DataBlockReady(void);
            /* See source file for description. */
            void ReaderThread(void);
            /* See source file for description. */
            void OutputFull(void);
            /* See source file for description. */
            int OutputBlock(void);

            /* See source file for description. */
            HANDLE StartThread(ThreadStarterFunction);
            /* See source file for description. */
            static DWORD ThreadStarterProc(LPVOID);

            /* See source file for description. */
            int tlsOverride(int);
            /* See source file for description. */
            static int SslValidateStatic(DWORD dwType, LPVOID pvArg, DWORD dwChainLen, LPBLOB pCertChain, DWORD dwFlags);
            /* See source file for description. */
            int SslValidate(DWORD dwType, DWORD dwChainLen, LPBLOB pCertChain, DWORD dwFlags);
            /* See source file for description. */
            int SslCrackValidate(LPBLOB);
            /* See source file for description. */
            int SslValidateCertificate(X509Certificate *);
            /* See source file for description. */
            static boolean ValidateExpiry(X509Certificate *);
            /* See source file for description. */
            boolean ValidateSubject(X509Certificate *);
            /* See source file for description. */
            static const char *getCN(const char *, size_t *);

            /**
             * Winsock secure sockets support neither nonblocking I/O nor overlapped I/O
             * (i.e. blocking on recv while calling send in another thread), so we're
             * limited to a non-parallel architecture. Additionally, there's no documented
             * way to ask a secure socket whether there's data to be received. This lock
             * is used to manually prevent contention between those operations so we get
             * consistent (non-racey) behavior from the system.
             */
            CRITICAL_SECTION    m_tlsLock;

            void InitializeSecureChannel();
            void DestroySecureChannel();
            boolean HaveSecureChannel();
            HMODULE     m_secureChannel;
            SSL_CRACK_CERTIFICATE_FN   fn_sslCrackCertificate;
            SSL_FREE_CERTIFICATE_FN    fn_sslFreeCertificate;
        };
    }
}

#endif

#endif
