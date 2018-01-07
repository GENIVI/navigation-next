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

    @file     palgps.cpp
    @date     03/27/2012
    @defgroup PALGPS_H PAL GPS

    @brief    Provides a platform-dependent gps implementation

*/
/*
    See file description in header file.

    (C) Copyright 2012 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */

#include "palerror.h"
#include "abpalgps.h"
#include "palstdlib.h"
#include "gpscontext.h"
#include "pal.h"

struct ABPAL_GpsContext
{
    GpsContext* m_pGpsContext;
};

PAL_Error ABPAL_GpsCreate(PAL_Instance* pal, ABPAL_GpsContext** gpsContext)
{
    ABPAL_GpsContext* context = NULL;
    GpsContext* pGpsContext = NULL;
    if (pal == NULL ||
        gpsContext == NULL)
    {
        return PAL_ErrBadParam;
    }

    context = (ABPAL_GpsContext*)nsl_malloc(sizeof(ABPAL_GpsContext));
    if (context == NULL)
    {
        return PAL_ErrNoMem;
    }

    pGpsContext = new GpsContext(pal);
    if (pGpsContext == NULL)
    {
        return PAL_ErrNoMem;
    }

    context->m_pGpsContext = pGpsContext;
    *gpsContext = context;

    return PAL_Ok;
}

PAL_Error ABPAL_GpsInitialize(
        ABPAL_GpsContext* gpsContext,
        const ABPAL_GpsConfig* config,
        uint32 configCount,
        ABPAL_GpsInitializeCallback* userCallback,
        ABPAL_GpsDeviceStateCallback* userCallbackDevState,
        const void* userData)
{
    if (!gpsContext || !config)
    {
        return PAL_ErrBadParam;
    }

    if (gpsContext->m_pGpsContext == NULL)
    {
        return PAL_ErrNoInit;
    }

    return gpsContext->m_pGpsContext->initGpsConfig(config, configCount, userCallback, userCallbackDevState, userData);
}

PAL_Error ABPAL_GpsDestroy(ABPAL_GpsContext* gpsContext)
{
    if (gpsContext == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (gpsContext->m_pGpsContext)
    {
        delete gpsContext->m_pGpsContext;
        gpsContext->m_pGpsContext = NULL;
    }

    nsl_free(gpsContext);

    return PAL_Ok;
}


PAL_Error ABPAL_GpsGetState(ABPAL_GpsContext* gpsContext, ABPAL_GpsState* gpsState)
{
    if (gpsContext == NULL ||
        gpsState == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (gpsContext->m_pGpsContext == NULL)
    {
        return PAL_ErrNoInit;
    }

    *gpsState = gpsContext->m_pGpsContext->getState();;

    return PAL_Ok;
}

PAL_Error ABPAL_GpsGetLocation(
        ABPAL_GpsContext* gpsContext,
        const ABPAL_GpsCriteria* criteria,
        ABPAL_GpsLocationCallback* userCallback,
        const void* userData)
{
    if (gpsContext == NULL ||
        criteria == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (gpsContext->m_pGpsContext == NULL)
    {
        return PAL_ErrNoInit;
    }

    return gpsContext->m_pGpsContext->getLocation(criteria, userCallback, userData);
}

PAL_Error ABPAL_GpsCancelGetLocation(ABPAL_GpsContext* gpsContext)
{
    if (gpsContext == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (gpsContext->m_pGpsContext == NULL)
    {
        return PAL_ErrNoInit;
    }

    return gpsContext->m_pGpsContext->cancelGetLocation();
}

PAL_Error ABPAL_GpsBeginTracking(ABPAL_GpsContext* gpsContext,
    uint16 observeOnly,
    const ABPAL_GpsCriteria* criteria,
    ABPAL_GpsLocationCallback* userCallback,
    const void* userData,
    ABPAL_GpsTrackingInfo* trackingInfo)
{
    if (gpsContext == NULL ||
        criteria == NULL ||
        trackingInfo == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (gpsContext->m_pGpsContext == NULL)
    {
        return PAL_ErrNoInit;
    }

    return gpsContext->m_pGpsContext->beginTracking(criteria, userCallback, userData, trackingInfo);
}

PAL_Error ABPAL_GpsEndTracking(ABPAL_GpsContext* gpsContext, const ABPAL_GpsTrackingInfo* trackingInfo)
{
    if (gpsContext == NULL ||
        trackingInfo == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (gpsContext->m_pGpsContext == NULL)
    {
        return PAL_ErrNoInit;
    }

    return gpsContext->m_pGpsContext->endTracking(trackingInfo);
}

PAL_Error ABPAL_GpsUpdateCriteria(
        ABPAL_GpsContext* gpsContext,
        const ABPAL_GpsCriteria* criteria,
        const ABPAL_GpsTrackingInfo* trackingInfo)
{
    if (gpsContext == NULL ||
        criteria == NULL ||
        trackingInfo == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (gpsContext->m_pGpsContext == NULL)
    {
        return PAL_ErrNoInit;
    }

    return gpsContext->m_pGpsContext->updateCriteria(criteria, trackingInfo);
}

PAL_Error ABPAL_GpsSuspend(ABPAL_GpsContext* gpsContext, const ABPAL_GpsTrackingInfo* trackingInfo)
{
    if (gpsContext == NULL ||
        trackingInfo == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (gpsContext->m_pGpsContext == NULL)
    {
        return PAL_ErrNoInit;
    }

    return gpsContext->m_pGpsContext->gpsSuspend(trackingInfo);
}

PAL_Error ABPAL_GpsResume(ABPAL_GpsContext* gpsContext, const ABPAL_GpsTrackingInfo* trackingInfo)
{
    if (gpsContext == NULL ||
        trackingInfo == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (gpsContext->m_pGpsContext == NULL)
    {
        return PAL_ErrNoInit;
    }

    return gpsContext->m_pGpsContext->gpsResume(trackingInfo);
}

nb_boolean ABPAL_IsGpsEnabled(ABPAL_GpsContext* gpsContext)
{
    if (gpsContext == NULL ||
        gpsContext->m_pGpsContext == NULL)
    {
        return FALSE;
    }
    return TRUE;
}

PAL_Error ABPAL_EnableBackgroundGeolocation(ABPAL_GpsContext* gpsContext, nb_boolean isEnabled)
{
    return PAL_ErrUnsupported;
}


/*! @} */
