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
   @file        TTSPlayer.cpp
   @defgroup    nkui
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

#include "TTSPlayer.h"
#include "nkui_macros.h"

using namespace nkui;
using namespace std;

// Utility to convert Annoucemnet to TTSData used by ABPal.
inline abpal::TTSDataPtr AnnouncementToTTSData(const AnnouncementPlayerItem& announce)
{
    //@todo: Navkit does not return phonetics now, this should be addressed.
    string emptyString;
    return abpal::TTSDataPtr(NKUI_NEW abpal::TTSData(announce.annoucement->GetAudioMimeType(),
                                                     emptyString,
                                                     announce.annoucement->GetText(),
                                                     announce.id));
}

// Implementation of TTSPlayer.
/* See description in header file. */
TTSPlayer::TTSPlayer(PAL_Instance* pal, const std::string& language)
        : AudioPlayer(pal, language),
          m_volumeLevel(AVL_High)
{
    if (!language.empty())
    {
        m_engine = abpal::TTSEngine::CreateInstance(language.c_str(),
                                                    abpal::EOT_DuckOthers,
                                                    abpal::EVT_Unspecified);
        if (m_engine)
        {
            m_engine->SetListener(this);
        }
    }
}

/* See description in header file. */
TTSPlayer::~TTSPlayer()
{
    if (m_engine)
    {
        m_engine->SetListener(NULL);
    }
}

/* See description in header file. */
void TTSPlayer::Play(const AnnouncementPlayerItem& announcement)
{
    //if set volume to mute, on ios 7.1 the real volume is middle volume, so add extra check
    if (m_engine)
    {
        if(m_volumeLevel != AVL_Mute)
        {
            m_engine->Play(AnnouncementToTTSData(announcement));
        }
        else
        {
            OnPlayCompleted(announcement.id);
        }
    }
    else if (m_listener)
    {
        m_listener->OnPlayError(announcement.id, InitializeError);
    }
}

/* See description in header file. */
void TTSPlayer::Stop()
{
    if (m_engine && m_engine->IsPlaying())
    {
        m_engine->Stop();
    }
}

/* See description in header file. */
void TTSPlayer::OnPlayCompleted(int32 dataID)
{
    if (m_listener)
    {
        m_listener->OnPlayCompleted(dataID);
    }
}

/* See description in header file. */
void TTSPlayer::OnPlayError(int32 dataID, abpal::EngineError error)
{
    if (m_listener)
    {
        switch (error)
        {
            case abpal::EE_NotInitialized:
            {
                m_listener->OnPlayError(dataID, InitializeError);
                break;
            }
            case abpal::EE_Canceled:
            {
                m_listener->OnPlayError(dataID, Canceled);
                break;
            }
            case abpal::EE_InternalError:
            {
                m_listener->OnPlayError(dataID, EngineError);
                break;
            }
            case abpal::EE_NoMemory:
            {
                m_listener->OnPlayError(dataID, NoMemory);
                break;
            }
            default:
            {
                m_listener->OnPlayError(dataID, Unknown);
                break;
            }
        }
    }
}

void TTSPlayer::SetVolumeLevel(AudioVolumeLevel level)
{
    m_volumeLevel = level;
    if (m_engine)
    {
        ABPAL_AudioVolumeLevel abpalVolumeLevel;
        switch (level)
        {
            case AVL_High:
                abpalVolumeLevel = ABPAL_AudioVolumeLevel_VeryHigh;
                break;
            case AVL_Low:
                abpalVolumeLevel = ABPAL_AudioVolumeLevel_Low;
                break;
            case AVL_Mute:
                abpalVolumeLevel = ABPAL_AudioVolumeLevel_Off;
                break;
            default:
                abpalVolumeLevel = ABPAL_AudioVolumeLevel_Off;
                break;
        }
        m_engine->SetVolume(abpalVolumeLevel);
        //tts engine can't dynamic change volume when speaking
        if(level == AVL_Mute)
        {
            m_engine->Stop();
        }
    }
}

/*! @} */
