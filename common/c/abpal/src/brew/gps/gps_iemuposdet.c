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
#include "palclock.h"
#include "palfile.h"
#include "gpscontext.h"
#include "gps_iemuposdet.h"
#include "palmath.h"
#include "brewutil.h"

#define GPSSIM_INITIAL_DELAY 5

#define GPS_FILE_NAME "file"
#define GPS_START_TIMESTAMP "start"
#define GPS_END_TIMESTAMP "end"

PosDetEntity* IEmuPosDet_Create(PAL_Instance* pal, ABPAL_GpsContext* context)
{
    PosDetEntity* posdet = (PosDetEntity*) nsl_malloc(sizeof(PosDetEntity));
    IEMUPosDetParams* params = NULL;

    if (!posdet)
        return NULL;

    params = (IEMUPosDetParams*) nsl_malloc(sizeof(IEMUPosDetParams));
    if (!params)
    {
        nsl_free(posdet);
        return NULL;
    }

    posdet->pal = pal;
    posdet->context = context;
    posdet->ext_params = params;
    posdet->init_func = IEmuPosDet_Init;
    posdet->getfix_func = IEmuPosDet_GetFix;
    posdet->cancelgetfix_func = IEmuPosDet_CancelFix;
    posdet->setcriteria_func = IEmuPosDet_SetCriteria;
    posdet->destroy_func = IEmuPosDet_Destroy;
    posdet->setfixcb_func = IEmuPosDet_SetFixCB;
    posdet->setinitcb_func = IEmuPosDet_SetInitCB;
    posdet->setstatecb_func = IEmuPosDet_SetStateCB;
    posdet->setaccelerate_func = IEmuPosDet_SetAccelerateMode;
    posdet->setcontextstatecb_func = IEmuPosDet_SetUpdateStateCB;

    return posdet;
}

PAL_Error IEmuPosDet_Init(PosDetEntity* posdet, const ABPAL_GpsConfig* config, uint32 configCount)
{
    PAL_Error err = PAL_Ok;
    const char* filename=NULL;
    const char* start=NULL;
    const char* end=NULL;
    IEMUPosDetParams* params = NULL;
    uint32 br = 0;
    GPSFileRecord* pPos;
    uint32 time;

    if (!posdet)
        return PAL_Failed;

    params = (IEMUPosDetParams*) posdet->ext_params;

    filename = SearchConfig(config, configCount, GPS_FILE_NAME);

    if ( PAL_FileExists(posdet->pal, filename) == PAL_Ok)
    {
        nsl_free(params->pFixes);
        err =PAL_FileLoadFile( posdet->pal, filename, (unsigned char**) &params->pFixes, &br );
        if ( err != PAL_Ok || br <= 0 )
        {
            return PAL_Failed;
        }
        params->nFixes = br/sizeof(*params->pFixes);
        params->nOffset = 0;

        params->pStart = params->pFixes;
        params->pStop = params->pFixes + params->nFixes;

        start = SearchConfig(config, configCount, GPS_START_TIMESTAMP);
        if(start != NULL && nsl_strlen(start)!=0)
        {
            uint32 startTimeStamp = nsl_strtoul(start,NULL,10);
            for(pPos = params->pStart; pPos+1 < params->pStop && (pPos+1)->dwTimeStamp <= startTimeStamp; pPos++);
            params->pStart = pPos;
        }

        end =  SearchConfig(config, configCount, GPS_END_TIMESTAMP);
        if(end != NULL && nsl_strlen(end) !=0)
        {
            uint32 endTimeStamp = nsl_strtoul(end,NULL,10);
            for(pPos = params->pStop-1; pPos-1 >= params->pStart && (pPos-1)->dwTimeStamp >= endTimeStamp; pPos--);
            params->pStop = pPos;
        }

        params->pCur = NULL;
        params->acceleratedMode = FALSE;

        for (pPos = params->pStart; pPos < params->pStop && pPos->dwTimeStamp == 0; pPos++) {
            ; /* Empty*/
        }

        if (pPos < params->pStop && pPos > params->pStart && pPos->dwTimeStamp != 0) {

            time = pPos->dwTimeStamp;
            pPos--;

            for (; pPos>=params->pStart; pPos--) {
                time -= GPSSIM_INITIAL_DELAY;
                pPos->dwTimeStamp = time;
            }
        }
    }
    else
    {
        err = PAL_Failed;
    }

    posdet->updatestate_func(posdet->context, PGS_Initialized);
    if (posdet->initUserCallback)
    {
        posdet->initUserCallback(posdet->initUserData, PAL_Ok, NULL);
    }

    return err;
}

static void
GetFixTimerCB(void * pData)
{
    PosDetEntity* posdet = (PosDetEntity*) pData;

    if (posdet->locationUserCallback)
    {
        posdet->locationUserCallback( posdet->locationUserData, PAL_Ok, &posdet->gpsfix, NULL);
        posdet->updatestate_func(posdet->context, PGS_Initialized);
    }
}

PAL_Error IEmuPosDet_GetFix(PosDetEntity* posdet)
{
    IEMUPosDetParams* params = NULL;

    uint32    curgpstime = PAL_ClockGetGPSTime();

    uint32  wait;
    uint32  wait_msec;

    if (!posdet)
        return PAL_Failed;

    params = (IEMUPosDetParams*) posdet->ext_params;

    if (!params)
        return PAL_Failed;

    if (params->pCur == NULL) {
        params->nOffset = curgpstime - params->pStart->dwTimeStamp + GPSSIM_INITIAL_DELAY;
        params->pCur = params->pStart;
    }

    while (params->pCur < params->pStop && (params->pCur->dwTimeStamp+params->nOffset) < curgpstime)
        params->pCur++;

    if (params->pCur < params->pStop)
    {
        wait = params->pCur->dwTimeStamp + params->nOffset - curgpstime;
        if (params->acceleratedMode)
        {
            wait_msec = wait * 250;
        }
        else
        {
            wait_msec = wait * 1000;
        }

        posdet->gpsfix.status = params->pCur->status;
        posdet->gpsfix.gpsTime = PAL_ClockGetGPSTime();
        posdet->gpsfix.valid = params->pCur->fValid;
        posdet->gpsfix.latitude = nsl_mul(params->pCur->dwLat, GPS_LATITUDE_CONSTANT);
        posdet->gpsfix.longitude = nsl_mul(params->pCur->dwLon, GPS_LONGITUDE_CONSTANT);
        posdet->gpsfix.heading = nsl_mul(params->pCur->wHeading, GPS_HEADING_CONSTANT);
        posdet->gpsfix.horizontalVelocity = nsl_mul(params->pCur->wVelocityHor, GPS_HORIZONTAL_VEL_CONSTANT);
        posdet->gpsfix.altitude = nsl_sub(params->pCur->wAltitude, GPS_ALTITUDE_CONSTANT);
        posdet->gpsfix.verticalVelocity = nsl_mul(params->pCur->bVelocityVer, GPS_VERTICAL_VEL_CONSTANT);
        posdet->gpsfix.horizontalUncertaintyAngleOfAxis = nsl_mul(params->pCur->bHorUncAngle, GPS_HORIZONTALUNCANGLEOFAXIS_CONSTANT);
        posdet->gpsfix.horizontalUncertaintyAlongPerpendicular = uncdecode(params->pCur->bHorUncPerp);
        posdet->gpsfix.horizontalUncertaintyAlongAxis = uncdecode(params->pCur->bHorUnc);
        posdet->gpsfix.verticalUncertainty = uncdecode(params->pCur->wVerUnc);

        //posdet->gpsfix.utcOffset = params->pCur->
        //posdet->gpsfix.numberOfSatellites = params->pCur->wVerUnc;


        //TODO i dont like this
        ISHELL_SetTimer(PAL_GetShell(posdet->pal), wait_msec, GetFixTimerCB, posdet);
    }
    else
    {
        params->pCur = NULL;
        params->nOffset = 0;
    }

    posdet->updatestate_func(posdet->context, PGS_Oneshot);

    return PAL_Ok;
}

PAL_Error IEmuPosDet_SetAccelerateMode(PosDetEntity* posdet, uint8 accelerate)
{
    IEMUPosDetParams* params = NULL;

    if (!posdet)
        return PAL_Failed;

    params = (IEMUPosDetParams*) posdet->ext_params;

    if (!params)
        return PAL_Failed;

    params->acceleratedMode = accelerate;

    return PAL_Ok;
}

PAL_Error IEmuPosDet_CancelFix(PosDetEntity* posdet)
{
    if (!posdet)
        return PAL_Failed;
    return PAL_Ok;
}

PAL_Error IEmuPosDet_SetCriteria(PosDetEntity* posdet, GpsCrit* criteria)
{
    if (!posdet)
        return PAL_Failed;

    posdet->criteria.accuracy = criteria->accuracy;
    posdet->criteria.fixes = criteria->fixes;
    posdet->criteria.interval = criteria->interval;
    posdet->criteria.req = criteria->req;
    posdet->criteria.flags = criteria->flags;

    return PAL_Ok;
}

PAL_Error IEmuPosDet_SetFixCB(PosDetEntity* posdet, ABPAL_GpsLocationCallback* locationUserCallback, const void* locationUserData)
{
    if (!posdet)
        return PAL_Failed;

    posdet->locationUserCallback = locationUserCallback;
    posdet->locationUserData = locationUserData;

    return PAL_Ok;
}

PAL_Error IEmuPosDet_SetInitCB(PosDetEntity* posdet, ABPAL_GpsInitializeCallback* initUserCallback, const void* initUserData)
{
    if (!posdet)
        return PAL_Failed;

    posdet->initUserCallback = initUserCallback;
    posdet->initUserData = initUserData;

    return PAL_Ok;
}

PAL_Error IEmuPosDet_SetStateCB(PosDetEntity* posdet, ABPAL_GpsDeviceStateCallback* stateUserCallback, const void* stateUserData)
{
    if (!posdet)
        return PAL_Failed;

    posdet->stateUserCallback = stateUserCallback;
    posdet->stateUserData = stateUserData;

    return PAL_Ok;
}

PAL_Error IEmuPosDet_SetUpdateStateCB(PosDetEntity* posdet, GPSContextStateCallBack* contextStateCallBack)
{
    if (!posdet)
        return PAL_Failed;

    posdet->updatestate_func = contextStateCallBack;

    return PAL_Ok;
}

PAL_Error IEmuPosDet_Destroy(PosDetEntity* posdet)
{
    IEMUPosDetParams* params = NULL;

    if (!posdet)
        return PAL_Failed;

    params = (IEMUPosDetParams*) posdet->ext_params;

    nsl_free(params->pFixes);
    nsl_free(params);
    nsl_free(posdet);
    return PAL_Ok;
}