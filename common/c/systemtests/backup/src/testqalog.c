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

    @file     testqalog.c
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

#include "testqalog.h"
#include "testnetwork.h"
#include "main.h"
#include "palfile.h"
#include "nbqalog.h"
#include "nbnetworkoptions.h"
#include "abqaloghandler.h"
#include "abqaloginformation.h"
#include "abqalogparameters.h"

#define QALOG_SMALL_FILE  "qalog_small.qa"
#define QALOG_MEDIUM_FILE "qalog_medium.qa"
#define QALOG_LARGE_FILE  "qalog_large.qa"

static void QaLogHandlerCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* pUserData);
static NB_NetworkConfiguration* GetAdminNetworkConfiguration(PAL_Instance* pal, NB_NetworkProtocol protocol, uint16 port);
static void UploadFile(const char* filename);

static void TestQaLogTestClientLogFunctions(void);

static void TestQaLogVerifyFiles(void);
static void TestQaLogUploadSmallFile(void);
static void TestQaLogUploadMediumFile(void);
static void TestQaLogUploadLargeFile(void);
static void TestQaLogUploadStartTwice(void);
static void TestQaLogUploadCancelRequest(void);

static void* g_uploadEvent = 0;
static char* g_identifier = 0;

void TestQaLog_AddTests(CU_pSuite pTestSuite, int level)
{
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestQaLogTestClientLogFunctions", TestQaLogTestClientLogFunctions);

    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestQaLogVerifyFiles", TestQaLogVerifyFiles);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestQaLogUploadSmallFile", TestQaLogUploadSmallFile);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestQaLogUploadMediumFile", TestQaLogUploadMediumFile);
    ADD_TEST(level, TestLevelFull, pTestSuite, "TestQaLogUploadLargeFile", TestQaLogUploadLargeFile);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestQaLogUploadStartTwice", TestQaLogUploadStartTwice);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestQaLogUploadCancelRequest", TestQaLogUploadCancelRequest);
};


int TestQaLog_SuiteSetup()
{
    g_uploadEvent = CreateCallbackCompletedEvent();
	return 0;
}


int TestQaLog_SuiteCleanup()
{
    DestroyCallbackCompletedEvent(g_uploadEvent);
	return 0;
}

void TestQaLogTestClientLogFunctions()
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        NB_QaLogKey(context, 65);
        NB_QaLogApp2AppCommand(context, "TEST App2App command log 0123456789ABCDEFGIJKLMNOPQRSTUVWXYZ");

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}

void TestQaLogVerifyFiles(void)
{
    PAL_Instance* pal = PAL_CreateInstance();
    CU_ASSERT_PTR_NOT_NULL(pal);

    CU_ASSERT_EQUAL(PAL_FileExists(pal, QALOG_SMALL_FILE), PAL_Ok);
    CU_ASSERT_EQUAL(PAL_FileExists(pal, QALOG_MEDIUM_FILE), PAL_Ok);
    CU_ASSERT_EQUAL(PAL_FileExists(pal, QALOG_LARGE_FILE), PAL_Ok);

    PAL_DestroyInstance(pal);
}

void TestQaLogUploadSmallFile(void)
{
    UploadFile(QALOG_SMALL_FILE);
}

void TestQaLogUploadMediumFile(void)
{
    UploadFile(QALOG_MEDIUM_FILE);
}

void TestQaLogUploadLargeFile(void)
{
    UploadFile(QALOG_LARGE_FILE);
}

void TestQaLogUploadStartTwice(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        NB_Error err = NE_OK;
        AB_QaLogParameters* parameters = 0;
        NB_NetworkProtocol protocol = TestNetwork_GetNetProtocol();
        uint16 port = TestNetwork_GetPort();
        NB_NetworkConfiguration* adminConfig = GetAdminNetworkConfiguration(pal, protocol, port);
        CU_ASSERT_PTR_NOT_NULL(adminConfig);

        err = AB_QaLogParametersCreate(context, QALOG_SMALL_FILE, adminConfig, &parameters);
        CU_ASSERT_EQUAL(err, NE_OK);
        if (!err)
        {
            AB_QaLogHandler* handler = 0;

            err = AB_QaLogHandlerCreate(context, NULL, &handler);
            CU_ASSERT_EQUAL(err, NE_OK);
            if (!err)
            {
                err = AB_QaLogHandlerStartRequest(handler, parameters);
                CU_ASSERT_EQUAL(err, NE_OK);

                err = AB_QaLogHandlerStartRequest(handler, parameters);
                CU_ASSERT_EQUAL(err, NE_BUSY);

                AB_QaLogHandlerDestroy(handler);
            }

            AB_QaLogParametersDestroy(parameters);
        }

        NB_NetworkConfigurationDestroy(adminConfig);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}

void TestQaLogUploadCancelRequest(void)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        NB_Error err = NE_OK;
        AB_QaLogParameters* parameters = 0;
        NB_NetworkProtocol protocol = TestNetwork_GetNetProtocol();
        uint16 port = TestNetwork_GetPort();
        NB_NetworkConfiguration* adminConfig = GetAdminNetworkConfiguration(pal, protocol, port);
        CU_ASSERT_PTR_NOT_NULL(adminConfig);

        err = AB_QaLogParametersCreate(context, QALOG_SMALL_FILE, adminConfig, &parameters);
        CU_ASSERT_EQUAL(err, NE_OK);
        if (!err)
        {
            AB_QaLogHandler* handler = 0;

            err = AB_QaLogHandlerCreate(context, NULL, &handler);
            CU_ASSERT_EQUAL(err, NE_OK);
            if (!err)
            {
                // Should be unexpected at this point
                err = AB_QaLogHandlerCancelRequest(handler);
                CU_ASSERT_EQUAL(err, NE_UNEXPECTED);

                err = AB_QaLogHandlerStartRequest(handler, parameters);
                CU_ASSERT_EQUAL(err, NE_OK);

                // Should be OK now that the request has been started
                err = AB_QaLogHandlerCancelRequest(handler);
                CU_ASSERT_EQUAL(err, NE_OK);

                // Should be unexpected again
                err = AB_QaLogHandlerCancelRequest(handler);
                CU_ASSERT_EQUAL(err, NE_UNEXPECTED);

                AB_QaLogHandlerDestroy(handler);
            }

            AB_QaLogParametersDestroy(parameters);
        }

        NB_NetworkConfigurationDestroy(adminConfig);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}


void UploadFile(const char* filename)
{
    PAL_Instance* pal = 0;
    NB_Context* context = 0;
    uint8 rc = CreatePalAndContext(&pal, &context);
    if (rc)
    {
        NB_Error err = NE_OK;
        AB_QaLogParameters* parameters = 0;
        NB_NetworkProtocol protocol = TestNetwork_GetNetProtocol();
        uint16 port = TestNetwork_GetPort();
        NB_NetworkConfiguration* adminConfig = GetAdminNetworkConfiguration(pal, protocol, port);
        CU_ASSERT_PTR_NOT_NULL(adminConfig);

        err = AB_QaLogParametersCreate(context, filename, adminConfig, &parameters);
        CU_ASSERT_EQUAL(err, NE_OK);
        if (!err)
        {
            AB_QaLogHandler* handler = 0;
            NB_RequestHandlerCallback callback = { QaLogHandlerCallback, 0 };

            err = AB_QaLogHandlerCreate(context, &callback, &handler);
            CU_ASSERT_EQUAL(err, NE_OK);
            if (!err)
            {
                err = AB_QaLogHandlerStartRequest(handler, parameters);
                CU_ASSERT_EQUAL(err, NE_OK);

                if (!err)
                {
                    // Wait a full 2 minutes as it can take a long time for a big qa file to upload
                    CU_ASSERT(WaitForCallbackCompletedEvent(g_uploadEvent, 120000));
                    CU_ASSERT_PTR_NOT_NULL(g_identifier);

                    if (g_identifier)
                    {
                        nsl_free(g_identifier);
                    }
                }

                AB_QaLogHandlerDestroy(handler);
            }

            AB_QaLogParametersDestroy(parameters);
        }

        NB_NetworkConfigurationDestroy(adminConfig);

        DestroyContext(context);
        PAL_DestroyInstance(pal);
    }
}


NB_NetworkConfiguration* GetAdminNetworkConfiguration(PAL_Instance* pal, NB_NetworkProtocol protocol, uint16 port)
{
    NB_NetworkOptionsDetailed options = { 0 };
    NB_NetworkConfiguration* configuration = NULL;
    char* tpsBuffer = 0;
    uint32 tpsBufferLength = 0;
    PAL_Error err = PAL_Ok;
    char deviceName[50] = { 0 };

    GetDeviceName(pal, deviceName, sizeof(deviceName) / sizeof(deviceName[0]));

    err = PlatformLoadFile(pal, "admintesla.tpl", (unsigned char**)&tpsBuffer, &tpsBufferLength);
    if (!err)
    {
        options.adsAppId = "N/A";
        options.classId = 0;
        options.credential = "EPVSQrhmn2zvyMvfAH1YChO3mi2RcQB2drzC/KOG";
        options.device = deviceName;
        options.hostname = "qalog";
        options.domain = "nimupdate.com";
        options.firmwareVersion = "N/A";
        options.language = Test_OptionsGet()->currentLanguage;
        options.mdn = GetMobileDirectoryNumber(pal);
        options.min = 123;
        options.platformId = 0;
        options.port = port;
        options.priceType = 0;
        options.protocol = protocol;
        options.tpslibData = tpsBuffer;
        options.tpslibLength = tpsBufferLength;

        (void)NB_NetworkConfigurationCreate(&options, &configuration);

        nsl_free(tpsBuffer);
    }

    return configuration;
}


void QaLogHandlerCallback(void* handler, NB_NetworkRequestStatus status, NB_Error err, uint8 up, int percent, void* userData)
{
    LOGOUTPUT(LogLevelHigh, ("QA Log %s progress: %d\n", up ? "upload" : "download", percent));

    if (!up)
    {
        if (err) 
        {
            CU_FAIL("Callback returned error");
            SetCallbackCompletedEvent(g_uploadEvent);
        }
        else if (percent == 100) 
        {
            AB_QaLogInformation* information = 0;
            NB_Error err = NE_OK;
            
            err = AB_QaLogHandlerGetInformation(handler, &information);
            CU_ASSERT_EQUAL(err, NE_OK);

            if (!err)
            {
                err = AB_QaLogInformationGetIdentifier(information, &g_identifier);
                CU_ASSERT_EQUAL(err, NE_OK);

                LOGOUTPUT(LogLevelHigh, ("QA Log Uploaded, id = %s\n", g_identifier));

                (void)AB_QaLogInformationDestroy(information);
            }

            SetCallbackCompletedEvent(g_uploadEvent);
        }
    }
}


/*! @} */
