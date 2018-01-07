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

    All test suites are loaded here and run from here.
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

#include "CUnit.h"
#include "Automated.h"
#include "Basic.h"
#include "TestRun.h"

#include "testspeedcameras.h"
#include "testgpstypes.h"
#include "testgeocode.h"
#include "testnetwork.h"
#include "testrastermap.h"
#include "testrastertile.h"
#include "testtileservice.h"
#include "testvectormap.h"
#include "testnavigation.h"
#include "testdirection.h"
#include "testspatial.h"
#include "testspeech.h"
#include "testfileset.h"
#include "testmotd.h"
#include "testshare.h"
#include "testsync.h"
#include "testprofile.h"
#include "testdatastore.h"
#include "testsms.h"
#include "testsearchbuilder.h"
#include "testreversegeocode.h"
#include "testqalog.h"
#include "testsubscription.h"
#include "testtristrip.h"
//#include "testanalytics.h"
#include "testers.h"
#include "testlicense.h"
#include "TestLocation.h"
#include "testlocationservices.h"
#include "testenhancedcontent.h"
#include "testsinglesearch.h"
#include "testpublictransit.h"
#include "namebitmaskmap.h"
#include "testpoiimagemanager.h"
#include "testmapview.h"
#include "TestLayerManager.h"
#include "testsinglesearchservices.h"
#include "testbatchtool.h"
#include "testnavapi.h"
#include "palclock.h"
#include "palfile.h"
#include "platformutil.h"
#include "argtableutil.h"
#include "palversion.h"
#include "abpalversion.h"
#include "cslversion.h"
#include "nbversion.h"
#include "abversion.h"
#include "main.h"
#include "paltimer.h"
#include "pal.h"

#ifdef _WIN32
    #include <windows.h>
    #include <winbase.h>
#endif

#include "protected/cslutil.h"

// Constants

// All test suites
//static const char TEST_SUITE_ANALYTICS[]            = "TestAnalytics";
static const char TEST_SUITE_BACTCHTOOL[]           = "TestBatchTool";
static const char TEST_SUITE_DATASTORE[]            = "TestSuiteDataStore";
static const char TEST_SUITE_DIRECTIONS[]           = "TestSuiteDirections";
static const char TEST_SUITE_ENHANCEDCONTENT[]      = "TestEnhancedContent";
static const char TEST_SUITE_ERS[]                  = "TestErs";
static const char TEST_SUITE_FILESET[]              = "TestSuiteFileset";
static const char TEST_SUITE_GEOCODE[]              = "TestSuiteGeocode";
static const char TEST_SUITE_GPSTYPES[]             = "TestSuiteGPSTypes";
static const char TEST_SUITE_LAYERMANAGER[]         = "TestLayerManager";
static const char TEST_SUITE_LICENSE[]              = "TestLicense";
static const char TEST_SUITE_LOCATIONSERVICES[]     = "TestLocationServices";
static const char TEST_SUITE_LOCATION[]             = "TestLocation";
static const char TEST_SUITE_MAPVIEW[]              = "TestMapView";
static const char TEST_SUITE_MOTD[]                 = "TestSuiteMotd";
static const char TEST_SUITE_NAV[]                  = "TestSuiteNavigation";
static const char TEST_SUITE_NETWORK[]              = "TestSuiteNetwork";
static const char TEST_SUITE_PLACEMESSAGE[]         = "TestSuitePlaceMessage";
static const char TEST_SUITE_POIIMAGEMANAGER[]      = "TestPoiImageManager";
static const char TEST_SUITE_PROFILE[]              = "TestSuiteProfile";
static const char TEST_SUITE_PUBLICTRANSIT[]        = "TestPublicTransit";
static const char TEST_SUITE_QALOG[]                = "TestQaLog";
static const char TEST_SUITE_RASTERMAP[]            = "TestSuiteRasterMap";
static const char TEST_SUITE_RASTERTILE[]           = "TestSuiteRasterTile";
static const char TEST_SUITE_REVERSEGEOCODE[]       = "TestReverseGeocode";
static const char TEST_SUITE_SEARCHBUILDER[]        = "TestSuiteSearchBuilder";
static const char TEST_SUITE_SINGLESEARCHSERVICES[] = "TestSingleSearchServices";
static const char TEST_SUITE_SINGLESEARCH[]         = "TestSingleSearch";
static const char TEST_SUITE_SMS[]                  = "TestSms";
static const char TEST_SUITE_SPATIAL[]              = "TestSuiteSpatial";
static const char TEST_SUITE_SPEECH[]               = "TestSuiteSpeech";
static const char TEST_SUITE_SPEEDCAMERAS[]         = "TestSpeedCameras";
static const char TEST_SUITE_SUBSCRIPTION[]         = "TestSubscription";
static const char TEST_SUITE_SYNC[]                 = "TestSuiteSync";
static const char TEST_SUITE_TILESERVICE[]          = "TestTileService";
static const char TEST_SUITE_TRISTRIP[]             = "TestTriStrip";
static const char TEST_SUITE_VECTORMAP[]            = "TestSuiteVectorMap";
static const char TEST_SUITE_NAVAPI[]               = "TestSuiteNavApi";

#ifdef _WIN32
static void SaveVersionInfo(char* filename)
{
    nb_version version = {0};
    FILE* pFile = NULL;
    char buf[1000] = {0};
    pFile = fopen(filename, "w");

    if (pFile == NULL)
    {
        return;
    }

    PAL_VersionGet(&version);
    sprintf(buf, "NBPAL Version: %d.%d.%d.%d\n", version.major, version.minor, version.feature, version.build);
    fwrite(buf, sizeof(char), nsl_strlen(buf), pFile);
    ABPAL_VersionGet(&version);
    sprintf(buf, "ABPAL Version: %d.%d.%d.%d\n", version.major, version.minor, version.feature, version.build);
    fwrite(buf, sizeof(char), nsl_strlen(buf), pFile);
    CSL_VersionGet(&version);
    sprintf(buf, "Core Services Version: %d.%d.%d.%d\n", version.major, version.minor, version.feature, version.build);
    fwrite(buf, sizeof(char), nsl_strlen(buf), pFile);
    NB_VersionGet(&version);
    sprintf(buf, "NB Services Version: %d.%d.%d.%d\n", version.major, version.minor, version.feature, version.build);
    fwrite(buf, sizeof(char), nsl_strlen(buf), pFile);
    AB_VersionGet(&version);
    sprintf(buf, "AB Services Version: %d.%d.%d.%d\n", version.major, version.minor, version.feature, version.build);
    fwrite(buf, sizeof(char), nsl_strlen(buf), pFile);
    fwrite("Token: ", sizeof(char), 7, pFile);
    fwrite(TestNetwork_GetToken(), sizeof(char), nsl_strlen(TestNetwork_GetToken()), pFile);
    fclose(pFile);
}

static void SaveListOfTests()
{
    CU_pTestRegistry reg = CU_get_registry();
    CU_pSuite suite = NULL;
    char* file = "tests_list.txt";
    char fullFile[MAX_PATH + 1] = {0};
    FILE* pFile = NULL;
    char dllFname[MAX_PATH + 1] = {0};
    char dir[MAX_PATH + 1] = {0};
    uint32 i = 0;
    uint32 j = 0;

    GetModuleFileNameA(NULL, dllFname, MAX_PATH);
    nsl_strncpy(dir, dllFname, nsl_strlen(dllFname)-nsl_strlen(nsl_strrchr(dllFname, '\\')));
    SetCurrentDirectoryA(dir);
    SaveVersionInfo("CCC_info.txt");

    if (reg == NULL)
    {
        return;
    }

    strncpy(fullFile, file, strlen(file));

    suite = reg->pSuite;
    pFile = fopen(fullFile, "w");

    if (pFile == NULL)
    {
        return;
    }

    for(i = 0; i < reg->uiNumberOfSuites; i++)
    {
        if (suite)
        {
            CU_pTest test = suite->pTest;

            fwrite("_", sizeof(char), 1, pFile);
            fwrite(suite->pName, sizeof(char), nsl_strlen(suite->pName) + 1, pFile);
            fwrite("\n", sizeof(char), 1, pFile);

            for(j = 0; j < suite->uiNumberOfTests; j++)
            {
                if (test)
                {
                    fwrite(test->pName, sizeof(char), nsl_strlen(test->pName) + 1, pFile);
                    fwrite("\n", sizeof(char), 1, pFile);
                    test = test->pNext;
                }
            }

            suite = suite->pNext;
        }
    }

    fclose(pFile);
}

#else //_WIN32
static void SaveListOfTests()
{
}
#endif //_WIN32

typedef void (*SuiteTestAdder)(CU_pSuite, int);
CU_pSuite CreateTestSuite(const char* name, CU_InitializeFunc setup, CU_CleanupFunc cleanup, SuiteTestAdder adder);

static void DisplayVersionInfo()
{
    nb_version version = { 0 };

    PAL_VersionGet(&version);
    PRINTF("NBPAL Version: %d.%d.%d.%d\n", version.major, version.minor, version.feature, version.build);

    ABPAL_VersionGet(&version);
    PRINTF("ABPAL Version: %d.%d.%d.%d\n", version.major, version.minor, version.feature, version.build);

    CSL_VersionGet(&version);
    PRINTF("Core Services Version: %d.%d.%d.%d\n", version.major, version.minor, version.feature, version.build);

    NB_VersionGet(&version);
    PRINTF("NB Services Version: %d.%d.%d.%d\n", version.major, version.minor, version.feature, version.build);

    AB_VersionGet(&version);
    PRINTF("AB Services Version: %d.%d.%d.%d\n", version.major, version.minor, version.feature, version.build);
}


static void DisplayMDN()
{
    PAL_Instance* pal = PAL_CreateInstance();
    if (pal)
    {
        char mdnString[20] = { 0 };
        uint64 mdn = GetMobileDirectoryNumber(pal);
        nsl_uint64tostr(mdn, mdnString, sizeof(mdnString));

        PRINTF("\nMDN: %s\n", mdnString);

        if (mdn == DEFAULT_MOBILE_DIRECTORY_NUMBER)
        {
            PRINTF("*****************************************************************************\n");
            PRINTF("*****                                                                   *****\n");
            PRINTF("*****                         Default MDN in use                        *****\n");
            PRINTF("*****  Create an mdn.txt file with your desk phone number (949453xxxx)  *****\n");
            PRINTF("*****                                                                   *****\n");
            PRINTF("*****************************************************************************\n");
        }

        PAL_DestroyInstance(pal);
    }
    else
    {
        PRINTF("MDN: Unknown, failed to create PAL instance");
    }
}


static void DisplayOptions()
{
    char* testLevelString[]     = {"Smoke", "Intermediate", "Full"};
    char* logLevelString[]      = {"None", "Low", "Medium", "High"};
    char* networkTypeString[]   = {"TCP", "TCP TLS", "HTTP", "HTTPS"};
    char* overWriteString[]     = {"Inactive", "Active"};
    Test_Options* testOptions   = Test_OptionsGet();
    char hostname[MAX_HOSTNAME_LENGTH + 1] = { 0 };
    byte sha[20] = { 0 };

    get_sha_hash(TestNetwork_GetToken(), nsl_strlen(TestNetwork_GetToken()), sha);
    hexlify(20, (const char*)sha, hostname);


    PRINTF("\nTesting Level     : %s (%d)\n", testLevelString[testOptions->testLevel], testOptions->testLevel);
    PRINTF("Test Output Level : %s (%d)\n", logLevelString[testOptions->logLevel], testOptions->logLevel);
    PRINTF("QA Logging Level  : %s\n", testOptions->verboseQaLog ? "Verbose" : "Normal");
    PRINTF("Network type      : %s\n", networkTypeString[testOptions->networkType]);
    PRINTF("Token             : %s\n", TestNetwork_GetToken());
    PRINTF("Hostname          : %s.%s\n", nsl_strlen(testOptions->hostname) > 0 ? testOptions->hostname : hostname, TestNetwork_GetDomain());
    PRINTF("OverWrite Flag    : %s (%d)\n", overWriteString[testOptions->overwriteFlag], testOptions->overwriteFlag);
	PRINTF("Output log file   : %s\n", nsl_strlen(testOptions->logfilename) > 0 ? testOptions->logfilename : "");

    DisplayMDN();
}

void ListSuites()
{
    int i = 0;

    PRINTF("%s", suiteMap[0].name);
    for (i = 1; i < suiteCount; i++)
    {
        PRINTF(", %s", suiteMap[i].name);
    }
    PRINTF("\n");
}

static void
debug_timer(PAL_Instance* pal, void* arg, PAL_TimerCBReason reason)
{
    if (reason == PTCBR_TimerFired)
    {
        fflush(stdout);
        PAL_TimerSet(pal, 2 * 1000, debug_timer, 0);
    }
}


/*! Main function for system tests.

Add all test suites to the main function.

@return 0
*/

static int g_countSuites = 0;
static int g_start = 0;
static int g_step = 0;

int test_main(int argc, char* argv[])
{
    CU_ErrorCode    result      = CUE_SUCCESS;
    nb_unixTime     startTime   = PAL_ClockGetUnixTime();
    nb_unixTime     endTime     = 0;
    unsigned int    testsFailed = 0;
    nb_boolean      err         = 0;
    Test_Options*   testOptions = NULL;
    char logfilename[MAX_LOGFILENAME_LENGTH + 1] = { 0 };
    PAL_Instance* pal = NULL;


    // process the arguments before initializing heap check as option can be used after parsing
    err = ProcessArguments(argc, argv);
    if (err == FALSE)
    {
        return 0;
    }

    if (Test_OptionsGet()->listSuitesAndExit)
    {
        ListSuites();
        return 0;
    }

    nsl_strcpy(logfilename, Test_OptionsGet()->logfilename);
    if (nsl_strlen(logfilename) > 0)
    {
        PAL_Instance *pal = PAL_CreateInstance();
        if(PAL_FileExists(pal, logfilename))
        {
        }
        PAL_Destroy(pal);
        pal = NULL;
    }
    else if (!(Test_OptionsGet()->listTestsAndExit))
    {
        DisplayVersionInfo();
        DisplayOptions();
    }

    // Set current language and voice style
    {
        pal = PAL_CreateInstance();
        if (pal)
        {
            {
                unsigned char* data = NULL;
                uint32 dataSize = 0;
                nsl_memset(Test_OptionsGet()->currentLanguage, 0, MAX_LANGUAGE_LENGTH + 1);
                if (PAL_Ok == PlatformLoadFile(pal, "current_language.txt", &data, &dataSize) && data && dataSize > 0 && dataSize < MAX_LANGUAGE_LENGTH)
                {
                    nsl_strlcpy(Test_OptionsGet()->currentLanguage, (char*)data, dataSize + 1);
                    nsl_free(data);
                }
            }
            {
                unsigned char* data = NULL;
                uint32 dataSize = 0;
                nsl_memset(Test_OptionsGet()->voiceStyle, 0, MAX_VOICE_STYLE_LENGTH + 1);
                if (PAL_Ok == PlatformLoadFile(pal, "current_voice.txt", &data, &dataSize) && data && dataSize > 0 && dataSize < MAX_VOICE_STYLE_LENGTH)
                {
                    nsl_strlcpy(Test_OptionsGet()->voiceStyle, (char*)data, dataSize + 1);
                    nsl_free(data);
                }
            }
        }
        PAL_Destroy(pal);
        pal = NULL;
    }

    pal = PAL_CreateInstance();
    if (pal)
    {
        PAL_TimerSet(pal, 2 * 1000, debug_timer, 0);
    }

    InitializeHeapCheck();

    // Set output verbosity
    if(nsl_strlen(logfilename) > 0)
    {
        CU_basic_set_mode(CU_BRM_SILENT);;
    }
    else
    {
        CU_basic_set_mode(CU_BRM_VERBOSE);
    }

    // Initialize Main Registry
    result = CU_initialize_registry();
    if ( result != CUE_SUCCESS )
    {
        return 0;
    }

    // Add all test suites and all tests
    testOptions   = Test_OptionsGet();

    if(testOptions->numStart != 0 && testOptions->numStep != 0)
    {
        g_start = testOptions->numStart;
        g_step = testOptions->numStep;
    }
    else
    {
        // Here we need to delete old content of ".\\Test Result\\Result.txt"

        PAL_File* palFileResult = NULL;
        PAL_FileOpen(pal, RESULT_FILE_PATH, PFM_Create, &palFileResult);
        PAL_FileClose(palFileResult);

        g_start = 1;             //If there are no arguments
        g_step = suiteCount;     //run all suites in one process
    }
    PAL_Destroy(pal);
    pal = NULL;

    if(testOptions->suites & SUITE_GPSTYPES)
    {
        // Add GPS Types test suite.
        CreateTestSuite(TEST_SUITE_GPSTYPES, &TestGPSTypes_SuiteSetup, &TestGPSTypes_SuiteCleanup, TestGPSTypes_AddTests);
    }

    if(testOptions->suites & SUITE_NETWORK)
    {
        // Add Network test suite.
        CreateTestSuite(TEST_SUITE_NETWORK, &TestNetwork_SuiteSetup, &TestNetwork_SuiteCleanup, TestNetwork_AddTests);
    }

    // I'm disabling raster-maps. This is legacy code and was last used in VZ Navigator V4. V5 and later uses raster-tile maps.

    if(testOptions->suites & SUITE_RASTERMAP)
    {
        // Add raster map test suite.
        CreateTestSuite(TEST_SUITE_RASTERMAP, &TestRasterMap_SuiteSetup, &TestRasterMap_SuiteCleanup, TestRasterMap_AddTests);
    }

    if(testOptions->suites & SUITE_RASTERTILE)
    {
        // Add raster map test suite.
        CreateTestSuite(TEST_SUITE_RASTERTILE, &TestRasterTile_SuiteSetup, &TestRasterTile_SuiteCleanup, TestRasterTile_AddTests);
    }

    if(testOptions->suites & SUITE_VECTORMAP)
    {
        // Add Vector Map test suite.
        CreateTestSuite(TEST_SUITE_VECTORMAP, &TestVectorMap_SuiteSetup, &TestVectorMap_SuiteCleanup, TestVectorMap_AddTests);
    }

    if(testOptions->suites & SUITE_GEOCODE)
    {
        // Add Geocode test suite.
        CreateTestSuite(TEST_SUITE_GEOCODE, &TestGeocode_SuiteSetup, &TestGeocode_SuiteCleanup, TestGeocode_AddTests);
    }

    if(testOptions->suites & SUITE_REVERSEGEOCODE)
    {
        CreateTestSuite(TEST_SUITE_REVERSEGEOCODE, &TestReverseGeocode_SuiteSetup, &TestReverseGeocode_SuiteCleanup, TestReverseGeocode_AddTests);
    }

    if(testOptions->suites & SUITE_SEARCHBUILDER)
    {
        // Add SearchBuilder test suite.
        CreateTestSuite(TEST_SUITE_SEARCHBUILDER, &TestSearchBuilder_SuiteSetup, &TestSearchBuilder_SuiteCleanup, TestSearchBuilder_AddTests);
    }

    if(testOptions->suites & SUITE_NAVIGATION)
    {
        // Add Navigation test suite.
        CreateTestSuite(TEST_SUITE_NAV, &TestNavigation_SuiteSetup, &TestNavigation_SuiteCleanup, TestNavigation_AddTests);
    }

    if(testOptions->suites & SUITE_DIRECTIONS)
    {
        // Add Direction test suite.
        CreateTestSuite(TEST_SUITE_DIRECTIONS, &TestDirections_SuiteSetup, &TestDirections_SuiteCleanup, TestDirections_AddTests);
    }

    if(testOptions->suites & SUITE_SPATIAL)
    {
        // Add spatial test suite.
        CreateTestSuite(TEST_SUITE_SPATIAL, &TestSpatial_SuiteSetup, &TestSpatial_SuiteCleanup, TestSpatial_AddTests);
    }

    if(testOptions->suites & SUITE_SPEECH)
    {
        // Add speech test suite.
        CreateTestSuite(TEST_SUITE_SPEECH, &TestSpeech_SuiteSetup, &TestSpeech_SuiteCleanup, TestSpeech_AddTests);
    }

    if(testOptions->suites & SUITE_FILESET)
    {
        // Add FileSet test suite.
        CreateTestSuite(TEST_SUITE_FILESET, &TestFileset_SuiteSetup, &TestFileset_SuiteCleanup, TestFileset_AddTests);
    }

    if(testOptions->suites & SUITE_MOTD)
    {
        // Add MOTD test suite.
        CreateTestSuite(TEST_SUITE_MOTD, &TestMotd_SuiteSetup, &TestMotd_SuiteCleanup, TestMotd_AddTests);
    }

    if(testOptions->suites & SUITE_PLACEMESSAGE)
    {
        // Add Place Message test suite.
        CreateTestSuite(TEST_SUITE_PLACEMESSAGE, &TestPlaceMessage_SuiteSetup, &TestPlaceMessage_SuiteCleanup, TestPlaceMessage_AddTests);
    }

    if(testOptions->suites & SUITE_SYNC)
    {
        // Add Sync test suite.
        CreateTestSuite(TEST_SUITE_SYNC, &TestSync_SuiteSetup, &TestSync_SuiteCleanup, TestSync_AddTests);
    }

    if(testOptions->suites & SUITE_PROFILE)
    {
        // Add Profile test suite.
        CreateTestSuite(TEST_SUITE_PROFILE, &TestProfile_SuiteSetup, &TestProfile_SuiteCleanup, TestProfile_AddTests);
    }

    if(testOptions->suites & SUITE_DATASTORE)
    {
        // Add DataStore test suite.
        CreateTestSuite(TEST_SUITE_DATASTORE, &TestDataStore_SuiteSetup, &TestDataStore_SuiteCleanup, TestDataStore_AddTests);
    }

    if(testOptions->suites & SUITE_SMS)
    {
        // Add SMS test suite.
        CreateTestSuite(TEST_SUITE_SMS, &TestSms_SuiteSetup, &TestSms_SuiteCleanup, TestSms_AddTests);
    }

    if(testOptions->suites & SUITE_QALOG)
    {
        CreateTestSuite(TEST_SUITE_QALOG, &TestQaLog_SuiteSetup, &TestQaLog_SuiteCleanup, TestQaLog_AddTests);
    }

    if(testOptions->suites & SUITE_SUBSCRIPTION)
    {
        CreateTestSuite(TEST_SUITE_SUBSCRIPTION, &TestSubscription_SuiteSetup, &TestSubscription_SuiteCleanup, TestSubscription_AddTests);
    }

    if (testOptions->suites & SUITE_TRISTRIP)
    {
        CreateTestSuite(TEST_SUITE_TRISTRIP, &TestTriStrip_SuiteSetup, &TestTriStrip_SuiteCleanup, TestTriStrip_AddTests);
    }

    //if (testOptions->suites & SUITE_ANALYTICS)
    //{
    //    CreateTestSuite(TEST_SUITE_ANALYTICS, &TestAnalytics_SuiteSetup, &TestAnalytics_SuiteCleanup, TestAnalytics_AddTests);
    //}

    if (testOptions->suites & SUITE_ERS)
    {
        CreateTestSuite(TEST_SUITE_ERS, &TestErs_SuiteSetup, &TestErs_SuiteCleanup, TestErs_AddTests);
    }

    if (testOptions->suites & SUITE_LICENSE)
    {
        CreateTestSuite(TEST_SUITE_LICENSE, &TestLicense_SuiteSetup, &TestLicense_SuiteCleanup, TestLicense_AddTests);
    }

    // @todo: Support enhanced content and location services suites for iPhone
    if (testOptions->suites & SUITE_ENHANCEDCONTENT)
    {
        CreateTestSuite(TEST_SUITE_ENHANCEDCONTENT, &TestEnhancedContent_SuiteSetup, &TestEnhancedContent_SuiteCleanup, TestEnhancedContent_AddTests);
    }

    if (testOptions->suites & SUITE_LOCATIONSERVICES)
    {
        CreateTestSuite(TEST_SUITE_LOCATIONSERVICES, &TestLocationServices_SuiteSetup, &TestLocationServices_SuiteCleanup, TestLocationServices_AddTests);
    }

    if(testOptions->suites & SUITE_TILESERVICE)
    {
        CreateTestSuite(TEST_SUITE_TILESERVICE, &TestTileService_SuiteSetup, &TestTileService_SuiteCleanup, TestTileService_AddTests);
    }

    if(testOptions->suites & SUITE_SPEEDCAMERAS)
    {
        // Add SpeedCameras test suite.
        CreateTestSuite(TEST_SUITE_SPEEDCAMERAS, &TestSpeedCameras_SuiteSetup, &TestSpeedCameras_SuiteCleanup, TestSpeedCameras_AddTests);
    }

    if(testOptions->suites & SUITE_PUBLICTRANSIT)
    {
        CreateTestSuite(TEST_SUITE_PUBLICTRANSIT, &TestPublicTransit_SuiteSetup, &TestPublicTransit_SuiteCleanup, TestPublicTransit_AddTests);
    }

    if(testOptions->suites & SUITE_SINGLESEARCH)
    {
        CreateTestSuite(TEST_SUITE_SINGLESEARCH, &TestSingleSearch_SuiteSetup, &TestSingleSearch_SuiteCleanup, TestSingleSearch_AddTests);
    }

    if(testOptions->suites & SUITE_SINGLESEARCHSERVICES)
    {
        CreateTestSuite(TEST_SUITE_SINGLESEARCHSERVICES, &TestSingleSearchServices_SuiteSetup, &TestSingleSearchServices_SuiteCleanup, TestSingleSearchServices_AddTests);
    }

    // @todo: LOCATION (???) is only testable under Mac, at present. Remove this when other
    //        platform is ready.
#ifdef __APPLE__
    // Following test cases are using new thread model.
    if (testOptions->suites & SUITE_LOCATION)
    {
        CreateTestSuite(TEST_SUITE_LOCATION, &TestLocation_SuiteSetup,
                        &TestLocation_SuiteCleanup, TestLocation_AddTests);
    }
#endif

    if(testOptions->suites & SUITE_LAYERMANAGER)
    {
        CreateTestSuite(TEST_SUITE_LAYERMANAGER,
                        &TestLayerManager_SuiteSetup,
                        &TestLayerManager_SuiteCleanup,
                        TestLayerManager_AddAllTests);
    }

    // After MapView is created, NBGM will try to read some resource files (fonts and so on)
    // that do not exist in system test. This will cause crash.
    if(testOptions->suites & SUITE_BATCHTOOL)
    {
        CreateTestSuite(TEST_SUITE_BACTCHTOOL, &TestBatchTool_SuiteSetup, &TestBatchTool_SuiteCleanup, TestBatchTool_AddTests);
    }

    if(testOptions->suites & SUITE_POIIMAGEMANAGER)
    {
        CreateTestSuite(TEST_SUITE_POIIMAGEMANAGER,
                        &TestPoiImageManager_SuiteSetup,
                        &TestPoiImageManager_SuiteCleanup,
                        TestPoiImageManager_AddAllTests);
    }

    if(testOptions->suites & SUITE_MAPVIEW)
    {
        CreateTestSuite(TEST_SUITE_MAPVIEW,
                        &TestMapView_SuiteSetup,
                        &TestMapView_SuiteCleanup,
                        TestMapView_AddAllTests);
    }

    /*if(testOptions->suites & SUITE_NAVAPI)
    {
        // Add Navigation test suite.
        CreateTestSuite(TEST_SUITE_NAVAPI, &TestNavApi_SuiteSetup, &TestNavApi_SuiteCleanup, TestNavApi_AddTests);
    }*/

    // Add additional suites here
    if (Test_OptionsGet()->listTestsAndExit)
    {
        SaveListOfTests();

        // Cleanup Main Registry
        CU_cleanup_registry();
        Test_OptionsDestroy();

        return 0;
    }

    // Run all tests for all suites
    if(nsl_strlen(logfilename) > 0)
    {
        CU_set_output_filename(logfilename);
        CU_automated_run_tests();
    }
    else
    {
        result = CU_basic_run_tests();
    }

    testsFailed = CU_get_number_of_tests_failed();

    endTime = PAL_ClockGetUnixTime();

    {
        nb_unixTime minutes = (endTime - startTime) / 60;
        nb_unixTime seconds = (endTime - startTime) % 60;

        PRINTF("\nTest time: %d minutes, %d seconds\n\n", minutes, seconds);
    }

    if (pal)
    {
        PAL_Destroy(pal);
        pal = NULL;
    }

    // Run test for one particular suite
    //result = CU_basic_run_suite(CU_pSuite pSuite);

    // Run one single test
    //result = CU_basic_run_test(CU_pSuite pSuite, CU_pTest pTest);

    // Show failures
    //CU_basic_show_failures(CU_pFailureRecord pFailure);

    // Cleanup Main Registry
    CU_cleanup_registry();
    return testsFailed;
}

CU_pSuite CreateTestSuite(const char* name, CU_InitializeFunc setup, CU_CleanupFunc cleanup, SuiteTestAdder adder)
{
    g_countSuites++;
/*
    Suite should correspond to this process. Check that number of current suite is located between
    first appropriate suite number and the last.
*/
    if (g_countSuites >= g_start && g_countSuites < g_start + g_step)
    {
        CU_pSuite pTestSuite = CU_add_suite(name, setup, cleanup);

        if (pTestSuite)
        {
            // Add all tests to suite
            adder(pTestSuite, Test_OptionsGet()->testLevel);
            CU_add_test(pTestSuite, "CheckHeap", &TestHeapCheck);
        }

        return pTestSuite;
    }
    else
    {
        return NULL;
    }
}

/*! @} */
