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

    @file     TestNetwork.c
    @date     01/22/2009
    @defgroup TestNetwork_h System Tests for Network connection.
*/
/*
    See description in header file.

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

extern "C"
{
#include "testnetwork.h"
#include "main.h"
#include "statictpslib.h"
#include "palconfig.h"
#include "palfile.h"
#include "palnet.h"
#include "platformutil.h"
#include "nbnetworkoptions.h"
#include "abqalog.h"
#include "utility.h"
#include "paltaskqueue.h"

#include "abservermessagehandler.h"
#include "abservermessageinformation.h"
#include "abservermessageparameters.h"

#include "nbgeocodehandler.h"
#include "nbgeocodeparameters.h"
#include "nbreversegeocodehandler.h"
#include "nbreversegeocodeparameters.h"
#include "nbcontextaccess.h"
#include "nbcontextprotected.h"
#include "nbvectortilemanager.h"
#include "nbpersistentdata.h"
#include "nbanalyticsprotected.h"
#include "nbwifiprobes.h"
}
#include "nbgm.h"


// Constants .....................................................................................

// How much should we download
const int RESULTS_PER_SLICE = 10;

static const byte VOICE_CACHE_OBFUSCATE_KEY[] = {
    70, 225,  82,  73, 156, 130, 140, 111, 157, 241,  72,  76,  78, 245,
    167,  59,  40,  95, 131, 205,  65, 110, 123, 157, 172,   7, 189, 197,
    104,  51, 121,  62, 101,  50, 157,  94, 149, 201, 107, 202, 221, 200,
    97,  16,  90,  76, 214, 231, 110,  98, 178, 222,  76,   6, 229, 112,
    110, 187, 208, 148, 124,  66, 161, 228, 185,  29, 228, 196, 205, 149,
    86, 226,  84,   5, 203, 189, 221,  98, 243, 148, 120,  70, 131, 242,
    184, 183, 189, 237,  54, 147,  65, 106, 218,  12,  22,  62, 171, 195,
    176,  28, 142,  42,   5,  29, 205,  51,  26, 172, 158,  51, 147,  79,
    97, 208, 154, 189,   3
};

/*! Obfuscate key for tile cache. */
static const byte RASTER_TILE_CACHE_OBFUSCATE_KEY[] =
{
    70, 225,  82,  73, 156, 130, 140, 111, 157, 241,  72,  76,  78, 245,
    167,  59,  40,  95, 131, 205,  65, 110, 123, 157, 172,   7, 189, 197,
    104,  51, 121,  62, 101,  50, 157,  94, 149, 201, 107, 202, 221, 200,
    97,  16,  90,  76, 214, 231, 110,  98, 178, 222,  76,   6, 229, 112,
    110, 187, 208, 148, 124,  66, 161, 228, 185,  29, 228, 196, 205, 149,
    86, 226,  84,   5, 203, 189, 221,  98, 243, 148, 120,  70, 131, 242,
    184, 183, 189, 237,  54, 147,  65, 106, 218,  12,  22,  62, 171, 195,
    176,  28, 142,  42,   5,  29, 205,  51,  26, 172, 158,  51, 147,  79,
    97, 208, 154, 189,   3
};


static const char SYSTEM_TEST_INVALID_CREDENTIALS[] = "DeadbeafDeadbeafDeadbeafDeadbeafDeadbeaf";   // An invalid credential for network failure handling tests

// TOKENS for System Tests (ABNAV_CCC) ==============================================================

// Current token spreadsheet at docs: //depot/scm/Tokens/Production/nbtokens.xls

//feature1------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]        = "B6hwvGHV3PY0k+gUlt+LCvPCBVYLlEbJlHaVq4p0";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]         = "B6hwvGHV3PY0k+gUlt+LCvPCBVYLlEbJlHaVq4p0";

// QA1 ---------------------------------------------------------------------------------------------
// static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "s2f2pYdiNUJznjyuNKY881oVR/0kbE9okRESFE40";
// static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "s2f2pYdiNUJznjyuNKY881oVR/0kbE9okRESFE40";

// QA3 -------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "mFfK0n94fAUE18VnH6zcg5ibW2TPHkB/pEecLT2z";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "mFfK0n94fAUE18VnH6zcg5ibW2TPHkB/pEecLT2z";

// QA8 ---------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "Hh1krqoAHFClU1h+RfVkOVqrjktxD/PvRic+0hS+";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "Hh1krqoAHFClU1h+RfVkOVqrjktxD/PvRic+0hS+";

// CS3 ---------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "mkX7zxVMuOKZpx+7xl0dEE1la0phoPuK6D1RkSOl";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "mkX7zxVMuOKZpx+7xl0dEE1la0phoPuK6D1RkSOl";

// CS11 --------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "n5/F53dio8+egrnS/Cu72H0f+mYN7EY8HoDWjM9G";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "n5/F53dio8+egrnS/Cu72H0f+mYN7EY8HoDWjM9G";

// DEV1 -----------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "MOU7aZ0+fQZmE2uW8kU2IwPbpyDkzEJ7nMFyGZLY";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "MOU7aZ0+fQZmE2uW8kU2IwPbpyDkzEJ7nMFyGZLY";

// DEV11 --------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "+qWTzdsQzuvO9Ci9Ipc3/AZsmhbZCPyEfXNNqnwY";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "+qWTzdsQzuvO9Ci9Ipc3/AZsmhbZCPyEfXNNqnwY";

// DEV13 ---------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "xyzYFFUnMb8zW00EUV5ed+dTuvFVpVuSHa+I6mDj";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "xyzYFFUnMb8zW00EUV5ed+dTuvFVpVuSHa+I6mDj";

// DEV14 -------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "7ekw6R/8xuJqPykKsyT15KbwxPe2ExNc5sSqHNXk";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "7ekw6R/8xuJqPykKsyT15KbwxPe2ExNc5sSqHNXk";

// CS3 -------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "ZTAwUoknDo5ppJsdyHWkX3WSk/GnOmYjXaBv5gr/";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "ZTAwUoknDo5ppJsdyHWkX3WSk/GnOmYjXaBv5gr/";

// DEV16 -------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]       = "EeA6zQE9UZsZem2zatcJgac/vnCIb/MNmTRfKDMt";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]        = "EeA6zQE9UZsZem2zatcJgac/vnCIb/MNmTRfKDMt";

// tj-lnx3 -------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "xdrHqvAq21MMjdkwHTvmXzT1PuxhXfgo5+uW+LJ9";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "xdrHqvAq21MMjdkwHTvmXzT1PuxhXfgo5+uW+LJ9";

// qa12 -------------------------------------------------------------------------------------------

// TCS	NBI	CCC			472231wKWrYNxzLCW1dP/8yMW2Q9rIjzHOW7Qxxz	eb1dafb671e8952e5a1c555c0b9f59573c8ab6a8	QA12	Non-Mediated	nbi.nimlbs.net	nbisdk	MUX	
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "472231wKWrYNxzLCW1dP/8yMW2Q9rIjzHOW7Qxxz";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "472231wKWrYNxzLCW1dP/8yMW2Q9rIjzHOW7Qxxz";

//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "FpNYNvrEHv92IhcRn5j0N6MvSNKTFUlkObe4Y6+V";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "FpNYNvrEHv92IhcRn5j0N6MvSNKTFUlkObe4Y6+V";

// // DEV12 -------------------------------------------------------------------------------------------
// static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "A/PckGjSgnqfU0w7ohwlwAAOm5bIm5/zLYasHLHM";
// static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "A/PckGjSgnqfU0w7ohwlwAAOm5bIm5/zLYasHLHM";

// QA4 -------------------------------------------------------------------------------------------
static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "d3f+tXtg7HrKj/kaNM9XEtRO8dEKlrErrzQaigUC";
static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "d3f+tXtg7HrKj/kaNM9XEtRO8dEKlrErrzQaigUC";

// // DEV4 ------------------------------------------------------------------------------------------
// static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "+i88bW19I5thJdZYklM65s3bXk4cCckB/uaD4C2n";
// static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "+i88bW19I5thJdZYklM65s3bXk4cCckB/uaD4C2n";

// CS12 -------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "UlJBp+DInAf1WI38LPq198S1l54k6V4KIFo83YMe";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "UlJBp+DInAf1WI38LPq198S1l54k6V4KIFo83YMe";

// DEV6 -------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "2GnI8/kfWu8s3+h/YRfSokprZzYjPr9B9Ltjz6uV";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "2GnI8/kfWu8s3+h/YRfSokprZzYjPr9B9Ltjz6uV";

// qa16 -------------------------------------------------------------------------------------------
//static const char DEFAULT_NETWORK_TOKEN_HTTP[]      = "Ew/TZU36934y2EbZFeWQUUAZ9cme6U5jt/IRWtib";
//static const char DEFAULT_NETWORK_TOKEN_TCP[]       = "Ew/TZU36934y2EbZFeWQUUAZ9cme6U5jt/IRWtib";

static const char TPSLIB_FILE[]   = "tesla.tpl";
static const char GUID_FILENAME[] = "guid.bin";
static const char PERSISTENT_DATA_FILENAME[] = "ccc.bin";

static const char* NBGM_WORK_SPACE = "NB_UNITTEST_DATA\\";

#define DEFAULT_PORT_HTTP     80
#define DEFAULT_PORT_HTTPS    443
#define DEFAULT_PORT_TCP      8128
#define DEFAULT_PORT_TCP_TLS  8129

#define TEST_HTTP_DOWNLOAD_CONNECTION_COUNT 6


// Local Functions ...............................................................................

static PAL_Error GetFileData(PAL_Instance* pal, const char* filename, char** dataBuffer, size_t* dataBufferLength);

static void VerifyNotifyEventCounts(int openCount, int closeCount, int dataPendingTrueCount, int dataPendingFalseCount);

static NB_Error GeocodeRequest(NB_Context* context, const char* address, NB_GeocodeHandler* handler);
static NB_Error ReverseGeocodeRequest(NB_Context* context, NB_LatitudeLongitude* location, NB_ReverseGeocodeHandler* handler);

static void ServerMessageCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* pUserData);
static void TestNetworkGeocodeCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* pUserData);
static void TestNetworkReverseGeocodeCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* pUserData);
static void NoTemplatesCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* pUserData);

static void NetworkNotifyEventCallback(NB_NetworkNotifyEvent event, void* data, void* userData);
static NB_NetworkConfiguration* GetNetworkConfiguration(PAL_Instance* pal, const char* credential, const char* hostname, NB_NetworkProtocol protocol, uint16 port, const char* tpsBuffer, size_t tpsBufferLength, const char* guidBuffer, size_t guidBufferLength);
static nb_boolean CreateContext(PAL_Instance* pal, const char* credential, const char* hostname, NB_NetworkProtocol protocol, uint16 port, NB_Context** context);
static nb_boolean SavePersistentDataToFileCallback(NB_PersistentData* persistentData, void* userData);
static nb_boolean TestPersistentDataChangedCallback(NB_PersistentData* persistentData, void* userData);
static NB_Error SetTargetMappings(NB_Context* context);

// All tests. Add all tests to the TestNetwork_AddTests function
static void TestNetworkGetServerMessageStatus(void);
static void TestNetworkConnectFail(void);
static void TestNetworkMultiRequest(void);
static void TestNetworkNotifyEvent1(void);
static void TestNetworkNotifyEvent2(void);
static void TestNetworkInvalidConfigurations(void);
static void TestNetworkNoTemplates(void);
static void TestNetworkPersistentDataCreate(void);
static void TestNetworkPersistentDataSetGet(void);
static void TestNetworkPersistentDataMasterClear(void);

static const char* TestNetwork_GetTpslibFile();
static const char* TestNetwork_GetHostname();

static void CreateContextCb(PAL_Instance* pal, void* userData);

struct TestNetworkCallbackData
{
    void*                           event;          /*!< Event to signal when callback complete */
    nb_boolean                      succeeded;      /*!< Whether or not request was successful */
    AB_ServerMessageInformation*    message;        /*!< Version retrieved from server */
};


struct NotifyEventInfo
{
    NotifyEventInfo():openCount(0), closeCount(0), dataPendingTrueCount(0), dataPendingFalseCount(0), closeEvent(NULL){};

    int     openCount;
    int     closeCount;
    int     dataPendingTrueCount;
    int     dataPendingFalseCount;
    void*   closeEvent;
};

struct CreateContextCbInfo
{
    NB_Context** context;
    void*  completeEvent;
};

NotifyEventInfo* g_notifyEventInfo = NULL;

int g_serializationNotifyCallbackCount = 0;

/*! Add all your test functions here

@return None
*/
void
TestNetwork_AddTests( CU_pSuite pTestSuite, int level )
{
    // ! Add all your function names here !
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestNetworkInvalidConfigurations", TestNetworkInvalidConfigurations);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestNetworkGetServerMessageStatus", TestNetworkGetServerMessageStatus);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestNetworkConnectFail", TestNetworkConnectFail);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestNetworkMultiRequest", TestNetworkMultiRequest);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestNetworkNotifyEvent1", TestNetworkNotifyEvent1);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestNetworkNotifyEvent2", TestNetworkNotifyEvent2);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestNetworkNoTemplates", TestNetworkNoTemplates);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestNetworkPersistentDataCreate", TestNetworkPersistentDataCreate);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestNetworkPersistentDataSetGet", TestNetworkPersistentDataSetGet);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestNetworkPersistentDataMasterClear", TestNetworkPersistentDataMasterClear);
};


/*! Add common initialization code here.

@return 0

@see TestNetwork_SuiteCleanup
*/
int
TestNetwork_SuiteSetup()
{
    g_notifyEventInfo = new NotifyEventInfo;
    return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestNetwork_SuiteSetup
*/
int
TestNetwork_SuiteCleanup()
{
    if (g_notifyEventInfo)
    {
        delete g_notifyEventInfo;
        g_notifyEventInfo = NULL;
    }

    return 0;
}


void TestNetworkGetServerMessageStatus(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    AB_ServerMessageHandler* handler = 0;
    AB_ServerMessageParameters* parameters = 0;
    NB_RequestHandlerCallback callback = { ServerMessageCallback, 0 };
    struct TestNetworkCallbackData callbackData = { 0 };

    callback.callbackData = &callbackData;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    CU_ASSERT_EQUAL(AB_ServerMessageHandlerCreate(context, &callback, &handler), NE_OK);
    CU_ASSERT_EQUAL(AB_ServerMessageParametersCreateStatus(context, "en", 0, &parameters), NE_OK);
    if (parameters)
    {
        callbackData.event = CreateCallbackCompletedEvent();
        CU_ASSERT_PTR_NOT_NULL(callbackData.event);

        if (handler && parameters)
        {
            CU_ASSERT_EQUAL(AB_ServerMessageHandlerStartRequest(handler, parameters), NE_OK);
        }

        CU_ASSERT(WaitForCallbackCompletedEvent(callbackData.event, 35000));
        CU_ASSERT_PTR_NOT_NULL(callbackData.message);

        if (callbackData.message)
        {
            AB_ServerMessageInformationDestroy(callbackData.message);
        }

        CU_ASSERT_EQUAL(AB_ServerMessageParametersDestroy(parameters), NE_OK);
    }

    if (handler)
    {
        CU_ASSERT_EQUAL(AB_ServerMessageHandlerDestroy(handler), NE_OK);
    }

    DestroyContext(context);
    PAL_Destroy(pal);

    DestroyCallbackCompletedEvent(callbackData.event);
}

/*! @brief Test for error handling on network connect failure.

@return None. CUnit asserts are called on failure.

@see TestNetwork_Initialize
@see TestNetwork_Cleanup
*/
void
TestNetworkConnectFail(void)
{
    PAL_Instance* pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);

    if (pal)
    {
        PAL_Error palError = PAL_Failed;
        nb_boolean rc = FALSE;
        NB_Context* context = 0;

        palError = PAL_NetInitialize(pal);
        CU_ASSERT_EQUAL(rc, PAL_Ok);

        rc = CreateContext(pal, SYSTEM_TEST_INVALID_CREDENTIALS, TestNetwork_GetHostname(), TestNetwork_GetNetProtocol(), TestNetwork_GetPort(),&context);
        CU_ASSERT_EQUAL(rc, TRUE);
        CU_ASSERT_PTR_NOT_NULL(context);

        if (rc)
        {
            NB_GeocodeParameters* parameters = 0;
            NB_Error err = NE_OK;

            err = NB_GeocodeParametersCreateAirport(context, "sna", 10, &parameters);
            CU_ASSERT_EQUAL(err, NE_OK);
            if (!err)
            {
                NB_GeocodeHandler* handler = 0;
                NB_RequestHandlerCallback callback = { TestNetworkGeocodeCallback, 0 };
                struct TestNetworkCallbackData callbackData = { 0 };

                callback.callbackData = &callbackData;

                /*
                    @todo: Rename function or clarify code!
                    This code doesn't look to me like a ConnectFail test???
                */

                err = NB_GeocodeHandlerCreate(context, &callback, &handler);
                CU_ASSERT_EQUAL(err, NE_OK);
                if (!err)
                {
                    err = NB_GeocodeHandlerStartRequest(handler, parameters);
                    CU_ASSERT_EQUAL(err, NE_OK);
                    if (!err)
                    {
                        callbackData.event = CreateCallbackCompletedEvent();
                        CU_ASSERT_PTR_NOT_NULL(callbackData.event);
                        CU_ASSERT(WaitForCallbackCompletedEvent(callbackData.event, 35000));
                        CU_ASSERT_EQUAL(callbackData.succeeded, FALSE);
                        DestroyCallbackCompletedEvent(callbackData.event);
                        callbackData.event = NULL;
                    }

                    NB_GeocodeHandlerDestroy(handler);
                }

                NB_GeocodeParametersDestroy(parameters);
            }
        }

        if (context)
        {
            DestroyContext(context);
        }
    }

    if (pal)
    {
        PAL_NetShutdown(pal);
        PAL_DestroyInstance(pal);
    }
}


void TestNetworkNotifyEvent1(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    AB_ServerMessageHandler* handler = 0;
    AB_ServerMessageParameters* parameters = 0;
    NB_RequestHandlerCallback callback = { ServerMessageCallback, 0 };
    struct TestNetworkCallbackData callbackData = { 0 };

	g_notifyEventInfo->closeCount = 0;
	g_notifyEventInfo->dataPendingFalseCount = 0;
	g_notifyEventInfo->dataPendingTrueCount = 0;
	g_notifyEventInfo->openCount = 0;

    callback.callbackData = &callbackData;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    CU_ASSERT_EQUAL(AB_ServerMessageHandlerCreate(context, &callback, &handler), NE_OK);
    CU_ASSERT_EQUAL(AB_ServerMessageParametersCreateStatus(context, "en", 0, &parameters), NE_OK);
    if (parameters)
    {
        callbackData.event = CreateCallbackCompletedEvent();
        CU_ASSERT_PTR_NOT_NULL(callbackData.event);

        if (handler && parameters)
        {
            CU_ASSERT_EQUAL(AB_ServerMessageHandlerStartRequest(handler, parameters), NE_OK);
        }

        // Should only have a data pending true at this point
        VerifyNotifyEventCounts(0, 0, 1, 0);

        CU_ASSERT(WaitForCallbackCompletedEvent(callbackData.event, 35000));
        CU_ASSERT_PTR_NOT_NULL(callbackData.message);

        if (callbackData.message)
        {
            AB_ServerMessageInformationDestroy(callbackData.message);
        }

        CU_ASSERT_EQUAL(AB_ServerMessageParametersDestroy(parameters), NE_OK);
    }

    if (handler)
    {
        CU_ASSERT_EQUAL(AB_ServerMessageHandlerDestroy(handler), NE_OK);
    }

    // One of everything except close should have happened by now
    VerifyNotifyEventCounts(1, 0, 1, 1);

    DestroyContext(context);
    PAL_Destroy(pal);

    // One of everything now
    VerifyNotifyEventCounts(1, 1, 1, 1);

    DestroyCallbackCompletedEvent(callbackData.event);
}


// This test is not part of the smoke test suite because it waits
// for the network layer to timeout and close
void TestNetworkNotifyEvent2(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    AB_ServerMessageHandler* handler = 0;
    AB_ServerMessageParameters* parameters = 0;
    NB_RequestHandlerCallback callback = { ServerMessageCallback, 0 };
    struct TestNetworkCallbackData callbackData = { 0 };

    callback.callbackData = &callbackData;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    CU_ASSERT_EQUAL(AB_ServerMessageHandlerCreate(context, &callback, &handler), NE_OK);
    CU_ASSERT_EQUAL(AB_ServerMessageParametersCreateStatus(context, "en", 0, &parameters), NE_OK);
    if (parameters)
    {
        callbackData.event = CreateCallbackCompletedEvent();
        CU_ASSERT_PTR_NOT_NULL(callbackData.event);

        if (handler && parameters)
        {
            CU_ASSERT_EQUAL(AB_ServerMessageHandlerStartRequest(handler, parameters), NE_OK);
        }

        // Should only have a data pending true at this point
        VerifyNotifyEventCounts(0, 0, 1, 0);

        CU_ASSERT(WaitForCallbackCompletedEvent(callbackData.event, 35000));
        CU_ASSERT_PTR_NOT_NULL(callbackData.message);

        if (callbackData.message)
        {
            AB_ServerMessageInformationDestroy(callbackData.message);
        }

        CU_ASSERT_EQUAL(AB_ServerMessageParametersDestroy(parameters), NE_OK);
    }

    if (handler)
    {
        CU_ASSERT_EQUAL(AB_ServerMessageHandlerDestroy(handler), NE_OK);
    }

    // One of everything except close should have happened by now
    VerifyNotifyEventCounts(1, 0, 1, 1);

    // Wait for the network layer to timeout and close connection
    g_notifyEventInfo->closeEvent = CreateCallbackCompletedEvent();
    CU_ASSERT(WaitForCallbackCompletedEvent(g_notifyEventInfo->closeEvent, 45000));
    DestroyCallbackCompletedEvent(g_notifyEventInfo->closeEvent);
    g_notifyEventInfo->closeEvent = 0;

    // One of everything now
    VerifyNotifyEventCounts(1, 1, 1, 1);

    DestroyContext(context);
    PAL_Destroy(pal);

    DestroyCallbackCompletedEvent(callbackData.event);
}


/*! Test network multi-request.

    Test multiple network requests.

    @return none
*/
void
TestNetworkMultiRequest(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_GeocodeHandler* forwardHandler = 0;
    NB_RequestHandlerCallback forwardCallback = { TestNetworkGeocodeCallback, 0 };
    NB_ReverseGeocodeHandler* reverseHandler = 0;
    NB_RequestHandlerCallback reverseCallback = { TestNetworkReverseGeocodeCallback, 0 };
    NB_LatitudeLongitude location = { 36.10, -115.12 };
    struct TestNetworkCallbackData forwardCallbackData = { 0 };
    struct TestNetworkCallbackData reverseCallbackData = { 0 };

    forwardCallback.callbackData = &forwardCallbackData;
    reverseCallback.callbackData = &reverseCallbackData;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    CU_ASSERT_EQUAL(NB_GeocodeHandlerCreate(context, &forwardCallback, &forwardHandler), NE_OK);
    CU_ASSERT_EQUAL(NB_ReverseGeocodeHandlerCreate(context, &reverseCallback, &reverseHandler), NE_OK);

    forwardCallbackData.event = CreateCallbackCompletedEvent();
    CU_ASSERT_PTR_NOT_NULL(forwardCallbackData.event);

    reverseCallbackData.event = CreateCallbackCompletedEvent();
    CU_ASSERT_PTR_NOT_NULL(reverseCallbackData.event);

    GeocodeRequest(context, "6 Liberty Aliso Viejo CA", forwardHandler);
    ReverseGeocodeRequest(context, &location, reverseHandler);

    CU_ASSERT(WaitForCallbackCompletedEvent(forwardCallbackData.event, 60000));
    CU_ASSERT_TRUE(forwardCallbackData.succeeded);

    CU_ASSERT(WaitForCallbackCompletedEvent(reverseCallbackData.event, 60000));
    CU_ASSERT_TRUE(reverseCallbackData.succeeded);

#if WANT_TO_TEST_TWENTY_MINUTES_OF_INACTIVITY
    {
        int i = 0;
        void* notHappeningEvent = CreateCallbackCompletedEvent();

        // Special case... wait 20 minutes to see if network will recover after that much inactivity
        for (i = 0; i < 20; i++)
        {
            WaitForCallbackCompletedEvent(notHappeningEvent, 60000);
        }
        DestroyCallbackCompletedEvent(notHappeningEvent);
    }
#endif

    // Make another geocode request on the same network object to test cause a socket reconnect
    ResetCallbackCompletedEvent(forwardCallbackData.event);
    GeocodeRequest(context, "1600 Pennsylvania Avenue Washington DC", forwardHandler);
    CU_ASSERT(WaitForCallbackCompletedEvent(forwardCallbackData.event, 60000));

    if (forwardHandler)
    {
        NB_GeocodeHandlerDestroy(forwardHandler);
    }

    if (reverseHandler)
    {
        NB_ReverseGeocodeHandlerDestroy(reverseHandler);
    }

    DestroyContext(context);
    PAL_Destroy(pal);

    DestroyCallbackCompletedEvent(forwardCallbackData.event);
    DestroyCallbackCompletedEvent(reverseCallbackData.event);
}


void TestNetworkInvalidConfigurations(void)
{
    NB_NetworkOptionsDetailed options = { 0 };
    NB_NetworkConfiguration* configuration = 0;
    PAL_Config palConfig = { 0 };
    PAL_Instance* pal = PAL_Create(&palConfig);
    char deviceName[50] = { 0 };

    CU_ASSERT_PTR_NOT_NULL(pal);

    GetDeviceName(pal, deviceName, sizeof(deviceName) / sizeof(deviceName[0]));

    options.adsAppId = "N/A";
    options.classId = 0;
    options.credential = DEFAULT_NETWORK_TOKEN_TCP;
    options.device = deviceName;
    options.domain = TestNetwork_GetDomain();
    options.firmwareVersion = "N/A";
    options.language = Test_OptionsGet()->currentLanguage;
    options.mdn = GetMobileDirectoryNumber(pal);
    options.min = GetMobileInformationNumber(pal);
    options.platformId = 0;
    options.port = 8128;
    options.priceType = 0;
    options.protocol = NB_NetworkProtocol_TCP;
    options.tpslibData = "junk";
    options.tpslibLength = nsl_strlen(options.tpslibData);
    options.clientGuidData = "junk";
    options.clientGuidLength = nsl_strlen(options.clientGuidData);

    options.credential = 0;
    CU_ASSERT_EQUAL(NB_NetworkConfigurationCreate(&options, &configuration), NE_INVAL);

    options.credential = "Less than 40 characters";
    CU_ASSERT_EQUAL(NB_NetworkConfigurationCreate(&options, &configuration), NE_INVAL);

    options.credential = "This string, on the other hand, is far more than 40 characters long";
    CU_ASSERT_EQUAL(NB_NetworkConfigurationCreate(&options, &configuration), NE_INVAL);

    options.credential = DEFAULT_NETWORK_TOKEN_HTTP;
    options.domain = 0;
    CU_ASSERT_EQUAL(NB_NetworkConfigurationCreate(&options, &configuration), NE_INVAL);

    options.domain = "";
    CU_ASSERT_EQUAL(NB_NetworkConfigurationCreate(&options, &configuration), NE_INVAL);

    options.domain = TestNetwork_GetDomain();
    options.protocol = NB_NetworkProtocol_Undefined;
    CU_ASSERT_EQUAL(NB_NetworkConfigurationCreate(&options, &configuration), NE_INVAL);

    options.protocol = NB_NetworkProtocol_TCP;
    options.language = 0;
    CU_ASSERT_EQUAL(NB_NetworkConfigurationCreate(&options, &configuration), NE_INVAL);

    options.language = "";
    CU_ASSERT_EQUAL(NB_NetworkConfigurationCreate(&options, &configuration), NE_INVAL);

    options.language = "1";
    CU_ASSERT_EQUAL(NB_NetworkConfigurationCreate(&options, &configuration), NE_INVAL);

    options.tpslibData = 0;
    CU_ASSERT_EQUAL(NB_NetworkConfigurationCreate(&options, &configuration), NE_INVAL);

    options.tpslibData = 0;
    CU_ASSERT_EQUAL(NB_NetworkConfigurationCreate(&options, &configuration), NE_INVAL);

    options.tpslibData = "junk";
    options.tpslibLength = 0;
    CU_ASSERT_EQUAL(NB_NetworkConfigurationCreate(&options, &configuration), NE_INVAL);

    options.tpslibLength = nsl_strlen(options.tpslibData);
    options.clientGuidData = 0;
    CU_ASSERT_EQUAL(NB_NetworkConfigurationCreate(&options, &configuration), NE_INVAL);

    options.clientGuidData = "junk";
    options.clientGuidLength = 0;
    CU_ASSERT_EQUAL(NB_NetworkConfigurationCreate(&options, &configuration), NE_INVAL);

    PAL_Destroy(pal);
}


void TestNetworkNoTemplates(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    AB_ServerMessageHandler* handler = 0;
    AB_ServerMessageParameters* parameters = 0;
    NB_RequestHandlerCallback callback = { NoTemplatesCallback, 0 };
    struct TestNetworkCallbackData callbackData = { 0 };
    char originalTpslib[MAX_TPSLIB_NAME_LENGTH + 1] = { 0  };

    callback.callbackData = &callbackData;

    // Save original tps library name
    nsl_strcpy(originalTpslib, Test_OptionsGet()->tpslib);
    // Set tps library name to qa upload tps
    nsl_strcpy(Test_OptionsGet()->tpslib, "admintesla.tpl");

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    // Restore original tps library name
    nsl_strcpy(Test_OptionsGet()->tpslib, originalTpslib);

    CU_ASSERT_EQUAL(AB_ServerMessageHandlerCreate(context, &callback, &handler), NE_OK);
    CU_ASSERT_EQUAL(AB_ServerMessageParametersCreateStatus(context, "en", 0, &parameters), NE_OK);
    if (parameters)
    {
        callbackData.event = CreateCallbackCompletedEvent();
        CU_ASSERT_PTR_NOT_NULL(callbackData.event);

        if (handler && parameters)
        {
            CU_ASSERT_EQUAL(AB_ServerMessageHandlerStartRequest(handler, parameters), NE_OK);
        }

        CU_ASSERT(WaitForCallbackCompletedEvent(callbackData.event, 35000));
        CU_ASSERT(callbackData.succeeded);

        CU_ASSERT_EQUAL(AB_ServerMessageParametersDestroy(parameters), NE_OK);
    }

    if (handler)
    {
        CU_ASSERT_EQUAL(AB_ServerMessageHandlerDestroy(handler), NE_OK);
    }

    DestroyContext(context);
    PAL_Destroy(pal);

    DestroyCallbackCompletedEvent(callbackData.event);
}

/*! Test persistent data create.

Test persistent data object creation and association with a context.  The object
should be accessible from the context, and should be destroyed with the context.

@return none
*/
void TestNetworkPersistentDataCreate(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    NB_PersistentData* persistentData = 0;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    persistentData = NB_ContextGetPersistentData(context);
    CU_ASSERT_PTR_NOT_NULL(persistentData);

    DestroyContext(context);
    (void)PAL_Destroy(pal);
}


/*! Test persistent data set/get.

Test general get/set functions of a persistent data object.

@return none
*/
void TestNetworkPersistentDataSetGet(void)
{
    const char* digitsName = "test_digits";
    const char* digits = "0123456789";
    const char* digitsReverse = "9876543210";
    const nb_size digitsSize = 10;

    const char* lettersName = "test_letters";
    const char* letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const nb_size lettersSize = 26;

    const char* bytesName = "test_bytes";
    uint8* bytes = 0;
    const nb_size bytesSize = 256;

    int n = 0;

    NB_Error err = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;

    NB_PersistentData* persistentData = 0;
    NB_PersistentDataChangedCallback callback = {0};

    uint8* data = 0;
    nb_size dataSize = 0;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    // create an empty persistent data object for testing only
    err = NB_PersistentDataCreate(NULL, 0, NULL, &persistentData);
    CU_ASSERT_EQUAL(err, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(persistentData);

    // set some data to persist
    err = NB_PersistentDataSet(persistentData, digitsName, (uint8*)digits, digitsSize, FALSE, FALSE);
    CU_ASSERT_EQUAL(err, NE_OK);

    err = NB_PersistentDataSet(persistentData, lettersName, (uint8*)letters, lettersSize, FALSE, FALSE);
    CU_ASSERT_EQUAL(err, NE_OK);

    // verify set items
    err = NB_PersistentDataGet(persistentData, digitsName, &data, &dataSize);
    CU_ASSERT_EQUAL(err, NE_OK);
    CU_ASSERT(data && !nsl_memcmp((const char*)data, digits, digitsSize));
    CU_ASSERT_EQUAL(dataSize, digitsSize);
    nsl_free(data);

    err = NB_PersistentDataGet(persistentData, lettersName, &data, &dataSize);
    CU_ASSERT_EQUAL(err, NE_OK);
    CU_ASSERT(data && !nsl_memcmp((const char*)data, letters, lettersSize));
    CU_ASSERT_EQUAL(dataSize, lettersSize);
    nsl_free(data);

    // try getting an unknown item
    err = NB_PersistentDataGet(persistentData, "test_aint_happening", &data, &dataSize);
    CU_ASSERT_EQUAL(err, NE_NOENT);
    CU_ASSERT_PTR_NULL(data);
    CU_ASSERT_EQUAL(dataSize, 0);

    // try changing an existing item and verify
    err = NB_PersistentDataSet(persistentData, digitsName, (uint8*)digitsReverse, digitsSize, FALSE, FALSE);
    CU_ASSERT_EQUAL(err, NE_OK);

    err = NB_PersistentDataGet(persistentData, digitsName, &data, &dataSize);
    CU_ASSERT_EQUAL(err, NE_OK);
    CU_ASSERT(data && !nsl_memcmp((const char*)data, digitsReverse, digitsSize));
    CU_ASSERT_EQUAL(dataSize, digitsSize);
    nsl_free(data);

    // serialize data items and save serialized data for another persistent data object instance
    err = NB_PersistentDataSerialize(persistentData, (const uint8**)&data, &dataSize);
    CU_ASSERT_EQUAL(err, NE_OK);

    // destroy persistent data (normally done with context)
    err = NB_PersistentDataDestroy(persistentData);
    CU_ASSERT_EQUAL(err, NE_OK);
    persistentData = 0;

    // create new persistent data object using previously serialized data and test
    callback.callback = TestPersistentDataChangedCallback;
    callback.callbackData = pal;
    err = NB_PersistentDataCreate(data, dataSize, &callback,  &persistentData);
    CU_ASSERT_EQUAL(err, NE_OK);

    // serialize data has been now been deserialized to the NB_PersistentData object, so free it
    nsl_free(data);
    data = 0;
    dataSize = 0;

    // verify previously serialized data is available and correct
    err = NB_PersistentDataGet(persistentData, digitsName, &data, &dataSize);
    CU_ASSERT_EQUAL(err, NE_OK);
    CU_ASSERT(data && !nsl_memcmp(data, digitsReverse, digitsSize));
    CU_ASSERT_EQUAL(dataSize, digitsSize);
    nsl_free(data);

    err = NB_PersistentDataGet(persistentData, lettersName, &data, &dataSize);
    CU_ASSERT_EQUAL(err, NE_OK);
    CU_ASSERT(data && !nsl_memcmp(data, letters, lettersSize));
    CU_ASSERT_EQUAL(dataSize, lettersSize);
    nsl_free(data);

    // add another data item and verify
    bytes = (uint8*)nsl_malloc(bytesSize);
    CU_ASSERT_PTR_NOT_NULL(bytes);
    for (n = 0; n < bytesSize; n++)
    {
        bytes[n] = (uint8)n;
    }

    err = NB_PersistentDataSet(persistentData, bytesName, bytes, bytesSize, FALSE, TRUE);
    CU_ASSERT_EQUAL(err, NE_OK);
    nsl_free(bytes);
    bytes = 0;

    err = NB_PersistentDataGet(persistentData, bytesName, &data, &dataSize);
    CU_ASSERT_EQUAL(err, NE_OK);
    CU_ASSERT_EQUAL(dataSize, bytesSize);

    for (n = 0; n < dataSize; n++)
    {
        if (data[n] != n)
        {
            break;
        }
    }
    CU_ASSERT_EQUAL(n, dataSize);
    nsl_free(data);

    (void)NB_PersistentDataDestroy(persistentData);

    DestroyContext(context);
    (void)PAL_Destroy(pal);

    // verify that the notification callback count is 1 for the last NB_PersistentDataSet call
    // that had notifyApplication set to TRUE
    CU_ASSERT_EQUAL(g_serializationNotifyCallbackCount, 1);
}


/*! Test persistent data master clear.

Test the master clear functionality of a persistent data object.

@return none
*/
void TestNetworkPersistentDataMasterClear(void)
{
    const char* clearableItem = "clearable";
    const char* unclearableItem = "unclearable";
    const char* digits = "0123456789";
    const nb_size digitsSize = 10;

    NB_Error err = NE_OK;
    PAL_Instance* pal = 0;
    NB_Context* context = 0;

    NB_PersistentData* persistentData = 0;

    uint8* data = 0;
    nb_size dataSize = 0;

    CU_ASSERT_EQUAL(CreatePalAndContext(&pal, &context), 1);
    CU_ASSERT_PTR_NOT_NULL(pal);
    CU_ASSERT_PTR_NOT_NULL(context);

    // create an empty persistent data object for testing only
    err = NB_PersistentDataCreate(NULL, 0, NULL, &persistentData);
    CU_ASSERT_EQUAL(err, NE_OK);
    CU_ASSERT_PTR_NOT_NULL(persistentData);

    // set a clearable and unclearable item
    err = NB_PersistentDataSet(persistentData, clearableItem, (uint8*)digits, digitsSize, FALSE, FALSE);
    CU_ASSERT_EQUAL(err, NE_OK);
    err = NB_PersistentDataSet(persistentData, unclearableItem, (uint8*)digits, digitsSize, TRUE, FALSE);
    CU_ASSERT_EQUAL(err, NE_OK);

    // do a master clear
    err = NB_PersistentDataMasterClear(persistentData);
    CU_ASSERT_EQUAL(err, NE_OK);

    // check that only unclearable item remains
    err = NB_PersistentDataGet(persistentData, unclearableItem, &data, &dataSize);
    CU_ASSERT_EQUAL(err, NE_OK);
    CU_ASSERT(data && !nsl_memcmp(data, digits, digitsSize));
    CU_ASSERT_EQUAL(dataSize, digitsSize);
    nsl_free(data);

    err = NB_PersistentDataGet(persistentData, clearableItem, &data, &dataSize);
    CU_ASSERT_EQUAL(err, NE_NOENT);
    CU_ASSERT_PTR_NULL(data);
    CU_ASSERT_EQUAL(dataSize, 0);
    nsl_free(data);

    (void)NB_PersistentDataDestroy(persistentData);

    DestroyContext(context);
    (void)PAL_Destroy(pal);
}

nb_boolean TestPersistentDataChangedCallback(NB_PersistentData* persistentData, void* userData)
{
    g_serializationNotifyCallbackCount++;
    return TRUE;
}


/*! Save persistent data to file.

All contexts created via CreateContext will include a persistent data object that will
be created with the content from ccc.bin, and will invoke this callback and save to ccc.bin
when there is a data change.

@return Non-zero if data successfully serialized and written to disk
*/
nb_boolean SavePersistentDataToFileCallback(NB_PersistentData* persistentData, void* userData)
{
    PAL_Instance* pal = (PAL_Instance*)userData;
    PAL_Error err = PAL_Ok;
    PAL_File* persistFile = 0;
    nb_boolean successfulSave = FALSE;

    // write out serialized persistent data to file
    err = PAL_FileOpen(pal, PERSISTENT_DATA_FILENAME, PFM_Create, &persistFile);
    if (!err)
    {
        NB_Error result = NE_OK;
        uint8* data = 0;
        nb_size dataSize = 0;
        uint32 dataWritten = 0;

        result = NB_PersistentDataSerialize(persistentData, (const uint8**)&data, &dataSize);
        CU_ASSERT_EQUAL(result, NE_OK);
        CU_ASSERT_PTR_NOT_NULL(data);

        if (!result)
        {
            err = PAL_FileWrite(persistFile, data, dataSize, &dataWritten);
            CU_ASSERT_EQUAL(err, PAL_Ok);
            CU_ASSERT_EQUAL(dataSize, (nb_size)dataWritten);

            err = PAL_FileClose(persistFile);
            CU_ASSERT_EQUAL(err, PAL_Ok);

            nsl_free(data);

            if (PAL_FileExists(pal, PERSISTENT_DATA_FILENAME) == PAL_Ok)
            {
                LOGOUTPUT(LogLevelHigh, (" modified persistent data saved to %s ... ", PERSISTENT_DATA_FILENAME));
                successfulSave = TRUE;
            }
        }
    }

    CU_ASSERT(successfulSave);
    return successfulSave;
}


NB_Error GeocodeRequest(NB_Context* context, const char* address, NB_GeocodeHandler* handler)
{
    NB_Error err = NE_OK;
    NB_GeocodeParameters* parameters = 0;

    err = NB_GeocodeParametersCreateFreeForm(context, address, "", 10, &parameters);
    CU_ASSERT_EQUAL(err, NE_OK);
    if (!err)
    {
        err = NB_GeocodeHandlerStartRequest(handler, parameters);
        (void)NB_GeocodeParametersDestroy(parameters);
    }

    return err;
}


NB_Error ReverseGeocodeRequest(NB_Context* context, NB_LatitudeLongitude* location, NB_ReverseGeocodeHandler* handler)
{
    NB_Error err = NE_OK;
    NB_ReverseGeocodeParameters* parameters = 0;

    err = NB_ReverseGeocodeParametersCreate(context, location, 10, &parameters);
    CU_ASSERT_EQUAL(err, NE_OK);
    if (!err)
    {
        err = NB_ReverseGeocodeHandlerStartRequest(handler, parameters);
        (void)NB_ReverseGeocodeParametersDestroy(parameters);
    }

    return err;
}

void ServerMessageCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* pUserData)
{
    if (!up)
    {
        TestNetworkCallbackData* callbackData = (TestNetworkCallbackData*)pUserData;

        if (status == NB_NetworkRequestStatus_Success)
        {
            AB_ServerMessageHandlerGetInformation((AB_ServerMessageHandler*)handler, &callbackData->message);
        }

        if (status != NB_NetworkRequestStatus_Progress)
        {
            SetCallbackCompletedEvent(callbackData->event);
        }
    }
}

void NoTemplatesCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* pUserData)
{
    if (!up)
    {
        TestNetworkCallbackData* callbackData = (TestNetworkCallbackData*)pUserData;

        if (status == NB_NetworkRequestStatus_Failed)
        {
            // Should get a NE_NOENT if the query can't be compiled
            callbackData->succeeded = (nb_boolean)(err == NE_NOENT);
        }

        if (status != NB_NetworkRequestStatus_Progress)
        {
            SetCallbackCompletedEvent(callbackData->event);
        }
    }
}

void TestNetworkGeocodeCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* pUserData)
{
    if (!up)
    {
        TestNetworkCallbackData* callbackData = (TestNetworkCallbackData*)pUserData;

        if (status != NB_NetworkRequestStatus_Progress)
        {
            callbackData->succeeded = (nb_boolean)(status == NB_NetworkRequestStatus_Success);
            SetCallbackCompletedEvent(callbackData->event);
        }
    }
}

void TestNetworkReverseGeocodeCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* pUserData)
{
    if (!up)
    {
        TestNetworkCallbackData* callbackData = (TestNetworkCallbackData*)pUserData;

        if (status != NB_NetworkRequestStatus_Progress)
        {
            callbackData->succeeded = (nb_boolean)(status == NB_NetworkRequestStatus_Success);
            SetCallbackCompletedEvent(callbackData->event);
        }
    }
}

/*! Get file data

! Important: Caller has to call free() once the data is no longer needed !

@return binary data
*/
PAL_Error GetFileData(PAL_Instance* pal, const char* filename, char** dataBuffer, size_t* dataBufferLength)
{
    if (dataBuffer == NULL || dataBufferLength == NULL)
    {
        return PAL_ErrBadParam;
    }

    return PAL_FileLoadFile(pal, filename, (unsigned char**)dataBuffer, (uint32*)dataBufferLength);
}


NB_NetworkConfiguration* GetNetworkConfiguration(PAL_Instance* pal, const char* credential, const char* hostname, NB_NetworkProtocol protocol, uint16 port, const char* tpsBuffer, size_t tpsBufferLength, const char* guidBuffer, size_t guidBufferLength)
{
    NB_NetworkOptionsDetailed options = { 0 };
    NB_NetworkConfiguration* configuration = NULL;
    char deviceName[50] = { 0 };

    GetDeviceName(pal, deviceName, sizeof(deviceName) / sizeof(deviceName[0]));

    options.adsAppId = "N/A";
    options.classId = 0;
    options.credential = credential;
    options.device = deviceName;
    options.domain = TestNetwork_GetDomain();
    options.firmwareVersion = "N/A";
    options.hostname = hostname;
    options.language = Test_OptionsGet()->currentLanguage;
    options.mdn = GetMobileDirectoryNumber(pal);
    options.min = GetMobileInformationNumber(pal);
    options.platformId = 0;
    options.port = port;
    options.priceType = 0;
    options.protocol = protocol;
    options.tpslibData = tpsBuffer;
    options.tpslibLength = tpsBufferLength;
    options.clientGuidData = guidBuffer;
    options.clientGuidLength = guidBufferLength;
    options.isEmulator = RunningInEmulator();

    (void)NB_NetworkConfigurationCreate(&options, &configuration);

    return configuration;
}

/*! Get persistent data object.

Create a persistent data object and initialize it with the serialized content of ccc.bin
from previous test runs.  Set data change callback to SavePersistentDataToFileCallback.

@return pointer to created and initialized persistent data change objecte
*/
NB_PersistentData* GetPersistentData(PAL_Instance* pal)
{
    NB_PersistentData* persistentData = 0;
    NB_PersistentDataChangedCallback callback = {0};

    uint8* serializedData = 0;
    size_t serializedDataSize = 0;

    if (PAL_FileExists(pal, PERSISTENT_DATA_FILENAME) == PAL_Ok)
    {
        CU_ASSERT_EQUAL(GetFileData(pal, PERSISTENT_DATA_FILENAME, (char**)&serializedData, &serializedDataSize), PAL_Ok);
    }

    callback.callback = SavePersistentDataToFileCallback;
    callback.callbackData = pal;
    CU_ASSERT_EQUAL(NB_PersistentDataCreate(serializedData, serializedDataSize, &callback, &persistentData), NE_OK);

    if (serializedData)
    {
        nsl_free(serializedData);
    }

    return persistentData;
}

nb_boolean CreateContext(PAL_Instance* pal, const char* credential, const char* hostname, NB_NetworkProtocol protocol, uint16 port, NB_Context** context)
{
    NB_Error error = NE_OK;
    NB_NetworkConfiguration* config = 0;
    NB_PersistentData* persistentData = 0;
    NB_CacheConfiguration voiceCacheConfig = {0};
    NB_CacheConfiguration rasterTileCacheConfig = {0};

    NB_NetworkNotifyEventCallback callback = { NetworkNotifyEventCallback, 0 };
    PAL_Error palErr = PAL_Ok;
    char* tpsData = 0;
    size_t tpsDataSize = 0;
    char* guidData = 0;
    size_t guidDataSize = 0;

    if (!pal || !credential || nsl_strlen(credential) == 0)
    {
        return FALSE;
    }

    // Voice cache settings
    voiceCacheConfig.cacheName                          = VOICE_CACHE_NAME;
    voiceCacheConfig.maximumItemsInMemoryCache          = VOICE_CACHE_MEMORY;
    voiceCacheConfig.maximumItemsInPersistentCache      = VOICE_CACHE_PERSISTENT;
    voiceCacheConfig.obfuscateKey                       = (byte*)VOICE_CACHE_OBFUSCATE_KEY;
    voiceCacheConfig.obfuscateKeySize                   = sizeof(VOICE_CACHE_OBFUSCATE_KEY);
    voiceCacheConfig.configuration                      = NULL;

    // Voice cache settings
    rasterTileCacheConfig.cacheName                     = RASTER_TILE_CACHE_NAME;
    rasterTileCacheConfig.maximumItemsInMemoryCache     = RASTERTILE_CACHE_MEMORY;
    rasterTileCacheConfig.maximumItemsInPersistentCache = RASTERTILE_CACHE_PERSISTENT;
    rasterTileCacheConfig.obfuscateKey                  = (byte*)RASTER_TILE_CACHE_OBFUSCATE_KEY;
    rasterTileCacheConfig.obfuscateKeySize              = sizeof(RASTER_TILE_CACHE_OBFUSCATE_KEY);
    rasterTileCacheConfig.configuration                 = NULL;

    palErr = GetFileData(pal, TestNetwork_GetTpslibFile(), &tpsData, &tpsDataSize);
    CU_ASSERT_EQUAL_FATAL(palErr, PAL_Ok);

    if (PAL_FileExists(pal, GUID_FILENAME) == PAL_Ok)
    {
        palErr = GetFileData(pal, GUID_FILENAME, &guidData, &guidDataSize);
        CU_ASSERT_EQUAL_FATAL(palErr, PAL_Ok);
    }

    config = GetNetworkConfiguration(pal, credential, hostname, protocol, port, tpsData, tpsDataSize, guidData, guidDataSize);
    CU_ASSERT_PTR_NOT_NULL(config);

    persistentData = GetPersistentData(pal);
    CU_ASSERT_PTR_NOT_NULL(persistentData);

    if (config && persistentData)
    {
        callback.callbackData = pal;

        // Create nbgm instance. Values taken from nbgu-ui-test
        NBGM_Config nbgmConfiguration = {0};
        nbgmConfiguration.dpi = 373.f;
        nbgmConfiguration.pal = pal;
        nbgmConfiguration.workSpace = NBGM_WORK_SPACE;
        nbgmConfiguration.nbmOffset = 0;
        nbgmConfiguration.useRoadBuildingVisibility = TRUE;
        nbgmConfiguration.preferredLanguageCode = 0;
        nbgmConfiguration.relativeCameraDistance = 111860800.0f;// camera height on zoom level 0, this value comes from Map Layering - Mapkit3D.xlsx.
        nbgmConfiguration.relativeZoomLevel = 0;
        nbgmConfiguration.metersPerPixelOnRelativeZoomLevel = 156543.034f;
        NBGM_Initialize(&nbgmConfiguration);

        error = NB_ContextCreate(pal, config, &callback, &voiceCacheConfig, &rasterTileCacheConfig, persistentData, context);
        CU_ASSERT_EQUAL_FATAL(error, PAL_Ok);
        CU_ASSERT_PTR_NOT_NULL(*context);

        if (*context)
        {
            AB_QaLogHeader header = { 0 };

            header.productName = "SysTests";
            header.productVersion = GetApplicationVersion();
            header.platformId = 1;                          /// @todo What should be used for platform id?
            header.mobileDirectoryNumber = GetMobileDirectoryNumber(pal);
            header.mobileInformationNumber = GetMobileInformationNumber(pal);

            error = AB_QaLogCreate(*context, &header, "SystemTests.qa", Test_OptionsGet()->verboseQaLog);
            CU_ASSERT_EQUAL(error, NE_OK);

            error = SetTargetMappings(*context);
            CU_ASSERT_EQUAL(error, NE_OK);

            error = NB_ContextInitializeGenericHttpDownloadManager(*context, TEST_HTTP_DOWNLOAD_CONNECTION_COUNT);
            CU_ASSERT_EQUAL(error, NE_OK);
        }

        error = NB_NetworkConfigurationDestroy(config);
        CU_ASSERT_EQUAL(error, NE_OK);
    }

    nsl_free(tpsData);
    if (guidData)
    {
        nsl_free(guidData);
    }

    return (*context) ? TRUE : FALSE;
}

void CreateContextCb(PAL_Instance* pal, void* userData)
{
    PAL_Error palError = PAL_Failed;
    nb_boolean rc = FALSE;
    NB_Error result = NE_OK;
    struct CreateContextCbInfo* info = (struct CreateContextCbInfo*)userData;
    NB_Context** context = info->context;

    palError = PAL_NetInitialize(pal);
    CU_ASSERT_EQUAL(rc, PAL_Ok);

    rc = CreateContext(pal, TestNetwork_GetToken(), TestNetwork_GetHostname(), TestNetwork_GetNetProtocol(), TestNetwork_GetPort(), context);
    CU_ASSERT_EQUAL(rc, TRUE);
    CU_ASSERT_PTR_NOT_NULL(*context);

    if (*context)
    {
        NB_GpsHistory* history = NULL;
        NB_VectorTileConfiguration config = {0};

        result = NB_GpsHistoryCreate(*context, 100 /* Max fixes */,
            5.5 /* m/s to consider heading valid */, &history);
        CU_ASSERT_PTR_NOT_NULL(history);
        CU_ASSERT_EQUAL(result, NE_OK);

        config.mapPasses = 8;
        config.mapZoomLevel = 16;
    }

    SetCallbackCompletedEvent(info->completeEvent);
}

uint8 CreatePalAndContext(PAL_Instance** pal, NB_Context** context)
{
    PAL_Error palError = PAL_Failed;
    NB_Error result = NE_OK;
    *pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(*pal);

    if (*pal)
    {
        nb_boolean rc = FALSE;

        palError = PAL_NetInitialize(*pal);
        CU_ASSERT_EQUAL(rc, PAL_Ok);

        rc = CreateContext(*pal, TestNetwork_GetToken(), TestNetwork_GetHostname(), TestNetwork_GetNetProtocol(), TestNetwork_GetPort(), context);
        CU_ASSERT_EQUAL(rc, TRUE);
        CU_ASSERT_PTR_NOT_NULL(*context);

        if (*context)
        {
            NB_GpsHistory* history = NULL;
            NB_VectorTileManager* manager = NULL;
            NB_VectorTileConfiguration config = {0};

            result = NB_ContextSetLocale(*context, "en-US"); //set "en-US" as default
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_GpsHistoryCreate(*context, 100 /* Max fixes */,
                5.5 /* m/s to consider heading valid */, &history);
            CU_ASSERT_PTR_NOT_NULL(history);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_ContextSetGpsHistoryNoOwnershipTransfer(*context, history);
            CU_ASSERT_EQUAL(result, NE_OK);

            config.mapPasses = 8;
            config.mapZoomLevel = 16;

            result = NB_VectorTileManagerCreate(*context, &config, NULL, &manager);
            CU_ASSERT_PTR_NOT_NULL(manager);
            CU_ASSERT_EQUAL(result, NE_OK);

            result = NB_ContextSetVectorTileManagerNoOwnershipTransfer(*context, manager);
            CU_ASSERT_EQUAL(result, NE_OK);
        }
    }

    if ((*pal) && !(*context))
    {
        PAL_NetShutdown(*pal);
        PAL_DestroyInstance(*pal);
        *pal = 0;
    }

    return (*pal) ? 1 : 0;
}

uint8 CreatePalAndContextMultiThread(PAL_Instance** pal, NB_Context** context)
{
    struct CreateContextCbInfo info = {0};
    *pal = PAL_CreateInstanceMultiThread();
    CU_ASSERT_PTR_NOT_NULL(*pal);

    info.context = context;
    info.completeEvent = CreateCallbackCompletedEvent();

    if (*pal)
    {
        TaskId tid = 0;
        PAL_EventTaskQueueAdd(*pal, CreateContextCb, &info, &tid);
        WaitForCallbackCompletedEvent(info.completeEvent, 2000);
    }

    if ((*pal) && !(*context))
    {
        PAL_NetShutdown(*pal);
        PAL_DestroyInstance(*pal);
        *pal = 0;
    }

    DestroyCallbackCompletedEvent(info.completeEvent);
    return (*pal) ? 1 : 0;
}
void DestroyContext(NB_Context* context)
{
    if (context != NULL)
    {
        NB_GpsHistory* history = NB_ContextGetGpsHistory(context);
        NB_VectorTileManager* manager = NB_ContextGetVectorTileManager(context);
        NB_Analytics* analytics = NB_ContextGetAnalytics(context);
        NB_WifiProbes* wifiProbes = NB_ContextGetWifiProbes(context);

        if (history != NULL)
        {
            (void)NB_GpsHistoryDestroy(history);
        }
        if (manager != NULL)
        {
            (void)NB_VectorTileManagerDestroy(manager);
        }
        if (analytics != NULL)
        {
            (void)NB_AnalyticsDestroy(analytics);
        }
        if (wifiProbes != NULL)
        {
            (void)NB_WifiProbesDestroy(wifiProbes);
        }

        (void)NB_ContextDestroy(context);
    }
}

const char* TestNetwork_GetToken()
{
    Test_Options* options = Test_OptionsGet();

    if (nsl_strlen(options->token) > 0)
    {
        return options->token;
    }

    if (options->networkType == TestNetworkHttp || options->networkType == TestNetworkHttps)
    {
        return DEFAULT_NETWORK_TOKEN_HTTP;
    }

    return DEFAULT_NETWORK_TOKEN_TCP;
}

const char* TestNetwork_GetDomain()
{
    char* domain = 0;

    Test_Options* options = Test_OptionsGet();
    switch (options->domain)
    {
    case TestNetworkDomainVerizon:
        domain = "mobile.vznavigator.net";
        break;

    case TestNetworkDomainDevLocal:
        domain = "dev.local";
        break;

    case TestNetworkDomainNavBuilder:
    default:
        domain = "navbuilder.nimlbs.net";
        break;
    }

    return domain;
}

NB_NetworkProtocol TestNetwork_GetNetProtocol()
{
    NB_NetworkProtocol protocol = NB_NetworkProtocol_TCP;
    switch (Test_OptionsGet()->networkType)
    {
    case TestNetworkTcp:
        protocol = NB_NetworkProtocol_TCP;
        break;
    case TestNetworkTcpTls:
        protocol = NB_NetworkProtocol_TCPTLS;
        break;
    case TestNetworkHttp:
        protocol = NB_NetworkProtocol_HTTP;
        break;
    case TestNetworkHttps:
        protocol = NB_NetworkProtocol_HTTPS;
        break;
    default:
        protocol = NB_NetworkProtocol_TCP;
        break;
    }

    return protocol;
}

uint16 TestNetwork_GetPort()
{
    uint16 port = 0;
    switch (Test_OptionsGet()->networkType)
    {
    case TestNetworkTcp:
        port = DEFAULT_PORT_TCP;
        break;
    case TestNetworkTcpTls:
        port = DEFAULT_PORT_TCP_TLS;
        break;
    case TestNetworkHttp:
        port = DEFAULT_PORT_HTTP;
        break;
    case TestNetworkHttps:
        port = DEFAULT_PORT_HTTPS;
        break;
    default:
        port = DEFAULT_PORT_TCP;
        break;
    }

    return port;
}

const char* TestNetwork_GetTpslibFile()
{
    Test_Options* options = Test_OptionsGet();
    if (nsl_strlen(options->tpslib) > 0)
    {
        return options->tpslib;
    }

    return TPSLIB_FILE;
}

const char* TestNetwork_GetHostname()
{
    Test_Options* options = Test_OptionsGet();
    if (nsl_strlen(options->hostname) > 0)
    {
        return options->hostname;
    }

    return 0;
}

void NetworkNotifyEventCallback(NB_NetworkNotifyEvent event, void* data, void* userData)
{
    int level = LogLevelMedium;

    if (event == NB_NetworkNotifyEvent_ClientGuidAssigned)
    {
        NB_NetworkNotifyEventClientGuid* guid = (NB_NetworkNotifyEventClientGuid*)data;
        PAL_Instance* pal = (PAL_Instance*)userData;
        PAL_File* file = 0;
        PAL_Error err = PAL_Ok;

        err = PAL_FileOpen(pal, GUID_FILENAME, PFM_Create, &file);
        if (!err)
        {
            uint32 written = 0;
            err = PAL_FileWrite(file, guid->guidData, guid->guidDataLength, &written);
            PAL_FileClose(file);
        }
    }

    if (g_notifyEventInfo)
    {
        switch (event)
        {
        case NB_NetworkNotifyEvent_ConnectionOpened:
            g_notifyEventInfo->openCount++;
            break;

        case NB_NetworkNotifyEvent_ConnectionClosed:
            g_notifyEventInfo->closeCount++;
            if (g_notifyEventInfo->closeEvent)
            {
                SetCallbackCompletedEvent(g_notifyEventInfo->closeEvent);
            }
            break;

        case NB_NetworkNotifyEvent_DataPending:
            if (*((nb_boolean*)data))
            {
                g_notifyEventInfo->dataPendingTrueCount++;
            }
            else
            {
                g_notifyEventInfo->dataPendingFalseCount++;
            }
            break;

        default:
            // No action
            break;
        }
    }

    if (Test_OptionsGet()->logLevel >= level)
    {
        char text[100] = { 0 };

        switch (event)
        {
        case NB_NetworkNotifyEvent_ConnectionClosed:
            nsl_strcpy(text, "Connection Closed");
            break;

        case NB_NetworkNotifyEvent_ConnectionOpened:
            nsl_strcpy(text, "Connection Opened");
            break;

        case NB_NetworkNotifyEvent_DataPending:
            nsl_sprintf(text, "DataPending - %s", *((nb_boolean*)data) ? "True" : "False");
            break;

        case NB_NetworkNotifyEvent_ClientGuidAssigned:
            nsl_strcpy(text, "GUID assigned");
            break;

        case NB_NetworkNotifyEvent_IdenError:
            {
                NB_NetworkNotifyEventError* error = (NB_NetworkNotifyEventError*)data;
                nsl_sprintf(text, "iden error - %d (%s)", error->code, error->description ? error->description : "?");
            }
            break;

        default:
            nsl_sprintf(text, "Unknown (%d)", event);
            break;
        }

        LOGOUTPUT(level, ("Network Notify Event: %s\n", text));
    }
}


void VerifyNotifyEventCounts(int openCount, int closeCount, int dataPendingTrueCount, int dataPendingFalseCount)
{
    CU_ASSERT_EQUAL(g_notifyEventInfo->openCount, openCount);
    CU_ASSERT_EQUAL(g_notifyEventInfo->closeCount, closeCount);
    CU_ASSERT_EQUAL(g_notifyEventInfo->dataPendingTrueCount, dataPendingTrueCount);
    CU_ASSERT_EQUAL(g_notifyEventInfo->dataPendingFalseCount, dataPendingFalseCount);
}


NB_Error SetTargetMappings(NB_Context* context)
{
    NB_Error err = NE_OK;

    if ((Test_OptionsGet()->carrier & TestCarrierVerizon) != 0)
    {
        NB_TargetMapping targets[] = {
            { "geocode",            "geocode,na_tt" },
            { "map",                "map,na_tt" },
            { "nav",                "nav,na_tt" },
            { "proxpoi",            "proxpoi,na_tt" },
            { "reverse-geocode",    "reverse-geocode,na_tt" }
        };

        err = NB_ContextSetTargetMappings(context, targets, sizeof(targets) / sizeof(targets[0]));
    }
    else
    {
        NB_TargetMapping targets[] = {
            { "geocode",            "geocode,global" },
            { "map",                "map,global" },
            { "nav",                "nav,global" },
            { "proxpoi",            "proxpoi,global" },
            { "reverse-geocode",    "reverse-geocode,global" },
            { "traffic-notify",     "traffic-notify,global" },
            { "vector-tile",        "vector-tile,global" }
        };

        err = NB_ContextSetTargetMappings(context, targets, sizeof(targets) / sizeof(targets[0]));
    }

    return err;
}

/*! @} */
