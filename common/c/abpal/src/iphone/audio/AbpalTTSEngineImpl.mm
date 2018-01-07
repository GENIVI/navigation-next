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
   @file        AbpalTTSEngineImpl.mm
   @defgroup    objc
*/
/*
   (C) Copyright 2014 by TeleCommunication Systems, Inc.

   The information contained herein is confidential, proprietary to
   TeleCommunication Systems, Inc., and considered a trade secret as defined
   in section 499C of the penal code of the State of California. Use of this
   information by anyone other than authorized employees of TeleCommunication
   Systems is granted only under a written non-disclosure agreement, expressly
   prescribing the scope and manner of such use.
--------------------------------------------------------------------------*/

/*! @{ */

#include "AbpalTTSEngineImpl.h"
#include "palstdlib.h"
#import <UIKit/UIDevice.h>

using namespace std;
using namespace abpal;

#define SYSTEM_VERSION_GREATER_THAN_OR_EQUAL_TO(v)  ([[[UIDevice currentDevice] systemVersion] compare:v options:NSNumericSearch] != NSOrderedAscending)

@implementation TTSEngineIphoneConfig

-(id) init
{
    self = [super init];

    // All set to default, please refer to <AVFoundation/AVFoundation.h> for descriptions.
    self.rate            = SYSTEM_VERSION_GREATER_THAN_OR_EQUAL_TO(@"8.0") ? 0.13 : 0.26; // this value sounds good.
    self.pitchMultiplier = 1;
    self.volume          = 1;

    return self;
}
@end

@implementation TTSEngineSynthesizerDelegate

-(id)initWithPlayer:(abpal::TTSEngineIphone *)engine
      withValidFlag:(shared_ptr<bool>)valid
{
    self = [super init];

    if (self)
    {
        m_engine = engine;
        m_isValid = valid;
        return self;
    }
    else
    {
        return nil;
    }
}

- (void)speechSynthesizer:(AVSpeechSynthesizer *)synthesizer
  didStartSpeechUtterance:(AVSpeechUtterance *)utterance
{
}

- (void)speechSynthesizer:(AVSpeechSynthesizer *)synthesizer
 didFinishSpeechUtterance:(AVSpeechUtterance *)utterance
{
    if (m_engine && m_isValid && *m_isValid)
    {
        m_engine->OnCompleted(utterance);
    }
}

- (void)speechSynthesizer:(AVSpeechSynthesizer *)synthesizer
  didPauseSpeechUtterance:(AVSpeechUtterance *)utterance
{
}

- (void)speechSynthesizer:(AVSpeechSynthesizer *)synthesizer
didContinueSpeechUtterance:(AVSpeechUtterance *)utterance
{
}

- (void)speechSynthesizer:(AVSpeechSynthesizer *)synthesizer
 didCancelSpeechUtterance:(AVSpeechUtterance *)utterance
{
    if (m_engine && m_isValid && *m_isValid)
    {
        m_engine->OnError(utterance, EE_Canceled);
    }
}

- (void)speechSynthesizer:(AVSpeechSynthesizer *)synthesizer
willSpeakRangeOfSpeechString:(NSRange)characterRange
                utterance:(AVSpeechUtterance *)utterance
{
}

@end

// Implementation of TTSEngineIphone.

static const char   BREAK_START_CHARACTER = '{';
static const char   BREAK_END_CHARACTER   = '}';

TTSEnginePtr TTSEngine::CreateInstance(const char*      language,
                                       EngineOutputType outputType,
                                       EngineVoiceType  voiceType)
{
    TTSEnginePtr engine;

    if (voiceType < EVT_Invalid && outputType < EOT_Invalid)
    {
        engine.reset(new TTSEngineIphone(language, outputType, voiceType));
    }

    return engine;
}

/* See description in header file. */
TTSEngineIphone::TTSEngineIphone(const char*      language,
                                 EngineOutputType outputType,
                                 EngineVoiceType  voiceType)
        : m_listener(NULL),
          m_outputType(outputType),
          m_isInitialized(false),
          m_language(language ? language:"en-US"),
          m_isValid(new bool(true)),
          m_dataID(0)
{
    //@todo: should check input language and map to proper language tag.
    m_utteranceArray = [[NSMutableArray alloc] init];

    m_synthesizer = [[AVSpeechSynthesizer alloc] init];
    m_delegate = [[TTSEngineSynthesizerDelegate alloc] initWithPlayer:this
                                                        withValidFlag:m_isValid];
    [m_synthesizer setDelegate:m_delegate];

    m_config = [[TTSEngineIphoneConfig alloc] init];

    // @tricky: we want to provide a way for DEV and QA to change Rate and Pitch of TTS, but
    //          we don't want to provide public API for this internal test, so a plugin is
    //          used here. It should be safe to keep this code since class name of this
    //          plugin is rather strange, and the chance to collision should be small. We
    //          can remove following lines in future releases.
    Class pluginConfigClass = NSClassFromString(@"TTSEngineIphoneConfigPlugin");
    if (pluginConfigClass != nil)
    {
        id pluginConfigInstance = [[pluginConfigClass alloc] init];
        if (pluginConfigInstance != nil)
        {
            id value = [pluginConfigInstance valueForKey:@"rate"];
            if (value != nil)
            {
                [m_config setValue:value forKey:@"rate"];
            }
            value = [pluginConfigInstance valueForKey:@"pitchMultiplier"];
            if (value != nil)
            {
                [m_config setValue:value forKey:@"pitchMultiplier"];
            }
        }
    }

    m_isInitialized = true;
}

/* See description in header file. */
TTSEngineIphone::~TTSEngineIphone()
{
    *m_isValid = false;
    Stop();
    [[AVAudioSession sharedInstance] setActive:FALSE error:nil];
}

/* See description in header file. */
void TTSEngineIphone::Stop()
{
    if (m_synthesizer)
    {
        [m_synthesizer stopSpeakingAtBoundary: AVSpeechBoundaryImmediate];

        //@bug: There is a bug in iOS where stopSpeakingAtBoundary sometimes does not work.
        //      A workaround is to add an empty utterance to engine and stop it.
        AVSpeechUtterance *utterance = [AVSpeechUtterance speechUtteranceWithString:@""];
        [m_synthesizer speakUtterance:utterance];
        [m_synthesizer stopSpeakingAtBoundary:AVSpeechBoundaryImmediate];
    }
    [m_utteranceArray removeAllObjects];
}

/* See description in header file. */
void TTSEngineIphone::Play(TTSDataPtr data, bool preempt)
{
    if (!m_isInitialized)
    {
        if (m_listener)
        {
            m_listener->OnPlayError(data->GetDataID(), EE_NotInitialized);
            return;
        }
    }

    if (preempt)
    {
        Stop();
    }

    // Set session properities before starting playing.
    AVAudioSession *audioSession = [AVAudioSession sharedInstance];
    [audioSession setActive:FALSE error:nil];

    switch (m_outputType)
    {
        case EOT_MixWithOthers:
        {
            [audioSession setCategory:AVAudioSessionCategoryPlayback
                          withOptions:AVAudioSessionCategoryOptionMixWithOthers error:nil];
            break;
        }
        case EOT_DuckOthers:
        {
            [audioSession setCategory:AVAudioSessionCategoryPlayback
                          withOptions:AVAudioSessionCategoryOptionDuckOthers error:nil];

            break;
        }
        default:
        {
            break;
        }
    }

    m_dataID = data->GetDataID();

    const char* announceText = data->GetRawText().c_str();
    const char* announceHead = announceText;
    const char* announceTail = announceHead + nsl_strlen(announceText);
    char* breakStart = NULL;
    char* breakEnd = NULL;
    if ((breakStart = nsl_strchr(announceHead, BREAK_START_CHARACTER)))
    {
        while (breakStart && breakStart < announceTail)
        {
            //add before announce into array
            *breakStart = 0;
            AVSpeechUtterance* utterance = [AVSpeechUtterance speechUtteranceWithString:[NSString stringWithUTF8String:announceHead]];
            [m_utteranceArray addObject:utterance];

            //find break time and set it to before utterance
            double breakTime = 0;
            breakStart++;
            nsl_sscanf(breakStart, "%lf", &breakTime);
            if (breakTime > 0)
            {
                [utterance setPostUtteranceDelay: breakTime];
            }

            //move head pointer to next position
            breakEnd = nsl_strchr(breakStart, BREAK_END_CHARACTER);
            if (!breakEnd)
            {
                break;
            }
            announceHead = breakEnd + 1;

            //find next break start
            breakStart = nsl_strchr(announceHead, BREAK_START_CHARACTER);
        }
        //add last announce into array
        AVSpeechUtterance* utteranceRemain = [AVSpeechUtterance speechUtteranceWithString:[NSString stringWithUTF8String:announceHead]];
        [m_utteranceArray addObject:utteranceRemain];
    }
    else
    {
        //no break case
        AVSpeechUtterance* utterance = [AVSpeechUtterance speechUtteranceWithString:[NSString stringWithUTF8String:announceText]];
        [m_utteranceArray addObject:utterance];
    }

    //play announce with utterance array.
    for(AVSpeechUtterance* utteranceItem in m_utteranceArray)
    {
        utteranceItem.voice = [AVSpeechSynthesisVoice voiceWithLanguage:[NSString stringWithUTF8String:m_language.c_str()]];;
        utteranceItem.rate  = m_config.rate;
        utteranceItem.pitchMultiplier = m_config.pitchMultiplier;
        utteranceItem.volume = m_config.volume;
        [m_synthesizer speakUtterance:utteranceItem];
    }
}

bool TTSEngineIphone::IsPlaying() const
{
    return  m_synthesizer ? (m_synthesizer.speaking == YES) : false;
}

void TTSEngineIphone::OnError(id utterance, EngineError error)
{
    // If no more utterance, deactivate this audio session to restore volumes of other apps.
    [m_utteranceArray removeObject:utterance];
    if (m_utteranceArray.count == 0)
    {
        [[AVAudioSession sharedInstance] setActive:FALSE error:nil];

        if (m_listener)
        {
            m_listener->OnPlayError(m_dataID, error);
        }
    }
}

void TTSEngineIphone::OnCompleted(id utterance)
{
    // If no more utterance, deactivate this audio session to restore volumes of other apps.
    [m_utteranceArray removeObject:utterance];
    if (m_utteranceArray.count == 0)
    {
        [[AVAudioSession sharedInstance] setActive:FALSE error:nil];

        if (m_listener)
        {
            m_listener->OnPlayCompleted(m_dataID);
        }
    }
}

void TTSEngineIphone::SetVolume(ABPAL_AudioVolumeLevel volume)
{
    switch (volume) {
        case ABPAL_AudioVolumeLevel_Off:{
            m_config.volume = 0.0;
            break;
        }
        case ABPAL_AudioVolumeLevel_Low:{
            m_config.volume = 0.25;
            break;
        }
        case ABPAL_AudioVolumeLevel_Medium:{
            m_config.volume = 0.5;
            break;
        }
        case ABPAL_AudioVolumeLevel_High:{
            m_config.volume = 0.75;
            break;
        }
        case ABPAL_AudioVolumeLevel_VeryHigh:{
            m_config.volume = 1.0;
            break;
        }
        default:
            break;
    }
    for(AVSpeechUtterance* utteranceItem in m_utteranceArray)
    {
        utteranceItem.volume = m_config.volume;
    }
}

/*! @} */
