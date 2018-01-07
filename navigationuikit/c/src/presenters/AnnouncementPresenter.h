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
 @file         AnnouncementPresenter.h
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

#ifndef __NAVIGATIONUIKIT_ANNOUNCEMENTPRESENTER_H__
#define __NAVIGATIONUIKIT_ANNOUNCEMENTPRESENTER_H__

#include "PresenterBase.h"
#include "AudioPlayer.h"
#include "NavApiListeners.h"
#include "AnnouncementWidget.h"
#include <map>

namespace nkui
{

class AnnouncementPresenter : public PresenterBase,
                              public nbnav::AnnouncementListener,
                              public AudioPlayerListener
{
public:
    AnnouncementPresenter();
    virtual ~AnnouncementPresenter();

    /*! please refer nbnav::AnnouncementListener. */
    virtual void Announce(shared_ptr<nbnav::Announcement> announcement);

    /*! please refer AudioPlayerListener. */
    virtual void OnInitializeCompleted();
    virtual void OnPlayCompleted(int32 annoucementID);
    virtual void OnPlayError(int32 annoucementID, AudioPlayerError error);

    /*! please refer PresenterBase. */
    virtual void NotifyEvent(NKEventID event, NKUIEventData data);
    virtual void HandleEvent(NKEventID id, void* data);

protected:
    /*! please refer PresenterBase. */
    virtual void OnActive();
    virtual void OnDeactivate();
    virtual void OnSetWidget();

private:
    /*! Utility to stop all players.

        This should be called when deactivating or ending trip.
     */
    void StopAllPlayers();
    void SetVolume();

    /*! Play a preserved audio.
     */
    void PlayPreservedAudio(PreservedAudio audioType);

    typedef map<std::string, AudioPlayerPtr> AUDIO_PLAYER_MAP;
    AUDIO_PLAYER_MAP m_players;

    AnnouncementWidget* m_audiowidget;
    AnnouncementPlayerItem m_currentAnnouncement;
    AudioVolumeLevel m_audioVolumeLevel;
};

}

#endif
