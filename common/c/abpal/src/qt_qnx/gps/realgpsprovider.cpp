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

    @file     realgpsprovider.cpp
    @date     03/27/2012
    @defgroup REALGPSPROVIDER_H Real Gps

    @brief    Real Gps

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

#include <bps/geolocation.h>
#include "realgpsprovider.h"
#include "palclock.h"
#include "palstdlib.h"
#include <unistd.h>
#include <sys/time.h>
#include "paltestlog.h"
#include <errno.h>
#include "pal.h"

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

RealGpsProvider* RealGpsProvider::m_pRealGpsProvider = NULL;
uint32 RealGpsProvider::m_Ref = 0;
pthread_mutex_t RealGpsProvider::m_lock = PTHREAD_MUTEX_INITIALIZER;

static PAL_Error
ConvertBpsGeolocationAbpalGpsLocation(bps_event_t *event, ABPAL_GpsLocation* location)
{
    static uint32 lastErrorTime = 0;
    static uint32 lastValid = 0;
    static uint32 sameValidCount = 0;

    if (location == NULL ||
        event == NULL ||
        bps_event_get_code(event) != GEOLOCATION_INFO)
    {
        return PAL_ErrBadParam;
    }

    location->status = PAL_Ok;
    location->utcOffset = 0;
    location->horizontalUncertaintyAngleOfAxis = 0;
    location->valid = PGV_UTCOffset;

    if (geolocation_event_is_latitude_valid(event))
    {
        location->latitude = geolocation_event_get_latitude(event);
        location->valid |= PGV_Latitude;
    }
    else
    {
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelError, "invalid lat %lf", geolocation_event_get_latitude(event));
    }

    if (geolocation_event_is_longitude_valid(event))
    {
        location->longitude = geolocation_event_get_longitude(event);
        location->valid |= PGV_Longitude;
    }
    else
    {
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelError, "invalid lon %lf", geolocation_event_get_longitude(event));
    }

    if (geolocation_event_is_accuracy_valid(event))
    {
        location->horizontalUncertaintyAlongAxis = geolocation_event_get_accuracy(event);
        location->horizontalUncertaintyAlongPerpendicular = location->horizontalUncertaintyAlongAxis;
        location->valid |= PGV_HorizontalUncertainty | PGV_AxisUncertainty | PGV_PerpendicularUncertainty;
    }

    if (geolocation_event_is_heading_valid(event))
    {
        location->heading = geolocation_event_get_heading(event);
        location->valid |= PGV_Heading;
    }

    if (geolocation_event_is_speed_valid(event))
    {
        location->horizontalVelocity = geolocation_event_get_speed(event);
        location->valid |= PGV_HorizontalVelocity;
    }

    if (geolocation_event_is_num_satellites_valid(event))
    {
        location->numberOfSatellites = geolocation_event_get_num_satellites_used(event);
        location->valid |= PGV_SatelliteCount;
    }

    if (geolocation_event_is_altitude_valid(event))
    {
        location->altitude = geolocation_event_get_altitude(event);
        location->valid |= PGV_Altitude;
    }

    if (geolocation_event_is_altitude_accuracy_valid(event))
    {
        location->verticalUncertainty = geolocation_event_get_altitude_accuracy(event);
        location->valid |= PGV_VerticalUncertainty;
    }

    // we do not need to have accurate time from the event
    location->gpsTime = PAL_ClockGetGPSTime();

    if (location->valid != ALL_VALID_VALUE)
    {
        uint32 now = PAL_ClockGetTimeMs();
        if (((now - lastErrorTime) > GPS_VALID_BITS_LOG_INTERVAL) || (lastValid != location->valid))
        {
            NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelError, "GPS: valid bits 0x%x, %d same before", location->valid, sameValidCount);

            sameValidCount = 0;
            lastErrorTime = now;
            lastValid = location->valid;
        }
        else
        {
            sameValidCount++;
        }
    }

    return PAL_Ok;
}

RealGpsProvider::RealGpsProvider(PAL_Instance* pal):
    GpsProvider(pal),
    m_pPal(pal),
    m_workerThread(0),
    m_idChannel(-1),
    m_eventDomain(0),
    m_backgroundEventsEnabled(false),
    m_logLocationAllowed(false)
{
    pthread_mutex_init(&m_channelMutex, NULL);
    pthread_cond_init(&m_channelCond, NULL);
}

RealGpsProvider* RealGpsProvider::GetInstantce(PAL_Instance* pal)
{
    if (m_pRealGpsProvider == NULL)
    {
        pthread_mutex_lock(&m_lock);
        if (m_pRealGpsProvider == NULL)
        {
            m_pRealGpsProvider = new RealGpsProvider(pal);
            if (m_pRealGpsProvider)
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

    return m_pRealGpsProvider;
}

void RealGpsProvider::RemoveInstance()
{
    if (--m_Ref == 0)
    {
        delete this;
        m_pRealGpsProvider = NULL;
    }
}

RealGpsProvider::~RealGpsProvider()
{
    DestroyThread();
    pthread_mutex_destroy(&m_channelMutex);
    pthread_cond_destroy(&m_channelCond);
}

PAL_Error RealGpsProvider::Initialize(const ABPAL_GpsConfig* gpsConfig, uint32 configCount)
{
    bool allowed = true;
    if (m_workerThread == 0)
    {
        m_eventDomain = bps_register_domain();
        if (m_eventDomain == -1)
        {
            return PAL_ErrNoInit;
        }

        int created_status = pthread_create(&m_workerThread, NULL, GpsWorkerThreadFunc, this);
        if (created_status)
        {
            return PAL_ErrNetGeneralFailure;
        }
    }

    allowed = logLocationAllowed(gpsConfig, configCount);
    if (m_logLocationAllowed != allowed)
    {
        m_logLocationAllowed = allowed;
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelError, "GPS: get log location allowed %s", m_logLocationAllowed ? "true" : "false");
    }

    return PAL_Ok;
}

bool RealGpsProvider::logLocationAllowed(const ABPAL_GpsConfig* config, uint32 configCount)
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

void* RealGpsProvider::GpsWorkerThreadFunc(void* user_data)
{
    RealGpsProvider* gpsProvider = reinterpret_cast<RealGpsProvider*>(user_data);
    nb_boolean threadIsRunning = TRUE;
    int requestEvent = -1;

    bps_initialize();
    // Set meaningful name
    pthread_setname_np((pthread_t)pthread_self(), "TCS GPS thread");

    pthread_mutex_lock(&gpsProvider->m_channelMutex);
    gpsProvider->m_idChannel = bps_channel_get_active();
    pthread_cond_signal(&gpsProvider->m_channelCond);
    pthread_mutex_unlock(&gpsProvider->m_channelMutex);

    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "RGPS: thread enter");

    geolocation_set_period(1);
    requestEvent = geolocation_request_events(0);

    if ( requestEvent != BPS_SUCCESS )
    {
        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelError, "RGPS:geolocation_request_events(0) return %d errono %d", requestEvent, errno);
    }

    bool gotFirstFix = false;
    uint32 gpsFixCount = 0;
    uint32 gpsFixCountBeginTime = 0;

    while (threadIsRunning)
    {
        int code = BPS_SUCCESS;
        bps_event_t *event = NULL;

        code = bps_get_event(&event, gpsProvider->getTimeOutValue(gotFirstFix));
        if (code == BPS_FAILURE)
        {
            NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelError, "RGPS: bps_get_event() failed errno = %d", errno);
            continue;
        }

        if (event == NULL)
        {
            // timed out
            requestEvent = geolocation_request_events(0);
            if ( requestEvent != BPS_SUCCESS )
            {
                NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelError, "GPS:geolocation_request_events(0) with %s after timeout", strerror(errno));
            }
            else
            {
                NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelError, "GPS: get location timeout");
            }
            continue;
        }

        if (bps_event_get_domain(event) == gpsProvider->m_eventDomain)
        {
            if (bps_event_get_code(event) == DESTROY_REQUEST)
            {
                threadIsRunning = FALSE;
            }
            else if (bps_event_get_code(event) == START_REQUEST)
            {
                gotFirstFix = false;
                gpsFixCountBeginTime = PAL_ClockGetTimeMs();
                gpsFixCount = 0;
                geolocation_set_period(1);
                NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "RGPS:request events");
                requestEvent = geolocation_request_events(0);
                if ( requestEvent != BPS_SUCCESS )
                {
                    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelError, "RGPS:geolocation_request_events(0) return %d, errno %d", requestEvent, errno);
                }

                if (requestEvent == BPS_FAILURE && errno == EACCES)
                {
                    ABPAL_GpsLocation location = {0};
                    location.latitude = INVALID_LATLON;
                    location.longitude = INVALID_LATLON;
                    PAL_Error error = PAL_ErrGpsLocationDisabled;
                    gpsProvider->NotifyListeners(error, location, false);
                }
            }
            else if (bps_event_get_code(event) == CANCEL_REQUEST)
            {
                if (gpsProvider->needCancel())
                {
                    gotFirstFix = false;
                    if (gpsFixCountBeginTime > 0)
                    {
                        NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "GPS:stop events, %d fix in %dms", gpsFixCount, PAL_ClockGetTimeMs() - gpsFixCountBeginTime);
                        gpsFixCountBeginTime = 0;
                    }
                    geolocation_stop_events(0);
                }
            }
            else if (bps_event_get_code(event) == BACKGROUND_GEOLOCATION_ON)
            {
                NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "RGPS:set background true");
                geolocation_set_background(true);
                gpsProvider->m_backgroundEventsEnabled = true;
            }
            else if (bps_event_get_code(event) == BACKGROUND_GEOLOCATION_OFF)
            {
                NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "RGPS:set background false");
                geolocation_set_background(false);
                gpsProvider->m_backgroundEventsEnabled = false;
            }
        }
        else if (bps_event_get_domain(event) == geolocation_get_domain())
        {
            if (bps_event_get_code(event) == GEOLOCATION_INFO)
            {
                ABPAL_GpsLocation location = {0};
                static ABPAL_GpsLocation lastLocation = {0};

                PAL_Error error = ConvertBpsGeolocationAbpalGpsLocation(event, &location);
                gotFirstFix = true;
                gpsFixCount++;

                gpsProvider->RecordLocation(location, lastLocation, geolocation_event_is_coarse(event));
                lastLocation = location;

                gpsProvider->NotifyListeners(error, location, !geolocation_event_is_coarse(event));
            }
            //Handle ERROR AND CANCEL
            else if (bps_event_get_code(event) == GEOLOCATION_ERROR)
            {
                int error = geolocation_event_get_error_code(event);
                NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelError, "GPS:Location:error code=%#x", error);
                ABPAL_GpsLocation location = {0};
                location.latitude = INVALID_LATLON;
                location.longitude = INVALID_LATLON;
                if (error == GEOLOCATION_ERROR_FATAL_DISABLED)
                {
                    gpsProvider->NotifyListeners(PAL_ErrGpsLocationDisabled, location, false);
                }
                else
                {
                    gpsProvider->NotifyListeners(PAL_ErrGpsGeneralFailure, location, false);
                }
            }
        }
    }

    NB_TEST_LOG(PAL_LogBuffer_1, PAL_LogLevelInfo, "RGPS:stop_events(0) thread exit");
    geolocation_stop_events(0);
    bps_shutdown();

    return NULL;
}

void RealGpsProvider::RecordLocation(ABPAL_GpsLocation &location, ABPAL_GpsLocation &lastLocation, bool coarse)
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

void RealGpsProvider::DestroyThread()
{
    if (m_workerThread)
    {
        SendEvent(DESTROY_REQUEST);
        pthread_join(m_workerThread, NULL);

        m_workerThread = 0;
    }
}

PAL_Error RealGpsProvider::SendEvent(int event)
{
    pthread_mutex_lock(&m_channelMutex);
    while (m_idChannel == -1)
    {
        pthread_cond_wait(&m_channelCond, &m_channelMutex);
    }
    pthread_mutex_unlock(&m_channelMutex);

    bps_event_t* stop_request_event = NULL;
    int bpsError = bps_event_create(&stop_request_event, m_eventDomain, event, NULL, NULL);
    bpsError = bpsError ? bpsError : bps_channel_push_event(m_idChannel, stop_request_event);

    if (BPS_SUCCESS != bpsError)
    {
        return PAL_ErrNoInit;
    }

    return PAL_Ok;
}

int RealGpsProvider::getTimeOutValue(bool gotFirstFix)
{
    // when device at "lock screen" the GPS signal from device will not be received.
    // so time out value should be -1 when app at background.
    if (gotFirstFix && !PAL_IsBackground(m_pPal) && !m_backgroundEventsEnabled)
    {
        return GPS_TIMEOUT;
    }
    else
    {
        return -1;
    }
}

bool RealGpsProvider::needCancel()
{
    // if m_backgroundEventsEnabled is true, GPS will not cancel.
    return (m_Ref == 1 || PAL_IsBackground(m_pPal)) && !m_backgroundEventsEnabled;
}

void RealGpsProvider::start()
{
    // TODO: Implement start as a setting desiredInterval as a period
    SendEvent(START_REQUEST);
}

void RealGpsProvider::updateCriteria(const ABPAL_GpsCriteria* criteria)
{
    // There is an issue with GPS fixes not delivered, due to interval set to 0 by someone else.
    // Hardcoding the fix interval to 1 for now, needs to be investigated further.
	nsl_memcpy(&m_criteria, criteria, sizeof(ABPAL_GpsCriteria));
    SendEvent(UPDATE_REQUEST);
}

void RealGpsProvider::stop()
{
    // TODO: Implement stop as setting more longer period for getting a fix
    SendEvent(CANCEL_REQUEST);
}

PAL_Error RealGpsProvider::enableBackgroundGeolocation(nb_boolean isEnabled)
{
    return SendEvent(isEnabled ? BACKGROUND_GEOLOCATION_ON : BACKGROUND_GEOLOCATION_OFF);
}

/*! @} */
