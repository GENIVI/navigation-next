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
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QCoreApplication>
#include "palstdlib.h"
#include "abpalttsengineimpl.h"
#include "flite.h"
#include "voxdefs.h"

namespace abpal
{
static const float PauseTime = 0.2;
static const int   SizeOfHeader = 16;
static const int   WaveExtraLength = 8 + SizeOfHeader + 12;
TTSEnginePtr TTSEngine::CreateInstance(const char* language, EngineOutputType outputType, EngineVoiceType voiceType )
{
    TTSEngineImpl* impl = new TTSEngineImpl();
    impl->Init();
    TTSEnginePtr enginePtr(impl);
    return enginePtr;
}

/*! Platform dependent TTSEngine. */
TTSEngineImpl::TTSEngineImpl()
    : m_pListener(NULL),
      m_bInitialize(false),
      m_bIsPlaying(false),
      m_dataID(0),
//      m_pMediaObject(NULL),
//      m_pAudioOutput(NULL),
      m_pVoice(NULL)
{
    m_bInitialize = false;
}

TTSEngineImpl::~TTSEngineImpl()
{
//    if(m_pMediaObject)
//    {
//        delete m_pMediaObject;
//    }
//    if(m_pAudioOutput)
//    {
//        delete m_pAudioOutput;
//    }

    if(m_pVoice)
    {
        UNREGISTER_VOX(m_pVoice);
    }
}

PAL_Error TTSEngineImpl::Init()
{
    flite_init();
    PAL_Error err = PAL_Ok;
//    m_pMediaObject = new Phonon::MediaObject;
//    m_pAudioOutput = new Phonon::AudioOutput;
//    if( !Phonon::createPath( m_pMediaObject, m_pAudioOutput ).isValid() )
//    {
        err = PAL_ErrUnsupported;
//    }
    m_pVoice = REGISTER_VOX(NULL);
    if(!m_pVoice)
    {
        err = PAL_ErrUnsupported;
    }
    if(err == PAL_Ok)
    {
//        connect(m_pMediaObject, SIGNAL(finished()), this, SLOT(onFinished()));
    }
    if(err == PAL_Ok)
    {
        m_bInitialize = true;
    }
    return err;
}

bool TTSEngineImpl::CstwaveToWaveBuffer(QVector<cst_wave*> waves, QBuffer* buffer, int length)
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

void TTSEngineImpl::SetListener(TTSEngineListenter* listener)
{
    m_pListener = listener;
}

void TTSEngineImpl::Stop()
{
//    if(m_pMediaObject)
//    {
//        m_pMediaObject->stop();
//    }

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
    m_dataID = data->GetDataID();
    QBuffer* pBuffer = new QBuffer;
    QString tempStr = data->GetRawText().c_str();
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
            nsl_memset(buf,0,pauseLength);
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
        if(m_pListener)
        {
            m_pListener->OnPlayCompleted(m_dataID);
        }
        delete pBuffer;
    }
    for(int i=0; i< waves.size();i++)
    {
        delete_wave(waves[i]);
    }
}

bool TTSEngineImpl::IsPlaying() const
{
     return m_bIsPlaying;
}

void TTSEngineImpl::SetVolume(ABPAL_AudioVolumeLevel volume)
{
    //TODO not support now
}

void TTSEngineImpl::Play(QBuffer* buffer)
{
//    Phonon::MediaSource mediaSource( buffer );
//    mediaSource.setAutoDelete( true );
//    m_pMediaObject->setCurrentSource( mediaSource );
//    m_pMediaObject->play();
}

void TTSEngineImpl::onFinished()
{
    if(m_pListener)
    {
        m_pListener->OnPlayCompleted(m_dataID);
    }
}

}

/*! @} */
