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

#include <e32base.h>
#include "abpalexp.h"
#include "palerror.h"
#include "abpalgps.h"
#include "palstdlib.h"
#include "pal.h"
#include "PalUtility.h"
#include "NimGpsModule.h"
#include "NIMGpsConfiguration.h"

struct ABPAL_GpsContext
{
	CNimGpsModule* gpsModule;
};

static PAL_Error ABPAL_GpsCreateL(PAL_Instance* aPal, ABPAL_GpsContext** aGpsContext)
	{
	PAL_Error result = PAL_ErrBadParam;
	if ((aPal != NULL) && (aGpsContext != NULL))
		{
		ABPAL_GpsContext* gpsContext = static_cast<ABPAL_GpsContext*>(nsl_malloc(sizeof(*gpsContext)));
		if (gpsContext)
			{
			gpsContext->gpsModule = CNimGpsModule::NewL(*aPal);
			*aGpsContext = gpsContext;
			result = PAL_Ok;
			}
		else
			{
			result = PAL_ErrNoMem;
			}
		}
	return result;
	}

ABPAL_DEF PAL_Error ABPAL_GpsCreate(PAL_Instance* pal, ABPAL_GpsContext** gpsContext)
	{
	PAL_Error ret = PAL_Ok;
	TRAPD(err, ret = ABPAL_GpsCreateL(pal, gpsContext));
	if (err != KErrNone)
		{
		ret = PALUtility::ConvertError(PALUtility::EGpsErrorType, err);
		}
	return ret;
	}

static PAL_Error ABPAL_GpsInitializeL(ABPAL_GpsContext* aGpsContext, const ABPAL_GpsConfig* aConfig,
										uint32 aConfigCount, ABPAL_GpsInitializeCallback* aInitCallback,
										ABPAL_GpsDeviceStateCallback* aStateCallback, const void* aUserData)
	{
	PAL_Error ret = PAL_ErrBadParam;
	if (aGpsContext != NULL)
		{
		CNIMGpsConfiguration* configuration = CNIMGpsConfiguration::NewLC(aConfigCount, aConfig);
		ret = aGpsContext->gpsModule->Initial(*configuration, aInitCallback, aStateCallback, aUserData);
		CleanupStack::PopAndDestroy(configuration);
		}
	return ret;
	}

ABPAL_DEF PAL_Error ABPAL_GpsInitialize(ABPAL_GpsContext* gpsContext, const ABPAL_GpsConfig* config,
										uint32 configCount, ABPAL_GpsInitializeCallback* userCallback,
										ABPAL_GpsDeviceStateCallback* userCallbackDevState,
										const void* userData)
	{
	PAL_Error ret = PAL_Ok;
	TRAPD(error, ret = ABPAL_GpsInitializeL(gpsContext, config, configCount, userCallback, userCallbackDevState, userData));
	if (error != KErrNone)
		{
		ret = PALUtility::ConvertError(PALUtility::EGpsErrorType, error);
		}
	return ret;
	}

ABPAL_DEF PAL_Error ABPAL_GpsDestroy(ABPAL_GpsContext* gpsContext)
	{
	gpsContext->gpsModule->Release();
	delete gpsContext->gpsModule;
	nsl_free(gpsContext);
	return PAL_Ok;
	}

ABPAL_DEF PAL_Error ABPAL_GpsGetState(ABPAL_GpsContext* gpsContext, ABPAL_GpsState* gpsState)
	{
	PAL_Error ret = PAL_ErrBadParam;
	if ((gpsContext != NULL) && (gpsState != NULL))
		{
		*gpsState = gpsContext->gpsModule->State();
		ret = PAL_Ok;
		}
	return ret;
	}

static PAL_Error ABPAL_GpsGetLocationL(ABPAL_GpsContext* aGpsContext, const ABPAL_GpsCriteria* aCriteria,
										ABPAL_GpsLocationCallback* aUserCallback, const void* aUserData)
	{
	PAL_Error ret = PAL_ErrBadParam;
	if ((aGpsContext != NULL) && (aCriteria != NULL))
		{
		ret = aGpsContext->gpsModule->GetLocation(*aCriteria, aUserCallback, aUserData);
		}
	return ret;
	}

ABPAL_DEF PAL_Error ABPAL_GpsGetLocation(ABPAL_GpsContext* gpsContext, const ABPAL_GpsCriteria* criteria,
											ABPAL_GpsLocationCallback* userCallback, const void* userData)
	{
	PAL_Error ret = PAL_Ok;
	TRAPD(error, ret = ABPAL_GpsGetLocationL(gpsContext, criteria, userCallback, userData));
	if (error != KErrNone)
		{
		ret = PALUtility::ConvertError(PALUtility::EGpsErrorType, error);
		}
	return ret;
	}

ABPAL_DEF PAL_Error ABPAL_GpsCancelGetLocation(ABPAL_GpsContext* gpsContext)
	{
	PAL_Error ret = PAL_ErrBadParam;
	if (gpsContext != NULL)
		{
		ret = gpsContext->gpsModule->CancelGetLocation();
		}
	return ret;
	}

ABPAL_DEF PAL_Error ABPAL_GpsBeginTracking(ABPAL_GpsContext* gpsContext, uint16 observeOnly,
											const ABPAL_GpsCriteria* criteria,
											ABPAL_GpsLocationCallback* userCallback,
											const void* userData,
											ABPAL_GpsTrackingInfo* trackingInfo)
	{
	PAL_Error ret = PAL_ErrBadParam;
	if ((gpsContext != NULL) && (criteria != NULL) && (trackingInfo != NULL))
		{
		ret = gpsContext->gpsModule->BeginTracking(observeOnly, *criteria, userCallback, userData, trackingInfo);
		}
	return ret;
	}

ABPAL_DEF PAL_Error ABPAL_GpsEndTracking(ABPAL_GpsContext* gpsContext, const ABPAL_GpsTrackingInfo* trackingInfo)
	{
	PAL_Error ret = PAL_ErrBadParam;
	if ((gpsContext != NULL) && (trackingInfo != NULL))
		{
		ret = gpsContext->gpsModule->EndTracking(trackingInfo);
		}
	return ret;
	}

ABPAL_DEF PAL_Error ABPAL_GpsUpdateCriteria(ABPAL_GpsContext* gpsContext, const ABPAL_GpsCriteria* criteria,
												const ABPAL_GpsTrackingInfo* trackingInfo)
	{
	PAL_Error ret = PAL_ErrBadParam;
	if (gpsContext != NULL)
		{
		ret = gpsContext->gpsModule->UpdateCriteria(*criteria, trackingInfo);
		}
	return ret;
	}

ABPAL_DEF PAL_Error ABPAL_GpsSuspend(ABPAL_GpsContext* gpsContext, const ABPAL_GpsTrackingInfo* trackingInfo)
	{
	PAL_Error result = PAL_ErrBadParam;
	if (gpsContext != NULL)
		{
		result = gpsContext->gpsModule->Suspend(trackingInfo);
		}
	return result;
	}

ABPAL_DEF PAL_Error ABPAL_GpsResume(ABPAL_GpsContext* gpsContext, const ABPAL_GpsTrackingInfo* trackingInfo)
	{
	PAL_Error result = PAL_ErrBadParam;
	if (gpsContext != NULL)
		{
		result = gpsContext->gpsModule->Resume(trackingInfo);
		}
	return result;
	}
