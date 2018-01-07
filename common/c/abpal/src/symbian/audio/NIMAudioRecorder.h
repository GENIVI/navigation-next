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

#ifndef NIMAUDIORECORDER_H_
#define NIMAUDIORECORDER_H_

#include <e32base.h>
#include <MdaAudioSampleEditor.h>
#include "abpalaudio.h"

NONSHARABLE_CLASS(CNIMAudioRecorder) : public CBase, public MMdaObjectStateChangeObserver
	{
public:
	static CNIMAudioRecorder* NewL(ABPAL_AudioFormat& aFormat);
	static CNIMAudioRecorder* NewLC(ABPAL_AudioFormat& aFormat);
	virtual ~CNIMAudioRecorder();

public:
	PAL_Error RecordL(byte** aBuffer, TInt aBufferMaxLength, ABPAL_AudioRecorderCallback* aCallback, void* aUserData);
	void Stop();
	ABPAL_AudioState State() const;

private:
	CNIMAudioRecorder(ABPAL_AudioFormat& aFormat);
	void ConstructL();

private:
	// From MMdaObjectStateChangeObserver
	virtual void MoscoStateChangeEvent(CBase* aObject, TInt aPreviousState, TInt aCurrentState, TInt aErrorCode);

private:
	void SetState(ABPAL_AudioState aState);
	void InitFinishedL();
	void Report(TInt aError);

private:
	ABPAL_AudioFormat				iFormat;
	TPtr8							iBuf;
	ABPAL_AudioState				iState;
	ABPAL_AudioRecorderCallback*	iCallback;
	void*							iUserData;
	CMdaAudioRecorderUtility*		iRecorder;
	TMdaDesClipLocation				iAudioLocation;
	TMdaAudioDataSettings			iAudioSettings;
	TMdaWavClipFormat				iAudioFormat;
	TMdaPcmWavCodec					iAudioCode;
	};

#endif // NIMAUDIORECORDER_H_
