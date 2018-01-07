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
 * qcpplayer.h: created 2005/01/13 by Mark Goddard.
 */

#ifndef QCPPLAYER_H
#define QCPPLAYER_H

#include <AEE.h>
#include <AEEMedia.h>
#include <AEESound.h>
#include "palerror.h"
#include "paltypes.h"
#include "abpalaudio.h"

typedef struct _abpal_qcp_player 
{
	IMedia*	pMediaQCP;
	IShell* shell;
	void* user;

	boolean takeover;

	byte*	pbuffer;	
	uint32	buffersize;
	uint16	volume;
	
	ABPAL_AudioState state;
	
	int		mediaError;

    ABPAL_AudioPlayerCallback* userCallback;
	void*					 userCallbackData;
}abpal_qcp_player ;

abpal_qcp_player*	abpal_qcp_player_create(IShell* shell, void* user);
PAL_Error	abpal_qcp_player_init(abpal_qcp_player* qp);
PAL_Error	abpal_qcp_player_destroy(abpal_qcp_player* qp);
PAL_Error	abpal_qcp_player_stop(abpal_qcp_player* qp);
PAL_Error	abpal_qcp_player_set_user_cb(abpal_qcp_player* qp, ABPAL_AudioPlayerCallback* userCallback, void* serCallbackData);
PAL_Error	abpal_qcp_player_play(abpal_qcp_player* qp, byte* pdata, uint32 nsize, int16 vol, boolean takeover, AEESoundDevice audio_device);
PAL_Error   abpal_qcp_player_set_volume(abpal_qcp_player* qp, uint16 volume);

#endif
