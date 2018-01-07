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

#include "abpalaudio.h"
#include "palerror.h"
#include "palstdlib.h"
#include "abpal_qcp_player.h"
#include "abpal_qcp_combine.h"
#include "brewutil.h"


struct ABPAL_AudioCombiner
{
	abpal_qcp_combine* audiocombiner;
};

struct ABPAL_AudioRecorder
{
	void* audiorecorder;
};

struct ABPAL_AudioPlayer
{
	byte* buffer;
	int size;
	ABPAL_AudioState state;
	abpal_qcp_player* audioplayer;
	int16 volume;
	AEESoundDevice audio_device;
    ABPAL_AudioPlayerCallback *callback;
    void *callbackData;
};

PAL_Error
ABPAL_AudioCombinerCreate(PAL_Instance *pal, ABPAL_AudioFormat format, ABPAL_AudioCombiner **ppac)
{
	PAL_Error err = PAL_Ok;
	ABPAL_AudioCombiner* ac = NULL;

	ac = (ABPAL_AudioCombiner*) nsl_malloc(sizeof(ABPAL_AudioCombiner));

    if (!ac)
    {
        return PAL_ErrNoMem;
    }

	ac->audiocombiner = abpal_qcp_combine_create();
    if(ac->audiocombiner)
    {
        ac->audiocombiner->audio_format = format;
    }
    else
	{
		nsl_free(ac);
		return PAL_Failed;
	}

	(*ppac) = (ABPAL_AudioCombiner*) ac;

	return err;
}

PAL_Error
ABPAL_AudioCombinerDestroy(ABPAL_AudioCombiner *combiner)
{
	if (combiner)
	{
		if (combiner->audiocombiner)
        {
			abpal_qcp_combine_destroy(combiner->audiocombiner);
			nsl_free(combiner->audiocombiner);
        }
		nsl_free(combiner);
		return PAL_Ok;
	}
    return PAL_Failed;
}

PAL_Error
ABPAL_AudioCombinerAddBuffer(ABPAL_AudioCombiner *combiner, byte *buf, int size, boolean copy_needed)
{
	PAL_Error err = PAL_Ok;

	if (!combiner)
		return PAL_Failed;

	abpal_qcp_combine_add( combiner->audiocombiner, buf, size );

	return err;
}

PAL_Error
ABPAL_AudioCombinerGetData(ABPAL_AudioCombiner* combiner, byte** ppdata, int* pnsize)
{
	PAL_Error err = PAL_Ok;
	abpal_qcp_combine_data_type type = getdata_qcp;

	if (!combiner)
		return PAL_Failed;

	if (combiner->audiocombiner->audio_format == ABPAL_AudioFormat_QCP)
		type = getdata_qcp;
	else if (combiner->audiocombiner->audio_format == ABPAL_AudioFormat_CMX)
		type = getdata_cmx;

	abpal_qcp_combine_getdata( combiner->audiocombiner, type, ppdata, (uint32*) pnsize );

	return err;
}

PAL_Error
ABPAL_AudioPlayerCreate(PAL_Instance *pal, ABPAL_AudioFormat format, ABPAL_AudioPlayer **ppap)
{
	PAL_Error err = PAL_Ok;
	ABPAL_AudioPlayer* ap=NULL;

	ap = (ABPAL_AudioPlayer*) nsl_malloc(sizeof(ABPAL_AudioPlayer));

    if (!ap)
    {
        return PAL_ErrNoMem;
    }

	ap->audioplayer = abpal_qcp_player_create( PAL_GetShell(pal), (void*) ap );
    ap->callback = NULL;
    ap->callbackData = NULL;

	if (!ap->audioplayer)
	{
		nsl_free(ap);
		return PAL_Failed;
	}

	if ( abpal_qcp_player_init(ap->audioplayer) != PAL_Ok )
		return PAL_Failed;

	if (err == PAL_Ok)
			ap->state = ABPAL_AudioState_Init;

    ap->audio_device = AEE_SOUND_DEVICE_UNKNOWN;
    ap->volume = 100;

	(*ppap) = (ABPAL_AudioPlayer*) ap;

	return err;
}

PAL_Error
ABPAL_AudioPlayerDestroy(ABPAL_AudioPlayer *player)
{
	if (player)
	{
		if (player->audioplayer)
        {
			abpal_qcp_player_destroy(player->audioplayer);
			nsl_free(player->audioplayer);
			player->state = ABPAL_AudioState_Unknown;
        }
		nsl_free(player);
		return PAL_Ok;
	}
    return PAL_Failed;
}

static void AudioPlayerCallback(void* userData, ABPAL_AudioState state)
{
    ABPAL_AudioPlayer* player = (ABPAL_AudioPlayer*) userData;

    if (player)
    {
        player->state = state;
        if (player->callback)
        {
            player->callback( player->callbackData, state );
        }        
    }
}

PAL_Error
ABPAL_AudioPlayerPlay(ABPAL_AudioPlayer *player,  byte* pdata, int nsize, boolean takeover, ABPAL_AudioPlayerCallback *cb, void *cbdata)
{
	PAL_Error err = PAL_Ok;

	if (player == NULL || player->audioplayer == NULL)
		return PAL_ErrBadParam;

    player->callback = cb;
    player->callbackData = cbdata;
    abpal_qcp_player_set_user_cb(player->audioplayer, AudioPlayerCallback, player);

    if (player->state == ABPAL_AudioState_Playing)
        return PAL_ErrAudioBusy;

	err = abpal_qcp_player_play(player->audioplayer, pdata, nsize, player->volume, takeover, player->audio_device); 

	if (err == PAL_Ok)
		player->state = ABPAL_AudioState_Playing;

	return err;
}

PAL_Error
ABPAL_AudioPlayerPlayEx(ABPAL_AudioPlayer *player, ABPAL_AudioCombiner *combiner, ABPAL_AudioPlayerCallback *cb, void *cbdata)
{
	PAL_Error err = PAL_Ok;
	return err;
}

PAL_Error
ABPAL_AudioPlayerCancel(ABPAL_AudioPlayer *player)
{
	PAL_Error err = PAL_Ok;

	if ( !player || !player->audioplayer )
		return PAL_Failed;

	err = abpal_qcp_player_stop(player->audioplayer);

	if (err == PAL_Ok)
		player->state = ABPAL_AudioState_Cancel;

	return err;
}

ABPAL_AudioState
ABPAL_AudioPlayerGetState(ABPAL_AudioPlayer *player)
{
	return player->state;
}

PAL_Error
ABPAL_AudioPlayerSetParameter(ABPAL_AudioPlayer* player, ABPAL_AudioParameter  audioParam, int32 param1, int32 param2)
{
    PAL_Error err = PAL_Ok;

	if (!player)
		return PAL_Failed;

    if (audioParam == ABPAL_AudioParameter_Volume)
    {
	    player->volume = param1;
        abpal_qcp_player_set_volume(player->audioplayer, player->volume);
    }

    if (audioParam == ABPAL_AudioParameter_SoundDevice)
    {
	    player->audio_device = param2;
    }

	return err;
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

PAL_Error
ABPAL_AudioRecorderPrepareToRecord(ABPAL_AudioRecorder* recorder)
{
    return PAL_ErrUnsupported;
}

PAL_Error
ABPAL_AudioRecorderSetParameter(ABPAL_AudioRecorder* recorder, ABPAL_AudioParameter audioParam, int32 param1, int32 param2)
{
    return PAL_ErrUnsupported;
}

