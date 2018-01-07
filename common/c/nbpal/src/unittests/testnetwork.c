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

    @file     testnetwork.c
    @date     01/01/09
    @defgroup NBIPAL_UNIT_TESTS Unit Tests for NBI PAL

    Unit tests for PAL network functions.

    This file contains all unit tests for the PAL networking component.
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

#include "testnetwork.h"
#include "main.h"
#include "PlatformUtil.h"
#include "palstdlib.h"

#include "palnet.h"

/* RECEIVE_BUFFER_SIZE should be set to large enough for the largest response */
#define RECEIVE_BUFFER_SIZE 20480

#define HTTPS_PORT 8443
#define HTTP_PORT 80

#define TEST_DEV12_HOSTNAME "https://dev12dts.navbuilder.nimlbs.net"
#define TEST_QA1_HOSTNAME   "qa1dts.navbuilder.nimlbs.net"

#define COMMON_TEST_REQUEST1 "/tiles/cmd/mobileTile?g=406&base=r&a=0230132021020&size=256&mkt=en-US&dpi=96&device=mobile&ids=12345678"
#define COMMON_TEST_REQUEST2 "/tiles/cmd/mobileTile?g=406&base=r&a=0230132021020&size=128&mkt=en-US&dpi=96&device=mobile&ids=12345678"

#define TEST_NETWORK_FILE_40k              0x00000001
#define TEST_NETWORK_FILE_16mb_forward     0x00000002
#define TEST_NETWORK_FILE_16mb_reverse     0x00000004

#define UNIT_TEST_VERBOSE_HIGH

typedef struct NetworkCallbackUserData
{
    void* connectedEvent;
    void* dataReceivedEvent;
    void* httpResponseStatusEvent;
    void* errorEvent;
    void* failedEvent;
    void* errorOrFailedEvent;
    void* closedEvent;
    char receiveBuffer[RECEIVE_BUFFER_SIZE];
    unsigned int receiveCount; /* where to store data in receive buffer */
} NetworkCallbackUserData;

NetworkCallbackUserData* NetworkCallbackUserDataCreate(void)
{
    NetworkCallbackUserData* pUserData
        = nsl_malloc(sizeof(NetworkCallbackUserData));

    if (pUserData)
    {
        pUserData->connectedEvent = CreateCallbackCompletedEvent();
        pUserData->dataReceivedEvent = CreateCallbackCompletedEvent();
        pUserData->httpResponseStatusEvent = CreateCallbackCompletedEvent();
        pUserData->errorOrFailedEvent = CreateCallbackCompletedEvent();
        pUserData->errorEvent = CreateCallbackCompletedEvent();
        pUserData->failedEvent = CreateCallbackCompletedEvent();
        pUserData->closedEvent = CreateCallbackCompletedEvent();
        pUserData->receiveCount = 0;
        nsl_memset(pUserData->receiveBuffer, 0, RECEIVE_BUFFER_SIZE);
    }

    return pUserData;
}

void NetworkCallbackUserDataDestroy(NetworkCallbackUserData* pUserData)
{
    if (pUserData)
    {
        DestroyCallbackCompletedEvent(pUserData->connectedEvent);
        DestroyCallbackCompletedEvent(pUserData->dataReceivedEvent );
        DestroyCallbackCompletedEvent(pUserData->httpResponseStatusEvent);
        DestroyCallbackCompletedEvent(pUserData->errorOrFailedEvent);
        DestroyCallbackCompletedEvent(pUserData->errorEvent);
        DestroyCallbackCompletedEvent(pUserData->failedEvent);
        DestroyCallbackCompletedEvent(pUserData->closedEvent);

        nsl_free(pUserData);
    }
}

void NetworkCallbackUserDataReinit(NetworkCallbackUserData* pUserData)
{
    if (pUserData)
    {
        nsl_memset(pUserData->receiveBuffer, 0, RECEIVE_BUFFER_SIZE);
        pUserData->receiveCount = 0;
    }
}

// Callback Functions ........................................................

void TestNetwork_ConnectionStatusCallback(void* userData, PAL_NetConnectionStatus status)
{
    NetworkCallbackUserData* callbackUserData = (NetworkCallbackUserData *)userData;
    char statusString[40];

    switch (status)
    {
        case PNCS_Closed:       nsl_strcpy(statusString, "PNCS_Closed");        break;
        case PNCS_Connected:    nsl_strcpy(statusString, "PNCS_Connected");     break;
        case PNCS_Connecting:   nsl_strcpy(statusString, "PNCS_Connecting");    break;
        case PNCS_Created:      nsl_strcpy(statusString, "PNCS_Created");       break;
        case PNCS_Error:        nsl_strcpy(statusString, "PNCS_Error");         break;
        case PNCS_Failed:       nsl_strcpy(statusString, "PNCS_Failed");        break;
        case PNCS_Resolving:    nsl_strcpy(statusString, "PNCS_Resolving");     break;
        case PNCS_Undefined:    nsl_strcpy(statusString, "PNCS_Undefined");     break;
        default:                nsl_sprintf(statusString, "(%d)", status);      break;
    }
    printf("%s\n", statusString);

    switch (status)
    {
        case PNCS_Connected:
            SetCallbackCompletedEvent(callbackUserData->connectedEvent);
            break;

        case PNCS_Error:
            SetCallbackCompletedEvent(callbackUserData->errorEvent);
            SetCallbackCompletedEvent(callbackUserData->errorOrFailedEvent);
            break;

        case PNCS_Failed:
            SetCallbackCompletedEvent(callbackUserData->failedEvent);
            SetCallbackCompletedEvent(callbackUserData->errorOrFailedEvent);
            break;

        case PNCS_Closed:
            SetCallbackCompletedEvent(callbackUserData->closedEvent);
            break;

        default:
            break;
    }

}

void TestNetwork_NetDataSentCallback(void* userData, const byte* bytes, uint32 count)
{
#ifdef UNIT_TEST_VERBOSE_HIGH
    char debugMsg[80];
    SPRINTF(debugMsg, "PAL %d bytes sent\n", count);
    PRINTF(debugMsg);
#endif
}

void TestNetwork_NetDataReceivedCallback(void* userData, const byte* bytes, uint32 count)
{
    NetworkCallbackUserData* callbackUserData = (NetworkCallbackUserData *)userData;
    unsigned int spaceAvailable = (RECEIVE_BUFFER_SIZE - 1 - callbackUserData->receiveCount);
    unsigned int bytesToCopy = (count < spaceAvailable) ? count : spaceAvailable;
#ifdef UNIT_TEST_VERBOSE_HIGH
    char debugMsg[80];
    SPRINTF(debugMsg, "PAL %d bytes received\n", count);
    PRINTF(debugMsg);
#endif

    if (bytesToCopy)
    {
        nsl_memcpy(callbackUserData->receiveBuffer + callbackUserData->receiveCount,
                bytes, bytesToCopy);
        callbackUserData->receiveCount += bytesToCopy;
        callbackUserData->receiveBuffer[callbackUserData->receiveCount] = '\0';
    }

    SetCallbackCompletedEvent(callbackUserData->dataReceivedEvent);
}


void TestNetwork_HttpDataReceivedCallback(void* userData, void* requestData, PAL_Error errorCode, const byte* bytes, uint32 count)
{
    NetworkCallbackUserData* callbackUserData = (NetworkCallbackUserData *)userData;
    unsigned int spaceAvailable = (RECEIVE_BUFFER_SIZE - 1 - callbackUserData->receiveCount);
    unsigned int bytesToCopy = (count < spaceAvailable) ? count : spaceAvailable;
#ifdef UNIT_TEST_VERBOSE_HIGH
    char debugMsg[80];
    SPRINTF(debugMsg, "PAL %d bytes received\n", count);
    PRINTF(debugMsg);
#endif

    if (bytesToCopy)
    {
        nsl_memcpy(callbackUserData->receiveBuffer + callbackUserData->receiveCount,
                bytes, bytesToCopy);
        callbackUserData->receiveCount += bytesToCopy;
        callbackUserData->receiveBuffer[callbackUserData->receiveCount] = '\0';
    }

    SetCallbackCompletedEvent(callbackUserData->dataReceivedEvent);
}



void TestNetwork_HttpStatusCallback(PAL_Error errorCode, void* userData, void* requestData, uint32 contentLength)
{
    NetworkCallbackUserData* callbackUserData = (NetworkCallbackUserData *)userData;
#ifdef UNIT_TEST_VERBOSE_HIGH
    char debugMsg[80];
    SPRINTF(debugMsg, "PAL HTTP response status code %d \n", errorCode);
    PRINTF(debugMsg);
#endif
    SetCallbackCompletedEvent(callbackUserData->httpResponseStatusEvent);
}

void TestNetwork_DnsResultCallback(void* userData, PAL_Error errorCode, const char* hostName, uint32 const* const* addresses, uint32 addressCount)
{
    NetworkCallbackUserData* callbackUserData = (NetworkCallbackUserData *)userData;
#ifdef UNIT_TEST_VERBOSE_HIGH
    char debugMsg[80];
    uint32 n  = 0;
    SPRINTF(debugMsg, "\nPAL DNS Status %s (PAL error code: %X addrs returned: %d)\n", hostName, errorCode, addressCount);
    PRINTF(debugMsg);
    for (n = 0; n < addressCount; n++)
    {
        SPRINTF(debugMsg, "%d: %04X\n", n, addresses[n]);
        PRINTF(debugMsg);
    }
#endif
    SetCallbackCompletedEvent(callbackUserData->connectedEvent);
}

void TestNetwork_HttpMultiReqStatusCallback(PAL_Error errorCode, void* userData, void* requestData, uint32 contentLength)
{
    NetworkCallbackUserData* callbackUserData = (NetworkCallbackUserData *)userData;
    static uint32  fileStatus = 0;
#ifdef UNIT_TEST_VERBOSE_HIGH
    char debugMsg[80];
    SPRINTF(debugMsg, "PAL HTTP response status code %d \n", errorCode);
    PRINTF(debugMsg);
#endif

     CU_ASSERT_NOT_EQUAL(0, requestData);

     if (0 != requestData && errorCode == PAL_Ok)
     {
         fileStatus |= (uint32) requestData;
     }

     /* Verify that we get status for all the files */
    if( fileStatus == (TEST_NETWORK_FILE_16mb_forward
                                    | TEST_NETWORK_FILE_16mb_reverse
                                      | TEST_NETWORK_FILE_40k) )
     {
        // Received status for all the files
         SetCallbackCompletedEvent(callbackUserData->httpResponseStatusEvent);
     }

}

void TestNetwork_HttpMultiReqDataCallback(void* userData, void* requestData, PAL_Error errorCode, const byte* bytes, uint32 count)
{
    NetworkCallbackUserData* callbackUserData = (NetworkCallbackUserData *)userData;
        static uint32  filesDownloaded = 0;
        uint32         fileRequestId = 0;

        CU_ASSERT_EQUAL(PAL_Ok, errorCode);
        CU_ASSERT_NOT_EQUAL(NULL, requestData);

        if (0 != requestData)
        {
           fileRequestId = (uint32) requestData;

          /* ASSERT to see if file is already downloaded to completion.
           * It shouldn't happen.
           */
           CU_ASSERT_EQUAL( 0, (fileRequestId & filesDownloaded));


           switch(fileRequestId)
           {
              case TEST_NETWORK_FILE_40k:
                   {
                    /* For 40k file, just verify that 40x1024 bytes of data is received. */
                     static uint32 bytesReceived = 0;
                     bytesReceived += count;

                     if (0 == count)
                     {
                       /* No more data for this file. Complete file received ( as per Network )*/
                        CU_ASSERT_EQUAL( 40*1024, bytesReceived);
                        filesDownloaded |= TEST_NETWORK_FILE_40k;

#ifdef UNIT_TEST_VERBOSE_HIGH
                       {
                         char debugMsg[80];
                         SPRINTF(debugMsg, "\"40k\" file downloaded. Bytes Received: %d \n", bytesReceived);
                         PRINTF(debugMsg);
                       }
#endif
                     }
                   }
                   break;

              case TEST_NETWORK_FILE_16mb_forward:
                  {
                    /*
                       "16mb_forward" data pattern:
                        0x00 0x01 0x02 ...... 0xfd 0xfe 0xff   ( 256 bytes )
                        This 256 bytes pattern repeats until the end of file.
                        The file size equals 256*256*256 bytes ( 16 mb )
                    */
                     static uint32 bytesReceived = 0;
                     static uint8  nextByte = 0;
                     static uint8  dirtyData = FALSE;

                     bytesReceived += count;

                     if (0 == count)
                     {
                       /* No more data for this file. Complete file received ( as per Network )*/
                        CU_ASSERT_EQUAL( 256*256*256, bytesReceived);
                        CU_ASSERT_EQUAL( FALSE, dirtyData);
                        filesDownloaded |= TEST_NETWORK_FILE_16mb_forward;

#ifdef UNIT_TEST_VERBOSE_HIGH
                       {
                         char debugMsg[80];
                         SPRINTF(debugMsg, "\"16mb_forward\" file downloaded. Bytes Received: %d \n", bytesReceived);
                         PRINTF(debugMsg);
                       }
#endif

                     }
                     else
                     {
                       if ( FALSE == dirtyData )
                       {
                           uint32 i = 0;
                           for( ; i < count; i++)
                           {
                              if( nextByte != bytes[i])
                              {
                                dirtyData = TRUE;
                              }
                              nextByte++;
                           }
                       }
                     }
                   }
                   break;

              case TEST_NETWORK_FILE_16mb_reverse:
                   {
                    /*
                       "16mb_reverse" data pattern:
                        0xff 0xfe 0xfd ...... 0x02 0x01 0x00   ( 256 bytes )
                        This 256 bytes pattern repeats until the end of file.
                        The file size equals 256*256*256 bytes ( 16 mb )
                    */
                     static uint32 bytesReceived = 0;
                     static uint8  nextByte = 0xff;
                     static uint8  dirtyData = FALSE;

                     bytesReceived += count;

                     if ( 0 == count)
                     {
                       /* No more data for this file. Complete file received ( as per Network )*/
                        CU_ASSERT_EQUAL( 256*256*256, bytesReceived);
                        CU_ASSERT_EQUAL( FALSE, dirtyData);
                        filesDownloaded |= TEST_NETWORK_FILE_16mb_reverse;
#ifdef UNIT_TEST_VERBOSE_HIGH
                       {
                         char debugMsg[80];
                         SPRINTF(debugMsg, "\"16mb_reverse\" file downloaded. Bytes Received: %d \n", bytesReceived);
                         PRINTF(debugMsg);
                       }
#endif

                     }
                     else
                     {
                       if ( FALSE == dirtyData )
                       {
                           uint32 i = 0;
                           for( ; i < count; i++)
                           {
                              if( nextByte != bytes[i])
                              {
                                dirtyData = TRUE;
                              }
                              nextByte--;
                           }
                       }
                     }
                   }
                break;

              default:
                /* Should not be here for any reason */
                CU_ASSERT_FALSE(FALSE);
                break;
           }

         /* Verify, if we got all the files. If so, signal the event. */
            if( filesDownloaded == (TEST_NETWORK_FILE_16mb_forward
                                    | TEST_NETWORK_FILE_16mb_reverse
                                      | TEST_NETWORK_FILE_40k) )
            {
              // Received all the files with correct data
                SetCallbackCompletedEvent(callbackUserData->dataReceivedEvent);
            }

    }
}

void DoConnectRequest(PAL_NetConnectionProtocol protocol)
{
    PAL_Instance *pal = PAL_CreateInstance();

    PAL_NetConnection* pConn = NULL;
    PAL_NetConnectionConfig cfg = {0};
    NetworkCallbackUserData* pUserData = NetworkCallbackUserDataCreate();
    PAL_Error err = PAL_Ok;
    char* hostName    = NULL;
    uint16 port = 0;

    if (protocol == PNCP_HTTPS)
    {
        hostName = TEST_DEV12_HOSTNAME;
        port = HTTPS_PORT;
    }
    else //  PNCP_HTTP
    {
        hostName = TEST_QA1_HOSTNAME;
        port = HTTP_PORT;
    }

    CU_ASSERT(pUserData != NULL);
    PAL_NetInitialize(pal);

    cfg.netStatusCallback = TestNetwork_ConnectionStatusCallback;
    cfg.netDataSentCallback = TestNetwork_NetDataSentCallback;
    cfg.netHttpDataReceivedCallback = TestNetwork_HttpDataReceivedCallback;
    cfg.netHttpResponseStatusCallback = TestNetwork_HttpStatusCallback;
    cfg.userData = pUserData;

    err = PAL_NetCreateConnection(pal, protocol, &pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_NetOpenConnection(pConn, &cfg, hostName, port);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->connectedEvent, 15000));

    err = PAL_NetHttpSend(pConn, NULL, 0,
                          "GET", COMMON_TEST_REQUEST1,
                          NULL,  NULL, NULL);


    CU_ASSERT_EQUAL(err, PAL_Ok);
    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->httpResponseStatusEvent, 15000));

    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->dataReceivedEvent, 15000));

    NetworkCallbackUserDataReinit(pUserData); // reinitialize buffer
    err = PAL_NetHttpSend(pConn, NULL, 0,
                          "GET", COMMON_TEST_REQUEST2,
                          NULL,  NULL, NULL);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->dataReceivedEvent, 15000));
    err = PAL_NetCloseConnection(pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);
    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->closedEvent, 30000));
    err = PAL_NetDestroyConnection(pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    NetworkCallbackUserDataDestroy(pUserData);
}
void ConnectAndClose(PAL_NetConnectionProtocol protocol)
{
    PAL_Error result = PAL_Ok;

    PAL_Instance* pal = PAL_CreateInstance();
    PAL_NetConnection* connection = NULL;
    PAL_NetConnectionConfig config = {0};
    NetworkCallbackUserData* userData = NetworkCallbackUserDataCreate();
    char* hostName = NULL;
    uint16 port = 0;

    CU_ASSERT(userData != NULL);
    if (!userData)
    {
        return;
    }

    if (protocol == PNCP_HTTPS)
    {
        hostName = TEST_DEV12_HOSTNAME;
        port = HTTPS_PORT;
    }
    else //  PNCP_HTTP
    {
        hostName = TEST_QA1_HOSTNAME;
        port = HTTP_PORT;
    }

    PAL_NetInitialize(pal);

    config.netStatusCallback = TestNetwork_ConnectionStatusCallback;
    config.netDataSentCallback = TestNetwork_NetDataSentCallback;
    config.netHttpDataReceivedCallback = TestNetwork_HttpDataReceivedCallback;
    config.netHttpResponseStatusCallback = TestNetwork_HttpStatusCallback;
    config.userData = userData;

    result = PAL_NetCreateConnection(pal, PNCP_HTTPS, &connection);
    CU_ASSERT_EQUAL(result, PAL_Ok);
    if (result != PAL_Ok)
    {
        return;
    }

    result = PAL_NetOpenConnection(connection, &config, hostName, port);
    CU_ASSERT_EQUAL(result, PAL_Ok);

    // Ignore the returned error because we may test this case when the network is disable
    WaitForCallbackCompletedEvent(userData->connectedEvent, 15000);


    result = PAL_NetHttpSend(connection, NULL, 0,
                          "GET", COMMON_TEST_REQUEST1,
                          NULL,  NULL, NULL);
    CU_ASSERT_EQUAL(result, PAL_Ok);

    // Ignore the returned error because we may test this case when the network is disable
    WaitForCallbackCompletedEvent(userData->httpResponseStatusEvent, 50000);

    result = PAL_NetDestroyConnection(connection);
    CU_ASSERT_EQUAL(result, PAL_Ok);

    NetworkCallbackUserDataDestroy(userData);
}
// All tests. Add all tests to the TestNetwork_AddAllTests function

/*! @brief Test simple TCP network connect & request.

@return None. CUnit asserts are called on failure.

@see TestNetwork_Initialize
@see TestNetwork_Cleanup
*/
void
TestNetworkTcpConnectRequest(void)
{
    PAL_Instance *pal = PAL_CreateInstance();
    const char* TEST_HOSTNAME = "www.nimupdate.com";
    const uint16 TEST_PORT = 80;
    const char* TEST_REQUEST1 = "GET www.nimupdate.com\r\n\r\n";

    PAL_NetConnection* pConn = NULL;
    PAL_NetConnectionConfig cfg = {0};
    NetworkCallbackUserData* pUserData = NetworkCallbackUserDataCreate();
    PAL_Error err = PAL_Ok;

    CU_ASSERT(pUserData != NULL);

    PAL_NetInitialize(pal);

    cfg.netStatusCallback = TestNetwork_ConnectionStatusCallback;
    cfg.netDataSentCallback = TestNetwork_NetDataSentCallback;
    cfg.netDataReceivedCallback = TestNetwork_NetDataReceivedCallback;
    cfg.userData = pUserData;

    err = PAL_NetCreateConnection(pal, PNCP_TCP, &pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_NetOpenConnection(pConn, &cfg, TEST_HOSTNAME, TEST_PORT);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->connectedEvent, 15000));

    err = PAL_NetSend(pConn, (byte*)TEST_REQUEST1, strlen(TEST_REQUEST1));
    CU_ASSERT_EQUAL(err, PAL_Ok);

    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->dataReceivedEvent, 30000));

    err = PAL_NetCloseConnection(pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);
    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->closedEvent, 30000));

    err = PAL_NetDestroyConnection(pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    PAL_NetShutdown(pal);
    PAL_Destroy(pal);

    NetworkCallbackUserDataDestroy(pUserData);
}

void
TestNetworkTcpConnectFail(void)
{
    PAL_Instance *pal = PAL_CreateInstance();
    const char* TEST_HOSTNAME = "fail.nimone.com";
    const uint16 TEST_PORT = 80;
    const char* TEST_REQUEST = "GET fail.html\r\n\r\n";

    PAL_NetConnection* pConn = NULL;
    PAL_NetConnectionConfig cfg = {0};
    NetworkCallbackUserData* pUserData = NetworkCallbackUserDataCreate();

    PAL_Error err = PAL_Ok;

    CU_ASSERT(pUserData != NULL);

    PAL_NetInitialize(pal);

    cfg.netStatusCallback = TestNetwork_ConnectionStatusCallback;
    cfg.netDataSentCallback = TestNetwork_NetDataSentCallback;
    cfg.netDataReceivedCallback = TestNetwork_NetDataReceivedCallback;
    cfg.userData = pUserData;

    err = PAL_NetCreateConnection(pal, PNCP_TCP, &pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_NetOpenConnection(pConn, &cfg, TEST_HOSTNAME, TEST_PORT);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_NetSend(pConn, (byte*)TEST_REQUEST, strlen(TEST_REQUEST));
    CU_ASSERT_EQUAL(err, PAL_ErrNetGeneralFailure);

    // No need to wait for event here.

    err = PAL_NetCloseConnection(pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_NetDestroyConnection(pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    PAL_NetShutdown(pal);
    PAL_Destroy(pal);

    NetworkCallbackUserDataDestroy(pUserData);
}

/*! @brief Test DNS result callback with a valid existing host name.

@return None. CUnit asserts are called on failure.

@see TestNetwork_Initialize
@see TestNetwork_Cleanup
*/
void
TestNetworkTcpDnsRequest(void)
{
    PAL_Instance *pal = PAL_CreateInstance();
    const char* TEST_HOSTNAME = "www.google.com";
    const uint16 TEST_PORT = 80;

    PAL_NetConnection* pConn = NULL;
    PAL_NetConnectionConfig cfg = {0};
    NetworkCallbackUserData* pUserData = NetworkCallbackUserDataCreate();
    PAL_Error err = PAL_Ok;

    CU_ASSERT(pUserData != NULL);

    PAL_NetInitialize(pal);

    cfg.netDnsResultCallback = TestNetwork_DnsResultCallback;
    cfg.userData = pUserData;

    err = PAL_NetCreateConnection(pal, PNCP_TCP, &pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_NetOpenConnection(pConn, &cfg, TEST_HOSTNAME, TEST_PORT);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->connectedEvent, 15000));

    err = PAL_NetCloseConnection(pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_NetDestroyConnection(pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    PAL_NetShutdown(pal);
    PAL_Destroy(pal);

    NetworkCallbackUserDataDestroy(pUserData);
}

/*! @brief Test DNS result callback with a valid non-existing host name.

@return None. CUnit asserts are called on failure.

@see TestNetwork_Initialize
@see TestNetwork_Cleanup
*/
void
TestNetworkTcpDnsRequestFail(void)
{
    PAL_Instance *pal = PAL_CreateInstance();
    const char* TEST_HOSTNAME = "fail.google.com";
    const uint16 TEST_PORT = 80;

    PAL_NetConnection* pConn = NULL;
    PAL_NetConnectionConfig cfg = {0};
    NetworkCallbackUserData* pUserData = NetworkCallbackUserDataCreate();
    PAL_Error err = PAL_Ok;

    CU_ASSERT(pUserData != NULL);

    PAL_NetInitialize(pal);

    cfg.netDnsResultCallback = TestNetwork_DnsResultCallback;
    cfg.userData = pUserData;

    err = PAL_NetCreateConnection(pal, PNCP_TCP, &pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_NetOpenConnection(pConn, &cfg, TEST_HOSTNAME, TEST_PORT);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    // This test does not really work. The callback "TestNetwork_DnsResultCallback" is not being called.
    // I think it would only be called once you make a send request. (That case is tested in other unit tests).

//    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->connectedEvent, 5000));

    err = PAL_NetCloseConnection(pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_NetDestroyConnection(pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    PAL_NetShutdown(pal);
    PAL_Destroy(pal);

    NetworkCallbackUserDataDestroy(pUserData);
}

/*! @brief Test simple HTTP network connect & request.

@return None. CUnit asserts are called on failure.

@see TestNetwork_Initialize
@see TestNetwork_Cleanup
*/
void
TestNetworkHttpConnectRequest(void)
{
    DoConnectRequest(PNCP_HTTP);
}



/*! @brief Test multiple requests on a HTTP connection.

@return None. CUnit asserts are called on failure.

@see TestNetwork_Initialize
@see TestNetwork_Cleanup
*/
void
TestNetworkHttpMultipleRequests(void)
{
    PAL_Instance *pal = PAL_CreateInstance();
    const char* TEST_HOSTNAME = "intranet.nimone.com";

    const char* TEST_OBJECT1 ="/dept/dev/ccc/unittestfiles/40k";
    const char* TEST_OBJECT2 ="/dept/dev/ccc/unittestfiles/16mb_forward";
    const char* TEST_OBJECT3 ="/dept/dev/ccc/unittestfiles/16mb_reverse";

    const char* TEST_ADDITIONAL_HEADER = "X-NAVBUILDER-CLIENTID: 000102030405060708090a0b0c0d0e0f\r\n";

    PAL_NetConnection* pConn = NULL;
    PAL_NetConnectionConfig cfg = {0};
    NetworkCallbackUserData* pUserData = NetworkCallbackUserDataCreate();

    PAL_Error err = PAL_Ok;

    CU_ASSERT(pUserData != NULL);

    PAL_NetInitialize(pal);

    cfg.netStatusCallback = TestNetwork_ConnectionStatusCallback;
    cfg.netDataSentCallback = TestNetwork_NetDataSentCallback;
    cfg.netHttpDataReceivedCallback = TestNetwork_HttpMultiReqDataCallback;
    cfg.netHttpResponseStatusCallback = TestNetwork_HttpMultiReqStatusCallback;
    cfg.userData = pUserData;

    err = PAL_NetCreateConnection(pal, PNCP_HTTP, &pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_NetOpenConnection(pConn, &cfg, TEST_HOSTNAME, 80);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->connectedEvent, 15000));


    err = PAL_NetHttpSend(pConn, NULL, 0, "GET", TEST_OBJECT1, NULL, TEST_ADDITIONAL_HEADER, (void*)TEST_NETWORK_FILE_40k);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_NetHttpSend(pConn, NULL, 0, "GET", TEST_OBJECT2, NULL, NULL, (void*)TEST_NETWORK_FILE_16mb_forward);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_NetHttpSend(pConn, NULL, 0, "GET", TEST_OBJECT3, NULL, NULL, (void*)TEST_NETWORK_FILE_16mb_reverse);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->httpResponseStatusEvent, 900000));
    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->dataReceivedEvent, 900000));

    err = PAL_NetCloseConnection(pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);
    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->closedEvent, 30000));

    err = PAL_NetDestroyConnection(pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    PAL_NetShutdown(pal);
    PAL_Destroy(pal);

    NetworkCallbackUserDataDestroy(pUserData);
}



void
TestNetworkHttpConnectFail(void)
{
    PAL_Instance *pal = PAL_CreateInstance();
    const char* TEST_HOSTNAME = "fail.nimone.com";
    const char* TEST_REQUEST = "GET fail.html\r\n\r\n";

    PAL_NetConnection* pConn = NULL;
    PAL_NetConnectionConfig cfg = {0};
    NetworkCallbackUserData* pUserData = NetworkCallbackUserDataCreate();

    PAL_Error err = PAL_Ok;

    CU_ASSERT(pUserData != NULL);

    PAL_NetInitialize(pal);

    cfg.netStatusCallback = TestNetwork_ConnectionStatusCallback;
    cfg.netDataSentCallback = TestNetwork_NetDataSentCallback;
    cfg.netDataReceivedCallback = TestNetwork_NetDataReceivedCallback;
    cfg.userData = pUserData;

    err = PAL_NetCreateConnection(pal, PNCP_HTTP, &pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_NetOpenConnection(pConn, &cfg, TEST_HOSTNAME, 80);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_NetSend(pConn, (byte*)TEST_REQUEST, strlen(TEST_REQUEST));
    CU_ASSERT_EQUAL(err, PAL_Ok);

    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->errorOrFailedEvent, 15000));

    err = PAL_NetCloseConnection(pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_NetDestroyConnection(pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    PAL_Destroy(pal);
    PAL_NetShutdown(pal);

    NetworkCallbackUserDataDestroy(pUserData);
}

/*! Test to open and close connection no matter the network is enable or disable

    @return None. CUnit Asserts get called on failures.
*/
void
TestNetworkHttpConnectAndClose(void)
{
    ConnectAndClose(PNCP_HTTP);
}

/*! @brief Test simple HTTPS network connect & request.

@return None. CUnit asserts are called on failure.

@see TestNetwork_Initialize
@see TestNetwork_Cleanup
*/
void
TestNetworkHttpsConnectRequest(void)
{
    DoConnectRequest(PNCP_HTTPS);
}

/*! @brief Test multiple requests on a HTTP connection.

@return None. CUnit asserts are called on failure.

@see TestNetwork_Initialize
@see TestNetwork_Cleanup
*/
void
TestNetworkHttpsMultipleRequests(void)
{
    PAL_Instance *pal = PAL_CreateInstance();

    const char* TEST_REQUEST1 = "/tiles/cmd/mobileTile?g=406&base=r&a=0230132021020&size=256&mkt=en-US&dpi=96&device=mobile&ids=12345678";
    const char* TEST_REQUEST2 = "/tiles/cmd/mobileTile?g=406&base=r&a=0230132021020&size=128&mkt=en-US&dpi=96&device=mobile&ids=12345678";


    const char* TEST_ADDITIONAL_HEADER = "X-NAVBUILDER-CLIENTID: 000102030405060708090a0b0c0d0e0f\r\n";

    PAL_NetConnection* pConn = NULL;
    PAL_NetConnection* pSecondConn = NULL;

    PAL_NetConnectionConfig cfg = {0};
    PAL_NetConnectionConfig secondCfg = {0};

    NetworkCallbackUserData* pUserData = NetworkCallbackUserDataCreate();
    NetworkCallbackUserData* pSecondUserData = NetworkCallbackUserDataCreate();

    PAL_Error err = PAL_Ok;

    CU_ASSERT(pUserData != NULL);

    PAL_NetInitialize(pal);

    cfg.netStatusCallback = TestNetwork_ConnectionStatusCallback;
    cfg.netDataSentCallback = TestNetwork_NetDataSentCallback;
    cfg.netHttpDataReceivedCallback = TestNetwork_HttpDataReceivedCallback;
    cfg.netHttpResponseStatusCallback = TestNetwork_HttpStatusCallback;
    cfg.userData = pUserData;

    secondCfg.netStatusCallback = TestNetwork_ConnectionStatusCallback;
    secondCfg.netDataSentCallback = TestNetwork_NetDataSentCallback;
    secondCfg.netHttpDataReceivedCallback = TestNetwork_HttpDataReceivedCallback;
    secondCfg.netHttpResponseStatusCallback = TestNetwork_HttpStatusCallback;
    secondCfg.userData = pSecondUserData;

    err = PAL_NetCreateConnection(pal, PNCP_HTTPS, &pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);
    err = PAL_NetCreateConnection(pal, PNCP_HTTPS, &pSecondConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_NetOpenConnection(pConn, &cfg, TEST_DEV12_HOSTNAME, HTTPS_PORT);
    CU_ASSERT_EQUAL(err, PAL_Ok);
    err = PAL_NetOpenConnection(pSecondConn, &secondCfg, TEST_DEV12_HOSTNAME, HTTPS_PORT);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->connectedEvent, 15000));
    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pSecondUserData->connectedEvent, 15000));

    err = PAL_NetHttpSend(pConn, NULL, 0,
                          "GET", TEST_REQUEST1,
                          NULL,  TEST_ADDITIONAL_HEADER, NULL);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_NetHttpSend(pSecondConn, NULL, 0,
                          "GET", TEST_REQUEST2,
                          NULL,  TEST_ADDITIONAL_HEADER, NULL);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->httpResponseStatusEvent, 900000));
    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->dataReceivedEvent, 900000));

    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pSecondUserData->httpResponseStatusEvent, 900000));
    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pSecondUserData->dataReceivedEvent, 900000));

    err = PAL_NetCloseConnection(pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);
    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->closedEvent, 30000));

    err = PAL_NetCloseConnection(pSecondConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);
    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pSecondUserData->closedEvent, 30000));

    err = PAL_NetDestroyConnection(pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_NetDestroyConnection(pSecondConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    NetworkCallbackUserDataDestroy(pUserData);
}



void
TestNetworkHttpsConnectFail(void)
{
    PAL_Instance *pal = PAL_CreateInstance();
    const char* TEST_HOSTNAME = "FailedLinkFailedLink.com";

    PAL_NetConnection* pConn = NULL;
    PAL_NetConnectionConfig cfg = {0};
    NetworkCallbackUserData* pUserData = NetworkCallbackUserDataCreate();

    PAL_Error err = PAL_Ok;

    CU_ASSERT(pUserData != NULL);

    PAL_NetInitialize(pal);

    cfg.netStatusCallback = TestNetwork_ConnectionStatusCallback;
    cfg.netDataSentCallback = TestNetwork_NetDataSentCallback;
    cfg.netDataReceivedCallback = TestNetwork_NetDataReceivedCallback;
    cfg.userData = pUserData;

    err = PAL_NetCreateConnection(pal, PNCP_HTTPS, &pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_NetOpenConnection(pConn, &cfg, TEST_HOSTNAME, HTTPS_PORT);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    // If you use proxy this test will be failed.
    // To verify connection, PAL_NetConnection check IP address of host, but if you use proxy,
    // in case of 'bad' host, CURL lib (which used for https connections) will return proxy IP address,
    // after that we will wait on the proxy-server socket, so as result we have timeout, but not fail or error

    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->errorOrFailedEvent, 15000));

    err = PAL_NetCloseConnection(pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_NetDestroyConnection(pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    NetworkCallbackUserDataDestroy(pUserData);
}

/*! Test to open and close connection no matter the network is enable or disable

    @return None. CUnit Asserts get called on failures.
*/

void
TestNetworkHttpsConnectAndClose(void)
{
    ConnectAndClose(PNCP_HTTPS);
}

void
TestNetworkTLSConnectRequest(void)
{
    PAL_Instance *pal = PAL_CreateInstance();
//    const char* TEST_HOSTNAME = "184.50.90.234";
    const char* TEST_HOSTNAME = "www.paypal.com";
    const uint16 TEST_PORT = 443;
//    const char* TEST_REQUEST1 = "GET https://www.paypalobjects.com/webstatic/mktg/consumer/pages/home/homepage-sell.png HTTP/1.1\r\n\r\n";
    const char* TEST_REQUEST1 = "GET https://www.paypal.com/c2/webapps/mpp/home HTTP/1.1\r\n\r\n";

    PAL_NetConnection* pConn = NULL;
    PAL_NetConnectionConfig cfg = {0};
    NetworkCallbackUserData* pUserData = NetworkCallbackUserDataCreate();
    PAL_Error err = PAL_Ok;

    CU_ASSERT(pUserData != NULL);

    PAL_NetInitialize(pal);

    cfg.netStatusCallback = TestNetwork_ConnectionStatusCallback;
    cfg.netDataSentCallback = TestNetwork_NetDataSentCallback;
    cfg.netDataReceivedCallback = TestNetwork_NetDataReceivedCallback;
    cfg.userData = pUserData;
    cfg.tlsConfig.name = (const char*)nsl_malloc(1024);
    nsl_memset((void*)cfg.tlsConfig.name, 0, 1024);
    nsl_strncpy((char*)cfg.tlsConfig.name, TEST_HOSTNAME, nsl_strlen(TEST_HOSTNAME));
    cfg.tlsConfig.override = PAL_TLS_OVERRIDE_NONE;

    err = PAL_NetCreateConnection(pal, PNCP_TCPTLS, &pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_NetOpenConnection(pConn, &cfg, TEST_HOSTNAME, TEST_PORT);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->connectedEvent, 15000));

    err = PAL_NetSend(pConn, (byte*)TEST_REQUEST1, strlen(TEST_REQUEST1));
    CU_ASSERT_EQUAL(err, PAL_Ok);

    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->dataReceivedEvent, 10000));

    err = PAL_NetCloseConnection(pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);
    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->closedEvent, 10000));

    // Send the request again after closing connection on purpose.
    err = PAL_NetSend(pConn, (byte*)TEST_REQUEST1, strlen(TEST_REQUEST1)); // It cannot be sent, err will be PAL_ErrNoInit. Because the connection is closed.
    CU_ASSERT_NOT_EQUAL(err, PAL_Ok);
    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->dataReceivedEvent, 10000));

    err = PAL_NetDestroyConnection(pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    nsl_free((void*)cfg.tlsConfig.name);

    PAL_NetShutdown(pal);
    PAL_Destroy(pal);

    NetworkCallbackUserDataDestroy(pUserData);
}

void
TestNetworkTLSConnectRequest2(void)
{
    PAL_Instance *pal = PAL_CreateInstance();
    const char* TEST_HOSTNAME = "www.paypal.com";
    const uint16 TEST_PORT = 443;
    const char* TEST_REQUEST1 = "GET https://www.paypalobjects.com/webstatic/mktg/consumer/pages/home/homepage-sell.png HTTP/1.1\r\n\r\n";

    PAL_NetConnection* pConn = NULL;
    PAL_NetConnectionConfig cfg = {0};
    NetworkCallbackUserData* pUserData = NetworkCallbackUserDataCreate();
    PAL_Error err = PAL_Ok;

    CU_ASSERT(pUserData != NULL);

    PAL_NetInitialize(pal);

    cfg.netStatusCallback = TestNetwork_ConnectionStatusCallback;
    cfg.netDataSentCallback = TestNetwork_NetDataSentCallback;
    cfg.netDataReceivedCallback = TestNetwork_NetDataReceivedCallback;
    cfg.userData = pUserData;
    cfg.tlsConfig.name = (const char*)nsl_malloc(1024);
    nsl_memset((void*)cfg.tlsConfig.name, 0, 1024);
    nsl_strncpy((char*)cfg.tlsConfig.name, TEST_HOSTNAME, nsl_strlen(TEST_HOSTNAME));
    cfg.tlsConfig.override = PAL_TLS_OVERRIDE_ALL;

    err = PAL_NetCreateConnection(pal, PNCP_TCPTLS, &pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_NetOpenConnection(pConn, &cfg, TEST_HOSTNAME, TEST_PORT);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->connectedEvent, 15000));

    err = PAL_NetSend(pConn, (byte*)TEST_REQUEST1, strlen(TEST_REQUEST1));
    CU_ASSERT_EQUAL(err, PAL_Ok);

    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->dataReceivedEvent, 30000));

    err = PAL_NetCloseConnection(pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);
    CU_ASSERT_TRUE(WaitForCallbackCompletedEvent(pUserData->closedEvent, 15000));

    err = PAL_NetDestroyConnection(pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    nsl_free((void*)cfg.tlsConfig.name);

    PAL_NetShutdown(pal);
    PAL_Destroy(pal);

    NetworkCallbackUserDataDestroy(pUserData);
}

void
TestNetworkTLSConnectFail(void)
{
    PAL_Instance *pal = PAL_CreateInstance();
    const char* TEST_HOSTNAME = "fail.paypal.com";
    const uint16 TEST_PORT = 443;
    const char* TEST_REQUEST = "GET fail.html\r\n\r\n";

    PAL_NetConnection* pConn = NULL;
    PAL_NetConnectionConfig cfg = {0};
    NetworkCallbackUserData* pUserData = NetworkCallbackUserDataCreate();

    PAL_Error err = PAL_Ok;

    CU_ASSERT(pUserData != NULL);

    PAL_NetInitialize(pal);

    cfg.netStatusCallback = TestNetwork_ConnectionStatusCallback;
    cfg.netDataSentCallback = TestNetwork_NetDataSentCallback;
    cfg.netDataReceivedCallback = TestNetwork_NetDataReceivedCallback;
    cfg.userData = pUserData;
    cfg.tlsConfig.name = (const char*)nsl_malloc(1024);
    nsl_memset((void*)cfg.tlsConfig.name, 0, 1024);
    cfg.tlsConfig.override = PAL_TLS_OVERRIDE_NONE;

    err = PAL_NetCreateConnection(pal, PNCP_TCPTLS, &pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_NetOpenConnection(pConn, &cfg, TEST_HOSTNAME, TEST_PORT);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_NetSend(pConn, (byte*)TEST_REQUEST, strlen(TEST_REQUEST));
    CU_ASSERT_EQUAL(err, PAL_Ok); // TLSConnection sends data in async way, so here the err is PAL_Ok.

    // No need to wait for event here.

    err = PAL_NetCloseConnection(pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_NetDestroyConnection(pConn);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    nsl_free((void*)cfg.tlsConfig.name);

    PAL_NetShutdown(pal);
    PAL_Destroy(pal);

    NetworkCallbackUserDataDestroy(pUserData);
}

/*! Add all your test functions here

@return None
*/
void
TestNetwork_AddAllTests( CU_pSuite pTestSuite )
{
    // ! Add all your function names here !
    CU_add_test(pTestSuite, "TestNetworkTcpConnectRequest", &TestNetworkTcpConnectRequest);
    CU_add_test(pTestSuite, "TestNetworkTcpConnectFail", &TestNetworkTcpConnectFail);

    CU_add_test(pTestSuite, "TestNetworkTcpDnsRequest", TestNetworkTcpDnsRequest);
    CU_add_test(pTestSuite, "TestNetworkTcpDnsRequestFail", TestNetworkTcpDnsRequestFail);

    CU_add_test(pTestSuite, "TestNetworkHttpConnectAndClose", &TestNetworkHttpConnectAndClose);
    CU_add_test(pTestSuite, "TestNetworkHttpConnectRequest", &TestNetworkHttpConnectRequest);
    CU_add_test(pTestSuite, "TestNetworkHttpMultipleRequests", &TestNetworkHttpMultipleRequests);
    CU_add_test(pTestSuite, "TestNetworkHttpConnectFail", &TestNetworkHttpConnectFail);

    CU_add_test(pTestSuite, "TestNetworkHttpsConnectAndClose", &TestNetworkHttpsConnectAndClose);
    CU_add_test(pTestSuite, "TestNetworkHttpsConnectRequest", &TestNetworkHttpsConnectRequest);
    CU_add_test(pTestSuite, "TestNetworkHttpsMultipleRequests", &TestNetworkHttpsMultipleRequests);
    CU_add_test(pTestSuite, "TestNetworkHttpsConnectFail", &TestNetworkHttpsConnectFail);

    CU_add_test(pTestSuite, "TestNetworkTLSConnectRequest", &TestNetworkTLSConnectRequest);
    CU_add_test(pTestSuite, "TestNetworkTLSConnectRequest2", &TestNetworkTLSConnectRequest2);
    CU_add_test(pTestSuite, "TestNetworkTLSConnectFail", &TestNetworkTLSConnectFail);
};


/*! Add common initialization code here.

@return 0

@see TestNetwork_SuiteCleanup
*/
int
TestNetwork_SuiteSetup()
{
    return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestNetwork_SuiteSetup
*/
int
TestNetwork_SuiteCleanup()
{
    return 0;
}

/*! @} */


