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

    @file     filegpsprovider.cpp
    @date     09/11/2012
    @defgroup FILEGPSPROVIDER_H GPS File
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

#include "filegpsprovider.h"
#include "palclock.h"
#include "abpalgpsutils.h"

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
    uint8              bUTCofset;            // UTC ofset
    uint16             wVerUnc;              // Vertical uncertainty.
} GPSFileRecord;

const static double Gps_Latitude_Constant                 = 0.00000536441803;
const static double Gps_Longitude_Constant                = 0.00000536441803;
const static double Gps_Heading_Constant                  = 0.3515625;
const static double Gps_Horizontal_Vel_Constant           = 0.25;
const static double Gps_Vertical_Vel_Constant             = 0.5;
const static double Gps_Horizontal_Unc_Angle_Of_AxisC_onstant = 5.625;
const static double Gps_Altitude_Constant                 = 500;
const static int    Gps_Fake_Num_Of_Satellites            = 4;

// According TIA/EIA IS-801 standard, Standard Deviation for Position Uncertainty (meters)
static const double unctbl[] = {0.5, 0.75, 1.0, 1.5, 2.0, 3.0, 4.0, 6.0, 8.0,
                                12.0, 16.0, 24.0, 32.0, 48.0, 64.0, 96.0, 128.0,
                                192.0, 256.0, 384.0, 512.0, 768.0, 1024.0, 1536.0,
                                2048.0, 3072.0, 4096.0, 6144.0, 8192.0, 12288.0};

static double
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

static PAL_Error
ConvertGPSFileRecordAbpalGpsLocation(GPSFileRecord* gpsFileRecord, ABPAL_GpsLocation* location)
{
    if (location == NULL ||
        gpsFileRecord == NULL)
    {
        return PAL_ErrBadParam;
    }

    location->status = gpsFileRecord->status;
    location->gpsTime = PAL_ClockGetGPSTime();
    location->valid = gpsFileRecord->fValid;
    location->latitude = gpsFileRecord->dwLat * Gps_Latitude_Constant;
    location->longitude = gpsFileRecord->dwLon * Gps_Longitude_Constant;
    location->heading = gpsFileRecord->wHeading * Gps_Heading_Constant;
    location->horizontalVelocity = gpsFileRecord->wVelocityHor * Gps_Horizontal_Vel_Constant;
    location->altitude = gpsFileRecord->wAltitude - Gps_Altitude_Constant;
    location->verticalVelocity = gpsFileRecord->bVelocityVer * Gps_Vertical_Vel_Constant;
    location->horizontalUncertaintyAngleOfAxis = gpsFileRecord->bHorUncAngle * Gps_Horizontal_Unc_Angle_Of_AxisC_onstant;
    location->horizontalUncertaintyAlongAxis = uncdecode(gpsFileRecord->bHorUnc);
    location->horizontalUncertaintyAlongPerpendicular = uncdecode(gpsFileRecord->bHorUncPerp);
    location->verticalUncertainty = uncdecode(gpsFileRecord->wVerUnc);
    location->utcOffset = 0;
    location->numberOfSatellites = Gps_Fake_Num_Of_Satellites;

    return PAL_Ok;
}

FileGpsProvider::FileGpsProvider(PAL_Instance* pal):
    GpsProvider(pal),
    m_pPal(pal),
    m_pFile(NULL),
    m_maxFixesCount(0),
    m_currentFix(0),
    m_bNextFixRequired(false)
{
	if(m_pPal){

	}
}

FileGpsProvider::~FileGpsProvider()
{
    stop();

    if (m_pFile != NULL)
    {
        PAL_FileClose(m_pFile);
        m_pFile = NULL;
    }
}

PAL_Error FileGpsProvider::Initialize(const ABPAL_GpsConfig* gpsConfig, uint32 number)
{
    PAL_Error error = PAL_Ok;
    uint32 filesize = 0;
    char* filename = NULL;

    filename = GetGpsConfigValue(gpsConfig, number, "gpsfilename");
    if (PAL_FileOpen(m_pPal, filename, PFM_Read, &m_pFile))
    {
        return PAL_ErrBadParam;
    }

    error = PAL_FileGetSize(m_pPal, filename, &filesize);
    if (error != PAL_Ok)
    {
        return error;
    }

    m_maxFixesCount = filesize/sizeof(GPSFileRecord);

    if (m_maxFixesCount < 1)
    {
        return PAL_ErrNoData;
    }

    return error;
}

void FileGpsProvider::TimerCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    FileGpsProvider* pThis = reinterpret_cast<FileGpsProvider*>(userData);

    if (reason == PTCBR_TimerFired)
    {
        ABPAL_GpsLocation location = {0};

        PAL_Error error = pThis->ReadNextPosition(&location);
        pThis->NotifyListeners(error, location);
        if (pThis->m_bNextFixRequired)
        {
            PAL_TimerSet(pThis->m_pPal,
                                 pThis->m_Period * 1000,
                                 TimerCallback,
                                 pThis);
        }
    }
}

PAL_Error FileGpsProvider::ReadNextPosition(ABPAL_GpsLocation* location)
{
    PAL_Error err = PAL_Ok;

    if (m_currentFix >= m_maxFixesCount)
    {
        m_currentFix = 0;
    }

    // Check whether pal file handler exists
    if (m_pFile)
    {
        int32 offset = m_currentFix * sizeof(GPSFileRecord);
        err = PAL_FileSetPosition(m_pFile, PFSO_Start, offset);
        if (err == PAL_Ok)
        {
            uint32 bytesReaded = 0;
            GPSFileRecord record = {0};

            err = PAL_FileRead(m_pFile, (uint8*)(&record), sizeof(GPSFileRecord), &bytesReaded);
            err = err ? err : ConvertGPSFileRecordAbpalGpsLocation(&record, location);
            if (err == PAL_Ok)
            {
                m_currentFix += 1;
            }
        }
    }
    else
    {
        err = PAL_Failed;
    }

    return err;
}

void FileGpsProvider::stop()
{
    PAL_TimerCancel(m_pPal, TimerCallback, this);
    m_bNextFixRequired = false;
}

void FileGpsProvider::start()
{
    // Set the time to zero when first get the GPS
    PAL_TimerSet(m_pPal, 200, TimerCallback, this);
    m_bNextFixRequired = true;
}

/*! @} */
