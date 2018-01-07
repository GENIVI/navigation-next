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
#include "PalUtility.h"
#include "NIMAudioCombiner.h"
#include "NIMAudioPlayer.h"
#include "NIMAudioRecorder.h"

static PAL_Error ABPAL_AudioPlayerCreateL(PAL_Instance* aPal, const ABPAL_AudioFormat& /*aFormat*/, ABPAL_AudioPlayer** aPlayer)
	{
	PAL_Error result = PAL_ErrBadParam;
	if ((aPal != NULL) && (aPlayer != NULL))
		{
		CNIMAudioPlayer** player = static_cast<CNIMAudioPlayer**>(static_cast<void*>(aPlayer));
		*player = CNIMAudioPlayer::NewL();
		result = PAL_Ok;
		}
	return result;
	}

ABPAL_DEF PAL_Error ABPAL_AudioPlayerCreate(PAL_Instance* pal, ABPAL_AudioFormat format, ABPAL_AudioPlayer** player)
	{
	PAL_Error result = PAL_Ok;
	TRAPD(error, result = ABPAL_AudioPlayerCreateL(pal, format, player));
	if (error != KErrNone)
		{
		result = PALUtility::ConvertError(PALUtility::EAudioPlayerErrorType, error);
		}
	return result;
	}

ABPAL_DEF PAL_Error ABPAL_AudioPlayerDestroy(ABPAL_AudioPlayer* player)
	{
	delete static_cast<CNIMAudioPlayer*>(static_cast<void*>(player));
	return PAL_Ok;
	}

static PAL_Error ABPAL_AudioPlayerPlayL(ABPAL_AudioPlayer* aPlayer, byte* aBuffer, int aBufferSize,
										uint8 aTakeOwnership, ABPAL_AudioPlayerCallback* aCallback,
										void* aUserData)
	{
	PAL_Error result = PAL_ErrBadParam;
	if ((aPlayer != NULL) && (aBuffer != NULL) && (aCallback != NULL) && (aUserData != NULL))
		{
		CNIMAudioPlayer* pPlayer = static_cast<CNIMAudioPlayer*>(static_cast<void*>(aPlayer));
		if (pPlayer->IsPlaying())
			{
			result = PAL_ErrAudioBusy;
			}
		else
			{
			ABPAL_AudioCombiner* combiner = NULL;
			result = ABPAL_AudioCombinerCreate(NULL, ABPAL_AudioFormat_AMR, &combiner);
			if (result == PAL_Ok)
				{
				result = ABPAL_AudioCombinerAddBuffer(combiner, aBuffer, aBufferSize, !aTakeOwnership);
				if (result == PAL_Ok)
					{
					pPlayer->PlayL(static_cast<CNimAudioCombiner*>(static_cast<void*>(combiner)), ETrue, aCallback, aUserData);
					result = PAL_Ok;
					}
				else
					{
					ABPAL_AudioCombinerDestroy(combiner);
					}
				}
			}
		}
	return result;    
	}

ABPAL_DEF PAL_Error ABPAL_AudioPlayerPlay(ABPAL_AudioPlayer* player, byte* buffer, int bufferSize,
											uint8 takeOwnership, ABPAL_AudioPlayerCallback* callback,
											void* userData)
	{
	PAL_Error result = PAL_Ok;
	TRAPD(error, result = ABPAL_AudioPlayerPlayL(player, buffer, bufferSize, takeOwnership, callback, userData));
	if (error != KErrNone)
		{
		result = PALUtility::ConvertError(PALUtility::EAudioPlayerErrorType, error);
		}
	return result;
	}

static PAL_Error ABPAL_AudioPlayerPlayCombinedL(ABPAL_AudioPlayer* aPlayer, ABPAL_AudioCombiner* aCombiner,
													ABPAL_AudioPlayerCallback* aCallback, void* aUserData)
	{
	PAL_Error result = PAL_ErrBadParam;
	if ((aPlayer != NULL) && (aCombiner != NULL) && (aCallback != NULL) && (aUserData != NULL))
		{
		CNIMAudioPlayer* pPlayer = static_cast<CNIMAudioPlayer*>(static_cast<void*>(aPlayer));
		if (pPlayer->IsPlaying())
			{
			result = PAL_ErrAudioBusy;
			}
		else
			{
			pPlayer->PlayL(static_cast<CNimAudioCombiner*>(static_cast<void*>(aCombiner)), EFalse, aCallback, aUserData);
			result = PAL_Ok;
			}
		}
	return result;
	}

ABPAL_DEF PAL_Error ABPAL_AudioPlayerPlayCombined(ABPAL_AudioPlayer* player, ABPAL_AudioCombiner* combiner,
													ABPAL_AudioPlayerCallback* callback, void* userData)
	{
	PAL_Error result = PAL_Ok;
	TRAPD(error, result = ABPAL_AudioPlayerPlayCombinedL(player, combiner, callback, userData));
	if (error != KErrNone)
		{
		result = PALUtility::ConvertError(PALUtility::EAudioPlayerErrorType, error);
		}
	return result;
	}

ABPAL_DEF PAL_Error ABPAL_AudioPlayerCancel(ABPAL_AudioPlayer* player)
	{
	PAL_Error result = PAL_ErrBadParam;
	if (player != NULL)
		{
		CNIMAudioPlayer* pPlayer = static_cast<CNIMAudioPlayer*>(static_cast<void*>(player));
		pPlayer->Cancel();
		result = PAL_Ok;
		}
	return result;
	}

ABPAL_DEF ABPAL_AudioState ABPAL_AudioPlayerGetState(ABPAL_AudioPlayer* player)
	{
	ABPAL_AudioState result = ABPAL_AudioState_Unknown;
	if (player != NULL)
		{
		CNIMAudioPlayer* pPlayer = static_cast<CNIMAudioPlayer*>(static_cast<void*>(player));
		result = pPlayer->State();
		}
	return result;
	}

ABPAL_DEF PAL_Error ABPAL_AudioPlayerSetParameter(ABPAL_AudioPlayer* /*player*/, ABPAL_AudioParameter /*audioParam*/,
													int32 /*param1*/, int32 /*param2*/)
	{
	// TODO:
	return PAL_ErrUnsupported;
	}

ABPAL_DEF PAL_Error ABPAL_AudioPlayerGetParameter(ABPAL_AudioPlayer* /*player*/, ABPAL_AudioParameter /*audioParam*/,
													int32* /*param1*/, int32* /*param2*/)
	{
	// TODO:
	return PAL_ErrUnsupported;
	}

static PAL_Error ABPAL_AudioRecorderCreateL(PAL_Instance* aPal, ABPAL_AudioFormat aFormat,
													ABPAL_AudioRecorder** aRecorder)
	{
	PAL_Error result = PAL_ErrBadParam;
	if ((aPal != NULL) && (aRecorder != NULL))
		{
		CNIMAudioRecorder** recorder = static_cast<CNIMAudioRecorder**>(static_cast<void*>(aRecorder));
		// TODO: Support format later.
		*recorder = CNIMAudioRecorder::NewL(aFormat);
		result = PAL_Ok;
		}
	return result;
	}

ABPAL_DEF PAL_Error ABPAL_AudioRecorderCreate(PAL_Instance* pal, ABPAL_AudioFormat format,
													ABPAL_AudioRecorder** recorder)
	{
	PAL_Error result = PAL_Ok;
	TRAPD(error, result = ABPAL_AudioRecorderCreateL(pal, format, recorder));
	if (error != KErrNone)
		{
		result = PALUtility::ConvertError(PALUtility::EAudioPlayerErrorType, error);
		}
	return result;
	}

ABPAL_DEF PAL_Error ABPAL_AudioRecorderDestroy(ABPAL_AudioRecorder* recorder)
	{
	delete static_cast<CNIMAudioRecorder*>(static_cast<void*>(recorder));
	return PAL_Ok;
	}

ABPAL_DEF PAL_Error ABPAL_AudioRecorderRecord(ABPAL_AudioRecorder* /*recorder*/, byte** /*buffer*/, int /*bufferSize*/,
												ABPAL_AudioRecorderCallback* /*callback*/, void* /*userData*/)
	{
	// TODO:
	return PAL_ErrUnsupported;
	}

ABPAL_DEF PAL_Error ABPAL_AudioRecorderStop(ABPAL_AudioRecorder* recorder)
	{
	PAL_Error result = PAL_ErrBadParam;
	if (recorder != NULL)
		{
		CNIMAudioRecorder* pRecorder = static_cast<CNIMAudioRecorder*>(static_cast<void*>(recorder));
		pRecorder->Stop();
		result = PAL_Ok;
		}
	return result;
	}

ABPAL_DEF ABPAL_AudioState ABPAL_AudioRecorderGetState(ABPAL_AudioRecorder* recorder)
	{
	ABPAL_AudioState result = ABPAL_AudioState_Unknown;
	if (recorder != NULL)
		{
		CNIMAudioRecorder* pPlayer = static_cast<CNIMAudioRecorder*>(static_cast<void*>(recorder));
		result = pPlayer->State();
		}
	return result;
	}

static PAL_Error ABPAL_AudioCombinerCreateL(PAL_Instance* aPal, ABPAL_AudioFormat /*aFormat*/, ABPAL_AudioCombiner** aCombiner)
	{
	PAL_Error result = PAL_ErrBadParam;
	if ((aPal != NULL) && (aCombiner != NULL))
		{
		CNimAudioCombiner** combiner = static_cast<CNimAudioCombiner**>(static_cast<void*>(aCombiner));
		*combiner = CNimAudioCombiner::NewL();
		result = PAL_Ok;
		}
	return result;
	}

ABPAL_DEF PAL_Error ABPAL_AudioCombinerCreate(PAL_Instance* pal, ABPAL_AudioFormat format, ABPAL_AudioCombiner** combiner)
	{
	PAL_Error result = PAL_Ok;
	TRAPD(error, result = ABPAL_AudioCombinerCreateL(pal, format, combiner));
	if (error != KErrNone)
		{
		result = PALUtility::ConvertError(PALUtility::EAudioCombinerErrorType, error);
		}
	return result;
	}

ABPAL_DEF PAL_Error ABPAL_AudioCombinerDestroy(ABPAL_AudioCombiner* combiner)
	{
	delete static_cast<CNimAudioCombiner*>(static_cast<void*>(combiner));
	return PAL_Ok;
	}

static PAL_Error ABPAL_AudioCombinerAddBufferL(ABPAL_AudioCombiner* aCombiner, byte* aBuffer,
													int aBufferSize, uint8 aCopyNeeded)
	{
	PAL_Error result = PAL_ErrBadParam;
	if (aCombiner != NULL)
		{
		CNimAudioCombiner* pCombiner = static_cast<CNimAudioCombiner*>(static_cast<void*>(aCombiner));
		pCombiner->AddBufferL(aBuffer, aBufferSize, aCopyNeeded);
		result = PAL_Ok;
		}
	return result;
	}

ABPAL_DEF PAL_Error ABPAL_AudioCombinerAddBuffer(ABPAL_AudioCombiner* combiner, byte* buffer,
													int bufferSize, uint8 copyNeeded)
	{
	PAL_Error result = PAL_Ok;
	TRAPD(error, result = ABPAL_AudioCombinerAddBufferL(combiner, buffer, bufferSize, copyNeeded));
	if (error != KErrNone)
		{
		result = PALUtility::ConvertError(PALUtility::EAudioCombinerErrorType, error);
		}
	return result;
	}

ABPAL_DEF PAL_Error ABPAL_AudioCombinerGetData(ABPAL_AudioCombiner* combiner, byte** buffer,
												int* bufferSize)
	{
	PAL_Error result = PAL_ErrBadParam;
	if ((combiner != NULL) && (buffer != NULL) && (bufferSize != NULL))
		{
		CNimAudioCombiner* pCombiner = static_cast<CNimAudioCombiner*>(static_cast<void*>(combiner));
		TPtr8 ptr = pCombiner->Data();
		*buffer = &ptr[0];
		*bufferSize = ptr.Length();
		result = PAL_Ok;
		}
	return result;
	}
