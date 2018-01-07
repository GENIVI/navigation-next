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

#include "AEEPosDet.h"
#include "palstdlib.h"
#include "paltimer.h"
#include "gpscontext.h"
#include "gps_iposdet.h"
#include "brewutil.h"
#include "palclock.h"
#include "palmath.h"
#include "BREWVersion.h"

#define PDE_HOST_KEY "host"
#define PDE_IP_KEY "ip"
#define PDE_PORT_KEY "port"
#define PDE_MODE_KEY "mode"
#define PDE_NFIX_KEY "nfix"
#define PDE_NINT_KEY "nint"
#define PDE_NOPT_KEY "nopt"
#define PDE_NQOS_KEY "nqos"

typedef enum
{
    PP_OneShot=0,
    PP_OneShotFast,
    PP_OneShotInstant,
    PP_TrackInit,
    PP_Track,
    PP_TrackDataPending,
    PP_Invalid
} IPosdetProfile;

static void GetFixTimeOutCB(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);
static void GetFixCB(void *arg);
static void CachedFixCB(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);

static PAL_Error SetGpsProfile(PosDetEntity* posdet, IPosdetProfile profile);

static PAL_Error
SetGpsProfile(PosDetEntity* posdet, IPosdetProfile profile)
{
    if (!posdet || !posdet->entity)
        return PAL_Failed;

    switch (profile)
    {
        case PP_OneShot:
            posdet->gpsconfig.mode = 1;
            posdet->gpsconfig.qos = 16;
            break;
        case PP_OneShotFast:
            posdet->gpsconfig.mode = 1;
            posdet->gpsconfig.qos = 5;
            break;
        case PP_OneShotInstant:
            posdet->gpsconfig.mode = 1;
            posdet->gpsconfig.qos = 0;
            break;
        case PP_TrackInit:
            posdet->gpsconfig.mode = 1;
            posdet->gpsconfig.qos = 5;
            break;
        case PP_Track:
            posdet->gpsconfig.mode = 4;
            posdet->gpsconfig.qos = 5;
            break;
        case PP_TrackDataPending:
            posdet->gpsconfig.mode = 4;
            posdet->gpsconfig.qos = 5;
            break;
        default:
            break;
    }

    posdet->gpsconfig.optim = 1;
    posdet->gpsconfig.nFixes = 14401;
    posdet->gpsconfig.nInterval = 1;

    if ( IPOSDET_SetGPSConfig((IPosDet*)posdet->entity, &posdet->gpsconfig) != SUCCESS )
    {
        return PAL_Failed;
    }

    return PAL_Ok;

}

PosDetEntity* IPosDet_Create(PAL_Instance* pal, ABPAL_GpsContext* context)
{
    PosDetEntity* ipos = (PosDetEntity*) nsl_malloc(sizeof(PosDetEntity));

    if (!ipos)
        return NULL;

    nsl_memset(ipos, 0, sizeof(PosDetEntity));

    ipos->entity = (IPosDet*) ab_createinstance(pal, AEECLSID_POSDET);

    if    (!ipos->entity)
    {
        nsl_free(ipos);
        return NULL;
    }

    ipos->pal = pal;
    ipos->context = context;

    ipos->init_func = IPosDet_Init;
    ipos->getfix_func = IPosDet_GetFix;
    ipos->cancelgetfix_func = IPosDet_CancelFix;
    ipos->setcriteria_func = IPosDet_SetCriteria;
    ipos->destroy_func = IPosDet_Destroy;
    ipos->setfixcb_func = IPosDet_SetFixCB;
    ipos->setinitcb_func = IPosDet_SetInitCB;
    ipos->setstatecb_func = IPosDet_SetStateCB;
    ipos->setcontextstatecb_func = IPosDet_SetUpdateStateCB;
    ipos->setaccelerate_func = NULL;

    return ipos;
}

PAL_Error IPosDet_Init(PosDetEntity* posdet, const ABPAL_GpsConfig* config, uint32 configCount)
{
    PAL_Error err = PAL_Ok;
    const char* type = NULL;
    const char* ip=NULL;
    const char* port=NULL;
    const char* nfix=NULL;
    const char* nint=NULL;
    const char* nopt=NULL;
    const char* nqos=NULL;
    int nport = 0;
    int ret = SUCCESS;

    nsl_memset(&posdet->gpsinfocb, 0, sizeof(posdet->gpsinfocb));
    CALLBACK_Init(&posdet->gpsinfocb, (PFNNOTIFY)GetFixCB, posdet);

     if ( IPOSDET_GetGPSConfig((IPosDet*)posdet->entity, &posdet->gpsconfig) != SUCCESS )
     {
         IPOSDET_Release((IPosDet*)posdet->entity);
         posdet->entity = NULL;
         return PAL_Failed;
     }

     type = SearchConfig(config, configCount, CFG_PROVIDER_TYPE);

     if(nsl_strcmp(CPT_POS_SGPS, type) == 0)
     {
#if GPS_STANDALONE_SUPPORTED
         posdet->gpsconfig.mode = AEEGPS_MODE_TRACK_STANDALONE;
         // Server-related values are not used in SGPS
         posdet->gpsconfig.server.svrType = AEEGPS_SERVER_DEFAULT;
#else
         return PAL_ErrUnsupported;
#endif
     }
     else
     {
         if(nsl_strcmp(CPT_POS_XTRA, type) == 0)
         {
            posdet->gpsconfig.mode = AEEGPS_MODE_TRACK_LOCAL;
         }
         else
         {
            posdet->gpsconfig.mode = AEEGPS_MODE_ONE_SHOT;
         }

         ip = SearchConfig(config, configCount, PDE_IP_KEY);
         port = SearchConfig(config, configCount, PDE_PORT_KEY);

         if (!ip || !port)
         {
             return PAL_Failed;
         }

         if (!INET_ATON(ip, &posdet->gpsconfig.server.svr.ipsvr.addr))
         {
            err = PAL_Failed;
         }
         posdet->gpsconfig.server.svrType = AEEGPS_SERVER_IP;

         nport = ATOI(port);
         posdet->gpsconfig.server.svr.ipsvr.port = nsl_htons((uint16) nport);

         nfix = SearchConfig(config, configCount, PDE_NFIX_KEY);
         posdet->gpsconfig.nFixes = (uint16)ATOI(nfix);
         nint = SearchConfig(config, configCount, PDE_NINT_KEY);
         posdet->gpsconfig.nInterval = (uint16)ATOI(nint);
    }

    nqos = SearchConfig(config, configCount, PDE_NQOS_KEY);
    posdet->gpsconfig.qos = (AEEGPSQos)ATOI(nqos);

    nopt = SearchConfig(config, configCount, PDE_NOPT_KEY);
    posdet->gpsconfig.optim = (AEEGPSOpt)ATOI(nopt);

    ret = IPOSDET_SetGPSConfig((IPosDet*)posdet->entity, &posdet->gpsconfig);
    if ( ret != SUCCESS )
    {
        IPOSDET_Release((IPosDet*)posdet->entity);
        return PAL_Failed;
    }

    posdet->updatestate_func(posdet->context, PGS_Initialized);
    if (posdet->initUserCallback)
    {
        posdet->initUserCallback(posdet->initUserData, PAL_Ok, NULL);
    }
    return err;
}

nb_boolean IPosDet_HasMaxAgeExpired(PosDetEntity* posdet)
{
    if (posdet->criteria.maxAge == 0)
    {
        return TRUE;
    }

    return PAL_ClockGetGPSTime() > posdet->cachedFixTime + posdet->criteria.maxAge;
}

PAL_Error IPosDet_GetFix(PosDetEntity* posdet)
{
    int err = SUCCESS;

    if (!posdet || !posdet->entity)
        return PAL_Failed;


    if (posdet->cachedFixTime && posdet->gpsfix.valid && (IPosDet_HasMaxAgeExpired(posdet) == FALSE))
    {
        // It is ok to return cached location to the user
        PAL_TimerSet(posdet->pal, 0, CachedFixCB, (void*)posdet);
        posdet->updatestate_func(posdet->context, PGS_Oneshot);
        return PAL_Ok;
    }

    if (posdet->state != PGS_Oneshot)
    {
        // If there was no outstanding GPS request issued, start it.
        err = IPOSDET_GetGPSInfo((IPosDet*)posdet->entity, posdet->criteria.req, (AEEGPSAccuracy) posdet->criteria.accuracy, &posdet->gpsinfo, &posdet->gpsinfocb);
    }
    else
    {
        // Otherwise, we'll just reset the timer, but will still wait for previous request
        // We're trying to avoid cancelling GPS requests (beyond the case of destroying GPS context)
        err = PAL_TimerCancel(posdet->pal, GetFixTimeOutCB, posdet);
    }

    if (err != SUCCESS)
        return PAL_Failed;

    posdet->updatestate_func(posdet->context, PGS_Oneshot);
    return PAL_TimerSet(posdet->pal,
                posdet->criteria.timeout ? posdet->criteria.timeout : GPS_FIX_TIMEOUT * 1000,
                GetFixTimeOutCB, (void*) posdet);
}

PAL_Error IPosDet_CancelFix(PosDetEntity* posdet)
{
    int err = PAL_Ok;
    if (!posdet || !posdet->entity)
        return PAL_Failed;

    // Do not cancel the fix - but remove the user callback.
    // If we get the fix, it will be in the ABPAL internal cache.
    posdet->locationUserCallback = NULL;
    err = PAL_TimerCancel(posdet->pal, CachedFixCB, posdet);
    err |= PAL_TimerCancel(posdet->pal, GetFixTimeOutCB, posdet);

    return (err == PAL_Ok ? PAL_Ok : PAL_Failed);
}

PAL_Error IPosDet_SetCriteria(PosDetEntity* posdet, GpsCrit* criteria)
{
    if (!posdet)
        return PAL_Failed;

    posdet->criteria.accuracy = criteria->accuracy;
    posdet->criteria.fixes = criteria->fixes;
    posdet->criteria.interval = criteria->interval;
    posdet->criteria.req = criteria->req;
    posdet->criteria.flags = criteria->flags;
    posdet->criteria.profile = criteria->profile;
    posdet->criteria.maxAge = criteria->maxAge;
    posdet->criteria.timeout = criteria->timeout;

#if GPS_STANDALONE_SUPPORTED
    if (posdet->gpsconfig.mode != AEEGPS_MODE_TRACK_STANDALONE) // Currently, we do not change parameters for GSPS,
    {                                                           // use QOS/optim values set at initialization,
        SetGpsProfile(posdet, posdet->criteria.profile);        // and timeout from criteria
    }
#endif

    return PAL_Ok;
}

PAL_Error IPosDet_SetFixCB(PosDetEntity* posdet, ABPAL_GpsLocationCallback* locationUserCallback, const void* locationUserData)
{
     if (!posdet)
        return PAL_Failed;

    posdet->locationUserCallback = locationUserCallback;
    posdet->locationUserData = locationUserData;

    return PAL_Ok;
}

PAL_Error IPosDet_SetInitCB(PosDetEntity* posdet, ABPAL_GpsInitializeCallback* initUserCallback, const void* initUserData)
{
    if (!posdet)
        return PAL_Failed;


    posdet->initUserCallback = initUserCallback;
    posdet->initUserData = initUserData;

    return PAL_Ok;
}

PAL_Error IPosDet_SetStateCB(PosDetEntity* posdet, ABPAL_GpsDeviceStateCallback* stateUserCallback, const void* stateUserData)
{
    if (!posdet)
        return PAL_Failed;

    posdet->stateUserCallback = stateUserCallback;
    posdet->stateUserData = stateUserData;

    return PAL_Ok;
}

PAL_Error IPosDet_SetUpdateStateCB(PosDetEntity* posdet, GPSContextStateCallBack* contextStateCallBack)
{
    if (!posdet)
        return PAL_Failed;

    posdet->updatestate_func = contextStateCallBack;

    return PAL_Ok;
}

PAL_Error IPosDet_Destroy(PosDetEntity* posdet)
{
    if (!posdet)
        return PAL_Failed;
    CALLBACK_Cancel(&posdet->gpsinfocb);
    if (posdet->entity)
    {
        IPOSDET_Release((IPosDet*)posdet->entity);
    }
    PAL_TimerCancel(posdet->pal, CachedFixCB, posdet);
    PAL_TimerCancel(posdet->pal, GetFixTimeOutCB, posdet);
    nsl_free(posdet);
    posdet = NULL;
    return PAL_Ok;
}

static void CachedFixCB(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    PosDetEntity* posdet = (PosDetEntity*) userData;
    int err = SUCCESS;

    if ((reason == PTCBR_TimerFired) && posdet->locationUserCallback)
    {
        posdet->locationUserCallback(posdet->locationUserData, PAL_Ok, &posdet->gpsfix, NULL);
        posdet->locationUserCallback = NULL;

        // Do a background fix just to refresh the cache. The result will not be translated to the client, we've just nullified its callback
        err = IPOSDET_GetGPSInfo((IPosDet*)posdet->entity, posdet->criteria.req, (AEEGPSAccuracy) posdet->criteria.accuracy, &posdet->gpsinfo, &posdet->gpsinfocb);
        if (err != SUCCESS)
        {
            // Reset the state
            posdet->updatestate_func(posdet->context, PGS_Initialized);
        }
        else
        {
            // After the fix is completed or timed-out, state will be reset
            PAL_TimerSet(posdet->pal,
                    posdet->criteria.timeout ? posdet->criteria.timeout : GPS_FIX_TIMEOUT * 1000,
                    GetFixTimeOutCB, (void*) posdet);
        }
    }
}

static void GetFixCB(void *arg)
{
    PosDetEntity* posdet = (PosDetEntity*) arg;
    PAL_Error err = PAL_Ok;

    PAL_TimerCancel(posdet->pal, GetFixTimeOutCB, posdet);

    // if the fix comes in with some info missing, go ahead and use it if the lat/lon appear to be valid
    if ( (posdet->gpsinfo.status == AEEGPS_ERR_ACCURACY_UNAVAIL || posdet->gpsinfo.status == AEEGPS_ERR_INFO_UNAVAIL) &&
        (posdet->gpsinfo.fValid & (AEEGPS_VALID_LAT | AEEGPS_VALID_LON)) &&
        posdet->gpsinfo.dwLat != 0 && posdet->gpsinfo.dwLon != 0 )
    {
        posdet->gpsinfo.status = AEEGPS_ERR_NO_ERR;
    }

    // if the lat and lon are exactly zero, mark as invalid
    if ((posdet->gpsinfo.status == AEEGPS_ERR_NO_ERR) &&
        (posdet->gpsinfo.dwLat == 0 && posdet->gpsinfo.dwLon == 0) &&
        (posdet->gpsinfo.fValid & (AEEGPS_VALID_LAT | AEEGPS_VALID_LON)))
    {
        posdet->gpsinfo.status = AEEGPS_ERR_INFO_UNAVAIL;
        err = PAL_ErrGpsInformationUnavailable;
    }

    if (((posdet->gpsinfo.fValid & AEEGPS_VALID_AUNC) &&
         (posdet->gpsinfo.bHorUnc == GPS_UNCERTAINTY_MAX_CONSTANT ||
          posdet->gpsinfo.bHorUnc == GPS_UNCERTAINTY_INCORRECT_CONSTANT)) ||
        ((posdet->gpsinfo.fValid & AEEGPS_VALID_PUNC) &&
         (posdet->gpsinfo.bHorUncPerp == GPS_UNCERTAINTY_MAX_CONSTANT ||
          posdet->gpsinfo.bHorUncPerp == GPS_UNCERTAINTY_INCORRECT_CONSTANT)) ||
        ((posdet->gpsinfo.fValid & AEEGPS_VALID_VUNC) &&
         (posdet->gpsinfo.wVerUnc == GPS_UNCERTAINTY_MAX_CONSTANT ||
          posdet->gpsinfo.wVerUnc == GPS_UNCERTAINTY_INCORRECT_CONSTANT)))
    {
        posdet->gpsinfo.status = AEEGPS_ERR_INFO_UNAVAIL;
        err = PAL_ErrGpsInformationUnavailable;
    }

    posdet->gpsfix.status = posdet->gpsinfo.status;
    posdet->gpsfix.gpsTime = posdet->gpsinfo.dwTimeStamp;
    posdet->gpsfix.valid = posdet->gpsinfo.fValid;
    posdet->gpsfix.latitude = nsl_mul(posdet->gpsinfo.dwLat, GPS_LATITUDE_CONSTANT);
    posdet->gpsfix.longitude = nsl_mul(posdet->gpsinfo.dwLon, GPS_LONGITUDE_CONSTANT);
    posdet->gpsfix.heading = nsl_mul(posdet->gpsinfo.wHeading, GPS_HEADING_CONSTANT);
    posdet->gpsfix.horizontalVelocity = nsl_mul(posdet->gpsinfo.wVelocityHor, GPS_HORIZONTAL_VEL_CONSTANT);
    posdet->gpsfix.altitude = nsl_sub(posdet->gpsinfo.wAltitude, GPS_ALTITUDE_CONSTANT);
    posdet->gpsfix.verticalVelocity = nsl_mul(posdet->gpsinfo.bVelocityVer, GPS_VERTICAL_VEL_CONSTANT);
    posdet->gpsfix.horizontalUncertaintyAngleOfAxis = nsl_mul(posdet->gpsinfo.bHorUncAngle, GPS_HORIZONTALUNCANGLEOFAXIS_CONSTANT);
    posdet->gpsfix.horizontalUncertaintyAlongPerpendicular = uncdecode(posdet->gpsinfo.bHorUncPerp);
    posdet->gpsfix.horizontalUncertaintyAlongAxis = uncdecode(posdet->gpsinfo.bHorUnc);
    posdet->gpsfix.verticalUncertainty = uncdecode(posdet->gpsinfo.wVerUnc);
    //not supported in versions older than BREW314
    #if 0
    posdet->gpsfix.utcOffset = posdet->gpsinfo.GPS_UTCOffset;
    #endif

    posdet->cachedFixTime = PAL_ClockGetGPSTime();

    if (posdet->locationUserCallback)
    {
        posdet->locationUserCallback( posdet->locationUserData, err, &posdet->gpsfix, NULL);
        posdet->updatestate_func(posdet->context, PGS_Initialized);
    }
}

static void GetFixTimeOutCB( PAL_Instance *pal, void *userData, PAL_TimerCBReason reason )
{
    PosDetEntity* posdet = (PosDetEntity*) userData;
    if (reason == PTCBR_TimerFired)
    {
        // GPS Fix request timed out.
        // Cancel the request to GPS driver
        CALLBACK_Cancel(&posdet->gpsinfocb);

        // Reset the state
        posdet->updatestate_func(posdet->context, PGS_Initialized);

        // Notify user
        posdet->locationUserCallback( posdet->locationUserData, PAL_ErrGpsTimeout, NULL, NULL );
    }
}
