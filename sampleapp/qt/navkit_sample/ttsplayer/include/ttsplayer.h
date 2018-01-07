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
    @file ttsplayer.h
    @date 10/28/2014
    @addtogroup navigationkit
*/
/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
---------------------------------------------------------------------------*/

/*! @{ */
#ifndef LOCATIONTOOLKIT_TTS_PLAYER_H
#define LOCATIONTOOLKIT_TTS_PLAYER_H

#include <QObject>
#include "ltkcontext.h"
#include "announcement.h"
#include "flite.h"
#include "ttsplayersignals.h"
//#include <phonon/mediaobject.h>
//#include <phonon/audiooutput.h>
#include <QBuffer>
#include <QObject>
#include <QVector>
#include <QThread>
#include <QStringList>
#include <QMutex>
#include <QQueue>

namespace locationtoolkit
{
class TTSPlayer: public QObject
{
    Q_OBJECT
public:
    /** Audio Volume level. */
    typedef enum
    {
        AVL_Inavlid,
        AVL_High,
        AVL_Low,
        AVL_Mute
    } AudioVolumeLevel;

public:
    /**
     * Get TTS player instance.
     *
     * @param ltkContext The LTKContext instance.
     * @param language The language for TTS player
     * @return TTS play.
     */
    static TTSPlayer* GetTTSPlayer(const LTKContext &ltkContext, const QString& language);

    ~TTSPlayer();

    /**
      * Is Playing announcement.
     */
    bool IsPlaying();

    /**
     * Play with the announcement information.
     *
     * @param announcement  The announcement data.
     */
    void Play(Announcement *announcement);

    /**
     * Stop play announcement.
     */
    void Stop();

    /**
     * set volume level.
     *
     * @param level The AudioVolumeLevel enum data.
     */
    void SetVolumeLevel(AudioVolumeLevel level);

    /**
     * Get signals for Audioplayer.
     */
    const TTSPlayerSignals& GetSignals() const;

signals:
    void SendTTS(const QString& str);
    void SendWaveFile(const QString& str);
private slots:
    void onFinished();
private:
    bool CstwaveToWaveBuffer(QVector<cst_wave*> waves, QBuffer* buffer, int length);
    void Play(QBuffer* buffer);
private:
    TTSPlayer(const LTKContext &ltkContext, const QString& language, QObject* parent);

    static TTSPlayer* mPlayer;

    TTSPlayerSignals mSignals;
    cst_voice *m_pVoice;
    bool m_bInitialize;
    bool m_bIsPlaying;
//    Phonon::MediaObject* m_pMediaObject;
//    Phonon::AudioOutput* m_pAudioOutput;
    Announcement* m_Announcement;
    QQueue<Announcement*> m_palyAnnouncementList;
};
}
#endif
/*! @} */
