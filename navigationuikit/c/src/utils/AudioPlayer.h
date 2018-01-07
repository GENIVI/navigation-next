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
 @file         AudioPlayer.h
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

#ifndef __NAVIGATIONUIKIT_AUDIOPLAYER_H__
#define __NAVIGATIONUIKIT_AUDIOPLAYER_H__

#include "nbcontext.h"
#include "pal.h"
#include "NavApiTypes.h"
#include "smartpointer.h"
#include <string>

namespace nkui
{
/** The annoucement player item which is used to be container of audio player play interface.
 */
class AnnouncementPlayerItem
{
public:
    int32 id;
    shared_ptr<nbnav::Announcement> annoucement;
    AnnouncementPlayerItem() : id(0) {}
};

class AudioPlayer;

typedef shared_ptr<AudioPlayer> AudioPlayerPtr;

/*! The error enum of audio player.
 */
typedef enum
{
    InitializeError,
    Canceled,
    EngineError,
    NoMemory,
    Unknown,
} AudioPlayerError;

/*! Audio Volume level. */
typedef enum
{
    AVL_INAVLID,
    AVL_High,
    AVL_Low,
    AVL_Mute
} AudioVolumeLevel;

/*! The audio player listener.

    The audio player listener which will be set to audio player,
    is used to receive notification of audio play process.
 */
class AudioPlayerListener
{
public:
    virtual ~AudioPlayerListener() {}

    /*! It will be invoked when audio player engine initialize completed.

        @param annoucementID The announcement ID which one is played
     */
    virtual void OnInitializeCompleted() {}

    /*! It will be invoked when one announcement play completed.
     */
    virtual void OnPlayCompleted(int32 annoucementID) {}

    /*! It will be invoked when error happend durning play process.

        @param annoucementID The announcement ID which one is on error
        @param error The error enum data.
     */
    virtual void OnPlayError(int32 annoucementID, AudioPlayerError error) {}
};

/*! The base audio player class.
 */
class AudioPlayer
{
public:
    /*! Static method to get audio player instance with mime type.

        @param pal  The pal instance.
        @param language The language for player
        @param mimeType The mime type
        @return audio play shared pointer.
     */
    static AudioPlayerPtr GetPlayer(PAL_Instance* pal,
                                    const std::string& language,
                                    const std::string& mimeType);

    virtual ~AudioPlayer() {};

    /*! Play with the announcement information.

        @param announcement  The announcement data.
     */
    virtual void Play(const AnnouncementPlayerItem& announcement) = 0;

    /*! Stop play annoucement.
     */
    virtual void Stop() = 0;

    /*! set volume level. */
    virtual void SetVolumeLevel(AudioVolumeLevel level) = 0;

    /*! Set a listener to audio player.

        @param listener The audio player listener instance.
     */
    void SetListener(AudioPlayerListener* listener)
    {
        m_listener = listener;
    }

protected:
    /*! AudioPlayer construction.

     @param pal  The pal instance.
     @param language The language for player
     */
    AudioPlayer(PAL_Instance* pal, const std::string& language)
            : m_pal(pal),
              m_language(language),
              m_listener(NULL) {};

    PAL_Instance* m_pal;
    std::string m_language;
    AudioPlayerListener* m_listener;
};

}

#endif

/*! @} */
