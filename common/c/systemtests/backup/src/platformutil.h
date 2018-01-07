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

    @file     PlatformUtil.h
    @date     02/06/2009
    @defgroup PLATFORM_UTIL Platform-specific Utility Functions

    Function declarations for platform-specific utilities used for system test.

    This file contains the declaration for the platform-specific utilities
    needed for system testing.
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

#ifndef _PLATFORMUTIL_H_
#define _PLATFORMUTIL_H_

#include "paltypes.h"
#include "pal.h"
#include "main.h"
#include "utility.h"

#ifdef __BREW__
#define SKIP_NAV_FILE_GPS
#define PRINTF DBGPRINTF
#else
#if defined(WIN32) && !defined(WINCE)
#define PRINTF OutputMessage

#ifdef __cplusplus
extern "C"
	{
#endif
void OutputMessage(const char* format, ...);
#ifdef __cplusplus
    }
#endif

#else
#define PRINTF printf
#endif
#define SPRINTF sprintf
#endif

#define MAX_LANGUAGE_LENGTH     10
#define MAX_VOICE_STYLE_LENGTH  25
#define MAX_TOKEN_LENGTH        50
#define MAX_TPSLIB_NAME_LENGTH  64
#define MAX_HOSTNAME_LENGTH     64
#define MAX_LOGFILENAME_LENGTH  64
#define MAX_TEST_NAME 1024

#define GUIDANCE_CONFIG_TPL_DATA "guidanceconfigdata.tpl"
#define IMAGES_TPS               "images.tps"
#define VOICES_TPS               "voices-%s.tps"
#define HOVMAPPING_TPS           "hovmapping.tps"
#define DISTANCE_TPS             "distance.tps"
#define TTF_MAPPING_TPS          "ttfmapping.tps"
#define BASIC_AUDIO_TPL_DATA     "basicaudiodata.tpl"
#define BASIC_AUDIO_TPS          "basicaudio.tps"
#define DIRECTIONS_TPS           "directions.tps"

#ifdef __cplusplus
extern "C"
	{
#endif

typedef struct
{
    Test_LogLevel logLevel;
    Test_TestLevel testLevel;
    Test_NetworkType networkType;
    Test_NetworkDomain domain;
    Test_Carrier carrier;
    Test_OverwriteFlag overwriteFlag;
    char token[MAX_TOKEN_LENGTH + 1];
    char tpslib[MAX_TPSLIB_NAME_LENGTH + 1];
    char hostname[MAX_HOSTNAME_LENGTH + 1];
	char logfilename[MAX_LOGFILENAME_LENGTH + 1];
    uint64 suites;
    nb_boolean verboseQaLog;
    nb_boolean listSuitesAndExit;
    nb_boolean listTestsAndExit;
    nb_boolean startFromParticularTest;
    int numStart;
    int numStep;
    char testName[MAX_TEST_NAME+1];
    char currentLanguage[MAX_LANGUAGE_LENGTH + 1];
    char voiceStyle[MAX_VOICE_STYLE_LENGTH + 1];
    Test_OnBoard onBoard;
    char mapConfig[MAX_TEST_NAME+1];
} Test_Options;


/*! Yields control to the platform OS to pump and process any pending messages.

    This function allows platform-independent test code to temporarily yield control
    to the OS to process any pending queued messages.  This function is called
    from within WaitForCallbackCompletedEvent to allow messages to be processed
    while waiting for a callback to be invoked, but should also be called when
    ending a test session so that any pending closing messages can be processed.
*/
void ProcessPendingMessages(void);

/*! Creates platform-specific event object. 

    This function allows platform-independent test code to create an event object
    that can be used to signal that a callback has been invoked with a response
    from a submitted request.

    @return Generic pointer to an event object

    @see WaitForCallbackCompletedEvent
    @see SetCallbackCompletedEvent
    @see DestroyCallbackCompletedEvent
*/
void* CreateCallbackCompletedEvent(void);

/*! Waits for the specified event object to be signalled, while allowing pending
    queued messages to be processed.

    This function waits for a given event object to be signalled, while periodically
    allowing queued messages to be processed.  Will return if the callback event
    has not been signalled within the given amount of time.

    @return TRUE indicating event was signalled, or FALSE indicating timeout or error

    @see CreateCallbackCompletedEvent
    @see SetCallbackCompletedEvent
    @see DestroyCallbackCompletedEvent
*/
nb_boolean WaitForCallbackCompletedEvent(
    void* callbackCompletedEvent,   /*!< event to wait on */
    uint32 timeoutMsec              /*!< maximum msec to wait for event to be signalled */
    );

/*! Signals given event object.

    This function allows platform-independent test code to signal the given event
    object.

    @see CreateCallbackCompletedEvent
    @see WaitForCallbackCompletedEvent
    @see DestroyCallbackCompletedEvent
*/
void SetCallbackCompletedEvent(
    void* callbackCompletedEvent    /*!< event to signal */
    );

/*! Resets given event object.

    This function allows platform-independent test code to reset the given event
    object.

    @see CreateCallbackCompletedEvent
    @see WaitForCallbackCompletedEvent
    @see DestroyCallbackCompletedEvent
*/
void ResetCallbackCompletedEvent(
    void* callbackCompletedEvent
    );


/*! Destroys the given event object.

    This function allows platform-independent test code to destroy the given event
    object.

    @see CreateCallbackCompletedEvent
*/
void DestroyCallbackCompletedEvent(
    void* callbackCompletedEvent    /*!< event to destroy */
    );

void InitializeHeapCheck();
void TestHeapCheck(void);

PAL_Instance* PAL_CreateInstance();
PAL_Instance* PAL_CreateInstanceMultiThread();
void PAL_DestroyInstance(PAL_Instance* pal);

/*! Returns the global variable of Test_Options struct

This function initializes the data members of the Test_Options struct

@see Test_OptionsGet()
*/
Test_Options* Test_OptionsCreate();

/*! Returns the address of global variable of Test_Options struct

This function returns the address of variable of Test_Options struct.

@see Test_OptionsCreate()
*/
Test_Options* Test_OptionsGet();

/*! Destroys the object.

This method is used to destroy the Test_Options object.

@see Test_OptionsCreate()
@see Test_OptionsGet()
*/
void Test_OptionsDestroy();


/*! Get MDN for the device */
uint64 GetMobileDirectoryNumber(PAL_Instance* pal);


/*! Get MIN for the device */
uint64 GetMobileInformationNumber(PAL_Instance* pal);

/*! See if tests are running in emulator */
nb_boolean RunningInEmulator();

/*! Get the device name

The device name is either a specific name assigned by NIM (e.g. sch.u750)
or a name retrieved from the device itself
*/
nb_boolean GetDeviceName(PAL_Instance* pal, char* buffer, nb_size bufferSize);

const char* GetBasePath();

PAL_Error PlatformLoadFile(PAL_Instance* pal, const char* filename, unsigned char** data, uint32* dataSize);

/*! Opens output file and starts main timer (timer with NULL key).
Note: This function internally creates PAL instance. It will do nothing
       if the PAL thread is already started.
*/
void PerformanceMeasurement_Begin();

/*! Closes output file, makes cleanup
*/
void PerformanceMeasurement_End();

/*! Prints formatted string to the file
returns PAL_Ok if success
*/
PAL_Error PerformanceMeasurement_Print(const char* format, ...);

/*! Returns elapsed time for selected timer.
    If the key is NULL, then the elapsed time of the main timer will be returned.
returns time in seconds or -1.0 if failed
*/
double PerformanceMeasurement_GetElapsedTime(const char* key);

/*! Starts a new timer with the key.
    If timer with the key is already started, then it will be restarted.
    If the key is NULL, main timer will be restarted.
returns TRUE if success
*/
nb_boolean PerformanceMeasurement_StartTimer(const char* key);

#ifdef __cplusplus
	}
#endif

#endif

/*! @} */
