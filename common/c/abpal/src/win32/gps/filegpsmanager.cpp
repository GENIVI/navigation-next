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

 @file filegpsmanager.cpp
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

#include "filegpsmanager.h"
#include "filegpshandlersingleton.h"
#include "palclock.h"
#include <string>
#include <algorithm>

using namespace std;
FileGpsManager* SELF = NULL;

FileGpsHandler* FileGpsHandlerFactor::filegps_Instance = NULL;

void CALLBACK TimerProc(HWND hwnd, UINT message, UINT timerID, DWORD time)
{
    ABPAL_GpsLocation *location = (ABPAL_GpsLocation*)malloc(sizeof(ABPAL_GpsLocation));
    PAL_Error err = SELF->readNextPosition(*location);
    SELF->gpsContext_Instance->locationCallBack(location, err, NULL);
    free(location);

    if (PGS_Oneshot == SELF->gpsContext_Instance->requestState())
    {
		KillTimer(NULL, SELF->_timer);
        SELF->_timer = 0;
    }
}


FileGpsManager::FileGpsManager(PAL_Instance* pal)
{
    _timer         = 0;
    gpsFileHandler = FileGpsHandlerFactor::getInstance();
    pal_Instance   = pal;
    palfile        = NULL;

    SELF           = this;
}

FileGpsManager::~FileGpsManager()
{
    if (_timer)
    {
        KillTimer(NULL, _timer);
        _timer = 0;
    }

    if (palfile)
    {
        PAL_FileClose(palfile);
        palfile = NULL;
    }
}

PAL_Error FileGpsManager::readNextPosition(ABPAL_GpsLocation& location)
{
    PAL_Error err = PAL_Ok;

    int current = gpsFileHandler->gpsFileOffset();

    if (current >= maxCount)
    {
        current = 0;
    }

    // Check whether pal file handler exists
    if (palfile)
    {
        int32 offset = current * sizeof(GPSFileRecord);
        err = PAL_FileSetPosition(palfile, PFSO_Start, offset);
        if (err == PAL_Ok)
        {
            uint32 bytesReaded = 0;
            GPSFileRecord record;
            memset(&record,0,sizeof(record));
            err = PAL_FileRead(palfile, (uint8*)(&record), sizeof(GPSFileRecord), &bytesReaded);

            if (err == PAL_Ok)
            {
                convert(record, location);
                ++current;
                upDateGpsOffset(current);
            }
        }

    }
    else
    {
        err = PAL_Failed;
    }


    return err;
}

void FileGpsManager::upDateGpsOffset(int offset)
{
    if(gpsFileHandler != NULL)
    {
        gpsFileHandler->setGpsFileOffset(offset);
    }
}

void FileGpsManager::convert(const GPSFileRecord& record, ABPAL_GpsLocation& location)
{
    const double KWGSCoefficient    = 0.00000536441803;
    const double KHeadingScale      = 0.3515625;
    const double KSpeedScale        = 0.25;

    const int KFakeNumOfSatellites  = 4;

    location.status             = PAL_Ok;
    location.gpsTime            = PAL_ClockGetGPSTime();
    location.valid              = record.fValid;
    location.latitude           = record.dwLat * KWGSCoefficient;
    location.longitude          = record.dwLon * KWGSCoefficient;
    location.heading            = record.wHeading * KHeadingScale;
    location.horizontalVelocity = record.wVelocityHor * KSpeedScale;
    location.altitude           = record.wAltitude * KWGSCoefficient;

    location.verticalVelocity                        = record.bVelocityVer / 2;
    location.horizontalUncertaintyAngleOfAxis        = record.bHorUncAngle;
    location.horizontalUncertaintyAlongAxis          = record.bHorUnc;
    location.horizontalUncertaintyAlongPerpendicular = record.bHorUncPerp;
    location.verticalUncertainty                     = record.wVerUnc;
    location.utcOffset                               = 0;
    location.numberOfSatellites                      = KFakeNumOfSatellites;
}

//GpsManagerProtocol interfaces
void FileGpsManager::addObserver(GpsContextCallBackProtocol* observer)
{
    gpsContext_Instance = observer;
    if(observer != NULL)
    {
    criteria = observer->palCriteria();
}
}

PAL_Error FileGpsManager::start()
{
    ABPAL_GpsState state = gpsContext_Instance->requestState();

    if (_timer)
    {
        KillTimer(NULL, _timer);
        _timer = 0;
    }

    if (PGS_Oneshot == state)
    {
        _timer = SetTimer(NULL, 0, 1000, TimerProc);
    }
    else if (PGS_Tracking == state)
    {
        _timer = SetTimer(NULL, 0, criteria.desiredInterval*1000, TimerProc);
    }
    return PAL_Ok;
}

PAL_Error FileGpsManager::cancelRequest(GpsContextCallBackProtocol* observer)
{
    return PAL_Ok;
}

PAL_Error FileGpsManager::setSimulationConfig(const GpsConfiguration* config)
{
    const char* filename = config->fileName();

    PAL_Error err = PAL_Ok;

    // Open file and keep file handler
    err = PAL_FileOpen(pal_Instance, filename, PFM_Read, &palfile);

    if (err != PAL_Ok)
    {
        return err;
    }

    upDateGpsFileName(filename);

    uint32 filesize = 0;
    err = PAL_FileGetSize(pal_Instance, filename, &filesize);
    if (err != PAL_Ok)
    {
        return err;
    }

    maxCount = filesize/sizeof(GPSFileRecord);

    if (maxCount < 1)
    {
        return PAL_ErrNoData;
    }

    return PAL_Ok;
}

void FileGpsManager::updateGpsCriteria()
{
    if (PGS_Tracking == gpsContext_Instance->requestState())
    {
        if (_timer)
        {
            KillTimer(NULL, _timer);
            _timer = 0;
        }
        criteria = gpsContext_Instance->palCriteria();
        _timer = SetTimer(NULL, 0, criteria.desiredInterval*1000, TimerProc);
    }
}

PAL_Error FileGpsManager::cancelGetLocation()
{
    if (_timer)
    {
        KillTimer(NULL, _timer);
        _timer = 0;
    }
    return PAL_Ok;
}

PAL_Error FileGpsManager::endTracking()
{
    if (_timer)
    {
        KillTimer(NULL, _timer);
        _timer = 0;
    }
    return PAL_Ok;
}

BOOL FileGpsManager::isServiceEnable()
{
    return TRUE;
}

void FileGpsManager::upDateGpsFileName(const char* name)
{
    const char* fileName = gpsFileHandler->fileName();

    if (fileName && name)
    {
        string nameLast(fileName);
        string nameNew(name);

        std::transform(nameLast.begin(), nameLast.end(), nameLast.begin(), ::tolower);
        std::transform(nameNew.begin(), nameNew.end(), nameNew.begin(), ::tolower);

        if (nameLast.compare(nameNew) != 0)
        {
            gpsFileHandler->setFileName(name);
            upDateGpsOffset(0);
        }
    }
    else
    {
        gpsFileHandler->setFileName(name);
        upDateGpsOffset(0);
    }
}
