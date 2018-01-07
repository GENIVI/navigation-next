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

    @file       PlatformUtil.m
    @date     03/31/2011
    @defgroup PLATFORM_UTIL Platform-specific Utility Functions

    Implementation of platform-specific utilities used for system test.

    This file contains the iPhone platform implementation of the platform-
    specific utilities needed for system testing.
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "platformutil.h"
#include "pal.h"
#include "palstdlib.h"
#include "palfile.h"
#include "main.h"

/*! Default MIN value */
#define DEFAULT_MOBILE_IDENTIFIER_NUMBER    999999999999999ULL

/*! Default MDN value */
#define DEFAULT_MOBILE_DIRECTORY_NUMBER     9999990020ULL

#define TEST_BASE_PATH                      "NB_UNITTEST_DATA"

#import <Foundation/Foundation.h>

typedef struct Event{
    nb_boolean value;
    CFRunLoopTimerRef timer_ref;
} Event;

void* CreateCallbackCompletedEvent(void)
{
    Event *event = (Event *)nsl_malloc(sizeof(Event));
    if (event)
    {
        event->value = FALSE;
        event->timer_ref = nil;
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
            CFRunLoopRemoveTimer(CFRunLoopGetCurrent(), timer, kCFRunLoopDefaultMode);
            event->timer_ref = nil;
            CFRunLoopStop(CFRunLoopGetCurrent());
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
    CFRunLoopAddTimer(CFRunLoopGetMain(), timer, kCFRunLoopDefaultMode);
    CFRelease(timer);
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
        CFRunLoopRemoveTimer(CFRunLoopGetCurrent(), event->timer_ref, kCFRunLoopDefaultMode);
        event->timer_ref = nil;
        CFRunLoopStop(CFRunLoopGetCurrent());
    }
}

PAL_Instance* PAL_CreateInstance()
{
    PAL_Config palConfig = { 0 };

    return PAL_Create(&palConfig);
}

uint64 GetMobileDirectoryNumber(PAL_Instance* pal)
{
    return DEFAULT_MOBILE_DIRECTORY_NUMBER;
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
    nsl_strncpy(buffer, "win32", bufferSize);
    return TRUE;
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

/* See header file for description */
char*
GetResourcePath(const char* filename)
{
    NSString* filenameString = [NSString stringWithUTF8String:filename];
    NSString* pathString = [[NSBundle mainBundle] pathForResource:filenameString ofType:@""];
    if (pathString != nil)
    {
        const char* fullPath = [pathString UTF8String];
        if (fullPath)
        {
            return nsl_strdup(fullPath);
        }
    }

    return NULL;
}

/*! @} */
