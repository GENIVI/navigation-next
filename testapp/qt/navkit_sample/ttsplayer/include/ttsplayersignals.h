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
    @file ttsplayersignals.h
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
#ifndef LOCATIONTOOLKIT_TTS_PLAYER_SIGNALS_H
#define LOCATIONTOOLKIT_TTS_PLAYER_SIGNALS_H

#include <QObject>

namespace locationtoolkit
{

class TTSPlayerSignals: public QObject
{
    Q_OBJECT
public:
    /** The error enum of audio player.*/
    typedef enum
    {
        APE_InitializeError,
        APE_Canceled,
        APE_EngineError,
        APE_NoMemory,
        APE_Unknown,
    } AudioPlayerError;

public:
    ~TTSPlayerSignals(){}
Q_SIGNALS:
    /**
     * Emit when TTS player be initialized.
     */
    void InitializeCompleted();

    /**
     * Emit when one announcement play completed.
     *
     * @param annoucementID The annoucement ID was played completed.
     */
    void PlayCompleted(qint32 annoucementID);

    /**
     * Emit when error happend durning play process.
     *
     * @param annoucementID The announcement ID which one is on error
     * @param error The error enum data.
     */
    void PlayError(qint32 annoucementID, AudioPlayerError error);
private:
    explicit TTSPlayerSignals(QObject* parent): QObject(parent){}
    friend class TTSPlayer;
    friend class TTSPlayerHandler;
};
}
#endif
/*! @} */
