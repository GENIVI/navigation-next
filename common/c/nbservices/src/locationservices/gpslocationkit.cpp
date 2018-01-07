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
 
 @file     gpslocationkit.cpp
 @date     03/15/2012
 @defgroup LOCATIONSERVICES_GROUP
 
 */
/*
 GpsLocationKit class implementation
 
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


#include "gpslocationkit.h"
#include "gpslocationkitworker.h"
#include "lsabpalgps.h"

extern "C"
{
#include "cslerrorutil.h"
#include "nbgpstypes.h"
}

const int NARROW_TO_ACCURATE_MIN_ACCURACY = 65000;
const int NARROW_TO_ACCURATE_TIMEOUT = 60000;
static char g_keyGpsMinAccuracy[] = "gpsminaccuracy";
static char g_keyGpsTimeout[] = "gpstimeout";


GpsLocationKit::GpsLocationKit():m_pal(NULL)
{
    m_lastLocation.valid = 0;
}

GpsLocationKit::~GpsLocationKit()
{
    for(std::vector<GpsLocationKitWorker*>::iterator it = m_listeners.begin(); it != m_listeners.end(); it++)
    {
        GpsLocationKitWorker* worker = *it;
        worker->m_trackingType = NB_TT_Cancel;
    }
    RemoveClosedRequest();
}

NB_Error
GpsLocationKit::Initialize(PAL_Instance* pal)
{
    m_pal = pal;
    return NE_OK;
}

NB_Error
GpsLocationKit::StartNavigationTracking(ABPAL_GpsConfig* config, uint32 configSize, NB_GpsCriteria nbcriteria, LocationServiceListenerInterface* listener)
{
    NB_Error error = NE_OK;
    ABPAL_GpsCriteria criteria = {0};
    InitGpsCriteria(criteria, nbcriteria);

    GpsLocationKitWorker* worker = FindWorker(listener);
    if(worker)
    {
        if (worker->m_trackingType == NB_TT_NarrowToAccurate)
        {
            StopTracking(listener);
            delete worker;
        }
        else
        {
            return error;
        }
    }

    worker = new GpsLocationKitWorker();
    if (worker == NULL)
    {
        error = NE_BADDATA;
    }
    else
    {
        worker->m_locationKit = this;
        worker->m_trackingType = NB_TT_Tracking;
        
        error = InitRequest(criteria, config, configSize, listener, worker);
    }

    if (error != NE_OK)
    {
        listener->GpsErrorReceived(error);
    }

    return error;
}

NB_Error
GpsLocationKit::StartNonNavigationTracking(ABPAL_GpsConfig* config, uint32 configSize, NB_GpsCriteria nbcriteria, LocationServiceListenerInterface* listener)
{
    return StartNavigationTracking(config, configSize, nbcriteria, listener);
}

NB_Error
GpsLocationKit::StopTracking(LocationServiceListenerInterface* listener)
{
    NB_Error error = NE_OK;    
    GpsLocationKitWorker* worker = FindWorker(listener);
    if (worker)
    {
        PAL_TimerCancel(worker->m_locationKit->m_pal, StopNarrowToAccurate, worker);
        PAL_Error err =  ABPAL_GpsEndTracking(worker->m_context, &worker->m_trackingInfo);
        error = CSL_TranslatePalError(err);
        worker->m_trackingType = NB_TT_Cancel;
    }

    RemoveClosedRequest();

    return error;
}

NB_Error
GpsLocationKit::NarrowToAccurate(ABPAL_GpsConfig *config, uint32 configSize, NB_GpsCriteria nbcriteria, LocationServiceListenerInterface *listener)
{
    NB_Error error = NE_OK;
    ABPAL_GpsCriteria criteria = {0};
    InitGpsCriteria(criteria, nbcriteria);

    GpsLocationKitWorker* worker = FindWorker(listener);
    if(worker)
    {
        if (worker->m_trackingType == NB_TT_Tracking)
        {
            StopTracking(listener);
            delete worker;
        }
        else
        {
            return error;
        }
    }

    worker = new GpsLocationKitWorker();
    if (worker == NULL)
    {
        error = NE_BADDATA;
    }
    else
    {
        worker->m_locationKit = this;
        worker->m_trackingType = NB_TT_NarrowToAccurate;
        worker->m_accuracy = criteria.desiredAccuracy;
        for (size_t i = 0 ; i < configSize; i++)
        {
            if (nsl_strcmp(config[i].name, g_keyGpsMinAccuracy) == 0)
            {
                uint32 initialAccuracy = nsl_atoi(config[i].value);
                worker->m_initialAccuracy = initialAccuracy?initialAccuracy:NARROW_TO_ACCURATE_MIN_ACCURACY;
            }
            else if (nsl_strcmp(config[i].name, g_keyGpsTimeout) == 0)
            {
                uint32 timeOut = nsl_atoi(config[i].value);
                worker->m_timeout = timeOut?timeOut:NARROW_TO_ACCURATE_TIMEOUT;
            }
        }

        criteria.desiredAccuracy = (uint16)worker->m_initialAccuracy;
        error = InitRequest(criteria, config, configSize, listener, worker);
    }

    if (error != NE_OK)
    {
        listener->GpsErrorReceived(error);
    }

    return error;
}

NB_GpsLocation&
GpsLocationKit::GetLastLocation()
{
    return m_lastLocation;
}

void GpsLocationKit::SetLastLocation (NB_GpsLocation location)
{
    m_lastLocation = location;
}

NB_Error
GpsLocationKit::UpdateCriteria(LocationServiceListenerInterface* listener, NB_GpsCriteria criteria)
{
    NB_Error error = NE_OK;    
    GpsLocationKitWorker* worker = FindWorker(listener);
    ABPAL_GpsCriteria gpsCriteria = {0};
    InitGpsCriteria(gpsCriteria, criteria);

    if (worker)
    {
        PAL_Error err =  ABPAL_GpsUpdateCriteria(worker->m_context, &gpsCriteria, &worker->m_trackingInfo);
        error = CSL_TranslatePalError(err);
        if (error == NE_OK)
        {
            worker->m_criteria = gpsCriteria;
        }
    }
    return error;
}

NB_Error
GpsLocationKit::InitRequest(ABPAL_GpsCriteria& criteria, ABPAL_GpsConfig* config, uint32 configSize, LocationServiceListenerInterface* listener , GpsLocationKitWorker* worker)
{
    ABPAL_GpsContext* gpsContext;
    PAL_Error err = ABPAL_GpsCreate(m_pal, &gpsContext);
    NB_Error error = CSL_TranslatePalError(err);
    if(error != NE_OK)
    {
        return error;
    }

    if(worker)
    {
        worker->m_context = gpsContext;
        worker->m_criteria =criteria;
        worker->m_listener = listener;
        err = ABPAL_GpsInitialize(gpsContext, config, configSize, GpsInitCallback, NULL, worker);
        error = CSL_TranslatePalError(err);
        if (error != NE_OK)
        {
            delete worker;
        }
        else
        {
            m_listeners.push_back(worker);
        }
    }
    else 
    {
        return NE_NOMEM;
    }

    return error;
}

void
GpsLocationKit::GpsInitCallback(const void* userData, PAL_Error error, const char* errorInfo)
{
    if(!userData)
    {
        return;
    }
    GpsLocationKitWorker* worker = (GpsLocationKitWorker*)userData;
    GpsLocationKit* locationKit = worker->m_locationKit;

    ABPAL_GpsTrackingInfo trackingInfo = {0};
    
    NB_Error nberror = CSL_TranslatePalError(error);

    if(nberror == NE_OK)
    {
        if (worker->m_trackingType == NB_TT_Tracking || worker->m_trackingType == NB_TT_NarrowToAccurate)
        {
            error = ABPAL_GpsBeginTracking(worker->m_context, false, &worker->m_criteria, GpsTrackingCallback, userData, &trackingInfo);
            worker->m_trackingInfo = trackingInfo;
            nberror = CSL_TranslatePalError(error);
        }

        if (nberror != NE_OK)
        {
            worker->m_trackingType = NB_TT_Cancel;
            locationKit->RemoveClosedRequest();
            LocationServiceListenerInterface* listener = worker->m_listener;
            listener->GpsErrorReceived(nberror);
        }
        else if (worker->m_trackingType == NB_TT_NarrowToAccurate)
        {
            PAL_TimerSet(locationKit->m_pal, worker->m_timeout, StopNarrowToAccurate, worker);
        }
    }
    else
    {
        worker->m_trackingType = NB_TT_Cancel;
        locationKit->RemoveClosedRequest();
        LocationServiceListenerInterface* listener = worker->m_listener;
        listener->GpsErrorReceived(nberror);
    }
}

void
GpsLocationKit::GpsTrackingCallback(const void* userData, PAL_Error error, const ABPAL_GpsLocation* location, const char* errorInfoXml)
{
    if(!userData)
    {
        return;
    }
    GpsLocationKitWorker* worker = (GpsLocationKitWorker*)userData;
    LocationServiceListenerInterface* listener = worker->m_listener;
    NB_Error nberror = CSL_TranslatePalError(error);
    if (worker->m_trackingType != NB_TT_Cancel && worker->m_trackingType != NB_TT_None) 
    {
        if(nberror == NE_OK)
        {
            NB_GpsLocation gpsLocation;
            nsl_memset(&gpsLocation, 0, sizeof(NB_GpsLocation));
            AbpalGps::ConvertToNbGpsLocation(*location, gpsLocation);
            worker->m_locationKit->m_lastLocation = gpsLocation;

            if (worker->m_trackingType == NB_TT_NarrowToAccurate)
            {
                listener->LocationReceived(&gpsLocation);
                
                if (gpsLocation.horizontalUncertaintyAlongAxis <= worker->m_accuracy)
                {
                    GpsLocationKit* locationKit = worker->m_locationKit;
                    locationKit->StopTracking(worker->m_listener);
                    listener->AccurateLocationReceived(&gpsLocation);
                }
            }
            else if (worker->m_trackingType == NB_TT_Tracking)
            {
                listener->AccurateLocationReceived(&gpsLocation);
            }
        }
        else
        {
            if (worker->m_trackingType == NB_TT_NarrowToAccurate)
            {
                worker->m_locationKit->StopTracking(listener);
            }
            listener->GpsErrorReceived(nberror);
        }
    }
}

void
GpsLocationKit::StopNarrowToAccurate(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    GpsLocationKitWorker* worker = (GpsLocationKitWorker*)userData;
    if (worker->m_trackingType == NB_TT_NarrowToAccurate && reason == PTCBR_TimerFired)
    {
        worker->m_locationKit->StopTracking(worker->m_listener);
        worker->m_listener->GpsErrorReceived(NE_GPS_TIMEOUT);
    }
}


void
GpsLocationKit::InitGpsCriteria(ABPAL_GpsCriteria& gpsCriteria, NB_GpsCriteria criteria)
{
    gpsCriteria.desiredAccuracy      = criteria.desiredAccuracy;
    gpsCriteria.desiredInformation   = criteria.desiredInformation;
    gpsCriteria.desiredInterval      = criteria.desiredInterval;
    gpsCriteria.desiredMaxAge        = criteria.desiredMaxAge;
    gpsCriteria.desiredNetworkAssist = criteria.desiredNetworkAssist;
    gpsCriteria.desiredPerformance   = criteria.desiredPerformance;
    gpsCriteria.desiredSetting       = criteria.desiredSetting;
    gpsCriteria.desiredTimeout       = criteria.desiredTimeout;
    gpsCriteria.desiredTotalFixes    = criteria.desiredTotalFixes;
    gpsCriteria.desiredPositionSource = (ABPAL_PositionSource)criteria.desiredPositionSource;
    gpsCriteria.desiredCalibrationPopup = criteria.desiredCalibrationPopup;
    gpsCriteria.desiredHeadingSpeedLimitation = criteria.desiredHeadingSpeedLimitation;
}

void
GpsLocationKit::RemoveClosedRequest()
{
    GpsLocationKitWorker* worker = NULL;
    for(std::vector<GpsLocationKitWorker*>::iterator it = m_listeners.begin(); it != m_listeners.end();)
    {
        worker = *it;
        if (worker->m_trackingType == NB_TT_Cancel)
        {
            ABPAL_GpsDestroy(worker->m_context);
            delete worker;
            m_listeners.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

GpsLocationKitWorker*
GpsLocationKit::FindWorker(LocationServiceListenerInterface* listener)
{
    GpsLocationKitWorker* worker = NULL;
    for(size_t i=0;i<m_listeners.size();i++)
    {
        if((m_listeners[i])->m_listener == listener && (m_listeners[i])->m_trackingType != NB_TT_Cancel)
        {
            worker = (m_listeners[i]);
        }
    }
    return worker;
}

NB_TrackingType
GpsLocationKit::CurrentTrackingType(LocationServiceListenerInterface* listener)
{
    GpsLocationKitWorker* worker = FindWorker(listener);
    if (worker)
    {
        return worker->m_trackingType;
    }
    else
    {
        return NB_TT_None;
    }
}
