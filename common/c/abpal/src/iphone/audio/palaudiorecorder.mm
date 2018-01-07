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

 @file     palaudiorecorder.m
 @date     09/01/2010
 @defgroup PAL Audio API

 @brief    Platform-independent audio API

 */
/*
    See file description in header file.

    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret
    as defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly prescribing
    the scope and manner of such use.

---------------------------------------------------------------------------*/
#import <CoreAudio/CoreAudioTypes.h>
#import <AudioToolbox/AudioToolbox.h>
#import <MediaPlayer/MPMusicPlayerController.h>
#import "palaudiorecorder.h"
#import "palfile.h"
#import "palaudiosessionsingleton.h"

#define kRecordFileHeaderSize 4096

@interface PalAudioRecorder(private_functions)

- (BOOL)initAudioSession;
- (void)resetAudioSession;
- (BOOL)initRecorder;
- (void)recordFinished;
- (void)recordErrored;
- (void)getDataFromFile;
- (PAL_Error)doPrepareToRecord:(BOOL)paused;

@end

@implementation PalAudioRecorder

- (void)dealloc
{
    if (recorderBuffer)
    {
        nsl_free(recorderBuffer);
    }

    [recordFileName release];
    [recorder release];
    [self resetAudioSession];
    [super dealloc];
}

- (id)initWithPal:(PAL_Instance*)pal audioFormat:(ABPAL_AudioFormat)format
{
    if (self = [super init])
    {
        palInstance = pal;
        recorderFormat = format;
        m_audioSessionType = ABPAL_AudioSession_Record; // Record is default audio session type for recorder
        recorderCallBack = nil;
        userData = nil;
        recorder = nil;
        recorderState = ABPAL_AudioState_Unknown;
        recorderBuffer = nil;
        recorderbufferSize = 0;
        recordFileName = nil;
        m_audioSession = NULL;
    }

    return self;
}

- (PAL_Error)startRecord:(ABPAL_AudioRecorderCallback*)callback userData:(void*)data
{
    // Save callback function and data
    recorderCallBack = callback;
    userData = data;

    PAL_Error err = [self doPrepareToRecord:NO];
    return err;
}

- (PAL_Error) prepareToRecord
{
    PAL_Error err = [self doPrepareToRecord:YES];
    return err;
}

- (PAL_Error)setAudioSessionType:(ABPAL_AudioSessionType)audioSessionType
{
    // Only Record and PlayAndRecord audio sessions are allowed
    switch (audioSessionType)
    {
        case ABPAL_AudioSession_Record:
        case ABPAL_AudioSession_PlayAndRecord:
            break;

        default:
            return PAL_ErrBadParam;
    }

    m_audioSessionType = audioSessionType;
    return PAL_Ok;
}

- (PAL_Error)stop
{
    // Only stop recording for Recording/ReadyToRecord states
    if (recorderState != ABPAL_AudioState_Recording &&
        recorderState != ABPAL_AudioState_ReadyToRecord)
    {
        return PAL_Ok;
    }

    recorderState = ABPAL_AudioState_Stopped;

    if (recorder && recorder.recording)
    {
        [recorder stop];
    }
    else
    {
        // When quickly press microphone button and release it very quickly,
        // recorder will not begin recording, need reset audio session then.
        [self resetAudioSession];
    }

    return PAL_Ok;
}

- (PAL_Error)getRecordedBuffer:(byte**)buffer size:(uint32*)bufferSize
{
    if (recorderBuffer != NULL)
    {
        // Caller will free the recorder buffer
        *buffer = recorderBuffer;
        *bufferSize = recorderbufferSize;

        recorderBuffer = NULL;
        recorderbufferSize = 0;

        return PAL_Ok;
    }

    return PAL_ErrNoData;
}

- (ABPAL_AudioState)state
{
    return recorderState;
}

#pragma mark -
#pragma mark AVAudioRecorderDelegate
- (void)audioRecorderDidFinishRecording:(AVAudioRecorder *)recorder successfully:(BOOL)flag
{
    if (flag)
    {
        [self recordFinished];
    }
    else
    {
        [self recordErrored];
    }
}

- (void)audioRecorderEncodeErrorDidOccur:(AVAudioRecorder *)recorder error:(NSError *)error
{
    [self recordErrored];
}

- (void)audioRecorderBeginInterruption:(AVAudioRecorder *)recorder
{
    [self recordErrored];
}

- (void)audioRecorderEndInterruption:(AVAudioRecorder *)recorder
{
}

- (void)audioRecorderEndInterruption:(AVAudioRecorder *)recorder withFlags:(NSUInteger)flags
{
}

@end

// Private functions implement
@implementation PalAudioRecorder(private_functions)

- (BOOL)initAudioSession
{
    [self resetAudioSession];

    PalAudioSessionManager* palAudioSessionManager = PalAudioSessionFactory::getInstance(palInstance);
    m_audioSession = (ABPAL_AudioSession*)[palAudioSessionManager OpenAudioSession:m_audioSessionType andPlayer:nil];

    return (m_audioSession != nil);
}

- (void)resetAudioSession
{
    if (m_audioSession != NULL)
    {
        ABPAL_AudioSessionDestroy(m_audioSession);
        m_audioSession = NULL;
    }
}

- (BOOL)initRecorder
{
    if (!recorder)
    {
        NSString* filePath = NSTemporaryDirectory();
        recordFileName = [[filePath stringByAppendingString:@"tempsound.dat"] retain];

        NSURL* url = [NSURL URLWithString:recordFileName];

        NSDictionary* settings = [[NSMutableDictionary alloc] init];
        [settings setValue:[NSNumber numberWithInt:kAudioFormatULaw] forKey:AVFormatIDKey];
        [settings setValue:[NSNumber numberWithFloat:8000.0] forKey:AVSampleRateKey];
        [settings setValue:[NSNumber numberWithFloat:8] forKey:AVLinearPCMBitDepthKey];
        [settings setValue:[NSNumber numberWithFloat:1] forKey:AVNumberOfChannelsKey];

        recorder = [[AVAudioRecorder alloc] initWithURL:url settings:settings error:nil];
        [recorder setDelegate:self];

        [settings release];
    }
    return (recorder != nil);
}

- (void)recordFinished
{
    // Do nothing if recording state is ABPAL_AudioState_Ended or ABPAL_AudioState_Error already
    if (recorderState == ABPAL_AudioState_Ended || recorderState == ABPAL_AudioState_Error)
    {
        return;
    }

    // Reset audio session after recording
    [self resetAudioSession];

    // Get recorded buffer and remove recording file
    [self getDataFromFile];
    [[NSFileManager defaultManager] removeItemAtPath:recordFileName error:nil];

    // Notify ended or error state
    if (recorderBuffer && recorderbufferSize > 0)
    {
        recorderState = ABPAL_AudioState_Ended;
    }
    else
    {
        recorderState = ABPAL_AudioState_Error;
    }

    //Guard against NULL
    if (recorderCallBack)
    {
        recorderCallBack(userData, recorderState);
    }
}

- (void)recordErrored
{
    // Do nothing if recording state is ABPAL_AudioState_Error already
    if (recorderState == ABPAL_AudioState_Error)
    {
        return;
    }

    // Reset audio session after recording
    [self resetAudioSession];

    // Remove recording file
    [[NSFileManager defaultManager] removeItemAtPath:recordFileName error:nil];

    // Notify error state
    recorderState = ABPAL_AudioState_Error;

    //Guard against NULL
    if (recorderCallBack)
    {
        recorderCallBack(userData, recorderState);
    }
}

- (void)getDataFromFile
{
    // Free old recorded buffer
    if (recorderBuffer != NULL)
    {
        nsl_free(recorderBuffer);
        recorderBuffer = NULL;
        recorderbufferSize = 0;
    }

    // Get recorder buffer
    byte* buffer = NULL;
    uint32 bufferSize = 0;
    if (PAL_FileLoadFile(palInstance, [recordFileName UTF8String], &buffer, &bufferSize) == PAL_Ok)
    {
        // ASR server require remove 4kb header for RAW format
        if (recorderFormat == ABPAL_AudioFormat_RAW)
        {
            byte* data = (byte*)nsl_malloc(bufferSize - kRecordFileHeaderSize);
            if (data)
            {
                recorderbufferSize = bufferSize - kRecordFileHeaderSize;
                nsl_memcpy(data, buffer + kRecordFileHeaderSize, recorderbufferSize);
                recorderBuffer = data;
            }
            nsl_free(buffer);
        }
        else
        {
            recorderBuffer = buffer;
            recorderbufferSize = bufferSize;
        }
    }
}

- (PAL_Error)doPrepareToRecord:(BOOL)paused
{
    // Return busy if current recording is in progress
    if (recorder && recorder.recording)
    {
        return PAL_ErrAudioBusy;
    }

    if (recorderState == ABPAL_AudioState_ReadyToRecord)
    {
        PAL_Error err = PAL_ErrAudioGeneral;

        if (!paused)
        {
            // Start recording if in ready to record state
            if ([recorder record])
            {
                err = PAL_Ok;
                recorderState = ABPAL_AudioState_Recording;
            }
        }

        return err;
    }

    // Only support raw/wav audio recorder format for iPhone project
    switch (recorderFormat)
    {
        case ABPAL_AudioFormat_CMX:
        case ABPAL_AudioFormat_AU:
        case ABPAL_AudioFormat_GSM:
        case ABPAL_AudioFormat_AMR:
        case ABPAL_AudioFormat_QCP:
        case ABPAL_AudioFormat_AAC:
        case ABPAL_AudioFormat_MP3:
        case ABPAL_AudioFormat_OGG:
            return PAL_ErrUnsupported;
        case ABPAL_AudioFormat_RAW:
        case ABPAL_AudioFormat_WAV:
        default:
            break;
    };

    // Recording steps: Init->InitAudioSession->Prepare->Record(Optional)
    PAL_Error err = PAL_ErrAudioGeneral;
    recorderState = ABPAL_AudioState_Error;

    if ([self initRecorder])
    {
        if (![self initAudioSession])
        {
            return PAL_ErrAudioGeneral;
        }

        if ([recorder prepareToRecord])
        {
            if (paused)
            {
                err = PAL_Ok;
                recorderState = ABPAL_AudioState_ReadyToRecord;
            }
            else
            {
                if ([recorder record])
                {
                    err = PAL_Ok;
                    recorderState = ABPAL_AudioState_Recording;
                }
            }
        }
    }

    if (err != PAL_Ok)
    {
        [self resetAudioSession];
    }

    return err;
}

@end

