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

#ifndef LBSHANDLER_H
#define LBSHANDLER_H

#include <e32base.h>
#include <lbs.h>
#include <lbssatellite.h>
#include "pal.h"

#include "LbsHandlerBase.h"
#include "Timer.h"
#include "GpsSingleton.h"

class CNIMGpsConfiguration;
class CGpsCallbackAo;

NONSHARABLE_CLASS(CLbsHandler) : public CActive, public MLbsHandlerBase
	{
public:
	static CLbsHandler* NewL(PAL_Instance& aPal);
	static CLbsHandler* NewLC(PAL_Instance& aPal);
	virtual ~CLbsHandler();

public:
	// From MLbsHandlerBase
	virtual TInt Initial(const CNIMGpsConfiguration& aConfig, ABPAL_GpsInitializeCallback* aInitCallback, ABPAL_GpsDeviceStateCallback* aStateCallback, const void* aUserData);
	virtual ABPAL_GpsState HandlerState() const;
	virtual TInt GetLocation(const ABPAL_GpsCriteria& aCriteria, ABPAL_GpsLocationCallback* aCallback, const void* aUserData);
	virtual void CancelGetLocation();
	virtual TInt BeginTracking(const ABPAL_GpsCriteria& aCriteria, ABPAL_GpsLocationCallback* aCallback, const void* aUserData, ABPAL_GpsTrackingInfo& aTrackingInfo);
	virtual void EndTracking(const ABPAL_GpsTrackingInfo& aTrackingInfo);
	virtual TInt UpdateCriteria(const ABPAL_GpsCriteria& aCriteria, const ABPAL_GpsTrackingInfo& aTrackingInfo);
	virtual void Suspend(const ABPAL_GpsTrackingInfo& aTrackingInfo);
	virtual void Resume(const ABPAL_GpsTrackingInfo& aTrackingInfo);
	virtual void Release();

private:
	// From CActive
	virtual void RunL();
	virtual void DoCancel();
	virtual TInt RunError(TInt aError);

private:
	CLbsHandler(PAL_Instance& aPal, CGpsSingleton& aGpsSingleton);
	void ConstructL();

private:
	TPositionModuleId FindFirstNonNetworkAssistModuleL() const;
	void GetLocationL(const ABPAL_GpsCriteria& aCriteria, ABPAL_GpsLocationCallback* aCallback, const void* aUserData);
	void BeginTrackingL(const ABPAL_GpsCriteria& aCriteria, ABPAL_GpsLocationCallback* aCallback, const void* aUserData, ABPAL_GpsTrackingInfo& aTrackingInfo);
	TBool VerifyTrackingInfo(const ABPAL_GpsTrackingInfo& aTrackingInfo) const;
	TBool NeedRetry();
	void Retry();
	void SetHandlerState(ABPAL_GpsState aState);
	void UpdateGpsCriteria(const ABPAL_GpsCriteria& aCriteria);
	TBool IsValidResult();
	ABPAL_GpsLocation ConvertPositionInfo(const TPositionInfo& aPositionInfo);
	ABPAL_GpsLocation ConvertPositionCourseInfo(const TPositionCourseInfo& aPositionInfo);
	ABPAL_GpsLocation ConvertSatelliteInfo(const TPositionSatelliteInfo& aSatelliteInfo);
	void Convert(ABPAL_GpsLocation& abLocation, TPosition& aPosition);
	void CompleteSelf(TInt aError);
	void ReportError(TInt aError);

	void ReportLocation();
	void ContinueOrRelease();
	void TrakcingForNextPosition();

private:
	static TInt InitIdleCallback(TAny* aPtr);

private:
	PAL_Instance&					iPal;
	CGpsSingleton&					iGpsSingleton;
	RPositionServer&				iPositionServer;
	RPositioner						iPositioner;
	TBool							iPositionOpened;
	TPositionUpdateOptions			iUpdateops;

	ABPAL_GpsInitializeCallback*	iInitCallback;
	ABPAL_GpsDeviceStateCallback*	iDeviceStateCallback;
	ABPAL_GpsTrackingInfo			iTrackingInfo;
	ABPAL_GpsState					iHandlerState;
	ABPAL_GpsLocationCallback*		iLocationCallback;
	const void*						iInitUserData;
	const void*						iUserData;

	ABPAL_GpsCriteria				iCriteria;

	TPositionInfo               	iPositionInfo;
//	TPositionCourseInfo				iPositionInfo;
//	TPositionSatelliteInfo      	iPositionInfo;
//	TPositionInfo					iPositionInfo;
	TPositionSatelliteInfo			iSatelliteInfo;
	TPositionInfoBase*				iPosInfoBase;
	TBool							iGettingLastknownPosition;

	TBool							iHasLastPosition;
	TPosition						iLastPosition;

	CIdle*							iIdle;
	TTime							iStartTime;
//	TTime							iTimeBeforeCallback;

	TBool							iKeepTracking;

	CGpsCallbackAo*					iCallbackAo;
	};

#endif	// LBSHANDLER_H
