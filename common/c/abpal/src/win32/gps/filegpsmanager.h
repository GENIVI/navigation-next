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

 @file filegpsmanager.h
 @date 4/18/12

 FileGPS manager

 */
/*
 (C) Copyright 2012 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret
 as defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems is granted only
 under a written non-disclosure agreement, expressly prescribing
 the scope and manner of such use.

 ---------------------------------------------------------------------------*/

/*! @{ */
#ifndef FILEGPSMANAGER_H
#define FILEGPSMANAGER_H

#ifdef __cplusplus
extern "C"
	{
#endif

#include "paltypes.h"
#include "abpalgps.h"
#include "gpsprotocol.h"
#include "palfile.h"
#include "windows.h"

class GpsConfiguration;
class FileGpsHandler;

typedef struct
{
    uint32             dwTimeStamp;          // Time, seconds since 1/6/1980
    uint32             status;               // Response status;
    int32              dwLat;                // Lat, 180/2^25 degrees, WGS-84 ellipsoid
    int32              dwLon;                // Lon, 360/2^26 degrees, WGS-84 ellipsoid
    int16              wAltitude;            // Alt, meters, WGS-84 ellipsoid
    uint16             wHeading;             // Heading, 360/2^10 degrees
    uint16             wVelocityHor;         // Horizontal velocity, 0.25 meters/second
    int8               bVelocityVer;         // Vertical velocity, 0.5 meters/second
    int8               accuracy;             // Accuracy of the data
    uint16             fValid;               // Flags indicating valid fields in the struct.
    uint8              bHorUnc;              // Horizontal uncertainty
    uint8              bHorUncAngle;         // Horizontal Uncertainty at angle
    uint8              bHorUncPerp;          // Horizontal uncertainty perpendicular
    uint8              bNothing;
    uint16             wVerUnc;              // Vertical uncertainty.
} GPSFileRecord;


class FileGpsManager : public GpsManagerProtocol
{
public:
    FileGpsManager(PAL_Instance* pal);
    virtual ~FileGpsManager();

    PAL_Error readNextPosition(ABPAL_GpsLocation& location);
    void upDateGpsOffset(int offset);
    void convert(const GPSFileRecord& record, ABPAL_GpsLocation& location);

    //GpsManagerProtocol interfaces
    virtual void addObserver(GpsContextCallBackProtocol* observer);
    virtual PAL_Error start();
    virtual PAL_Error cancelRequest(GpsContextCallBackProtocol* observer);
    virtual PAL_Error setSimulationConfig(const GpsConfiguration* config);
    virtual void updateGpsCriteria();
    virtual PAL_Error cancelGetLocation();
    virtual PAL_Error endTracking();
    virtual BOOL isServiceEnable();
public:
    void upDateGpsFileName(const char* name);

    FileGpsHandler *gpsFileHandler;
    PAL_Instance *pal_Instance;
    GpsContextCallBackProtocol* gpsContext_Instance;
    int maxCount;

    UINT _timer;
    ABPAL_GpsCriteria  criteria;
    PAL_File *palfile;
};

#ifdef __cplusplus
    }
#endif

#endif //FILEGPSMANAGER_H