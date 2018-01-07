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

#include <lbs.h>

#include "palfunc.h"
#include "PalUtility.h"
#include "NimGpsModule.h"
#include "GpsSingleton.h"
#include "LbsHandler.h"
#include "GpsFileHandler.h"

CNimGpsModule* CNimGpsModule::NewL(PAL_Instance& aPal)
	{
	CNimGpsModule* self = CNimGpsModule::NewLC(aPal);
	CleanupStack::Pop(self);
	return self;
	}

CNimGpsModule* CNimGpsModule::NewLC(PAL_Instance& aPal)
	{
	CNimGpsModule* self = new (ELeave) CNimGpsModule(aPal);
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

CNimGpsModule::CNimGpsModule(PAL_Instance& aPal) : iPal(aPal)
	{
	}

CNimGpsModule::~CNimGpsModule()
	{
	Release();
	}

void CNimGpsModule::ConstructL()
	{
	CBase* gpsExtend = PAL_GetGpsExtend(&iPal);
	if (gpsExtend == NULL)
		{
		CGpsSingleton* gpsSingleton = CGpsSingleton::NewL();
		PAL_SetGpsExtend(&iPal, gpsSingleton);
		}
	}

PAL_Error CNimGpsModule::Initial(const CNIMGpsConfiguration& aConfig,
									ABPAL_GpsInitializeCallback* aInitCallback,
									ABPAL_GpsDeviceStateCallback* aStateCallback,
									const void* aUserData)
	{
	PAL_Error ret = PAL_Ok;
	TRAPD(err, ret = InitialL(aConfig, aInitCallback, aStateCallback, aUserData));
	if (err != KErrNone)
		{
		ret = PALUtility::ConvertError(PALUtility::EGpsErrorType, err);
		}
	return ret;
	}

ABPAL_GpsState CNimGpsModule::State()
	{
	ABPAL_GpsState state = PGS_Undefined;
	if (iLbsHandler != NULL)
		{
		state = iLbsHandler->HandlerState();
		}
	return state;
	}

void CNimGpsModule::Release()
	{
	if (iLbsHandler != NULL)
		{
		iLbsHandler->Release();
		delete iLbsHandler;
		iLbsHandler = NULL;
		}
	}

PAL_Error CNimGpsModule::GetLocation(const ABPAL_GpsCriteria& aCriteria,
										ABPAL_GpsLocationCallback* aCallback, const void* aUserData)
	{
	PAL_Error ret = PAL_ErrGpsGeneralFailure;
	if (iLbsHandler != NULL)
		{
		TInt result = iLbsHandler->GetLocation(aCriteria, aCallback, aUserData);
		if (result != KErrNone)
			{
			ret = PALUtility::ConvertError(PALUtility::EGpsErrorType, result);
			}
		else
			{
			ret = PAL_Ok;
			}
		}
	return ret;
	}

PAL_Error CNimGpsModule::CancelGetLocation()
	{
	if (iLbsHandler != NULL)
		{
		iLbsHandler->CancelGetLocation();
		}
	return PAL_Ok;
	}

PAL_Error CNimGpsModule::BeginTracking(TBool /*aObserveOnly*/, const ABPAL_GpsCriteria& aCriteria,
										ABPAL_GpsLocationCallback* aCallback, const void* aUserData,
										ABPAL_GpsTrackingInfo* aTrackingInfo)
	{
	PAL_Error ret = PAL_ErrGpsGeneralFailure;
	if (iLbsHandler != NULL)
		{
		TInt result = iLbsHandler->BeginTracking(aCriteria, aCallback, aUserData, *aTrackingInfo);
		if (result != KErrNone)
			{
			ret = PALUtility::ConvertError(PALUtility::EGpsErrorType, result);
			}
		else
			{
			ret = PAL_Ok;
			}
		}
	return ret;
	}

PAL_Error CNimGpsModule::EndTracking(const ABPAL_GpsTrackingInfo* aTrackingInfo)
	{
	if (iLbsHandler != NULL)
		{
		iLbsHandler->EndTracking(*aTrackingInfo);
		}
	return PAL_Ok;
	}

PAL_Error CNimGpsModule::UpdateCriteria(const ABPAL_GpsCriteria& aCriteria,
											const ABPAL_GpsTrackingInfo* aTrackingInfo)
	{
	if (iLbsHandler != NULL)
		{
		iLbsHandler->UpdateCriteria(aCriteria, *aTrackingInfo);
		}
	return PAL_Ok;
	}

PAL_Error CNimGpsModule::Suspend(const ABPAL_GpsTrackingInfo* aTrackingInfo)
	{
	if (iLbsHandler != NULL)
		{
		iLbsHandler->Suspend(*aTrackingInfo);
		}
	return PAL_Ok;
	}

PAL_Error CNimGpsModule::Resume(const ABPAL_GpsTrackingInfo* aTrackingInfo)
	{
	if (iLbsHandler != NULL)
		{
		iLbsHandler->Resume(*aTrackingInfo);
		}
	return PAL_Ok;
	}

PAL_Error CNimGpsModule::InitialL(const CNIMGpsConfiguration& aConfig,
									ABPAL_GpsInitializeCallback* aInitCallback,
									ABPAL_GpsDeviceStateCallback* aStateCallback,
									const void* aUserData)
	{
	PAL_Error ret = PAL_ErrGpsGeneralFailure;
	if (iLbsHandler != NULL)
		{
		ret = PAL_ErrGpsInvalidState;
		}
	else
		{
		TInt result = KErrNone;
		if (aConfig.ProviderType() == CNIMGpsConfiguration::ENIMGpsProviderTypeGpsId)
			{
			iLbsHandler = CLbsHandler::NewL(iPal);
			result = iLbsHandler->Initial(aConfig, aInitCallback, aStateCallback, aUserData);
			}
		else if (aConfig.ProviderType() == CNIMGpsConfiguration::ENIMGpsProviderTypeEmu)
			{
			iLbsHandler = CGpsFileHandler::NewL(iPal);
			result = iLbsHandler->Initial(aConfig, aInitCallback, aStateCallback, aUserData);
			}

		if (result != KErrNone)
			{
			ret = PALUtility::ConvertError(PALUtility::EGpsErrorType, result);
			}
		else
			{
			ret = PAL_Ok;
			}
		}
	return ret;
	}
