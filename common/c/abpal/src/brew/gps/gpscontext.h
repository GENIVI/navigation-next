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



#ifndef GPSCONTEXT_H
#define GPSCONTEXT_H

#include "AEEPosDet.h"

#include "pal.h"
#include "palerror.h"
#include "abpalgps.h"
#include "palvector.h"

#define GPS_FIX_TIMEOUT 60

// Constants for the conversion AEEGPSInfo to ABPAL_GpsLocation
// For detailed information: TIA/EIA IS-801 standard
#define GPS_LATITUDE_CONSTANT                 (0.00000536441803)
#define GPS_LONGITUDE_CONSTANT                (0.00000536441803)
#define GPS_HEADING_CONSTANT                  (0.3515625)
#define GPS_HORIZONTAL_VEL_CONSTANT           (0.25)
#define GPS_VERTICAL_VEL_CONSTANT             (0.5)
#define GPS_HORIZONTALUNCANGLEOFAXIS_CONSTANT (5.625)
#define GPS_ALTITUDE_CONSTANT                 (500)

// According TIA/EIA IS-801 standard GPS_UNCERTAINTY_MAX_CONSTANT corresponds uncertainty more than 12,288
#define GPS_UNCERTAINTY_MAX_CONSTANT          (0x1E)

// According TIA/EIA IS-801 standard GPS_UNCERTAINTY_INCORRECT_CONSTANT corresponds not computable uncertainty
#define GPS_UNCERTAINTY_INCORRECT_CONSTANT    (0x1F)

#define CFG_PROVIDER_TYPE "providertype"
#define CPT_POS "pos"
#define CPT_EMU "emu"
#define CPT_POS_SGPS "pos-sgps"
#define CPT_POS_XTRA "pos-xtra"

typedef struct _GpsCrit GpsCrit;
typedef struct _PosDetEntity PosDetEntity;
typedef struct _GpsContext GpsContext;
typedef struct _GpsTracker GpsTracker;

typedef PAL_Error GPSContextStateCallBack(ABPAL_GpsContext* gpscontext, ABPAL_GpsState state);

typedef PAL_Error gps_init(PosDetEntity* posdet, const ABPAL_GpsConfig* config, uint32 configCount);
typedef PAL_Error gps_getfix(PosDetEntity* posdet);
typedef PAL_Error gps_cancelfix(PosDetEntity* posdet);
typedef PAL_Error gps_destroy(PosDetEntity* posdet);
typedef PAL_Error gps_setcriteria(PosDetEntity* posdet, GpsCrit* criteria);
typedef PAL_Error gps_setfixcb(PosDetEntity* posdet, ABPAL_GpsLocationCallback* locationUserCallback, const void* locationUserData);
typedef PAL_Error gps_setinitcb(PosDetEntity* posdet, ABPAL_GpsInitializeCallback* initUserCallback, const void* initUserData);
typedef PAL_Error gps_setstatecb(PosDetEntity* posdet, ABPAL_GpsDeviceStateCallback* stateUserCallback, const void* stateUserData);
typedef PAL_Error gps_setcontextstatecb(PosDetEntity* posdet, GPSContextStateCallBack* stateUserCallback);
typedef PAL_Error gps_setacceleratemode(PosDetEntity* posdet, uint8 accelerate);

typedef struct _GpsCrit
{
    uint16 accuracy;
    uint16 fixes;
    uint32 interval;
    uint16 profile;
    AEEGPSReq req;
    unsigned char flags;
    AEEGPSConfig gpsconfig;
    uint32 maxAge;
    uint32 timeout;
} GpsCrit;

typedef struct _PosDetEntity
{
    void* entity;
    ABPAL_GpsContext* context;
    PAL_Instance* pal;

    gps_init* init_func;
    gps_getfix* getfix_func;
    gps_cancelfix* cancelgetfix_func;
    gps_destroy* destroy_func;
    gps_setcriteria* setcriteria_func;
    gps_setfixcb* setfixcb_func;
    gps_setinitcb* setinitcb_func;
    gps_setstatecb* setstatecb_func;
    gps_setacceleratemode* setaccelerate_func;
    gps_setcontextstatecb* setcontextstatecb_func;

    GPSContextStateCallBack* updatestate_func;

    ABPAL_GpsLocation gpsfix;
    ABPAL_GpsState state;

    AEEGPSConfig gpsconfig;
    AEEGPSInfo gpsinfo;
    AEECallback gpsinfocb;
    AEECallback gpscachecb;
    GpsCrit criteria;

    const void* initUserData;
    const void* stateUserData;
    const void* locationUserData;
    ABPAL_GpsInitializeCallback* initUserCallback;
    ABPAL_GpsDeviceStateCallback* stateUserCallback;
    ABPAL_GpsLocationCallback* locationUserCallback;

    nb_gpsTime cachedFixTime;

    void* ext_params;

} PosDetEntity;

typedef struct _GpsTracker
{
    uint16 observeOnly;
    GpsCrit criteria;
    ABPAL_GpsLocationCallback* locationUserCallback;
    const void* locationUserData;
    ABPAL_GpsLocation gpsfix;
    uint16 isActive;
}GpsTracker;

typedef struct _GpsContext
{
    PAL_Instance* pal;
    PosDetEntity* posdet;
    const ABPAL_GpsConfig* palgpsconfig;
    uint32 configCount;
    ABPAL_GpsState state;
    PAL_Vector* trackerList;

    //TEMP
    uint16                  TotalFixes;
    uint16                  Interval;
    uint16                  Timeout;
    uint16                  trackingSessionActive;
} GpsContext;

ABPAL_GpsContext* gpsContext_Create(PAL_Instance* pal);
PAL_Error gpsContext_Init(ABPAL_GpsContext* gpscontext, const ABPAL_GpsConfig* config, uint32 configCount, ABPAL_GpsInitializeCallback* userCallback, ABPAL_GpsDeviceStateCallback* userCallbackDevState,  const void* userData);
PAL_Error gpsContext_SetInitCallback(ABPAL_GpsContext* gpscontext, ABPAL_GpsInitializeCallback* userCallback, const void* userData);
PAL_Error gpsContext_UpdateGPSCriteria(ABPAL_GpsContext* gpscontext, const ABPAL_GpsCriteria* criteria, const ABPAL_GpsTrackingInfo* trackingInfo);
PAL_Error gpsContext_GetLocation(ABPAL_GpsContext* gpscontext, ABPAL_GpsLocationCallback* locationUserCallback, const void* locationUserData);
PAL_Error gpsContext_CancelGetLocation(ABPAL_GpsContext* gpscontext);
ABPAL_GpsState gpsContext_GetState(ABPAL_GpsContext* gpscontext);
PAL_Error gpsContext_Destroy(ABPAL_GpsContext* gpscontext);
PAL_Error gpsContext_UpdateState(ABPAL_GpsContext* gpscontext, ABPAL_GpsState state);

PAL_Error gpsContext_BeginTracking(ABPAL_GpsContext* gpscontext, uint16 observeOnly, const ABPAL_GpsCriteria* criteria, ABPAL_GpsLocationCallback* userCallback, const void* userData, ABPAL_GpsTrackingInfo* trackingInfo);
PAL_Error gpsContext_EndTracking(ABPAL_GpsContext* gpscontext, const ABPAL_GpsTrackingInfo* trackingInfo);
PAL_Error gpsContext_Suspend(ABPAL_GpsContext* gpscontext, const ABPAL_GpsTrackingInfo* trackingInfo );
PAL_Error gpsContext_Resume(ABPAL_GpsContext* gpscontext, const ABPAL_GpsTrackingInfo* trackingInfo );

PAL_Error gpsContext_SetAccelerateMode(ABPAL_GpsContext* gpsContext, uint8 setAcceleratedMode);
nb_boolean gpsContext_IsGpsEnabled(ABPAL_GpsContext* gpsContext);

char* SearchConfig(const ABPAL_GpsConfig* config, uint32 configCount, char* key);
double uncdecode(uint8 unc);

#endif