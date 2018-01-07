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
 
 @file     palaudio.h
 @date     09/01/2010
 @defgroup PAL Audio API
 
 @brief    Platform-independent Audio API
 
 */
/*
    See file description in header file.

    (C) Copyright 2011 by TeleCommunication Systems, Inc.
 
    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret
    as defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly prescribing
    the scope and manner of such use.
 
 ---------------------------------------------------------------------------*/

#include "paltypes.h"
#include "palaudioimpl.h"
#include "abpalaudio.h"
#include "palaudioplayer.h"
#include "nimaudiocombiner.h"
#include "palqueueaudiorecorder.h"
#include "palfile.h"
#include "palaudiosessionsingleton.h"

PAL_Error
ABPAL_AudioCombinerCreate(PAL_Instance* pal, ABPAL_AudioFormat format, ABPAL_AudioCombiner** combiner)
{
    PAL_Error err = PAL_Ok;

    if (!pal || !combiner)
    {
        return PAL_ErrBadParam;
    }
    *combiner = NULL;

    ABPAL_AudioCombiner* pThis = (ABPAL_AudioCombiner*)nsl_malloc(sizeof(*pThis));
    if (pThis)
    {
        err = NIMAudioCombiner_Create(format, &pThis->pAudioCombiner);
        if (!err)
        {
            *combiner = pThis;
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

PAL_Error
ABPAL_AudioCombinerDestroy(ABPAL_AudioCombiner* combiner)
{
    if (!combiner)
    {
        return PAL_ErrBadParam;
    }

    NIMAudioCombiner_Destroy(combiner->pAudioCombiner);
    nsl_free(combiner);

    return PAL_Ok;
}

PAL_Error
ABPAL_AudioCombinerAddBuffer(ABPAL_AudioCombiner* combiner, byte* buf, int size, nb_boolean copy_needed)
{
    if (!combiner || !buf)
    {
      return PAL_ErrBadParam;
    }

    return NIMAudioCombiner_AddData(combiner->pAudioCombiner, buf, size, copy_needed);
}

PAL_Error
ABPAL_AudioCombinerGetData(ABPAL_AudioCombiner* combiner, byte** ppdata, int* pnsize)
{
    if (!combiner || !ppdata || !pnsize)
    {
        return PAL_ErrBadParam;
    }

    return NIMAudioCombiner_GetData(combiner->pAudioCombiner, ppdata, pnsize);
}


PAL_Error
ABPAL_AudioPlayerCreate(PAL_Instance* pal, ABPAL_AudioFormat format, ABPAL_AudioPlayer** player)
{
    PAL_Error result = PAL_Ok;
    PalAudioPlayer* _player = [[PalAudioPlayer alloc] init];
    if (_player == nil) {
        result = PAL_ErrNoMem;
    }
    else {
        _player.m_AudioFormat = format;
        _player.m_palInstance = pal;
        *player = (ABPAL_AudioPlayer*)_player;
    }
    return result;
}

PAL_Error
ABPAL_AudioPlayerDestroy(ABPAL_AudioPlayer* player)
{
    if (player)
    {
        PalAudioPlayer* _player = (PalAudioPlayer*)player;
        [_player release];
    }
    return PAL_Ok;
}

PAL_Error
ABPAL_AudioPlayerPlay(ABPAL_AudioPlayer* player, byte* buffer, int bufferSize, nb_boolean takeOwnership, ABPAL_AudioPlayerCallback* callback, void* callbackData)
{
    PAL_Error result = PAL_Ok;
    PalAudioPlayer* _aqplayer = (PalAudioPlayer*)player;
    result = [_aqplayer addAudioBuffer:(const char*)buffer andSize:bufferSize];
    if (takeOwnership)
    {
        nsl_free(buffer);
        buffer = NULL;
    }
    if (result == PAL_Ok)
    {
        _aqplayer.m_callback = callback;
        _aqplayer.m_callbackData = callbackData;
        result = [_aqplayer play];
    }
    return result;
}

PAL_Error
ABPAL_AudioPlayerPlayCombined(ABPAL_AudioPlayer* player, ABPAL_AudioCombiner* combiner,
                              ABPAL_AudioPlayerCallback* callback, void* userData)
{
    if (!player || !combiner)
        return PAL_ErrBadParam;

    byte    *pData=0;
    int        nSize=0;
    PAL_Error err = ABPAL_AudioCombinerGetData(combiner, &pData, &nSize);
    if (err == PAL_Ok)
    {
        ABPAL_AudioPlayerPlay(player, pData, (uint32)nSize, FALSE, callback, userData);

    }
    nsl_free(pData);
    return err;
}

PAL_Error
ABPAL_AudioPlayerCancel(ABPAL_AudioPlayer* player)
{
    PAL_Error error = PAL_Ok;

    if (player)
    {
        PalAudioPlayer* _aqplayer = (PalAudioPlayer*)player;
        [_aqplayer stop];
    }
    else
    {
        error = PAL_ErrBadParam;
    }

    return error;
}

ABPAL_AudioState
ABPAL_AudioPlayerGetState(ABPAL_AudioPlayer* player)
{
    return ABPAL_AudioState_Unknown;
}

PAL_Error
ABPAL_AudioPlayerSetParameter(ABPAL_AudioPlayer* player, ABPAL_AudioParameter audioParam, int32 param1, int32 param2)
{
    PAL_Error err = PAL_Ok;

    if (!player)
    {
        return PAL_ErrBadParam;
    }

    // Cast
    PalAudioPlayer* _aqplayer = (PalAudioPlayer*)player;
    switch (audioParam)
    {
        case ABPAL_AudioParameter_Volume:
        {
            [_aqplayer setVolume:(ABPAL_AudioVolumeLevel)param1];
            break;
        }
        case ABPAL_AudioParameter_SoundDevice:
        {
            // the param1 equal 0 means use default device to play audio.
            [_aqplayer setUseDefaultAudioRoute:(param1 == 0)];
            break;
        }
        case ABPAL_AudioParameter_AudioSession:
        {
            err = [_aqplayer setAudioSessionType:(ABPAL_AudioSessionType)param1];
            break;
        }
        default:
            err = PAL_ErrUnsupported;
            break;
    }
    return err;
}

PAL_Error
ABPAL_AudioPlayerGetParameter(ABPAL_AudioPlayer* player, ABPAL_AudioParameter audioParam, int32 *param1, int32 *param2)
{
    PAL_Error err = PAL_ErrUnsupported;
    if (!player || !param1)
    {
        return PAL_ErrBadParam;
    }
    // Cast
    PalAudioPlayer* _aqplayer = (PalAudioPlayer*)player;
    switch (audioParam)
    {
        case ABPAL_AudioParameter_BT:
        {
            *param1 = (int32)[_aqplayer isAudioRouteBT];
            break;
        }
        default:
            err = PAL_ErrUnsupported;
            break;
    }

    return err;
}


PAL_Error
ABPAL_AudioRecorderCreate(PAL_Instance* pal, ABPAL_AudioFormat format, ABPAL_AudioRecorder** recorder)
{
    PAL_Error err = PAL_Ok;

    if (!pal || !recorder)
    {
        return PAL_ErrBadParam;
    }

    *recorder = NULL;

    ABPAL_AudioRecorder* pThis = (ABPAL_AudioRecorder*)nsl_malloc(sizeof(ABPAL_AudioRecorder));
    if (pThis)
    {
        pThis->pAudioRecorder = [[PalQueueAudioRecorder alloc] initWithPal:pal audioFormat:format];

        if (pThis->pAudioRecorder)
        {
            *recorder = pThis;
        }
        else
        {
            nsl_free(pThis);
            err = PAL_ErrNoMem;
        }
    }
    else
    {
        err = PAL_ErrNoMem;
    }

    return err;
}

PAL_Error
ABPAL_AudioRecorderDestroy(ABPAL_AudioRecorder* recorder)
{
    if (recorder != NULL)
    {
        ABPAL_AudioRecorderStop(recorder);
        [recorder->pAudioRecorder release];

        nsl_free(recorder);
    }

    return PAL_Ok;
}

PAL_Error
ABPAL_AudioRecorderRecord(ABPAL_AudioRecorder* recorder, byte**, int, ABPAL_AudioRecorderCallback* callback, void* userData)
{
    if (!recorder || !userData || !callback)
    {
        return PAL_ErrBadParam;
    }

    PAL_Error result = [recorder->pAudioRecorder startRecord:callback userData:userData];

    return result;
}

PAL_Error
ABPAL_AudioRecorderPrepareToRecord(ABPAL_AudioRecorder* recorder)
{
    if (!recorder)
    {
        return PAL_ErrBadParam;
    }

    PAL_Error result = [recorder->pAudioRecorder prepareToRecord];

    return result;
}

PAL_Error
ABPAL_AudioRecorderSetParameter(ABPAL_AudioRecorder* recorder, ABPAL_AudioParameter audioParam, int32 param1, int32)
{
    PAL_Error err = PAL_Ok;

    if (!recorder)
    {
        return PAL_ErrBadParam;
    }

    switch (audioParam)
    {
        case ABPAL_AudioParameter_AudioSession:
        {
            err = [recorder->pAudioRecorder setAudioSessionType:(ABPAL_AudioSessionType)param1];
            break;
        }

        case ABPAL_AudioParameter_StreamRecordingMode:
        {
            err = [recorder->pAudioRecorder setStreamRecordingModeWithBufferDuration:(uint32)param1];
            break;
        }

        case ABPAL_AudioParameter_SampleRate:
        {
            err = [recorder->pAudioRecorder setSampleRate:(uint32)param1];
            break;
        }

        case ABPAL_AudioParameter_BitsPerSample:
        {
            err = [recorder->pAudioRecorder setBitsPerSample:(uint32)param1];
            break;
        }

        default:
        {
            err = PAL_ErrUnsupported;
            break;
        }
    }

    return err;
}

PAL_Error
ABPAL_AudioRecorderGetParameter(ABPAL_AudioRecorder* recorder, ABPAL_AudioParameter audioParam, int32* param1, int32*)
{
    PAL_Error err = PAL_ErrUnsupported;

    if (!recorder || !param1)
    {
        return PAL_ErrBadParam;
    }

    switch (audioParam)
    {
        case ABPAL_AudioParameter_LoudnessLevel:
        {
            uint32 loudnessLevel = 0;
            err = [recorder->pAudioRecorder getLoudnessLevel:&loudnessLevel];

            if (err == PAL_Ok)
            {
                *param1 = (int32)loudnessLevel;
            }

            break;
        }

        case ABPAL_AudioParameter_SampleRate:
        {
            uint32 sampleRate = 0;
            err = [recorder->pAudioRecorder getSampleRate:&sampleRate];

            if (err == PAL_Ok)
            {
                *param1 = (int32)sampleRate;
            }

            break;
        }

        case ABPAL_AudioParameter_BitsPerSample:
        {
            uint32 bitsPerSample = 0;
            err = [recorder->pAudioRecorder getBitsPerSample:&bitsPerSample];

            if (err == PAL_Ok)
            {
                *param1 = (int32)bitsPerSample;
            }

            break;
        }

        default:
            break;
    }

    return err;
}

PAL_Error
ABPAL_AudioRecorderGetRecordedData(ABPAL_AudioRecorder* recorder, byte** buffer, uint32* size)
{
    if (!recorder || !buffer || !size)
    {
        return PAL_ErrBadParam;
    }

    PAL_Error err = [recorder->pAudioRecorder getRecordedBuffer:buffer bufferSize:size];
    return err;
}

PAL_Error
ABPAL_AudioRecorderStop(ABPAL_AudioRecorder* recorder)
{
    if (!recorder)
    {
       return PAL_ErrBadParam;
    }

    PAL_Error err = [recorder->pAudioRecorder stop];
    return err;
}

ABPAL_AudioState
ABPAL_AudioRecorderGetState(ABPAL_AudioRecorder* recorder)
{
    if (!recorder)
    {
        return ABPAL_AudioState_Unknown;
    }

    ABPAL_AudioState state = [recorder->pAudioRecorder state];
    return state;
}

PAL_Error
ABPAL_AudioSessionCreate(PAL_Instance* pal, ABPAL_AudioSessionType audioSessionType, ABPAL_AudioSession** audioSession)
{
    PAL_Error err = PAL_Ok;

    if (!pal || !audioSession)
    {
        return PAL_ErrBadParam;
    }

    PalAudioSessionManager* palAudioSessionManager = PalAudioSessionFactory::getInstance(pal);
    PalAudioSession* _audioSession = [palAudioSessionManager OpenAudioSession:audioSessionType andPlayer:nil];

    if (!_audioSession)
    {
        err = PAL_ErrNoMem;
    }
    else
    {
        *audioSession = (ABPAL_AudioSession*)_audioSession;
    }

    return err;
}

PAL_Error
ABPAL_AudioSessionDestroy(ABPAL_AudioSession* audioSession)
{
    if (audioSession)
    {
        PalAudioSessionManager* palAudioSessionManager = PalAudioSessionFactory::getInstance();
        [palAudioSessionManager CloseAudioSession:(PalAudioSession*)audioSession];
    }

    return PAL_Ok;
}

typedef struct
{
    byte* buf;
    uint32 bufSize;
} Soundbuf;

OSStatus
AudioFileReadProc(void *inClientData, SInt64 inPosition, UInt32 requestCount, void *buffer, UInt32 *actualCount)
{
    Soundbuf* buf = (Soundbuf*)inClientData;
    if (!buf || !buffer || !actualCount || !buf->buf)
    {
        return kAudioFileNotOpenError;
    }
    if ((inPosition+requestCount) < buf->bufSize)
    {
        nsl_memcpy(buffer, buf->buf + inPosition, requestCount);
        *actualCount = requestCount;
    }
    else
    {
        nsl_memcpy(buffer, buf->buf + inPosition, buf->bufSize - inPosition);
        *actualCount = buf->bufSize - inPosition;
    }
    return noErr;
}

SInt64 AudioFileGetSizeProc(void  *inClientData)
{
    Soundbuf* buf = (Soundbuf*)inClientData;
    return buf->bufSize;
}

PAL_Error
ABPAL_AudioGetEstimatedDuration(byte* buf, uint32 bufSize, double* duration)
{
    AudioFileID fileId = 0;
    Soundbuf bufData = {0};
    Float64 estimatedDuration = 0.0;
    UInt32 propertySize = 0;
    OSStatus result = noErr;

    if (!buf || !duration || bufSize == 0)
    {
        return PAL_ErrBadParam;
    }

    bufData.buf = buf;
    bufData.bufSize = bufSize;

    result = AudioFileOpenWithCallbacks(&bufData, AudioFileReadProc, NULL,
                               AudioFileGetSizeProc, NULL, 0, &fileId);

    if(noErr == result)
    {
        propertySize = sizeof(estimatedDuration);
        result = AudioFileGetProperty(fileId, kAudioFilePropertyEstimatedDuration,
                                  &propertySize, &estimatedDuration);
    }

    // Use default duration in half second
    if (result == noErr)
    {
        *duration = (double)estimatedDuration;
        return PAL_Ok;
    }
    return PAL_Failed;
}
