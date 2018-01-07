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

    @file palaudio.cpp
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.                

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of TeleCommunication Systems is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*!
    @addtogroup abpalaudiowinmobile
    @{
*/

#include "paltypes.h"
#include "palaudioimpl.h"
#include "abpalaudio.h"
#include "nimaudioplayer.h"
#include "nimaudiocombiner.h"
#include "nimaudiorecorder.h"
#include "audiobluetooth.h"


/*---------------------------------------------------------------------------*/
/* AB PAL AudioCombiner functions                                            */
/*---------------------------------------------------------------------------*/

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


/*---------------------------------------------------------------------------*/
/* AB PAL AudioPlayer functions                                              */
/*---------------------------------------------------------------------------*/

PAL_Error
ABPAL_AudioPlayerCreate(PAL_Instance* pal, ABPAL_AudioFormat format, ABPAL_AudioPlayer** player)
{
    PAL_Error err = PAL_Ok;

    if (!pal || !player)
    {
        return PAL_ErrBadParam;
    }
    *player = NULL;

    ABPAL_AudioPlayer* pThis = (ABPAL_AudioPlayer*)nsl_malloc(sizeof(*pThis));
    if (pThis)
    {
        pThis->pAudioPlayer = new NIMAudioPlayer(pal, format);
        if (pThis->pAudioPlayer)
        {
            *player = pThis;
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
ABPAL_AudioPlayerDestroy(ABPAL_AudioPlayer* player)
{
    if (!player)
    {
        return PAL_ErrBadParam;
    }

    delete player->pAudioPlayer;
    nsl_free(player);

    return PAL_Ok;
}

PAL_Error
ABPAL_AudioPlayerPlay(ABPAL_AudioPlayer* player, byte* buffer, int bufferSize, nb_boolean takeOwnership, ABPAL_AudioPlayerCallback* callback, void* callbackData)
{ 
    if (!player || !buffer)
    {
        return PAL_ErrBadParam;
    }

    return player->pAudioPlayer->Play(buffer, bufferSize, takeOwnership, callback, callbackData);
}

PAL_Error
ABPAL_AudioPlayerCancel(ABPAL_AudioPlayer* player)
{
    if (!player)
    {
        return PAL_ErrBadParam;
    }

    nb_boolean stopped = player->pAudioPlayer->Stop();
    if (!stopped)
    {
        return PAL_Failed;
    }

    return PAL_Ok;
}

ABPAL_AudioState
ABPAL_AudioPlayerGetState(ABPAL_AudioPlayer* player)
{
    if (!player)
    {
        return ABPAL_AudioState_Unknown;
    }

    return player->pAudioPlayer->GetState();
}

PAL_Error
ABPAL_AudioPlayerSetParameter(ABPAL_AudioPlayer* player, ABPAL_AudioParameter audioParam, int32 param1, int32 param2)
{
    PAL_Error err = PAL_Ok;
    
    if (!player)
    {
        return PAL_ErrBadParam;
    }
    
    switch (audioParam)
    {
        case ABPAL_AudioParameter_Volume:
            player->pAudioPlayer->SetVolumeLevel((ABPAL_AudioVolumeLevel)param1);
            break;
        case ABPAL_AudioParameter_BT:
            if (param1)
            {
                err = AudioBluetoothGatewayOpen();
            }
            else
            {
                err = AudioBluetoothGatewayClose();
            }
            break;
        case ABPAL_AudioParameter_BT_SpeakerVolume:
            err = AudioBluetoothSetSpeakerVolume((uint16)param1);
            break;
        case ABPAL_AudioParameter_BT_MicrophoneVolume:
            err = AudioBluetoothSetMicrophoneVolume((uint16)param1);
            break;
        default:
            err = PAL_ErrUnsupported;
            break;
    }

    return err;
}

PAL_Error
ABPAL_AudioPlayerGetParameter(ABPAL_AudioPlayer* player, ABPAL_AudioParameter audioParam, int32 *param1, int32 *param2)
{
    PAL_Error err = PAL_ErrAudioGeneral;
    uint16 value = 0;

    if (!player || !param1)
    {
        return PAL_ErrBadParam;
    }

    switch (audioParam)
    {
        case ABPAL_AudioParameter_Volume:
            *param1 = (int32)player->pAudioPlayer->GetVolumeLevel();
            err = PAL_Ok;
            break;
        case ABPAL_AudioParameter_BT_SpeakerVolume:
            err = AudioBluetoothGetSpeakerVolume(&value);
            if (!err)
            {
                *param1 = (int32)value;
            }
            break;
        case ABPAL_AudioParameter_BT_MicrophoneVolume:
            err = AudioBluetoothGetMicrophoneVolume(&value);
            if (!err)
            {
                *param1 = (int32)value;
            }
            break;
        default:
            err = PAL_ErrUnsupported;
    }
    
    return err;
}


/*---------------------------------------------------------------------------*/
/* AB PAL AudioRecorder functions                                            */
/*---------------------------------------------------------------------------*/

PAL_Error
ABPAL_AudioRecorderCreate(PAL_Instance* pal, ABPAL_AudioFormat format, ABPAL_AudioRecorder** recorder)
{
    PAL_Error err = PAL_Ok;

    if (!pal || !recorder)
    {
        return PAL_ErrBadParam;
    }
    *recorder = NULL;

    ABPAL_AudioRecorder* pThis = (ABPAL_AudioRecorder*)nsl_malloc(sizeof(*pThis));
    if (pThis)
    {
        pThis->pAudioRecorder = new NIMAudioRecorder(pal, format);
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
    if (!recorder)
    {
        return PAL_ErrBadParam;
    }

    ABPAL_AudioRecorderStop(recorder);
    delete recorder->pAudioRecorder;
    nsl_free(recorder);

    return PAL_Ok;
}

PAL_Error
ABPAL_AudioRecorderRecord(ABPAL_AudioRecorder* recorder, byte** buffer, int bufferSize, ABPAL_AudioRecorderCallback* callback, void* userData)
{
    if (!recorder || !buffer)
    {
        return PAL_ErrBadParam;
    }

    int result = recorder->pAudioRecorder->Record(buffer, bufferSize, callback, userData);
    if (result != 0)
    {
        return PAL_Failed;
    }

    return PAL_ErrUnsupported;
}

PAL_Error
ABPAL_AudioRecorderStop(ABPAL_AudioRecorder* recorder)
{
    if (!recorder)
    {
        return PAL_ErrBadParam;
    }

    nb_boolean stopped = recorder->pAudioRecorder->Stop();
    if (!stopped)
    {
        return PAL_Failed;
    }


    return PAL_Ok;
}

ABPAL_AudioState
ABPAL_AudioRecorderGetState(ABPAL_AudioRecorder* recorder)
{
    if (!recorder)
    {
        return ABPAL_AudioState_Unknown;
    }

    return recorder->pAudioRecorder->GetState();
}

PAL_Error
ABPAL_AudioRecorderGetRecordedData(
                                   ABPAL_AudioRecorder* recorder,        /*!< Pointer to the recorder returned by ABPAL_AudioRecorderCreate */
                                   byte**  buffer,                  /*!< recorded buffer data */
                                   uint32* size                           /*!< recorder data size when record stop*/
                                    )
{
    return PAL_ErrUnsupported;
}

PAL_Error
ABPAL_AudioSessionCreate(
                         PAL_Instance* pal,                          /*!< PAL instance */
                         ABPAL_AudioSessionType audioSessionType,    /*!< Ausio session to be setup */
                         ABPAL_AudioSession** audioSession           /*!< [OUT parameter] that will return a pointer to the ABPAL_AudioSession.
                                                                          The caller must pass this player to every ABPAL_AudioSessionXXXX API */
                         )
{
    return PAL_ErrUnsupported;
}

PAL_Error
ABPAL_AudioSessionDestroy(
                         ABPAL_AudioSession* audioSession            /*!< Pointer to the ABPAL_AudioSession returned on call to ABPAL_AudioSessionCreate */
                         )
{
    return PAL_ErrUnsupported;
}

/*! @} */
