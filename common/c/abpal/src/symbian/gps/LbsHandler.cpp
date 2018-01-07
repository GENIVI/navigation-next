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

#include "palclock.h"
#include "PALUtility.h"
#include "LbsHandler.h"
#include "palfunc.h"
#include "paldebuglog.h"
#include "GpsCallbackAo.h"

const TInt64 KThousand			=     1000;
const TInt64 KDefaultTimeOut	= 10000000;		// 10 seconds
const TInt64 KDefaultInterval	=  1000000;		// 1 second
const TInt64 KDefaultMaxAge		=   500000;		// 0.5 second

CLbsHandler* CLbsHandler::NewL(PAL_Instance& aPal)
	{
	CLbsHandler* self = CLbsHandler::NewLC(aPal);
	CleanupStack::Pop(self);
	return self;
	}

CLbsHandler* CLbsHandler::NewLC(PAL_Instance& aPal)
	{
	CGpsSingleton* gpsSingleton = static_cast<CGpsSingleton*>(PAL_GetGpsExtend(&aPal));
	CLbsHandler* self = new (ELeave) CLbsHandler(aPal, *gpsSingleton);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CLbsHandler::CLbsHandler(PAL_Instance& aPal, CGpsSingleton& aGpsSingleton)
			: CActive(EPriorityUserInput + 1), iPal(aPal), iGpsSingleton(aGpsSingleton), iPositionServer(aGpsSingleton.PositionServer())
	{
	SetHandlerState(PGS_Undefined);
	CActiveScheduler::Add(this);
	}

CLbsHandler::~CLbsHandler()
	{
	Cancel();
	delete iIdle;
	Release();
	if (iPositionOpened)
		{
		iPositioner.Close();
		iPositionOpened = EFalse;
		}
	delete iCallbackAo;
	iGpsSingleton.ReleasePositionServer();
	}

void CLbsHandler::ConstructL()
	{
	iGpsSingleton.ConnectPositionServerL();
	iIdle = CIdle::NewL(CActive::EPriorityIdle);
	}

TInt CLbsHandler::Initial(const CNIMGpsConfiguration& /*aConfig*/, ABPAL_GpsInitializeCallback* aInitCallback, ABPAL_GpsDeviceStateCallback* aStateCallback, const void* aUserData)
	{
	TInt ret = KErrGeneral;
	if (HandlerState() == PGS_Undefined)
		{
		iInitCallback = aInitCallback;
		iDeviceStateCallback = aStateCallback;
		iInitUserData = aUserData;
		if (!iIdle->IsActive())
			{
			iIdle->Start(TCallBack(InitIdleCallback, this));
			}
		SetHandlerState(PGS_InitializePending);
		ret = KErrNone;
		}
	return ret;
	}

ABPAL_GpsState CLbsHandler::HandlerState() const
	{
	return iHandlerState;
	}

TInt CLbsHandler::GetLocation(const ABPAL_GpsCriteria& aCriteria, ABPAL_GpsLocationCallback* aCallback, const void* aUserData)
	{
	TRAPD(ret, GetLocationL(aCriteria, aCallback, aUserData));
	return ret;
	}

void CLbsHandler::CancelGetLocation()
	{
	if (IsActive() && HandlerState() == PGS_Oneshot)
		{
		debugf("Cancel Get Location");
		Cancel();
		Release();
		}
	}

TInt CLbsHandler::BeginTracking(const ABPAL_GpsCriteria& aCriteria, ABPAL_GpsLocationCallback* aCallback, const void* aUserData, ABPAL_GpsTrackingInfo& aTrackingInfo)
	{
	TRAPD(ret, BeginTrackingL(aCriteria, aCallback, aUserData, aTrackingInfo));
	return ret;
	}

void CLbsHandler::EndTracking(const ABPAL_GpsTrackingInfo& aTrackingInfo)
	{
	if (iKeepTracking && VerifyTrackingInfo(aTrackingInfo) && (HandlerState() == PGS_Tracking))
		{
		if (IsActive())
			{
			debugf("End The Activing Tracking");
			Cancel();
			iIdle->Cancel();
			Release();
			}
		else
			{
			debugf("End The Stoped Tracking");
			iKeepTracking = EFalse;
			}
		}
	}

TInt CLbsHandler::UpdateCriteria(const ABPAL_GpsCriteria& aCriteria, const ABPAL_GpsTrackingInfo& aTrackingInfo)
	{
	TInt ret = KErrNotReady;
	if (VerifyTrackingInfo(aTrackingInfo) && iKeepTracking && (HandlerState() == PGS_Tracking))
		{
		debugf("Update Criteria");
		Cancel();
		iCriteria = aCriteria;
		UpdateGpsCriteria(iCriteria);
		ret = iPositioner.SetUpdateOptions(iUpdateops);
		}
	return ret;
	}

void CLbsHandler::Suspend(const ABPAL_GpsTrackingInfo& aTrackingInfo)
	{
	if (VerifyTrackingInfo(aTrackingInfo) && iKeepTracking && (HandlerState() == PGS_Tracking))
		{
		debugf("Suspend Tracking");
		Cancel();
		iKeepTracking = EFalse;
		SetHandlerState(PGS_Suspended);
		}
	}

void CLbsHandler::Resume(const ABPAL_GpsTrackingInfo& aTrackingInfo)
	{
	if ((!IsActive()) && VerifyTrackingInfo(aTrackingInfo) && (!iKeepTracking) && (HandlerState() == PGS_Suspended))
		{
		TInt ret = iPositioner.SetUpdateOptions(iUpdateops);
		if (ret == KErrNone)
			{
			debugf("Resume Tracking.");
			iPositioner.NotifyPositionUpdate(*iPosInfoBase, iStatus);
			SetActive();
			iKeepTracking = ETrue;
			SetHandlerState(PGS_Tracking);
			}
		else
			{
			ReportError(ret);
			}
		}
	}

void CLbsHandler::RunL()
	{
	debugf("CLbsHandler::RunL In with status = %d", iStatus.Int());
	switch (iStatus.Int())
		{
		case KErrNone:
		case KPositionPartialUpdate:
			{
			if (IsValidResult())
				{
				iStartTime.HomeTime();
//				iTimeBeforeCallback.HomeTime();
				ReportLocation();
				TrakcingForNextPosition();
				}
			else if (NeedRetry())
				{
				Retry();
				}
			else
				{
				ReportError(KErrTimedOut);
				}
			}
			break;

		case KErrArgument:
			{
			debugf("CLbsHandler::RunL change to use TPositionInfo.");
			iPosInfoBase = &iPositionInfo;
			if (NeedRetry())
				{
				Retry();
				}
			else
				{
				ReportError(iStatus.Int());
				}
			}
			break;

		case KPositionQualityLoss:
			{
			if (iGettingLastknownPosition)
				{
				iPosInfoBase = &iSatelliteInfo;
				}
			if (NeedRetry())
				{
				Retry();
				}
			else
				{
				ReportError(iStatus.Int());
				}
			}
			break;

		case KErrAccessDenied:
			{
			ReportError(iStatus.Int());
			Release();
			}
			break;

		case KErrTimedOut:
			{
			if (iGettingLastknownPosition)
				{
				iPosInfoBase = &iSatelliteInfo;
				}
			if (NeedRetry())
				{
				Retry();
				}
			else
				{
				ReportError(iStatus.Int());
				}
			}
			break;

		case KErrCancel:
			{
			ReportError(iStatus.Int());
			Release();
			}
			break;

		case KErrUnknown:
			{
			if (iGettingLastknownPosition)
				{
				iPosInfoBase = &iSatelliteInfo;
				iGettingLastknownPosition = EFalse;
				}
			if (NeedRetry())
				{
				Retry();
				}
			else
				{
				ReportError(iStatus.Int());
				}
			}
			break;

		case KErrServerTerminated:
			{
			ReportError(iStatus.Int());
			Release();
			}
			break;

		default:
			{
			ReportError(iStatus.Int());
			Release();
			}
			break;
		}

	if (iGettingLastknownPosition)
		{
		iGettingLastknownPosition = EFalse;
		}

	ContinueOrRelease();
	debugf("CLbsHandler::RunL out");
	}

void CLbsHandler::DoCancel()
	{
	// When a client application wishes to close one of its connections to Location
	// Server, there can be no outstanding requests on that particular connection
	// If a client application attempts to close a connection before outstanding
	// requests have been cancelled or completed, it is panicked

	//If we are getting the last known position
	if (iGettingLastknownPosition)
		{
		//Cancel the last known position request
		iPositioner.CancelRequest(EPositionerGetLastKnownPosition);
		}
	else
		{
		iPositioner.CancelRequest(EPositionerNotifyPositionUpdate);
		}
	}

TInt CLbsHandler::RunError(TInt aError)
	{
	ReportError(aError);
	Release();
	return KErrNone;
	}

TPositionModuleId CLbsHandler::FindFirstNonNetworkAssistModuleL() const
	{
	debugf("FindFirstNonNetworkAssistModuleL In");
	TPositionModuleId id = TUid::Uid(0);
	TUint count = 0;
	User::LeaveIfError(iPositionServer.GetNumModules(count));
	for (TUint i=0; i<count; ++i)
		{
		TPositionModuleInfo info;
		iPositionServer.GetModuleInfoByIndex(i, info);
		if (info.IsAvailable())
			{
			TPositionModuleInfo::TTechnologyType type = info.TechnologyType();
			if (type == TPositionModuleInfo::ETechnologyTerminal)
				{
				id = info.ModuleId();
				break;
				}
			}
		}

	if (id.iUid == 0)
		{
		User::Leave(KErrNotFound);
		}
	debugf("FindFirstNonNetworkAssistModuleL Out with the id: %d", id);
	return id;
	}

void CLbsHandler::GetLocationL(const ABPAL_GpsCriteria& aCriteria, ABPAL_GpsLocationCallback* aCallback, const void* aUserData)
	{
	debugf("CLbsHandler::GetLocationL In");
	if (HandlerState() != PGS_Initialized)
		{
		debugf("State = %d is not correct", HandlerState());
		User::Leave(KErrGeneral);
		}

	delete iCallbackAo;
	iCallbackAo = NULL;
	iCallbackAo = CGpsCallbackAo::NewL(aCallback, aUserData);

	debugf("Open positioner");
	if (!aCriteria.desiredNetworkAssist)
		{
		TPositionModuleId id = FindFirstNonNetworkAssistModuleL();
		User::LeaveIfError(iPositioner.Open(iPositionServer, id));
		}
	else
		{
		User::LeaveIfError(iPositioner.Open(iPositionServer));
		}
	iPositionOpened = ETrue;
	RProcess thisProcess;
	debugf("Set requestor.");
	User::LeaveIfError(iPositioner.SetRequestor(CRequestor::ERequestorService, CRequestor::EFormatApplication, thisProcess.FileName()));

	iCriteria = aCriteria;
	UpdateGpsCriteria(aCriteria);

	iStartTime.HomeTime();
	debugf("Set update options");
	User::LeaveIfError(iPositioner.SetUpdateOptions(iUpdateops));

	iPosInfoBase = &iPositionInfo;
	iPositioner.NotifyPositionUpdate(*iPosInfoBase, iStatus);
	SetActive();
	iLocationCallback = aCallback;
	iUserData = aUserData;
	SetHandlerState(PGS_Oneshot);
	debugf("CLbsHandler::GetLocationL out");
	}

void CLbsHandler::BeginTrackingL(const ABPAL_GpsCriteria& aCriteria, ABPAL_GpsLocationCallback* aCallback, const void* aUserData, ABPAL_GpsTrackingInfo& aTrackingInfo)
	{
	debugf("CLbsHandler::BeginTrackingL In");
	if (HandlerState() != PGS_Initialized)
		{
		debugf("State = %d is not correct", HandlerState());
		User::Leave(KErrGeneral);
		}

	delete iCallbackAo;
	iCallbackAo = NULL;
	iCallbackAo = CGpsCallbackAo::NewL(aCallback, aUserData);

	debugf("Open positioner.");
	if (!aCriteria.desiredNetworkAssist)
		{
		TPositionModuleId id = FindFirstNonNetworkAssistModuleL();
		User::LeaveIfError(iPositioner.Open(iPositionServer, id));
		}
	else
		{
		User::LeaveIfError(iPositioner.Open(iPositionServer));
		}
	iPositionOpened = ETrue;
	RProcess thisProcess;
	debugf("Set requestor");
	User::LeaveIfError(iPositioner.SetRequestor(CRequestor::ERequestorService, CRequestor::EFormatApplication, thisProcess.FileName()));

	iCriteria = aCriteria;
	UpdateGpsCriteria(aCriteria);

	iStartTime.HomeTime();
	debugf("Set update options");
	User::LeaveIfError(iPositioner.SetUpdateOptions(iUpdateops));

	iPosInfoBase = &iSatelliteInfo;
	iPositioner.NotifyPositionUpdate(*iPosInfoBase, iStatus);
	SetActive();
	aTrackingInfo.Id = 1;	// TODO:
	iTrackingInfo = aTrackingInfo;
	iLocationCallback = aCallback;
	iUserData = aUserData;
	SetHandlerState(PGS_Tracking);
	iKeepTracking = ETrue;
	debugf("CLbsHandler::BeginTrackingL out");
	}

TBool CLbsHandler::VerifyTrackingInfo(const ABPAL_GpsTrackingInfo& /*aTrackingInfo*/) const
	{
	return ETrue;
	}

TBool CLbsHandler::NeedRetry()
	{
	TBool ret = ETrue;

	TTime now;
	now.HomeTime();

	TTimeIntervalMicroSeconds microSeconds = now.MicroSecondsFrom(iStartTime);
	if ((microSeconds.Int64() / KThousand) >= iCriteria.desiredTimeout)
		{
		debugf("We don't need retry because time out.");
		ret = EFalse;
		}
	return ret;
	}

void CLbsHandler::Retry()
	{
	iPositioner.NotifyPositionUpdate(*iPosInfoBase, iStatus);
	SetActive();
	}

void CLbsHandler::Release()
	{
	if (iPositionOpened && HandlerState() != PGS_DestroyPending)
		{
		debugf("CLbsHandler::Release");
		iPositioner.CancelRequest(EPositionerNotifyPositionUpdate);
		iLocationCallback = NULL;
		iUserData = NULL;
		iKeepTracking = EFalse;
		SetHandlerState(PGS_DestroyPending);
		}
	}

void CLbsHandler::SetHandlerState(ABPAL_GpsState aState)
	{
	iHandlerState = aState;
	}

void CLbsHandler::UpdateGpsCriteria(const ABPAL_GpsCriteria& aCriteria)
	{
	// If you set option, you should attention: maxage < interval < timeout
	TInt64 interval = aCriteria.desiredInterval * KThousand;
	if (interval < KDefaultInterval)
		{
		interval = KDefaultInterval;
		}
	iUpdateops.SetUpdateInterval(TTimeIntervalMicroSeconds(interval));
	iUpdateops.SetUpdateTimeOut(TTimeIntervalMicroSeconds(KDefaultTimeOut));
	iUpdateops.SetMaxUpdateAge(TTimeIntervalMicroSeconds(interval / 2));

	iUpdateops.SetAcceptPartialUpdates(EFalse);
//	iUpdateops.SetAcceptPartialUpdates(ETrue);
	}

TBool CLbsHandler::IsValidResult()
	{
	TBool ret = EFalse;

//	TTime nowTime;
//	nowTime.HomeTime();
//	TTimeIntervalSeconds seconds = 0;
//	TTimeIntervalMicroSeconds microSeconds = nowTime.MicroSecondsFrom(iStartTime);
//	if ((microSeconds.Int64() / KThousand) >= iCriteria.desiredInterval)
		{
		TPositionInfo* positionInfo = static_cast<TPositionInfo*>(iPosInfoBase);
		TPosition position;
		positionInfo->GetPosition(position);
		if (Math::IsNaN(position.Latitude()) || Math::IsNaN(position.Longitude()))
			{
			debugf("  Lat or Lon is not valid.");
			ret = EFalse;
			}
		else
			{
			TReal32 hAcc = position.HorizontalAccuracy();
			if (Math::IsNaN(hAcc))
				{
				debugf("    hAcc is not valid.");
				ret = EFalse;
				}
			else if (hAcc > iCriteria.desiredAccuracy)
				{
				debugf("    lat = %.8f, lon = %.8f, hAcc is %f, the expect value of hAcc is less than: %d.", position.Latitude(), position.Longitude(), hAcc, iCriteria.desiredAccuracy);
				ret = EFalse;
				}
			else
				{
				ret = ETrue;
				}
			}
		}
	return ret;
	}

ABPAL_GpsLocation CLbsHandler::ConvertPositionInfo(const TPositionInfo& aPositionInfo)
	{
//	debugf("ConvertPositionInfo TPositionInfo In");
	ABPAL_GpsLocation location;
	memclr(&location, sizeof(location));
	location.status = PAL_Ok;

	TPosition position;
	aPositionInfo.GetPosition(position);
	Convert(location, position);

	location.gpsTime = PAL_ClockGetGPSTime();
	debugf("	GPS Time = %d", location.gpsTime);

//	debugf("ConvertPositionInfo TPositionInfo Out");
	return location;
	}

ABPAL_GpsLocation CLbsHandler::ConvertPositionCourseInfo(const TPositionCourseInfo& aPositionInfo)
	{
	debugf("ConvertPositionInfo TPositionCourseInfo In");
	ABPAL_GpsLocation location;
	memclr(&location, sizeof(location));
	location.status = PAL_Ok;

	TPosition position;
	aPositionInfo.GetPosition(position);
	Convert(location, position);

	location.gpsTime = PAL_ClockGetGPSTime();
	debugf("	GPS Time = %d", location.gpsTime);

	TCourse course;
	aPositionInfo.GetCourse(course);

	TReal32 heading = course.Heading();
	if (!Math::IsNaN(heading))
		{
		debugf("	Heading = %f", heading);
		location.heading = heading;
		location.valid |= PGV_Heading;
		}

	TReal32 ha = course.HeadingAccuracy();
	if (!Math::IsNaN(ha))
		{
		debugf("	Heading Accuracy = %f", ha);
		location.horizontalUncertaintyAngleOfAxis = ha;
		location.valid |= PGV_HorizontalUncertainty;
		}

	TReal32 speed = course.Speed();
	if (!Math::IsNaN(speed))
		{
		debugf("	Speed = %f", speed);
		location.horizontalVelocity = speed;
		location.valid |= PGV_HorizontalVelocity;
		}
	debugf("ConvertPositionInfo TPositionCourseInfo Out");
	return location;
	}

ABPAL_GpsLocation CLbsHandler::ConvertSatelliteInfo(const TPositionSatelliteInfo& aSatelliteInfo)
	{
	debugf("ConvertPositionInfo TPositionSatelliteInfo In");
	ABPAL_GpsLocation location;
	memclr(&location, sizeof(location));
	location.status = PAL_Ok;

	TPosition position;
	aSatelliteInfo.GetPosition(position);
	Convert(location, position);

	TTime time = aSatelliteInfo.SatelliteTime();
	TTimeIntervalSeconds seconds;
	const TTime KDefaultFromTime(TDateTime(1980, EJanuary, 5, 0, 0, 0, 0));
	if (KErrNone == time.SecondsFrom(KDefaultFromTime, seconds))
		{
		debugf("	GPS Time = %d", seconds.Int());
		location.gpsTime = static_cast<nb_gpsTime>(seconds.Int());
		}

	TCourse course;
	aSatelliteInfo.GetCourse(course);
	TReal heading = static_cast<TReal>(course.Heading());
	if (!Math::IsNaN(heading))
		{
		debugf("	Heading = %f", heading);
		location.heading = heading;
		location.valid = location.valid | PGV_Heading;
		}

	TInt satelliteCount = aSatelliteInfo.NumSatellitesUsed();
	if (satelliteCount > 0)
		{
		debugf("	Number of Satellites = %d", satelliteCount);
		location.numberOfSatellites = static_cast<uint16>(satelliteCount);
		location.valid = location.valid | PGV_SatelliteCount;
		}
	debugf("ConvertPositionInfo TPositionSatelliteInfo Out");
	return location;
	}

void CLbsHandler::Convert(ABPAL_GpsLocation& abLocation, TPosition& aPosition)
	{
	TReal lat = aPosition.Latitude();
	if(!Math::IsNaN(lat))
		{
//		debugf("	Lat = %f", lat);
		abLocation.latitude = lat;
		abLocation.valid |= PGV_Latitude;
		}

	TReal lon = aPosition.Longitude();
	if(!Math::IsNaN(lon))
		{
//		debugf("	Lon = %f", lon);
		abLocation.longitude = lon;
		abLocation.valid |= PGV_Longitude;
		}

	TReal alt = aPosition.Altitude();
	if(!Math::IsNaN(alt))
		{
//		debugf("	Alt = %f", alt);
		abLocation.altitude = alt;
		abLocation.valid |= PGV_Altitude;
		}

	TReal32 ha = aPosition.HorizontalAccuracy();
	if (!Math::IsNaN(ha))
		{
//		debugf("	Horizontal Accuracy = %f", ha);
		abLocation.horizontalUncertaintyAlongPerpendicular = ha;
		abLocation.valid |= PGV_PerpendicularUncertainty;
		}

	TReal32 va = aPosition.VerticalAccuracy();
	if (!Math::IsNaN(va))
		{
//		debugf("	Vertical Accuracy = %f", va);
		abLocation.verticalUncertainty = va;
		abLocation.valid |= PGV_VerticalUncertainty;
		}

	if (iHasLastPosition)
		{
		TReal32 speed = 0;
		TInt ret = aPosition.Speed(iLastPosition, speed);
		if (ret == KErrNone)
			{
//			debugf("	Speed = %f", speed);
			abLocation.horizontalVelocity = speed;
			abLocation.valid |= PGV_HorizontalVelocity;
			}
		else
			{
//			debugf("	Can't get speed. Error = %d", ret);
			}

		TReal32 bearing = 0;
		TReal32 delta = 0;
		aPosition.SetHorizontalAccuracy(0);
		ret = iLastPosition.BearingTo(aPosition, bearing, delta);
		if (ret == KErrNone)
			{
//			debugf("	Heading = %f", bearing);
			abLocation.heading = bearing;
			abLocation.valid |= PGV_Heading;
			}
		else
			{
//			debugf("	Can't get heading. Error = %d", ret);
			}
		}

	iLastPosition = aPosition;
	iHasLastPosition = ETrue;

//	{
//	abLocation.horizontalUncertaintyAngleOfAxis = 0;
//	abLocation.valid |= PGV_HorizontalUncertainty;
//
//	abLocation.horizontalUncertaintyAlongAxis = 250;
//	abLocation.valid |= PGV_AxisUncertainty;
//
//	abLocation.horizontalUncertaintyAlongPerpendicular = 250;
//	abLocation.valid |= PGV_PerpendicularUncertainty;
//	}
	}

void CLbsHandler::CompleteSelf(TInt aError)
	{
	iStatus = KRequestPending;
	SetActive();
	TRequestStatus* status = &iStatus;
	User::RequestComplete(status, aError);
	}

void CLbsHandler::ReportError(TInt aError)
	{
	PAL_Error err = PALUtility::ConvertError(PALUtility::EGpsErrorType, aError);
//	if (iLocationCallback != NULL)
//		{
//		iLocationCallback(iUserData, err, NULL, NULL);
//		}
	iCallbackAo->UpdateResult(err, NULL);

	if (HandlerState() == PGS_Oneshot)
		{
		Release();
		}
	}

void CLbsHandler::ReportLocation()
	{
	ABPAL_GpsLocation location;
	memclr(&location, sizeof(location));
	if(&iPositionInfo == iPosInfoBase)
		{
		location = ConvertPositionInfo(iPositionInfo);
		}
	else if(&iSatelliteInfo == iPosInfoBase)
		{
		location = ConvertSatelliteInfo(iSatelliteInfo);
		}
	else
		{
		ASSERT(EFalse);
		}

	debugf("\tReport location: lat = %.8f, lon = %.8f", location.latitude, location.longitude);

//	if (iLocationCallback != NULL)
//		{
//		iLocationCallback(iUserData, PAL_Ok, &location, NULL);
//		}
	iCallbackAo->UpdateResult(PAL_Ok, &location);
	}

void CLbsHandler::ContinueOrRelease()
	{
	if (IsActive())
		{
		// Continue.
		}
	else if (HandlerState() == PGS_Suspended)
		{
		// Continue.
		}
	else if (iLocationCallback == NULL)
		{
		Release();
		}
	else if (iKeepTracking && HandlerState() == PGS_Tracking)
		{
		iStartTime.HomeTime();
		iPositioner.NotifyPositionUpdate(*iPosInfoBase, iStatus);
		SetActive();
		}
	else
		{
		Release();
		}
	}

TInt CLbsHandler::InitIdleCallback(TAny* aPtr)
	{
	CLbsHandler* self = static_cast<CLbsHandler*>(aPtr);
	if (self->iInitCallback != NULL)
		{
		self->SetHandlerState(PGS_Initialized);
		self->iInitCallback(self->iInitUserData, PAL_Ok, NULL);
		}
	return EFalse;
	}

void CLbsHandler::TrakcingForNextPosition()
	{
	if (!IsActive() && iKeepTracking && HandlerState() == PGS_Tracking)
		{
		debugf("CLbsHandler Tracking For Next Position");
//		iStartTime.HomeTime();
		iPositioner.NotifyPositionUpdate(*iPosInfoBase, iStatus);
		SetActive();
		}
	}
