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


#include "AACCombiner.h"
extern "C"
{
#include "paltypes.h"
#include "abpalaudio.h"
#include "paltimer.h"
#include "paltaskqueue.h"
#include "paltestlog.h"
#include <limits.h>
#include "palaudioimpl.h"
}

static void AudioPlayerUserCallbackInvoke(PAL_Instance* pal, void* userData);
static void AudioPlayerCallback(void* userData, ABPAL_AudioState state);


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
    PAL_Error err = PAL_Ok;

    if (!combiner)
    {
        return PAL_ErrBadParam;
    }

    err = NIMAudioCombiner_Destroy(combiner->pAudioCombiner);
    nsl_free(combiner);

    return err;
}

ABPAL_DEF PAL_Error
ABPAL_AudioCombinerAddBuffer(
        ABPAL_AudioCombiner *combiner,
        byte *buf,
        int size,
        nb_boolean copy_needed)
{
    if (!combiner || !buf || size <= 0)
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

typedef struct {
    PAL_Instance *pal;
    ABPAL_AudioPlayerCallback *cb;

    void *data;
} local_info;

ABPAL_DEF PAL_Error
ABPAL_AudioPlayerCreate(PAL_Instance *pal, ABPAL_AudioFormat format, ABPAL_AudioPlayer **c)
{
    if (!pal || !c)
        return PAL_ErrBadParam;
    if ((*c = static_cast<ABPAL_AudioPlayer *>(nsl_malloc(sizeof(**c)))) == NULL)
        return PAL_ErrNoMem;

    PAL_Error err = QNXPlayerCreate(&(*c)->pQnxPlayer);
    if(err!= PAL_Ok) {
        nsl_free(*c);
        return err;
    }
    (*c)->format = format;
    (*c)->currentState = ABPAL_AudioState_Init;
    (*c)->newState = ABPAL_AudioState_Unknown;
    (*c)->pal = pal;

    return PAL_Ok;
}

ABPAL_DEF PAL_Error
ABPAL_AudioPlayerDestroy(ABPAL_AudioPlayer *player)
{
    if (!player)
        return PAL_ErrBadParam;
    QNXPlayerDestroy(player->pQnxPlayer);
    nsl_free(player);
    return PAL_Ok;
}


ABPAL_DEF PAL_Error
ABPAL_AudioPlayerCancel(ABPAL_AudioPlayer *player)
{
    if (!player)
        return PAL_ErrBadParam;

    if(player->pQnxPlayer)
    {
        QNXPlayerStop(player->pQnxPlayer);
    }

    return PAL_Ok;
}

void AudioPlayerUserCallbackInvoke(PAL_Instance* pal, void* userData)
{
    ABPAL_AudioPlayer* player = (ABPAL_AudioPlayer*) userData;

    if (player)
    {
        player->currentState = player->newState;

        if (player->cb)
        {
        	CONLOG( PAL_LogLevelInfo, "ABCB: calling cb %p/%p", player->cb, player->cbdata );
            player->cb(player->cbdata, player->currentState);
        }
    }
}

void AudioPlayerCallback(void* userData, ABPAL_AudioState state)
{
    ABPAL_AudioPlayer* player = (ABPAL_AudioPlayer*) userData;

    if (player)
    {
        TaskId tid = 0;

        player->newState = state;

        PAL_EventTaskQueueAddWithPriority(player->pal, AudioPlayerUserCallbackInvoke, player, &tid, MAX_USER_TASK_PRIORITY);
    }
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
    PAL_Error err = PAL_Ok;

    if (!player || !pdata || nsize <= 0)
        return PAL_ErrBadParam;


    player->currentState = ABPAL_AudioState_Playing;

    player->cb = cb;
    player->cbdata = cbdata;

    err = QNXPlayerPlay(player->pQnxPlayer, pdata, (uint32)nsize, player->format, AudioPlayerCallback, player);
    if (err)
    {
        player->currentState = ABPAL_AudioState_Error;
    }

    if(cb)
    {
        cb(cbdata, player->currentState);
    }

    return err;
}

ABPAL_DEF PAL_Error
ABPAL_AudioPlayerPlayEx(
        ABPAL_AudioPlayer *player,
        ABPAL_AudioCombiner *combiner,
        ABPAL_AudioPlayerCallback *cb,
        void *cbdata)
{
    if (!player || !combiner)
        return PAL_ErrBadParam;
    return ABPAL_AudioPlayerPlayCombined(player, combiner, cb, cbdata);
}

ABPAL_DEF ABPAL_AudioState
ABPAL_AudioPlayerGetState(ABPAL_AudioPlayer *player)
{
    if (!player)
        return ABPAL_AudioState_Unknown;
    return player->currentState;
}

ABPAL_DEF PAL_Error
ABPAL_AudioPlayerGetParameter(
        ABPAL_AudioPlayer* player,
        ABPAL_AudioParameter audioParam,
        int32 *param1,
        int32 *param2)
{
    if (!player || !param1 || !player->pQnxPlayer)
    {
        return PAL_ErrBadParam;
    }

	switch (audioParam)
	{
	case ABPAL_AudioParameter_Volume:
		QNXPlayerGetVolume( player->pQnxPlayer, param1 );
		CONLOG( PAL_LogLevelInfo, "Get volume: %d", (int)*param1 );
		break;
	default:
		break;
	}

    return PAL_Ok;
}

ABPAL_DEF PAL_Error
ABPAL_AudioPlayerSetParameter(
        ABPAL_AudioPlayer* player,
        ABPAL_AudioParameter audioParam,
        int32 param1,
        int32 param2)
{
    if (!player || !player->pQnxPlayer)
    {
        return PAL_ErrBadParam;
    }
    switch (audioParam)
    {
    case ABPAL_AudioParameter_Volume:
        QNXPlayerSetVolume(player->pQnxPlayer, param1);
        break;
    case ABPAL_AudioParameter_Output:
        QNXPlayerSetOutput(player->pQnxPlayer, (int32*)param1);
        break;
    default:
        // Just ignore unknown parameter
        break;
    }
    return PAL_Ok;
}

ABPAL_DEF PAL_Error
ABPAL_AudioRecorderCreate(
        PAL_Instance* pal,
        ABPAL_AudioFormat format,
        ABPAL_AudioRecorder** recorder)
{
    if (!pal || !recorder)
        return PAL_ErrBadParam;
    if ((*recorder = static_cast<ABPAL_AudioRecorder*>(nsl_malloc(sizeof(**recorder)))) == NULL)
        return PAL_ErrNoMem;
    (*recorder)->format = format;
    (*recorder)->level = ABPAL_AudioVolumeLevel_Medium;
    (*recorder)->state = ABPAL_AudioState_Init;
    if (((*recorder)->other = nsl_malloc(sizeof(local_info))) == NULL) {
        nsl_free(*recorder);
        return PAL_ErrNoMem;
    }
    ((local_info *)(*recorder)->other)->pal = pal;
    return PAL_Ok;
}

ABPAL_DEF PAL_Error
ABPAL_AudioRecorderDestroy(ABPAL_AudioRecorder* recorder)
{
    if (!recorder)
        return PAL_ErrBadParam;
    nsl_free(((local_info *)(recorder->other)));
    nsl_free(recorder);
    return PAL_Ok;
}

static void record_timer_callback(
    PAL_Instance *pal,          /*!< caller-supplied reference to PAL structure */
    void *userData,               /*!< caller-supplied data reference for timer */
    PAL_TimerCBReason reason    /*!< caller-supplied reason for callback being called */
)
{
    if (!pal || !userData)
        return;

    switch (reason) {
    case PTCBR_TimerFired:
        ((ABPAL_AudioRecorder *) userData)->state = ABPAL_AudioState_Ended;
        break;
    case PTCBR_TimerCanceled:
        ((ABPAL_AudioRecorder *) userData)->state = ABPAL_AudioState_Stopped;
        break;
    default:
        ((ABPAL_AudioRecorder *) userData)->state = ABPAL_AudioState_Error;
        break;
    }
    if (((local_info *)(((ABPAL_AudioRecorder *) userData)->other))->cb)
        ((local_info *)(((ABPAL_AudioRecorder *) userData)->other))->cb(
            ((local_info *)(((ABPAL_AudioRecorder *) userData)->other))->data, ((ABPAL_AudioRecorder *) userData)->state);
}

ABPAL_DEF PAL_Error
ABPAL_AudioRecorderStop(ABPAL_AudioRecorder* recorder)
{
    if (!recorder)
        return PAL_ErrBadParam;
    if (recorder->state != ABPAL_AudioState_Recording)
        return PAL_Failed;
    if (PAL_TimerCancel(((local_info *)(recorder->other))->pal, record_timer_callback, recorder) != PAL_Ok)
        return PAL_Failed;
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
    PAL_Error err = PAL_Ok;
    if (!recorder || !buffer || bufferSize <= 0)
        return PAL_ErrBadParam;

    if (recorder->state == ABPAL_AudioState_Recording)
        if (ABPAL_AudioRecorderStop(recorder) != PAL_Ok)
            return PAL_Failed;

    ((local_info *) recorder->other)->cb = callback;
    ((local_info *) recorder->other)->data = userData;
    if (PAL_TimerSet(((local_info *)recorder->other)->pal, 2000, record_timer_callback, recorder) != PAL_Ok) {
        recorder->state = ABPAL_AudioState_Error;
        err = PAL_Failed;
    } else {
        recorder->state = ABPAL_AudioState_Recording;
    }

    return err;
}

ABPAL_DEF ABPAL_AudioState
ABPAL_AudioRecorderGetState(ABPAL_AudioRecorder* recorder)
{
    if (!recorder)
        return ABPAL_AudioState_Unknown;
    return recorder->state;
}

ABPAL_DEF PAL_Error
ABPAL_AudioPlayerPlayCombined(ABPAL_AudioPlayer* player, ABPAL_AudioCombiner* combiner, ABPAL_AudioPlayerCallback* callback, void* userData)
{
    if (!player || !combiner)
        return PAL_ErrBadParam;
    if (!player->pQnxPlayer)
    {
        return PAL_ErrBadParam;
    }
    byte    *pData=0;
    int        nSize=0;
    PAL_Error err = ABPAL_AudioCombinerGetData(combiner, &pData, &nSize);
    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "[ABPC]: Playing %d bytes, format %d, GetDataErr=%d", nSize, player->format, err);
    if (err == PAL_Ok)
    {
        ABPAL_AudioPlayerPlay(player, pData, (uint32)nSize, FALSE, callback, userData);

    }
    nsl_free(pData);
    return err;
}

ABPAL_DEF PAL_Error
ABPAL_AudioRecorderPrepareToRecord(ABPAL_AudioRecorder* recorder)
{
    return PAL_ErrUnsupported;
}

ABPAL_DEF PAL_Error
ABPAL_AudioRecorderSetParameter(ABPAL_AudioRecorder* recorder, ABPAL_AudioParameter audioParam, int32 param1, int32 param2)
{
    return PAL_ErrUnsupported;
}

ABPAL_DEF PAL_Error
ABPAL_AudioRecorderGetParameter(ABPAL_AudioRecorder* recorder, ABPAL_AudioParameter audioParam, int32* param1, int32* param2)
{
    return PAL_ErrUnsupported;
}

ABPAL_DEF PAL_Error
ABPAL_AudioRecorderGetRecordedData(ABPAL_AudioRecorder* recorder, byte** buffer, uint32* size)
{
    return PAL_ErrUnsupported;
}

ABPAL_DEF PAL_Error
ABPAL_AudioSessionCreate(PAL_Instance* pal, ABPAL_AudioSessionType audioSessionType, ABPAL_AudioSession** audioSession)
{
    return PAL_ErrUnsupported;
}

ABPAL_DEF PAL_Error
ABPAL_AudioSessionDestroy(ABPAL_AudioSession* audioSession)
{
    return PAL_ErrUnsupported;
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
