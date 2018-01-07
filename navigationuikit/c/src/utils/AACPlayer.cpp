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
 @file         AACPlayer.cpp
 @defgroup     nkui
 */
/*
 (C) Copyright 2014 by TeleCommunications Systems, Inc.

 The information contained herein is confidential, proprietary to
 TeleCommunication Systems, Inc., and considered a trade secret as defined
 in section 499C of the penal code of the State of California. Use of this
 information by anyone other than authorized employees of TeleCommunication
 Systems is granted only under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.
 --------------------------------------------------------------------------*/

/*! @{ */

#include "AACPlayer.h"
#include "nkui_macros.h"

using namespace std;
using namespace nbnav;
using namespace nkui;

class AACPlayerData
{
public:
    AACPlayer* player;
    shared_ptr<bool> isValid;
    int32 announcementID;
};

AACPlayer::AACPlayer(PAL_Instance* pal, const string& language)
        : AudioPlayer(pal, language),
          m_player(NULL),
          m_isValid(NKUI_NEW bool(true)),
          m_isPlaying(false)
{
}

AACPlayer::~AACPlayer()
{
    *m_isValid = false;
    Stop();
    if (m_player)
    {
        ABPAL_AudioPlayerDestroy(m_player);
    }
}

void AACPlayer::Play(const AnnouncementPlayerItem& announcement)
{
    if (!InitPlayer())
    {
        return;
    }

    BinaryBuffer buffer = announcement.annoucement->GetAudio();
    int bufferSize = buffer.GetData().size();
    byte* data = (byte*)nsl_malloc(bufferSize);

    if (!data)
    {
        if(m_listener)
        {
            m_listener->OnPlayError(announcement.id, NoMemory);
        }
        return;
    }

    AACPlayerData* userData = new AACPlayerData();
    if (!userData)
    {
        m_listener->OnPlayError(announcement.id, NoMemory);
        return;
    }
    userData->player = this;
    userData->isValid = m_isValid;
    userData->announcementID = announcement.id;

    nsl_memset(data, 0, bufferSize);
    nsl_memcpy(data, &(buffer.GetData()[0]), bufferSize);

    m_isPlaying = true;

    if (ABPAL_AudioPlayerPlay(m_player, data, bufferSize, true, &AACPlayer::PlayCallback, userData) != PAL_Ok)
    {
        delete userData;
    }
}

void AACPlayer::Stop()
{
    if (m_player)
    {
        ABPAL_AudioPlayerCancel(m_player);
    }
}

void AACPlayer::SetVolumeLevel(AudioVolumeLevel level)
{
    if (!InitPlayer())
    {
        return;
    }

    if(m_player)
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
        ABPAL_AudioPlayerSetParameter(m_player, ABPAL_AudioParameter_Volume, abpalVolumeLevel, 0);
    }
}

bool AACPlayer::InitPlayer()
{
    if (m_player != NULL)
    {
        return true;
    }

    if (ABPAL_AudioPlayerCreate(m_pal, ABPAL_AudioFormat_AAC, &m_player) == PAL_Ok)
    {
        // use default audio route
        ABPAL_AudioPlayerSetParameter(m_player, ABPAL_AudioParameter_SoundDevice, 0, 0);
        if (m_listener)
        {
            m_listener->OnInitializeCompleted();
        }
        return true;
    }

    return false;
}

void AACPlayer::PlayCallback(void* userData, ABPAL_AudioState state)
{
    AACPlayerData* aacPlayer = static_cast<AACPlayerData*>(userData);
    if (aacPlayer)
    {
        if (aacPlayer->isValid && (*aacPlayer->isValid) && aacPlayer->player && aacPlayer->player->m_listener)
        {
            switch (state) {
                case ABPAL_AudioState_Cancel:
                {
                    aacPlayer->player->m_isPlaying = false;
                    aacPlayer->player->m_listener->OnPlayError(aacPlayer->announcementID, Canceled);
                    break;
                }
                case ABPAL_AudioState_Error:
                {
                    aacPlayer->player->m_isPlaying = false;
                    aacPlayer->player->m_listener->OnPlayError(aacPlayer->announcementID, EngineError);
                    break;
                }
                case ABPAL_AudioState_Ended:
                case ABPAL_AudioState_Stopped:
                {
                    aacPlayer->player->m_isPlaying = false;
                    aacPlayer->player->m_listener->OnPlayCompleted(aacPlayer->announcementID);
                    break;
                }
                default:
                    break;
            }
        }
        delete aacPlayer;
    }
}
/*! @} */
