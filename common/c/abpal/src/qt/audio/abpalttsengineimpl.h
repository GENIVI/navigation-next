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
   @file        abpalttsengineimpl.h
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

#ifndef _ABPALTTSENGINE_IMPL_H_
#define _ABPALTTSENGINE_IMPL_H_

#include "abpalttsengine.h"
//#include <phonon/mediaobject.h>
//#include <phonon/audiooutput.h>
#include <QBuffer>
#include <QObject>
#include <QVector>
#include "flite.h"
#include "palerror.h"
#include "abpalaudio.h"

namespace abpal
{
class ttsengineplaythread;
class PhonoPlayer;
/*! Platform dependent TTSEngine. */
class TTSEngineImpl:public QObject,public TTSEngine
{
    Q_OBJECT
public:
    TTSEngineImpl();
    virtual ~TTSEngineImpl();

    PAL_Error Init();

    virtual void SetListener(TTSEngineListenter* listener);

    /*! Stop playing */
    virtual void Stop();

    /*! Play new TTSData */
    virtual void Play(TTSDataPtr data,      /*!< Data to be played */
                      bool preempt = true   /*!< Flag to indicate whether to preempt or not.
                                                 If set to yes, TTSEngine will stop playing
                                                 current data, and play the new one. */
                      );

    /*! Check if it is playing or not. */
    virtual bool IsPlaying() const;

    /*! Set volume. Allowed values are in the range from 0.0 (silent) to 1.0 (loudest). The default volume is 1.0.*/
    virtual void SetVolume(ABPAL_AudioVolumeLevel volume);
private slots:
    void onFinished();
private:
    bool CstwaveToWaveBuffer(QVector<cst_wave*> waves, QBuffer* buffer, int length);
    void Play(QBuffer* buffer);
private:
    TTSEngineListenter *m_pListener;
    bool m_bInitialize;
    bool m_bIsPlaying;
    int32 m_dataID;
//    Phonon::MediaObject* m_pMediaObject;
//    Phonon::AudioOutput* m_pAudioOutput;
    cst_voice *m_pVoice;
};

}
#endif /* _ABPALTTSENGINE_H_ */

/*! @} */
