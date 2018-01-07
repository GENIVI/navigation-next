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

    @file       testoffboardconverttilemanager.cpp

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
    #include "networkutil.h"
    #include "platformutil.h"
    #include "palfile.h"
}

#include "CUnit.h"
#include "testoffboardconverttilemanager.h"
#include "OffboardConvertTileManager.h"
#include "OffboardTileType.h"

using namespace std;
using namespace nbcommon;
using namespace nbmap;

// Local Constants ..............................................................................

/* These two priorities are different. First is used to calculate download level in generic
   download manager. Second is priority of tile request in the download level (queue).
*/


static const uint32 TEST_DOWNLOAD_PRIORITY = 4;
static const uint32 TEST_TILE_REQUEST_PRIORITY = 0;

static const uint16 TEST_TILE_REQUEST_PORT = 80;
static const uint32 TEST_TILE_REQUEST_RETRY_TIMES = 3;
static const uint32 TEST_TILE_REQUEST_MAX_RETRY_TIMES = 0;
static const uint32 TEST_DATA_SIZE = 4096;

static const char TEST_TILE_DATA_TYPE[] = "LAM";
static const char TEST_ENTIRE_URL_TEMPLATE[] = "http://qa14dts.navbuilder.nimlbs.net:8080/unified_map?type=LAM&gen=1755811585&fmt=$fmt&x=$x&y=$y&vfmt=$vfmt";
static const char TEST_CONTENT_ID_TEPLATE[] = "T_LAM_1_$fmt_$vfmt_$x_$y";

static const char TEST_KEY_FORMAT[] = "$fmt";
static const char TEST_VALUE_FORMAT[] = "PNG";

static const char TEST_KEY_FORMAT_VERSION[] = "$vfmt";
static const char TEST_VALUE_FORMAT_VERSION[] = "30";

static const char TEST_METADATA_PARAMETER_LOCALE[] = "$loc";
static const char TEST_METADATA_PARAMETER_RESOLUTION[] = "$res";
static const char TEST_METADATA_PARAMETER_SIZE[] = "$sz";
static const char TEST_METADATA_PARAMETER_VERSION[] = "$v";

static const char TEST_METADATA_DEFAULT_LOCALE[] = "en-US";
static const char TEST_METADATA_DEFAULT_RESOLUTION[] = "192";
static const char TEST_METADATA_DEFAULT_TILE_SIZE[] = "256";
static const char TEST_METADATA_DEFAULT_VERSION[] = "1.0";

static const uint32 TEST_DEFAULT_MAX_REQUEST_COUNT = 16;
static const uint32 TEST_BEYOND_MAX_REQUEST_COUNT = 2;
static const uint32 TEST_TILE_KEY_COUNT = 5;

static const char TEST_KEY_X[] = "$x";
static const char TEST_VALUE_X[TEST_TILE_KEY_COUNT][8] = { "35", "35", "36", "36", "37" };

static const char TEST_KEY_Y[] = "$y";
static const char TEST_VALUE_Y[TEST_TILE_KEY_COUNT][8] = { "54", "55", "55", "56", "57" };

// Count of HTTP download connections for testing
static const uint32 TEST_DEFAULT_HTTP_CONNECTION_COUNT = 6;
static const uint32 TEST_HTTP_CONNECTION_COUNT_FOR_PRIORITY = 1;

// Local Types ..................................................................................

/*! Collection of all instances used for the unit tests */
class TestOffboardConvertTileInstances
{
public:
    /* See definition for description */

    TestOffboardConvertTileInstances();
    virtual ~TestOffboardConvertTileInstances();
    NB_Error Initialize(uint32 httpConnectionCount = TEST_DEFAULT_HTTP_CONNECTION_COUNT,
                        uint32 initialRequestIndex = TEST_TILE_KEY_COUNT,
                        uint32 maximumRequestCount = TEST_DEFAULT_MAX_REQUEST_COUNT);
    void Reset();

    NB_Error m_requestError;    /*!< Error during requesting tiles */
    uint32 m_requestIndex;      /*!< Index of request. If this value is not less than TEST_TILE_REQUEST_COUNT
                                     in tile request callback function, the callback completed event is set. */
    PAL_Instance* m_pal;        /*!< Pointer to PAL instance */
    NB_Context* m_context;      /*!< Pointer to current context */
    void* m_event;              /*!< Callback completed event */
    OffboardConvertTileManagerPtr m_tileManager;   /*!< An OffboardConvertTileManager object */
};

/*! Test callback object for getting data of tile */
class TestOffboardConvertCallback : public AsyncCallbackWithRequest<shared_ptr<map<string, string> >, TilePtr>
{
public:
    /* See definition for description */

    TestOffboardConvertCallback(TestOffboardConvertTileInstances& testInstances);
    virtual ~TestOffboardConvertCallback();

    /* See description in AsyncCallback.h */
    virtual void Success(shared_ptr<map<string, string> > request,
                         TilePtr response);
    virtual void Error(shared_ptr<map<string, string> > request,
                       NB_Error error);
    virtual bool Progress(int percentage);

    // Copy constructor and assignment operator are not supported.
    TestOffboardConvertCallback(const TestOffboardConvertCallback& callback);
    TestOffboardConvertCallback& operator=(const TestOffboardConvertCallback& callback);

    TestOffboardConvertTileInstances& m_testInstances; /*!< A TestOffboardConvertTileInstances object */
};


// Test Functions ...............................................................................

static void TestOffboardConvertTileManagerInitialization();
static void TestOffboardConvertTileManagerGetTileWithoutCallback();
static void TestOffboardConvertTileManagerGetTileWithCallback();
static void TestOffboardConvertTileManagerGetTileWithCallbackFast();
static void TestOffboardConvertTileManagerRemoveAllTiles();
static void TestOffboardConvertTileManagerGetTileWithPriority();
static void TestOffboardConvertTileManagerBeyondMaximumRequestCount();

// Helper functions .............................................................................

shared_ptr<map<string, string> > CreateOffboardConvertTileRequestParameters(const char valueX[],
                                                                     const char valueY[]);
bool OffboardConvertTileRequestEqual(shared_ptr<map<string, string> > request,
                                     const char valueX[],
                                     const char valueY[]);


// Public Functions .............................................................................

/*! Add all your test functions here

    @return None
*/
void
TestOffboardConvertTileManager_AddAllTests(CU_pSuite pTestSuite)
{
    // ! Add all your function names here !
    CU_add_test(pTestSuite, "TestOffboardConvertTileManagerInitialization", &TestOffboardConvertTileManagerInitialization);
    CU_add_test(pTestSuite, "TestOffboardConvertTileManagerGetTileWithoutCallback", &TestOffboardConvertTileManagerGetTileWithoutCallback);
    CU_add_test(pTestSuite, "TestOffboardConvertTileManagerGetTileWithCallback", &TestOffboardConvertTileManagerGetTileWithCallback);
    CU_add_test(pTestSuite, "TestOffboardConvertTileManagerGetTileWithCallbackFast", &TestOffboardConvertTileManagerGetTileWithCallbackFast);
    CU_add_test(pTestSuite, "TestOffboardConvertTileManagerRemoveAllTiles", &TestOffboardConvertTileManagerRemoveAllTiles);
    CU_add_test(pTestSuite, "TestOffboardConvertTileManagerGetTileWithPriority", &TestOffboardConvertTileManagerGetTileWithPriority);
    CU_add_test(pTestSuite, "TestOffboardConvertTileManagerBeyondMaximumRequestCount", &TestOffboardConvertTileManagerBeyondMaximumRequestCount);
};

/*! Add common initialization code here

    @return 0
*/
int
TestOffboardConvertTileManager_SuiteSetup()
{
    return 0;
}

/*! Add common cleanup code here

    @return 0
*/
int
TestOffboardConvertTileManager_SuiteCleanup()
{
    return 0;
}

extern TileLayerInfoPtr CreateDefaulltTileLayerInfo();

// Test Functions ...............................................................................

/*! Test initializing an OffboardConvertTileManager object

    @return None. CUnit Asserts get called on failures.
*/
void
TestOffboardConvertTileManagerInitialization()
{
    NB_Error error = NE_OK;
    TestOffboardConvertTileInstances testInstances;

    // Initialize an OffboardConvertTileManager object in the function TestOffboardConvertTileInstances::Initialize.
    error = testInstances.Initialize();
    if (error != NE_OK)
    {
        CU_FAIL("TestOffboardConvertTileInstances::Initialize() failed");
        return;
    }
}

/*! Test requesting a tile without a callback

    @return None. CUnit Asserts get called on failures.
*/
void
TestOffboardConvertTileManagerGetTileWithoutCallback()
{
    NB_Error error = NE_OK;
    TestOffboardConvertTileInstances testInstances;
    OffboardConvertTileManagerPtr tileManager;
    shared_ptr<map<string, string> > templateParameters;

    error = testInstances.Initialize();
    if (error != NE_OK)
    {
        CU_FAIL("TestOffboardConvertTileInstances::Initialize() failed");
        return;
    }

    // Get the tile manager.
    tileManager = testInstances.m_tileManager;
    if (!tileManager)
    {
        CU_FAIL("Pointer to tile manager is NULL");
        return;
    }

    // Create the parameters to request a tile.
    templateParameters = CreateOffboardConvertTileRequestParameters(TEST_VALUE_X[0],
                                                                    TEST_VALUE_Y[0]);
    if (!templateParameters)
    {
        CU_FAIL("Parameters to request a tile is NULL");
        return;
    }

    // Request a tile.
    tileManager->GetTile(templateParameters,
                         TEST_TILE_REQUEST_PRIORITY);

    WaitForCallbackCompletedEvent(testInstances.m_event, 10000);

    // Check if an error occurred during requesting tile.
    if (testInstances.m_requestError != NE_OK)
    {
        CU_FAIL("An error occurred during requesting tile");
    }
}

/*! Test requesting a previously cached tile with a callback

    @return None. CUnit Asserts get called on failures.
*/
void
TestOffboardConvertTileManagerGetTileWithCallback()
{
    NB_Error error = NE_OK;
    TestOffboardConvertCallback* callback = NULL;
    TestOffboardConvertTileInstances testInstances;
    OffboardConvertTileManagerPtr tileManager;
    shared_ptr<map<string, string> > templateParameters;

    error = testInstances.Initialize();
    if (error != NE_OK)
    {
        CU_FAIL("TestOffboardConvertTileInstances::Initialize() failed");
        return;
    }

    // Get the tile manager.
    tileManager = testInstances.m_tileManager;
    if (!tileManager)
    {
        CU_FAIL("Pointer to tile manager is NULL");
        return;
    }

    // Create the parameters to request a tile.
    templateParameters = CreateOffboardConvertTileRequestParameters(TEST_VALUE_X[0],
                                                                    TEST_VALUE_Y[0]);
    if (!templateParameters)
    {
        CU_FAIL("Parameters to request a tile is NULL");
        return;
    }

    // Create a callback object.
    callback = new TestOffboardConvertCallback(testInstances);
    if (!callback)
    {
        CU_FAIL("Out of memory when allocated callback");
        return;
    }

    // Request a tile with the flag 'fastLoadOnly' false.
    tileManager->GetTile(shared_ptr<std::map<std::string, std::string> >(templateParameters),
                         shared_ptr<AsyncCallbackWithRequest<shared_ptr<std::map<std::string, std::string> >, TilePtr> >(callback),
                         TEST_TILE_REQUEST_PRIORITY,
                         false);

    if (!WaitForCallbackCompletedEvent(testInstances.m_event, 10000))
    {
        CU_FAIL("OffboardConvertTileManager::GetTile timeout");
        return;
    }

    // Check if an error occurred during requesting tile.
    if (testInstances.m_requestError != NE_OK)
    {
        CU_FAIL("An error occurred during requesting tile");
    }
}

/*! Test requesting a previously cached tile in fast only mode with a callback

    @return None. CUnit Asserts get called on failures.
*/
void
TestOffboardConvertTileManagerGetTileWithCallbackFast()
{
    NB_Error error = NE_OK;
    TestOffboardConvertCallback* callback = NULL;
    TestOffboardConvertTileInstances testInstances;
    OffboardConvertTileManagerPtr tileManager;
    shared_ptr<map<string, string> > templateParameters;

    error = testInstances.Initialize();
    if (error != NE_OK)
    {
        CU_FAIL("TestOffboardConvertTileInstances::Initialize() failed");
        return;
    }

    // Get the tile manager.
    tileManager = testInstances.m_tileManager;
    if (!tileManager)
    {
        CU_FAIL("Pointer to tile manager is NULL");
        return;
    }

    // Create the parameters to request a tile.
    templateParameters = CreateOffboardConvertTileRequestParameters(TEST_VALUE_X[0],
                                                                    TEST_VALUE_Y[0]);
    if (!templateParameters)
    {
        CU_FAIL("Parameters to request a tile is NULL");
        return;
    }

    // Create a callback object.
    callback = new TestOffboardConvertCallback(testInstances);
    if (!callback)
    {
        CU_FAIL("Out of memory when allocated callback");
        return;
    }

    // Request a tile with the flag 'fastLoadOnly' true.
    tileManager->GetTile(templateParameters,
                         shared_ptr<AsyncCallbackWithRequest<shared_ptr<map<string, string> >, TilePtr> >(callback),
                         TEST_TILE_REQUEST_PRIORITY,
                         true);

    if (!WaitForCallbackCompletedEvent(testInstances.m_event, 100))
    {
        CU_FAIL("OffboardConvertTileManager::GetTile timeout");
        return;
    }

    // Check if an error occurred during requesting tile.
    if (testInstances.m_requestError == NE_OK)
    {
        CU_FAIL("Should fail but don't");
    }
}

/*! Test removing all tiles

    @return None. CUnit Asserts get called on failures.
*/
void
TestOffboardConvertTileManagerRemoveAllTiles()
{
    NB_Error error = NE_OK;
    TestOffboardConvertTileInstances testInstances;
    OffboardConvertTileManagerPtr tileManager;

    error = testInstances.Initialize();
    if (error != NE_OK)
    {
        CU_FAIL("TestOffboardConvertTileInstances::Initialize() failed");
        return;
    }

    // Get the tile manager.
    tileManager = testInstances.m_tileManager;
    if (!tileManager)
    {
        CU_FAIL("Pointer to tile manager is NULL");
        return;
    }

    // Remove all tiles.
    tileManager->RemoveAllTiles();
}

/*! Test requesting tiles with different priorities

    Five requests are sent in this function. Requested order is:

    Tile        Priority
    0           0 (highest)
    3           3 (highest + 3)
    4           3 (highest + 3)
    2           2 (highest + 2)
    1           1 (highest + 1)

    Because lower number has higher priority. Expected responsed order
    is 0-1-2-3-4.

    @return None. CUnit Asserts get called on failures.
*/
void
TestOffboardConvertTileManagerGetTileWithPriority()
{
    NB_Error error = NE_OK;
    const uint32 highestPriority = 0;
    TestOffboardConvertTileInstances testInstances;
    OffboardConvertTileManagerPtr tileManager;
    shared_ptr<map<string, string> > templateParameters;

    error = testInstances.Initialize(TEST_HTTP_CONNECTION_COUNT_FOR_PRIORITY,
                                     0);
    if (error != NE_OK)
    {
        CU_FAIL("TestOffboardTileInstances::Initialize() failed");
        return;
    }

    // Get the tile manager.
    tileManager = testInstances.m_tileManager;
    if (!tileManager)
    {
        CU_FAIL("Pointer to tile manager is NULL");
        return;
    }

    // Create a callback object.
    shared_ptr<AsyncCallbackWithRequest<shared_ptr<map<string, string> >, TilePtr> > callback(new TestOffboardConvertCallback(testInstances));
    if (!callback)
    {
        CU_FAIL("Out of memory when allocated callback");
        return;
    }

    // Request tile 0 with (highest) priority.
    tileManager->GetTile(CreateOffboardConvertTileRequestParameters(TEST_VALUE_X[0], TEST_VALUE_Y[0]),
                         callback,
                         highestPriority,
                         false);

    // Request tile 3 with (highest + 3) priority.
    tileManager->GetTile(CreateOffboardConvertTileRequestParameters(TEST_VALUE_X[3], TEST_VALUE_Y[3]),
                         callback,
                         highestPriority + 3,
                         false);

    // Request tile 4 with (highest + 3) priority.
    tileManager->GetTile(CreateOffboardConvertTileRequestParameters(TEST_VALUE_X[4], TEST_VALUE_Y[4]),
                         callback,
                         highestPriority + 3,
                         false);

    // Request tile 2 with (highest + 2) priority.
    tileManager->GetTile(CreateOffboardConvertTileRequestParameters(TEST_VALUE_X[2], TEST_VALUE_Y[2]),
                         callback,
                         highestPriority + 2,
                         false);

    // Request tile 1 with (highest + 1) priority.
    tileManager->GetTile(CreateOffboardConvertTileRequestParameters(TEST_VALUE_X[1], TEST_VALUE_Y[1]),
                         callback,
                         highestPriority + 1,
                         false);

    if (!WaitForCallbackCompletedEvent(testInstances.m_event, 60000))
    {
        CU_FAIL("OffboardTileManager::GetTile timeout");
        return;
    }

    // Check if an error occurred durng requesting tiles.
    if (testInstances.m_requestError != NE_OK)
    {
        CU_FAIL("An error occurred during requesting tile");
    }
}

/*! Test requesting tiles beyond maximum count of requests

    Five requests are sent in this function. Requested order is:

    Tile        Priority
    0           0 (highest)
    4           4 (highest + 4)
    3           3 (highest + 3)
    2           2 (highest + 2)
    1           1 (highest + 1)

    Because setting of maximum requests ignores priority. And request of
    tile 0 is downloading in progress and it cannot be removed. So expected
    responsed order is 0-1.

    @return None. CUnit Asserts get called on failures.
*/
void
TestOffboardConvertTileManagerBeyondMaximumRequestCount()
{
    NB_Error error = NE_OK;
    const uint32 highestPriority = 0;
    TestOffboardConvertTileInstances testInstances;
    OffboardConvertTileManagerPtr tileManager;
    shared_ptr<map<string, string> > templateParameters;

    error = testInstances.Initialize(TEST_HTTP_CONNECTION_COUNT_FOR_PRIORITY,
                                     0,
                                     TEST_BEYOND_MAX_REQUEST_COUNT);
    if (error != NE_OK)
    {
        CU_FAIL("TestOffboardTileInstances::Initialize() failed");
        return;
    }

    // Get the tile manager.
    tileManager = testInstances.m_tileManager;
    if (!tileManager)
    {
        CU_FAIL("Pointer to tile manager is NULL");
        return;
    }

    // Create a callback object.
    shared_ptr<AsyncCallbackWithRequest<shared_ptr<map<string, string> >, TilePtr> > callback(new TestOffboardConvertCallback(testInstances));
    if (!callback)
    {
        CU_FAIL("Out of memory when allocated callback");
        return;
    }

    // Request tile 0 with (highest) priority.
    tileManager->GetTile(CreateOffboardConvertTileRequestParameters(TEST_VALUE_X[0], TEST_VALUE_Y[0]),
                         callback,
                         highestPriority,
                         false);

    // Request tile 4 with (highest + 4) priority.
    tileManager->GetTile(CreateOffboardConvertTileRequestParameters(TEST_VALUE_X[4], TEST_VALUE_Y[4]),
                         callback,
                         highestPriority + 4,
                         false);

    // Request tile 3 with (highest + 3) priority.
    tileManager->GetTile(CreateOffboardConvertTileRequestParameters(TEST_VALUE_X[3], TEST_VALUE_Y[3]),
                         callback,
                         highestPriority + 3,
                         false);

    // Request tile 2 with (highest + 2) priority.
    tileManager->GetTile(CreateOffboardConvertTileRequestParameters(TEST_VALUE_X[2], TEST_VALUE_Y[2]),
                         callback,
                         highestPriority + 2,
                         false);

    // Request tile 1 with (highest + 1) priority.
    tileManager->GetTile(CreateOffboardConvertTileRequestParameters(TEST_VALUE_X[1], TEST_VALUE_Y[1]),
                         callback,
                         highestPriority + 1,
                         false);

    WaitForCallbackCompletedEvent(testInstances.m_event, 10000);

    // Check if an error occurred durng requesting tiles, requests may be ignored or need to retry again.
    if ((testInstances.m_requestError != NE_OK) &&
        (testInstances.m_requestError != NE_IGNORED) &&
        (testInstances.m_requestError != NE_AGAIN))
    {
        CU_FAIL("An error occurred during requesting tile");
        return;
    }

    // Final index less or equals maximum count of requests.
    CU_ASSERT(testInstances.m_requestIndex <= TEST_BEYOND_MAX_REQUEST_COUNT);
}

// Helper functions .............................................................................

/*! Create the parameters to request a tile

    @return A template parameter map to request a tile
*/
shared_ptr<map<string, string> >
CreateOffboardConvertTileRequestParameters(const char valueX[], const char valueY[])
{
    shared_ptr<map<string, string> > parameters = shared_ptr<map<string, string> >(new map<string, string>());

    if (!parameters)
    {
        return shared_ptr<map<string, string> >();
    }

    parameters->insert(pair<string, string>(TEST_KEY_X, valueX));
    parameters->insert(pair<string, string>(TEST_KEY_Y, valueY));

    return parameters;
}

/*! Check if a request is equal with values of X and Y

    @return true if a request is equal with values of X and Y, false otherwise.
*/
bool OffboardConvertTileRequestEqual(shared_ptr<map<string, string> > request,
                                     const char valueX[],
                                     const char valueY[])
{
    if (!request)
    {
        return FALSE;
    }

    return (((*request)[TEST_KEY_X] == valueX) &&
            ((*request)[TEST_KEY_Y] == valueY));
}

/* Write data to a file

    @return NE_OK if success
*/
NB_Error
WriteConvertDataToFile(PAL_Instance* pal,
                       const string& filename,
                       DataStreamPtr dataStream
                       )
{
    PAL_Error palError = PAL_Ok;
    uint32 position = 0;
    uint32 dataSizeGot = TEST_DATA_SIZE;
    uint32 remainedSize = 0;
    PAL_File* file = NULL;
    uint8 data[TEST_DATA_SIZE] = {0};

    if ((!pal) || filename.empty() || (!dataStream))
    {
        CU_FAIL("Parameters of WriteConvertDataToFile is invalid");
        return NE_INVAL;
    }

    PAL_FileRemove(pal, filename.c_str());

    palError = PAL_FileOpen(pal, filename.c_str(), PFM_Create, &file);
    CU_ASSERT_EQUAL(palError, PAL_Ok);
    if (palError != PAL_Ok)
    {
        return NE_FSYS;
    }

    remainedSize = dataStream->GetDataSize();
    CU_ASSERT(remainedSize > 0);
    while (dataSizeGot == TEST_DATA_SIZE)
    {
        dataSizeGot = dataStream->GetData(data, position, TEST_DATA_SIZE);
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

// TestOffboardTileInstances functions ..........................................................

/*! TestOffboardTileInstances constructor */
TestOffboardConvertTileInstances::TestOffboardConvertTileInstances() : m_requestError(NE_OK),
                                                         m_requestIndex(0),
                                                         m_pal(NULL),
                                                         m_context(NULL),
                                                         m_event(NULL),
                                                         m_tileManager()
{
    // Nothing to do here.
}

/*! TestOffboardTileInstances destructor */
TestOffboardConvertTileInstances::~TestOffboardConvertTileInstances()
{
    Reset();
}

/*! Initialize a TestOffboardTileInstances object

    @return NE_OK if success,
            NE_EXIST if this object has already been initialized,
            NE_NOTINIT if this object is not properly initialized.
*/
NB_Error
TestOffboardConvertTileInstances::Initialize(uint32 httpConnectionCount,   /*!< Count of connections for the generic HTTP download manager */
                                             uint32 initialRequestIndex,   /*!< Initial request index. If this value is not less than TEST_TILE_REQUEST_COUNT
                                                                           in tile request callback function, the callback completed event is set. */
                                             uint32 maximumRequestCount    /*!< Maximum count of requests to download tiles */
                                             )
{
    NB_Error error = NE_OK;

    // Check if this TestOffboardTileInstances object has already been initialized.
    if (m_pal && m_context && m_event && m_tileManager)
    {
        return NE_EXIST;
    }

    // Check if this TestOffboardTileInstances object is not properly initialized.
    if (m_pal || m_context || m_event || m_tileManager)
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

    // Create an OffboardTileManager object.
    {
        // Create an OffboardTileType object to initialize the tile manager.
        OffboardTileType* tileType = new OffboardTileType(shared_ptr<string>(new string(TEST_TILE_DATA_TYPE)),
                                                          shared_ptr<string>(new string(TEST_ENTIRE_URL_TEMPLATE)),
                                                          shared_ptr<string>(new string(TEST_CONTENT_ID_TEPLATE)));
        if (!tileType)
        {
            Reset();
            return NE_NOMEM;
        }

        RasterTileConverter* tileConverter = new RasterTileConverter();

        if (!tileConverter)
        {
            delete tileType;
            tileType = NULL;

            Reset();
            return NE_NOMEM;
        }

        // Create and initialize an OffboardConvertTileManager object with the OffboardTileType object.
        m_tileManager = OffboardConvertTileManagerPtr(new OffboardConvertTileManager());

        if (!m_tileManager)
        {
            delete tileType;
            tileType = NULL;
            delete tileConverter;
            tileConverter = NULL;

            Reset();
            return NE_NOMEM;
        }

        TileLayerInfoPtr info = CreateDefaulltTileLayerInfo();

        // Take ownership of the OffboardConvertTileType object.
        error = m_tileManager->Initialize(m_context,
                                          false,
                                          TEST_DOWNLOAD_PRIORITY,
                                          TEST_TILE_REQUEST_RETRY_TIMES,
                                          TEST_TILE_REQUEST_MAX_RETRY_TIMES,
                                          maximumRequestCount,
                                          TileTypePtr(tileType),
                                          TileConverterPtr(tileConverter),
                                          info);
        if (error != NE_OK)
        {
            Reset();
            return error;
        }

        // Set the common parameters for the tile manager.
        {
            map<string, string>* commonParameters = NULL;

            commonParameters = new map<string, string>();
            if (!commonParameters)
            {
                Reset();
                return NE_NOMEM;
            }

            commonParameters->insert(pair<string, string>(TEST_KEY_FORMAT, TEST_VALUE_FORMAT));
            commonParameters->insert(pair<string, string>(TEST_KEY_FORMAT_VERSION, TEST_VALUE_FORMAT_VERSION));
            commonParameters->insert(pair<string, string>(TEST_METADATA_PARAMETER_VERSION, TEST_METADATA_DEFAULT_VERSION));
            commonParameters->insert(pair<string, string>(TEST_METADATA_PARAMETER_LOCALE, TEST_METADATA_DEFAULT_LOCALE));
            commonParameters->insert(pair<string, string>(TEST_METADATA_PARAMETER_RESOLUTION, TEST_METADATA_DEFAULT_RESOLUTION));
            commonParameters->insert(pair<string, string>(TEST_METADATA_PARAMETER_SIZE, TEST_METADATA_DEFAULT_TILE_SIZE));

            // Take ownership of the common parameter map object.
            error = m_tileManager->SetCommonParameterMap(shared_ptr<std::map<std::string, std::string> > (commonParameters));
            if (error != NE_OK)
            {
                Reset();
                return error;
            }
        }
    }

    if (error == NE_OK)
    {
        m_requestIndex = initialRequestIndex;
    }

    return error;
}

/*! Reset a TestOffboardConvertTileInstances object to be uninitialized

    Destroy the PAL, context, event and OffboardConvertTileManager object in the
    TestOffboardConvertTileInstances object.

    @return None
*/
void
TestOffboardConvertTileInstances::Reset()
{
    m_tileManager = OffboardConvertTileManagerPtr();

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

    m_requestIndex = 0;
    m_requestError = NE_OK;
}

// TestOffboardConvertCallback functions ...............................................................

/*! TestOffboardConvertCallback constructor */
TestOffboardConvertCallback::TestOffboardConvertCallback(TestOffboardConvertTileInstances& testInstances /*!< A TestOffboardConvertTileInstances object */
                                                 ) : m_testInstances(testInstances)
{
    // Nothing to do here.
}

/*! TestOffboardConvertCallback destructor */
TestOffboardConvertCallback::~TestOffboardConvertCallback()
{
    // Nothing to do here.
}

/* See description in AsyncCallback.h */
void
TestOffboardConvertCallback::Success(shared_ptr<map<string, string> > request,
                                     TilePtr response)
{
    NB_Error error = NE_OK;
    shared_ptr<string> dataType;
    shared_ptr<string> contentId;
    DataStreamPtr dataStream;

    if (!response)
    {
        CU_FAIL("Pointer to responsed tile is NULL");
        SetCallbackCompletedEvent(m_testInstances.m_event);
        return;
    }

    dataType = response->GetDataType();
    contentId = response->GetContentID();
    dataStream = response->GetData();

    if ((!dataType) || (!contentId) || (!dataStream))
    {
        CU_FAIL("Responsed tile is invalid");
        SetCallbackCompletedEvent(m_testInstances.m_event);
        return;
    }

    error = WriteConvertDataToFile(m_testInstances.m_pal, *contentId, dataStream);

    if (error != NE_OK)
    {
        CU_FAIL("WriteConvertDataToFile fails");
        SetCallbackCompletedEvent(m_testInstances.m_event);
        return;
    }

    if (m_testInstances.m_requestIndex >= (TEST_TILE_KEY_COUNT - 1))
    {
        // The request succeeds.
        SetCallbackCompletedEvent(m_testInstances.m_event);
    }
    else
    {
        if (!OffboardConvertTileRequestEqual(request,
                                             TEST_VALUE_X[m_testInstances.m_requestIndex],
                                             TEST_VALUE_Y[m_testInstances.m_requestIndex]))
        {
            CU_FAIL("Responsed order is wrong");
            SetCallbackCompletedEvent(m_testInstances.m_event);
            return;
        }

        ++(m_testInstances.m_requestIndex);
    }
}

/* See description in AsyncCallback.h */
void
TestOffboardConvertCallback::Error(shared_ptr<map<string, string> > request,
                                   NB_Error error)
{
    // Save the request error in TestOffboardTileInstances object.
    m_testInstances.m_requestError = error;
    // The request fails.
    SetCallbackCompletedEvent(m_testInstances.m_event);
}

/* See description in AsyncCallback.h */
bool
TestOffboardConvertCallback::Progress(int percentage)
{
    // Nothing to do here.
    return false;
}

/*! @} */
