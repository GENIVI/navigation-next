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

    @file     hsgpsprovider.cpp
    @date     02/25/2014
    @defgroup HSGPSPROVIDER_H HS Gps

    @brief    HS Gps

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
#include "hsgpsprovider.h"
#include "palclock.h"
#include "palstdlib.h"
#include "paltestlog.h"
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include "pal.h"
#include "abpalgpsutils.h"

enum
{
    DESTROY_REQUEST = 0,
    START_REQUEST,
    CANCEL_REQUEST,
    UPDATE_REQUEST,
    BACKGROUND_GEOLOCATION_ON,
    BACKGROUND_GEOLOCATION_OFF
};

#define INVALID_LATLON          (-999.0)
#define GPS_TIMEOUT        5000 // ms
#define ALL_VALID_VALUE (PGV_Latitude | PGV_Longitude | PGV_Altitude | PGV_Heading \
    | PGV_HorizontalVelocity | PGV_HorizontalUncertainty | PGV_AxisUncertainty \
    | PGV_PerpendicularUncertainty | PGV_VerticalUncertainty | PGV_UTCOffset | PGV_SatelliteCount)

#define GPS_VALID_BITS_LOG_INTERVAL (30 * 1000)

HsGpsProvider* HsGpsProvider::m_pHsGpsProvider = NULL;
uint32 HsGpsProvider::m_Ref = 0;
pthread_mutex_t HsGpsProvider::m_lock = PTHREAD_MUTEX_INITIALIZER;

HsGpsProvider::HsGpsProvider(PAL_Instance* pal):
    GpsProvider(pal),
    m_pPal(pal),
    m_backgroundEventsEnabled(false),
    m_logLocationAllowed(false),
    m_bThreadWorking(false),
    m_threadId(0),
    m_serialHandle(-1)
{	
    m_gpsupdated = false;
    memset(&m_gpsLocation, 0, sizeof(m_gpsLocation));
}

HsGpsProvider* HsGpsProvider::GetInstantce(PAL_Instance* pal)
{
    if (m_pHsGpsProvider == NULL)
    {
        pthread_mutex_lock(&m_lock);
        if (m_pHsGpsProvider == NULL)
        {
            m_pHsGpsProvider = new HsGpsProvider(pal);
            if (m_pHsGpsProvider)
            {
                m_Ref = 1;
            }
        }
        pthread_mutex_unlock(&m_lock);
    }
    else
    {
        m_Ref += 1;
    }

    return m_pHsGpsProvider;
}

HsGpsProvider* HsGpsProvider::GetAvailableInstantce(){
    if (m_Ref>0){
        return m_pHsGpsProvider;
    }

    return NULL;
}

void HsGpsProvider::RemoveInstance()
{
    if (--m_Ref == 0)
    {
        delete this;
        m_pHsGpsProvider = NULL;
    }
}

HsGpsProvider::~HsGpsProvider()
{

}

void HsGpsProvider::LocationInfoProcess(ABPAL_GpsLocation &LocationInfo, bool isGpsFix){
    pthread_mutex_lock(&m_lock);
    m_gpsupdated = IsLocationUpdated(m_gpsLocation, LocationInfo);
    if (m_gpsupdated == true){
        memset(&m_gpsLocation, 0, sizeof(ABPAL_GpsLocation));
        memcpy(&m_gpsLocation, &LocationInfo, sizeof(ABPAL_GpsLocation));
    }
    pthread_mutex_unlock(&m_lock);
}

void HsGpsProvider::NotifyLocationInfomation()
{
    bool gpsupdated = false;

    pthread_mutex_lock(&m_lock);
    gpsupdated = m_gpsupdated;
    pthread_mutex_unlock(&m_lock);

    if (gpsupdated == true){
        NotifyListeners(PAL_Ok,m_gpsLocation,true);

        pthread_mutex_lock(&m_lock);
        m_gpsupdated = false;
        pthread_mutex_unlock(&m_lock);
    }
}

PAL_Error HsGpsProvider::Initialize(const ABPAL_GpsConfig* gpsConfig, uint32 configCount)
{	
    char *cssocketport = NULL;
    //open serial port and set up the parameters for the serial port.
    //assert(gpsConfig);

    if (gpsConfig == NULL){
        return PAL_ErrBadParam;
    }

    return PAL_Ok;
}

bool HsGpsProvider::logLocationAllowed(const ABPAL_GpsConfig* config, uint32 configCount)
{
    for (uint32 i = 0; i < configCount; i++)
    {
        if (nsl_strcmp(config[i].name, "logLocationAllowed") != 0)
        {
            continue;
        }

        if ( nsl_strcmp(config[i].value, "true") == 0 )
            return true;
        else
            return false;
    }

    return false;
}

void HsGpsProvider::RecordLocation(ABPAL_GpsLocation &location, ABPAL_GpsLocation &lastLocation, bool coarse)
{
    static bool lastCoarse = true;
    char buffer[1024] = "GPS:";
    int stringLength = nsl_strlen("GPS:");

    if (m_logLocationAllowed && ((location.latitude != lastLocation.latitude) || (location.longitude != lastLocation.longitude)))
    {
        stringLength += sprintf(buffer + stringLength, "(lat,lon)=%lf,%lf ", location.latitude, location.longitude);
    }

    if (location.heading != lastLocation.heading)
    {
        stringLength += sprintf(buffer + stringLength, "hd=%.0f ", location.heading);
    }

    if (location.horizontalVelocity != lastLocation.horizontalVelocity)
    {
        stringLength += sprintf(buffer + stringLength, "spd=%.0f ", location.horizontalVelocity);
    }

    if (location.altitude != lastLocation.altitude)
    {
        stringLength += sprintf(buffer + stringLength, "alt=%.1f ", location.altitude);
    }

    if (location.horizontalUncertaintyAlongAxis != lastLocation.horizontalUncertaintyAlongAxis)
    {
        stringLength += sprintf(buffer + stringLength, "accuracy=%.0f ", location.horizontalUncertaintyAlongAxis);
    }

    if (location.numberOfSatellites != lastLocation.numberOfSatellites)
    {
        stringLength += sprintf(buffer + stringLength, "sat=%d ", location.numberOfSatellites);
    }

    if (lastCoarse != coarse)
    {
        stringLength += sprintf(buffer + stringLength, "coarse=%d", coarse);
        lastCoarse = coarse;
    }

    if (stringLength > nsl_strlen("GPS:"))
    {
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, buffer);
    }
}

void HsGpsProvider::start()
{
    if(0 == pthread_create(&m_threadId, NULL, HsGpsProvider::GpsWorkerThreadFunc, this))
    {
        m_bEnabled = true;
    }
}

void HsGpsProvider::updateCriteria(const ABPAL_GpsCriteria* criteria)
{

}

void HsGpsProvider::stop()
{
    m_bThreadWorking = false;
    pthread_join(m_threadId, 0);
}

PAL_Error HsGpsProvider::enableBackgroundGeolocation(nb_boolean isEnabled)
{
    return PAL_Ok;
}

void* HsGpsProvider::GpsWorkerThreadFunc(void* user_data)
{
    HsGpsProvider* gpsProvider = reinterpret_cast<HsGpsProvider*>(user_data);
    assert(gpsProvider);
    gpsProvider->m_bThreadWorking = true;

    while(gpsProvider->m_bThreadWorking)
    {
        //read data from serial port
        //parse data and fill the ABPAL_GpsLocation structure.
        usleep(200*1000);
        gpsProvider->NotifyLocationInfomation();
    }
}

bool HsGpsProvider::IsLocationUpdated(ABPAL_GpsLocation &oldLocation, ABPAL_GpsLocation &newlastLocation){
    bool bUpdated = true;
    if ((oldLocation.status == newlastLocation.status) && (oldLocation.gpsTime == newlastLocation.gpsTime)
            && (oldLocation.valid == newlastLocation.valid) && (oldLocation.latitude == newlastLocation.latitude)
            && (oldLocation.longitude == newlastLocation.longitude) && (oldLocation.heading == newlastLocation.heading)
            && (oldLocation.horizontalVelocity == newlastLocation.horizontalVelocity) && (oldLocation.altitude == newlastLocation.altitude)
            && (oldLocation.verticalVelocity == newlastLocation.verticalVelocity) && (oldLocation.horizontalUncertaintyAngleOfAxis == newlastLocation.horizontalUncertaintyAngleOfAxis)
            && (oldLocation.horizontalUncertaintyAlongAxis == newlastLocation.horizontalUncertaintyAlongAxis) && (oldLocation.horizontalUncertaintyAlongPerpendicular == newlastLocation.horizontalUncertaintyAlongPerpendicular)
            && (oldLocation.verticalUncertainty == newlastLocation.verticalUncertainty) && (oldLocation.utcOffset == newlastLocation.utcOffset)
            && (oldLocation.numberOfSatellites == newlastLocation.numberOfSatellites) && (oldLocation.gpsHeading == newlastLocation.gpsHeading)
            && (oldLocation.compassHeading == newlastLocation.compassHeading) && (oldLocation.compassAccuracy == newlastLocation.compassAccuracy)
            ){
        bUpdated = false;
    }

    return bUpdated;
}

/*! @} */

