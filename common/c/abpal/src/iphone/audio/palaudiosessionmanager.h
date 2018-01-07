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

 @file     palaudiosessionmanager.h
 @date     05/24/2011
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

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import "abpalaudio.h"
#import "pallock.h"
#import "palaudioplayer.h"
#import "palqueueaudiorecorder.h"

@interface PalAudioSession : NSObject
{
    ABPAL_AudioSessionType  sessionType;
    PalAudioPlayer*         audioPlayer;               /*!< Optional Audio player. */
    PalQueueAudioRecorder*  audioRecorder;             /*!< Optional Audio recorder. */
    NSThread*               audioSessionThreadOwner;   /*!< Optional Audio session thread owner. */
}

@property(nonatomic, readonly, getter=getSessionType) ABPAL_AudioSessionType sessionType;
@property(nonatomic, readonly, assign) PalAudioPlayer* audioPlayer;
@property(nonatomic, readonly, assign) PalQueueAudioRecorder* audioRecorder;
@property(nonatomic, readonly, assign) NSThread* audioSessionThreadOwner;

- (id) initWithSessionType:(ABPAL_AudioSessionType) _sessionType andPlayer:(PalAudioPlayer*)_player andRecorder:(PalQueueAudioRecorder*)_recorder;
- (void) activate;
- (void) deactivate;

@end


@interface PalAudioSessionManager : NSObject <AVAudioSessionDelegate>
{
    NSMutableArray*  sessionStack;
    PAL_Lock*        lock;
}

- (id) initWithPal:(PAL_Instance*) pal;
- (PalAudioSession*) OpenAudioSession:(ABPAL_AudioSessionType) audioSessionType andPlayer:(PalAudioPlayer*)player;
- (PalAudioSession*) OpenAudioSession:(ABPAL_AudioSessionType) audioSessionType andRecorder:(PalQueueAudioRecorder*)recorder;
- (void) CloseAudioSession:(PalAudioSession*) audioSession;

@end
