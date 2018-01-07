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
#include "palfile.h"
#include "NIMGpsConfiguration.h"
#include "GpsFileHandler.h"
#include "palfunc.h"

const TInt64 KThousand = 1000;
const TInt64 KDefaultTimeOut = 60000000;	// 1 minute
const TInt	 KIdleTimeIntervalInSecond = 1;

CGpsFileHandler* CGpsFileHandler::NewL(PAL_Instance& aPal)
	{
	CGpsFileHandler* self = CGpsFileHandler::NewLC(aPal);
	CleanupStack::Pop(self);
	return self;
	}

CGpsFileHandler* CGpsFileHandler::NewLC(PAL_Instance& aPal)
	{
	CGpsSingleton* gpsSingleton = static_cast<CGpsSingleton*>(PAL_GetGpsExtend(&aPal));
	CGpsFileHandler* self = new (ELeave) CGpsFileHandler(aPal, *gpsSingleton);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CGpsFileHandler::CGpsFileHandler(PAL_Instance& aPal, CGpsSingleton& aGpsSingleton)
				: iPal(aPal), iGpsSingleton(aGpsSingleton)
	{
	SetHandlerState(PGS_Undefined);
	}

CGpsFileHandler::~CGpsFileHandler()
	{
	Release();
	delete iIntervalTimer;
	}

void CGpsFileHandler::ConstructL()
	{
	iIntervalTimer = CNimTimer::NewL(*this);
	}

TInt CGpsFileHandler::Initial(const CNIMGpsConfiguration& aConfig, ABPAL_GpsInitializeCallback* aInitCallback, ABPAL_GpsDeviceStateCallback* aStateCallback, const void* aUserData)
	{
	TInt ret = KErrGeneral;
	if (HandlerState() == PGS_Undefined)
		{
		const char* filename = aConfig.GpsFileName();
		UpdateGpsFileName(filename);
		filename = iGpsSingleton.GpsFileName();

		uint32 filesize = 0;
		PAL_Error err = PAL_FileGetSize(&iPal, filename, &filesize);
		if (err != PAL_Ok)
			{
			return err;
			}

		iMaxCount = filesize / sizeof(GPSFileRecord);
		if (iMaxCount < 1)
			{
			return PAL_ErrNoData;
			}

//		UpdateGpsOffset(0);
//		ABPAL_GpsLocation startlocation;
//		nsl_memset(&startlocation, 0, sizeof(startlocation));
//		ReadNextPosition(startlocation);
//
//		UpdateGpsOffset(iMaxCount - 1);
//		ABPAL_GpsLocation endlocation;
//		nsl_memset(&endlocation, 0, sizeof(endlocation));
//		ReadNextPosition(endlocation);
//		UpdateGpsOffset(0);

		iInitCallback = aInitCallback;
		iDeviceStateCallback = aStateCallback;
		iInitUserData = aUserData;
		if (!iIntervalTimer->IsActive())
			{
			iIntervalTimer->Start(KIdleTimeIntervalInSecond);
			}
		SetHandlerState(PGS_InitializePending);
		ret = KErrNone;
		}
	return ret;
	}

ABPAL_GpsState CGpsFileHandler::HandlerState() const
	{
	return iHandlerState;
	}

TInt CGpsFileHandler::GetLocation(const ABPAL_GpsCriteria& aCriteria, ABPAL_GpsLocationCallback* aCallback, const void* aUserData)
	{
	TInt ret = KErrInUse;
	if (HandlerState() == PGS_Initialized)
		{
		UpdateGpsCriteria(aCriteria);
		if (!iIntervalTimer->IsActive())
			{
			iIntervalTimer->Start(KIdleTimeIntervalInSecond);
			}
		iLocationCallback = aCallback;
		iUserData = aUserData;
		SetHandlerState(PGS_Oneshot);
		ret = KErrNone;
		}
	return ret;
	}

void CGpsFileHandler::CancelGetLocation()
	{
	if (HandlerState() == PGS_Oneshot)
		{
		iIntervalTimer->Cancel();
		Release();
		}
	}

TInt CGpsFileHandler::BeginTracking(const ABPAL_GpsCriteria& aCriteria, ABPAL_GpsLocationCallback* aCallback, const void* aUserData, ABPAL_GpsTrackingInfo& aTrackingInfo)
	{
	TInt ret = KErrInUse;
	if (HandlerState() == PGS_Initialized)
		{
		UpdateGpsOffset(0);	// Reset the position to the first block of the file.
		UpdateGpsCriteria(aCriteria);

		if (!iIntervalTimer->IsActive())
			{
			iIntervalTimer->StartByMilliSecond(iCriteria.desiredInterval);
			}

		iLocationCallback = aCallback;
		iUserData = aUserData;
		aTrackingInfo.Id = 1;
		iTrackingInfo = aTrackingInfo;
		SetHandlerState(PGS_Tracking);
		ret = KErrNone;
		}
	return ret;
	}

void CGpsFileHandler::EndTracking(const ABPAL_GpsTrackingInfo& aTrackingInfo)
	{
	if (VerifyTrackingInfo(aTrackingInfo) && HandlerState() == PGS_Tracking)
		{
		iIntervalTimer->Cancel();
		Release();
		}
	}

TInt CGpsFileHandler::UpdateCriteria(const ABPAL_GpsCriteria& aCriteria, const ABPAL_GpsTrackingInfo& aTrackingInfo)
	{
	TInt ret = KErrNotReady;
	if (VerifyTrackingInfo(aTrackingInfo) && HandlerState() == PGS_Tracking)
		{
		iIntervalTimer->Cancel();
		UpdateGpsCriteria(aCriteria);

		if (!iIntervalTimer->IsActive())
			{
			iIntervalTimer->StartByMilliSecond(iCriteria.desiredInterval);
			}

		ret = KErrNone;
		}
	return ret;
	}

void CGpsFileHandler::Suspend(const ABPAL_GpsTrackingInfo& aTrackingInfo)
	{
	if (VerifyTrackingInfo(aTrackingInfo) && HandlerState() == PGS_Tracking)
		{
		iIntervalTimer->Cancel();
		SetHandlerState(PGS_Suspended);
		}
	}

void CGpsFileHandler::Resume(const ABPAL_GpsTrackingInfo& aTrackingInfo)
	{
	if (VerifyTrackingInfo(aTrackingInfo) && HandlerState() == PGS_Suspended)
		{
		iIntervalTimer->Cancel();
		iIntervalTimer->StartByMilliSecond(iCriteria.desiredInterval);
		SetHandlerState(PGS_Tracking);
		}
	}

void CGpsFileHandler::TimeExpiredL(CNimTimer* aTimer)
	{
	if (aTimer == iIntervalTimer)
		{
		switch (HandlerState())
			{
			case PGS_InitializePending:
				{
				if (iInitCallback != NULL)
					{
					SetHandlerState(PGS_Initialized);
					iInitCallback(iInitUserData, PAL_Ok, NULL);
					}
				}
				break;
			case PGS_Oneshot:
				{
				if (iLocationCallback != NULL)
					{
					ABPAL_GpsLocation location;
					memclr(&location, sizeof(location));
					PAL_Error err = ReadNextPosition(location);
					SetHandlerState(PGS_Initialized);
					if (err == PAL_Ok)
						{
						iLocationCallback(iUserData, PAL_Ok, &location, NULL);
						}
					else
						{
						ReportError(err);
						}
					}
				}
				break;
			case PGS_Tracking:
				{
				if (iLocationCallback != NULL)
					{
					ABPAL_GpsLocation location;
					memclr(&location, sizeof(location));
					PAL_Error err = ReadNextPosition(location);
					if (err == PAL_Ok)
						{
						iIntervalTimer->StartByMilliSecond(iCriteria.desiredInterval);
						iLocationCallback(iUserData, PAL_Ok, &location, NULL);
						}
					else
						{
						ReportError(err);
						}
					}
				}
				break;
			default:
				break;
			}
		}
	}

void CGpsFileHandler::TimerCanceled(CNimTimer* /*aTimer*/)
	{
	}

void CGpsFileHandler::Release()
	{
	iLocationCallback = NULL;
	iUserData = NULL;
	SetHandlerState(PGS_Initialized);
	}

TBool CGpsFileHandler::VerifyTrackingInfo(const ABPAL_GpsTrackingInfo& /*aTrackingInfo*/) const
	{
	return ETrue;
	}

void CGpsFileHandler::SetHandlerState(ABPAL_GpsState aState)
	{
	iHandlerState = aState;
	}

void CGpsFileHandler::UpdateGpsCriteria(const ABPAL_GpsCriteria& aCriteria)
	{
	iCriteria = aCriteria;
	}

void CGpsFileHandler::ReportError(TInt /*aError*/)
	{
	SetHandlerState(PGS_Initialized);
	iLocationCallback(iUserData, PAL_ErrGpsInformationUnavailable, NULL, NULL);
	}

PAL_Error CGpsFileHandler::ReadNextPosition(ABPAL_GpsLocation& aLocation)
	{
	TInt current = iGpsSingleton.GpsFileOffset();
	if (current >= iMaxCount)
		{
		current = 0;
		}

	PAL_Error err = PAL_Ok;
	PAL_File* file = NULL;
	const char* gpsFilename = iGpsSingleton.GpsFileName();
	err = PAL_FileOpen(&iPal, gpsFilename, PFM_Read, &file);
	if (err != PAL_Ok)
		{
		return err;
		}

	int32 offset = current * sizeof(GPSFileRecord);
	err = PAL_FileSetPosition(file, PFSO_Start, offset);
	if (err != PAL_Ok)
		{
		PAL_FileClose(file);
		return err;
		}

	uint32 bytesReaded = 0;
	GPSFileRecord record;
	memclr(&record, sizeof(record));
	err = PAL_FileRead(file, (uint8*)(&record), sizeof(GPSFileRecord), &bytesReaded);
	if (err != PAL_Ok)
		{
		PAL_FileClose(file);
		return err;
		}

	Convert(record, aLocation);

	PAL_FileClose(file);
	++current;
	UpdateGpsOffset(current);

	return err;
	}

void CGpsFileHandler::Convert(const GPSFileRecord& aRecord, ABPAL_GpsLocation& aLocation)
	{
	const TReal64 KWGSCoefficient	= 0.00000536441803;
	const TReal64 KHeadingScale		= 0.3515625;
	const TReal64 KSpeedScale		= 0.25;

	const TInt KFakeNumOfSatellites	= 4;

	aLocation.status = PAL_Ok;
	aLocation.gpsTime = PAL_ClockGetGPSTime();
	aLocation.valid = aRecord.fValid;
//	aLocation.valid = PGV_Latitude | PGV_Longitude | PGV_Altitude | PGV_Heading;// | PGV_HorizontalVelocity;
//	aLocation.valid = PGV_Latitude | PGV_Longitude | PGV_Altitude | PGV_Heading | PGV_AxisUncertainty | PGV_HorizontalUncertainty | PGV_PerpendicularUncertainty;
	aLocation.latitude = aRecord.dwLat * KWGSCoefficient;
	aLocation.longitude = aRecord.dwLon * KWGSCoefficient;
	aLocation.heading = aRecord.wHeading * KHeadingScale;
	aLocation.horizontalVelocity = aRecord.wVelocityHor * KSpeedScale;
	aLocation.altitude = aRecord.wAltitude * KWGSCoefficient;
//
//	if (aLocation.horizontalVelocity < 6)
//		{
//		aLocation.horizontalVelocity = 10;
//		}
	aLocation.verticalVelocity = aRecord.bVelocityVer / 2;
	aLocation.horizontalUncertaintyAngleOfAxis = aRecord.bHorUncAngle;
	aLocation.horizontalUncertaintyAlongAxis = aRecord.bHorUnc;
	aLocation.horizontalUncertaintyAlongPerpendicular = aRecord.bHorUncPerp;
	aLocation.verticalUncertainty = aRecord.wVerUnc;
	aLocation.utcOffset = 0;
	aLocation.numberOfSatellites = KFakeNumOfSatellites;

//	aLocation.valid = aRecord.fValid;
//	aLocation.latitude = aRecord.dwLat * KWGSCoefficient;
//	aLocation.longitude = aRecord.dwLon * KWGSCoefficient;
//	aLocation.heading = aRecord.wHeading * KHeadingScale;
//	aLocation.horizontalVelocity = aRecord.wVelocityHor * KSpeedScale;
//	aLocation.altitude = aRecord.wAltitude * KWGSCoefficient;
//	aLocation.verticalVelocity = aRecord.bVelocityVer / 2;
//	aLocation.horizontalUncertaintyAngleOfAxis = aRecord.bHorUncAngle;
//	aLocation.horizontalUncertaintyAlongAxis = aRecord.bHorUnc;
//	aLocation.horizontalUncertaintyAlongPerpendicular = aRecord.bHorUncPerp;
//	aLocation.verticalUncertainty = aRecord.wVerUnc;
//	aLocation.utcOffset = 0;
//	aLocation.numberOfSatellites = KFakeNumOfSatellites;
	}

void CGpsFileHandler::UpdateGpsFileName(const char* aFilename)
	{
	const char* gpsFilename = iGpsSingleton.GpsFileName();
	if ((gpsFilename == NULL) || (nsl_strcmp(aFilename, gpsFilename) != 0))
		{
		iGpsSingleton.SetGpsFileL(aFilename);
		UpdateGpsOffset(0);
		}
	}

void CGpsFileHandler::UpdateGpsOffset(TInt aOffset)
	{
	iGpsSingleton.SetGpsFileOffset(aOffset);
	}

TInt CGpsFileHandler::InitIdleCallback(TAny* aPtr)
	{
	CGpsFileHandler* self = static_cast<CGpsFileHandler*>(aPtr);
	if (self->iInitCallback != NULL)
		{
		self->SetHandlerState(PGS_Initialized);
		self->iInitCallback(self->iInitUserData, PAL_Ok, NULL);
		}
	return EFalse;
	}

TInt CGpsFileHandler::GetLocationCallback(TAny* aPtr)
	{
	CGpsFileHandler* self = static_cast<CGpsFileHandler*>(aPtr);
	if (self->iLocationCallback != NULL)
		{
		ABPAL_GpsLocation location;
		memclr(&location, sizeof(location));
		PAL_Error err = self->ReadNextPosition(location);
		self->SetHandlerState(PGS_Initialized);
		if (err == PAL_Ok)
			{
			self->iLocationCallback(self->iUserData, PAL_Ok, &location, NULL);
			}
		else
			{
			self->ReportError(err);
			}
		}
	return EFalse;
	}

TInt CGpsFileHandler::TrackingCallback(TAny* aPtr)
	{
	CGpsFileHandler* self = static_cast<CGpsFileHandler*>(aPtr);
	if (self->iLocationCallback != NULL)
		{
		ABPAL_GpsLocation location;
		memclr(&location, sizeof(location));
		PAL_Error err = self->ReadNextPosition(location);
		if (err == PAL_Ok)
			{
			self->iLocationCallback(self->iUserData, PAL_Ok, &location, NULL);
			}
		else
			{
			self->ReportError(err);
			}
		}
	return EFalse;
	}
