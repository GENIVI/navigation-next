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
    @date     02/27/2012
    @defgroup PLATFORM_UTIL Platform-specific Utility Functions

    Implementation of platform-specific utilities used for system test.

    This file contains the QNX platform implementation of the platform-
    specific utilities needed for system testing.
*/
/*
    See file description in header file.

    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include <QCoreApplication>
#include <QTimer>
#include <QEventLoop>
#include <qdebug>

#include <unistd.h>
#include <inttypes.h>
#include "pal.h"
#include "palstdlib.h"
#include "PlatformUtil.h"
#include "mainview.h"

#define BUF_SIZE 1024

typedef struct Event{
    bool value;
    QEventLoop* mainLoop;
} Event;

void* CreateCallbackCompletedEvent(void)
{
    Event *event = (Event *)nsl_malloc(sizeof(Event));
    if (event)
    {
        event->mainLoop = NULL;
        event->value = false;
    }
    return event;
}

void DestroyCallbackCompletedEvent(void* callbackCompletedEvent)
{
    Event *event = (Event *) callbackCompletedEvent;
    if (event)
    {
        if (event->mainLoop)
        {
            delete event->mainLoop;
        }
        nsl_free(event);
    }
}

boolean WaitForCallbackCompletedEvent(void* callbackCompletedEvent,
        uint32 timeoutMsec)
{
    Event *event = (Event *)callbackCompletedEvent;
    boolean result = FALSE;

    if (event == NULL)
    {
        return FALSE;
    }

    if (event->value)   /* event signaled before wait */
    {
        event->value = false; /* reset event */
        return TRUE;
    }

    if (event->mainLoop == NULL)
    {
        event->mainLoop = new QEventLoop();
        if (event->mainLoop == NULL)
        {
            return FALSE;
        }
    }

    if (timeoutMsec)
    {
        QTimer::singleShot(timeoutMsec, event->mainLoop, SLOT(quit()));
    }

    event->mainLoop->exec();

    delete event->mainLoop;
    event->mainLoop = NULL;

    result = event->value;
    event->value = false;   /* reset event */

    return result;
}

void SetCallbackCompletedEvent(void* callbackCompletedEvent)
{
    Event *event = (Event *)callbackCompletedEvent;

    if (event != NULL)
    {
        event->value = true;
        if (event->mainLoop)    /* wake up waiting event loop */
        {
            event->mainLoop->quit();
        }
    }
}

PAL_Instance* PAL_CreateInstance()
{
    PAL_Config palConfig = {0};   // dummy config

    return PAL_Create(&palConfig);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    const char* appdir = "/app";
    char cwdir[BUF_SIZE] = {0};

    if (!getcwd(cwdir, BUF_SIZE-nsl_strlen(appdir)))
        return -1;
    nsl_strcat(cwdir, appdir);
    if (chdir(cwdir))
        return -1;

    MainView test;
    QTimer::singleShot(100, &test, SLOT(startMain()));
    app.connect(&test, SIGNAL(finished()), &app, SLOT(quit()));

    return app.exec();
}

void InitializeHeapCheck()
{
}

void TestHeapCheck(void)
{
}

/*! @} */
