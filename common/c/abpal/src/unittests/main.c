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
#include "testaudio.h"
#include "testgps.h"
#include "main.h"


// Constants

#define TESTING_AUDIO
//#define TESTING_GPS


// All test suites
static const char testSuiteAudio[]      = "TestSuiteAudio";
static const char testSuiteGps[]        = "TestSuiteGps";


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

#ifdef TESTING_AUDIO
    // Add audio test suite
    pTestSuite = CU_add_suite(testSuiteAudio, &TestAudio_SuiteSetup, &TestAudio_SuiteCleanup);
    if (pTestSuite)
    {
        // Add all tests to suite
        TestAudio_AddAllTests(pTestSuite);
    }
#endif

//#ifdef WINCE
#ifdef TESTING_GPS
    // Add GPS test suite
    pTestSuite = CU_add_suite(testSuiteGps, &TestGps_SuiteSetup, &TestGps_SuiteCleanup);
    if (pTestSuite)
    {
        // Add all tests to suite
        TestGps_AddAllTests(pTestSuite);
    }
#endif
//#endif

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
