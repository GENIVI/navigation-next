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

 @file gpscontext.cpp
 @date 4/18/12

 GPS context

 */
/*
 (C) Copyright 2012 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret
 as defined in section 499C of the penal code of the State of
 as defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems is granted only
 under a written non-disclosure agreement, expressly prescribing
 the scope and manner of such use.

 ---------------------------------------------------------------------------*/

/*! @{ */

#include "gpscontext.h"
#include "filegpsmanager.h"
#include "gpsconfiguration.h"
#include <windows.h>
#include <new>


const double InitlizeTimeInterval = 0.25; // initlize time interval

static ABPAL_GpsLocation latestLocation;
static bool latestLocationValid = false;
static GPSContext* SELF = NULL;

void CALLBACK stateChanged(HWND hwnd, UINT message, UINT timerID, DWORD time) 
{
    switch (SELF->gpsState)
    {
        case PGS_Initialized:
        {
            if (SELF->timeout_timer)
            {
                KillTimer(NULL, SELF->timeout_timer);
                SELF->timeout_timer = 0;
            }
            SELF->initlizeCallBack(SELF->userData, PAL_Ok,NULL);
        }
            break;
        case PGS_Tracking:
        {
            if (SELF->timeout_timer)
            {
                KillTimer(NULL, SELF->timeout_timer);
                SELF->timeout_timer = 0;
            }

            if(SELF->firstTimeTracking && latestLocationValid)
            {
                SELF->firstTimeTracking = false;
                SELF->gpsCallBack(SELF->userData, PAL_Ok, &latestLocation, NULL);
            }
            else
            {
                SELF->gpsCallBack(SELF->userData, PAL_ErrGpsTimeout, NULL, NULL);
            }
        }
            break;
        case PGS_Oneshot:
        {
            if (SELF->timeout_timer)
            {
                KillTimer(NULL, SELF->timeout_timer);
                SELF->timeout_timer = 0;
            }
            
            SELF->gpsManager->cancelRequest((GpsContextCallBackProtocol*)SELF);

            if(latestLocationValid)
            {
                SELF->gpsCallBack(SELF->userData, PAL_Ok, &latestLocation, NULL);
            }
            else
            {
                SELF->gpsCallBack(SELF->userData, PAL_ErrGpsTimeout, NULL, NULL);
            }
        }
            break;
        default:
            break;
    }
}

void CALLBACK trackingTimerExpried(HWND hwnd, UINT message, UINT timerID, DWORD time) 
{
    if (SELF->hasFix && SELF->lastLocation)
    {
        SELF->gpsCallBack(SELF->userData, PAL_Ok, SELF->lastLocation, NULL);
        SELF->hasFix = FALSE;
    }
    else if (SELF->lastLocationError != PAL_Ok)
    {
        SELF->gpsCallBack(SELF->userData, SELF->lastLocationError, NULL, NULL);
        SELF->hasFix = FALSE;
    }
}

GPSContext::GPSContext(PAL_Instance* pal)
: GpsContextProtocol(), GpsContextCallBackProtocol()
{
    gpsState            = PGS_Undefined;
    hasFix              = FALSE;
    pal_Instance        = pal; // for simulation gps
    stateChangeCallBack = NULL;
    initlizeCallBack    = NULL;
    gpsCallBack         = NULL;
    userData            = NULL;
    gpsManager          = NULL;
    timer               = 0;
    timeout_timer       = 0;
    lastLocationInfo    = NULL;
    lastLocation        = NULL;
    trackingInterval    = 0;
	
    SELF                = this;
}

GPSContext::~GPSContext()
{
    if (gps_Type == Gps_Simulation)
    {
        if(gpsManager)
        {
            gpsManager->cancelGetLocation();
            gpsManager->endTracking();
            delete gpsManager;
            gpsManager = NULL;
        }
    }

    if(lastLocationInfo)
    {
        delete[] lastLocationInfo;
    }

    if (NULL != lastLocation) {
        free(lastLocation);
    }

    if (timer)
    {
        KillTimer(NULL, timer);
        timer = 0;
    }
    if (timeout_timer)
    {
        KillTimer(NULL, timeout_timer);
        timeout_timer = 0;
    }
}

//GpsContextCallBackProtocol
void GPSContext::locationCallBack(ABPAL_GpsLocation* location,
                                  PAL_Error error,
                                  const char* errorInfo
                                  )
{
    lastLocationError = error;

    if (NULL != lastLocationInfo)
    {
        delete []lastLocationInfo;
        lastLocationInfo = NULL;
    }

    if (NULL != errorInfo)
    {
        int length = strlen(errorInfo);
        lastLocationInfo = new(std::nothrow) char[length+1];
        if(lastLocationInfo)
        {
            memset(lastLocationInfo, 0, length+1);
            memcpy(lastLocationInfo, errorInfo, length);
         }
    }

    if (NULL != lastLocation)
    {
        free(lastLocation);
        lastLocation = NULL;
    }

    if(NULL != location)
    {
        lastLocation = (ABPAL_GpsLocation*)(malloc(sizeof(ABPAL_GpsLocation)));
        memset(lastLocation, 0, sizeof(ABPAL_GpsLocation));
        memcpy(lastLocation, location, sizeof(ABPAL_GpsLocation));
        hasFix = TRUE;
    }

    if(location)
    {
        latestLocation = *location;
        latestLocationValid = TRUE;
    }
    firstTimeTracking = FALSE;

    if (gpsState == PGS_Oneshot)
    {

        if (timeout_timer)
        {
            KillTimer(NULL, timeout_timer);
            timeout_timer = 0;
        }
        gpsCallBack(userData, error, location, errorInfo);

        if (gps_Type == Gps_Real)
        {
            gpsManager->cancelRequest(this);
        }
    }
    else // tracking gps
    {
        if (timeout_timer)
        {
            KillTimer(NULL, timeout_timer);
            timeout_timer = 0;
        }

        if (gpsManager->isServiceEnable())
        {
            timeout_timer = SetTimer(NULL, 0, trackingInterval*1000, stateChanged);
        }
    }
}

ABPAL_GpsCriteria GPSContext::palCriteria()
{
    return palcriteria;
}

ABPAL_GpsState GPSContext::requestState()
{
    return gpsState;
}

//GpsContextProtocol
PAL_Error GPSContext::initGpsConfig(const GpsConfiguration* config,
		                    ABPAL_GpsInitializeCallback* userCallback,
                                    ABPAL_GpsDeviceStateCallback* stateCallback,
                                    const void* data
                                    )
{
    PAL_Error result = PAL_Failed;

    if (gpsManager != NULL)
    {
        result = PAL_ErrGpsInvalidState;
    }
    else
    {
        initlizeCallBack    = userCallback;
        stateChangeCallBack = stateCallback;
        userData            = data;

        if (config->providerType() == GpsProviderTypeGpsId)
        {
            gps_Type = Gps_Real;
            gpsManager = NULL;//RealGpsFactory::getInstance(pal_Instance);
        }
        else
        {
            gps_Type = Gps_Simulation;
            gpsManager = new FileGpsManager(pal_Instance);

            if (gpsManager)
            {
                result = gpsManager->setSimulationConfig(config);
            }
        }

        if (NULL != gpsManager)
        {
            result = PAL_Ok;
        }

        gpsState = PGS_Initialized;

        timeout_timer = SetTimer(NULL, 0, (int)(InitlizeTimeInterval*1000), stateChanged);

    }
    return result;
}

ABPAL_GpsState GPSContext::handlerState()
{
    return gpsState;
}

PAL_Error GPSContext::getLocation(const ABPAL_GpsCriteria* criteria,
		                  ABPAL_GpsLocationCallback* callBack,
                                  const void* data
                                  )
{
    PAL_Error result = PAL_ErrBadParam;

    if (gpsState != PGS_Initialized)
    {
        return PAL_ErrNoInit;
    }

    palcriteria = *criteria;
    gpsCallBack = callBack;
    userData    = data;
    gpsState    = PGS_Oneshot;

    gpsManager->addObserver(this);
    result = gpsManager->start();

    // Only start a timeout timer when service enabled
    if (gpsManager->isServiceEnable())
    {
        timeout_timer = SetTimer(NULL, 0, palcriteria.desiredTimeout*100, stateChanged);
    }
    return result;
}

PAL_Error GPSContext::cancelGetLocation()
{
    PAL_Error result = PAL_Ok;

    if (timeout_timer)
    {
        KillTimer(NULL, timeout_timer);
        timeout_timer = 0;
    }

    if (timer)
    {
        KillTimer(NULL, timer);
        timer = 0;
    }

    if(gps_Type == Gps_Simulation)
    {
        result = gpsManager->cancelGetLocation();
        gpsManager->addObserver(NULL);
    }
    else
    {
        result= gpsManager->cancelRequest(this);
    }

    return result;
}

PAL_Error GPSContext::beginTracking(const ABPAL_GpsCriteria* criteria,
		                    ABPAL_GpsLocationCallback* callBack,
                                    const void* data,
                                    ABPAL_GpsTrackingInfo* trackInfo
                                    )
{
    PAL_Error result = PAL_ErrBadParam;

    if (gpsState != PGS_Initialized)
    {
        return PAL_ErrNoInit;
    }

    palcriteria = *criteria;
    gpsCallBack = callBack;
    userData    = data;
    trackInfo->Id  = 1; //todo

    gpsState = PGS_Tracking;

    gpsManager->addObserver(this);
    result = gpsManager->start();

    trackingInterval = palcriteria.desiredTimeout;
    if (gpsManager->isServiceEnable())
    {
        timeout_timer = SetTimer(NULL, 0, trackingInterval*1000, stateChanged);
    }

    timer = SetTimer(NULL, 0, palcriteria.desiredInterval*1000, trackingTimerExpried);

    firstTimeTracking = TRUE;

    return result;
}

PAL_Error GPSContext::endTracking(const ABPAL_GpsTrackingInfo* trackintInfo)
{
    PAL_Error result = PAL_Ok;

    if (timer)
    {
        KillTimer(NULL, timer);
        timer = 0;
    }

    if (timeout_timer)
    {
        KillTimer(NULL, timeout_timer);
        timeout_timer = 0;
    }
    if(gps_Type == Gps_Simulation)
    {
        result = gpsManager->endTracking();
        gpsManager->addObserver(NULL);
    }
    else
    {
        result= gpsManager->cancelRequest(this);
    }

    return result;
}

PAL_Error GPSContext::gpsSusPend(const ABPAL_GpsTrackingInfo* trackintInfo)
{
    PAL_Error result = PAL_Ok;

    //todo

    return result;
}

PAL_Error GPSContext::gpsResume(const ABPAL_GpsTrackingInfo* trackintInfo)
{
    PAL_Error result = PAL_Ok;

    // todo
    return result;
}

PAL_Error GPSContext::updateCriteria(const ABPAL_GpsCriteria* criteria,
		                     const ABPAL_GpsTrackingInfo* trackInfo
                                     )
{
    PAL_Error result = PAL_Ok;

    palcriteria = *criteria;

    if (gpsState == PGS_Tracking)
    {
        if(trackingInterval != palcriteria.desiredTimeout)
        {
            trackingInterval = palcriteria.desiredTimeout;
            if (timeout_timer)
            {
                KillTimer(NULL, timeout_timer);
                timeout_timer = 0;
            }

            if (gpsManager->isServiceEnable())
            {
                timeout_timer = SetTimer(NULL, 0, palcriteria.desiredTimeout*1000, stateChanged);
            }
        }

        if (timer)
        {
            KillTimer(NULL, timer);
            timer = 0;
        }
        timer = SetTimer(NULL, 0, palcriteria.desiredInterval*1000, trackingTimerExpried);
    }

    return result;
}
