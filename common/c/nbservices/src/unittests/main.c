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

@file     Main.c
@date     01/01/09

Main source for unit tests

*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "Basic.h"
#include "CUnit.h"

#include "main.h"
#include "testspatial.h"
#include "testcontentmanager.h"
#include "testmetadata.h"
#include "testdownload.h"
#include "testmobilecoupons.h"
#include "testhttpdownloadmanager.h"
#include "testoffboardtilemanager.h"
#include "testoffboardconverttilemanager.h"
#include "testpersistentdata.h"
#include "testcachingtilemanager.h"
#include "testcache.h"
#include "testtuple.h"
#include "TestMap.h"
#include "TestLamProcessor.h"
//#include "testpinlayer.h"
//#include "testpinmanager.h"
//#include "TestPinCushion.h"
#include "TestSingleSearchProtocol.h"

// Constants

// All test suites
static const char testSuiteSpatial[]             = "TestSuiteSpatial";
static const char testSuiteContentManager[]      = "TestSuiteContentManager";
static const char testSuiteMetadata[]            = "TestSuiteMetadata";
static const char testSuiteDownload[]            = "TestSuiteDownload";
static const char testSuiteMobileCoupons[]       = "TestSuiteMobileCoupons";
static const char testSuiteHttpDownloadManager[] = "TestSuiteHttpDownloadManager";
static const char testSuiteOffboardTileManager[] = "TestOffboardTileManager";
static const char testSuiteOffboardConvertTileManager[] = "TestOffboardConvertTileManager";
static const char testSuitePersistentData[]      = "testSuitePersistentData";
static const char testSuiteCachingTileManager[]  = "testSuiteCachingTileManager";
static const char testSuiteCache[]               = "testSuiteCache";
static const char testSuiteTuple[]               = "testSuiteTuple";
static const char testSuiteMap[]                 = "testMap";
static const char testSuiteLamProcessor[]        = "testLamProcessor";
static const char testSuitePinLayer[]            = "testSuitePinLayer";
static const char testSuitePinManager[]          = "testSuitePinManager";
static const char testSuitePinCushion[]          = "testSuitePinCushion";
static const char testSuiteSingleSearchProtocol[] = "testSuiteSingleSearchProtocol";

/*! Main function for unit tests.

Add all test suites to the main function.

@return 0
*/
int test_main(void)
{
    CU_ErrorCode result = CUE_SUCCESS;
    CU_pSuite pTestSuite = NULL;

    // Set output verbosity
    CU_basic_set_mode(CU_BRM_VERBOSE); //CU_BRM_NORMAL );

    // Initialize Main Registry
    result = CU_initialize_registry();
    if (result != CUE_SUCCESS)
    {
        return 0;
    }

    // Add all test suites and all tests
    // Add spatial test suite.
    pTestSuite = CU_add_suite(testSuiteSingleSearchProtocol, &TestSingleSearchProtocol_SuiteSetup, &TestSingleSearchProtocol_SuiteCleanup);
    if (pTestSuite)
    {
        // Add all tests to suite
        TestSingleSearchProtocol_AddAllTests(pTestSuite);
    }
#if 0
    // Add spatial test suite.
    pTestSuite = CU_add_suite(testSuiteSpatial, &TestSpatial_SuiteSetup, &TestSpatial_SuiteCleanup);
    if (pTestSuite)
    {
        // Add all tests to suite
        TestSpatial_AddAllTests(pTestSuite);
    }

    // Add content manager test suite.
    pTestSuite = CU_add_suite(testSuiteContentManager, &TestContentManager_SuiteSetup, &TestContentManager_SuiteCleanup);
     if (pTestSuite)
     {
         // Add all tests to suite
         TestContentManager_AddAllTests(pTestSuite);
     }

     // Add metadata test suite.
     pTestSuite = CU_add_suite(testSuiteMetadata, &TestMetadata_SuiteSetup, &TestMetadata_SuiteCleanup);
     if (pTestSuite)
     {
         // Add all tests to suite
         TestMetadata_AddAllTests(pTestSuite);
     }

     // Add download test suite.
     pTestSuite = CU_add_suite(testSuiteDownload, &TestDownload_SuiteSetup, &TestDownload_SuiteCleanup);
     if (pTestSuite)
     {
         // Add all tests to suite
         TestDownload_AddAllTests(pTestSuite);
     }

     // Add mobile coupons test suite.
     pTestSuite = CU_add_suite(testSuiteMobileCoupons, &TestMobileCoupons_SuiteSetup, &TestMobileCoupons_SuiteCleanup);
     if (pTestSuite)
     {
         // Add all tests to suite
         TestMobileCoupons_AddAllTests(pTestSuite);
     }

    // Add HTTP download manager test suite.
    pTestSuite = CU_add_suite(testSuiteHttpDownloadManager, &TestHttpDownloadManager_SuiteSetup, &TestHttpDownloadManager_SuiteCleanup);
    if (pTestSuite)
    {
        // Add all tests to suite
        TestHttpDownloadManager_AddAllTests(pTestSuite);
    }

    // Add offboard tile manager test suite.
    pTestSuite = CU_add_suite(testSuiteOffboardTileManager, &TestOffboardTileManager_SuiteSetup,
                              &TestOffboardTileManager_SuiteCleanup);
    if (pTestSuite)
    {
        TestOffboardTileManager_AddAllTests(pTestSuite);
    }

    // Add offboard convert tile manager test suite.
    pTestSuite = CU_add_suite(testSuiteOffboardConvertTileManager, &TestOffboardConvertTileManager_SuiteSetup,
                              &TestOffboardConvertTileManager_SuiteCleanup);
    if (pTestSuite)
    {
        TestOffboardConvertTileManager_AddAllTests(pTestSuite);
    }

    // Add persistent data test suite.
    pTestSuite = CU_add_suite(testSuitePersistentData, &TestPersistentData_SuiteSetup,
                              &TestPersistentData_SuiteCleanup);
    if (pTestSuite)
    {
        TestPersistentData_AddAllTests(pTestSuite);
    }

    // Add caching tile manager test suite.
    pTestSuite = CU_add_suite(testSuiteCachingTileManager, &TestCachingTileManager_SuiteSetup,
                              &TestCachingTileManager_SuiteCleanup);
    if (pTestSuite)
    {
        TestCachingTileManager_AddAllTests(pTestSuite);
    }

    // Add cache test suite.
    pTestSuite = CU_add_suite(testSuiteCache, &TestCache_SuiteSetup, &TestCache_SuiteCleanup);
    if (pTestSuite)
    {
        TestCache_AddAllTests(pTestSuite);
    }

    // Add tuple test suite.
    pTestSuite = CU_add_suite(testSuiteTuple, &TestTuple_SuiteSetup, &TestTuple_SuiteCleanup);
    if (pTestSuite)
    {
        TestTuple_AddAllTests(pTestSuite);
    }

    pTestSuite = CU_add_suite(testSuiteMap, &TestMap_SuiteSetup, &TestMap_SuiteCleanup);
    if (pTestSuite)
    {
        TestMap_AddAllTests(pTestSuite);
    }

    // Add offboard convert tile manager test suite.
    pTestSuite = CU_add_suite(testSuiteOffboardConvertTileManager, &TestOffboardConvertTileManager_SuiteSetup,
                              &TestOffboardConvertTileManager_SuiteCleanup);
    if (pTestSuite)
    {
        TestOffboardConvertTileManager_AddAllTests(pTestSuite);
    }

    // Add caching tile manager test suite.
    pTestSuite = CU_add_suite(testSuiteCachingTileManager, &TestCachingTileManager_SuiteSetup,
                              &TestCachingTileManager_SuiteCleanup);
    if (pTestSuite)
    {
        TestCachingTileManager_AddAllTests(pTestSuite);
    }

    // Add cache test suite.
    pTestSuite = CU_add_suite(testSuiteCache, &TestCache_SuiteSetup, &TestCache_SuiteCleanup);
    if (pTestSuite)
    {
        TestCache_AddAllTests(pTestSuite);
    }

    // Add tuple test suite.
    pTestSuite = CU_add_suite(testSuiteTuple, &TestTuple_SuiteSetup, &TestTuple_SuiteCleanup);
    if (pTestSuite)
    {
        TestTuple_AddAllTests(pTestSuite);
    }

    pTestSuite = CU_add_suite(testSuiteMap, &TestMap_SuiteSetup, &TestMap_SuiteCleanup);
    if (pTestSuite)
    {
        TestMap_AddAllTests(pTestSuite);
    }

    pTestSuite = CU_add_suite(testSuiteLamProcessor, &TestLamProcessor_SuiteSetup, &TestLamProcessor_SuiteCleanup);
    if (pTestSuite)
    {
        TestLamProcessor_AddAllTests(pTestSuite);
    }

    // Add pin layer test suite.
    pTestSuite = CU_add_suite(testSuitePinLayer, &TestPinLayer_SuiteSetup, &TestPinLayer_SuiteCleanup);
    if (pTestSuite)
    {
        TestPinLayer_AddAllTests(pTestSuite);
    }

    // Add pin manager test suite.
    pTestSuite = CU_add_suite(testSuitePinManager, &TestPinManager_SuiteSetup, &TestPinManager_SuiteCleanup);
    if (pTestSuite)
    {
        TestPinManager_AddAllTests(pTestSuite);
    }

    // Add pin cushion test suite.
    pTestSuite = CU_add_suite(testSuitePinCushion, &TestPinCushion_SuiteSetup,
                              &TestPinCushion_SuiteCleanup);
    if (pTestSuite)
    {
        TestPinCushion_AddAllTests(pTestSuite);
    }
#endif

	// Add additional suites here

    // Run all tests for all suites
    result = CU_basic_run_tests();

    // Run test for one particular suite
    //		result = CU_basic_run_suite(CU_pSuite pSuite);

    // Run one single test
    //		result = CU_basic_run_test(CU_pSuite pSuite, CU_pTest pTest);

    // Show failures
    //		CU_basic_show_failures(CU_pFailureRecord pFailure);

    // Cleanup Main Registry
    CU_cleanup_registry();
    return 0;
}

/*! @} */
