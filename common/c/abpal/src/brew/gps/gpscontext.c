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

#include "palstdlib.h"
#include "paltimer.h"
#include "gpscontext.h"
#include "gps_iposdet.h"
#include "gps_iemuposdet.h"
#include "brewutil.h"
#include "BREWVersion.h"

#define TRACKING_SESSION_ACTIVE 1
#define TRACKING_SESSION_INACTIVE 0

// According TIA/EIA IS-801 standard, Standard Deviation for Position Uncertainty (meters)
static const double unctbl[] = {0.5, 0.75, 1.0, 1.5, 2.0, 3.0, 4.0, 6.0, 8.0,
                                12.0, 16.0, 24.0, 32.0, 48.0, 64.0, 96.0, 128.0,
                                192.0, 256.0, 384.0, 512.0, 768.0, 1024.0, 1536.0,
                                2048.0, 3072.0, 4096.0, 6144.0, 8192.0, 12288.0};

static void gpsContext_TrackGetLocation(PAL_Instance *pal, void *userData,PAL_TimerCBReason reason);
static void gpsContext_TrackCB(const void* userData, PAL_Error error, const ABPAL_GpsLocation* location, const char* errorInfoXml);

ABPAL_GpsContext* gpsContext_Create(PAL_Instance* pal)
{
    GpsContext* context = (GpsContext*) nsl_malloc(sizeof(GpsContext));
    if (!context)
    {
        return NULL;
    }
    context->state = PGS_InitializePending;
    context->pal = pal;
    context->posdet = NULL;
    context->trackerList = PAL_VectorAlloc(sizeof(GpsTracker));
    context->Interval = 0;
    context->TotalFixes = 0;
    context->trackingSessionActive = TRACKING_SESSION_INACTIVE;
    return (ABPAL_GpsContext*) context;
}

PAL_Error gpsContext_Init(ABPAL_GpsContext* gpscontext, const ABPAL_GpsConfig* config, uint32 configCount, ABPAL_GpsInitializeCallback* userCallback, ABPAL_GpsDeviceStateCallback* userCallbackDevState,  const void* userData)
{
    PAL_Error err = PAL_Ok;
    GpsContext* context = (GpsContext*) gpscontext;
    char* type=NULL;

    if ( !context || !config )
    {
        return PAL_Failed;
    }

    context->palgpsconfig = config;
    context->configCount = configCount;

    type = SearchConfig(context->palgpsconfig, context->configCount, CFG_PROVIDER_TYPE);

    if ( (nsl_strcmp(CPT_POS, type) == 0) || (nsl_strcmp(CPT_POS_SGPS, type) == 0) )
    {
        if(context->posdet)
        {
         err = context->posdet->destroy_func(context->posdet);
        }
        context->posdet = IPosDet_Create( context->pal, gpscontext );
    }
    else if ( nsl_strcmp(CPT_EMU, type) == 0 )
    {
        if(context->posdet)
        {
            err = context->posdet->destroy_func(context->posdet);
        }
        context->posdet = IEmuPosDet_Create( context->pal, gpscontext );
    }

    if (context->posdet)
    {
        err = context->posdet->setcontextstatecb_func(context->posdet, gpsContext_UpdateState);
        err = context->posdet->setinitcb_func(context->posdet, userCallback, userData);
        err = context->posdet->setstatecb_func(context->posdet, userCallbackDevState, userData);
        err = context->posdet->init_func(context->posdet, context->palgpsconfig, context->configCount);
    }
    else
    {
        err = PAL_Failed;
    }

    return err;
}

PAL_Error gpsContext_Destroy(ABPAL_GpsContext* gpscontext)
{
    GpsContext* context = (GpsContext*) gpscontext;
    PAL_Error err = PAL_Ok;

    if (!context)
    {
        return PAL_Failed;
    }

    context->state = PGS_DestroyPending;

    if (context->posdet)
    {
        err = context->posdet->destroy_func(context->posdet);
    }

    PAL_VectorDealloc(context->trackerList);

    if (context && err == PAL_Ok)
    {
        nsl_free(context);
    }
    return err;
}

PAL_Error gpsContext_UpdateGPSCriteria(ABPAL_GpsContext* gpscontext, const ABPAL_GpsCriteria* criteria, const ABPAL_GpsTrackingInfo* trackingInfo)
{
    GpsContext* context = (GpsContext*) gpscontext;
    GpsCrit crit = {0};
    PAL_Error err = PAL_Ok;

    if (!context)
        return PAL_Failed;

    if (context->posdet)
    {
        crit.accuracy = criteria->desiredAccuracy;
        crit.req = criteria->desiredInformation;
        crit.profile = criteria->desiredSetting;
        crit.timeout = criteria->desiredTimeout;
        crit.maxAge = criteria->desiredMaxAge;
        err = context->posdet->setcriteria_func(context->posdet, &crit);
    }
    else
    {
        return PAL_Failed;
    }

    if (trackingInfo)
    {
        GpsTracker* tracker = (GpsTracker*) PAL_VectorGetPtr(context->trackerList, trackingInfo->Id);
        if (tracker)
        {
            tracker->criteria.accuracy = criteria->desiredAccuracy;
            tracker->criteria.fixes = criteria->desiredTotalFixes;
            tracker->criteria.interval = criteria->desiredInterval;
        }
    }

    return err;
}

PAL_Error gpsContext_GetLocation(ABPAL_GpsContext* gpscontext, ABPAL_GpsLocationCallback* locationUserCallback, const void* locationUserData)
{
    GpsContext* context = (GpsContext*) gpscontext;
    PAL_Error err = PAL_Ok;

    if (!context)
        return PAL_Failed;

    if (context->posdet)
    {
        err = context->posdet->setfixcb_func(context->posdet, locationUserCallback, locationUserData);
        err = context->posdet->getfix_func(context->posdet);
    }

    return err;
}

PAL_Error gpsContext_CancelGetLocation(ABPAL_GpsContext* gpscontext)
{
    GpsContext* context = (GpsContext*) gpscontext;
    PAL_Error err = PAL_Ok;

    if (!context)
            return PAL_Failed;

    if ((context->posdet) && (PAL_VectorLen(context->trackerList) == 0)) // Makes no sense to cancel a location in tracking mode -
    {                                                                    // we could occasionally kill the tracking
        err = context->posdet->cancelgetfix_func(context->posdet);
    }

    return err;
}

PAL_Error gpsContext_UpdateState(ABPAL_GpsContext* gpscontext, ABPAL_GpsState state)
{
    GpsContext* context = (GpsContext*) gpscontext;

    if (!context)
        return PAL_Failed;

    context->state = state;

    return PAL_Ok;
}

ABPAL_GpsState gpsContext_GetState(ABPAL_GpsContext* gpscontext)
{
    GpsContext* context = (GpsContext*) gpscontext;

    if (!context)
        return PGS_Undefined;

    return context->state;
}

PAL_Error gpsContext_Suspend(ABPAL_GpsContext* gpscontext, const ABPAL_GpsTrackingInfo* trackingInfo)
{
    GpsContext* context = (GpsContext*) gpscontext;
    if (!context)
    {
        return PAL_Failed;
    }

    if (trackingInfo == NULL)
    {
        if (context->trackingSessionActive == TRACKING_SESSION_ACTIVE)
        {
            context->trackingSessionActive = TRACKING_SESSION_INACTIVE;
            PAL_TimerCancel( context->pal, gpsContext_TrackGetLocation, (void*) context );
            return PAL_Ok;
        }
    }
    else
    {
        GpsTracker* tracker = (GpsTracker*)PAL_VectorGetPtr(context->trackerList, trackingInfo->Id);
        if (tracker)
        {
            tracker->isActive = TRACKING_SESSION_INACTIVE;
        }
    }

    return PAL_Failed;
}

PAL_Error gpsContext_Resume(ABPAL_GpsContext* gpscontext, const ABPAL_GpsTrackingInfo* trackingInfo)
{
    GpsContext* context = (GpsContext*) gpscontext;
    PAL_Error err = PAL_Ok;

    if (!context)
    {
        return PAL_Failed;
    }

    if (trackingInfo == NULL)
    {
        err = context->posdet->setfixcb_func(context->posdet, gpsContext_TrackCB, (void*) context);
        err = context->posdet->getfix_func(context->posdet);

        if (err == PAL_Ok)
        {
            context->trackingSessionActive = TRACKING_SESSION_ACTIVE;
        }
    }
    else
    {
        GpsTracker* tracker = (GpsTracker*)PAL_VectorGetPtr(context->trackerList, trackingInfo->Id);
        if (tracker)
        {
            tracker->isActive = TRACKING_SESSION_ACTIVE;
        }
    }

    return err;

}

PAL_Error gpsContext_SetAccelerateMode(ABPAL_GpsContext* gpsContext, uint8 setAcceleratedMode)
{
    GpsContext* context = (GpsContext*) gpsContext;
    PAL_Error err = PAL_Ok;
    if (!context || !context->posdet)
    {
        return PAL_Failed;
    }

    if ( context->posdet->setaccelerate_func)
    {
        err = context->posdet->setaccelerate_func(context->posdet, setAcceleratedMode);
    }

    return err;
}

PAL_Error gpsContext_BeginTracking(ABPAL_GpsContext* gpscontext, uint16 observeOnly, const ABPAL_GpsCriteria* criteria, ABPAL_GpsLocationCallback* userCallback, const void* userData, ABPAL_GpsTrackingInfo* trackingInfo)
{
    PAL_Error err = PAL_Ok;
    GpsContext* context = (GpsContext*) gpscontext;
    GpsTracker tracker = {0};

    if (!context)
        return PAL_Failed;

    tracker.observeOnly = observeOnly;
    tracker.isActive = TRACKING_SESSION_ACTIVE;
    tracker.criteria.accuracy = criteria->desiredAccuracy;
    tracker.criteria.fixes = criteria->desiredTotalFixes;
    tracker.criteria.interval = criteria->desiredInterval;
    tracker.locationUserCallback = userCallback;
    tracker.locationUserData = userData;

    // PAL_Vector makes its own copy of the item
    PAL_VectorAppend(context->trackerList, (void*)&tracker);

    trackingInfo->Id = PAL_VectorLen(context->trackerList);

    if (!observeOnly)
    {
        context->trackingSessionActive = TRACKING_SESSION_ACTIVE;
        context->TotalFixes = context->TotalFixes + criteria->desiredTotalFixes;
        if ( criteria->desiredInterval == 0 || context->Interval < criteria->desiredInterval )
            context->Interval = criteria->desiredInterval;
        err = context->posdet->setfixcb_func(context->posdet, gpsContext_TrackCB, (void*) context);
        err = context->posdet->getfix_func(context->posdet);
    }

    if (err != PAL_Ok)
    {
        gpsContext_EndTracking( (ABPAL_GpsContext*) context, trackingInfo);
    }

    return err;
}

PAL_Error gpsContext_EndTracking(ABPAL_GpsContext* gpscontext, const ABPAL_GpsTrackingInfo* trackingInfo)
{
    GpsContext* context = (GpsContext*) gpscontext;

    if (!context)
        return PAL_Failed;

    if ( trackingInfo == NULL )
    {
        PAL_VectorRemoveAll(context->trackerList);
    }
    else
    {
        PAL_VectorRemove(context->trackerList, trackingInfo->Id);
    }

    if (PAL_VectorLen(context->trackerList) <= 0)
    {
        PAL_TimerCancel( context->pal, gpsContext_TrackGetLocation, (void*) context );

        // There still could be an outstanding fix request
        context->posdet->cancelgetfix_func(context->posdet);

        context->TotalFixes = 0;
        context->Interval = 0;
        context->trackingSessionActive = TRACKING_SESSION_INACTIVE;
    }

    return PAL_Ok;

}

static void gpsContext_TrackGetLocation(PAL_Instance *pal, void *userData,PAL_TimerCBReason reason)
{
    GpsContext* context = (GpsContext*) userData;
    if (reason == PTCBR_TimerFired)
    {
        context->TotalFixes--;
        if (context->TotalFixes > 0)
        {
            context->posdet->setfixcb_func(context->posdet, gpsContext_TrackCB, (void*) context);
            context->posdet->getfix_func(context->posdet);
        }
        else
        {
            gpsContext_EndTracking( (ABPAL_GpsContext*)context, NULL );
        }
    }
}

static void gpsContext_TrackCB(const void* userData, PAL_Error error, const ABPAL_GpsLocation* location, const char* errorInfoXml)
{
    if (error == PAL_Ok)
    {
        GpsContext* context = (GpsContext*) userData;
        GpsTracker* tracker = NULL;
        int i;
        for (i=0;i<PAL_VectorLen(context->trackerList);i++)
        {
            tracker = (GpsTracker*)PAL_VectorGetPtr(context->trackerList, i);
            if (tracker && tracker->isActive == TRACKING_SESSION_ACTIVE)
            {
                tracker->locationUserCallback(tracker->locationUserData, PAL_Ok, location, NULL);
                tracker->criteria.fixes--;
                if ( tracker->criteria.fixes <= 0 )
                {
                    PAL_VectorRemove(context->trackerList, i);
                }
            }
        }
        PAL_TimerSet(context->pal, context->Interval, gpsContext_TrackGetLocation, (void*) context);
    }
}

double
uncdecode(uint8 unc)
{
    if (unc < (sizeof(unctbl) / sizeof(*unctbl)))
    {
        return (unctbl[unc]);
    }
    else
    {
        return (0.0);
    }
}

char* SearchConfig(const ABPAL_GpsConfig* config, uint32 configCount, char* key)
{
    uint32 i=0;
    for ( i=0; i<configCount; i++ )
    {
        if (config[i].name)
        {
            if ( nsl_strcmp(config[i].name, key) == 0 )
            {
                return config[i].value;
            }
        }
    }
    return NULL;
}

nb_boolean gpsContext_IsGpsEnabled(ABPAL_GpsContext* gpsContext)
{
    // if BREW version is earlier than 3.1.4 let's consider GPS is enabled
    boolean isEnabled = TRUE;

#if MIN_BREW_VERSIONEx(3,1,4)
    GpsContext* context = (GpsContext*) gpsContext;

    if (!context)
    {
        return (nb_boolean)(FALSE);
    }
    // GPS is enabled if client runs on simulator
    if (PAL_IsSimulator(context->pal) == FALSE)
    {
        int size = 1;
        boolean isEmergencyOnly = FALSE;
        IShell* pIShell = PAL_GetShell(context->pal);

        if (pIShell)
        {
            ISHELL_GetDeviceInfoEx(pIShell, AEE_DEVICEITEM_POSDET_EMERGENCY_ONLY, &isEmergencyOnly, &size);
            isEnabled = !isEmergencyOnly;
        }
    }
#endif

    return (nb_boolean)(isEnabled);
}
