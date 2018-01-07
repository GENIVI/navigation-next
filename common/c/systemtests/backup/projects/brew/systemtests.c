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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

/*===========================================================================

FILE: systemtests.c
===========================================================================*/


/*===============================================================================
INCLUDES AND VARIABLE DEFINITIONS
=============================================================================== */
#include "AEEModGen.h"          // Module interface definitions
#include "AEEAppGen.h"          // Applet interface definitions
#include "AEEShell.h"           // Shell interface definitions
#include "AEEText.h"
#include "AEEStdLib.h"

#include "AEETapi.h"      // TAPI Interface definitions

#include "systemtests.h"
#include "systemtests.bid"
#include "namebitmaskmap.h"

#include "CUnit.h"
#include "Basic.h"
#include "PlatformUtil.h"
#include "TestRun.h"
#include "TestDB.h"

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
#include "testanalytics.h"
#include "testers.h"
#include "testlicense.h"
#include "testlocation.h"
#include "testlocationservices.h"
#include "testenhancedcontent.h"


#include "st_brewutil.h"

#define BUFFER_LENGTH 100
////////////////////////////////////////

// All test suites
static const char TEST_SUITE_GPSTYPES[]         = "TestSuiteGPSTypes";
static const char TEST_SUITE_NETWORK[]          = "TestSuiteNetwork";
static const char TEST_SUITE_RASTERMAP[]        = "TestSuiteRasterMap";
static const char TEST_SUITE_RASTERTILE[]       = "TestSuiteRasterTile";
static const char TEST_SUITE_VECTORMAP[]        = "TestSuiteVectorMap";
static const char TEST_SUITE_GEOCODE[]          = "TestSuiteGeocode";
static const char TEST_SUITE_SEARCHBUILDER[]    = "TestSuiteSearchBuilder";
static const char TEST_SUITE_NAV[]              = "TestSuiteNavigation";
static const char TEST_SUITE_DIRECTIONS[]       = "TestSuiteDirections";
static const char TEST_SUITE_SPATIAL[]          = "TestSuiteSpatial";
static const char TEST_SUITE_SPEECH[]           = "TestSuiteSpeech";
static const char TEST_SUITE_FILESET[]          = "TestSuiteFileset";
static const char TEST_SUITE_MOTD[]             = "TestSuiteMotd";
static const char TEST_SUITE_PLACEMESSAGE[]     = "TestSuitePlaceMessage";
static const char TEST_SUITE_SYNC[]             = "TestSuiteSync";
static const char TEST_SUITE_PROFILE[]          = "TestSuiteProfile";
static const char TEST_SUITE_DATASTORE[]        = "TestSuiteDataStore";
static const char TEST_SUITE_SMS[]              = "TestSms";
static const char TEST_SUITE_REVERSEGEOCODE[]   = "TestReverseGeocode";
static const char TEST_SUITE_QALOG[]            = "TestQaLog";
static const char TEST_SUITE_SUBSCRIPTION[]     = "TestSubscription";
static const char TEST_SUITE_TRISTRIP[]         = "TestTriStrip";
static const char TEST_SUITE_ANALYTICS[]        = "TestAnalytics";
static const char TEST_SUITE_ERS[]              = "TestErs";
static const char TEST_SUITE_LICENSE[]          = "TestLicense";
static const char TEST_SUITE_LOCATION[]         = "TestLocation";
static const char TEST_SUITE_ENHANCEDCONTENT[]  = "TestEnhancedContent";
static const char TEST_SUITE_LOCATIONSERVICES[] = "TestLocationServices";
static const char TEST_SUITE_TILESERVICE[]      = "TestTileService";




/*--------------------------------------------------------------------
Function Prototypes
--------------------------------------------------------------------*/
static  boolean systemtests_HandleEvent(systemtests *pMe, 
                                                   AEEEvent eCode, uint16 wParam, 
                                                   uint32 dwParam);
static boolean systemtests_InitAppData(systemtests *pMe);
static void systemtests_FreeAppData(systemtests *pMe);
 
void DisplayText(systemtests *pMe, ITextCtl *txtCtl, const char *format, ...);

static void ThreadStart( IThread *pThread, systemtests *papp );
static void ExitThread();
static void ThreadDone( systemtests *pMe );
static void PrintResults(systemtests *pMe);

typedef void (*SuiteTestAdder)(CU_pSuite, int);
CU_pSuite CreateTestSuite(const char *name, CU_InitializeFunc setup, CU_CleanupFunc cleanup, SuiteTestAdder adder);

/*===============================================================================
FUNCTION DEFINITIONS
=============================================================================== */

/*===========================================================================
FUNCTION: AEEClsCreateInstance

DESCRIPTION
   This function is invoked while the app is being loaded. All Modules must provide this 
   function. Ensure to retain the same name and parameters for this function.
   In here, the module must verify the ClassID and then invoke the AEEApplet_New() function
   that has been provided in AEEAppGen.c. 

   After invoking AEEApplet_New(), this function can do app specific initialization. In this
   example, a generic structure is provided so that app developers need not change app specific
   initialization section every time except for a call to IDisplay_InitAppData(). 
   This is done as follows: InitAppData() is called to initialize AppletData 
   instance. It is app developers responsibility to fill-in app data initialization 
   code of InitAppData(). App developer is also responsible to release memory 
   allocated for data contained in AppletData -- this can be done in 
   IDisplay_FreeAppData().

PROTOTYPE:
   int AEEClsCreateInstance(AEECLSID ClsId,IShell * pIShell,IModule * po,void ** ppObj)

PARAMETERS:
  clsID: [in]: Specifies the ClassID of the applet which is being loaded

  pIShell: [in]: Contains pointer to the IShell object. 

  pIModule: pin]: Contains pointer to the IModule object to the current module to which
  this app belongs

  ppObj: [out]: On return, *ppObj must point to a valid IApplet structure. Allocation
  of memory for this structure and initializing the base data members is done by AEEApplet_New().

DEPENDENCIES
  none

RETURN VALUE
  AEE_SUCCESS: If the app needs to be loaded and if AEEApplet_New() invocation was
     successful
  EFAILED: If the app does not need to be loaded or if errors occurred in 
     AEEApplet_New(). If this function returns FALSE, the app will not be loaded.

SIDE EFFECTS
  none
===========================================================================*/
int AEEClsCreateInstance(AEECLSID ClsId, IShell *pIShell, IModule *po, void **ppObj)
{
  *ppObj = NULL;

  if( ClsId == AEECLSID_SYSTEMTESTS )
  {
    // Create the applet and make room for the applet structure
    if( AEEApplet_New(sizeof(systemtests),
                          ClsId,
                          pIShell,
                          po,
                          (IApplet**)ppObj,
                          (AEEHANDLER)systemtests_HandleEvent,
                          (PFNFREEAPPDATA)systemtests_FreeAppData) ) // the FreeAppData function is called after sending EVT_APP_STOP to the HandleEvent function
    {
      //Initialize applet data, this is called before sending EVT_APP_START
            // to the HandleEvent function
      if(systemtests_InitAppData((systemtests*)*ppObj))
      {
        //Data initialized successfully
        return(AEE_SUCCESS);
      }
      else
      {
        //Release the applet. This will free the memory allocated for the applet when
        // AEEApplet_New was called.
        IAPPLET_Release((IApplet*)*ppObj);
        return EFAILED;
      }

    } // end AEEApplet_New

  }

  return(EFAILED);
}


/*===========================================================================
FUNCTION SampleAppWizard_HandleEvent

DESCRIPTION
  This is the EventHandler for this app. All events to this app are handled in this
  function. All APPs must supply an Event Handler.

PROTOTYPE:
  boolean SampleAppWizard_HandleEvent(IApplet * pi, AEEEvent eCode, uint16 wParam, uint32 dwParam)

PARAMETERS:
  pi: Pointer to the AEEApplet structure. This structure contains information specific
  to this applet. It was initialized during the AEEClsCreateInstance() function.

  ecode: Specifies the Event sent to this applet

   wParam, dwParam: Event specific data.

DEPENDENCIES
  none

RETURN VALUE
  TRUE: If the app has processed the event
  FALSE: If the app did not process the event

SIDE EFFECTS
  none
===========================================================================*/

//////////////////////////////////////////


static boolean systemtests_HandleEvent(systemtests *pMe, AEEEvent eCode, uint16 wParam, uint32 dwParam)
{
    switch (eCode) 
    {
        // App is told it is starting up
        case EVT_APP_START:
          // Add your code here...
          DisplayText(pMe, pMe->m_textCtl, "SYSTEM TESTS ARE IN PROGRESS...");
          CALLBACK_Init( &pMe->t1Done, (PFNNOTIFY)ThreadDone, (void*) pMe );
          ITHREAD_Join( pMe->t1->pit, &pMe->t1Done, &pMe->t1Result );
          ITHREAD_Start( pMe->t1->pit, 1024, (PFNTHREAD)ThreadStart, (void*) pMe);   

          return(TRUE);

        // App is told it is exiting
        case EVT_APP_STOP:
          // Add your code here...
          ITHREAD_Exit( pMe->t1->pit , SUCCESS );
          return(TRUE);

        // App is being suspended 
        case EVT_APP_SUSPEND:
          ITHREAD_Suspend( pMe->t1->pit );
          return(TRUE);

        case EVT_EXIT_THREAD:
          ITHREAD_Exit( pMe->t1->pit , SUCCESS );
          return(TRUE);

        // App is being resumed
        case EVT_APP_RESUME:
          return(TRUE);

        // An SMS message has arrived for this app. Message is in the dwParam above as (char *)
        // sender simply uses this format "//BREW:ClassId:Message", example //BREW:0x00000001:Hello World
        case EVT_APP_MESSAGE:
          // Add your code here...

            return(TRUE);

        // A key was pressed. Look at the wParam above to see which key was pressed. The key
        // codes are in AEEVCodes.h. Example "AVK_1" means that the "1" key was pressed.
        case EVT_KEY:
          // Add your code here...

          return(TRUE);

        // If nothing fits up to this point then we'll just break out
        default:
            break;
   }

   return FALSE;
}

// this function is called when your application is starting up
boolean systemtests_InitAppData(systemtests *pMe)
{
    CU_ErrorCode    result       = CUE_SUCCESS;
    Test_Options    *testOptions = NULL;
    AEERect         rectMenu     = {0};

    pMe->DeviceInfo.wStructSize = sizeof(pMe->DeviceInfo);
    ISHELL_GetDeviceInfo(pMe->a.m_pIShell,&pMe->DeviceInfo);

    pMe->m_pShell   = pMe->a.m_pIShell;
    pMe->m_pDisplay = pMe->a.m_pIDisplay;
    pMe->m_width    = pMe->DeviceInfo.cxScreen;
    pMe->m_height   = pMe->DeviceInfo.cyScreen;

    pMe->t1 = nsl_malloc(sizeof(ThreadCtx));
    if ( !pMe->t1 )
    {
      return FALSE;
    }

    pMe->t1->shell = pMe->a.m_pIShell;
    pMe->t1->n = 0;

    InitializeHeapCheck();

    if(AEE_SUCCESS != ISHELL_CreateInstance(pMe->m_pShell, AEECLSID_THREAD, (void **)&pMe->t1->pit))
    {
       return FALSE;
    }


    if ( ISHELL_CreateInstance(pMe->m_pShell, AEECLSID_TEXTCTL, (void**) &(pMe->m_textCtl)) == SUCCESS )
    {
      SETAEERECT (&rectMenu, 15, 0, pMe->DeviceInfo.cxScreen, 20);
      ITEXTCTL_SetRect(pMe->m_textCtl, &rectMenu);
      IDISPLAY_ClearScreen(pMe->m_pDisplay);
    }

    if ( ISHELL_CreateInstance(pMe->m_pShell, AEECLSID_TEXTCTL, (void**) &(pMe->m_txtTestsRun)) == SUCCESS )
    {
      SETAEERECT (&rectMenu, 5, 30, pMe->DeviceInfo.cxScreen, 20);
      ITEXTCTL_SetRect(pMe->m_txtTestsRun, &rectMenu);
      IDISPLAY_ClearScreen(pMe->m_pDisplay);
    }

    if ( ISHELL_CreateInstance(pMe->m_pShell, AEECLSID_TEXTCTL, (void**) &(pMe->m_txtTestsFailed)) == SUCCESS )
    {
      SETAEERECT (&rectMenu, 5, 60, pMe->DeviceInfo.cxScreen, 20);
      ITEXTCTL_SetRect(pMe->m_txtTestsFailed, &rectMenu);
      IDISPLAY_ClearScreen(pMe->m_pDisplay);
    }

    result = CU_initialize_registry();
    if ( result != CUE_SUCCESS )
    {
        return 0;
    }
    // Set output verbosity 
    CU_basic_set_mode(CU_BRM_VERBOSE); //CU_BRM_NORMAL );

    testOptions = Test_OptionsGet();

    if(testOptions->suites = SUITE_NETWORK)
    {
        // Add Network test suite.
        CreateTestSuite(TEST_SUITE_NETWORK, &TestNetwork_SuiteSetup, &TestNetwork_SuiteCleanup, TestNetwork_AddTests);
    }

    if(testOptions->suites = SUITE_RASTERTILE)
    {
        // Add raster map test suite.
        CreateTestSuite(TEST_SUITE_RASTERTILE, &TestRasterTile_SuiteSetup, &TestRasterTile_SuiteCleanup, TestRasterTile_AddTests);
    }

    if(testOptions->suites = SUITE_GPSTYPES)
    {
        // Add GPS Types test suite.
        CreateTestSuite(TEST_SUITE_GPSTYPES, &TestGPSTypes_SuiteSetup, &TestGPSTypes_SuiteCleanup, TestGPSTypes_AddTests);
    }

    if(testOptions->suites = SUITE_VECTORMAP)
    {
        // Add Vector Map test suite.
        CreateTestSuite(TEST_SUITE_VECTORMAP, &TestVectorMap_SuiteSetup, &TestVectorMap_SuiteCleanup, TestVectorMap_AddTests);
    }

    if(testOptions->suites = SUITE_GEOCODE)
    {
        // Add Geocode test suite.
        CreateTestSuite(TEST_SUITE_GEOCODE, &TestGeocode_SuiteSetup, &TestGeocode_SuiteCleanup, TestGeocode_AddTests);
    }

    if(testOptions->suites = SUITE_REVERSEGEOCODE)
    {
        CreateTestSuite(TEST_SUITE_REVERSEGEOCODE, &TestReverseGeocode_SuiteSetup, &TestReverseGeocode_SuiteCleanup, TestReverseGeocode_AddTests);
    }

    /*if(testOptions->suites = SUITE_SEARCHBUILDER)//crash
    {
        // Add SearchBuilder test suite.
        CreateTestSuite(TEST_SUITE_SEARCHBUILDER, &TestSearchBuilder_SuiteSetup, &TestSearchBuilder_SuiteCleanup, TestSearchBuilder_AddTests);
    }*/

    if(testOptions->suites = SUITE_NAVIGATION)
    {
        // Add Navigation test suite.
        CreateTestSuite(TEST_SUITE_NAV, &TestNavigation_SuiteSetup, &TestNavigation_SuiteCleanup, TestNavigation_AddTests);
    }

    /*if(testOptions->suites = SUITE_DIRECTIONS)//crash
    {
        // Add Direction test suite.
        CreateTestSuite(TEST_SUITE_DIRECTIONS, &TestDirections_SuiteSetup, &TestDirections_SuiteCleanup, TestDirections_AddTests);
    }*/

    if(testOptions->suites = SUITE_SPATIAL)
    {
        // Add spatial test suite.
        CreateTestSuite(TEST_SUITE_SPATIAL, &TestSpatial_SuiteSetup, &TestSpatial_SuiteCleanup, TestSpatial_AddTests);
    }

    if(testOptions->suites = SUITE_ASR)
    {
    // Add speech test suite.
    CreateTestSuite(TEST_SUITE_SPEECH, &TestSpeech_SuiteSetup, &TestSpeech_SuiteCleanup, TestSpeech_AddTests);
    }

    if(testOptions->suites = SUITE_FILESET)
    {
        // Add FileSet test suite.
        CreateTestSuite(TEST_SUITE_FILESET, &TestFileset_SuiteSetup, &TestFileset_SuiteCleanup, TestFileset_AddTests);
    }

    if(testOptions->suites = SUITE_MOTD)
    {
        // Add MOTD test suite.
        CreateTestSuite(TEST_SUITE_MOTD, &TestMotd_SuiteSetup, &TestMotd_SuiteCleanup, TestMotd_AddTests);
    }

    if(testOptions->suites = SUITE_PLACEMESSAGE)
    {
        // Add Place Message test suite.
        CreateTestSuite(TEST_SUITE_PLACEMESSAGE, &TestPlaceMessage_SuiteSetup, &TestPlaceMessage_SuiteCleanup, TestPlaceMessage_AddTests);
    }

    if(testOptions->suites = SUITE_SYNC)
    {
        // Add Sync test suite.
        CreateTestSuite(TEST_SUITE_SYNC, &TestSync_SuiteSetup, &TestSync_SuiteCleanup, TestSync_AddTests);
    }

    if(testOptions->suites = SUITE_PROFILE)
    {
        // Add Profile test suite.
        CreateTestSuite(TEST_SUITE_PROFILE, &TestProfile_SuiteSetup, &TestProfile_SuiteCleanup, TestProfile_AddTests);
    }

    if(testOptions->suites = SUITE_DATASTORE)
    {
        // Add DataStore test suite.
        CreateTestSuite(TEST_SUITE_DATASTORE, &TestDataStore_SuiteSetup, &TestDataStore_SuiteCleanup, TestDataStore_AddTests);
    }

    if(testOptions->suites = SUITE_SMS)
    {
        // Add SMS test suite.
        CreateTestSuite(TEST_SUITE_SMS, &TestSms_SuiteSetup, &TestSms_SuiteCleanup, TestSms_AddTests);
    }

    /*if(testOptions->suites = SUITE_QALOG)//crash
    {
        CreateTestSuite(TEST_SUITE_QALOG, &TestQaLog_SuiteSetup, &TestQaLog_SuiteCleanup, TestQaLog_AddTests);
    }*/

    if(testOptions->suites = SUITE_SUBSCRIPTION)
    {
        CreateTestSuite(TEST_SUITE_SUBSCRIPTION, &TestSubscription_SuiteSetup, &TestSubscription_SuiteCleanup, TestSubscription_AddTests);
    }

    if (testOptions->suites = SUITE_TRISTRIP)
    {
        CreateTestSuite(TEST_SUITE_TRISTRIP, &TestTriStrip_SuiteSetup, &TestTriStrip_SuiteCleanup, TestTriStrip_AddTests);
    }

    /*if (testOptions->suites = SUITE_ANALYTICS)//crash
    {
        CreateTestSuite(TEST_SUITE_ANALYTICS, &TestAnalytics_SuiteSetup, &TestAnalytics_SuiteCleanup, TestAnalytics_AddTests);
    }*/

    if (testOptions->suites = SUITE_ERS)
    {
        CreateTestSuite(TEST_SUITE_ERS, &TestErs_SuiteSetup, &TestErs_SuiteCleanup, TestErs_AddTests);
    }

    if (testOptions->suites = SUITE_LICENSE)
    {
        CreateTestSuite(TEST_SUITE_LICENSE, &TestLicense_SuiteSetup, &TestLicense_SuiteCleanup, TestLicense_AddTests);
    }

    if (testOptions->suites = SUITE_LOCATION)
    {
        CreateTestSuite(TEST_SUITE_LOCATION, &TestLocation_SuiteSetup, &TestLocation_SuiteCleanup, TestLocation_AddTests);
    }

    if (testOptions->suites = SUITE_ENHANCEDCONTENT)
    {
        CreateTestSuite(TEST_SUITE_ENHANCEDCONTENT, &TestEnhancedContent_SuiteSetup, &TestEnhancedContent_SuiteCleanup, TestEnhancedContent_AddTests);
    }

    if (testOptions->suites = SUITE_LOCATIONSERVICES)
    {
        CreateTestSuite(TEST_SUITE_LOCATIONSERVICES, &TestLocationServices_SuiteSetup, &TestLocationServices_SuiteCleanup, TestLocationServices_AddTests);
    }

    if(testOptions->suites = SUITE_TILESERVICE)
    {
        CreateTestSuite(TEST_SUITE_TILESERVICE, &TestTileService_SuiteSetup, &TestTileService_SuiteCleanup, TestTileService_AddTests);
    }

    if ( ISHELL_CreateInstance( pMe->m_pShell, AEECLSID_THREAD, (void **)&pMe->t1->pit)  != SUCCESS )
    {
      return FALSE;
    }

    pMe->t1->shell = pMe->m_pShell;

    return TRUE;
}


ThreadCtx* GetThreadCtx(systemtests *papp)
{
  if (!papp)
    return NULL;

  return papp->t1;
}


CU_pSuite CreateTestSuite(const char *name, CU_InitializeFunc setup, CU_CleanupFunc cleanup, SuiteTestAdder adder)
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


// this function is called when your application is exiting
void systemtests_FreeAppData(systemtests *pMe)
{
    // insert your code here for freeing any resources you have allocated...

    // example to use for releasing each interface:
    if ( pMe->m_textCtl != NULL )         // check for NULL first
    {
      ITEXTCTL_Release(pMe->m_textCtl);        // release the interface
      pMe->m_textCtl = NULL;                   // set to NULL so no problems trying to free later

      ITEXTCTL_Release(pMe->m_txtTestsRun);    // release the interface
      pMe->m_txtTestsRun = NULL;               // set to NULL so no problems trying to free later

      ITEXTCTL_Release(pMe->m_txtTestsFailed); // release the interface
      pMe->m_txtTestsFailed = NULL;            // set to NULL so no problems trying to free later
    }

    nsl_free(pMe->t1);
}

//Draw textbox function
void DisplayText(systemtests *pMe, ITextCtl *txtCtl, const char *format, ...)
{
  va_list args;
  uint32 len = 0;
  char* buffer = NULL;
  AECHAR *textToDisplay = NULL;

  va_start(args, format);
  // this function returns length + 1 for NULL symbol
  len = VSNPRINTF(buffer, len, format, args);

  if ( len > 0 && NULL != (buffer = ( char* )MALLOC(len * sizeof(char))) )
  {
    VSNPRINTF(buffer, len, format, args);
    PRINTF(buffer);
  }

  if (txtCtl && buffer && NULL != (textToDisplay = ( AECHAR* )MALLOC(len * sizeof(AECHAR))))
  {
    STRTOWSTR(buffer, textToDisplay, len * sizeof(AECHAR));
    ITEXTCTL_SetText(txtCtl, textToDisplay, -1);
    ITEXTCTL_SetActive(txtCtl,TRUE);
    (void) ITEXTCTL_Redraw(txtCtl);
  }

  IDISPLAY_Update(pMe->m_pDisplay);

  if (buffer)
  {
    FREE(buffer);
    buffer = NULL;
  }

  if (textToDisplay)
  {
    FREE(textToDisplay);
    textToDisplay = NULL;
  }
}


static void ThreadStart( IThread *pThread, systemtests *papp )
{
  nb_unixTime     startTime   = 0;
  nb_unixTime     endTime     = 0;

  WaitForCallbackCompletedEvent(NULL, 3000); //Wait for some DLLs to be loaded before proceeding.
 
  // Run all tests for all suites
  startTime   = PAL_ClockGetUnixTime();
  papp->t1Result = CU_basic_run_tests();
  endTime = PAL_ClockGetUnixTime();
  DisplayText(papp, papp->m_textCtl, "SYSTEM TESTS ARE FINISHED...");
  DisplayText(papp, NULL, "\nTest time: %d minutes, %d seconds", ( (endTime - startTime) / 60 ), ( (endTime - startTime) % 60 ));

  ExitThread();
}

static void ThreadDone( systemtests *pMe )
{
  PrintResults( pMe );
  CU_cleanup_registry();
  PRINTF( pMe->t1Result == SUCCESS ? "Thread exit SUCCESS" : "Thread exit FAILED" );
  CALLBACK_Cancel( &pMe->t1Done );
}


static void ExitThread()
{
  ISHELL_SendEvent(brew_getshell(), AEECLSID_SYSTEMTESTS, EVT_EXIT_THREAD, 0, 0);
}

static void PrintResults(systemtests *pMe)
{
  CU_pRunSummary pRunSummary = CU_get_run_summary();
  CU_pFailureRecord record = NULL;
  char *testName = NULL;

  DisplayText(pMe, pMe->m_txtTestsRun, "Tests Run: %d", pRunSummary->nTestsRun);
  DisplayText(pMe, pMe->m_txtTestsFailed, "Tests Failed: %d", pRunSummary->nTestsFailed);

  record = CU_get_failure_list();
  while (record != NULL)
  {
    if ( NULL == testName || 0 != nsl_strcmp(record->pTest->pName, testName) )
    {
        testName = record->pTest->pName;
        DisplayText(pMe, NULL, "\n***********Test: %s", testName);
    }
    DisplayText(pMe, NULL, "\nFile: %s...Line: %d...%s", record->strFileName, record->uiLineNumber, record->strCondition);
    record = record->pNext;
  }
}

void SetTestEvent(systemtests *papp, void *event, TestEventID id)
{
  papp->testEvent[id] = event;
}

void* GetTestEvent(systemtests *papp, TestEventID id)
{
  return papp->testEvent[id];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
