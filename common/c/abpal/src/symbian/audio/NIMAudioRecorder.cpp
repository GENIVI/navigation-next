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

#include <Resource.h>
#include "NIMAudioRecorder.h"

CNIMAudioRecorder* CNIMAudioRecorder::NewL(ABPAL_AudioFormat& aFormat)
	{
	CNIMAudioRecorder* self = CNIMAudioRecorder::NewLC(aFormat);
	CleanupStack::Pop(self);
	return self;
	}

CNIMAudioRecorder* CNIMAudioRecorder::NewLC(ABPAL_AudioFormat& aFormat)
	{
	CNIMAudioRecorder* self = new (ELeave) CNIMAudioRecorder(aFormat);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CNIMAudioRecorder::CNIMAudioRecorder(ABPAL_AudioFormat& aFormat) : iBuf(NULL, 0)
	{
	iFormat = aFormat;
	}

CNIMAudioRecorder::~CNIMAudioRecorder()
	{
	Stop();
	}

void CNIMAudioRecorder::ConstructL()
	{
	SetState(ABPAL_AudioState_Unknown);
	}

PAL_Error CNIMAudioRecorder::RecordL(byte** aBuffer, TInt aBufferMaxLength, ABPAL_AudioRecorderCallback* aCallback,
										void* aUserData)
	{
	PAL_Error result = PAL_Failed;
	ABPAL_AudioState state = State();
	if (state != ABPAL_AudioState_Unknown)
		{
		result = PAL_ErrAudioBusy;
		}
	else
		{
		delete iRecorder;
		iRecorder = NULL;
		iRecorder = CMdaAudioRecorderUtility::NewL(*this);
		iBuf.Set(*aBuffer, 0, aBufferMaxLength);

		// TODO: Not support format now.
		iAudioLocation = iBuf;
		iAudioCode.iBits = TMdaPcmWavCodec::E16BitPcm;
		iAudioSettings.iSampleRate = 8000;
		iAudioSettings.iChannels = 1;

		iRecorder->OpenL(&iAudioLocation, &iAudioFormat, &iAudioCode, &iAudioSettings);
		SetState(ABPAL_AudioState_Init);
		iCallback = aCallback;
		iUserData = aUserData;
		result = PAL_Ok;
		}
	return result;
	}

void CNIMAudioRecorder::Stop()
	{
	if (iRecorder != NULL)
		{
		iRecorder->Stop();
		iRecorder->Close();
		delete iRecorder;
		iRecorder = NULL;
		}
	}

ABPAL_AudioState CNIMAudioRecorder::State() const
	{
	return iState;
	}

void CNIMAudioRecorder::MoscoStateChangeEvent(CBase* aObject, TInt aPreviousState, TInt aCurrentState, TInt aErrorCode)
	{
	if (aErrorCode != KErrNone)
		{
		SetState(ABPAL_AudioState_Error);
		Report(aErrorCode);
		}
	else if (aObject == iRecorder)
		{
		switch (aCurrentState)
			{
			case CMdaAudioClipUtility::EOpen:
				{
				if (aPreviousState == CMdaAudioClipUtility::ENotReady)
					{
					TRAPD(err, InitFinishedL());
					if (err != KErrNone)
						{
						SetState(ABPAL_AudioState_Error);
						Report(err);
						}
					}
				else if ((aPreviousState == CMdaAudioClipUtility::ERecording) && (iState == ABPAL_AudioState_Recording))
					{
					SetState(ABPAL_AudioState_Ended);
					Report(KErrNone);
					}
				}
				break;
			default:
				break;
			}
		}
	}

void CNIMAudioRecorder::SetState(ABPAL_AudioState aState)
	{
	iState = aState;
	}

void CNIMAudioRecorder::InitFinishedL()
	{
	iRecorder->SetVolume(iRecorder->MaxVolume());
	iRecorder->SetGain(iRecorder->MaxGain());
	iRecorder->SetAudioDeviceMode(CMdaAudioRecorderUtility::ELocal);
	iRecorder->RecordL();
	SetState(ABPAL_AudioState_Recording);
	}

void CNIMAudioRecorder::Report(TInt /*aError*/)
	{
	Stop();
	if (iCallback != NULL)
		{
		iCallback(iUserData, State());
		}
	}
