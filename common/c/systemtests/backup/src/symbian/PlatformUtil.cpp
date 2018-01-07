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

	@file	 PlatformUtil.c
	@date	 05/20/2009
	@defgroup PLATFORM_UTIL Platform-specific Utility Functions

	Implementation of platform-specific utilities used for system test.

	This file contains the Symbian platform implementation of the platform-
	specific utilities needed for system testing.
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

#include "PlatformUtil.h"
#include "palstdlib.h"
#include "palconfig.h"
#include "cunit.h"

#include "pal.h"
#include "Event.h"

Test_Options g_TestOptions;

void* CreateCallbackCompletedEvent(void)
	{
	CEvent* event = CEvent::NewL();
	return event;
	}

void ProcessPendingMessages(void)
	{
	}

nb_boolean WaitForCallbackCompletedEvent(void* callbackCompletedEvent, uint32 timeoutMsec)
	{
	CEvent* event = static_cast<CEvent*>(callbackCompletedEvent);
	return event->Start(timeoutMsec);
	}

void SetCallbackCompletedEvent(void* callbackCompletedEvent)
	{
	CEvent* event = static_cast<CEvent*>(callbackCompletedEvent);
	event->Complete();
	}

void ResetCallbackCompletedEvent(void* /*callbackCompletedEvent*/)
	{
//	ResetEvent(callbackCompletedEvent);
	}

void DestroyCallbackCompletedEvent(void* callbackCompletedEvent)
	{
	CEvent* event = static_cast<CEvent*>(callbackCompletedEvent);
	delete event;
	}

PAL_Instance* PAL_CreateInstance()
	{
	PAL_Config palConfig;
	memclr(&palConfig, sizeof(palConfig));
	PAL_Instance* pal = PAL_Create(&palConfig);
	return pal;
	}

void PAL_DestroyInstance(PAL_Instance* pal)
	{
	if (pal != NULL)
		{
		PAL_Destroy(pal);
		}
	}

Test_Options* Test_OptionsCreate()
	{
	nsl_memset(&g_TestOptions, 0, sizeof(g_TestOptions));
	g_TestOptions.logLevel = LogLevelNone;
	g_TestOptions.networkType = TestNetworkTcp;
	g_TestOptions.testLevel = TestLevelIntermediate;
	g_TestOptions.overwriteFlag = TestOverwriteFlagFalse;

	return &g_TestOptions;
	}

Test_Options* Test_OptionsGet()
	{
	return &g_TestOptions;
	}

void Test_OptionsDestroy()
	{
	}

void InitializeHeapCheck()
	{
	}

void TestHeapCheck(void)
	{
	}

uint64 GetMobileDirectoryNumberFromFile(PAL_Instance* /*pal*/)
	{
	#define DEFAULT_MOBILE_DIRECTORY_NUMBER     9494833800ULL
	return DEFAULT_MOBILE_DIRECTORY_NUMBER;
	}

uint64 GetMobileDirectoryNumber(PAL_Instance* pal)
	{
	return GetMobileDirectoryNumberFromFile(pal);
	}

uint64 GetMobileInformationNumber(PAL_Instance* /*pal*/)
	{
	#define DEFAULT_MOBILE_IDENTIFIER_NUMBER    999999999999999ULL
	return DEFAULT_MOBILE_IDENTIFIER_NUMBER;
	}

nb_boolean RunningInEmulator()
	{
#ifdef __WINSCW__
	return TRUE;
#else
	return FALSE;
#endif
	}

nb_boolean GetDeviceName(PAL_Instance* pal, char* buffer, nb_size bufferSize)
	{
	/// @todo Add code to get device name
	nsl_assert(FALSE);
	nsl_strncpy(buffer, "s60", bufferSize);
	return TRUE;
	}

//_LIT(KTextConsoleTitle, "Console");
//_LIT(KTextPressAnyKey, " [press any key]\n");

//CConsoleBase* console;

TInt doExampleL()
	{
	char**	  argvar	  = NULL;
	int		 index	   = 0;
	int		 i		   = 0;
	int		 errorCount  = 0;
	const char* progname	= "systemtests";

	argvar = (char**)nsl_malloc(1);
	if (argvar == NULL)
		{
		goto exit;
		}
	nsl_memset(argvar, 0, 1);

	argvar[index] = (char*)nsl_malloc(nsl_strlen(progname)+1);
	if(argvar[index] ==  NULL)
	{
		goto exit;
	}
	nsl_memset(argvar[index], 0, nsl_strlen(progname)+1);
	nsl_strcpy(argvar[index++], progname);

	errorCount = test_main(index, argvar);

exit:
	for (i = 0; i < index; ++i)
	{
		nsl_free(argvar[i]);
	}
	nsl_free (argvar); 

	return errorCount;
	}

LOCAL_C void callExampleL()
	{
	CActiveScheduler* scheduler = new (ELeave) CActiveScheduler();
	CleanupStack::PushL(scheduler);
	CActiveScheduler::Install(scheduler);

//	console = Console::NewL(KTextConsoleTitle, TSize(KConsFullScreen,KConsFullScreen));
//	CleanupStack::PushL(console);
	TRAPD(error,doExampleL());
//	console->Printf(KTextPressAnyKey);
//	console->Getch();
//	CleanupStack::PopAndDestroy(console);

	CleanupStack::PopAndDestroy(scheduler);
	}

GLDEF_C TInt E32Main()
	{
	__UHEAP_MARK;
	CTrapCleanup* cleanup = CTrapCleanup::New();
	TRAPD(error, callExampleL());
	ASSERT(!error);
	delete cleanup;
	__UHEAP_MARKEND;
	return 0;
	}

/*! @} */
