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
 @file         AnnouncementPresenter.cpp
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
 --------------------------------------------------------------------------
 */

/*! @{ */

#include "AnnouncementPresenter.h"
#include "PreservedAudioAnnouncement.h"
#include "palatomic.h"

using namespace std;
using namespace nbnav;
using namespace nkui;

AnnouncementPresenter::AnnouncementPresenter()
    : m_audiowidget(NULL),
      m_audioVolumeLevel(AVL_High)
{
}

AnnouncementPresenter::~AnnouncementPresenter()
{
    if (m_currentAnnouncement.annoucement)
    {
        m_currentAnnouncement.annoucement->OnAnnouncementComplete();
    }
}

void AnnouncementPresenter::Announce(shared_ptr<Announcement> announcement)
{
    if (!IsActive())
    {
        return;
    }

    AudioPlayerPtr player;
    string mimeType = announcement->GetAudioMimeType();

    AUDIO_PLAYER_MAP::iterator iter = m_players.begin();
    AUDIO_PLAYER_MAP::iterator end  = m_players.end();
    for (; iter != end; ++iter)
    {
        iter->second->Stop(); // Stop all players first.
        if (!iter->first.compare(mimeType))
        {
            player = iter->second;
        }
    }

    if (!player)
    {
        //@todo:get local setting from app or change the interface of navkit to tranport
        //language from server side.
        player = AudioPlayer::GetPlayer(m_pNavUIContex->GetPalInstance(),
                                        m_pNKUIController->GetLocale(),
                                        mimeType);

        if (player)
        {
            player->SetListener(this);
            player->SetVolumeLevel(m_audioVolumeLevel);
            m_players.insert(AUDIO_PLAYER_MAP::value_type(mimeType, player));
        }
    }

    if (player)
    {
        /*! if in calling state, navuiKit should not play the announcement, but play "clang" */
        if (m_pNKUIController->GetCallingState())
        {
            //@todo:shall we play "clang" in NavUIkit or notify the calling state to navkit?
            announcement->OnAnnouncementComplete();
        }
        else
        {
            m_audiowidget->SetFocused(true);
            if (m_currentAnnouncement.annoucement)
            {
                m_currentAnnouncement.annoucement->OnAnnouncementComplete();
            }
            PAL_AtomicIncrement(&m_currentAnnouncement.id);

            m_currentAnnouncement.annoucement = announcement;
            player->SetVolumeLevel(m_audioVolumeLevel);
            player->Play(m_currentAnnouncement);
        }
    }
}

void AnnouncementPresenter::OnInitializeCompleted()
{
}

void AnnouncementPresenter::OnPlayCompleted(int32 annoucementID)
{
    if (m_currentAnnouncement.annoucement && m_currentAnnouncement.id == annoucementID)
    {
        m_currentAnnouncement.annoucement->OnAnnouncementComplete();
        m_currentAnnouncement.annoucement.reset();
    }
    m_audiowidget->SetFocused(false);
}

void AnnouncementPresenter::OnPlayError(int32 annoucementID, AudioPlayerError error)
{
    if (m_currentAnnouncement.annoucement && m_currentAnnouncement.id == annoucementID)
    {
        m_currentAnnouncement.annoucement->OnAnnouncementComplete();
        m_currentAnnouncement.annoucement.reset();
    }
    m_audiowidget->SetFocused(false);
}

void AnnouncementPresenter::OnActive()
{
    PresenterBase::OnActive();
    m_pNavUIContex->GetNavigation()->AddAnnouncementListener(this);
    m_pNKUIController->RegisterEvent(EVT_TAP_NAV_HEADER, this);
    m_pNKUIController->RegisterEvent(EVT_END_TRIP, this);
    m_pNKUIController->RegisterEvent(EVT_ENTER_CALLING_STATE, this);
    m_pNKUIController->RegisterEvent(EVT_SAR_SHOW, this);
    m_pNKUIController->RegisterEvent(EVT_SAR_HIDE, this);
    m_pNKUIController->RegisterEvent(EVT_ANNOUNCE, this);
    m_pNKUIController->RegisterEvent(EVT_ANNOUNCE_MANEUVER, this);
    m_audioVolumeLevel = AVL_High;
    SetVolume();
}

void AnnouncementPresenter::OnDeactivate()
{
    PresenterBase::OnDeactivate();
    m_pNavUIContex->GetNavigation()->RemoveAnnouncementListener(this);
}

void AnnouncementPresenter::OnSetWidget()
{
    m_audiowidget = dynamic_cast<AnnouncementWidget*>(m_pWidget);
}

void AnnouncementPresenter::HandleEvent(NKEventID id, void* data)
{
    if(id == EVT_TAP_VOLUME_BUTTON)
    {
        //current requirement only have two state:High and Mute
        if(m_audioVolumeLevel == AVL_High)
        {
            m_audioVolumeLevel = AVL_Mute;
        }
        else
        {
            m_audioVolumeLevel = AVL_High;
        }
        SetVolume();
    }
}

void AnnouncementPresenter::SetVolume()
{
    std::string volumeImageName;
    switch (m_audioVolumeLevel) {
        case AVL_High:
            volumeImageName = "icon_audio_high.png";
            break;
        case AVL_Low:
            volumeImageName = "icon_audio_low.png";
            break;
        case AVL_Mute:
            volumeImageName = "icon_audio_off.png";
            break;
        default:
            break;
    }
    m_audiowidget->SetVolumeImage(volumeImageName);
    for(AUDIO_PLAYER_MAP::iterator iter = m_players.begin(); iter != m_players.end(); iter++)
    {
        iter->second->SetVolumeLevel(m_audioVolumeLevel);
    }
}

/* See description in header file. */
void AnnouncementPresenter::PlayPreservedAudio(PreservedAudio audioType)
{
    static const char* s_preservedAudioFileNames[PreservedAudio_Num] = {
        "press-start-to-navigate"
    };

    static const char* s_preservedAudioTextIds[PreservedAudio_Num] = {
        "IDS_PRESS_START_TO_NAVIGATE"
    };
    if (audioType < PreservedAudio_Num)
    {
        std::string filePath = m_pNKUIController->GetWorkFolder();
        std::string voicesStyle = m_pNavUIContex->GetRouteOptions().GetPronunciationStyle();
        std::string audioLocale = "NK_" + m_pNKUIController->GetLocale() + "_audio";
        filePath += audioLocale + PATH_DELIMITER + voicesStyle + PATH_DELIMITER + s_preservedAudioFileNames[audioType] + ".aac";
        AudioDataType type = m_pNavUIContex->GetPreferences().GetDownloadableAudioEnabled() ? AudioDataType_AAC : AudioDataType_TTS;
        std::string audioText = m_pStringHelper->GetLocalizedString(s_preservedAudioTextIds[audioType]);
        shared_ptr<Announcement> announcement = shared_ptr<Announcement>(
                                                    new PreservedAudioAnnouncement(
                                                        m_pNavUIContex->GetPalInstance(),
                                                        audioText.c_str(),
                                                        filePath.c_str(),
                                                        type));
        Announce(announcement);
    }
}

void AnnouncementPresenter::NotifyEvent(NKEventID event, NKUIEventData data)
{
    switch (event)
    {
        case EVT_TAP_NAV_HEADER:
        {
            m_pNavUIContex->GetNavigation()->Announce();
            break;
        }
        case EVT_END_TRIP:
        {
            m_audiowidget->PrepareToDestroy();
            StopAllPlayers();
            break;
        }
        case EVT_ENTER_CALLING_STATE:
        {
            StopAllPlayers();
            break;
        }
        case EVT_SAR_HIDE:
        {
            m_pWidget->Show();
            break;
        }
        case EVT_SAR_SHOW:
        {
            m_pWidget->Hide();
            break;
        }
        case EVT_ANNOUNCE:
        {
            PreservedAudio audio = *(PreservedAudio*)data.eventData;
            PlayPreservedAudio(audio);
            break;
        }
        case EVT_ANNOUNCE_MANEUVER:
        {
            int maneuverID = *((int*)data.eventData);
            m_pNavUIContex->GetNavigation()->Announce(maneuverID);
            break;
        }
        default:
        {
            break;
        }
    }
}

/* See description in header file. */
void AnnouncementPresenter::StopAllPlayers()
{
    AUDIO_PLAYER_MAP::iterator iter = m_players.begin();
    AUDIO_PLAYER_MAP::iterator end  = m_players.end();
    for (; iter != end; ++iter)
    {
        iter->second->Stop();
    }
}

/*! @} */
