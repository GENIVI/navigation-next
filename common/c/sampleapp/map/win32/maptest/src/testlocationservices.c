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

    @file     testlocationservices.c
    @date     05/09/2011
    @defgroup TestLocationServices_h System Tests for Location functions

*/
/*
    See description in header file.

    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "testlocationservices.h"
#include "testnetwork.h"
#include "main.h"
#include "nbcontext.h"
#include "nblocationconnector.h"
#include "abpalgps.h"

#define NUMBER_OF_FIXES 5
#define GPS_FILENAME    "todowntown.bgp"
#define CALLBACK_TIMEOUT 30000

static void TestLocationServicesContextCreate(void);
static void TestLocationServicesContextCreateWithoutContext(void);
static void TestLocationServicesContextDestroy(void);
static void TestLocationServicesContextDestroyWithoutContext(void);
static void TestLocationServicesSetPalAndContext(void);
static void TestLocationServicesSetPalAndContextWithoutContext(void);
static void TestLocationServicesSetPalAndContextWithoutPal(void);
static void TestLocationServicesSetPalAndContextWithoutNbContext(void);
static void TestLocationServicesGetOneFix(void);
static void TestLocationServicesGetOneFixWithoutContext(void);
static void TestLocationServicesGetOneFixWithoutNbContext(void);
static void TestLocationServicesGetOneFixWithIncorrectFixType(void);
static void TestLocationServicesCancelGetOneFix(void);
static void TestLocationServicesCancelGetOneFixWithoutContext(void);
static void TestLocationServicesGetState(void);
static void TestLocationServicesGetStateWithoutContext(void);
static void TestLocationServicesGetStateWithoutState(void);
static void TestLocationServicesStartReceivingFixes(void);
static void TestLocationServicesStartReceivingFixesWithoutContext(void);
static void TestLocationServicesStartReceivingFixesWithoutNbContext(void);
static void TestLocationServicesStopReceivingFixes(void);
static void TestLocationServicesStopReceivingFixesWithoutContext(void);
static void TestLocationServicesInitialize(void);
static void TestLocationServicesInitializeWithoutContext(void);
static void TestLocationServicesInitializeWithoutConfig(void);
static void TestLocationServicesInitializeWithoutGpsConfig(void);
static void TestLocationServicesResetCache(void);
static void TestLocationServicesResetCacheWithoutContext(void);


static void LocationOneFixCallback(void* appData, NB_LocationContext* context, const NB_LS_Location* location, NB_Error error);
static void LocationSeveralFixesCallback(void* appData, NB_LocationContext* context, const NB_LS_Location* location, NB_Error error);

typedef struct TestLocationServicesCallbackData
{
    void*                   event;            /*!< Event to signal when callback complete */
    NB_Context*             context;
    NB_LocationContext*     locationContext;
    uint8                   countOfFixes;
} TestLocationServicesCallbackData;

/*! Add all your test functions here

@return None
*/
void
TestLocationServices_AddTests( CU_pSuite pTestSuite, int level )
{
    // ! Add all your function names here !

    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesContextCreate", TestLocationServicesContextCreate);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesContextCreateWithoutContext", TestLocationServicesContextCreateWithoutContext);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesContextDestroy", TestLocationServicesContextDestroy);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesContextDestroyWithoutContext", TestLocationServicesContextDestroyWithoutContext);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesSetPalAndContext", TestLocationServicesSetPalAndContext);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesSetPalAndContextWithoutContext", TestLocationServicesSetPalAndContextWithoutContext);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesSetPalAndContextWithoutPal", TestLocationServicesSetPalAndContextWithoutPal);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesSetPalAndContextWithoutNbContext", TestLocationServicesSetPalAndContextWithoutNbContext);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesGetOneFix", TestLocationServicesGetOneFix);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesGetOneFixWithoutContext", TestLocationServicesGetOneFixWithoutContext);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesGetOneFixWithoutNbContext", TestLocationServicesGetOneFixWithoutNbContext);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesGetOneFixWithIncorrectFixType", TestLocationServicesGetOneFixWithIncorrectFixType);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesCancelGetOneFix", TestLocationServicesCancelGetOneFix);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesCancelGetOneFixWithoutContext", TestLocationServicesCancelGetOneFixWithoutContext);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesGetState", TestLocationServicesGetState);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesGetStateWithoutContext", TestLocationServicesGetStateWithoutContext);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesGetStateWithoutState", TestLocationServicesGetStateWithoutState);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesStartReceivingFixes", TestLocationServicesStartReceivingFixes);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesStartReceivingFixesWithoutContext", TestLocationServicesStartReceivingFixesWithoutContext);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesStartReceivingFixesWithoutNbContext", TestLocationServicesStartReceivingFixesWithoutNbContext);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesStopReceivingFixes", TestLocationServicesStopReceivingFixes);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesStopReceivingFixesWithoutContext", TestLocationServicesStopReceivingFixesWithoutContext);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesInitialize", TestLocationServicesInitialize);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesInitializeWithoutContext", TestLocationServicesInitializeWithoutContext);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesInitializeWithoutConfig", TestLocationServicesInitializeWithoutConfig);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesInitializeWithoutGpsConfig", TestLocationServicesInitializeWithoutGpsConfig);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesResetCache", TestLocationServicesResetCache);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationServicesResetCacheWithoutContext", TestLocationServicesResetCacheWithoutContext);
};


/*! Add common initialization code here.

@return 0

@see TestNetwork_SuiteCleanup
*/
int
TestLocationServices_SuiteSetup()
{
    return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestNetwork_SuiteSetup
*/
int
TestLocationServices_SuiteCleanup()
{
    return 0;
}

void
TestLocationServicesContextCreate(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        NB_LocationContextDestroy(callbackData->locationContext);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesContextCreateWithoutContext(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        err = NB_LocationContextCreate(NULL);
        CU_ASSERT_EQUAL(err, NE_INVAL);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesContextDestroy(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        NB_LocationContextDestroy(callbackData->locationContext);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesContextDestroyWithoutContext(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        NB_LocationContextDestroy(callbackData->locationContext);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesSetPalAndContext(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationSetPalAndContext(callbackData->locationContext, pal, callbackData->context);
        CU_ASSERT_EQUAL(err, NE_OK);

        NB_LocationContextDestroy(callbackData->locationContext);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesSetPalAndContextWithoutContext(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationSetPalAndContext(NULL, pal, callbackData->context);
        CU_ASSERT_EQUAL(err, NE_INVAL);

        NB_LocationContextDestroy(callbackData->locationContext);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesSetPalAndContextWithoutPal(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationSetPalAndContext(callbackData->locationContext, NULL, callbackData->context);
        CU_ASSERT_EQUAL(err, NE_INVAL);

        NB_LocationContextDestroy(callbackData->locationContext);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesSetPalAndContextWithoutNbContext(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationSetPalAndContext(callbackData->locationContext, pal, NULL);
        CU_ASSERT_EQUAL(err, NE_INVAL);

        NB_LocationContextDestroy(callbackData->locationContext);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesGetOneFix(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        NB_LS_LocationConfig config = {0};
        ABPAL_GpsConfig      gpsConfig = {0};
        uint32               gpsConfigCount = 0;

        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationSetPalAndContext(callbackData->locationContext, pal, callbackData->context);
        CU_ASSERT_EQUAL(err, NE_OK);

        config.emulationMode = TRUE;
        nsl_strcpy(config.emulationFilename, GPS_FILENAME);

        callbackData->event = CreateCallbackCompletedEvent();

        err = NB_LocationInitialize(callbackData->locationContext, &config, &gpsConfig, gpsConfigCount,
            NULL, NULL, 3, callbackData);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationGetOneFix(callbackData->locationContext, callbackData->context, LocationOneFixCallback, NBFixType_Fast, callbackData);
        CU_ASSERT_EQUAL(err, NE_OK);

        CU_ASSERT(WaitForCallbackCompletedEvent(callbackData->event, CALLBACK_TIMEOUT))

        NB_LocationContextDestroy(callbackData->locationContext);
        DestroyCallbackCompletedEvent(callbackData->event);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesGetOneFixWithoutContext(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        NB_LS_LocationConfig config = {0};
        ABPAL_GpsConfig      gpsConfig = {0};
        uint32               gpsConfigCount = 0;

        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationSetPalAndContext(callbackData->locationContext, pal, callbackData->context);
        CU_ASSERT_EQUAL(err, NE_OK);

        config.emulationMode = TRUE;
        nsl_strcpy(config.emulationFilename, GPS_FILENAME);

        callbackData->event = CreateCallbackCompletedEvent();

        err = NB_LocationInitialize(callbackData->locationContext, &config, &gpsConfig, gpsConfigCount,
            NULL, NULL, 3, callbackData);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationGetOneFix(NULL, callbackData->context, LocationOneFixCallback, NBFixType_Fast, callbackData);
        CU_ASSERT_EQUAL(err, NE_INVAL);

        NB_LocationContextDestroy(callbackData->locationContext);
        DestroyCallbackCompletedEvent(callbackData->event);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesGetOneFixWithoutNbContext(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        NB_LS_LocationConfig config = {0};
        ABPAL_GpsConfig      gpsConfig = {0};
        uint32               gpsConfigCount = 0;

        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationSetPalAndContext(callbackData->locationContext, pal, callbackData->context);
        CU_ASSERT_EQUAL(err, NE_OK);

        config.emulationMode = TRUE;
        nsl_strcpy(config.emulationFilename, GPS_FILENAME);

        callbackData->event = CreateCallbackCompletedEvent();

        err = NB_LocationInitialize(callbackData->locationContext, &config, &gpsConfig, gpsConfigCount,
            NULL, NULL, 3, callbackData);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationGetOneFix(callbackData->locationContext, NULL, LocationOneFixCallback, NBFixType_Fast, callbackData);
        CU_ASSERT_EQUAL(err, NE_INVAL);

        NB_LocationContextDestroy(callbackData->locationContext);
        DestroyCallbackCompletedEvent(callbackData->event);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesGetOneFixWithIncorrectFixType(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        NB_LS_LocationConfig config = {0};
        ABPAL_GpsConfig      gpsConfig = {0};
        uint32               gpsConfigCount = 0;

        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationSetPalAndContext(callbackData->locationContext, pal, callbackData->context);
        CU_ASSERT_EQUAL(err, NE_OK);

        config.emulationMode = TRUE;
        nsl_strcpy(config.emulationFilename, GPS_FILENAME);

        callbackData->event = CreateCallbackCompletedEvent();

        err = NB_LocationInitialize(callbackData->locationContext, &config, &gpsConfig, gpsConfigCount,
            NULL, NULL, 3, callbackData);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationGetOneFix(callbackData->locationContext, callbackData->context, LocationOneFixCallback, 1024, callbackData);
        CU_ASSERT_EQUAL(err, NE_INVAL);

        NB_LocationContextDestroy(callbackData->locationContext);
        DestroyCallbackCompletedEvent(callbackData->event);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesCancelGetOneFix(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        NB_LS_LocationConfig config = {0};
        ABPAL_GpsConfig      gpsConfig = {0};
        uint32               gpsConfigCount = 0;

        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationSetPalAndContext(callbackData->locationContext, pal, callbackData->context);
        CU_ASSERT_EQUAL(err, NE_OK);

        config.emulationMode = TRUE;
        nsl_strcpy(config.emulationFilename, GPS_FILENAME);

        callbackData->event = CreateCallbackCompletedEvent();

        err = NB_LocationInitialize(callbackData->locationContext, &config, &gpsConfig, gpsConfigCount,
            NULL, NULL, 3, callbackData);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationGetOneFix(callbackData->locationContext, callbackData->context, LocationOneFixCallback, NBFixType_Fast, callbackData);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationCancelGetOneFix(callbackData->locationContext, LocationOneFixCallback, callbackData);
        CU_ASSERT_EQUAL(err, NE_OK);

        NB_LocationContextDestroy(callbackData->locationContext);
        DestroyCallbackCompletedEvent(callbackData->event);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesCancelGetOneFixWithoutContext(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        NB_LS_LocationConfig config = {0};
        ABPAL_GpsConfig      gpsConfig = {0};
        uint32               gpsConfigCount = 0;

        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationSetPalAndContext(callbackData->locationContext, pal, callbackData->context);
        CU_ASSERT_EQUAL(err, NE_OK);

        config.emulationMode = TRUE;
        nsl_strcpy(config.emulationFilename, GPS_FILENAME);

        callbackData->event = CreateCallbackCompletedEvent();

        err = NB_LocationInitialize(callbackData->locationContext, &config, &gpsConfig, gpsConfigCount,
            NULL, NULL, 3, callbackData);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationGetOneFix(callbackData->locationContext, callbackData->context, LocationOneFixCallback, NBFixType_Fast, callbackData);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationCancelGetOneFix(NULL, LocationOneFixCallback, callbackData);
        CU_ASSERT_EQUAL(err, NE_INVAL);

        NB_LocationContextDestroy(callbackData->locationContext);
        DestroyCallbackCompletedEvent(callbackData->event);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesGetState(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    NB_LS_LocationState state = NBPGS_Undefined;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        NB_LS_LocationConfig config = {0};
        ABPAL_GpsConfig      gpsConfig = {0};
        uint32               gpsConfigCount = 0;

        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationSetPalAndContext(callbackData->locationContext, pal, callbackData->context);
        CU_ASSERT_EQUAL(err, NE_OK);

        config.emulationMode = TRUE;
        nsl_strcpy(config.emulationFilename, GPS_FILENAME);

        callbackData->event = CreateCallbackCompletedEvent();

        err = NB_LocationGetState(callbackData->locationContext, &state);
        CU_ASSERT_EQUAL(err, NE_OK);
        CU_ASSERT_EQUAL(state, NBPGS_Undefined);

        err = NB_LocationInitialize(callbackData->locationContext, &config, &gpsConfig, gpsConfigCount,
            NULL, NULL, 3, callbackData);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationGetState(callbackData->locationContext, &state);
        CU_ASSERT_EQUAL(err, NE_OK);
        CU_ASSERT_EQUAL(state, NBPGS_Initialized);

        err = NB_LocationGetOneFix(callbackData->locationContext, callbackData->context, LocationOneFixCallback, NBFixType_Fast, callbackData);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationGetState(callbackData->locationContext, &state);
        CU_ASSERT_EQUAL(err, NE_OK);
        CU_ASSERT_EQUAL(state, NBPGS_Oneshot);

        CU_ASSERT(WaitForCallbackCompletedEvent(callbackData->event, CALLBACK_TIMEOUT))

        err = NB_LocationGetState(callbackData->locationContext, &state);
        CU_ASSERT_EQUAL(err, NE_OK);
        CU_ASSERT_EQUAL(state, NBPGS_Initialized);

        NB_LocationContextDestroy(callbackData->locationContext);
        DestroyCallbackCompletedEvent(callbackData->event);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesGetStateWithoutContext(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    NB_LS_LocationState state = NBPGS_Undefined;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationGetState(NULL, &state);
        CU_ASSERT_EQUAL(err, NE_INVAL);
        CU_ASSERT_EQUAL(state, NBPGS_Undefined);

        NB_LocationContextDestroy(callbackData->locationContext);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesGetStateWithoutState(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationGetState(callbackData->locationContext, NULL);
        CU_ASSERT_EQUAL(err, NE_INVAL);

        NB_LocationContextDestroy(callbackData->locationContext);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesStartReceivingFixes(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        NB_LS_LocationConfig config = {0};
        ABPAL_GpsConfig      gpsConfig = {0};
        uint32               gpsConfigCount = 0;

        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationSetPalAndContext(callbackData->locationContext, pal, callbackData->context);
        CU_ASSERT_EQUAL(err, NE_OK);

        config.emulationMode = TRUE;
        nsl_strcpy(config.emulationFilename, GPS_FILENAME);

        callbackData->event = CreateCallbackCompletedEvent();

        err = NB_LocationInitialize(callbackData->locationContext, &config, &gpsConfig, gpsConfigCount,
            NULL, NULL, 3, callbackData);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationStartReceivingFixes(callbackData->locationContext, callbackData->context, LocationSeveralFixesCallback, callbackData);
        CU_ASSERT_EQUAL(err, NE_OK);

        CU_ASSERT(WaitForCallbackCompletedEvent(callbackData->event, CALLBACK_TIMEOUT))

        DestroyCallbackCompletedEvent(callbackData->event);
        NB_LocationContextDestroy(callbackData->locationContext);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesStartReceivingFixesWithoutContext(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        NB_LS_LocationConfig config = {0};
        ABPAL_GpsConfig      gpsConfig = {0};
        uint32               gpsConfigCount = 0;

        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationSetPalAndContext(callbackData->locationContext, pal, callbackData->context);
        CU_ASSERT_EQUAL(err, NE_OK);

        config.emulationMode = TRUE;
        nsl_strcpy(config.emulationFilename, GPS_FILENAME);

        callbackData->event = CreateCallbackCompletedEvent();

        err = NB_LocationInitialize(callbackData->locationContext, &config, &gpsConfig, gpsConfigCount,
            NULL, NULL, 3, callbackData);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationStartReceivingFixes(NULL, callbackData->context, LocationSeveralFixesCallback, callbackData);
        CU_ASSERT_EQUAL(err, NE_INVAL);

        DestroyCallbackCompletedEvent(callbackData->event);
        NB_LocationContextDestroy(callbackData->locationContext);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesStartReceivingFixesWithoutNbContext(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        NB_LS_LocationConfig config = {0};
        ABPAL_GpsConfig      gpsConfig = {0};
        uint32               gpsConfigCount = 0;

        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationSetPalAndContext(callbackData->locationContext, pal, callbackData->context);
        CU_ASSERT_EQUAL(err, NE_OK);

        config.emulationMode = TRUE;
        nsl_strcpy(config.emulationFilename, GPS_FILENAME);

        callbackData->event = CreateCallbackCompletedEvent();

        err = NB_LocationInitialize(callbackData->locationContext, &config, &gpsConfig, gpsConfigCount,
            NULL, NULL, 3, callbackData);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationStartReceivingFixes(callbackData->locationContext, NULL, LocationSeveralFixesCallback, callbackData);
        CU_ASSERT_EQUAL(err, NE_INVAL);

        DestroyCallbackCompletedEvent(callbackData->event);
        NB_LocationContextDestroy(callbackData->locationContext);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesStopReceivingFixes(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        NB_LS_LocationConfig config = {0};
        ABPAL_GpsConfig      gpsConfig = {0};
        uint32               gpsConfigCount = 0;

        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationSetPalAndContext(callbackData->locationContext, pal, callbackData->context);
        CU_ASSERT_EQUAL(err, NE_OK);

        config.emulationMode = TRUE;
        nsl_strcpy(config.emulationFilename, GPS_FILENAME);

        callbackData->event = CreateCallbackCompletedEvent();

        err = NB_LocationInitialize(callbackData->locationContext, &config, &gpsConfig, gpsConfigCount,
            NULL, NULL, 3, callbackData);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationStartReceivingFixes(callbackData->locationContext, callbackData->context, LocationSeveralFixesCallback, callbackData);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationStopReceivingFixes(callbackData->locationContext, LocationSeveralFixesCallback, callbackData);
        CU_ASSERT_EQUAL(err, NE_OK);

        DestroyCallbackCompletedEvent(callbackData->event);
        NB_LocationContextDestroy(callbackData->locationContext);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesStopReceivingFixesWithoutContext(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        NB_LS_LocationConfig config = {0};
        ABPAL_GpsConfig      gpsConfig = {0};
        uint32               gpsConfigCount = 0;

        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationSetPalAndContext(callbackData->locationContext, pal, callbackData->context);
        CU_ASSERT_EQUAL(err, NE_OK);

        config.emulationMode = TRUE;
        nsl_strcpy(config.emulationFilename, GPS_FILENAME);

        callbackData->event = CreateCallbackCompletedEvent();

        err = NB_LocationInitialize(callbackData->locationContext, &config, &gpsConfig, gpsConfigCount,
            NULL, NULL, 3, callbackData);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationStartReceivingFixes(callbackData->locationContext, callbackData->context, LocationSeveralFixesCallback, callbackData);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationStopReceivingFixes(NULL, LocationSeveralFixesCallback, callbackData);
        CU_ASSERT_EQUAL(err, NE_INVAL);

        DestroyCallbackCompletedEvent(callbackData->event);
        NB_LocationContextDestroy(callbackData->locationContext);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesInitialize(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        NB_LS_LocationConfig config = {0};
        ABPAL_GpsConfig      gpsConfig = {0};
        uint32               gpsConfigCount = 0;

        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationSetPalAndContext(callbackData->locationContext, pal, callbackData->context);
        CU_ASSERT_EQUAL(err, NE_OK);

        config.emulationMode = TRUE;
        nsl_strcpy(config.emulationFilename, GPS_FILENAME);

        callbackData->event = CreateCallbackCompletedEvent();

        err = NB_LocationInitialize(callbackData->locationContext, &config, &gpsConfig, gpsConfigCount,
            NULL, NULL, 3, callbackData);
        CU_ASSERT_EQUAL(err, NE_OK);

        NB_LocationContextDestroy(callbackData->locationContext);
        DestroyCallbackCompletedEvent(callbackData->event);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesInitializeWithoutContext(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        NB_LS_LocationConfig config = {0};
        ABPAL_GpsConfig      gpsConfig = {0};
        uint32               gpsConfigCount = 0;

        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationSetPalAndContext(callbackData->locationContext, pal, callbackData->context);
        CU_ASSERT_EQUAL(err, NE_OK);

        config.emulationMode = TRUE;
        nsl_strcpy(config.emulationFilename, GPS_FILENAME);

        callbackData->event = CreateCallbackCompletedEvent();

        err = NB_LocationInitialize(NULL, &config, &gpsConfig, gpsConfigCount,
            NULL, NULL, 3, callbackData);
        CU_ASSERT_EQUAL(err, NE_INVAL);

        NB_LocationContextDestroy(callbackData->locationContext);
        DestroyCallbackCompletedEvent(callbackData->event);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesInitializeWithoutConfig(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        NB_LS_LocationConfig config = {0};
        ABPAL_GpsConfig      gpsConfig = {0};
        uint32               gpsConfigCount = 0;

        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationSetPalAndContext(callbackData->locationContext, pal, callbackData->context);
        CU_ASSERT_EQUAL(err, NE_OK);

        config.emulationMode = TRUE;
        nsl_strcpy(config.emulationFilename, GPS_FILENAME);

        callbackData->event = CreateCallbackCompletedEvent();

        err = NB_LocationInitialize(callbackData->locationContext, NULL, &gpsConfig, gpsConfigCount,
            NULL, NULL, 3, callbackData);
        CU_ASSERT_EQUAL(err, NE_INVAL);

        NB_LocationContextDestroy(callbackData->locationContext);
        DestroyCallbackCompletedEvent(callbackData->event);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesInitializeWithoutGpsConfig(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        NB_LS_LocationConfig config = {0};
        uint32               gpsConfigCount = 0;

        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationSetPalAndContext(callbackData->locationContext, pal, callbackData->context);
        CU_ASSERT_EQUAL(err, NE_OK);

        config.emulationMode = TRUE;
        nsl_strcpy(config.emulationFilename, GPS_FILENAME);

        callbackData->event = CreateCallbackCompletedEvent();

        err = NB_LocationInitialize(callbackData->locationContext, &config, NULL, gpsConfigCount,
            NULL, NULL, 3, callbackData);
        CU_ASSERT_EQUAL(err, NE_INVAL);

        NB_LocationContextDestroy(callbackData->locationContext);
        DestroyCallbackCompletedEvent(callbackData->event);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesResetCache(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        NB_LS_LocationConfig config = {0};
        ABPAL_GpsConfig      gpsConfig = {0};
        uint32               gpsConfigCount = 0;

        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationSetPalAndContext(callbackData->locationContext, pal, callbackData->context);
        CU_ASSERT_EQUAL(err, NE_OK);

        config.emulationMode = TRUE;
        nsl_strcpy(config.emulationFilename, GPS_FILENAME);

        callbackData->event = CreateCallbackCompletedEvent();

        err = NB_LocationInitialize(callbackData->locationContext, &config, &gpsConfig, gpsConfigCount,
            NULL, NULL, 3, callbackData);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationResetCache(callbackData->context);
        CU_ASSERT_EQUAL(err, NE_OK);

        NB_LocationContextDestroy(callbackData->locationContext);
        DestroyCallbackCompletedEvent(callbackData->event);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}

void
TestLocationServicesResetCacheWithoutContext(void)
{
    NB_Error err = NE_OK;
    struct TestLocationServicesCallbackData* callbackData = {0};
    PAL_Instance* pal = NULL;
    uint8 rc = 0;

    callbackData = nsl_malloc(sizeof(TestLocationServicesCallbackData));
    CU_ASSERT_PTR_NOT_NULL(callbackData);
    if(callbackData)
    {
        nsl_memset(callbackData, 0, sizeof(TestLocationServicesCallbackData));
        rc = CreatePalAndContext(&pal, &(callbackData->context));
    }

    if (rc)
    {
        NB_LS_LocationConfig config = {0};
        ABPAL_GpsConfig      gpsConfig = {0};
        uint32               gpsConfigCount = 0;

        err = NB_LocationContextCreate(&(callbackData->locationContext));
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationSetPalAndContext(callbackData->locationContext, pal, callbackData->context);
        CU_ASSERT_EQUAL(err, NE_OK);

        config.emulationMode = TRUE;
        nsl_strcpy(config.emulationFilename, GPS_FILENAME);

        callbackData->event = CreateCallbackCompletedEvent();

        err = NB_LocationInitialize(callbackData->locationContext, &config, &gpsConfig, gpsConfigCount,
            NULL, NULL, 3, callbackData);
        CU_ASSERT_EQUAL(err, NE_OK);

        err = NB_LocationResetCache(NULL);
        CU_ASSERT_EQUAL(err, NE_INVAL);

        NB_LocationContextDestroy(callbackData->locationContext);
        DestroyCallbackCompletedEvent(callbackData->event);
    }

    if(callbackData)
    {
        DestroyContext(callbackData->context);
        nsl_free(callbackData);
    }

    PAL_DestroyInstance(pal);
}


static void LocationOneFixCallback(void* appData, NB_LocationContext* context, const NB_LS_Location* location, NB_Error error)
{
    CU_ASSERT_EQUAL(error, NE_OK);

    if (error == NE_OK)
    {
        TestLocationServicesCallbackData* callbackData = (TestLocationServicesCallbackData*) appData;

        if (callbackData->event)
        {
            SetCallbackCompletedEvent(callbackData->event);
        }
    }
}

static void LocationSeveralFixesCallback(void* appData, NB_LocationContext* context, const NB_LS_Location* location, NB_Error error)
{
    CU_ASSERT_EQUAL(error, NE_OK);

    if (error == NE_OK)
    {
        TestLocationServicesCallbackData* callbackData = (TestLocationServicesCallbackData*) appData;

        callbackData->countOfFixes++;
        if (callbackData->event && callbackData->countOfFixes == NUMBER_OF_FIXES)
        {
            SetCallbackCompletedEvent(callbackData->event);
        }
    }
}