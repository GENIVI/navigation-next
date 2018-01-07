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
   @file        AbpalTTSEngineImpl.h
   @defgroup    abpal
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

#ifndef _ABPALTTSENGINEIMPL_H_
#define _ABPALTTSENGINEIMPL_H_

#import <AVFoundation/AVFoundation.h>

#include "abpalttsengine.h"

namespace abpal
{
class TTSEngineIphone;
}

// Configuration of TTS Engine.
@interface TTSEngineIphoneConfig : NSObject
@property(nonatomic) float rate;             // Values are pinned between AVSpeechUtteranceMinimumSpeechRate and AVSpeechUtteranceMaximumSpeechRate.
@property(nonatomic) float pitchMultiplier;  // [0.5 - 2] Default = 1
@property(nonatomic) float volume;           // [0-1] Default = 1
-(id) init;
@end

@interface TTSEngineSynthesizerDelegate : NSObject<AVSpeechSynthesizerDelegate>
{
    @private
            abpal::TTSEngineIphone* m_engine;
            shared_ptr<bool> m_isValid;
}
- (id)initWithPlayer:(abpal::TTSEngineIphone*) player
       withValidFlag:(shared_ptr<bool>) valid;
@end


namespace abpal
{

// Refer to TTSEngine for descriptions please.
class TTSEngineIphone : public TTSEngine
{
public:
    TTSEngineIphone(const char*  language,
                    EngineOutputType outputType,
                    EngineVoiceType voiceType);
    virtual ~TTSEngineIphone();

    virtual void SetListener(TTSEngineListenter* listener)
    {
        m_listener = listener;
    }

    virtual void Stop();
    virtual void Play(TTSDataPtr data, bool preempt = true);
    virtual bool IsPlaying() const;
    virtual void SetVolume(ABPAL_AudioVolumeLevel volume);

    // Notify listeners about status of synthesizer.
    void OnError(id utterance, EngineError error);
    void OnCompleted(id utterance);

private:

    TTSEngineListenter*           m_listener;
    AVSpeechSynthesizer*          m_synthesizer;
    TTSEngineSynthesizerDelegate* m_delegate;
    TTSEngineIphoneConfig*        m_config;
    NSMutableArray*               m_utteranceArray;
    EngineOutputType              m_outputType;
    bool                          m_isInitialized;
    string                        m_language;
    shared_ptr<bool>              m_isValid;
    int32                         m_dataID;
};

}




#endif /* _ABPALTTSENGINEIMPL_H_ */

/*! @} */
