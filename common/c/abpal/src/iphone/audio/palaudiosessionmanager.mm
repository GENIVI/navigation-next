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

 @file     palaudiosessionmanager.m
 @date     05/24/2011
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

#import <AudioToolbox/AudioToolbox.h>
#import "palaudiosessionmanager.h"
#import "palaudioplayer.h"
#import "palaudiosessionsingleton.h"

// Put static object at .mm file to avoid import head file more than once cause error
PalAudioSessionManager* PalAudioSessionFactory::m_instance = NULL;

@implementation PalAudioSession

@synthesize sessionType;
@synthesize audioPlayer;
@synthesize audioRecorder;
@synthesize audioSessionThreadOwner;

- (id) initWithSessionType:(ABPAL_AudioSessionType) _sessionType andPlayer:(PalAudioPlayer*)_player andRecorder:(PalQueueAudioRecorder*)_recorder
{
    if (self = [super init])
    {
        sessionType = _sessionType;
        audioPlayer = _player;
        audioRecorder = _recorder;
        audioSessionThreadOwner = [NSThread currentThread];
    }

    return self;
}

- (void) activate
{
    switch (sessionType)
    {
        case ABPAL_AudioSession_Silent:
        {
            AVAudioSession* audioSession = [AVAudioSession sharedInstance];
            [audioSession setCategory:AVAudioSessionCategoryPlayback
                          withOptions:AVAudioSessionCategoryOptionMixWithOthers
                                error:nil];
            [audioSession setActive:YES error:nil];
        }
        break;

        case ABPAL_AudioSession_Play:
        {
            AVAudioSession* audioSession = [AVAudioSession sharedInstance];
            if (audioPlayer.m_useDefaultAudioRoute == 1)
            {
                if (audioSession.isOtherAudioPlaying &&
                    audioSession.categoryOptions != AVAudioSessionCategoryOptionDuckOthers)
                {
                    [audioSession setActive:FALSE error:nil];
                }
                [audioSession setCategory:AVAudioSessionCategoryPlayback
                              withOptions:AVAudioSessionCategoryOptionDuckOthers
                                    error:nil];
            }
            else
            {
                [audioSession setCategory:AVAudioSessionCategoryPlayAndRecord
                                    error:nil];
                [audioSession overrideOutputAudioPort:AVAudioSessionPortOverrideSpeaker error:nil];
            }
            [audioSession setActive:YES error:nil];
        }
        break;

        case ABPAL_AudioSession_Record:
        {
            AVAudioSession* audioSession = [AVAudioSession sharedInstance];
            [audioSession setCategory:AVAudioSessionCategoryRecord
                          withOptions:AVAudioSessionCategoryOptionAllowBluetooth
                                error:nil];
            [audioSession setActive:YES error:nil];
        }
        break;

        case ABPAL_AudioSession_PlayAndRecord:
        {
            AVAudioSession* audioSession = [AVAudioSession sharedInstance];
            // Redirect playing to speaker when BT handset is unplugged.
            // By default for AVAudioSessionCategoryPlayAndRecord sounds goes to receiver.
            [audioSession setCategory:AVAudioSessionCategoryPlayAndRecord
                          withOptions:AVAudioSessionCategoryOptionAllowBluetooth | AVAudioSessionCategoryOptionDefaultToSpeaker
                                error:nil];
            [audioSession setActive:YES error:nil];
        }
        break;

        default:
        break;
    }
}

- (void) deactivate
{
    AVAudioSession* audioSession = [AVAudioSession sharedInstance];
    [audioSession setActive:NO
                withOptions:AVAudioSessionSetActiveOptionNotifyOthersOnDeactivation
                      error:nil];
}

@end

@interface PalAudioSessionManager(private_functions)

- (PalAudioSession*) OpenAudioSessionImp:(ABPAL_AudioSessionType) audioSessionType andPlayer:(PalAudioPlayer*)player andRecorder:(PalQueueAudioRecorder*)recorder;
- (void) CloseAudioSessionImp:(PalAudioSession*) audioSession;
- (BOOL) IsEmpty;

- (void) audioInterrupted:(NSNotification*)notification;
- (void) beginInterruption;
- (void) endInterruption;
@end

@implementation PalAudioSessionManager

- (id) initWithPal:(PAL_Instance*) pal
{
    if (self = [super init])
    {
        sessionStack = [[NSMutableArray alloc] init];
        lock = NULL;

        if (pal != NULL)
        {
            PAL_LockCreate(pal, &lock);
        }

        if (sessionStack == nil || lock == NULL)
        {
            [self release];
            self = nil;
        }
    }
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(audioInterrupted:)
                                                 name: AVAudioSessionInterruptionNotification
                                               object:[AVAudioSession sharedInstance]];
    return self;
}

- (void) dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:AVAudioSessionInterruptionNotification
                                                  object:[AVAudioSession sharedInstance]];
    if (sessionStack && ![self IsEmpty])
    {
        [[sessionStack lastObject] deactivate];
        [sessionStack removeAllObjects];
    }

    [sessionStack release];

    if (lock != NULL)
    {
        PAL_LockDestroy(lock);
        lock = NULL;
    }

    [super dealloc];
}

- (PalAudioSession*) OpenAudioSessionImp:(ABPAL_AudioSessionType) audioSessionType andPlayer:(PalAudioPlayer*)player andRecorder:(PalQueueAudioRecorder*)recorder
{
    // Session object to be added to stack
    PalAudioSession* audioSession = [[[PalAudioSession alloc] initWithSessionType:audioSessionType andPlayer:player andRecorder:recorder] autorelease];

    if (![self IsEmpty])
    {
        PalAudioSession* lastSession = [sessionStack lastObject];
        ABPAL_AudioSessionType lastSessionType = lastSession.sessionType;

        if ((lastSessionType == ABPAL_AudioSession_Record && audioSessionType == ABPAL_AudioSession_Play) ||
            (lastSessionType == ABPAL_AudioSession_PlayAndRecord && (audioSessionType == ABPAL_AudioSession_Play || audioSessionType == ABPAL_AudioSession_Record)))
        {
            // Forbit playing when recording
            return nil;
        }
        else if ((lastSessionType == ABPAL_AudioSession_Play && audioSessionType != ABPAL_AudioSession_Silent) ||
                 (lastSessionType == ABPAL_AudioSession_PlayAndRecord && audioSessionType == ABPAL_AudioSession_PlayAndRecord && lastSession.audioPlayer && player))
        {
            // Begin a new playing will stop old playing.
            // Few recordes with ABPAL_AudioSession_PlayAndRecord are allowed at same time.
            PalAudioPlayer* audioPlayer = (PalAudioPlayer*)lastSession.audioPlayer;

            if (audioPlayer)
            {
                NSThread* audioSessionThreadOwner = lastSession.audioSessionThreadOwner;

                // Stop audio player from it thread owner
                if ((audioSessionThreadOwner == nil) || (audioSessionThreadOwner == [NSThread currentThread]) ||
                    [audioSessionThreadOwner isFinished] || [audioSessionThreadOwner isCancelled])
                {
                    [audioPlayer stopOnInterruption];
                }
                else
                {
                    [audioPlayer performSelector:@selector(stopOnInterruption) onThread:audioSessionThreadOwner withObject:nil waitUntilDone:NO];
                }
            }
        }

        //Do not interrupt play or record sessions with silent. Just insert silent session behind play/record.
        if ( audioSessionType == ABPAL_AudioSession_Silent &&
            (lastSessionType == ABPAL_AudioSession_Play || lastSessionType == ABPAL_AudioSession_Record || lastSessionType == ABPAL_AudioSession_PlayAndRecord) )
        {
            int idx;
            for (idx = [sessionStack count] - 2; idx >= 0; idx--)
            {
                ABPAL_AudioSessionType tmpSessionType = [[sessionStack objectAtIndex:idx] getSessionType];

                if (tmpSessionType != ABPAL_AudioSession_Play && tmpSessionType != ABPAL_AudioSession_Record && tmpSessionType != ABPAL_AudioSession_PlayAndRecord)
                    break;
            }

            [sessionStack insertObject:audioSession atIndex:(idx + 1)];
            return [audioSession retain];
        }

        //If requested session type is the same with current just push session object to stack
        if (audioSessionType == lastSessionType)
        {
            [sessionStack addObject:audioSession];
            return [audioSession retain];
        }

        //Deactivate current audio session
        [lastSession deactivate];
    }

    //Push
    [sessionStack addObject:audioSession];

    //Activate passed audio session
    [audioSession activate];

    return [audioSession retain];
}

- (void) CloseAudioSessionImp:(PalAudioSession*) audioSession
{
    //Guard against nil
    if (audioSession == nil)
    {
        return;
    }

    [audioSession autorelease];

    //Guard against empty stack
    if ([self IsEmpty])
    {
        return;
    }

    if ([sessionStack lastObject] == audioSession)
    {
        //Audio session to close is on a top

        //Pop
        [sessionStack removeLastObject];

        //If current session type is the same with previous just remove session object
        if (![self IsEmpty] && [[sessionStack lastObject] getSessionType] == audioSession.sessionType)
        {
            return;
        }
        else
        {
            //Deactivate current audio session
            [audioSession deactivate];
        }

        if (![self IsEmpty])
        {
            //Activate previous audio session
            [[sessionStack lastObject] activate];
        }
    }
    else
    {
        // Audio session to close is not on a top. Just remove session object.
        // audioSession object could be already removed from stack if beginInterruption was invoked
        if ([sessionStack containsObject:audioSession])
        {
            [sessionStack removeObject:audioSession];
        }
    }
}

- (BOOL) IsEmpty
{
    return ([sessionStack count] == 0);
}

- (void) audioInterrupted:(NSNotification*)notification
{
    id interruptionType = [notification.userInfo objectForKey:AVAudioSessionInterruptionTypeKey];
    if (interruptionType)
    {
        AVAudioSessionInterruptionType type = [interruptionType intValue];
        switch (type)
        {
            case AVAudioSessionInterruptionTypeBegan:
                [self beginInterruption];
                break;
            case AVAudioSessionInterruptionTypeEnded:
                [self endInterruption];
                break;
        }
    }
}

- (void) beginInterruption
{
    PAL_LockLock(lock);
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    if (![self IsEmpty])
    {
        PalAudioSession* lastSession = (PalAudioSession*)[sessionStack lastObject];
        ABPAL_AudioSessionType lastSessionType = lastSession.sessionType;

        // Stop and remove play/record audio sessions since we can't resume their in further
        if (lastSessionType != ABPAL_AudioSession_Silent)
        {
            PalAudioPlayer* audioPlayer = lastSession.audioPlayer;
            PalQueueAudioRecorder* audioRecorder = lastSession.audioRecorder;
            NSThread* audioSessionThreadOwner = lastSession.audioSessionThreadOwner;

            [lastSession deactivate];
            [sessionStack removeObject:lastSession];
            lastSession = nil;

            if (audioPlayer)
            {
                // Stop audio player from it thread owner
                if ((audioSessionThreadOwner == nil) || (audioSessionThreadOwner == [NSThread currentThread]) ||
                    [audioSessionThreadOwner isFinished] || [audioSessionThreadOwner isCancelled])
                {
                    [audioPlayer stopOnInterruption];
                }
                else
                {
                    [audioPlayer performSelector:@selector(stopOnInterruption) onThread:audioSessionThreadOwner withObject:nil waitUntilDone:NO];
                }
            }

            if (audioRecorder)
            {
                // Stop audio recorder from it thread owner
                if ((audioSessionThreadOwner == nil) || (audioSessionThreadOwner == [NSThread currentThread]) ||
                    [audioSessionThreadOwner isFinished] || [audioSessionThreadOwner isCancelled])
                {
                    [audioRecorder stop];
                }
                else
                {
                    [audioRecorder performSelector:@selector(stop) onThread:audioSessionThreadOwner withObject:nil waitUntilDone:NO];
                }
            }
        }
    }

    [pool release];
    PAL_LockUnlock(lock);
}

- (void) endInterruption
{
    PAL_LockLock(lock);
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    if (![self IsEmpty])
    {
        PalAudioSession* lastSession = (PalAudioSession*)[sessionStack lastObject];
        ABPAL_AudioSessionType lastSessionType = lastSession.sessionType;

        // Resume silent audio session
        if (lastSessionType == ABPAL_AudioSession_Silent)
        {
            [[sessionStack lastObject] activate];
        }
    }

    [pool release];
    PAL_LockUnlock(lock);
}

- (PalAudioSession*) OpenAudioSession:(ABPAL_AudioSessionType) audioSessionType andPlayer:(PalAudioPlayer*)player
{
    PAL_LockLock(lock);
    PalAudioSession* palAudioSession = [self OpenAudioSessionImp:audioSessionType andPlayer:player andRecorder:nil];
    PAL_LockUnlock(lock);
    return palAudioSession;
}

- (PalAudioSession*) OpenAudioSession:(ABPAL_AudioSessionType) audioSessionType andRecorder:(PalQueueAudioRecorder*)recorder
{
    PAL_LockLock(lock);
    PalAudioSession* palAudioSession = [self OpenAudioSessionImp:audioSessionType andPlayer:nil andRecorder:recorder];
    PAL_LockUnlock(lock);
    return palAudioSession;
}

- (void) CloseAudioSession:(PalAudioSession*) audioSession
{
    PAL_LockLock(lock);
    [self CloseAudioSessionImp:audioSession];
    PAL_LockUnlock(lock);
}

@end
