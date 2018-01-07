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
    @file ttsplayer.cpp
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
#include "ttsplayer.h"
#include "flite.h"
#include "voxdefs.h"
#include "cst_audio.h"
#include <unistd.h>
#include <QRegExp>
#include <QVector>
#include <QDebug>
#include <QFile>
#ifdef Q_OS_LINUX
#include "playwave.h"
#endif
using namespace locationtoolkit;

TTSPlayer* TTSPlayer::mPlayer = NULL;

TTSPlayerHandler::TTSPlayerHandler(TTSPlayerThread *thread, TTSPlayerSignals *signal):
                                        mThread(thread), mTTSPlayerSignals(signal)
{
}

void TTSPlayerHandler::HandleTTS(const QString& str)
{
    const QRegExp rx("[\\{][0-9]+[\\}]");

    qDebug() << "Handle TTS, threadid:" << QThread::currentThreadId();
    QStringList texts = str.split(rx);//Split if find string "{n}" in tts text.

    QVector<int> durations;
    int pos = 0;

    //Get "{n}" pause durations in tts text.
    while ((pos = rx.indexIn(str, pos)) != -1) 
    {
        QString s = rx.cap();//"{n}"
        s.replace("{", "");
        s.replace("}", "");//"n"
        durations.append(s.toInt());
        pos += rx.matchedLength();
    }

    for (int i=0; i<texts.size(); i++)
    {
        if(mThread->mStopTTS)
        {
            break;
        }

        flite_text_to_speech(texts[i].toStdString().c_str(), mThread->mVoice, "play");//Play a tts text using flite.
        if (durations.size() && i<durations.size())
        {
            usleep(1000000 * (durations[i] + 0.5));//Pause n+0.5 seconds if read "{n}" in tts text.
        }            
    }

    emit mTTSPlayerSignals->PlayCompleted(0);
}

void TTSPlayerHandler::HandleWave(const QString& str)
{
    qDebug() << "Handle TTS, threadid:" << QThread::currentThreadId();
#ifdef Q_OS_LINUX
    PlayWave(str.toStdString().c_str());
#endif
    //play complete delete the temp file
    QFile::remove(str);
    emit mTTSPlayerSignals->PlayCompleted(0);
}

TTSPlayerThread::TTSPlayerThread():mStopTTS(false)
{
}

void TTSPlayerThread::Init(cst_voice* Voice, TTSPlayer *player)
{
    mVoice = Voice;
    mPlayer = player;
    mStopTTS = false;
}

void TTSPlayerThread::run()
{
    if(!mPlayer)
    {
        return;
    }

    TTSPlayerHandler* handler = new TTSPlayerHandler(this, (TTSPlayerSignals *)&this->mPlayer->GetSignals());
    QObject::connect(mPlayer, SIGNAL(SendTTS(const QString&)), handler, SLOT(HandleTTS(const QString&)), Qt::QueuedConnection);
    QObject::connect(mPlayer, SIGNAL(SendWaveFile(const QString&)), handler, SLOT(HandleWave(const QString&)), Qt::QueuedConnection);

    exec();
}

void TTSPlayerThread::StopTTS()
{
   mStopTTS = true;
   play_wave_stop();
}

void TTSPlayerThread::EnableTTS()
{
    mStopTTS = false;
}

TTSPlayer* TTSPlayer::GetTTSPlayer(const LTKContext &ltkContext, const QString& language)
{
    if (mPlayer == NULL)
    {
        mPlayer = new TTSPlayer(ltkContext, language, NULL);
    }
    return mPlayer;
}

TTSPlayer::TTSPlayer(const LTKContext &ltkContext, const QString& language, QObject* parent): m_bInitialize(false), mSignals(parent)
{
    flite_init();
    m_pVoice = REGISTER_VOX(NULL);
    if(m_pVoice)
    {
        mTTSPlayerThread.Init(m_pVoice, this);
        mTTSPlayerThread.start();
        m_bInitialize = true;
    }
}

TTSPlayer::~TTSPlayer()
{
    mTTSPlayerThread.exit(0);
    mTTSPlayerThread.wait();
    disconnect();

    if(m_pVoice)
    {
        UNREGISTER_VOX(m_pVoice);
    }
}

void TTSPlayer::Play(const Announcement& announcement)
{
    if(!m_bInitialize)
    {
        emit mSignals.PlayError(0, TTSPlayerSignals::APE_InitializeError);
        return;
    }
    if (announcement.GetAudioMimeType() == "ipa")//TTS
    {
        qDebug("TTS: %s ", announcement.GetText().toStdString().c_str());
        mTTSPlayerThread.EnableTTS();
        emit SendTTS(announcement.GetText());
    }
    else if(announcement.GetAudioMimeType() == "tone/aac")
    {
        QString toneWaveName = "./announceTone.wave";
        //save the bytestream to wave file and play it
        QFile mybfile(toneWaveName);
        if (!mybfile.open(QIODevice::WriteOnly))
        {
            qDebug()<<"NavigationSession::OnAnnounce wirte temp aac file failed.";
            return ;
        }
        mybfile.write (announcement.GetAudio());
        mybfile.close();
        emit SendWaveFile(toneWaveName);
    }
}

void TTSPlayer::Stop()
{
    mTTSPlayerThread.StopTTS();
}

void TTSPlayer::SetVolumeLevel(AudioVolumeLevel level)
{
}

const TTSPlayerSignals& TTSPlayer::GetSignals() const
{
    return mSignals;
}

/*! @} */
