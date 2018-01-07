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

 @file abpalgps.m
 @date 9/13/10

 ABPAL interface functions

 */
/*
    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret
    as defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly prescribing
    the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */


#import "abpalgps.h"
#import "gpsconfiguration.h"
#import "gpscontext.h"

struct ABPAL_GpsContext
{
    GPSContext *gpsContext;
};


ABPAL_DEC PAL_Error ABPAL_GpsCreate(PAL_Instance* pal,ABPAL_GpsContext** gpsContext)
{

    PAL_Error result = PAL_ErrBadParam;

    if ((nil != pal) && (nil != gpsContext))
    {
        ABPAL_GpsContext *gpsPalContext =malloc(sizeof(*gpsPalContext));

        if (nil != gpsPalContext)
        {
            gpsPalContext->gpsContext = [[GPSContext alloc] init:pal];
            *gpsContext = gpsPalContext;
            result = PAL_Ok;
        }
        else
        {
            result = PAL_ErrNoMem;
        }
    }
    return result;
}

ABPAL_DEC PAL_Error ABPAL_GpsInitialize(
                    ABPAL_GpsContext* gpsContext,
                    const ABPAL_GpsConfig* config,
                    uint32 configCount,
                    ABPAL_GpsInitializeCallback* userCallback,
                    ABPAL_GpsDeviceStateCallback* userCallbackDevState,
                    const void* userData
                    )
{

    PAL_Error result = PAL_ErrBadParam;

    if (nil != gpsContext)
    {
        GpsConfiguration *configuration = [[GpsConfiguration alloc] initWithConfigCount:configCount gpsConfigPointer:config];

        result = [gpsContext->gpsContext initGpsConfig:configuration initlizeCallBack:userCallback deviceStateCallback:userCallbackDevState userData:userData];

        [configuration release];
    }

    return result;
}

ABPAL_DEC PAL_Error ABPAL_GpsDestroy(ABPAL_GpsContext* gpsContext)
{
    [gpsContext->gpsContext release];
    free(gpsContext);
    return PAL_Ok;
}

ABPAL_DEC PAL_Error ABPAL_GpsGetState(ABPAL_GpsContext* gpsContext,ABPAL_GpsState* gpsState)
{
    PAL_Error result = PAL_ErrBadParam;

    if ((nil != gpsContext) && (nil != gpsState))
    {
        *gpsState = [gpsContext->gpsContext  handlerState];

        result = PAL_Ok;
    }
    return result;
}

ABPAL_DEC PAL_Error ABPAL_GpsGetLocation(ABPAL_GpsContext* gpsContext,const ABPAL_GpsCriteria* criteria,ABPAL_GpsLocationCallback* userCallback,const void* userData)
{
    PAL_Error result = PAL_ErrBadParam;

    if ((nil != gpsContext) && (nil != criteria))
    {
    result    = [gpsContext->gpsContext getLocation:criteria locationCallback:userCallback userData:userData];
    }
    return result;
}

ABPAL_DEC PAL_Error ABPAL_GpsCancelGetLocation(ABPAL_GpsContext* gpsContext)
{
    PAL_Error result = PAL_ErrBadParam;

    if (nil != gpsContext)
    {
        result = [gpsContext->gpsContext cancelGetLocation];
    }

    return result;
}


ABPAL_DEC PAL_Error ABPAL_GpsBeginTracking(ABPAL_GpsContext* gpsContext,uint16 observeOnly,const ABPAL_GpsCriteria* criteria,
                       ABPAL_GpsLocationCallback* userCallback,const void* userData,ABPAL_GpsTrackingInfo* trackingInfo)
{

    PAL_Error result = PAL_ErrBadParam;

    if ((nil != gpsContext) && (nil != criteria) && (nil != trackingInfo))
    {
        result = [gpsContext->gpsContext beginTracking:criteria locationCallback:userCallback userData:userData trackingInfo:trackingInfo];
    }
    return result;
}

ABPAL_DEC PAL_Error ABPAL_GpsEndTracking(ABPAL_GpsContext* gpsContext,const ABPAL_GpsTrackingInfo* trackingInfo)
{
    PAL_Error result = PAL_ErrBadParam;

    if ((nil != gpsContext) && (nil != trackingInfo))
    {
        result = [gpsContext->gpsContext endTracking:trackingInfo];
    }
    return result;
}


ABPAL_DEC PAL_Error ABPAL_GpsUpdateCriteria(ABPAL_GpsContext* gpsContext,const ABPAL_GpsCriteria* criteria,
                        const ABPAL_GpsTrackingInfo* trackingInfo)
{

    PAL_Error result = PAL_ErrBadParam;

    if (nil != gpsContext)
    {
        result = [gpsContext->gpsContext updateCriteria:criteria trackingInfo:trackingInfo];
    }
    return result;

}

ABPAL_DEC PAL_Error ABPAL_GpsSuspend(ABPAL_GpsContext* gpsContext,const ABPAL_GpsTrackingInfo* trackingInfo)
{
    PAL_Error result = PAL_ErrBadParam;
    if (gpsContext != NULL)
    {
        result = [gpsContext->gpsContext gpsSusPend:trackingInfo];
    }
    return result;
}

ABPAL_DEF PAL_Error ABPAL_GpsResume(ABPAL_GpsContext* gpsContext, const ABPAL_GpsTrackingInfo* trackingInfo)
{
    PAL_Error result = PAL_ErrBadParam;
    if (gpsContext != NULL)
    {
        result = [gpsContext->gpsContext gpsResume:trackingInfo];
    }
    return result;
}

ABPAL_DEF nb_boolean
ABPAL_IsGpsEnabled(ABPAL_GpsContext* gpsContext)
{
    nb_boolean result = FALSE;

    if (gpsContext)
    {
        // TODO: implement this
        result = TRUE;
    }

    return result;
}

ABPAL_DEC PAL_Error
ABPAL_EnableBackgroundGeolocation(ABPAL_GpsContext* gpsContext, nb_boolean isEnabled)
{
    // TODO:
    return PAL_Ok;
}
