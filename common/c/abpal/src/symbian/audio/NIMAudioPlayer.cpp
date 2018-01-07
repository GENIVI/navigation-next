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

#include "NIMAudioPlayer.h"
#include "NimAudioCombiner.h"

CNIMAudioPlayer* CNIMAudioPlayer::NewL()
	{
	CNIMAudioPlayer* self = CNIMAudioPlayer::NewLC();
	CleanupStack::Pop(self); 
	return self;
	}

CNIMAudioPlayer* CNIMAudioPlayer::NewLC()
	{
	CNIMAudioPlayer* self = new (ELeave) CNIMAudioPlayer();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CNIMAudioPlayer::CNIMAudioPlayer()
	{
	}

CNIMAudioPlayer::~CNIMAudioPlayer()
	{
	Close();
	delete iPlayer;
	}

void CNIMAudioPlayer::ConstructL()
	{
	iPlayer = CMdaAudioPlayerUtility::NewL(*this);
	}

TBool CNIMAudioPlayer::IsPlaying()const
	{
	return (iState == ABPAL_AudioState_Init)||(iState == ABPAL_AudioState_Playing);
	}

void CNIMAudioPlayer::PlayL(CNimAudioCombiner* aCombiner, TBool aTakeover, ABPAL_AudioPlayerCallback* aCb, void* aCbData)
	{
	iPlayer->OpenDesL(aCombiner->Data());
	if (aTakeover)
		{
		iCombiner = aCombiner;
		}
	iCallback = aCb;
	iCallbackData = aCbData;
	iState = ABPAL_AudioState_Init;
	}

void CNIMAudioPlayer::MapcInitComplete(TInt aError, const TTimeIntervalMicroSeconds& /*aDuration*/)
	{
	if (aError != KErrNone) 
		{
		iState = ABPAL_AudioState_Error;
		iCallback(iCallbackData, iState);			
		Close();
		}
	else
		{
		iCallback(iCallbackData, iState);			
		iState = ABPAL_AudioState_Playing;
		iPlayer->Play();
		}
	}

void CNIMAudioPlayer::MapcPlayComplete(TInt aError)
	{
	if (KErrNone == aError)
		{
		iState = ABPAL_AudioState_Ended;	
		}
	else
		{
		iState = ABPAL_AudioState_Error;		
		}
	iCallback(iCallbackData, iState);			
	Close();
	}

void CNIMAudioPlayer::Close()
	{
	iPlayer->Close();
	delete iCombiner;
	iCombiner = NULL;
	iCallback = NULL;
	}

void CNIMAudioPlayer::Cancel()
	{
	iState = ABPAL_AudioState_Cancel;
	iPlayer->Stop();
	Close();
	}

ABPAL_AudioState CNIMAudioPlayer::State()const
	{
	return iState;
	}
