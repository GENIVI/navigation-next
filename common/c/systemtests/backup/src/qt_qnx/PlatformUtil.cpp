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

    @file     PlatformUtil.cpp
    @date     05/03/2012
    @defgroup PLATFORM_UTIL Platform-specific Utility Functions

    Implementation of platform-specific utilities used for system test.

    This file contains the QNX platform implementation of the platform-
    specific utilities needed for system testing.
    */
    /*
    (C) Copyright 2012 by TeleCommunication, Inc.

    The information contained herein is confidential, proprietary
    to Telecommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Telecommunication Systems is granted only
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
#include <signal.h>
#include "pal.h"
#include "palfile.h"
#include "palstdlib.h"
#include "PlatformUtil.h"
#include "Maintest.h"

#define BUF_SIZE        (1024)
#define TEST_BASE_PATH  ("BASE")

typedef struct Event{
    int value;
    QEventLoop* loop;
} Event;

void* CreateCallbackCompletedEvent(void)
{
    Event *event = (Event *)nsl_malloc(sizeof(Event));
    if (event)
    {
        event->loop = new QEventLoop();
        event->value = 0;
    }
    return event;
}

void DestroyCallbackCompletedEvent(void* callbackCompletedEvent)
{
    Event *event = (Event *) callbackCompletedEvent;
    if (event)
    {
        delete event->loop;
        nsl_free(event);
    }
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

    if (event->loop == NULL)
    {
        event->loop = new QEventLoop();
        if (event->loop == NULL)
        {
            return FALSE;
        }
    }

    if (timeoutMsec)
    {
        QTimer::singleShot(timeoutMsec, event->loop, SLOT(quit()));
    }

    event->loop->exec();

    delete event->loop;
    event->loop = NULL;

    result = event->value;
    event->value = FALSE;   /* reset event */

    return result;
}

void SetCallbackCompletedEvent(void* callbackCompletedEvent)
{
    Event *event = (Event *)callbackCompletedEvent;

    if (event != NULL)
    {
        event->value = 1;
        if (event->loop)	/* wake up waiting event loop */
        {
            event->loop->quit();
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
    nsl_strncpy(buffer, "Blackberry", bufferSize);
    return TRUE;
}

void SigPipeHandler(int signalNum)
{
    printf("Signal %d caught!\n", signalNum);
#if 0
    abort();      // abort to generate core file
#endif
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    const char* appdir = "/app/native";
    // catch SIGPIPE, otherwise the program might be terminated silently.
    //@TODO: other signals?
    signal(SIGPIPE, SigPipeHandler);
    // we can use argv[0] as well to retrieve current directory
    char cwdir[BUF_SIZE] = {0};
    if (!getcwd(cwdir, BUF_SIZE-nsl_strlen(appdir)))
        return -1;
    nsl_strcat(cwdir, appdir);
    if (chdir(cwdir))
        return -1;

    MainTest test(argc, argv);
    QTimer::singleShot(100, &test, SLOT(startMain()));
    app.connect(&test, SIGNAL(finished()), &app, SLOT(quit()));

    return app.exec();
}


const char* GetBasePath()
{
    return TEST_BASE_PATH;
}

/*! @} */

PAL_Error PlatformLoadFile(PAL_Instance* pal, const char* filename, unsigned char** data, uint32* dataSize)
{
    if (data == NULL || dataSize == NULL)
    {
        return PAL_ErrBadParam;
    }

    return PAL_FileLoadFile(pal, filename, data, dataSize);

}

//TODO: These functions will implemented in future

/*! Opens output file and starts main timer (timer with NULL key).
Note: This function internally creates PAL instance. It will do nothing
       if the PAL thread is already started.
*/
void PerformanceMeasurement_Begin()
{

}

/*! Closes output file, makes cleanup
*/
void PerformanceMeasurement_End()
{

}

/*! Prints formatted string to the file
returns PAL_Ok if success
*/
PAL_Error PerformanceMeasurement_Print(const char* format, ...)
{
    return PAL_Ok;
}

/*! Returns elapsed time for selected timer.
    If the key is NULL, then the elapsed time of the main timer will be returned.
returns time in seconds or -1.0 if failed
*/
double PerformanceMeasurement_GetElapsedTime(const char* key)
{
	return 0.0;
}

/*! Starts a new timer with the key.
    If timer with the key is already started, then it will be restarted.
    If the key is NULL, main timer will be restarted.
returns TRUE if success
*/
nb_boolean PerformanceMeasurement_StartTimer(const char* key)
{
	return true;
}
/*! @} */
