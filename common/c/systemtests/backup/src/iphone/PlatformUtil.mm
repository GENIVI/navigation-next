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

    @file     PlatformUtil.m
    @date     02/06/2009
    @defgroup PLATFORM_UTIL Platform-specific Utility Functions

    Implementation of platform-specific utilities used for system test.

    This file contains the Linux platform implementation of the platform-
    specific utilities needed for system testing.
    */
    /*
    (C) Copyright 2010 by TeleCommunication Systems.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */
extern "C" {
#include "pal.h"
#include "palstdlib.h"
#include "palfile.h"
#include "platformutil.h"
#include "palconfig.h"
#include "utility.h"
}

#include <signal.h>

#import <Foundation/Foundation.h>

#define QALOG_SMALL_FILE  "qalog_small.qa"
#define QALOG_MEDIUM_FILE "qalog_medium.qa"
#define QALOG_LARGE_FILE  "qalog_large.qa"

/*! Test base path */
#define TEST_BASE_PATH                      "NB_UNITTEST_DATA"

typedef struct Event{
    nb_boolean value;
    CFRunLoopTimerRef timer_ref;
    CFRunLoopRef m_runLoop;
} Event;

void* CreateCallbackCompletedEvent(void)
{
    Event *event = (Event *)nsl_malloc(sizeof(Event));
    if (event)
    {
        event->value = FALSE;
        event->timer_ref = nil;
        event->m_runLoop = nil;
    }

    return event;
}

void DestroyCallbackCompletedEvent(void* callbackCompletedEvent)
{
    Event *event = (Event *)callbackCompletedEvent;

    if (event)
    {
        nsl_free(event);
    }
}

static void EventTimeoutCallback(CFRunLoopTimerRef timer, void* user_data)
{
    if (user_data)
    {
        Event* event = (Event*)user_data;
        if (timer != nil && event->timer_ref == timer)
        {
            CFRunLoopRemoveTimer(event->m_runLoop, event->timer_ref, kCFRunLoopDefaultMode);
            event->timer_ref = nil;
            CFRunLoopStop(event->m_runLoop);
            CFRelease(event->m_runLoop);
            event->m_runLoop = NULL;
        }
    }
}

nb_boolean WaitForCallbackCompletedEvent(void* callbackCompletedEvent, nb_usize timeoutMsec)
{
    Event *event = (Event *)callbackCompletedEvent;
    nb_boolean result = FALSE;
    CFRunLoopTimerContext context = {0, event, NULL, NULL, NULL};
    CFRunLoopTimerRef timer = CFRunLoopTimerCreate(
            NULL, (CFAbsoluteTimeGetCurrent() + (timeoutMsec / 1000.0)), 0, 0, 0, EventTimeoutCallback,
            (CFRunLoopTimerContext*)&context
            );
    event->m_runLoop = CFRunLoopGetCurrent();
    CFRetain(event->m_runLoop);
    CFRunLoopAddTimer(event->m_runLoop, timer, kCFRunLoopDefaultMode);

    assert (event->timer_ref == nil);
    event->timer_ref = timer;

    CFRunLoopRun();
    result = event->value;
    event->value = FALSE;
    return result;
}

void SetCallbackCompletedEvent(void* callbackCompletedEvent)
{
    if (callbackCompletedEvent == nil)
        return;

    Event *event = (Event *)callbackCompletedEvent;
    event->value = TRUE;
    if (event->timer_ref != nil)
    {
        CFRunLoopRemoveTimer(event->m_runLoop, event->timer_ref, kCFRunLoopDefaultMode);
        event->timer_ref = nil;
        CFRunLoopStop(event->m_runLoop);
        CFRelease(event->m_runLoop);
        event->m_runLoop = NULL;
    }
}

void ResetCallbackCompletedEvent(void* callbackCompletedEvent)
{
    Event* event = (Event*) callbackCompletedEvent;

    if (event)
    {
        event->value = FALSE;
    }
}

PAL_Instance* PAL_CreateInstance()
{
    PAL_Config palConfig = {0};   // dummy config
    return PAL_Create(&palConfig);
}

PAL_Instance* PAL_CreateInstanceMultiThread()
{
    PAL_Config palConfig = { 0 };
    palConfig.multiThreaded = TRUE;

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

void CopyQaLogFileIfNecessary(const char* filename)
{
    NSString* filenameString = [NSString stringWithUTF8String:filename];

    NSArray *array = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString* docPath = [array objectAtIndex:0];

    NSString* fileDocFullName = [docPath stringByAppendingFormat:@"/%@", filenameString];

    NSFileManager* fileManager = [NSFileManager defaultManager];
    if (![fileManager fileExistsAtPath:fileDocFullName])
    {
        NSString *path = [[NSBundle mainBundle] pathForResource:filenameString ofType:@""];
        if (path != nil)
        {
            [fileManager copyItemAtPath:path toPath:fileDocFullName    error:nil];
        }
    }
}

static Test_Options g_TestOptions;

Test_Options* Test_OptionsCreate()
{
    nsl_memset(&g_TestOptions, 0, sizeof(g_TestOptions));

    g_TestOptions.logLevel = LogLevelNone;
    g_TestOptions.networkType = TestNetworkTcp;
    g_TestOptions.testLevel = TestLevelIntermediate;
    g_TestOptions.overwriteFlag = TestOverwriteFlagFalse;

    CopyQaLogFileIfNecessary(QALOG_SMALL_FILE);
    CopyQaLogFileIfNecessary(QALOG_MEDIUM_FILE);
    CopyQaLogFileIfNecessary(QALOG_LARGE_FILE);

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
    nsl_strncpy(buffer, "iphone", bufferSize);
    return TRUE;
}

//static nb_boolean StopEventLoop(/*GMainLoop *mainLoop*/ void* user_data)
//{
//    if (mainLoop)
//    {
//        g_main_loop_quit(mainLoop);
//    }
//
//    return FALSE;
//}

static void FlushEventLoop(void)
{
//    GMainLoop *mainLoop = g_main_loop_new(NULL, FALSE);
//
//    g_idle_add((GSourceFunc)StopEventLoop, mainLoop);
//    g_main_loop_run(mainLoop);
//
//    g_main_loop_unref(mainLoop);
}

const char* GetBasePath()
{
    PAL_Error err = PAL_Ok;
    PAL_Instance* dummyPal = (PAL_Instance*)0xAABBCCDD;
    static char basePath[256];

    nsl_memset(basePath, 0, 256);

    NSArray* array = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString* docPath = [array objectAtIndex:0];

    nsl_strcpy(basePath, [docPath UTF8String]);

    err = PAL_FileAppendPath(dummyPal, basePath, 256, TEST_BASE_PATH);
    if (err != PAL_Ok)
    {
        return NULL;
    }

    return basePath;
}

PAL_Error PlatformLoadFile(PAL_Instance* pal, const char* filename, unsigned char** data, uint32* dataSize)
{
    if (data == NULL || dataSize == NULL)
    {
        return PAL_ErrBadParam;
    }

    NSString *filenameString = [NSString stringWithUTF8String:filename];
    NSString *path = [[NSBundle mainBundle] pathForResource:filenameString ofType:@""];
    if (path == nil)
    {
        return PAL_FileLoadFile(pal, filename, data, dataSize);
    }
    else
    {
        return PAL_FileLoadFile(pal, [path cStringUsingEncoding:NSUTF8StringEncoding], data, dataSize);
    }
}

void PerformanceMeasurement_Begin()
{
}

void PerformanceMeasurement_End()
{
}

PAL_Error PerformanceMeasurement_Print(const char* format, ...)
{
    return PAL_Ok;
}

double PerformanceMeasurement_GetElapsedTime(const char* key)
{
    return 0;
}

nb_boolean PerformanceMeasurement_StartTimer(const char* key)
{
    return TRUE;
}
