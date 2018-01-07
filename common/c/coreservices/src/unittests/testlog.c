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

    @defgroup TestLog_c Unit Tests for CSL Logging

    Unit Tests for CSL Logging

    This file contains all unit tests for the Core logging component
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

#include "testlog.h"
#include "main.h"
#include "platformutil.h"
#ifdef WINCE
#include "winbase.h"
#endif
#include "logimpl.h"
#include "fileutil.h"

#define TEST_LOG_FILENAME1      "test1.log"
#define TEST_LOG_FILENAME2      "test2.log"
#define TEST_LOG_FILENAME3      "test3.log"

static char * pCompName[PALLOG_MAX_IDS] = {"Comp01", "Comp02", "Comp03", "Comp04", "Comp05", "Comp06",
                                           "Comp07", "Comp08", "Comp09", "Comp10", "Comp11", "Comp12",
                                           "Comp13", "Comp14", "Comp15", "Comp16", "Comp17", "Comp18",
                                           "Comp19", "Comp20", "Comp21", "Comp22", "Comp23", "Comp24",
                                           "Comp25", "Comp26", "Comp27", "Comp28", "Comp29", "Comp30",
                                           "Comp31", "Comp32"};
                                           
// Local Functions 
static void TestLogCreation(void);
static void TestLogComponents(void);
static void TestLogWriteF(void);
static void TestLogBlockFeature(void);
static void TestLogRemoveFiles(PAL_Instance* pal);

/*! Add all your test functions here

    @return None
*/
void
TestLog_AddAllTests(CU_pSuite pTestSuite)
{
    // ! Add all your function names here !
    CU_add_test(pTestSuite, "TestLogCreation", &TestLogCreation);
    CU_add_test(pTestSuite, "TestLogComponents", &TestLogComponents);
    CU_add_test(pTestSuite, "TestLogWriteF", &TestLogWriteF);
    CU_add_test(pTestSuite, "TestLogBlockFeature", &TestLogBlockFeature);
};


/*! Add common initialization code here.

    @return 0

    @see TestNetwork_SuiteCleanup
*/
int
TestLog_SuiteSetup()
{
    return 0;
}


/*! Add common cleanup code here.

    @return 0

    @see TestLog_SuiteSetup
*/
int 
TestLog_SuiteCleanup()
{
    return 0;
}


static void TestLogCreation(void)
{
    PAL_Instance *pal = PAL_CreateInstance();
    PAL_Error err = NE_OK;
    PAL_Log* log1 = NULL;
    PAL_Log* log2 = NULL;
    PAL_Log* log3 = NULL;
    TestLogRemoveFiles(pal);

    // Create log file 1
    err = PAL_LogCreate(pal, TEST_LOG_FILENAME1, &log1);
    CU_ASSERT_EQUAL_FATAL(err, PAL_Ok);
    CU_ASSERT(log1 != NULL);
   
    // Create log file 2
    err = PAL_LogCreate(pal, TEST_LOG_FILENAME2, &log2);
    CU_ASSERT_EQUAL_FATAL(err, PAL_Ok);
    CU_ASSERT(log2 != NULL);
   
    // Create log file 3
    err = PAL_LogCreate(pal, TEST_LOG_FILENAME3, &log3);
    CU_ASSERT_EQUAL_FATAL(err, PAL_Ok);
    CU_ASSERT(log3 != NULL);

    // Destroy the log instances
    PAL_LogDestroy(log1);
    PAL_LogDestroy(log2);
    PAL_LogDestroy(log3);

    PAL_Destroy(pal);
}

static void TestLogComponents(void)
{
    PAL_Instance *pal = PAL_CreateInstance();
    PAL_Error err = NE_OK;
    PAL_Log* log = NULL;
    int i = 0;
    PAL_LogComponentID compID[PALLOG_MAX_IDS] = {0};
    PAL_LogComponentID id = 0;
    PAL_LogSeverity severity = PAL_LogSeverityNone;

    // Open the log file previously created
    err = PAL_LogCreate(pal, TEST_LOG_FILENAME1, &log);
    CU_ASSERT_EQUAL_FATAL(err, PAL_Ok);
    CU_ASSERT(log != NULL);
   
    // Set the options mask for the logs
    err = PAL_LogOn(log, PAL_LOG_OPTION_ALL);
    CU_ASSERT_EQUAL(err, PAL_Ok);
   
    // Register all of our components
    for(i = 0; i < PALLOG_MAX_IDS; i++)
    {
        err = PAL_LogRegisterComponent(log, pCompName[i], &compID[i]);
        CU_ASSERT_EQUAL(err, PAL_Ok);
    }

    // Create one extra component than the max to check failure
    err = PAL_LogRegisterComponent(log, "ShouldFail", &id);
    CU_ASSERT_EQUAL(err, PAL_Failed);

    // Turn on all of the components
    for(i = 0; i < PALLOG_MAX_IDS; i++)
    {
        err = PAL_LogComponentOn(log, compID[i], PAL_LogSeverityDebug);
        CU_ASSERT_EQUAL(err, PAL_Ok);
    }

    // Check that we can write entries for all of the components
    for(i = 0; i < PALLOG_MAX_IDS; i++)
    {
        err = PAL_LogWrite(log, compID[i], PAL_LogSeverityDebug, __FILE__, __FUNCTION__, __LINE__, "Testing component write");
        CU_ASSERT_EQUAL(err, PAL_Ok);
    }

    // Change the severity level and ensure that the logging honors severity settings
    for(i = 0; i < PALLOG_MAX_IDS; i++)
    {
        err = PAL_LogComponentOn(log, compID[i], PAL_LogSeverityMinorInfo);
        CU_ASSERT_EQUAL(err, PAL_Ok);

        err = PAL_LogWrite(log, compID[i], PAL_LogSeverityDebug, __FILE__, __FUNCTION__, __LINE__, "Testing component write - SHOULD NOT SHOW IN LOG");
        CU_ASSERT_EQUAL(err, PAL_ErrLogSeverity);
    }

    // Turn off the logging and ensure that the log library honors the setting
    for(i = 0; i < PALLOG_MAX_IDS; i++)
    {
        err = PAL_LogComponentOff(log, compID[i]);
        CU_ASSERT_EQUAL(err, PAL_Ok);

        err = PAL_LogWrite(log, compID[i], PAL_LogSeverityDebug, __FILE__, __FUNCTION__, __LINE__, "Testing component write - SHOULD NOT SHOW IN LOG");
        CU_ASSERT_EQUAL(err, PAL_ErrLogSeverity);
    }

    // Turn back on the severity for all components
    for(i = 0; i < PALLOG_MAX_IDS; i++)
    {
        err = PAL_LogComponentOn(log, compID[i], PAL_LogSeverityInfo);
        CU_ASSERT_EQUAL(err, PAL_Ok);
    }

    // Turn off master logging switch
    PAL_LogOff(log);

    // Ensure that the master log switch is honored
    for(i = 0; i < PALLOG_MAX_IDS; i++)
    {
        err = PAL_LogWrite(log, compID[i], PAL_LogSeverityCritical, __FILE__, __FUNCTION__, __LINE__, "Testing component write - SHOULD NOT SHOW IN LOG");
        CU_ASSERT_EQUAL(err, PAL_ErrLogSeverity);
    }

    // Ensure that the severity level returns correctly
    for(i = 0; i < PALLOG_MAX_IDS; i++)
    {
        err = PAL_LogComponentGetStatus(log, compID[i], &severity);
        CU_ASSERT_EQUAL(severity, PAL_LogSeverityInfo);
    }

    // Try to write to an invalid componentID
    err = PAL_LogWrite(log, PALLOG_MAX_IDS+256, PAL_LogSeverityCritical, __FILE__, __FUNCTION__, __LINE__, "Testing component for invalid ID - SHOULD NOT SHOW IN LOG");
    CU_ASSERT_EQUAL(err, PAL_ErrBadParam);

    // Try to set the severity for an invalid component
    err = PAL_LogComponentOn(log, PALLOG_MAX_IDS+256, PAL_LogSeverityCritical);
    CU_ASSERT_EQUAL(err, PAL_ErrBadParam);

    // Destroy the log instances
    PAL_LogDestroy(log);

    PAL_Destroy(pal);
}

static void TestLogWriteF(void)
{
    PAL_Instance *pal = PAL_CreateInstance();
    PAL_Error err = NE_OK;
    PAL_Log* log = NULL;
    int i = 0;
    PAL_LogComponentID compID[PALLOG_MAX_IDS] = {0};

    // Open the second log file
    err = PAL_LogCreate(pal, TEST_LOG_FILENAME2, &log);
    CU_ASSERT_EQUAL_FATAL(err, PAL_Ok);
    CU_ASSERT(log != NULL);
   
    // Set the options mask for the logs
    err = PAL_LogOn(log, PAL_LOG_OPTION_ALL);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    // Register all of our components and turn them to the lowest severity
    for(i = 0; i < PALLOG_MAX_IDS; i++)
    {
        err = PAL_LogRegisterComponent(log, pCompName[i], &compID[i]);
        CU_ASSERT_EQUAL(err, PAL_Ok);

        err = PAL_LogComponentOn(log, compID[i], PAL_LogSeverityDebug);
        CU_ASSERT_EQUAL(err, PAL_Ok);
    }

    // Check that we can write entries for all of the components
    for(i = 0; i < PALLOG_MAX_IDS; i++)
    {
        err = PAL_LogWriteF(log, compID[i], PAL_LogSeverityDebug, __FILE__, __FUNCTION__, __LINE__, 
            "Testing component writeF - id: %d. i: %d. name: %s", compID[i], i, pCompName[i]);
        CU_ASSERT_EQUAL(err, PAL_Ok);
    }

    // Change the severity level and ensure that the logging honors severity settings
    for(i = 0; i < PALLOG_MAX_IDS; i++)
    {
        err = PAL_LogComponentOn(log, compID[i], PAL_LogSeverityMinorInfo);
        CU_ASSERT_EQUAL(err, PAL_Ok);

        err = PAL_LogWrite(log, compID[i], PAL_LogSeverityDebug, __FILE__, __FUNCTION__, __LINE__, "Testing component write - SHOULD NOT SHOW IN LOG");
        CU_ASSERT_EQUAL(err, PAL_ErrLogSeverity);
    }

    // Destroy the log instances
    PAL_LogDestroy(log);

    PAL_Destroy(pal);
}

static void TestLogBlockFeature(void)
{
    PAL_Instance *pal = PAL_CreateInstance();
    PAL_Error err = NE_OK;
    PAL_Log* log = NULL;
    int j = 0, i = 0;
    PAL_LogComponentID compId = 0;
    char buffer[256];

    // Open the third log file
    err = PAL_LogCreate(pal, TEST_LOG_FILENAME3, &log);
    CU_ASSERT_EQUAL_FATAL(err, PAL_Ok);
    CU_ASSERT(log != NULL);
   
    // Set the options mask for the logs
    err = PAL_LogOn(log, PAL_LOG_OPTION_ALL);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    // Register a single and turn them to the lowest severity
    err = PAL_LogRegisterComponent(log, pCompName[0], &compId);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = PAL_LogComponentOn(log, compId, PAL_LogSeverityDebug);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    // Check that block begin and end work up to 20 levels deep
    for(j = 0; j < 20; j++)
    {
        nsl_sprintf(buffer, "Start Level %d", j+1);
        err = PAL_LogWriteBlockBegin(log, compId, __FILE__, __FUNCTION__, __LINE__, buffer);
        CU_ASSERT_EQUAL(err, PAL_Ok);

        // Add entries for all severities at each level
        for(i = (int)PAL_LogSeverityCritical; i <= (int)PAL_LogSeverityDebug; i++)
        {
            err = PAL_LogWriteF(log, compId, (PAL_LogSeverity)i, __FILE__, __FUNCTION__, __LINE__, 
                "Entry for level %d. severity: %d", j+1, i);
            CU_ASSERT_EQUAL(err, PAL_Ok);
        }
    }

    for(j = 19; j >= 0; j--)
    {
        nsl_sprintf(buffer, "End Level %d", j+1);
        err = PAL_LogWriteBlockEnd(log, compId, __FILE__, __FUNCTION__, __LINE__, buffer);
        CU_ASSERT_EQUAL(err, PAL_Ok);
    }

    // Destroy the log instances
    PAL_LogDestroy(log);

    PAL_Destroy(pal);
}

static void TestLogRemoveFiles(PAL_Instance* pal)
{
    // Ensure that our test files are deleted
    if(fileexists(pal, TEST_LOG_FILENAME1))
    {
        CU_ASSERT(removefile(pal, TEST_LOG_FILENAME1) == TRUE);
    }

    if(fileexists(pal, TEST_LOG_FILENAME2))
    {
        CU_ASSERT(removefile(pal, TEST_LOG_FILENAME2) == TRUE);
    }

    if(fileexists(pal, TEST_LOG_FILENAME3))
    {
        CU_ASSERT(removefile(pal, TEST_LOG_FILENAME3) == TRUE);
    }

}

/*! @} */


