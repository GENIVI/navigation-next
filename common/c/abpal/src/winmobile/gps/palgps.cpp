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

/*!--------------------------------------------------------------------------

@file palgps.cpp
*/
/*
(C) Copyright 2014 by TeleCommunication Systems, Inc.                

The information contained herein is confidential, proprietary 
to TeleCommunication Systems, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of TeleCommunication Systems is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*!
    @addtogroup abpalgpswinmobile
    @{
*/

#include "windef.h"
#include "palerror.h"
#include "abpalgps.h"
#include "gpscontext.h"
#include "gpscriteria.h"

struct ABPAL_GpsContext
{
	GpsContext* context;
};


PAL_Error ABPAL_GpsCreate(PAL_Instance* pal, ABPAL_GpsContext** gpsContext)
{
	if (!pal || !gpsContext)
	{
		return PAL_ErrBadParam;
	}
    *gpsContext = 0;

    ABPAL_GpsContext* pThis = new ABPAL_GpsContext;
    if (!pThis)
    {
        return PAL_ErrNoMem;
    }
    memset(pThis, 0, sizeof(*pThis));

	pThis->context = new GpsContext(pal);
    if (!pThis->context)
    {
        delete pThis;
        return PAL_ErrNoMem;
    }

    *gpsContext = pThis;

    return PAL_Ok;
}


PAL_Error ABPAL_GpsInitialize(ABPAL_GpsContext* abpalContext, const ABPAL_GpsConfig* config, uint32 configCount,
    ABPAL_GpsInitializeCallback* userCallback, ABPAL_GpsDeviceStateCallback* userCallbackDevState, const void* userData)
{
	if (!abpalContext || !config || configCount == 0)
	{
		return PAL_ErrBadParam;
	}

    return abpalContext->context->Initialize(config, configCount, userCallback, userCallbackDevState, userData);
}

PAL_Error ABPAL_GpsDestroy(ABPAL_GpsContext* abpalContext)
{
	if (!abpalContext)
	{
		return PAL_ErrBadParam;
	}

    delete abpalContext->context;
	delete abpalContext;

    return PAL_Ok;
}


PAL_Error ABPAL_GpsGetState(ABPAL_GpsContext* abpalContext, ABPAL_GpsState* gpsState)
{
	if (!abpalContext)
	{
		return PAL_ErrBadParam;
	}

    /// @todo Get the state

    return PAL_Ok;
}

PAL_Error ABPAL_GpsGetLocation(ABPAL_GpsContext* abpalContext, const ABPAL_GpsCriteria* criteria, ABPAL_GpsLocationCallback* callback, const void* callbackData)
{
	if (!abpalContext || !callback)
	{
		return PAL_ErrBadParam;
	}

    return abpalContext->context->GetLocation(criteria, callback, callbackData);
}

PAL_Error ABPAL_GpsCancelGetLocation(ABPAL_GpsContext* abpalContext)
{
    if (!abpalContext || !abpalContext->context)
	{
		return PAL_ErrBadParam;
	}

    return abpalContext->context->CancelGetLocation();
}

PAL_Error ABPAL_GpsBeginTracking(ABPAL_GpsContext* abpalContext, uint16 observeOnly, const ABPAL_GpsCriteria* criteria, ABPAL_GpsLocationCallback* callback, const void* callbackData, ABPAL_GpsTrackingInfo* trackingInfo)
{
	if (!abpalContext)
	{
		return PAL_ErrBadParam;
	}

    return abpalContext->context->AddListener(criteria, callback, callbackData, trackingInfo);
}

PAL_Error ABPAL_GpsEndTracking(ABPAL_GpsContext* abpalContext, const ABPAL_GpsTrackingInfo* trackingInfo)
{
	if (!abpalContext || !trackingInfo)
	{
		return PAL_ErrBadParam;
	}

    return abpalContext->context->RemoveListener(trackingInfo);
}

PAL_Error ABPAL_GpsUpdateCriteria(ABPAL_GpsContext* abpalContext, const ABPAL_GpsCriteria* criteria, const ABPAL_GpsTrackingInfo* trackingInfo)
{
	if (!abpalContext || !trackingInfo)
	{
		return PAL_ErrBadParam;
	}

    /// @todo Implement this

    return PAL_ErrUnsupported;
}

PAL_Error ABPAL_GpsSuspend(ABPAL_GpsContext* abpalContext, const ABPAL_GpsTrackingInfo* trackingInfo)
{
	if (!abpalContext || !trackingInfo)
	{
		return PAL_ErrBadParam;
	}

    return abpalContext->context->SuspendListener(trackingInfo);
}

PAL_Error ABPAL_GpsResume(ABPAL_GpsContext* abpalContext, const ABPAL_GpsTrackingInfo* trackingInfo)
{
	if (!abpalContext || !trackingInfo)
	{
		return PAL_ErrBadParam;
	}

    return abpalContext->context->ResumeListener(trackingInfo);
}

nb_boolean ABPAL_IsGpsEnabled(ABPAL_GpsContext* gpsContext)
{
    return (nb_boolean)(TRUE);
}

/*! @} */
