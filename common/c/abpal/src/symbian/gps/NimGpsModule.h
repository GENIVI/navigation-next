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

#ifndef NIMGPSMODULE_H
#define NIMGPSMODULE_H

#include <e32base.h>
#include <lbs.h>

#include "palerror.h"
#include "abpalgps.h"
#include "NIMGpsConfiguration.h"

class MLbsHandlerBase;

NONSHARABLE_CLASS(CNimGpsModule) : public CBase
	{
public:
	static CNimGpsModule* NewL(PAL_Instance& aPal);
	static CNimGpsModule* NewLC(PAL_Instance& aPal);
	virtual ~CNimGpsModule();

public:
	PAL_Error Initial(const CNIMGpsConfiguration& aConfig, ABPAL_GpsInitializeCallback* aInitCallback, ABPAL_GpsDeviceStateCallback* aStateCallback, const void* aUserData);
	void InitialFinish();
	void Finish();
	ABPAL_GpsState State();
	void Release();
	PAL_Error GetLocation(const ABPAL_GpsCriteria& aCriteria, ABPAL_GpsLocationCallback* aCallback, const void* aUserData);
	PAL_Error CancelGetLocation();
	PAL_Error BeginTracking(TBool aObserveOnly, const ABPAL_GpsCriteria& aCriteria, ABPAL_GpsLocationCallback* aCallback, const void* aUserData, ABPAL_GpsTrackingInfo* aTrackingInfo);
	PAL_Error EndTracking(const ABPAL_GpsTrackingInfo* aTrackingInfo);
	PAL_Error UpdateCriteria(const ABPAL_GpsCriteria& aCriteria, const ABPAL_GpsTrackingInfo* aTrackingInfo);
	PAL_Error Suspend(const ABPAL_GpsTrackingInfo* aTrackingInfo);
	PAL_Error Resume(const ABPAL_GpsTrackingInfo* aTrackingInfo);

private:
	CNimGpsModule(PAL_Instance& aPal);
	void ConstructL();

private:
	PAL_Error InitialL(const CNIMGpsConfiguration& aConfig, ABPAL_GpsInitializeCallback* aInitCallback, ABPAL_GpsDeviceStateCallback* aStateCallback, const void* aUserData);

private:
	PAL_Instance&					iPal;
	MLbsHandlerBase*				iLbsHandler;
	};

#endif // NIMGPSMODULE_H
