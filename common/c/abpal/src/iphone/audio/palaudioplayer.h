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

 @file     palaudioplayer.h
 @date     09/01/2010
 @defgroup PAL Audio API

 @brief    Platform-independent Audio API

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

#import <CoreAudio/CoreAudioTypes.h>
#import <AudioToolbox/AudioToolbox.h>
#import <Foundation/Foundation.h>
#import "pal.h"
#import "abpalaudio.h"

void PalAudioPlayer_DeriveBufferSize(AudioStreamBasicDescription* ASBDesc,
                                     UInt32                       maxPacketSize,
                                     Float64                      seconds,
                                     UInt32                       *outBufferSize,
                                     UInt32                       *outNumPacketsToRead);

@interface PalAudioPlayer : NSObject {
    ABPAL_AudioFormat              m_AudioFormat;
    AudioQueueRef                  m_Queue;
    BOOL                           m_IsRunning;
    NSMutableArray*                m_AudioItemArray;
    NSInteger                      m_CurrentIndex;

    ABPAL_AudioVolumeLevel         m_audioVolumelevel;
    PAL_Instance*                  m_palInstance;
    ABPAL_AudioPlayerCallback*     m_callback;
    void*                          m_callbackData;
    Float32                        m_volume;
    nb_boolean                     m_useDefaultAudioRoute;
    ABPAL_AudioSession*            m_audioSession;
    ABPAL_AudioSessionType         m_audioSessionType;
}

@property(nonatomic, assign) ABPAL_AudioFormat m_AudioFormat;
@property(nonatomic, assign) PAL_Instance* m_palInstance;
@property(nonatomic, assign) ABPAL_AudioPlayerCallback* m_callback;
@property(nonatomic, assign) void* m_callbackData;
@property(nonatomic, assign) nb_boolean m_useDefaultAudioRoute;
@property(assign) NSThread* m_currentThread;
@property(assign) AudioQueueRef m_tempQueue; // to check whether callback is valid

- (void)setVolume:(ABPAL_AudioVolumeLevel)volume;
- (void)setUseDefaultAudioRoute:(BOOL)use;
- (PAL_Error)addAudioBuffer:(const char*)buffer andSize:(NSInteger)bufferSize;
- (PAL_Error)play;
- (void)stopOnInterruption;
- (void)stop;
- (PAL_Error)setAudioSessionType:(ABPAL_AudioSessionType)audioSessionType;
- (BOOL)isAudioRouteBT;

@end
