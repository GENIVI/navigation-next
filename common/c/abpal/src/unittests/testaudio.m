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

    @defgroup TestAudio_c Unit Tests for PAL Audio

    Unit Tests for PAL Audio

    This file contains all unit tests for the PAL audio component
*/
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret
    as defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly prescribing
    the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "testaudio.h"
#include "main.h"
#include "abpalaudio.h"
#include "platformutil.h"
#include "palfile.h"
#ifdef WINCE
#include "winbase.h"
#endif


// Constants ................................................................

const int PLAY_TIMEOUT_MSEC = 10000;
const int PLAY_CUTOFF_MSEC = 10000;

const int RECORD_TIMEOUT_MSEC = 2500;


// Static helper functions ..................................................

static void
AudioPlayerCallback(void* userData, ABPAL_AudioState state)
{
#ifdef UNIT_TEST_VERBOSE_HIGH
    char* stateString = 0;
    switch (state)
    {
        case ABPAL_AudioState_Init:
            stateString = "ABPAL_AudioState_Init";
            break;
        case ABPAL_AudioState_Playing:
            stateString = "ABPAL_AudioState_Playing";
            break;
        case ABPAL_AudioState_Recording:
            stateString = "ABPAL_AudioState_Recording";
            break;
        case ABPAL_AudioState_Ended:
            stateString = "ABPAL_AudioState_Ended";
            break;
        case ABPAL_AudioState_Error:
            stateString = "ABPAL_AudioState_Error";
            break;
        case ABPAL_AudioState_Stopped:
            stateString = "ABPAL_AudioState_Stopped";
            break;
        case ABPAL_AudioState_Paused:
            stateString = "ABPAL_AudioState_Paused";
            break;
        case ABPAL_AudioState_Cancel:
            stateString = "ABPAL_AudioState_Cancel";
            break;
        case ABPAL_AudioState_Unknown:
        default:
            stateString = "ABPAL_AudioState_Unknown";
            break;
    }

    PRINTF("AudioPlayerCallback: state = %s\n", stateString);
#endif

    if (state == ABPAL_AudioState_Ended)
    {
        SetCallbackCompletedEvent(userData);
    }
}

static void
PlayAudioFile(ABPAL_AudioFormat fileFormat, const char* path)
{
    const char *filePath = [[[NSBundle mainBundle] pathForResource:@"sound" ofType:@"amr"] cStringUsingEncoding:NSUTF8StringEncoding];
    PAL_Error err = PAL_Ok;

    PAL_Instance* pal = 0;
    ABPAL_AudioPlayer* player = 0;

    unsigned char* fileData = 0;
    uint32 fileSize = 0;

    void* callbackCompletedEvent = CreateCallbackCompletedEvent();

    pal = PAL_CreateInstance();
    CU_ASSERT_NOT_EQUAL(pal, 0);

    err = PAL_FileLoadFile(pal, filePath, &fileData, &fileSize);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = ABPAL_AudioPlayerCreate(pal, fileFormat, &player);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = ABPAL_AudioPlayerPlay(player, fileData, fileSize, TRUE, &AudioPlayerCallback, callbackCompletedEvent);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    if (WaitForCallbackCompletedEvent(callbackCompletedEvent, PLAY_TIMEOUT_MSEC))
    {
        CU_ASSERT(TRUE);
    }
    else
    {
        CU_FAIL("PlayAudioFile timed out");
    }

    err = ABPAL_AudioPlayerDestroy(player);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    PAL_Destroy(pal);
    DestroyCallbackCompletedEvent(callbackCompletedEvent);
}

static void
PlayMultipleFilesUsingCombiner(ABPAL_AudioFormat audioFormat, const char* files[], int filesCount)
{
    PAL_Error err = PAL_Ok;

    PAL_Instance* pal = 0;

    ABPAL_AudioCombiner* combiner = 0;
    ABPAL_AudioPlayer* player = 0;

    unsigned char* data = 0;
    int dataSize = 0;

    int index = 0;

    void* callbackCompletedEvent = CreateCallbackCompletedEvent();

    pal = PAL_CreateInstance();
    CU_ASSERT_NOT_EQUAL(pal, 0);

    err = ABPAL_AudioCombinerCreate(pal, audioFormat, &combiner);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    for (index = 0; index < filesCount; index++)
    {
        uint8* fileData = 0;
        uint32 fileSize = 0;

        err = PAL_FileLoadFile(pal, files[index], &fileData, &fileSize);
        CU_ASSERT_EQUAL(err, PAL_Ok);

        err = ABPAL_AudioCombinerAddBuffer(combiner, fileData, fileSize, FALSE);
        CU_ASSERT_EQUAL(err, PAL_Ok);
    }

    err = ABPAL_AudioPlayerCreate(pal, audioFormat, &player);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = ABPAL_AudioCombinerGetData(combiner, &data, &dataSize);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = ABPAL_AudioPlayerPlay(player, data, dataSize, TRUE, AudioPlayerCallback, callbackCompletedEvent);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    if (WaitForCallbackCompletedEvent(callbackCompletedEvent, PLAY_TIMEOUT_MSEC))
    {
        CU_ASSERT(TRUE);
    }
    else
    {
        CU_FAIL("TestAudioPlayerMultipleUsingCombiner timed out");
    }

    err = ABPAL_AudioPlayerDestroy(player);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = ABPAL_AudioCombinerDestroy(combiner);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    PAL_Destroy(pal);
    DestroyCallbackCompletedEvent(callbackCompletedEvent);
}


// Test functions ...........................................................

void
TestAudioPlayerPlayQCP(void)
{
    PlayAudioFile(ABPAL_AudioFormat_QCP, "dest-approaching-right.qcp");
}

void
TestAudioPlayerPlayAMR(void)
{
    PlayAudioFile(ABPAL_AudioFormat_AMR, "make-legal-uturn.amr");
}

void
TestAudioPlayerPlayWAV(void)
{
    PlayAudioFile(ABPAL_AudioFormat_WAV, "tada.wav");
}

void
TestAudioPlayerCancel(void)
{
    const char *path = [[[NSBundle mainBundle] pathForResource:@"sound" ofType:@"amr"] cStringUsingEncoding:NSUTF8StringEncoding];
    PAL_Error err = PAL_Ok;

    PAL_Instance* pal = 0;
    ABPAL_AudioPlayer* player = 0;

    unsigned char* fileData = 0;
    uint32 fileSize = 0;

    void* callbackCompletedEvent = CreateCallbackCompletedEvent();

    pal = PAL_CreateInstance();
    CU_ASSERT_NOT_EQUAL(pal, 0);

    err = PAL_FileLoadFile(pal, path, &fileData, &fileSize);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = ABPAL_AudioPlayerCreate(pal, ABPAL_AudioFormat_AMR, &player);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = ABPAL_AudioPlayerPlay(player, fileData, fileSize, TRUE, &AudioPlayerCallback, callbackCompletedEvent);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    // give play a chance to start... and then cancel
    WaitForCallbackCompletedEvent(callbackCompletedEvent, 20000);

    // should cut "your destination is approaching on the right" off mid-stream
    err = ABPAL_AudioPlayerCancel(player);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    if (WaitForCallbackCompletedEvent(callbackCompletedEvent, PLAY_TIMEOUT_MSEC))
    {
        CU_ASSERT(TRUE);
    }
    else
    {
        CU_FAIL("TestAudioPlayerCancel timed out");
    }

    err = ABPAL_AudioPlayerDestroy(player);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    PAL_Destroy(pal);
    DestroyCallbackCompletedEvent(callbackCompletedEvent);
}

void
TestAudioPlayerVolume(void)
{
    const ABPAL_AudioVolumeLevel levels[] = {
        ABPAL_AudioVolumeLevel_Off,
        ABPAL_AudioVolumeLevel_Low,
        ABPAL_AudioVolumeLevel_Medium,
        ABPAL_AudioVolumeLevel_High,
        ABPAL_AudioVolumeLevel_VeryHigh
    };

    const int levelsCount = (sizeof(levels) / sizeof(levels[0]));

    PAL_Error err = PAL_Ok;

    PAL_Instance* pal = 0;
    ABPAL_AudioPlayer* player = 0;

    unsigned char* fileData = 0;
    uint32 fileSize = 0;

    int index = 0;

    void* callbackCompletedEvent = CreateCallbackCompletedEvent();

    pal = PAL_CreateInstance();
    CU_ASSERT_NOT_EQUAL(pal, 0);

    err = PAL_FileLoadFile(pal, "make-legal-uturn.amr", &fileData, &fileSize);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = ABPAL_AudioPlayerCreate(pal, ABPAL_AudioFormat_AMR, &player);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    for (index = 0; index < levelsCount; index++)
    {
        int32 param1 = 0;
        int32 param2 = 0;

        // set volume parameter
        err = ABPAL_AudioPlayerSetParameter(player, ABPAL_AudioParameter_Volume, levels[index], 0);
        CU_ASSERT_EQUAL(err, PAL_Ok);

        // verify volume parameter got set
        err = ABPAL_AudioPlayerGetParameter(player, ABPAL_AudioParameter_Volume, &param1, &param2);
        CU_ASSERT_EQUAL(err, PAL_Ok);
        CU_ASSERT_EQUAL(param1, (int32)levels[index]);
        CU_ASSERT_EQUAL(param2, 0);

        err = ABPAL_AudioPlayerPlay(player, fileData, fileSize, TRUE, &AudioPlayerCallback, callbackCompletedEvent);
        CU_ASSERT_EQUAL(err, PAL_Ok);

        if (WaitForCallbackCompletedEvent(callbackCompletedEvent, PLAY_TIMEOUT_MSEC))
        {
            CU_ASSERT(TRUE);
        }
        else
        {
            CU_FAIL("TestAudioPlayerVolume timed out");
        }
    }

    err = ABPAL_AudioPlayerDestroy(player);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    PAL_Destroy(pal);
    DestroyCallbackCompletedEvent(callbackCompletedEvent);
}


void
TestAudioPlayerMultipleCutoff(void)
{
    const char* files[] = {
        "cross-bridge-right.amr",
        "escalator-left.amr",
        "make-legal-uturn.amr",
    };

    const int filesCount = (sizeof(files) / sizeof(files[0]));

    PAL_Error err = PAL_Ok;

    PAL_Instance* pal = 0;
    ABPAL_AudioPlayer* player = 0;

    int index = 0;

    void* callbackCompletedEvent = CreateCallbackCompletedEvent();

    pal = PAL_CreateInstance();
    CU_ASSERT_NOT_EQUAL(pal, 0);

    err = ABPAL_AudioPlayerCreate(pal, ABPAL_AudioFormat_AMR, &player);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    for (index = 0; index < filesCount; index++)
    {
        unsigned char* fileData = 0;
        uint32 fileSize = 0;

        err = PAL_FileLoadFile(pal, files[index], &fileData, &fileSize);
        CU_ASSERT_EQUAL(err, PAL_Ok);

        err = ABPAL_AudioPlayerPlay(player, fileData, fileSize, TRUE, 0, 0);
        CU_ASSERT_EQUAL(err, PAL_Ok);

        // wait a bit and then try to play the next file without waiting for notification
        // that the previous completed; should stop playback and begin playing the next
        (void)WaitForCallbackCompletedEvent(callbackCompletedEvent, PLAY_CUTOFF_MSEC);
    }

    err = ABPAL_AudioPlayerDestroy(player);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    PAL_Destroy(pal);
    DestroyCallbackCompletedEvent(callbackCompletedEvent);
}


void
TestAudioPlayerMultipleUsingCombinerQCP(void)
{
    const char* files[] = {
        "dest-approaching-right.qcp",
        "enter-tunnel-left.qcp",
        "fourteen.qcp",
    };

    const int filesCount = (sizeof(files) / sizeof(files[0]));

    PlayMultipleFilesUsingCombiner(ABPAL_AudioFormat_QCP, files, filesCount);
}


void
TestAudioPlayerMultipleUsingCombinerAMR(void)
{
    const char* files[] = {
        "cross-bridge-right.amr",
        "escalator-left.amr",
        "make-legal-uturn.amr",
    };

    const int filesCount = (sizeof(files) / sizeof(files[0]));

    PlayMultipleFilesUsingCombiner(ABPAL_AudioFormat_AMR, files, filesCount);
}


void
TestAudioPlayerPlayThruBluetooth(void)
{
    PAL_Error err = PAL_Ok;

    PAL_Instance* pal = 0;
    ABPAL_AudioPlayer* player = 0;

    unsigned char* fileData = 0;
    uint32 fileSize = 0;

    void* callbackCompletedEvent = CreateCallbackCompletedEvent();

    pal = PAL_CreateInstance();
    CU_ASSERT_NOT_EQUAL(pal, 0);

    err = PAL_FileLoadFile(pal, "make-legal-uturn.amr", &fileData, &fileSize);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = ABPAL_AudioPlayerCreate(pal, ABPAL_AudioFormat_AMR, &player);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    // turn Bluetooth on... if device is present, next play will be through Bluetooth audio device
    err = ABPAL_AudioPlayerSetParameter(player, ABPAL_AudioParameter_BT, TRUE, 0);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    // set volume parameter
    err = ABPAL_AudioPlayerSetParameter(player, ABPAL_AudioParameter_Volume, 15, 0);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = ABPAL_AudioPlayerPlay(player, fileData, fileSize, FALSE, &AudioPlayerCallback, callbackCompletedEvent);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    if (WaitForCallbackCompletedEvent(callbackCompletedEvent, PLAY_TIMEOUT_MSEC))
    {
        CU_ASSERT(TRUE);
    }
    else
    {
        CU_FAIL("PlayAudioFile to Bluetooth timed out");
    }

    DestroyCallbackCompletedEvent(callbackCompletedEvent);
    callbackCompletedEvent = CreateCallbackCompletedEvent();

    // turn Bluetooth off... if device is present, next play will from speaker
    err = ABPAL_AudioPlayerSetParameter(player, ABPAL_AudioParameter_BT, FALSE, 0);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = ABPAL_AudioPlayerPlay(player, fileData, fileSize, TRUE, &AudioPlayerCallback, callbackCompletedEvent);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    if (WaitForCallbackCompletedEvent(callbackCompletedEvent, PLAY_TIMEOUT_MSEC))
    {
        CU_ASSERT(TRUE);
    }
    else
    {
        CU_FAIL("PlayAudioFile to speaker after Bluetooth timed out");
    }

    err = ABPAL_AudioPlayerDestroy(player);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    PAL_Destroy(pal);
    DestroyCallbackCompletedEvent(callbackCompletedEvent);
}

void
TestAudioRecorderRecord(void)
{
    PAL_Error err = PAL_Ok;

    PAL_Instance* pal = 0;
    ABPAL_AudioRecorder* recorder = 0;

    void* callbackCompletedEvent = CreateCallbackCompletedEvent();

    pal = PAL_CreateInstance();
    CU_ASSERT_NOT_EQUAL(pal, 0);

    err = ABPAL_AudioRecorderCreate(pal, ABPAL_AudioFormat_QCP, &recorder);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    err = ABPAL_AudioRecorderRecord(recorder, NULL, 0, &AudioPlayerCallback, callbackCompletedEvent);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    WaitForCallbackCompletedEvent(callbackCompletedEvent, 25000);

    err = ABPAL_AudioRecorderStop(recorder);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    WaitForCallbackCompletedEvent(callbackCompletedEvent, 25000);

    err = ABPAL_AudioRecorderDestroy(recorder);
    CU_ASSERT_EQUAL(err, PAL_Ok);

    PAL_Destroy(pal);
    DestroyCallbackCompletedEvent(callbackCompletedEvent);
}


/*! Add all your test functions here
@return None
*/
void
TestAudio_AddAllTests(CU_pSuite pTestSuite)
{
    CU_add_test(pTestSuite, "TestAudioPlayerPlayQCP", &TestAudioPlayerPlayQCP);
    CU_add_test(pTestSuite, "TestAudioPlayerPlayAMR", &TestAudioPlayerPlayAMR);
    CU_add_test(pTestSuite, "TestAudioPlayerPlayWAV", &TestAudioPlayerPlayWAV);
    CU_add_test(pTestSuite, "TestAudioPlayerCancel", &TestAudioPlayerCancel);
    CU_add_test(pTestSuite, "TestAudioPlayerVolume", &TestAudioPlayerVolume);
    CU_add_test(pTestSuite, "TestAudioPlayerMultipleCutoff", &TestAudioPlayerMultipleCutoff);
    CU_add_test(pTestSuite, "TestAudioPlayerMultipleUsingCombinerQCP", &TestAudioPlayerMultipleUsingCombinerQCP);
    CU_add_test(pTestSuite, "TestAudioPlayerMultipleUsingCombinerAMR", &TestAudioPlayerMultipleUsingCombinerAMR);
    CU_add_test(pTestSuite, "TestAudioPlayerPlayThruBluetooth", &TestAudioPlayerPlayThruBluetooth);
    CU_add_test(pTestSuite, "TestAudioRecorderRecord", &TestAudioRecorderRecord);
}

/*! Add common initialization code here.

@return 0

@see TestAudio_SuiteCleanup
*/
int

TestAudio_SuiteSetup()
{
    return 0;
}


/*! Add common cleanup code here.

@return 0

@see TestAudio_SuiteSetup
*/
int
TestAudio_SuiteCleanup()
{
    return 0;
}

/*! @} */
