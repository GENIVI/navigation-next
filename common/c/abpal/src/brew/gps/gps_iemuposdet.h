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

#ifndef GPS_IEMUPOSDET_H
#define GPS_IEMUPOSDET_H

#include "AEEShell.h"
#include "gpscontext.h"

typedef struct {
	uint32             dwTimeStamp;          // Time, seconds since 1/6/1980
	uint32             status;               // Response status;
	int32              dwLat;                // Lat, 180/2^25 degrees, WGS-84 ellipsoid
	int32              dwLon;                // Lon, 360/2^26 degrees, WGS-84 ellipsoid
	int16              wAltitude;            // Alt, meters, WGS-84 ellipsoid
	uint16             wHeading;             // Heading, 360/2^10 degrees
	uint16             wVelocityHor;         // Horizontal velocity, 0.25 meters/second
	int8               bVelocityVer;         // Vertical velocity, 0.5 meters/second
	AEEGPSAccuracy     accuracy;             // Accuracy of the data
	uint16             fValid;               // Flags indicating valid fields in the struct.
	uint8              bHorUnc;              // Horizontal uncertainity
	uint8              bHorUncAngle;         // Hor. Uncertainity at angle
	uint8              bHorUncPerp;          // Hor. uncertainty perpendicular
	uint16             wVerUnc;              // Vertical uncertainity.
} GPSFileRecord;

typedef struct _IEMUPosDetParams
{
	char*		gpsfilename;

	GPSFileRecord*		pFixes;
	GPSFileRecord*		pStart;
	GPSFileRecord*		pStop;
	GPSFileRecord*		pCur;
	uint32			nFixes;
	uint32			nOffset;
    nb_boolean      acceleratedMode;        

} IEMUPosDetParams;

PosDetEntity* IEmuPosDet_Create(PAL_Instance* pal, ABPAL_GpsContext* context);
PAL_Error IEmuPosDet_Init(PosDetEntity* posdet, const ABPAL_GpsConfig* config, uint32 configCount);
PAL_Error IEmuPosDet_Destroy(PosDetEntity* posdet);
PAL_Error IEmuPosDet_GetFix(PosDetEntity* posdet);
PAL_Error IEmuPosDet_SetCriteria(PosDetEntity* posdet, GpsCrit* criteria);
PAL_Error IEmuPosDet_CancelFix(PosDetEntity* posdet);
PAL_Error IEmuPosDet_SetFixCB(PosDetEntity* posdet, ABPAL_GpsLocationCallback* locationUserCallback, const void* locationUserData);
PAL_Error IEmuPosDet_SetInitCB(PosDetEntity* posdet, ABPAL_GpsInitializeCallback* initUserCallback, const void* initUserData);
PAL_Error IEmuPosDet_SetStateCB(PosDetEntity* posdet, ABPAL_GpsDeviceStateCallback* stateUserCallback, const void* stateUserData);
PAL_Error IEmuPosDet_SetAccelerateMode(PosDetEntity* posdet, uint8 accelerate);
PAL_Error IEmuPosDet_SetUpdateStateCB(PosDetEntity* posdet, GPSContextStateCallBack* contextStateCallBack);

#endif