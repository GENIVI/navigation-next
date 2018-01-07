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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#include "paltypes.h"
#include "abpalaudio.h"
#include "palaudioimpl.h"

//#include <phonon/audiooutput.h>
//#include <phonon/mediaobject.h>
#include <QBuffer>

//static Phonon::MediaObject* g_pMediaObject = NULL;
//static Phonon::AudioOutput* g_pAudioOutput = NULL;

ABPAL_DEF PAL_Error
ABPAL_AudioCombinerCreate(PAL_Instance *pal, ABPAL_AudioFormat format, ABPAL_AudioCombiner **ppac)
{
    PAL_Error err = PAL_Ok;

    if (!pal || !ppac)
    {
        return PAL_ErrBadParam;
    }
    *ppac = NULL;

    ABPAL_AudioCombiner* pThis = (ABPAL_AudioCombiner*)nsl_malloc(sizeof(*pThis));
    if (pThis)
    {
        err = NIMAudioCombiner_Create(format, &pThis->pAudioCombiner);
        if (!err)
        {
            *ppac = pThis;
        }
        else
        {
            nsl_free(pThis);
        }
    }
    else
    {
        err = PAL_ErrNoMem;
    }

    return err;
}

ABPAL_DEF PAL_Error
ABPAL_AudioCombinerDestroy(ABPAL_AudioCombiner *combiner)
{
    if (!combiner)
    {
        return PAL_ErrBadParam;
    }

    NIMAudioCombiner_Destroy(combiner->pAudioCombiner);
    nsl_free(combiner);

    return PAL_Ok;
}

ABPAL_DEF PAL_Error
ABPAL_AudioCombinerAddBuffer(
        ABPAL_AudioCombiner *combiner,
        byte *buf,
        int size,
        nb_boolean copy_needed)
{
    if (!combiner || !buf)
    {
        return PAL_ErrBadParam;
    }

    return NIMAudioCombiner_AddData(combiner->pAudioCombiner, buf, size, copy_needed);
}

ABPAL_DEF PAL_Error
ABPAL_AudioCombinerGetData(ABPAL_AudioCombiner* combiner, byte** ppdata, int* pnsize)
{
    if (!combiner || !ppdata || !pnsize)
    {
        return PAL_ErrBadParam;
    }

    return NIMAudioCombiner_GetData(combiner->pAudioCombiner, ppdata, pnsize);
}

ABPAL_DEF PAL_Error
ABPAL_AudioPlayerCreate(PAL_Instance *pal, ABPAL_AudioFormat format, ABPAL_AudioPlayer **ppap)
{
//    g_pMediaObject = new Phonon::MediaObject;
//    g_pAudioOutput = new Phonon::AudioOutput;
//    if( Phonon::createPath( g_pMediaObject, g_pAudioOutput ).isValid() )
//    {
//        return PAL_Ok;
//    }
//    else
    {
        return PAL_ErrUnsupported;
    }
}

ABPAL_DEF PAL_Error
ABPAL_AudioPlayerDestroy(ABPAL_AudioPlayer *player)
{
//    if( g_pAudioOutput )
//    {
//        delete g_pAudioOutput;
//    }
//    if( g_pMediaObject )
//    {
//        delete g_pMediaObject;
//    }
    return PAL_Ok;
}

ABPAL_DEF PAL_Error
ABPAL_AudioPlayerPlay(
        ABPAL_AudioPlayer *player,
        byte* pdata,
        int nsize,
        nb_boolean takeover,
        ABPAL_AudioPlayerCallback *cb,
        void *cbdata)
{
    QBuffer* pBuffer = new QBuffer;
    pBuffer->setData( (char *)pdata, nsize );
//    Phonon::MediaSource mediaSource( pBuffer );
//    mediaSource.setAutoDelete( true );
//    g_pMediaObject->setCurrentSource( mediaSource );
//    g_pMediaObject->play();
//    cb( cbdata, ABPAL_AudioState_Playing );
//    return PAL_Ok;
    return PAL_ErrUnsupported;
}

ABPAL_DEF PAL_Error
ABPAL_AudioPlayerPlayEx(
        ABPAL_AudioPlayer *player,
        ABPAL_AudioCombiner *combiner,
        ABPAL_AudioPlayerCallback *cb,
        void *cbdata)
{
    return PAL_ErrUnsupported;
}

ABPAL_DEF PAL_Error
ABPAL_AudioPlayerCancel(ABPAL_AudioPlayer *player)
{
//    g_pMediaObject->stop();
//    return PAL_Ok;
    return PAL_ErrUnsupported;
}

ABPAL_DEF ABPAL_AudioState
ABPAL_AudioPlayerGetState(ABPAL_AudioPlayer *player)
{
//    Phonon::State phState = g_pMediaObject->state();
    ABPAL_AudioState state = ABPAL_AudioState_Unknown;

//    switch( phState )
//    {
//    case Phonon::LoadingState:
//        state = ABPAL_AudioState_Init;
//        break;

//    case Phonon::StoppedState:
//        state = ABPAL_AudioState_Stopped;
//        break;

//    case Phonon::PlayingState:
//        state = ABPAL_AudioState_Playing;
//        break;

//    case Phonon::BufferingState:
//        state = ABPAL_AudioState_Recording;
//        break;

//    case Phonon::PausedState:
//        state = ABPAL_AudioState_Paused;
//        break;

//    case Phonon::ErrorState:
//        state = ABPAL_AudioState_Error;
//        break;
//    }

    return state;
}

ABPAL_DEF PAL_Error
ABPAL_AudioPlayerGetParameter(
        ABPAL_AudioPlayer* player,
        ABPAL_AudioParameter  audioParam,
        int32 *param1,
        int32 *param2)
{
    return PAL_Ok;
}

ABPAL_DEF PAL_Error
ABPAL_AudioPlayerSetParameter(
        ABPAL_AudioPlayer* player,
        ABPAL_AudioParameter  audioParam,
        int32 param1,
        int32 param2)
{
    return PAL_Ok;
}

ABPAL_DEF PAL_Error
ABPAL_AudioRecorderCreate(
        PAL_Instance* pal,
        ABPAL_AudioFormat format,
        ABPAL_AudioRecorder** recorder)
{
    *recorder = NULL;
    return PAL_Ok;
}

ABPAL_DEF PAL_Error
ABPAL_AudioRecorderDestroy(ABPAL_AudioRecorder* recorder)
{
    return PAL_Ok;
}

ABPAL_DEF PAL_Error
ABPAL_AudioRecorderRecord(
        ABPAL_AudioRecorder* recorder,
        byte** buffer,
        int bufferSize,
        ABPAL_AudioRecorderCallback* callback,
        void* userData)
{
    return PAL_Ok;
}

ABPAL_DEF PAL_Error
ABPAL_AudioRecorderStop(ABPAL_AudioRecorder* recorder)
{
    return PAL_Ok;
}

ABPAL_DEF ABPAL_AudioState
ABPAL_AudioRecorderGetState(ABPAL_AudioRecorder* recorder)
{
    ABPAL_AudioState state = ABPAL_AudioState_Unknown;
    return state;
}

ABPAL_DEF PAL_Error
ABPAL_AudioGetEstimatedDuration(byte* buf, uint32 bufSize, double* duration)
{
    // @TODO Add other formats, maybe new API will be required for it in order to provide
    // ABPAL_AudioFormat format or ABPAL_AudioPlayer
    if (!duration)
    {
        return PAL_ErrBadParam;
    }
    *duration = pal::audio::AACUtil::GetDuration(buf, bufSize);
    if (*duration == -1.0)
    {
        return PAL_ErrFileNotExist;
    }
    else
    {
        return PAL_Ok;
    }
}

