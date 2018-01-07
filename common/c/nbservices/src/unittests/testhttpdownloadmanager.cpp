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

    @file       testhttpdownloadmanager.cpp

    See header file for description.
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

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
    #include "nbcontext.h"
    #include "nbcontextprotected.h"
    #include "paltimer.h"
    #include "palfile.h"
    #include "networkutil.h"
    #include "platformutil.h"
}

#include "testhttpdownloadmanager.h"
#include "downloadrequest.h"
#include "datastream.h"
#include "httpdownloadmanager.h"
#include "StringUtility.h"
#include "urlparser.h"

using namespace std;
using namespace nbcommon;

// Local Constants ..............................................................................

static const uint32 TEST_DATA_SIZE = 4096;

static const uint32 TEST_DOWNLOAD_CONNECTION_COUNT = 3;
static const uint32 TEST_RETRY_TIMES = 3;

static const char TEST_HOSTNAME[] = "http://dev14dts.navbuilder.nimlbs.net/";
static const uint16 TEST_PORT = 8080;

static const char TEST_FIELD_NAME_CLIENT_GUID[] = "X-Navbuilder-Clientid";
static const char TEST_FIELD_NAME_ETAG[] = "ETag";
static const char TEST_FIELD_NAME_IF_NONE_MATCH[] = "If-None-Match";
static const char TEST_FIELD_CONTENT_CLIENT_GUID[] = "\"CCC test\"";

static const uint32 TEST_REQUEST_COUNT = 8;

static const uint32 TEST_DOWNLOAD_LEVEL[TEST_REQUEST_COUNT] = {
    2000,
    1000,
    100,
    100,
    1,
    80,
    20,
    40
};

static const uint32 TEST_DOWNLOAD_PRIORITY[TEST_REQUEST_COUNT] = {
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8
};

// The last URL is a wrong URL.
static const char* TEST_URL[TEST_REQUEST_COUNT] = {
    "unified_map?type=DMAT&gen=921158887&fmt=NBM&vfmt=23",
    "map?loc=en-US&v=1.0&fmt=png&res=96&x=0&y=0&z=2&sz=128",
    "map?loc=en-US&v=1.0&fmt=png&res=96&x=0&y=1&z=2&sz=128",
    "map?loc=en-US&v=1.0&fmt=png&res=96&x=0&y=4&z=4&sz=128",
    "map?loc=en-US&v=1.0&fmt=png&res=96&x=4&y=4&z=4&sz=128",
    "map?loc=en-US&v=1.0&fmt=png&res=96&x=6&y=16&z=6&sz=128",
    "map?loc=en-US&v=1.0&fmt=png&res=96&x=20&y=22&z=6&sz=128",
    "wrongurl"
};

static const char* TEST_FILENAME[TEST_REQUEST_COUNT] = {
    "unifiedmap.nbm",
    "map-0-0-2.png",
    "map-0-1-2.png",
    "map-0-4-4.png",
    "map-4-4-4.png",
    "map-6-16-6.png",
    "map-20-22-6.png",
    "wrong.png"
};

static const char TEST_HTTPS_URL1[] = "https://maps.google.com/maps/api/staticmap?center=42.355522,-71.068977&size=270x240&zoom=12&maptype=roadmap&sensor=false";
static const char TEST_HTTPS_URL2[] = "https://maps.google.com/maps/api/staticmap?center=33.599965,-117.70674&size=270x240&zoom=12&maptype=roadmap&sensor=false";

static const char TEST_HTTPS_FILENAME1[] = "BOSTON.png";
static const char TEST_HTTPS_FILENAME2[] = "LAGUNA-HILLS.png";

static const uint32 TEST_HTTPS_PRIORITY1 = 800;
static const uint32 TEST_HTTPS_PRIORITY2 = 100;

// Count of HTTP download connections for testing
static const uint32 TEST_DEFAULT_HTTP_CONNECTION_COUNT = 6;
static const uint32 TEST_HTTP_CONNECTION_COUNT_FOR_PRIORITY = 1;


// Local Types ..................................................................................

/*! Collection of all instances used for the unit tests */
class TestHttpDownloadInstances
{
public:
    /* See definition for description */

    TestHttpDownloadInstances();
    virtual ~TestHttpDownloadInstances();
    NB_Error Initialize(uint32 httpConnectionCount = TEST_DEFAULT_HTTP_CONNECTION_COUNT,
                        uint32 initialRequestIndex = TEST_REQUEST_COUNT);
    void Reset();
    DownloadManager* GetGenericDownloadManager();

    NB_Error m_downloadingError;                /*!< Error during downloading */
    int m_downloadingRequestCount;              /*!< Count of requests which are downloading in progress */
    uint32 m_requestIndex;                      /*!< Index of request. If this value is not less than TEST_REQUEST_COUNT
                                                     in tile request callback function, the callback completed event is set. */
    PAL_Instance* m_pal;                        /*!< Pointer to PAL instance */
    NB_Context* m_context;                      /*!< Pointer to current context */
    void* m_event;                              /*!< Callback completed event */
    DownloadRequestPtr m_request;               /*!< A request saved temporarily */
};

/*! Download request for testing */
class TestDownloadRequest : public DownloadRequest
{
public:
    /* See definition for description */

    TestDownloadRequest(TestHttpDownloadInstances& testInstances,
                        uint16 port,
                        uint32 retryTimes,
                        uint32 priority,
                        DownloadProtocol protocol,
                        shared_ptr<string> hostname,
                        shared_ptr<string> url,
                        shared_ptr<string> filename);
    virtual ~TestDownloadRequest();
    NB_Error WriteDataToFile();

    /* See description in 'downloadrequest.h' */
    virtual DownloadProtocol GetProtocol();
    virtual shared_ptr<string> GetHostname();
    virtual uint16 GetPort();
    virtual shared_ptr<string> GetUrl();
    virtual uint32 GetRetryTimes();
    virtual void DuplicatedRequestWillBeIgnored(DownloadRequestPtr duplicatedRequest);
    virtual void AddedToDownloadQueue(DownloadRequestPtr request);
    virtual void RemovedFromDownloadQueue(DownloadRequestPtr request);
    virtual void DataAvailable(uint32 size);
    virtual void DataError(NB_Error error);

private:
    // Copy constructor and assignment operator are not supported.
    TestDownloadRequest(const TestDownloadRequest& testDownloadRequest);
    TestDownloadRequest& operator=(const TestDownloadRequest& testDownloadRequest);

public:
    TestHttpDownloadInstances& m_testInstances; /*!< 'TestHttpDownloadInstances' object */
    uint16 m_port;                              /*!< Port for testing */
    uint32 m_retryTimes;                        /*!< Retry times for testing */
    DownloadProtocol m_protocol;                /*!< Protocol for testing */
    shared_ptr<string> m_hostname;              /*!< Hostname for testing */
    shared_ptr<string> m_url;                   /*!< URL for testing */
    shared_ptr<string> m_filename;              /*!< Filename for testing */
};


// Test Functions ...............................................................................

static void TestHttpDownloadManagerInitialization();
static void TestHttpDownloadManagerRequestData();
static void TestHttpDownloadManagerCancelRequest();
static void TestHttpDownloadManagerClearRequests();
static void TestHttpDownloadManagerPauseAndResumeDownloading();
static void TestHttpDownloadManagerRequestDuplicatedData();
static void TestHttpDownloadManagerRequestDataFailed();
static void TestHttpDownloadManagerRequestAndResponseHeaders();
static void TestHttpDownloadManagerRequestHttpsData();
static void TestHttpDownloadManagerRequestDataWithPriority();


// Helper functions .............................................................................

static DownloadRequestPtr CreateRequest(TestHttpDownloadInstances& testInstances,
                                        int index);
static std::vector<DownloadRequestPtr> CreateAllRequests(TestHttpDownloadInstances& testInstances);
static DownloadRequestPtr CreateRequestByEntireUrl(TestHttpDownloadInstances& testInstances,
                                                   const char* entireUrl,
                                                   const char* filename,
                                                   uint32 downloadLevel);

static void TimerCallbackForCancelingRequest(PAL_Instance *pal,
                                             void *userData,
                                             PAL_TimerCBReason reason);
static void TimerCallbackForClearingRequests(PAL_Instance *pal,
                                             void *userData,
                                             PAL_TimerCBReason reason);
static void TimerCallbackForResumingDownloading(PAL_Instance *pal,
                                                void *userData,
                                                PAL_TimerCBReason reason);


// Public Functions .............................................................................

/*! Add all your test functions here

    @return None
*/
void
TestHttpDownloadManager_AddAllTests(CU_pSuite pTestSuite)
{
    // ! Add all your function names here !
    CU_add_test(pTestSuite, "TestHttpDownloadManagerInitialization", &TestHttpDownloadManagerInitialization);
    CU_add_test(pTestSuite, "TestHttpDownloadManagerRequestData", &TestHttpDownloadManagerRequestData);
    CU_add_test(pTestSuite, "TestHttpDownloadManagerCancelRequest", &TestHttpDownloadManagerCancelRequest);
    CU_add_test(pTestSuite, "TestHttpDownloadManagerClearRequests", &TestHttpDownloadManagerClearRequests);
    CU_add_test(pTestSuite, "TestHttpDownloadManagerPauseAndResumeDownloading", &TestHttpDownloadManagerPauseAndResumeDownloading);
    CU_add_test(pTestSuite, "TestHttpDownloadManagerRequestDuplicatedData", &TestHttpDownloadManagerRequestDuplicatedData);
    CU_add_test(pTestSuite, "TestHttpDownloadManagerRequestDataFailed", &TestHttpDownloadManagerRequestDataFailed);
    CU_add_test(pTestSuite, "TestHttpDownloadManagerRequestAndResponseHeaders", &TestHttpDownloadManagerRequestAndResponseHeaders);
    CU_add_test(pTestSuite, "TestHttpDownloadManagerRequestHttpsData", &TestHttpDownloadManagerRequestHttpsData);
    CU_add_test(pTestSuite, "TestHttpDownloadManagerRequestDataWithPriority", &TestHttpDownloadManagerRequestDataWithPriority);
}

/*! Add common initialization code here

    @return 0
*/
int
TestHttpDownloadManager_SuiteSetup()
{
    return 0;
}

/*! Add common cleanup code here

    @return 0
*/
int
TestHttpDownloadManager_SuiteCleanup()
{
    return 0;
}


// Test Functions ...............................................................................

/*! Test initializing a 'HttpDownloadManager' object

    @return None. CUnit Asserts get called on failures.
*/
void
TestHttpDownloadManagerInitialization()
{
    NB_Error error = NE_OK;
    TestHttpDownloadInstances testInstances;

    // Initialize a 'HttpDownloadManager' object in the function 'Initialize' of class 'TestHttpDownloadInstances'.
    error = testInstances.Initialize();
    CU_ASSERT_EQUAL(error, NE_OK);
}

/*! Test requesting data

    @return None. CUnit Asserts get called on failures.
*/
void
TestHttpDownloadManagerRequestData()
{
    NB_Error error = NE_OK;
    DownloadManager* downloadManager = NULL;
    TestHttpDownloadInstances testInstances;

    // Initialize the 'TestHttpDownloadInstances' object.
    error = testInstances.Initialize();
    CU_ASSERT_EQUAL(error, NE_OK);
    if (error != NE_OK)
    {
        return;
    }

    // Get the generic download manager.
    downloadManager = testInstances.GetGenericDownloadManager();
    if (!downloadManager)
    {
        CU_FAIL("Cannot get generic download manager");
        return;
    }

    /* Request data for every level of 'HttpDownloadManager' object. This test case is complete when
       all requests are downloaded.

       PAL, context, event and 'HttpDownloadManager' object have been checked in the function
       'Initialize' of class 'TestHttpDownloadInstances'.
    */
    {
        std::vector<DownloadRequestPtr> requests = CreateAllRequests(testInstances);
        std::vector<DownloadRequestPtr>::const_iterator iterator = requests.begin();
        std::vector<DownloadRequestPtr>::const_iterator end = requests.end();
        int i = 0;
        int requestCount = requests.size();

        if (requests.empty())
        {
            CU_FAIL("No request is created");
            return;
        }

        for (; iterator != end; ++iterator)
        {
            std::vector<DownloadRequestPtr> tempRequests;

            tempRequests.push_back(*iterator);
            error = downloadManager->RequestData(tempRequests, TEST_DOWNLOAD_LEVEL[i], NULL);
            if (error != NE_OK)
            {
                CU_FAIL("HttpDownloadManager::RequestData() failed");
                return;
            }

            // Increase the count of requests which are downloading in progress.
            ++(testInstances.m_downloadingRequestCount);

            ++i;
        }

        if (!WaitForCallbackCompletedEvent(testInstances.m_event, 60000))
        {
            CU_FAIL("HttpDownloadManager::RequestData() timeout");
            return;
        }

        // Check if an error occurred during downloading.
        if (testInstances.m_downloadingError != NE_OK)
        {
            CU_FAIL("An error occurred during downloading");
            return;
        }

        // Check if all download levels are empty and not paused.
        for (i = 0; i < requestCount; ++i)
        {
            CU_ASSERT(downloadManager->IsEmpty(TEST_DOWNLOAD_LEVEL[i]));
            CU_ASSERT(!(downloadManager->IsPaused(TEST_DOWNLOAD_LEVEL[i])));
        }

        // Get data and write to files.
        iterator = requests.begin();
        end = requests.end();
        for (; iterator != end; ++iterator)
        {
            DownloadRequest* request = (*iterator).get();
            TestDownloadRequest* testRequest = (TestDownloadRequest*) request;

            CU_ASSERT_PTR_NOT_NULL(testRequest);
            if (!testRequest)
            {
                return;
            }

            error = testRequest->WriteDataToFile();
            CU_ASSERT_EQUAL(error, NE_OK);
            if (error != NE_OK)
            {
                return;
            }
        }
    }
}

/*! Test canceling a request

    @return None. CUnit Asserts get called on failures.
*/
void
TestHttpDownloadManagerCancelRequest()
{
    NB_Error error = NE_OK;
    DownloadManager* downloadManager = NULL;
    TestHttpDownloadInstances testInstances;

    // Initialize the 'TestHttpDownloadInstances' object.
    error = testInstances.Initialize();
    CU_ASSERT_EQUAL(error, NE_OK);
    if (error != NE_OK)
    {
        return;
    }

    // Get the generic download manager.
    downloadManager = testInstances.GetGenericDownloadManager();
    if (!downloadManager)
    {
        CU_FAIL("Cannot get generic download manager");
        return;
    }

    /* There are two cases:
       1. Request data and cancel this request directly.
       2. Request data again and then cancel this request in the timer callback function.

       PAL, context, event and 'HttpDownloadManager' object have been checked in the function
       'Initialize' of class 'TestHttpDownloadInstances'.
    */
    {
        DownloadRequestPtr request = CreateRequest(testInstances, 0);
        std::vector<DownloadRequestPtr> tempRequests;

        tempRequests.push_back(request);

        // Case 1. Request data and cancel this request directly.
        {
            error = downloadManager->RequestData(tempRequests,
                                                     TEST_DOWNLOAD_LEVEL[0],
                                                     NULL);
            if (error != NE_OK)
            {
                CU_FAIL("HttpDownloadManager::RequestData() failed");
                return;
            }

            error = downloadManager->CancelRequest(request);
            if (error != NE_OK)
            {
                CU_FAIL("HttpDownloadManager::CancelRequest() failed");
                return;
            }

            // Check if the download level is empty and not paused.
            CU_ASSERT(downloadManager->IsEmpty(TEST_DOWNLOAD_LEVEL[0]));
            CU_ASSERT(!(downloadManager->IsPaused(TEST_DOWNLOAD_LEVEL[0])));
        }

        // Case 2. Request data again and then cancel this request in the timer callback function.
        {
            PAL_Error palError = PAL_Ok;
            error = downloadManager->RequestData(tempRequests,
                                                 TEST_DOWNLOAD_LEVEL[0],
                                                 NULL);
            if (error != NE_OK)
            {
                CU_FAIL("HttpDownloadManager::RequestData() failed");
                return;
            }

            // Increase the count of requests which are downloading in progress.
            ++(testInstances.m_downloadingRequestCount);

            // Save this request in 'TestHttpDownloadInstances' object and set the timer for canceling the request.
            testInstances.m_request = request;
            palError = PAL_TimerSet(testInstances.m_pal, 100, TimerCallbackForCancelingRequest, (void*) (&testInstances));
            CU_ASSERT_EQUAL(palError, PAL_Ok);
            if (palError != PAL_Ok)
            {
                return;
            }

            if (!WaitForCallbackCompletedEvent(testInstances.m_event, 20000))
            {
                CU_FAIL("HttpDownloadManager::RequestData() timeout");
                palError = PAL_TimerCancel(testInstances.m_pal, TimerCallbackForCancelingRequest, (void*) (&testInstances));
                return;
            }

            // Check if an error occurred during downloading.
            if (testInstances.m_downloadingError != NE_OK)
            {
                CU_FAIL("An error occurred during downloading");
                palError = PAL_TimerCancel(testInstances.m_pal, TimerCallbackForCancelingRequest, (void*) (&testInstances));
                return;
            }

            // Cancel the timer for canceling the request.
            palError = PAL_TimerCancel(testInstances.m_pal, TimerCallbackForCancelingRequest, (void*) (&testInstances));
            CU_ASSERT_EQUAL(palError, PAL_Ok);
            if (palError != PAL_Ok)
            {
                return;
            }

            // Check if the download level is empty and not paused.
            CU_ASSERT(downloadManager->IsEmpty(TEST_DOWNLOAD_LEVEL[0]));
            CU_ASSERT(!(downloadManager->IsPaused(TEST_DOWNLOAD_LEVEL[0])));
        }
    }
}

/*! Test clearing all requests

    @return None. CUnit Asserts get called on failures.
*/
void
TestHttpDownloadManagerClearRequests()
{
    NB_Error error = NE_OK;
    DownloadManager* downloadManager = NULL;
    TestHttpDownloadInstances testInstances;

    // Initialize the 'TestHttpDownloadInstances' object.
    error = testInstances.Initialize();
    CU_ASSERT_EQUAL(error, NE_OK);
    if (error != NE_OK)
    {
        return;
    }

    // Get the generic download manager.
    downloadManager = testInstances.GetGenericDownloadManager();
    if (!downloadManager)
    {
        CU_FAIL("Cannot get generic download manager");
        return;
    }

    /* Request data and then clear requests in the timer callback function.

       PAL, context, event and 'HttpDownloadManager' object have been checked in the function
       'Initialize' of class 'TestHttpDownloadInstances'.
    */
    {
        PAL_Error palError = PAL_Ok;
        std::vector<DownloadRequestPtr> requests = CreateAllRequests(testInstances);

        if (requests.empty())
        {
            CU_FAIL("No request is created");
            return;
        }

        // Request data.
        error = downloadManager->RequestData(requests,
                                             TEST_DOWNLOAD_LEVEL[0],
                                             NULL);
        if (error != NE_OK)
        {
            CU_FAIL("HttpDownloadManager::RequestData() failed");
            return;
        }

        // Increase the count of requests which are downloading in progress.
        testInstances.m_downloadingRequestCount += requests.size();

        // Set the timer for clearing requests.
        palError = PAL_TimerSet(testInstances.m_pal, 3000, TimerCallbackForClearingRequests, (void*) (&testInstances));
        CU_ASSERT_EQUAL(palError, PAL_Ok);
        if (palError != PAL_Ok)
        {
            return;
        }

        if (!WaitForCallbackCompletedEvent(testInstances.m_event, 60000))
        {
            CU_FAIL("HttpDownloadManager::RequestData() timeout");
            palError = PAL_TimerCancel(testInstances.m_pal, TimerCallbackForClearingRequests, (void*) (&testInstances));
            return;
        }

        // Check if an error occurred during downloading.
        if (testInstances.m_downloadingError != NE_OK)
        {
            CU_FAIL("An error occurred during downloading");
            palError = PAL_TimerCancel(testInstances.m_pal, TimerCallbackForClearingRequests, (void*) (&testInstances));
            return;
        }

        // Cancel the timer for clearing requests.
        palError = PAL_TimerCancel(testInstances.m_pal, TimerCallbackForClearingRequests, (void*) (&testInstances));
        CU_ASSERT_EQUAL(palError, PAL_Ok);
        if (palError != PAL_Ok)
        {
            return;
        }

        // Check if the download level is empty and not paused.
        CU_ASSERT(downloadManager->IsEmpty(TEST_DOWNLOAD_LEVEL[0]));
        CU_ASSERT(!(downloadManager->IsPaused(TEST_DOWNLOAD_LEVEL[0])));
    }
}

/*! Test pausing and resuming downloading

    @return None. CUnit Asserts get called on failures.
*/
void
TestHttpDownloadManagerPauseAndResumeDownloading()
{
    NB_Error error = NE_OK;
    DownloadManager* downloadManager = NULL;
    TestHttpDownloadInstances testInstances;

    // Initialize the 'TestHttpDownloadInstances' object.
    error = testInstances.Initialize();
    CU_ASSERT_EQUAL(error, NE_OK);
    if (error != NE_OK)
    {
        return;
    }

    // Get the generic download manager.
    downloadManager = testInstances.GetGenericDownloadManager();
    if (!downloadManager)
    {
        CU_FAIL("Cannot get generic download manager");
        return;
    }

    /* Request data and pause downloading. And then resume downloading in the timer callback function.

       PAL, context, event and 'HttpDownloadManager' object have been checked in the function
       'Initialize' of class 'TestHttpDownloadInstances'.
    */
    {
        PAL_Error palError = PAL_Ok;
        std::vector<DownloadRequestPtr> requests = CreateAllRequests(testInstances);

        if (requests.empty())
        {
            CU_FAIL("No request is created");
            return;
        }

        // Request data.
        error = downloadManager->RequestData(requests,
                                             TEST_DOWNLOAD_LEVEL[0],
                                             NULL);
        if (error != NE_OK)
        {
            CU_FAIL("HttpDownloadManager::RequestData() failed");
            return;
        }

        // Increase the count of requests which are downloading in progress.
        testInstances.m_downloadingRequestCount += requests.size();

        // Pause downloading but do not cancel requests which are downloading in progress.
        error = downloadManager->PauseDownloading(TEST_DOWNLOAD_LEVEL[0],
                                                  FALSE);
        if (error != NE_OK)
        {
            CU_FAIL("HttpDownloadManager::PauseDownloading() failed");
            return;
        }

        // Check if the download level is paused.
        CU_ASSERT(downloadManager->IsPaused(TEST_DOWNLOAD_LEVEL[0]));

        // Set the timer for resuming downloading.
        palError = PAL_TimerSet(testInstances.m_pal, 5000, TimerCallbackForResumingDownloading, (void*) (&testInstances));
        CU_ASSERT_EQUAL(palError, PAL_Ok);
        if (palError != PAL_Ok)
        {
            return;
        }

        if (!WaitForCallbackCompletedEvent(testInstances.m_event, 60000))
        {
            CU_FAIL("HttpDownloadManager::RequestData() timeout");
            palError = PAL_TimerCancel(testInstances.m_pal, TimerCallbackForResumingDownloading, (void*) (&testInstances));
            return;
        }

        // Check if an error occurred during downloading.
        if (testInstances.m_downloadingError != NE_OK)
        {
            CU_FAIL("An error occurred during downloading");
            palError = PAL_TimerCancel(testInstances.m_pal, TimerCallbackForResumingDownloading, (void*) (&testInstances));
            return;
        }

        // Cancel the timer for clearing requests.
        palError = PAL_TimerCancel(testInstances.m_pal, TimerCallbackForResumingDownloading, (void*) (&testInstances));
        CU_ASSERT_EQUAL(palError, PAL_Ok);
        if (palError != PAL_Ok)
        {
            return;
        }

        // Check if the download level is empty and not paused.
        CU_ASSERT(downloadManager->IsEmpty(TEST_DOWNLOAD_LEVEL[0]));
        CU_ASSERT(!(downloadManager->IsPaused(TEST_DOWNLOAD_LEVEL[0])));
    }
}

/*! Test requesting duplicated data

    @return None. CUnit Asserts get called on failures.
*/
void
TestHttpDownloadManagerRequestDuplicatedData()
{
    NB_Error error = NE_OK;
    DownloadManager* downloadManager = NULL;
    TestHttpDownloadInstances testInstances;

    // Initialize the 'TestHttpDownloadInstances' object.
    error = testInstances.Initialize();
    CU_ASSERT_EQUAL(error, NE_OK);
    if (error != NE_OK)
    {
        return;
    }

    // Get the generic download manager.
    downloadManager = testInstances.GetGenericDownloadManager();
    if (!downloadManager)
    {
        CU_FAIL("Cannot get generic download manager");
        return;
    }

    /* There are two cases:
       1. Send requests in same download level. The parameter 'duplicatedLevels' of function
          'HttpDownloadManager::RequestData' need not be specified.
       2. Send requests in different download levels. The parameter 'duplicatedLevels' need
          be specified.

       PAL, context, event and 'HttpDownloadManager' object have been checked in the function
       'Initialize' of class 'TestHttpDownloadInstances'.
    */
    {
        DownloadRequestPtr request1 = CreateRequest(testInstances, 0);
        DownloadRequestPtr request2 = CreateRequest(testInstances, 0);
        std::vector<DownloadRequestPtr> tempRequests1;
        std::vector<DownloadRequestPtr> tempRequests2;

        tempRequests1.push_back(request1);
        tempRequests2.push_back(request2);

        /* Case 1. Send requests in same download level. The parameter 'duplicatedLevels' of
                   function 'HttpDownloadManager::RequestData' need not be specified.
        */
        {
            error = downloadManager->RequestData(tempRequests1,
                                                 TEST_DOWNLOAD_LEVEL[0],
                                                 NULL);
            if (error != NE_OK)
            {
                CU_FAIL("HttpDownloadManager::RequestData() failed");
                return;
            }

            // Increase the count of requests which are downloading in progress.
            ++(testInstances.m_downloadingRequestCount);

            error = downloadManager->RequestData(tempRequests2,
                                                 TEST_DOWNLOAD_LEVEL[0],
                                                 NULL);
            if (error != NE_OK)
            {
                CU_FAIL("HttpDownloadManager::RequestData() failed");
                return;
            }

            if (!WaitForCallbackCompletedEvent(testInstances.m_event, 20000))
            {
                CU_FAIL("HttpDownloadManager::RequestData() timeout");
                return;
            }

            // Check if an error occurred during downloading.
            if (testInstances.m_downloadingError != NE_OK)
            {
                CU_FAIL("An error occurred during downloading");
                return;
            }

            // Check if the download level is empty and not paused.
            CU_ASSERT(downloadManager->IsEmpty(TEST_DOWNLOAD_LEVEL[0]));
            CU_ASSERT(!(downloadManager->IsPaused(TEST_DOWNLOAD_LEVEL[0])));
        }

        /* Case 2. Send requests in different download levels. The parameter
                   'duplicatedLevels' need be specified.
        */
        {
            std::vector<uint32> duplicatedLevels;

            duplicatedLevels.push_back(TEST_DOWNLOAD_LEVEL[0]);

            error = downloadManager->RequestData(tempRequests1,
                                                 TEST_DOWNLOAD_LEVEL[0],
                                                 NULL);
            if (error != NE_OK)
            {
                CU_FAIL("HttpDownloadManager::RequestData() failed");
                return;
            }

            // Increase the count of requests which are downloading in progress.
            ++(testInstances.m_downloadingRequestCount);

            error = downloadManager->RequestData(tempRequests2,
                                                 TEST_DOWNLOAD_LEVEL[1],
                                                 &duplicatedLevels);
            if (error != NE_OK)
            {
                CU_FAIL("HttpDownloadManager::RequestData() failed");
                return;
            }

            if (!WaitForCallbackCompletedEvent(testInstances.m_event, 20000))
            {
                CU_FAIL("HttpDownloadManager::RequestData() timeout");
                return;
            }

            // Check if an error occurred during downloading.
            if (testInstances.m_downloadingError != NE_OK)
            {
                CU_FAIL("An error occurred during downloading");
                return;
            }

            // Check if the download level is empty and not paused.
            CU_ASSERT(downloadManager->IsEmpty(TEST_DOWNLOAD_LEVEL[0]));
            CU_ASSERT(!(downloadManager->IsPaused(TEST_DOWNLOAD_LEVEL[0])));
            CU_ASSERT(downloadManager->IsEmpty(TEST_DOWNLOAD_LEVEL[1]));
            CU_ASSERT(!(downloadManager->IsPaused(TEST_DOWNLOAD_LEVEL[1])));
        }
    }
}

/*! Test requesting data failed

    @return None. CUnit Asserts get called on failures.
*/
void
TestHttpDownloadManagerRequestDataFailed()
{
    NB_Error error = NE_OK;
    DownloadManager* downloadManager = NULL;
    TestHttpDownloadInstances testInstances;

    // Initialize the 'TestHttpDownloadInstances' object.
    error = testInstances.Initialize();
    CU_ASSERT_EQUAL(error, NE_OK);
    if (error != NE_OK)
    {
        return;
    }

    // Get the generic download manager.
    downloadManager = testInstances.GetGenericDownloadManager();
    if (!downloadManager)
    {
        CU_FAIL("Cannot get generic download manager");
        return;
    }

    /* Test with wrong URL.

       PAL, context, event and 'HttpDownloadManager' object have been checked in the function
       'Initialize' of class 'TestHttpDownloadInstances'.
    */
    {
        DownloadRequestPtr request = CreateRequest(testInstances, TEST_REQUEST_COUNT - 1);
        std::vector<DownloadRequestPtr> tempRequests;

        tempRequests.push_back(request);

        error = downloadManager->RequestData(tempRequests,
                                             TEST_DOWNLOAD_LEVEL[0],
                                             NULL);
        if (error != NE_OK)
        {
            CU_FAIL("HttpDownloadManager::RequestData() failed");
            return;
        }

        // Increase the count of requests which are downloading in progress.
        ++(testInstances.m_downloadingRequestCount);

        if (!WaitForCallbackCompletedEvent(testInstances.m_event, 60000))
        {
            CU_FAIL("HttpDownloadManager::RequestData() timeout");
            return;
        }

        // An error occurred when downloading with a wrong URL.
        CU_ASSERT_NOT_EQUAL(testInstances.m_downloadingError, NE_OK);

        // Check if the download level is empty and not paused.
        CU_ASSERT(downloadManager->IsEmpty(TEST_DOWNLOAD_LEVEL[0]));
        CU_ASSERT(!(downloadManager->IsPaused(TEST_DOWNLOAD_LEVEL[0])));
    }
}

/*! Test sending a HTTP request with headers and receiving a HTTP response with headers

    @return None. CUnit Asserts get called on failures.
*/
void
TestHttpDownloadManagerRequestAndResponseHeaders()
{
    NB_Error error = NE_OK;
    DownloadManager* downloadManager = NULL;
    TestHttpDownloadInstances testInstances;
    shared_ptr<string> eTag;

    // Initialize the 'TestHttpDownloadInstances' object.
    error = testInstances.Initialize();
    CU_ASSERT_EQUAL(error, NE_OK);
    if (error != NE_OK)
    {
        return;
    }

    // Get the generic download manager.
    downloadManager = testInstances.GetGenericDownloadManager();
    if (!downloadManager)
    {
        CU_FAIL("Cannot get generic download manager");
        return;
    }

    /*  Test sending a HTTP request with a client GUID header and save the field content of
        ETag if it exists.

        PAL, context, event and 'HttpDownloadManager' object have been checked in the function
        'Initialize' of class 'TestHttpDownloadInstances'.
    */
    {
        DownloadRequestPtr request = CreateRequest(testInstances, 0);
        std::vector<DownloadRequestPtr> tempRequests;

        // Add a client GUID header.
        error = request->AddContentForRequestHeaderName(TEST_FIELD_NAME_CLIENT_GUID,
                                                        TEST_FIELD_CONTENT_CLIENT_GUID);
        if (error != NE_OK)
        {
            CU_FAIL("DownloadRequest::AddContentForRequestHeaderName() failed");
            return;
        }
        tempRequests.push_back(request);

        error = downloadManager->RequestData(tempRequests,
                                             TEST_DOWNLOAD_LEVEL[0],
                                             NULL);
        if (error != NE_OK)
        {
            CU_FAIL("HttpDownloadManager::RequestData() failed");
            return;
        }

        // Increase the count of requests which are downloading in progress.
        ++(testInstances.m_downloadingRequestCount);

        if (!WaitForCallbackCompletedEvent(testInstances.m_event, 30000))
        {
            CU_FAIL("HttpDownloadManager::RequestData() timeout");
            return;
        }

        // Check if an error occurred during downloading.
        if (testInstances.m_downloadingError != NE_OK)
        {
            CU_FAIL("An error occurred during downloading");
            return;
        }

        // Get the field content of ETag.
        eTag = request->GetContentByResponseHeaderName(TEST_FIELD_NAME_ETAG);
    }

    /*  Test sending a HTTP request with a client GUID header and a If-None-Match header with
        the latest ETag. This test case should get a NE_HTTP_NOT_MODIFIED error.
    */
    if (eTag)
    {
        DownloadRequestPtr requestWithEtagHeader = CreateRequest(testInstances, 0);
        std::vector<DownloadRequestPtr> tempRequestsWithEtagHeader;

        // Add a client GUID header and a If-None-Match header with the latest ETag.
        error = error ? error : requestWithEtagHeader->AddContentForRequestHeaderName(TEST_FIELD_NAME_CLIENT_GUID,
                                                                                      TEST_FIELD_CONTENT_CLIENT_GUID);
        error = error ? error : requestWithEtagHeader->AddContentForRequestHeaderName(TEST_FIELD_NAME_IF_NONE_MATCH,
                                                                                      *eTag);
        if (error != NE_OK)
        {
            CU_FAIL("DownloadRequest::AddContentForRequestHeaderName() failed");
            return;
        }
        tempRequestsWithEtagHeader.push_back(requestWithEtagHeader);

        error = downloadManager->RequestData(tempRequestsWithEtagHeader,
                                             TEST_DOWNLOAD_LEVEL[0],
                                             NULL);
        if (error != NE_OK)
        {
            CU_FAIL("HttpDownloadManager::RequestData() failed");
            return;
        }

        // Increase the count of requests which are downloading in progress.
        ++(testInstances.m_downloadingRequestCount);

        if (!WaitForCallbackCompletedEvent(testInstances.m_event, 30000))
        {
            CU_FAIL("HttpDownloadManager::RequestData() timeout");
            return;
        }

        // Check if the downloading error is NE_HTTP_NOT_MODIFIED if the ETag is latest.
        CU_ASSERT_EQUAL(testInstances.m_downloadingError, NE_HTTP_NOT_MODIFIED);
    }
}

/*! Test requesting data from URL of HTTPS

    @return None. CUnit Asserts get called on failures.
*/
void
TestHttpDownloadManagerRequestHttpsData()
{
    NB_Error error = NE_OK;
    DownloadManager* downloadManager = NULL;
    TestHttpDownloadInstances testInstances;

    // Initialize the 'TestHttpDownloadInstances' object.
    error = testInstances.Initialize();
    CU_ASSERT_EQUAL(error, NE_OK);
    if (error != NE_OK)
    {
        return;
    }

    // Get the generic download manager.
    downloadManager = testInstances.GetGenericDownloadManager();
    if (!downloadManager)
    {
        CU_FAIL("Cannot get generic download manager");
        return;
    }

    /* Request data for every level of 'HttpDownloadManager' object. This test case is complete when
       all requests are downloaded.

       PAL, context, event and 'HttpDownloadManager' object have been checked in the function
       'Initialize' of class 'TestHttpDownloadInstances'. */
    vector<DownloadRequestPtr> requests;

    // Add 2 requests of HTTP.
    requests.push_back(CreateRequest(testInstances, 0));
    requests.push_back(CreateRequest(testInstances, 1));

    // Add 1 request of HTTPS.
    requests.push_back(CreateRequestByEntireUrl(testInstances,
                                                TEST_HTTPS_URL1,
                                                TEST_HTTPS_FILENAME1,
                                                TEST_HTTPS_PRIORITY1));

    // Add 2 requests of HTTP.
    requests.push_back(CreateRequest(testInstances, 2));
    requests.push_back(CreateRequest(testInstances, 3));

    // Add 1 request of HTTPS.
    requests.push_back(CreateRequestByEntireUrl(testInstances,
                                                TEST_HTTPS_URL2,
                                                TEST_HTTPS_FILENAME2,
                                                TEST_HTTPS_PRIORITY2));

    // Add 2 requests of HTTP.
    requests.push_back(CreateRequest(testInstances, 4));
    requests.push_back(CreateRequest(testInstances, 5));

    // Request data to download.
    error = downloadManager->RequestData(requests,
                                         TEST_DOWNLOAD_LEVEL[0],
                                         NULL);
    if (error != NE_OK)
    {
        CU_FAIL("HttpDownloadManager::RequestData() failed");
        return;
    }

    // Set the count of requests to download.
    testInstances.m_downloadingRequestCount = static_cast<int>(requests.size());

    if (!WaitForCallbackCompletedEvent(testInstances.m_event, 60000))
    {
        CU_FAIL("HttpDownloadManager::RequestData() timeout");
        return;
    }

    // Check if an error occurred during downloading.
    if (testInstances.m_downloadingError != NE_OK)
    {
        CU_FAIL("An error occurred during downloading");
        return;
    }

    // Get data and write to files.
    vector<DownloadRequestPtr>::iterator requestIterator = requests.begin();
    vector<DownloadRequestPtr>::const_iterator requestEnd = requests.end();
    for (; requestIterator != requestEnd; ++requestIterator)
    {
        DownloadRequest* request = (*requestIterator).get();
        TestDownloadRequest* testRequest = (TestDownloadRequest*) request;

        CU_ASSERT_PTR_NOT_NULL(testRequest);
        if (!testRequest)
        {
            return;
        }

        error = testRequest->WriteDataToFile();
        CU_ASSERT_EQUAL(error, NE_OK);
        if (error != NE_OK)
        {
            return;
        }
    }
}

/*! Test requesting data with different priorities

    @return None. CUnit Asserts get called on failures.
*/
void
TestHttpDownloadManagerRequestDataWithPriority()
{
    NB_Error error = NE_OK;
    DownloadManager* downloadManager = NULL;
    TestHttpDownloadInstances testInstances;

    // Initialize the 'TestHttpDownloadInstances' object.
    error = testInstances.Initialize(TEST_HTTP_CONNECTION_COUNT_FOR_PRIORITY,
                                     0);
    CU_ASSERT_EQUAL(error, NE_OK);
    if (error != NE_OK)
    {
        return;
    }

    // Get the generic download manager.
    downloadManager = testInstances.GetGenericDownloadManager();
    if (!downloadManager)
    {
        CU_FAIL("Cannot get generic download manager");
        return;
    }

    /* Request data for every level of 'HttpDownloadManager' object. This test case is complete when
       all requests are downloaded.

       PAL, context, event and 'HttpDownloadManager' object have been checked in the function
       'Initialize' of class 'TestHttpDownloadInstances'.
    */
    {
        std::vector<DownloadRequestPtr> requests = CreateAllRequests(testInstances);
        std::vector<DownloadRequestPtr>::const_iterator iterator = requests.begin();
        std::vector<DownloadRequestPtr>::const_iterator end = requests.end();
        int i = 0;
        int requestCount = requests.size();

        if (requests.empty())
        {
            CU_FAIL("No request is created");
            return;
        }

        for (; iterator != end; ++iterator)
        {
            std::vector<DownloadRequestPtr> tempRequests;

            tempRequests.push_back(*iterator);
            error = downloadManager->RequestData(tempRequests, TEST_DOWNLOAD_LEVEL[i], NULL);
            if (error != NE_OK)
            {
                CU_FAIL("HttpDownloadManager::RequestData() failed");
                return;
            }

            // Increase the count of requests which are downloading in progress.
            ++(testInstances.m_downloadingRequestCount);

            ++i;
        }

        if (!WaitForCallbackCompletedEvent(testInstances.m_event, 60000))
        {
            CU_FAIL("HttpDownloadManager::RequestData() timeout");
            return;
        }

        // Check if an error occurred during downloading.
        if (testInstances.m_downloadingError != NE_OK)
        {
            CU_FAIL("An error occurred during downloading");
            return;
        }

        // Check if all download levels are empty and not paused.
        for (i = 0; i < requestCount; ++i)
        {
            CU_ASSERT(downloadManager->IsEmpty(TEST_DOWNLOAD_LEVEL[i]));
            CU_ASSERT(!(downloadManager->IsPaused(TEST_DOWNLOAD_LEVEL[i])));
        }

        // Get data and write to files.
        iterator = requests.begin();
        end = requests.end();
        for (; iterator != end; ++iterator)
        {
            DownloadRequest* request = (*iterator).get();
            TestDownloadRequest* testRequest = (TestDownloadRequest*) request;

            CU_ASSERT_PTR_NOT_NULL(testRequest);
            if (!testRequest)
            {
                return;
            }

            error = testRequest->WriteDataToFile();
            CU_ASSERT_EQUAL(error, NE_OK);
            if (error != NE_OK)
            {
                return;
            }
        }
    }
}


// Helper functions .............................................................................

/*! Create a request by index

    The parameter 'index' must be less than 'TEST_REQUEST_COUNT'.

    @return A request created
*/
DownloadRequestPtr
CreateRequest(TestHttpDownloadInstances& testInstances, /*!< A 'TestHttpDownloadInstances' object */
              int index                                 /*!< Index to create. It must be
                                                             less than 'TEST_REQUEST_COUNT'. */
              )
{
    string* hostname = NULL;
    string* url = NULL;
    string* filename = NULL;
    TestDownloadRequest* request = NULL;

    if (index >= TEST_REQUEST_COUNT)
    {
        CU_FAIL("Create a request with wrong index");
        return DownloadRequestPtr();
    }

    hostname = new string(TEST_HOSTNAME);
    if (!hostname)
    {
        CU_FAIL("Out of memory when allocated hostname");
        return DownloadRequestPtr();
    }

    url = new string(TEST_URL[index]);
    if (!url)
    {
        delete hostname;
        hostname = NULL;

        CU_FAIL("Out of memory when allocated URL");
        return DownloadRequestPtr();
    }

    filename = new string(TEST_FILENAME[index]);
    if (!filename)
    {
        delete hostname;
        hostname = NULL;

        delete url;
        url = NULL;

        CU_FAIL("Out of memory when allocated filename");
        return DownloadRequestPtr();
    }

    // Take ownership of string object.
    request = new TestDownloadRequest(testInstances,
                                      TEST_PORT,
                                      TEST_RETRY_TIMES,
                                      TEST_DOWNLOAD_PRIORITY[index],
                                      DOWNLOAD_PROTOCOL_UNDEFINED,
                                      shared_ptr<string>(hostname),
                                      shared_ptr<string>(url),
                                      shared_ptr<string>(filename));
    if (!request)
    {
        CU_FAIL("Out of memory when allocated TestDownloadRequest");
        return DownloadRequestPtr();
    }

    return DownloadRequestPtr(request);
}

/*! Create all requests

    @return A vector contained all requests created
*/
std::vector<DownloadRequestPtr>
CreateAllRequests(TestHttpDownloadInstances& testInstances  /*!< A 'TestHttpDownloadInstances' object */
                  )
{
    int i = 0;
    std::vector<DownloadRequestPtr> requests;

    // The last URL is a wrong URL.
    for (i = 0; i < (TEST_REQUEST_COUNT - 1); ++i)
    {
        DownloadRequestPtr request = CreateRequest(testInstances, i);

        // Take ownership of 'TestDownloadRequest' object.
        requests.push_back(request);
    }

    return requests;
}

/*! Create a request by a string of entire URL

    @return A request created
*/
DownloadRequestPtr
CreateRequestByEntireUrl(TestHttpDownloadInstances& testInstances,  /*!< A 'TestHttpDownloadInstances' object */
                         const char* entireUrl,                     /*!< A string of entire URL */
                         const char* filename,                      /*!< A filename to save data of this request */
                         uint32 priority                            /*!< Priority of the request */
                         )
{
    if ((!entireUrl) || (!filename))
    {
        return DownloadRequestPtr();
    }

    // Set DOWNLOAD_PROTOCOL_HTTP as default value.
    DownloadProtocol protocol = DOWNLOAD_PROTOCOL_HTTP;

    // Parse the protocol from URL.
    nbcommon::UrlParser urlParser(entireUrl);
    shared_ptr<string> protocolString = urlParser.GetProtocol();
    if (protocolString && (!(protocolString->empty())))
    {
        const char* protocolChar = protocolString->c_str();
        if (protocolChar && (nsl_strlen(protocolChar) > 0))
        {
            if (nsl_stricmp(protocolChar, HTTP_PROTOCOL_IDENTIFIER) == 0)
            {
                protocol = DOWNLOAD_PROTOCOL_HTTP;
            }
            else if (nsl_stricmp(protocolChar, HTTPS_PROTOCOL_IDENTIFIER) == 0)
            {
                protocol = DOWNLOAD_PROTOCOL_HTTPS;
            }
        }
    }

    // Create the request.
    DownloadRequestPtr request(new TestDownloadRequest(testInstances,
                                                       urlParser.GetPort(),
                                                       TEST_RETRY_TIMES,
                                                       priority,
                                                       protocol,
                                                       urlParser.GetHostname(),
                                                       urlParser.GetLocation(),
                                                       shared_ptr<string>(new string(filename))));
    if (!request)
    {
        CU_FAIL("Out of memory when allocated TestDownloadRequest");
        return DownloadRequestPtr();
    }

    return DownloadRequestPtr(request);
}

/*! Timer callback function for canceling a request

    @return None
*/
void
TimerCallbackForCancelingRequest(PAL_Instance *pal,
                                 void *userData,
                                 PAL_TimerCBReason reason)
{
    NB_Error error = NE_OK;
    TestHttpDownloadInstances* testInstances = (TestHttpDownloadInstances*) userData;

    if (reason == PTCBR_TimerCanceled)
    {
        return;
    }

    CU_ASSERT_PTR_NOT_NULL(testInstances);
    if (testInstances)
    {
        // Get the generic download manager.
        DownloadManager* downloadManager = testInstances->GetGenericDownloadManager();
        CU_ASSERT_PTR_NOT_NULL(downloadManager);

        if (downloadManager)
        {
            // Cancel the request.
            error = downloadManager->CancelRequest(testInstances->m_request);
            CU_ASSERT_EQUAL(error, NE_OK);

            --(testInstances->m_downloadingRequestCount);
            CU_ASSERT_EQUAL(testInstances->m_downloadingRequestCount, 0);
        }
    }

    // Cancel the request successfully.
    SetCallbackCompletedEvent(testInstances->m_event);
}

/*! Timer callback function for clearing requests

    @return None
*/
void
TimerCallbackForClearingRequests(PAL_Instance *pal,
                                 void *userData,
                                 PAL_TimerCBReason reason)
{
    NB_Error error = NE_OK;
    TestHttpDownloadInstances* testInstances = (TestHttpDownloadInstances*) userData;

    if (reason == PTCBR_TimerCanceled)
    {
        return;
    }

    CU_ASSERT_PTR_NOT_NULL(testInstances);
    if (testInstances)
    {
        // Get the generic download manager.
        DownloadManager* downloadManager = testInstances->GetGenericDownloadManager();
        CU_ASSERT_PTR_NOT_NULL(downloadManager);

        if (downloadManager)
        {
            // Clearing requests.
            error = downloadManager->ClearRequests(TEST_DOWNLOAD_LEVEL[0],
                                                   TRUE);
            CU_ASSERT_EQUAL(error, NE_OK);

            testInstances->m_downloadingRequestCount = 0;
        }
    }

    // Clear requests successfully.
    SetCallbackCompletedEvent(testInstances->m_event);
}

/*! Timer callback function for resuming downloading

    @return None
*/
void
TimerCallbackForResumingDownloading(PAL_Instance *pal,
                                    void *userData,
                                    PAL_TimerCBReason reason)
{
    NB_Error error = NE_OK;
    TestHttpDownloadInstances* testInstances = (TestHttpDownloadInstances*) userData;

    if (reason == PTCBR_TimerCanceled)
    {
        return;
    }

    CU_ASSERT_PTR_NOT_NULL(testInstances);
    if (testInstances)
    {
        // Get the generic download manager.
        DownloadManager* downloadManager = testInstances->GetGenericDownloadManager();
        CU_ASSERT_PTR_NOT_NULL(downloadManager);

        if (downloadManager)
        {
            // Resume downloading.
            error = downloadManager->ResumeDownloading(TEST_DOWNLOAD_LEVEL[0]);
            CU_ASSERT_EQUAL(error, NE_OK);
        }
    }
}


// TestDownloadRequest functions ................................................................

/*! TestDownloadRequest constructor */
TestDownloadRequest::TestDownloadRequest(TestHttpDownloadInstances& testInstances,  /*!< A 'TestHttpDownloadInstances' object */
                                         uint16 port,                               /*!< Port for testing */
                                         uint32 retryTimes,                         /*!< Retry times for testing */
                                         uint32 priority,                           /*!< Download priority for testing */
                                         DownloadProtocol protocol,                 /*!< Protocol for testing */
                                         shared_ptr<string> hostname,               /*!< Hostname for testing */
                                         shared_ptr<string> url,                    /*!< URL for testing */
                                         shared_ptr<string> filename                /*!< Filename for testing */
                                         ) : DownloadRequest(priority),
                                             m_testInstances(testInstances),
                                             m_port(port),
                                             m_retryTimes(retryTimes),
                                             m_protocol(protocol),
                                             m_hostname(hostname),
                                             m_url(url),
                                             m_filename(filename)
{
    // Nothing to do here.
}

/*! TestDownloadRequest destructor */
TestDownloadRequest::~TestDownloadRequest()
{
    // Nothing to do here.
}

/*! Write download data to a file

    @return NE_OK if success
*/
NB_Error
TestDownloadRequest::WriteDataToFile()
{
    PAL_Error palError = PAL_Ok;
    uint32 position = 0;
    uint32 dataSizeGot = TEST_DATA_SIZE;
    uint32 remainedSize = 0;
    const char* filename = NULL;
    PAL_Instance* pal = NULL;
    PAL_File* file = NULL;
    uint8 data[TEST_DATA_SIZE] = {0};
    DataStreamPtr stream = GetDataStream();

    if (!m_filename)
    {
        CU_FAIL("Pointer to filename of request is NULL");
        return NE_UNEXPECTED;
    }
    filename = m_filename->c_str();

    pal = m_testInstances.m_pal;
    if (!pal)
    {
        CU_FAIL("Pointer to PAL_Instance is NULL");
        return NE_UNEXPECTED;
    }

    if (!stream)
    {
        CU_FAIL("Pointer to DataStream object of request is NULL");
        return NE_UNEXPECTED;
    }

    PAL_FileRemove(pal, filename);

    palError = PAL_FileOpen(pal, filename, PFM_Create, &file);
    CU_ASSERT_EQUAL(palError, PAL_Ok);
    if (palError != PAL_Ok)
    {
        return NE_FSYS;
    }

    remainedSize = stream->GetDataSize();
    CU_ASSERT(remainedSize > 0);
    while (dataSizeGot == TEST_DATA_SIZE)
    {
        dataSizeGot = stream->GetData(data, position, TEST_DATA_SIZE);
        if (dataSizeGot > 0)
        {
            uint32 bytesWritten = 0;

            palError = PAL_FileWrite(file, data, dataSizeGot, &bytesWritten);
            CU_ASSERT_EQUAL(palError, PAL_Ok);
            if (palError != PAL_Ok)
            {
                return NE_FSYS;
            }

            position += dataSizeGot;

            palError = PAL_FileSetPosition(file, PFSO_Start, position);
            CU_ASSERT_EQUAL(palError, PAL_Ok);
            if (palError != PAL_Ok)
            {
                return NE_FSYS;
            }

            remainedSize -= dataSizeGot;
        }
    }
    CU_ASSERT_EQUAL(remainedSize, 0);

    palError = PAL_FileClose(file);
    CU_ASSERT_EQUAL(palError, PAL_Ok);
    if (palError != PAL_Ok)
    {
        return NE_FSYS;
    }

    return NE_OK;
}

/* See description in 'downloadrequest.h' */
shared_ptr<string>
TestDownloadRequest::GetHostname()
{
    return m_hostname;
}

/* See description in 'downloadrequest.h' */
DownloadProtocol
TestDownloadRequest::GetProtocol()
{
    return m_protocol;
}

/* See description in 'downloadrequest.h' */
uint16
TestDownloadRequest::GetPort()
{
    return m_port;
}

/* See description in 'downloadrequest.h' */
shared_ptr<string>
TestDownloadRequest::GetUrl()
{
    return m_url;
}

/* See description in 'downloadrequest.h' */
uint32
TestDownloadRequest::GetRetryTimes()
{
    return m_retryTimes;
}

/* See description in 'downloadrequest.h' */
void
TestDownloadRequest::AddedToDownloadQueue(DownloadRequestPtr request)
{
    // Nothing to do here.
}

/* See description in 'downloadrequest.h' */
void
TestDownloadRequest::DuplicatedRequestWillBeIgnored(DownloadRequestPtr duplicatedRequest)
{
    // Nothing to do here.
}

/* See description in 'downloadrequest.h' */
void
TestDownloadRequest::RemovedFromDownloadQueue(DownloadRequestPtr request)
{
    // Nothing to do here.
}

/* See description in 'downloadrequest.h' */
void
TestDownloadRequest::DataAvailable(uint32 size)
{
    if (size == 0)
    {
        if (m_testInstances.m_requestIndex < (TEST_REQUEST_COUNT - 1))
        {
            shared_ptr<string> testUrl(new string(TEST_URL[m_testInstances.m_requestIndex]));
            if (!StringUtility::IsStringEqual(testUrl, GetUrl()))
            {
                CU_FAIL("Responsed order is wrong");
                SetCallbackCompletedEvent(m_testInstances.m_event);
                return;
            }

            ++(m_testInstances.m_requestIndex);
        }

        if ((--(m_testInstances.m_downloadingRequestCount)) == 0)
        {
            // All download requests succeed.
            SetCallbackCompletedEvent(m_testInstances.m_event);
        }
    }
}

/* See declaration for description */
void
TestDownloadRequest::DataError(NB_Error error)
{
    // Save the downloading error in TestHttpDownloadInstances object.
    m_testInstances.m_downloadingError = error;

    // A download request fails.
    SetCallbackCompletedEvent(m_testInstances.m_event);
}


// TestHttpDownloadInstances functions ..........................................................

/*! TestHttpDownloadInstances constructor */
TestHttpDownloadInstances::TestHttpDownloadInstances()
: m_downloadingError(NE_OK),
  m_downloadingRequestCount(0),
  m_requestIndex(0),
  m_pal(NULL),
  m_context(NULL),
  m_event(NULL),
  m_request()
{
    // Nothing to do here.
}

/*! TestHttpDownloadInstances destructor */
TestHttpDownloadInstances::~TestHttpDownloadInstances()
{
    Reset();
}

/*! Initialize a 'TestHttpDownloadInstances' object

    @return NE_OK if success,
            NE_EXIST if this object has already been initialized,
            NE_NOTINIT if this object is not properly initialized.
*/
NB_Error
TestHttpDownloadInstances::Initialize(uint32 httpConnectionCount,   /*!< Count of connections for the generic HTTP download manager */
                                      uint32 initialRequestIndex    /*!< Initial request index. If this value is not less than TEST_REQUEST_COUNT
                                                                         in tile request callback function, the callback completed event is set. */
                                      )
{
    NB_Error error = NE_OK;

    // Check if this 'TestHttpDownloadInstances' object has already been initialized.
    if (m_pal && m_context && m_event)
    {
        return NE_EXIST;
    }

    // Check if this 'TestHttpDownloadInstances' object is not properly initialized.
    if (m_pal || m_context || m_event)
    {
        return NE_NOTINIT;
    }

    // Create a PAL and context.
    if (!CreatePalAndContextWithHttpConnectionCount(httpConnectionCount, &m_pal, &m_context))
    {
        Reset();
        return NE_NOTINIT;
    }
    CU_ASSERT_PTR_NOT_NULL(m_pal);
    CU_ASSERT_PTR_NOT_NULL(m_context);

    // Create an event.
    m_event = CreateCallbackCompletedEvent();
    CU_ASSERT_PTR_NOT_NULL(m_event);
    if (!m_event)
    {
        Reset();
        return NE_NOTINIT;
    }

    if (error == NE_OK)
    {
        m_requestIndex = initialRequestIndex;
    }

    return error;
}

/*! Reset a 'TestHttpDownloadInstances' object to be uninitialized

    Destroy the PAL, context and event in the 'TestHttpDownloadInstances' object.

    @return None
*/
void
TestHttpDownloadInstances::Reset()
{
    if (m_event)
    {
        DestroyCallbackCompletedEvent(m_event);
        m_event = NULL;
    }

    if (m_context)
    {
        NB_Error error = NE_OK;

        error = NB_ContextDestroy(m_context);
        CU_ASSERT_EQUAL(error, NE_OK);
        m_context = NULL;
    }

    if (m_pal)
    {
        PAL_Destroy(m_pal);
        m_pal = NULL;
    }

    m_request = DownloadRequestPtr();
    m_downloadingRequestCount = 0;
    m_downloadingError = NE_OK;
}

/*! Get the generic download manager

    @return A pointer to DownloadManager object
*/
DownloadManager*
TestHttpDownloadInstances::GetGenericDownloadManager()
{
    return (DownloadManager*) NB_ContextGetGenericHttpDownloadManager(m_context);
}

/*! @} */
