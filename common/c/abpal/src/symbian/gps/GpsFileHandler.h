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

#ifndef GPSFILEHANDLER_H
#define GPSFILEHANDLER_H

#include <e32base.h>
#include <lbs.h>
#include <lbssatellite.h>

#include "LbsHandlerBase.h"
#include "Timer.h"
#include "GpsSingleton.h"

class CNIMGpsConfiguration;

/***************************************************
GPS FileRecord struct that is stored in gps files
****************************************************/
typedef struct {
	uint32             dwTimeStamp;          // Time, seconds since 1/6/1980
	uint32             status;               // Response status;
	int32              dwLat;                // Lat, 180/2^25 degrees, WGS-84 ellipsoid
	int32              dwLon;                // Lon, 360/2^26 degrees, WGS-84 ellipsoid
	int16              wAltitude;            // Alt, meters, WGS-84 ellipsoid
	uint16             wHeading;             // Heading, 360/2^10 degrees
	uint16             wVelocityHor;         // Horizontal velocity, 0.25 meters/second
	int8               bVelocityVer;         // Vertical velocity, 0.5 meters/second
	int8               accuracy;			 // Accuracy of the data
	uint16             fValid;               // Flags indicating valid fields in the struct.
	uint8              bHorUnc;              // Horizontal uncertainty
	uint8              bHorUncAngle;         // Horizontal Uncertainty at angle
	uint8              bHorUncPerp;          // Horizontal uncertainty perpendicular
	uint8              bNothing;
	uint16             wVerUnc;              // Vertical uncertainty.
} GPSFileRecord;

NONSHARABLE_CLASS(CGpsFileHandler) : public CBase, public MLbsHandlerBase, public MNimTimerObserver
	{
public:
	static CGpsFileHandler* NewL(PAL_Instance& aPal);
	static CGpsFileHandler* NewLC(PAL_Instance& aPal);
	virtual ~CGpsFileHandler();

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
	// From MNimTimerObserver
	virtual void TimeExpiredL(CNimTimer* aTimer);
	virtual void TimerCanceled(CNimTimer* aTimer);

private:
	CGpsFileHandler(PAL_Instance& aPal, CGpsSingleton& aGpsSingleton);
	void ConstructL();

private:
	TBool VerifyTrackingInfo(const ABPAL_GpsTrackingInfo& aTrackingInfo) const;
	void SetHandlerState(ABPAL_GpsState aState);
	void UpdateGpsCriteria(const ABPAL_GpsCriteria& aCriteria);
	void ReportError(TInt aError);
	PAL_Error ReadNextPosition(ABPAL_GpsLocation& aLocation);
	void Convert(const GPSFileRecord& aRecord, ABPAL_GpsLocation& aLocation);
	void UpdateGpsFileName(const char* aFilename);
	void UpdateGpsOffset(TInt aOffset);

private:
	static TInt InitIdleCallback(TAny* aPtr);
	static TInt GetLocationCallback(TAny* aPtr);
	static TInt TrackingCallback(TAny* aPtr);

private:
	PAL_Instance&					iPal;
	CGpsSingleton&					iGpsSingleton;
	ABPAL_GpsInitializeCallback*	iInitCallback;
	ABPAL_GpsDeviceStateCallback*	iDeviceStateCallback;
	ABPAL_GpsTrackingInfo			iTrackingInfo;
	ABPAL_GpsState					iHandlerState;
	ABPAL_GpsLocationCallback*		iLocationCallback;
	const void*						iInitUserData;
	const void*						iUserData;

	ABPAL_GpsCriteria				iCriteria;

	CNimTimer*						iIntervalTimer;
	TTime							iStartTime;
	TInt							iMaxCount;
	};

#endif	// GPSFILEHANDLER_H
