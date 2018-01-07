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

#include "Timer.h"

const TInt64 KMaxMicroSecondsUsedForAfter	= 2147483647;		// From RTimer in sdk.
const TInt KOneSecond = 1000000;
const TInt KOneMilliSecond = 1000;

CNimTimer* CNimTimer::NewL(MNimTimerObserver& aObserver)
	{
	CNimTimer* self = new(ELeave) CNimTimer(aObserver);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

CNimTimer::CNimTimer(MNimTimerObserver& aObserver)
			: CActive(CActive::EPriorityUserInput), iObserver(aObserver)
	{
	CActiveScheduler::Add(this);
	}

CNimTimer::~CNimTimer()
	{
	Cancel();
	iTimer.Close();
	}

void CNimTimer::ConstructL()
	{
	User::LeaveIfError(iTimer.CreateLocal());
	}

void CNimTimer::Start(TInt aSeconds)
	{
	TInt64 milliSeconds = aSeconds;
	milliSeconds *= KOneMilliSecond;
	StartByMilliSecond(milliSeconds);
	}

void CNimTimer::StartByMilliSecond(TInt64 aMilliSeconds)
	{
	if (!IsActive())
		{
		//The TTimeIntervalMicroSeconds32 is between +-35 minutes, 47 seconds
		//so RTimer::After(TRequestStatus &aStatus, TTimeIntervalMicroSeconds32 anInterval) may out of range.
		//then use RTimer::At(TTime)
		if ((aMilliSeconds * KOneMilliSecond) >= KMaxMicroSecondsUsedForAfter)
			{
			iTime.HomeTime();
			iTime += TTimeIntervalSeconds(aMilliSeconds / KOneMilliSecond);
			iTimer.At(iStatus, iTime);
			SetActive();
			}
		else
			{
			iInterval = aMilliSeconds * KOneMilliSecond;
			iTimer.After(iStatus, iInterval);
			SetActive();
			}
		}
	}

void CNimTimer::DoCancel()
	{
	iObserver.TimerCanceled(this);
	iTimer.Cancel();
	}

void CNimTimer::RunL()
	{
	switch (iStatus.Int ())
		{
		case KErrAbort:
			iTimer.At (iStatus, iTime);
			SetActive ();
			break;
		default:
			iObserver.TimeExpiredL (this);
			break;
		}
	}

TInt CNimTimer::RunError(TInt /*aError*/)
	{
	return KErrNone;
	}
