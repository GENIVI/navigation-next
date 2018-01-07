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

/*! 
    @file palgps.cpp
*/
/*
* Copyright (c) 2008 Networks In Motion, Inc.
* All rights reserved. This file and associated materials are the
* trade secrets, confidential information, and copyrighted works of
* Networks In Motion, Inc.
*
* This intellectual property is for the internal use only by Networks
* In Motion, Inc. This source code contains proprietary information
* of Networks In Motion, Inc., and shall not be used, copied by, or
* disclosed to, anyone other than a Networks In Motion, Inc.,
* certified employee that has written authorization to view or modify
* said source code.
*/

/*! @{ */

#include "palerror.h"
#include "abpalgps.h"
#include "gpsconfiguration.h"
#include "gpscontext.h"
#include <new>

struct ABPAL_GpsContext
{
    GPSContext *gpsContext;
};

PAL_Error ABPAL_GpsCreate(PAL_Instance* pal, ABPAL_GpsContext** gpsContext)
{
    PAL_Error result = PAL_ErrBadParam;

    if ((NULL != pal) && (NULL != gpsContext))
    {
        ABPAL_GpsContext *gpsPalContext = (ABPAL_GpsContext*)malloc(sizeof(*gpsPalContext));

        if (NULL != gpsPalContext)
        {
            gpsPalContext->gpsContext = new(std::nothrow) GPSContext(pal);
            if(NULL != gpsPalContext->gpsContext)
            {
                *gpsContext = gpsPalContext;
                result = PAL_Ok;
            }
            else
            {
                result = PAL_ErrNoMem;
                free(gpsPalContext);
            }
        }
        else
        {
            result = PAL_ErrNoMem;
        }
    }
    return result;
}

PAL_Error 
ABPAL_GpsInitialize(
    ABPAL_GpsContext* gpsContext,
    const ABPAL_GpsConfig* config,
    uint32 configCount,
    ABPAL_GpsInitializeCallback* userCallback,
    ABPAL_GpsDeviceStateCallback* userCallbackDevState,
    const void* userData)
{
    PAL_Error result = PAL_ErrBadParam;

    if ((NULL != gpsContext) && (NULL != config))
    {
        GpsConfiguration *configuration = new(std::nothrow) GpsConfiguration(configCount, config);
        if(NULL != configuration)
        {
            result = gpsContext->gpsContext->initGpsConfig(configuration, userCallback, userCallbackDevState, userData);
            delete configuration;
            configuration = NULL;
        }
    }

    return result;
}

PAL_Error ABPAL_GpsDestroy(ABPAL_GpsContext* gpsContext)
{
    if(NULL != gpsContext->gpsContext)
    {
        delete gpsContext->gpsContext;
        gpsContext->gpsContext = NULL;
    }
    if(NULL != gpsContext)
    {
        free(gpsContext);
        gpsContext = NULL;
    }
    return PAL_Ok;
}


PAL_Error ABPAL_GpsGetState(ABPAL_GpsContext* gpsContext, ABPAL_GpsState* gpsState)
{
    PAL_Error result = PAL_ErrBadParam;

    if ((NULL != gpsContext) && (NULL != gpsState))
    {
        *gpsState = gpsContext->gpsContext->handlerState();

        result = PAL_Ok;
    }
    return result;
}

PAL_Error ABPAL_GpsGetLocation(ABPAL_GpsContext* gpsContext, const ABPAL_GpsCriteria* criteria, ABPAL_GpsLocationCallback* userCallback, const void* userData)
{
    PAL_Error result = PAL_ErrBadParam;

    if ((NULL != gpsContext) && (NULL != criteria))
    {
        result =gpsContext->gpsContext->getLocation(criteria, userCallback, userData);
    }
    return result;
}

PAL_Error ABPAL_GpsCancelGetLocation(ABPAL_GpsContext* gpsContext)
{
    PAL_Error result = PAL_ErrBadParam;

    if (NULL != gpsContext)
    {
        result = gpsContext->gpsContext->cancelGetLocation();
    }

    return result;
}

PAL_Error ABPAL_GpsBeginTracking(ABPAL_GpsContext* gpsContext, uint16 observeOnly, const ABPAL_GpsCriteria* criteria, ABPAL_GpsLocationCallback* userCallback, const void* userData, ABPAL_GpsTrackingInfo* trackingInfo)
{
    PAL_Error result = PAL_ErrBadParam;

    if ((NULL != gpsContext) && (NULL != criteria) && (NULL != trackingInfo))
    {
        result = gpsContext->gpsContext->beginTracking(criteria, userCallback, userData, trackingInfo);
    }
    return result;
}

PAL_Error ABPAL_GpsEndTracking(ABPAL_GpsContext* gpsContext, const ABPAL_GpsTrackingInfo* trackingInfo)
{
    PAL_Error result = PAL_ErrBadParam;

    if ((NULL != gpsContext) && (NULL != trackingInfo))
    {
        result = gpsContext->gpsContext->endTracking(trackingInfo);
    }
    return result;
}

PAL_Error ABPAL_GpsUpdateCriteria(ABPAL_GpsContext* gpsContext, const ABPAL_GpsCriteria* criteria, const ABPAL_GpsTrackingInfo* trackingInfo)
{
    PAL_Error result = PAL_ErrBadParam;

    if (NULL != gpsContext)
    {
        result = gpsContext->gpsContext->updateCriteria(criteria, trackingInfo);
    }
    return result;
}

PAL_Error ABPAL_GpsSuspend(ABPAL_GpsContext* gpsContext, const ABPAL_GpsTrackingInfo* trackingInfo)
{
    PAL_Error result = PAL_ErrBadParam;
    if (gpsContext != NULL)
    {
        result = gpsContext->gpsContext->gpsSusPend(trackingInfo);
    }
    return result;
}

PAL_Error ABPAL_GpsResume(ABPAL_GpsContext* gpsContext, const ABPAL_GpsTrackingInfo* trackingInfo)
{
    PAL_Error result = PAL_ErrBadParam;
    if (gpsContext != NULL)
    {
        result = gpsContext->gpsContext->gpsResume(trackingInfo);
    }
    return result;
}

PAL_Error ABPAL_GpsPlaySkip(ABPAL_GpsContext* gpsContext, uint32 seconds)
{
    if (!gpsContext)
    {
        return PAL_ErrBadParam;
    }

    return PAL_Ok;
}

PAL_Error ABPAL_GpsPlayAccelerate(ABPAL_GpsContext* gpsContext, nb_boolean setAcceleratedMode)
{
    if (!gpsContext)
    {
        return PAL_ErrBadParam;
    }

    return PAL_Ok;
}

nb_boolean ABPAL_IsGpsEnabled(ABPAL_GpsContext* gpsContext)
{
    nb_boolean result = FALSE;

    if (gpsContext)
    {
        // TODO: implement this
        result = TRUE;
    }

    return result;
}

/*! @} */
