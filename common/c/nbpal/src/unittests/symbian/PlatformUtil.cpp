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

    @file     PlatformUtil.c
    @date     02/06/2009
    @defgroup PLATFORM_UTIL Platform-specific Utility Functions

    Implementation of platform-specific utilities used for system test.

    This file contains the Windows platform implementation of the platform-
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
//#include "pal.h"
#include "palstdlib.h"
#include "palconfig.h"
#include "main.h"

#include "Event.h"

void* CreateCallbackCompletedEvent(void)
	{
	CEvent* event = CEvent::NewL();
	return event;
	}

void ProcessPendingMessages(void)
	{
	}

boolean WaitForCallbackCompletedEvent(void* callbackCompletedEvent, uint32 timeoutMsec)
	{
	CEvent* event = static_cast<CEvent*>(callbackCompletedEvent);
	return event->Start(timeoutMsec);
	}

void SetCallbackCompletedEvent(void* callbackCompletedEvent)
	{
	CEvent* event = static_cast<CEvent*>(callbackCompletedEvent);
	event->Complete();
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

TInt doExampleL()
	{
	return test_main();
	}

LOCAL_C void callExampleL()
	{
	CActiveScheduler* scheduler = new (ELeave) CActiveScheduler();
	CleanupStack::PushL(scheduler);
	CActiveScheduler::Install(scheduler);

//	console = Console::NewL(KTextConsoleTitle, TSize(KConsFullScreen,KConsFullScreen));
//	CleanupStack::PushL(console);
	TRAPD(error, doExampleL());
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
