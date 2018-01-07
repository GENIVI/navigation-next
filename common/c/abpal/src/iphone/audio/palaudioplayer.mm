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

 @file     palaudioplayer.m
 @date     09/01/2010
 @defgroup PAL Audio API

 @brief    Platform-independent Audio API

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

/*! @{ */

#import <AVFoundation/AVFoundation.h>
#include "palfile.h"
#import "palaudioplayer.h"
#import "playeritem.h"
#import "palaudiosessionsingleton.h"

@interface PalAudioPlayer(private_functions)

- (void) HandleOutputBuffer:(AudioQueueRef)inAQ andBuffer:(AudioQueueBufferRef)inBuffer;
- (void) PropertyChangedInQueue:(AudioQueueRef)inAQ andProperty:(AudioQueuePropertyID)inID;
- (void) DisposeAudioQueue;
- (BOOL) initAudioSession;
- (void) resetAudioSession;

@end

@implementation PalAudioPlayer(private_functions)

- (void) HandleOutputBuffer:(AudioQueueRef)inAQ andBuffer:(AudioQueueBufferRef)inBuffer {
    NSInteger count = [m_AudioItemArray count];
    if (m_CurrentIndex < count) {
        PlayerItem* item = [m_AudioItemArray objectAtIndex:m_CurrentIndex];
        [item prepare:m_Queue];
        ++m_CurrentIndex;
    }
    else {
        AudioQueueStop(m_Queue, false);
    }
}

- (void)audioQueueStopped:(id)sender
{
    if (self.m_tempQueue == m_Queue)
    {
        [self DisposeAudioQueue];
        if (self.m_callback)
        {
            self.m_callback(self.m_callbackData, ABPAL_AudioState_Ended);
        }
    }
    
}

- (void) PropertyChangedInQueue:(AudioQueueRef)inAQ andProperty:(AudioQueuePropertyID)inID
{
    switch (inID)
    {
        case kAudioQueueProperty_IsRunning:
        {
            UInt32 state = 0;
            UInt32 stateSize = sizeof(state);
            AudioQueueGetProperty(inAQ, kAudioQueueProperty_IsRunning, &state, &stateSize);
            if (!state)
            {
                //This callback has to get called from the thread where client uses PAL Audio.
                //This callback will get called asynchronously after the current task has finished
                // no retain/release are needed for objects in block: see block info
                self.m_tempQueue = inAQ;
                [self performSelector:@selector(audioQueueStopped:) onThread:self.m_currentThread withObject:nil waitUntilDone:NO];
            }
            break;
        }
        default:
            break;
    }
}

- (void) DisposeAudioQueue {
    // Clean Up After Playing
    if (m_Queue) {
        // If you pass the second param inImmediate = true, stopping occurs immediately (that is, synchronously).
        AudioQueueStop(m_Queue, true);
        AudioQueueDispose(m_Queue, true);
        m_Queue = NULL;
    }
    [m_AudioItemArray removeAllObjects];

    m_IsRunning = NO;

    [self resetAudioSession];
}

- (BOOL) initAudioSession
{
    [self resetAudioSession];

    PalAudioSessionManager* palAudioSessionManager = PalAudioSessionFactory::getInstance(m_palInstance);
    m_audioSession = (ABPAL_AudioSession*)[palAudioSessionManager OpenAudioSession:m_audioSessionType andPlayer:self];

    return (m_audioSession != nil);
}

- (void) resetAudioSession
{
    if (m_audioSession != NULL)
    {
        ABPAL_AudioSessionDestroy(m_audioSession);
        m_audioSession = NULL;
    }
}

@end


void PalAudioPlayer_QueueStateChanged(void* inUserData, AudioQueueRef inAQ, AudioQueuePropertyID inID) {
    PalAudioPlayer* aqData = (PalAudioPlayer*)inUserData;
    [aqData PropertyChangedInQueue:inAQ andProperty:inID];
}

void PalAudioPlayer_HandleOutputBuffer(void* aqData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer) {
    PalAudioPlayer* pAqData = (PalAudioPlayer*)(aqData);
    [pAqData HandleOutputBuffer:inAQ andBuffer:inBuffer];
}

void PalAudioPlayer_DeriveBufferSize(AudioStreamBasicDescription* ASBDesc,
                                     UInt32                      maxPacketSize,
                                     Float64                     seconds,
                                     UInt32                      *outBufferSize,
                                     UInt32                      *outNumPacketsToRead)
{
    static const int maxBufferSize = 0x50000;
    static const int minBufferSize = 0x4000;

    if (ASBDesc->mFramesPerPacket != 0) {
        Float64 numPacketsForTime = ASBDesc->mSampleRate / ASBDesc->mFramesPerPacket * seconds;
        *outBufferSize = numPacketsForTime * maxPacketSize;
    } else {
        *outBufferSize = (maxBufferSize > maxPacketSize) ? maxBufferSize : maxPacketSize;
    }

    if ((*outBufferSize > maxBufferSize) && (*outBufferSize > maxPacketSize)) {
        *outBufferSize = maxBufferSize;
    }
    else {
        if (*outBufferSize < minBufferSize) {
            *outBufferSize = minBufferSize;
        }
    }

    *outNumPacketsToRead = *outBufferSize / maxPacketSize;
}


@implementation PalAudioPlayer

@synthesize m_AudioFormat;
@synthesize m_palInstance;
@synthesize m_callback;
@synthesize m_callbackData;
@synthesize m_useDefaultAudioRoute;
@synthesize m_currentThread;
@synthesize m_tempQueue;

- (id)init {
    if (self = [super init]) {
        m_palInstance = nil;
        m_volume = 1.0;
        m_audioSession = NULL;
        m_audioSessionType = ABPAL_AudioSession_Play; // Play is default audio session type for player
        m_AudioItemArray = [[NSMutableArray alloc] init];
        if (m_AudioItemArray == nil) {
            [self release];
            self = nil;
        }
    }
    return self;
}

- (void)dealloc {
    [self DisposeAudioQueue];
    [m_AudioItemArray release];
    [super dealloc];
}

- (void)setVolume:(ABPAL_AudioVolumeLevel)volume
{
    switch (volume) {
        case ABPAL_AudioVolumeLevel_Off:{
            m_volume = 0.0;
            break;
        }
        case ABPAL_AudioVolumeLevel_Low:{
            m_volume = 0.25;
            break;
        }
        case ABPAL_AudioVolumeLevel_Medium:{
            m_volume = 0.5;
            break;
        }
        case ABPAL_AudioVolumeLevel_High:{
            m_volume = 0.75;
            break;
        }
        case ABPAL_AudioVolumeLevel_VeryHigh:{
            m_volume = 1.0;
            break;
        }
        default:
            break;
    }

    if (m_IsRunning) {
        AudioQueueSetParameter(m_Queue, kAudioQueueParam_Volume, m_volume);
    }
}

- (void)setUseDefaultAudioRoute:(BOOL)use {
    m_useDefaultAudioRoute = use;
}

- (PAL_Error)addAudioBuffer:(const char*)buffer andSize:(NSInteger)bufferSize {
    PAL_Error result = PAL_Ok;
    do {
        NSInteger count = [m_AudioItemArray count];
        NSArray *array = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
        NSString* path = [array objectAtIndex:0];
        NSString* filename = [path stringByAppendingFormat:@"/Audio_%02ld", (long)count];

        result = PAL_FileExists(self.m_palInstance, [filename UTF8String]);
        if (result == PAL_Ok) {
            result = PAL_FileRemove(self.m_palInstance, [filename UTF8String]);
            if (result != PAL_Ok) {
                break;
            }
        }

        PAL_File* file = NULL;
        result = PAL_FileOpen(self.m_palInstance, [filename UTF8String], PFM_Create, &file);
        if (result != PAL_Ok) {
            break;
        }

        uint32 size = 0;
        result = PAL_FileWrite(file, (const uint8*)buffer, bufferSize, &size);
        if (result != PAL_Ok || size != bufferSize) {
            PAL_FileClose(file);
            break;
        }

        result  = PAL_FileClose(file);
        if (result != PAL_Ok) {
            break;
        }

        PlayerItem* item = [[PlayerItem alloc] init];
        if (item == nil) {
            result = PAL_ErrNoMem;
            break;
        }

        [item setFile:filename];
        [m_AudioItemArray addObject:item];
        [item release];
    } while (false);
    return result;
}

- (PAL_Error)play {
    PAL_Error error = PAL_Ok;
    do {
        // Set audio session to play
        if (![self initAudioSession])
        {
            error = PAL_ErrAudioGeneral;
            break;
        }

        PlayerItem* item = [m_AudioItemArray objectAtIndex:0];
        AudioStreamBasicDescription dataFormat;
        [item getFileFormat:&dataFormat];

        // Create a Playback Audio Queue
        self.m_currentThread =  [NSThread currentThread]; // save client thread
        OSStatus result = AudioQueueNewOutput(&dataFormat, PalAudioPlayer_HandleOutputBuffer, self,
                                              NULL, NULL,
                                              0, &m_Queue);
        if (result != 0) {
            error = PAL_ErrAudioGeneral;
            break;
        }

        m_CurrentIndex = 0;
        result = [item prepare:m_Queue];
        ++m_CurrentIndex;
        if (result != 0) {
            error = PAL_ErrAudioGeneral;
            break;
        }

        // Set an Audio Queue’s Playback Gain
        result = AudioQueueSetParameter(m_Queue, kAudioQueueParam_Volume, m_volume);
        if (result != 0) {
            error = PAL_ErrAudioGeneral;
            break;
        }

        // Add the property linster.
        result = AudioQueueAddPropertyListener(m_Queue, kAudioQueueProperty_IsRunning,
                                               PalAudioPlayer_QueueStateChanged, self);
        if (result != 0) {
            error = PAL_ErrAudioGeneral;
            break;
        }

        result = AudioQueueStart(m_Queue, NULL);
        if (result != 0) {
            error = PAL_ErrAudioGeneral;
            break;
        }

        // Start and Run an Audio Queue
        m_IsRunning = true;
    } while (false);

    if (error != PAL_Ok)
    {
        [self resetAudioSession];
    }

    return error;
}

- (void)stopOnInterruption
{
    if(m_Queue)
    {
        AudioQueueStop(m_Queue, false);
    }
}

- (void)stop {
    [self DisposeAudioQueue];
}

- (PAL_Error)setAudioSessionType:(ABPAL_AudioSessionType)audioSessionType
{
    // Only Play and PlayAndRecord audio sessions are allowed
    switch (audioSessionType)
    {
        case ABPAL_AudioSession_Play:
        case ABPAL_AudioSession_PlayAndRecord:
            break;

        default:
            return PAL_ErrBadParam;
    }

    m_audioSessionType = audioSessionType;
    return PAL_Ok;
}

- (BOOL)isAudioRouteBT
{
    BOOL isBT = NO;

    AVAudioSession* audioSession = [AVAudioSession sharedInstance];
    AVAudioSessionRouteDescription* routeDescription = audioSession.currentRoute;
    if (routeDescription.outputs)
    {
        AVAudioSessionPortDescription* description = routeDescription.outputs[0];
        NSString* portType = description.portType;
        isBT = [portType isEqual:AVAudioSessionPortBluetoothA2DP] ||
                [portType isEqual:AVAudioSessionPortBluetoothLE] ||
                [portType isEqual:AVAudioSessionPortBluetoothHFP];
    }

    return isBT;
}

@end
