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
#include "pal.h"
#include "palstdlib.h"
#include "platformutil.h"
#include "palconfig.h"
#include "utility.h"

#include <signal.h>
#include <glib.h>
#include <glib-object.h>

typedef struct Event{
    GMainLoop*  mainLoop;
    guint timerId;
    gboolean value;
} Event;

void* CreateCallbackCompletedEvent(void)
{
    Event *event = (Event *)nsl_malloc(sizeof(Event));
    if (event)
    {
        event->mainLoop = NULL;
        event->timerId = 0;
        event->value = FALSE;
    }

    return event;
}

void DestroyCallbackCompletedEvent(void* callbackCompletedEvent)
{
    Event *event = (Event *)callbackCompletedEvent;

    if (event)
    {
        if (event->timerId)
        {
            g_source_remove(event->timerId);
            event->timerId = 0;
        }

        if (event->mainLoop)
        {
            g_main_loop_unref(event->mainLoop);
            event->mainLoop = NULL;
        }

        nsl_free(event);
    }
}

static gboolean EventTimeoutCallback(Event* event)
{
    if (event)
    {
        event->timerId = 0; /* timeout will be destroy automatically */

        if (event->mainLoop)
        {
            g_main_loop_quit(event->mainLoop);
        }
    }

    return FALSE;   /* one-shot timer */
}

nb_boolean WaitForCallbackCompletedEvent(void* callbackCompletedEvent,
        uint32 timeoutMsec)
{
    Event *event = (Event *)callbackCompletedEvent;
    nb_boolean result = FALSE;

    if (event == NULL)
    {
        return FALSE;
    }

    if (event->value)   /* event signaled before wait */
    {
        event->value = FALSE; /* reset event */
        return TRUE;
    }

    if (event->mainLoop == NULL)
    {
        event->mainLoop = g_main_loop_new(NULL, FALSE);
        if (event->mainLoop == NULL)   /* can not create event loop */
        {
            return FALSE;
        }
    }

    if (timeoutMsec)
    {
        event->timerId = g_timeout_add(timeoutMsec,
                (GSourceFunc)EventTimeoutCallback, event);
        if (event->timerId == 0)
        {
            g_main_loop_unref(event->mainLoop);
            event->mainLoop = NULL;
            return FALSE;
        }
    }

    g_main_loop_run(event->mainLoop);

    /* timeout or event signaled */
    g_main_loop_unref(event->mainLoop);
    event->mainLoop = NULL;

    result = event->value;
    event->value = FALSE;   /* reset event */

    return (result);
}

void SetCallbackCompletedEvent(void* callbackCompletedEvent)
{
    Event *event = (Event *)callbackCompletedEvent;

    if (event)
    {
        event->value = TRUE;
        if (event->timerId)		/* cancel timeout timer */
        {
            g_source_remove(event->timerId);
            event->timerId = 0;
        }

        if (event->mainLoop)	/* wake up waiting event loop */
        {
            g_main_loop_quit(event->mainLoop);
        }
    }
}

void ResetCallbackCompletedEvent(void* callbackCompletedEvent)
{
    Event *event = (Event *)callbackCompletedEvent;

    if (event)
    {
        event->value = FALSE;
    }
}

PAL_Instance* PAL_CreateInstance()
{
    PAL_Config palConfig = { 0 };

    return PAL_Create(&palConfig);
}

void PAL_DestroyInstance(PAL_Instance* pal)
{
    if (!pal)
    {
        return;
    }

    PAL_Destroy(pal);
}

void InitializeHeapCheck()
{
}


void TestHeapCheck(void)
{
}

static Test_Options g_TestOptions;

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

uint64 GetMobileDirectoryNumber(PAL_Instance* pal)
{
    return GetMobileDirectoryNumberFromFile(pal);
}

uint64 GetMobileInformationNumber(PAL_Instance* pal)
{
    return DEFAULT_MOBILE_IDENTIFIER_NUMBER;
}

nb_boolean RunningInEmulator()
{
    return FALSE;
}

nb_boolean GetDeviceName(PAL_Instance* pal, char* buffer, nb_size bufferSize)
{
    nsl_strncpy(buffer, "linux", bufferSize);
    return TRUE;
}

static gboolean StopEventLoop(GMainLoop *mainLoop)
{
    if (mainLoop)
    {
        g_main_loop_quit(mainLoop);
    }

    return FALSE;
}

static void FlushEventLoop(void)
{
    GMainLoop *mainLoop = g_main_loop_new(NULL, FALSE);

    g_idle_add((GSourceFunc)StopEventLoop, mainLoop);
    g_main_loop_run(mainLoop);

    g_main_loop_unref(mainLoop);
}

void SigPipeHandler(int signalNum)
{
    printf("Signal %d caught!\n", signalNum);
#if 0
    abort();      /* abort to generate core file*/
#endif
}

int main(int argc, char *argv[])
{
    int result = 0;
    g_thread_init(NULL);
    g_type_init();

    // catch SIGPIPE, otherwise the program might be terminated silently.
    //@TODO: other signals?
    signal(SIGPIPE, SigPipeHandler);

    result = test_main(argc, argv);

    FlushEventLoop();

    return result;
}

/*! @} */
