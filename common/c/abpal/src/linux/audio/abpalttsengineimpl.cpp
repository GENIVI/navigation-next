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
   @file        abpalttsengineimpl.cpp
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
#include "abpalttsengineimpl.h"
#include "flite.h"
#include "voxdefs.h"

namespace abpal
{
TTSEnginePtr TTSEngine::CreateInstance(const char* language, EngineOutputType outputType, EngineVoiceType voiceType )
{
    TTSEnginePtr enginePtr(new TTSEngineImpl());
    return enginePtr;
}

/*! Platform dependent TTSEngine. */
TTSEngineImpl::TTSEngineImpl():m_pListener(NULL), m_bInitialize(false), m_bIsPlaying(false)
{
    flite_init();
    m_pVoice = REGISTER_VOX(NULL);
    assert(m_pVoice);
    if(m_pVoice)
    {
        m_bInitialize = true;
    }
}

TTSEngineImpl::~TTSEngineImpl()
{
    if(m_pVoice)
    {
        UNREGISTER_VOX(m_pVoice);
    }
}

void TTSEngineImpl::SetListener(TTSEngineListenter* listener)
{
    m_pListener = listener;
}

void TTSEngineImpl::Stop()
{

}

void TTSEngineImpl::Play(TTSDataPtr data,bool preempt)
{
    if(!m_bInitialize)
    {
        if(m_pListener)
        {
            m_pListener->OnPlayError(data->GetDataID(), EE_NotInitialized);
        }

        return;
    }

    m_bIsPlaying = true;

    if(preempt)
    {
       Stop();
    }

    flite_text_to_speech(data->GetRawText().c_str(), m_pVoice, "play");

    if(m_pListener)
    {
        m_pListener->OnPlayCompleted(data->GetDataID());
    }

    m_bIsPlaying = false;
}

bool TTSEngineImpl::IsPlaying() const
{
     return m_bIsPlaying;
}

void TTSEngineImpl::SetVolume(ABPAL_AudioVolumeLevel volume)
{

}

}

/*! @} */