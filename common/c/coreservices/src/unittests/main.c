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
@defgroup SYSTEMTEST_H System Tests for NIM NAVBuilder Services

   This is the main file for all the system test.    

This is the main file for all the system test. All test suites are
loaded here and run from here.
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

#include "Basic.h"
#include "CUnit.h"
#include "main.h"

#include "testcache.h"
#include "testnetwork.h"
#include "testlog.h"
#include "testutil.h"
#include "testtps.h"
#include "testhashtable.h"

// Constants

// All test suites
static const char testSuiteCache[]      = "TestSuiteCache";
static const char testSuiteNetwork[]    = "TestSuiteNetwork";
static const char testSuiteLog[]        = "TestSuiteLog";
static const char testSuiteUtil[]       = "TestSuiteUtil";
static const char testSuiteTPS[]        = "TestSuiteTPS";
static const char testSuiteHashTable[]  = "TestHashTable";


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

	// Add hash table test suite.
    pTestSuite = CU_add_suite(testSuiteHashTable, &TestHashTable_SuiteSetup, &TestHashTable_SuiteCleanup);
    if (pTestSuite)
    {
        // Add all tests to suite
        TestHashTable_AddAllTests(pTestSuite);
    }

// NOTE: Disabled this tps test with hard-coded tpslib for now due to server changes.
//       This functionality is tested extensively in system tests.
#if 0
	// Add network test suite.
    pTestSuite = CU_add_suite(testSuiteNetwork, &TestNetwork_SuiteSetup, &TestNetwork_SuiteCleanup);
    if (pTestSuite)
    {
        // Add all tests to suite
        TestNetwork_AddAllTests(pTestSuite);
    }
#endif

	// Add cache test suite.
    pTestSuite = CU_add_suite(testSuiteCache, &TestCache_SuiteSetup, &TestCache_SuiteCleanup);
    if (pTestSuite)
    {
        // Add all tests to suite
        TestCache_AddAllTests(pTestSuite);
    }

	// Add logging test suite.
    pTestSuite = CU_add_suite(testSuiteLog, &TestLog_SuiteSetup, &TestLog_SuiteCleanup);
    if (pTestSuite)
    {
        // Add all tests to suite
        TestLog_AddAllTests(pTestSuite);
    }

	// Add util test suite.
    pTestSuite = CU_add_suite(testSuiteUtil, &TestUtil_SuiteSetup, &TestUtil_SuiteCleanup);
    if (pTestSuite)
    {
        // Add all tests to suite
        TestUtil_AddAllTests(pTestSuite);
    }

	// Add TPS test suite.
    pTestSuite = CU_add_suite(testSuiteTPS, &TestTPS_SuiteSetup, &TestTPS_SuiteCleanup);
    if (pTestSuite)
    {
        // Add all tests to suite
        TestTPS_AddAllTests(pTestSuite);
    }

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
