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

FILE: main.c
===========================================================================*/


/*===============================================================================
INCLUDES AND VARIABLE DEFINITIONS
=============================================================================== */
#include "AEEModGen.h"          // Module interface definitions
#include "AEEAppGen.h"          // Applet interface definitions
#include "AEEShell.h"           // Shell interface definitions
#include "AEEText.h"
#include "AEEStdLib.h"

#include "AEEFile.h"			// File interface definitions
#include "AEEDB.h"				// Database interface definitions
#include "AEENet.h"				// Socket interface definitions
#include "AEESound.h"			// Sound Interface definitions
#include "AEETapi.h"			// TAPI Interface definitions

#include "pal.h"
#include "palclock.h"
#include "paltimer.h"
#include "palfile.h"

#include "unittests.bid"
#include "unittests.h"
#include "ThreadCtx.h"
#include "st_brewutil.h"

//Unit Tests
#include "Basic.h"
#include "CUnit.h"
#include "testcache.h"

/*-------------------------------------------------------------------
Function Prototypes
-------------------------------------------------------------------*/
static  boolean unittests_HandleEvent(unittests* pMe,
                                                   AEEEvent eCode, uint16 wParam,
                                                   uint32 dwParam);
boolean unittests_InitAppData(unittests* pMe);
void    unittests_FreeAppData(unittests* pMe);

//Draw textbox function
void DisplayText(unittests* pMe, ITextCtl* txtCtl, AECHAR* buffer);
void DisplayHeaderText(unittests* pMe, const AECHAR* string);
static void ThreadMain( ThreadCtx *thread );
static void ThreadStart( IThread *pThread, ThreadCtx *thread );
static void ThreadDone( unittests *pMe );

// All test suites
static const char testSuiteCache[]     = "TestSuiteCache";


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

	if( ClsId == AEECLSID_UNITTESTS )
	{
		// Create the applet and make room for the applet structure
		if( AEEApplet_New(sizeof(unittests),
                          ClsId,
                          pIShell,
                          po,
                          (IApplet**)ppObj,
                          (AEEHANDLER)unittests_HandleEvent,
                          (PFNFREEAPPDATA)unittests_FreeAppData) ) // the FreeAppData function is called after sending EVT_APP_STOP to the HandleEvent function

		{
			//Initialize applet data, this is called before sending EVT_APP_START
            // to the HandleEvent function
			if(unittests_InitAppData((unittests*)*ppObj))
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
static boolean unittests_HandleEvent(unittests* pMe, AEEEvent eCode, uint16 wParam, uint32 dwParam)
{

	switch (eCode)
	{
		// App is told it is starting up
	case EVT_APP_START:
		// Add your code here...
		DisplayHeaderText(pMe, L"NIMNBIPAL UNIT TESTS");
		CALLBACK_Init( &pMe->t1Done, (PFNNOTIFY)ThreadDone, (void*) pMe );
		ITHREAD_Join( pMe->t1->pit, &pMe->t1Done, &pMe->t1Result );
		ITHREAD_Start( pMe->t1->pit, 512, (PFNTHREAD)ThreadStart, (void*) pMe->t1);
		return(TRUE);

		// App is told it is exiting
	case EVT_APP_STOP:
		// Add your code here...
		CU_cleanup_registry();
		ITHREAD_Exit( pMe->t1->pit , SUCCESS );
		return(TRUE);

		// App is being suspended
	case EVT_APP_SUSPEND:
		return(TRUE);

	case EVT_SUSPEND_THREAD:
		// Add your code here...
		ITHREAD_Suspend( pMe->t1->pit );
		return(TRUE);

	case EVT_EXIT_THREAD:
		{
			AECHAR txtBuffer[100];
			unsigned int testsFailed=0;
			unsigned int testsRun=0;
			CU_pFailureRecord record = NULL;
			CU_pFailureRecord cur = NULL;
			testsRun = CU_get_number_of_tests_run();
			WSPRINTF(txtBuffer, 100, L"Tests Run: %d", testsRun);
			DisplayText(pMe, pMe->m_txtTestsRun, txtBuffer);

			testsFailed = CU_get_number_of_tests_failed();
			WSPRINTF(txtBuffer, 100, L"Tests Failed: %d", testsFailed);
			DisplayText(pMe, pMe->m_txtTestsFailed, txtBuffer);

			record = CU_get_failure_list();
			cur = record;
			while (cur != NULL)
			{
				DBGPRINTF("Failed Tests: %s | %s | %d", cur->pTest->pName, cur->strCondition, cur->uiLineNumber );
				cur = cur->pNext;
			}

			ITHREAD_Exit( pMe->t1->pit , SUCCESS );
		}
		return(TRUE);

		// App is being resumed
	case EVT_APP_RESUME:
		return(TRUE);

	case EVT_RESUME_THREAD:
		// Add your code here...
		ISHELL_Resume( pMe->t1->shell, ITHREAD_GetResumeCBK( pMe->t1->pit ) );
		ITHREAD_Suspend( pMe->t1->pit );
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
boolean unittests_InitAppData(unittests* pMe)
{

	CU_ErrorCode result = CUE_SUCCESS;
	CU_pSuite pTestSuite = NULL;

	pMe->DeviceInfo.wStructSize = sizeof(pMe->DeviceInfo);
	ISHELL_GetDeviceInfo(pMe->a.m_pIShell,&pMe->DeviceInfo);

	pMe->m_pShell = pMe->a.m_pIShell;
	pMe->m_pDisplay = pMe->a.m_pIDisplay;
	pMe->m_width =  pMe->DeviceInfo.cxScreen;
	pMe->m_height = pMe->DeviceInfo.cyScreen;
	pMe->t1 = nsl_malloc(sizeof(ThreadCtx));

	pMe->t1->shell = pMe->m_pShell;
	pMe->t1->n = 0;


	if ( ISHELL_CreateInstance( pMe->m_pShell, AEECLSID_THREAD, (void **)&pMe->t1->pit)  != SUCCESS )
	{
		return FALSE;
	}

	if ( ISHELL_CreateInstance(pMe->m_pShell, AEECLSID_TEXTCTL, (void**) &(pMe->m_textCtl)) == SUCCESS )
	{
		SETAEERECT (&(pMe->m_rectMenu), 0, 0, pMe->DeviceInfo.cxScreen, 10);
		ITEXTCTL_SetRect(pMe->m_textCtl, &(pMe->m_rectMenu));
		IDISPLAY_ClearScreen(pMe->m_pDisplay);
	}

	if ( ISHELL_CreateInstance(pMe->m_pShell, AEECLSID_TEXTCTL, (void**) &(pMe->m_txtTestsRun)) == SUCCESS )
	{
		SETAEERECT (&(pMe->m_rectMenu), 5, 15, pMe->DeviceInfo.cxScreen, 10);
		ITEXTCTL_SetRect(pMe->m_txtTestsRun, &(pMe->m_rectMenu));
		IDISPLAY_ClearScreen(pMe->m_pDisplay);
	}

	if ( ISHELL_CreateInstance(pMe->m_pShell, AEECLSID_TEXTCTL, (void**) &(pMe->m_txtTestsFailed)) == SUCCESS )
	{
		SETAEERECT (&(pMe->m_rectMenu), 5, 30, pMe->DeviceInfo.cxScreen, 10);
		ITEXTCTL_SetRect(pMe->m_txtTestsFailed, &(pMe->m_rectMenu));
		IDISPLAY_ClearScreen(pMe->m_pDisplay);
	}

	// Set output verbosity
	CU_basic_set_mode(CU_BRM_VERBOSE); //CU_BRM_NORMAL );

	// Initialize Main Registry
	result = CU_initialize_registry();

	if (result != CUE_SUCCESS)
	{
		return 0;
	}

	// Add stdcache test suite.
	pTestSuite = CU_add_suite(testSuiteCache, &TestCache_SuiteSetup, &TestCache_SuiteCleanup);
	if (pTestSuite)
	{
		// Add all tests to suite
		TestCache_AddAllTests(pTestSuite);
	}


	return TRUE;
}

// this function is called when your application is exiting
void unittests_FreeAppData(unittests* pMe)
{
    // insert your code here for freeing any resources you have allocated...

    // example to use for releasing each interface:
     if ( pMe->m_textCtl != NULL )         // check for NULL first
     {
        ITEXTCTL_Release(pMe->m_textCtl);   // release the interface
        pMe->m_textCtl = NULL;             // set to NULL so no problems trying to free later

		ITEXTCTL_Release(pMe->m_txtTestsRun);   // release the interface
		pMe->m_txtTestsRun = NULL;             // set to NULL so no problems trying to free later

		ITEXTCTL_Release(pMe->m_txtTestsFailed);   // release the interface
		pMe->m_txtTestsFailed = NULL;             // set to NULL so no problems trying to free later

     }
	 nsl_free(pMe->t1);
}

//Draw textbox function
void DisplayText(unittests* pMe, ITextCtl* txtCtl, AECHAR* buffer)
{
	if (txtCtl)
	{
		ITEXTCTL_SetText(txtCtl, buffer, -1);
		ITEXTCTL_SetActive(txtCtl,TRUE);
		(void) ITEXTCTL_Redraw(txtCtl);
	}

	IDISPLAY_Update(pMe->m_pDisplay);
}

void DisplayHeaderText(unittests* pMe, const AECHAR* string)
{
	AECHAR buffer[120];
	WSPRINTF(buffer, 120, string);
	ITEXTCTL_SetText(pMe->m_textCtl, buffer, -1);
	ITEXTCTL_SetActive(pMe->m_textCtl,TRUE);
	(void) ITEXTCTL_Redraw(pMe->m_textCtl);

	IDISPLAY_Update(pMe->m_pDisplay);
}

static void ThreadMain( ThreadCtx *thread )
{
	CU_basic_run_tests();
	ExitThread();
}

static void ThreadStart( IThread *pThread, ThreadCtx *thread )
{
	ThreadMain( thread );
}

static void ThreadDone( unittests *pMe )
{
	DBGPRINTF( pMe->t1Result == SUCCESS ? "Thread exit SUCCESS" : "Thread exit FAILED" );
}

void SuspendThread()
{
	ISHELL_SendEvent(brew_getshell(), AEECLSID_UNITTESTS, EVT_SUSPEND_THREAD, 0, 0);
}

void ResumeThread()
{
	ISHELL_SendEvent(brew_getshell(), AEECLSID_UNITTESTS, EVT_RESUME_THREAD, 0, 0);
}

void ExitThread()
{
	ISHELL_SendEvent(brew_getshell(), AEECLSID_UNITTESTS, EVT_EXIT_THREAD, 0, 0);
}

int main()
{
    return 0;
}