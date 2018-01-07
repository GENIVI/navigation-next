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

/*****************************************************************/
/*                                                               */
/* (C) Copyright 2005 by Networks In Motion, Inc.                */
/*                                                               */
/* The information contained herein is confidential, proprietary */
/* to Networks In Motion, Inc., and considered a trade secret as */
/* defined in section 499C of the penal code of the State of     */
/* California. Use of this information by anyone other than      */
/* authorized employees of Networks In Motion is granted only    */
/* under a written non-disclosure agreement, expressly           */
/* prescribing the scope and manner of such use.                 */
/*                                                               */
/*****************************************************************/

/*-
 * qcpplayer.c: created 2005/01/13 by Mark Goddard.
 */

#include "AEEMediaUtil.h"
#include "abpal_qcp_player.h"
#include "palstdlib.h"
#include "abpalaudio.h"

static void update_user_state(abpal_qcp_player* qp, ABPAL_AudioState state);

static void
abpal_notify_qcp(void * pUser, AEEMediaCmdNotify * pCmdNotify)
{
    abpal_qcp_player* qp = (abpal_qcp_player*) pUser;
    boolean play_next = FALSE;
    int rv;

    switch (pCmdNotify->nCmd)
    {
        case MM_CMD_PLAY:
            if (pCmdNotify->nStatus == MM_STATUS_START)
            {
                if (qp->pMediaQCP != NULL)
                {
                    rv = IMEDIA_SetVolume(qp->pMediaQCP, qp->volume);
                }
            }

            else if (pCmdNotify->nStatus == MM_STATUS_DONE)
            {
                if (qp->pbuffer != NULL)
                {
                    nsl_free(qp->pbuffer);
                    qp->pbuffer = NULL;
                }

                if (qp->userCallback)
                {
                    qp->userCallback(qp->userCallbackData, ABPAL_AudioState_Ended);
                }

                play_next = TRUE;
            }
            else if (pCmdNotify->nStatus == MM_STATUS_ABORT)
            {
                if (qp->pbuffer != NULL)
                {
                    nsl_free(qp->pbuffer);
                    qp->pbuffer = NULL;
                }

                if (qp->userCallback)
                {
                    qp->userCallback(qp->userCallbackData, ABPAL_AudioState_Cancel);
                }

                play_next = TRUE;
            }
            break;
    }

    if (play_next)
    {
        abpal_qcp_player_stop(qp);
    }
}

abpal_qcp_player*
abpal_qcp_player_create(IShell* shell, void* user)
{
    abpal_qcp_player *qp = (abpal_qcp_player*) nsl_malloc(sizeof(abpal_qcp_player));

    if (!qp)
        return NULL;

    qp->shell = shell;
    qp->user = user;

    return qp;
}

PAL_Error
abpal_qcp_player_init(abpal_qcp_player* qp)
{
    if (qp == NULL)
        return PAL_Failed;

    qp->pMediaQCP = NULL;
    qp->takeover = FALSE;
    qp->pbuffer = NULL;
    qp->buffersize = 0;
    qp->mediaError = 0;

    return PAL_Ok;
}

PAL_Error
abpal_qcp_player_destroy(abpal_qcp_player* qp)
{
    abpal_qcp_player_stop(qp);
    return PAL_Ok;
}

PAL_Error
abpal_qcp_player_stop(abpal_qcp_player* qp)
{
    PAL_Error err = PAL_Ok;

    if (qp->pMediaQCP)
    {
        if (IMEDIA_Stop(qp->pMediaQCP) != SUCCESS)
            err = PAL_Failed;

        if (qp->pbuffer)
        {
            nsl_free(qp->pbuffer);
            qp->pbuffer = NULL;
        }

        qp->takeover = FALSE;
        qp->buffersize = 0;
        qp->mediaError = 0;

        IMEDIA_Release(qp->pMediaQCP);
        qp->pMediaQCP = NULL;
    }

    return err;
}

PAL_Error
abpal_qcp_player_play(abpal_qcp_player* qp, byte* pdata, uint32 nsize, int16 vol, boolean takeover, AEESoundDevice audio_device)
{
    AEEMediaData md;

    if (!qp || !pdata || !nsize)
    {
        return PAL_Failed;
    }

    qp->pMediaQCP = NULL;
    qp->takeover = takeover;

    if (qp->pbuffer != NULL)
    {
        abpal_qcp_player_stop(qp);
    }

    qp->pbuffer = (byte*) nsl_malloc(nsize);
    if (qp->pbuffer == NULL)
    {
        return PAL_ErrNoMem;
    }

    nsl_memcpy(qp->pbuffer, pdata, nsize);

    qp->buffersize = nsize;
    md.clsData = MMD_BUFFER;
    md.pData = qp->pbuffer;
    md.dwSize = qp->buffersize;

    if (SUCCESS != AEEMediaUtil_CreateMedia(qp->shell, &md, &qp->pMediaQCP))
    {
        qp->pMediaQCP = NULL;
        nsl_free(qp->pbuffer);
        qp->pbuffer = NULL;
        return PAL_Failed;
    }

    if (SUCCESS != IMEDIA_RegisterNotify(qp->pMediaQCP, abpal_notify_qcp, qp))
    {
        IMEDIA_Release(qp->pMediaQCP);
        qp->pMediaQCP = NULL;
        nsl_free(qp->pbuffer);
        qp->pbuffer = NULL;
        return PAL_Failed;
    }

    IMEDIA_SetVolume(qp->pMediaQCP, vol);
    IMEDIA_SetMuteCtl(qp->pMediaQCP, vol == 0);

    if ( audio_device != AEE_SOUND_DEVICE_UNKNOWN)
    {
        IMEDIA_SetAudioDevice (qp->pMediaQCP, audio_device);
    }

    if (IMEDIA_Play(qp->pMediaQCP) != SUCCESS)
    {
        return PAL_Failed;
    }

    return PAL_Ok;
}

PAL_Error
abpal_qcp_player_set_user_cb(abpal_qcp_player* qp, ABPAL_AudioPlayerCallback* userCallback, void* userCallbackData)
{
    if (!qp)
        return PAL_Failed;

    qp->userCallback = userCallback;
    qp->userCallbackData = userCallbackData;

    return PAL_Ok;
}

static void update_user_state(abpal_qcp_player* qp, ABPAL_AudioState state)
{
    abpal_qcp_player* user = (abpal_qcp_player*) qp->user;
    if (user)
    {
        user->state = state;
    }
}

PAL_Error
abpal_qcp_player_set_volume(abpal_qcp_player* qp, uint16 volume)
{
    if (!qp)
    {
        return PAL_Failed;
    }

    /* Commented the code as IMedia_SetVolume is returning error(-2) when tested on the device */
    qp->volume = volume;

    if (qp->pMediaQCP)
    {
        IMEDIA_SetVolume(qp->pMediaQCP, qp->volume);
        IMEDIA_SetMuteCtl(qp->pMediaQCP, qp->volume == 0);
    }

    return PAL_Ok;
}
