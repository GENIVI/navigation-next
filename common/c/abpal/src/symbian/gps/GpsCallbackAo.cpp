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

#include "palstdlib.h"
#include "GpsCallbackAo.h"

CGpsCallbackAo* CGpsCallbackAo::NewL(ABPAL_GpsLocationCallback* aCallback, const TAny* aUserData)
	{
	CGpsCallbackAo* self = new (ELeave) CGpsCallbackAo(aCallback, aUserData);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

CGpsCallbackAo::CGpsCallbackAo(ABPAL_GpsLocationCallback* aCallback, const TAny* aUserData) : CActive(EPriorityUserInput)
	{
	iCallback = aCallback;
	iUserData = aUserData;
	CActiveScheduler::Add(this);
	}

CGpsCallbackAo::~CGpsCallbackAo()
	{
	Cancel();
	}

void CGpsCallbackAo::ConstructL()
	{
	}

void CGpsCallbackAo::UpdateResult(PAL_Error aCode, ABPAL_GpsLocation* aLocation)
	{
	iCode = aCode;
	if (aLocation != NULL)
		{
		nsl_memcpy(&iLocation, aLocation, sizeof(iLocation));
		}
	CompleteSelf(KErrNone);
	}

void CGpsCallbackAo::RunL()
	{
	if (iCallback != NULL)
		{
		if (iCode != PAL_Ok)
			{
			iCallback(iUserData, iCode, NULL, NULL);
			}
		else
			{
			iCallback(iUserData, PAL_Ok, &iLocation, NULL);
			}
		}
	}

void CGpsCallbackAo::DoCancel()
	{
	}

void CGpsCallbackAo::CompleteSelf(TInt aCode)
	{
	if (!IsActive())
		{
		iStatus = KRequestPending;
		SetActive();
		TRequestStatus* status = &iStatus;
		User::RequestComplete(status, aCode);
		}
	}
