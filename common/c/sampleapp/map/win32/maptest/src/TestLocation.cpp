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
   @file        TestLocation.cpp
   @defgroup

   Description:

*/
/*
   (C) Copyright 2012 by TeleCommunications Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.

 --------------------------------------------------------------------------*/

/*! @{ */
#include "CUnit.h"
extern "C"
{
#include "palclock.h"
#include "testnetwork.h"
#include "main.h"
#include "platformutil.h"
#include "palmath.h"
#include "paltaskqueue.h"
#include "nblocationhandler.h"
}

#include "TestLocation.h"
#include "TestInstance.h"

#ifdef __GNUC__
#define FUNCTION(func) instance->func()
#else
#define FUNCTION(func) instance->##func()
#endif

#define PERFORM_TEST(CLASS, func, waitTimeout)        \
    do {                                              \
        CLASS* instance = new CLASS();                \
        CU_ASSERT_PTR_NOT_NULL(instance);             \
        if (instance != NULL)                         \
        {                                             \
            FUNCTION(func);                           \
            if (!instance->WaitEvent(waitTimeout))    \
            {                                         \
                CU_FAIL("Test is timed out!");        \
                instance->ScheduleFinishTest();       \
                instance->WaitEvent(DESTROY_TIMEOUT); \
            }                                         \
            delete instance;                          \
        }                                             \
    } while (0);

#define DO_REAL_TEST(TEST_CLASS, TEST_FUNCTION, CALLBACK_FUNCTION)  \
    do                                                              \
    {                                                               \
        m_functor = new TestTask<TEST_CLASS>                        \
                    (this,                                          \
                     TEST_FUNCTION,                                 \
                     CALLBACK_FUNCTION);                            \
        CallTestFunction();                                         \
    } while (0)

const uint32 CALLBACK_TIMEOUT = 20 * 1000;

class TestLocationInstance : public TestInstance
{
public:
    TestLocationInstance();
    virtual ~TestLocationInstance();

    static void Static_LocationCallback(void* handler,
                                        NB_RequestStatus status,
                                        NB_Error err,
                                        uint8 up,
                                        int percent,
                                        void* userData);

    // Test functions.
    void TestLocationGsm();
    void TestLocationCdma();
    void TestLocationWifi();
    void TestLocationGsmCdma();
    void TestLocationMultipleWifi();


private:

    typedef struct
    {
        uint32 mobileCountryCode;
        uint32 mobileNetworkCode;
        uint32 locationAreaCode;
        uint32 cellId;
        int16 signalStrength;
    } GsmParameters;

    typedef struct
    {
        uint32 systemId;
        uint32 networkId;
        uint32 cellId;
        int16 signalStrength;
    } CdmaParameters;

    typedef struct
    {
        const char* macAddress;
        int16 signalStrength;
    } WifiParameters;


    void FinishTestPrivate();

    void TestCellIdLocationRequest(GsmParameters* gsm,
                                   CdmaParameters* cdma,
                                   WifiParameters* wifi);
    void LocationCallback(void* handler,
                          NB_RequestStatus status,
                          NB_Error err,
                          uint8 up,
                          int percent);
    void LocationCallBack(void);


    void LocationGsm(void);
    void LocationCdma(void);
    void LocationWifi(void);
    void LocationGsmCdma(void);
    void LocationMultipleWifi(void);

    NB_LocationInformation*     m_information;
    NB_LocationHandler*         m_handler;
    NB_LocationParameters*      m_params;
};


//
TestLocationInstance::TestLocationInstance()
        : TestInstance()
{
}

TestLocationInstance::~TestLocationInstance()
{
}

void TestLocationInstance::FinishTestPrivate()
{
    NB_Error err = NE_OK;

    if (m_params)
    {
        err = NB_LocationParametersDestroy(m_params);
        m_params = NULL;
        CU_ASSERT_EQUAL(err, NE_OK);
    }

    if (m_handler)
    {
        err = NB_LocationHandlerDestroy(m_handler);
        m_handler = NULL;
        CU_ASSERT_EQUAL(err, NE_OK);
    }

}

void TestLocationInstance::TestCellIdLocationRequest(GsmParameters* gsm,
                                                     CdmaParameters* cdma,
                                                     WifiParameters* wifi)
{
    NB_Error err = NE_OK;

    NB_RequestHandlerCallback callback = {0};

    callback.callback = &Static_LocationCallback;
    callback.callbackData = this;

    err = NB_LocationHandlerCreate(m_context, &callback, &m_handler);
    CU_ASSERT_EQUAL(err, NE_OK);

    err = NB_LocationParametersCreate(m_context, &m_params);
    CU_ASSERT_EQUAL(err, NE_OK);


    if (gsm)
    {
        err = NB_LocationParametersAddGsm(m_params, gsm->mobileCountryCode,
                                          gsm->mobileNetworkCode, gsm->locationAreaCode,
                                          gsm->cellId, gsm->signalStrength);
        CU_ASSERT_EQUAL(err, NE_OK);
    }

    if (cdma)
    {
        err = NB_LocationParametersAddCdma(m_params, cdma->systemId, cdma->networkId,
                                           cdma->cellId, cdma->signalStrength);
        CU_ASSERT_EQUAL(err, NE_OK);
    }

    if (wifi)
    {
        err = NB_LocationParametersAddWifi(m_params, wifi->macAddress, wifi->signalStrength);
        CU_ASSERT_EQUAL(err, NE_OK);
    }

    err = NB_LocationHandlerStartRequest(m_handler, m_params);
    CU_ASSERT_EQUAL(err, NE_OK);
    if (err != NE_OK)
    {
        CU_FAIL("Failed to send request.");
    }
}

void TestLocationInstance::Static_LocationCallback(void* handler,
                                                   NB_RequestStatus status,
                                                   NB_Error err,
                                                   uint8 up,
                                                   int percent,
                                                   void* userData)
{
    TestLocationInstance* pThis = static_cast <TestLocationInstance*>(userData);
    if (pThis)
    {
        pThis->LocationCallback(handler, status, err, up, percent);
    }
}

void TestLocationInstance::LocationCallback(void* handler,
                                            NB_RequestStatus status,
                                            NB_Error err,
                                            uint8 up,
                                            int percent)
{
    if (!up)
    {
        if (err != NE_OK)
        {
            CU_FAIL("Callback returned error");
            ScheduleFinishTest();
            return;
        }

        switch (status)
        {
            case NB_NetworkRequestStatus_Progress:
                // not complete yet... return
                return;
            case NB_NetworkRequestStatus_Success:
                // create info from request and store pointer back using userData
                err = NB_LocationHandlerGetInformation( m_handler, &m_information );
                CU_ASSERT_EQUAL(err, NE_OK);
                break;
            case NB_NetworkRequestStatus_TimedOut:
                CU_FAIL("NB_NetworkRequestStatus_TimedOut");
                break;
            case NB_NetworkRequestStatus_Canceled:
                CU_FAIL("NB_NetworkRequestStatus_Canceled");
                break;
            case NB_NetworkRequestStatus_Failed:
                CU_FAIL("NB_NetworkRequestStatus_Failed");
                break;
        }
    }

    if (this->m_functor)
    {
        this->m_functor->ExecuteCallback();
    }
}


void TestLocationInstance::LocationCallBack(void)
{
    NB_Error err = NE_OK;

    NB_LocationRequestStatus status = NB_LRS_Success;
    NB_LatitudeLongitude location = { 0 };
    uint32 accuracy = 0;
    char source[80] = {0};

    err = NB_LocationInformationGetNetworkPosition(m_information, &status, &location,
                                                   &accuracy, source, sizeof(source));
    CU_ASSERT_EQUAL(err, NE_OK);
    err = NB_LocationInformationDestroy(m_information);
    CU_ASSERT_EQUAL(err, NE_OK);

    ScheduleFinishTest();
}

void TestLocationInstance::LocationGsm(void)
{
    GsmParameters gsm = { 0 };

    gsm.mobileCountryCode = 208;
    gsm.mobileNetworkCode = 1;
    gsm.locationAreaCode  = 1025;
    gsm.cellId            = 27296;
    gsm.signalStrength    = -40;

    CU_ASSERT_PTR_NOT_NULL(m_pal);
    CU_ASSERT_PTR_NOT_NULL(m_context);

    TestCellIdLocationRequest(&gsm, NULL, NULL);
}

void TestLocationInstance::LocationCdma(void)
{
    CdmaParameters cdma = { 0 };

    cdma.systemId       = 2;
    cdma.networkId      = 55;
    cdma.cellId         = 507;
    cdma.signalStrength = -40;

    CU_ASSERT_PTR_NOT_NULL(m_pal);
    CU_ASSERT_PTR_NOT_NULL(m_context);

    TestCellIdLocationRequest(NULL, &cdma, NULL);
}

void TestLocationInstance::LocationWifi(void)
{
    WifiParameters wifi = { 0 };

    wifi.macAddress     = "00:1a:70:90:A0:00";
    wifi.signalStrength = -40;

    CU_ASSERT_PTR_NOT_NULL(m_pal);
    CU_ASSERT_PTR_NOT_NULL(m_context);

    TestCellIdLocationRequest(NULL, NULL, &wifi);
}

void TestLocationInstance::LocationGsmCdma(void)
{
    GsmParameters  gsm  = { 0 };
    CdmaParameters cdma = { 0 };

    gsm.mobileCountryCode = 208;
    gsm.mobileNetworkCode = 1;
    gsm.locationAreaCode  = 1025;
    gsm.cellId            = 27296;
    gsm.signalStrength    = -40;

    cdma.systemId       = 2;
    cdma.networkId      = 55;
    cdma.cellId         = 507;
    cdma.signalStrength = -40;

    CU_ASSERT_PTR_NOT_NULL(m_pal);
    CU_ASSERT_PTR_NOT_NULL(m_context);

    TestCellIdLocationRequest(&gsm, &cdma, NULL);
}

void TestLocationInstance::LocationMultipleWifi(void)
{
    NB_Error err = NE_OK;

    CU_ASSERT_PTR_NOT_NULL(m_pal);
    CU_ASSERT_PTR_NOT_NULL(m_context);

    NB_RequestHandlerCallback callback = {0};
    callback.callback                  = &Static_LocationCallback;
    callback.callbackData              = this;

    err = NB_LocationHandlerCreate(m_context, &callback, &m_handler);
    CU_ASSERT_EQUAL(err, NE_OK);

    err = NB_LocationParametersCreate(m_context, &m_params);
    CU_ASSERT_EQUAL(err, NE_OK);

    err = NB_LocationParametersAddWifi(m_params, "00:1A:70:90:A0:01", -40);
    CU_ASSERT_EQUAL(err, NE_OK);

    err = NB_LocationParametersAddWifi(m_params, "00:1A:70:90:A0:02", -30);
    CU_ASSERT_EQUAL(err, NE_OK);

    err = NB_LocationParametersAddWifi(m_params, "00:1A:70:90:A0:03", -20);
    CU_ASSERT_EQUAL(err, NE_OK);

    err = NB_LocationHandlerStartRequest(m_handler,m_params);
    CU_ASSERT_EQUAL(err, NE_OK);
}



void TestLocationInstance::TestLocationGsm()
{
    DO_REAL_TEST(TestLocationInstance, &TestLocationInstance::LocationGsm,
                 &TestLocationInstance::LocationCallBack);
}

void TestLocationInstance::TestLocationCdma()
{
    DO_REAL_TEST(TestLocationInstance, &TestLocationInstance::LocationCdma,
                 &TestLocationInstance::LocationCallBack);
}

void TestLocationInstance::TestLocationWifi()
{
    DO_REAL_TEST(TestLocationInstance, &TestLocationInstance::LocationWifi,
                 &TestLocationInstance::LocationCallBack);
}

void TestLocationInstance::TestLocationGsmCdma()
{
    DO_REAL_TEST(TestLocationInstance, &TestLocationInstance::LocationGsmCdma,
                 &TestLocationInstance::LocationCallBack);
}

void TestLocationInstance::TestLocationMultipleWifi()
{
    DO_REAL_TEST(TestLocationInstance, &TestLocationInstance::LocationMultipleWifi,
                 &TestLocationInstance::LocationCallBack);
}

//

/*! @brief Test location via GSM query

  @return None. CUnit asserts are called on failure.

  @see TestNetwork_Initialize
  @see TestNetwork_Cleanup
*/
void
TestLocationGsm(void)
{
    PERFORM_TEST(TestLocationInstance, TestLocationGsm, CALLBACK_TIMEOUT);
}

/*! @brief Test location via CDMA query

  @return None. CUnit asserts are called on failure.

  @see TestNetwork_Initialize
  @see TestNetwork_Cleanup
*/
void
TestLocationCdma(void)
{
    PERFORM_TEST(TestLocationInstance, TestLocationGsm, CALLBACK_TIMEOUT);
}

/*! @brief Test location via Wifi query

  @return None. CUnit asserts are called on failure.

  @see TestNetwork_Initialize
  @See TestNetwork_Cleanup
*/
void
TestLocationWifi(void)
{
    PERFORM_TEST(TestLocationInstance, TestLocationWifi, CALLBACK_TIMEOUT);
}

/*! @brief Test location via GSM/CDMA query

  @return None. CUnit asserts are called on failure.

  @see TestNetwork_Initialize
  @see TestNetwork_Cleanup
*/
void
TestLocationGsmCdma(void)
{
    PERFORM_TEST(TestLocationInstance, TestLocationGsmCdma, CALLBACK_TIMEOUT);
}

/*! @brief Test location via a multiple WiFi query

  @return None. CUnit asserts are called on failure.

  @see TestNetwork_Initialize
  @see TestNetwork_Cleanup
*/
void
TestLocationMultipleWifi(void)
{
    PERFORM_TEST(TestLocationInstance, TestLocationMultipleWifi, CALLBACK_TIMEOUT);
}


//

/*! Add common initialization code here.

  @return 0

  @see TestNetwork_SuiteCleanup
*/
int
TestLocation_SuiteSetup()
{
    return 0;
}


/*! Add common cleanup code here.

  @return 0

  @see TestNetwork_SuiteSetup
*/
int
TestLocation_SuiteCleanup()
{
    return 0;
}

/*! Add all your test functions here

@return None
*/
void
TestLocation_AddTests( CU_pSuite pTestSuite, int level )
{
    // ! Add all your function names here !
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationGsm", TestLocationGsm);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationCdma", TestLocationCdma);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationWifi", TestLocationWifi);
    ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationGsmCdma", TestLocationGsmCdma);
    // ADD_TEST(level, TestLevelSmoke, pTestSuite, "TestLocationMultipleWifi",
    //          TestLocationMultipleWifi);
};

/*! @} */

