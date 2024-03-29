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

This file contains the Linux platform implementation of the platform-
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
#include "systemtests.h"
#include "ThreadCtx.h"
#include "pal.h"
#include "palconfig.h"
#include "palstdlib.h"
#include "PlatformUtil.h"
#include "st_brewutil.h"

#define TEST_BASE_PATH "NB_UNITTEST_DATA"

void SuspendTimeOut(void* callbackCompletedEvent)
{
  IShell* sh = brew_getshell();
  systemtests* papp = (systemtests*) GETAPPINSTANCE();
  ISHELL_CancelTimer(sh, SuspendTimeOut, callbackCompletedEvent);
  ISHELL_Resume( papp->t1->shell, ITHREAD_GetResumeCBK( papp->t1->pit ) );
  ITHREAD_Suspend( papp->t1->pit ); 
}

void* CreateCallbackCompletedEvent(void)
{
  Event* event = (Event*) nsl_malloc(sizeof(Event));
  uint8 id;
  if (event)
  {
    GETRAND(&id, sizeof(uint8));
    event->id = id; //just giving this thing a unique identifier
    event->signalled = FALSE;
  }

  return event;
}

void DestroyCallbackCompletedEvent(void* callbackCompletedEvent)
{
  Event *event = (Event *)callbackCompletedEvent;
  IShell* sh = brew_getshell();
  ISHELL_CancelTimer(sh, NULL, callbackCompletedEvent);
  if (event)
  {
    nsl_free(event);
  }
}

boolean WaitForCallbackCompletedEvent(void* callbackCompletedEvent,
                    uint32 timeoutMsec)
{
  IShell* sh = brew_getshell();
  systemtests* papp = (systemtests*) GETAPPINSTANCE();
  ISHELL_SetTimer(sh, timeoutMsec, SuspendTimeOut, callbackCompletedEvent);
  ITHREAD_Suspend( papp->t1->pit );

  return TRUE;
}

void SetCallbackCompletedEvent(void* callbackCompletedEvent)
{   
  Event *event = (Event*) callbackCompletedEvent;
  IShell* sh = brew_getshell();
  systemtests* papp = (systemtests*) GETAPPINSTANCE();
  ISHELL_CancelTimer(sh, SuspendTimeOut, callbackCompletedEvent);
  ISHELL_Resume( papp->t1->shell, ITHREAD_GetResumeCBK( papp->t1->pit ) );
  ITHREAD_Suspend( papp->t1->pit ); 
  event->signalled = TRUE;
}

PAL_Instance* PAL_CreateInstance()
{
  PAL_Config* palConfig = (PAL_Config*)nsl_malloc(sizeof(PAL_Config));

  palConfig->shell = brew_getshell();

  return PAL_Create(palConfig);
}

void InitializeHeapCheck()
{
}


void TestHeapCheck(void)
{
}

nb_boolean RunningInEmulator()
{
#if defined(AEE_SIMULATOR)
    return TRUE;
#else
    return FALSE;
#endif
}

nb_boolean GetDeviceName(PAL_Instance* pal, char* buffer, nb_size bufferSize)
{
    /// @todo Add code to get device name
    nsl_assert(FALSE);
    return TRUE;
}
Test_Options g_TestOptions;
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
uint64 GetMobileDirectoryNumber(PAL_Instance* pal)
{
    return DEFAULT_MOBILE_DIRECTORY_NUMBER;
}

uint64 GetMobileInformationNumber(PAL_Instance* pal)
{
    return DEFAULT_MOBILE_IDENTIFIER_NUMBER;
}

void
ResetCallbackCompletedEvent(void* callbackCompletedEvent)
{
    DestroyCallbackCompletedEvent(callbackCompletedEvent);
}

void PAL_DestroyInstance(PAL_Instance* pal)
{
    if (!pal)
    {
        return;
    }

    PAL_Destroy(pal);
}

const char* GetBasePath()
{
    
    return TEST_BASE_PATH;
}
/*! @} */
