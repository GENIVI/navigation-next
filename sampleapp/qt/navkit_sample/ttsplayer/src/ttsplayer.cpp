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

static const float PauseTime = 0.2;
static const int   SizeOfHeader = 16;
static const int   WaveExtraLength = 8 + SizeOfHeader + 12;

TTSPlayer* TTSPlayer::mPlayer = NULL;

TTSPlayer* TTSPlayer::GetTTSPlayer(const LTKContext &ltkContext, const QString& language)
{
    if (mPlayer == NULL)
    {
        mPlayer = new TTSPlayer(ltkContext, language, NULL);
    }
    return mPlayer;
}

TTSPlayer::TTSPlayer(const LTKContext &ltkContext, const QString& language, QObject* parent)
    : m_bInitialize(false),
      m_bIsPlaying(false),
      mSignals(parent),
      m_Announcement(NULL)
{
    flite_init();
    m_pVoice = REGISTER_VOX(NULL);
    if(m_pVoice)
    {
//        m_pMediaObject = new Phonon::MediaObject;
//        m_pAudioOutput = new Phonon::AudioOutput;
//        if( Phonon::createPath( m_pMediaObject, m_pAudioOutput ).isValid() )
//        {
//            connect(m_pMediaObject, SIGNAL(finished()), this, SLOT(onFinished()));
//            m_bInitialize = true;
//        }
    }
}

TTSPlayer::~TTSPlayer()
{
    disconnect();

    if(m_pVoice)
    {
        UNREGISTER_VOX(m_pVoice);
    }
//    if(m_pMediaObject)
//    {
//        delete m_pMediaObject;
//    }
//    if(m_pAudioOutput)
//    {
//        delete m_pAudioOutput;
//    }

}

bool TTSPlayer::IsPlaying()
{
    return m_bIsPlaying;
}

void TTSPlayer::Play(Announcement* announcement)
{
    if (!announcement)
    {
        return;
    }
    if(!m_bInitialize)
    {
        emit mSignals.PlayError(0, TTSPlayerSignals::APE_InitializeError);
        return;
    }
    if(m_bIsPlaying)
    {
        m_palyAnnouncementList.enqueue(announcement);
        return;
    }
    m_bIsPlaying = true;
    m_Announcement = announcement;
    if (announcement->GetAudioMimeType() == "ipa")//TTS
    {
        QBuffer* pBuffer = new QBuffer;
        const QString tempStr = announcement->GetText();

        QStringList list = tempStr.split(" ");
        QString sentence;
        QRegExp regExp("^\\{([0-9])+\\}$");
        QVector<cst_wave*> waves;
        int length = 0;
        cst_wave* wave;
        for(int i = 0; i< list.size(); i++)
        {
            QString word = list[i].trimmed();
            if(regExp.exactMatch(word))
            {
                //new sentence
                wave = flite_text_to_wave(sentence.toStdString().c_str(), m_pVoice);
                length += wave->num_channels*wave->num_samples*sizeof(short);
                waves.push_back(wave);
                //add a pause
                //@todo get pause value, the format is like "{value}", current it always {0}
                cst_wave* pause = new_wave();
                pause->num_channels = wave->num_channels;
                pause->num_samples = wave->sample_rate*PauseTime;
                int pauseLength = pause->num_channels*pause->num_samples*sizeof(short);
                char* buf = new char[pauseLength];
                memset(buf,0,pauseLength);
                pause->samples = (short int*)buf;
                length += pause->num_channels*pause->num_samples*sizeof(short);
                waves.push_back(pause);
                sentence = "";
            }
            else
            {
                sentence = sentence + " " + word;
            }
        }
        //the last one
        wave = flite_text_to_wave(sentence.toStdString().c_str(), m_pVoice);
        length += wave->num_channels*wave->num_samples*sizeof(short);
        waves.push_back(wave);

        if(CstwaveToWaveBuffer(waves, pBuffer, length))
        {
            Play(pBuffer);  //the buffer will delete by phonon it self
        }
        else
        {
            emit mSignals.PlayError(0, TTSPlayerSignals::APE_Unknown);
            delete pBuffer;
        }
        for(int i=0; i< waves.size();i++)
        {
            delete_wave(waves[i]);
        }
    }
    else if(announcement->GetAudioMimeType() == "tone/aac")
    {
        QBuffer* pBuffer = new QBuffer;
        pBuffer->setData(announcement->GetAudio());
        Play(pBuffer);
    }
}

void TTSPlayer::Stop()
{
    m_palyAnnouncementList.clear();
//    if(m_pMediaObject)
//    {
//        m_pMediaObject->stop();
//        m_bIsPlaying = false;
//    }
}

void TTSPlayer::SetVolumeLevel(AudioVolumeLevel level)
{
}

const TTSPlayerSignals& TTSPlayer::GetSignals() const
{
    return mSignals;
}

bool TTSPlayer::CstwaveToWaveBuffer(QVector<cst_wave*> waves, QBuffer* buffer, int length)
{
    const char *info;
    short stortValue;
    int intValue;
    int numBytes;
    bool error = true;

    if(waves.size()> 0)
    {
        cst_wave* wave = waves[0];
        //add wave's header
        buffer->open(QBuffer::ReadWrite);
        info = "RIFF";
        buffer->write(info,4);
        numBytes = length + WaveExtraLength; //file's length
        buffer->write((char*)&numBytes,4);
        info = "WAVE";
        buffer->write(info,4);
        info = "fmt ";
        buffer->write(info,4);
        numBytes = SizeOfHeader;                   /* size of header */
        buffer->write((char*)&numBytes,4);
        stortValue = RIFF_FORMAT_PCM;        /* sample type */
        buffer->write((char*)&stortValue,2);
        stortValue = wave->num_channels; /* number of channels */
        buffer->write((char*)&stortValue,2);
        intValue = wave->sample_rate;  /* sample rate */
        buffer->write((char*)&intValue,4);
        intValue = wave->sample_rate*wave->num_channels*sizeof(short); /* average bytes per second */
        buffer->write((char*)&intValue,4);
        stortValue = (wave->num_channels* sizeof(short));      /* block align */
        buffer->write((char*)&stortValue,2);
        stortValue = sizeof(short) * 8;                  /* bits per sample */
        buffer->write((char*)&stortValue,2);
        info = "data";
        buffer->write(info,4);
        intValue = length;	      /* bytes in data */
        buffer->write((char*)&intValue,4);
        for(int i=0; i<waves.size();i++)
        {
            wave = waves[i];
            qint64 length = (sizeof(short))*cst_wave_num_channels(wave)*cst_wave_num_samples(wave);
            if(buffer->write((char*)wave->samples,length) != length)
            {
                error = false;
                break;
            }
        }
    }
    return error;
}

void TTSPlayer::Play(QBuffer* buffer)
{
//    Phonon::MediaSource mediaSource( buffer );
//    mediaSource.setAutoDelete( true );
//    m_pMediaObject->setCurrentSource( mediaSource );
//    m_pMediaObject->play();
}

void TTSPlayer::onFinished()
{
    m_bIsPlaying = false;
    if (m_Announcement)
    {
        m_Announcement->Finished();
    }
    if(m_palyAnnouncementList.count() > 0)
    {
        Play(m_palyAnnouncementList.dequeue());
    }
}

/*! @} */
