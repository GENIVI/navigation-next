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

    @file     AbpalGps.cpp
    @date     05/05/2011
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
    AbpalGps class declaration. This is ABPAL GPS object wrapper

    (C) Copyright 2011 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*! @{ */



#include "lsabpalgps.h"
#include "gpslocationrequest.h"
#include "trackermanager.h"
#include "palmath.h"
extern "C" {
  #include "nbqalog.h"
}

  #include "palmath.h"
const int AbpalGps::TRACKING_FIXES_INTERVAL_CONSTANT = 2000; // in milliseconds

AbpalGps::AbpalGps(NB_LocationContext* locationContext)
    : m_locationContext(locationContext),
      m_gpsContext(NULL),
      m_initialized(FALSE),
      m_gpsInitCompleted(FALSE)
{
}

AbpalGps::~AbpalGps()
{
    if (m_gpsContext != NULL)
    {
        ABPAL_GpsDestroy(m_gpsContext);
    }
}

ABPAL_GpsContext* AbpalGps::GpsContext(void)
{
    return m_gpsContext;
}

NB_Error AbpalGps::Initialize(const NB_LS_LocationConfig* config, const ABPAL_GpsConfig* gpsConfig, uint32 gpsConfigCount)
{
    if (m_initialized)
    {
        return NE_OK;
    }

    NB_Error ret = NE_GPS;

    if (m_locationContext != NULL && m_locationContext->m_PAL_Instance != NULL)
    {
        PAL_Error err = PAL_Ok;
        if (m_gpsContext == NULL)
        {
            err = ABPAL_GpsCreate(m_locationContext->m_PAL_Instance,
                &m_gpsContext);
        }

        if (err == PAL_Ok)
        {

            // Try to initialize GPS
            err = ABPAL_GpsInitialize(m_gpsContext,
                                      gpsConfig,
                                      gpsConfigCount,
                                      &StaticGpsInitializeCallback,
                                      &StaticGpsDeviceStateCallback,
                                      this);
            if (err == PAL_Ok)
            {
                m_initialized = TRUE;
                ret = NE_OK;
            }
        }
    }

    return ret;
}

void AbpalGps::StaticGpsInitializeCallback(const void* userData, PAL_Error error, const char *errorInfoXml)
{
    AbpalGps* me = (AbpalGps *)userData;
    if (me != NULL)
    {
        me->GpsInitializeCallback(error, errorInfoXml);
    }
}

bool AbpalGps::IsGPSInitialized()
{
    return m_gpsInitCompleted;
}

bool AbpalGps::IsGPSEnabled()
{
    nb_boolean ret = ABPAL_IsGpsEnabled(m_gpsContext);
    DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]GPS enabled:%d", ret));

    return (ret == TRUE);
}

void AbpalGps::GpsInitializeCallback(PAL_Error error, const char *errorInfoXml)
{
    if (error == PAL_Ok)
    {
        m_gpsInitCompleted = TRUE;
        m_locationContext->m_gpsLocationRequest->GpsInitialized();

        // Do the warm-up fix, just to speed up the satellite search. We don't care about the results.
        if (m_locationContext->locationConfig.warmUpFix)  
        {
            StartWarmupFix();
        }
    }

    // The initialization callback does not return an error per NBI Phase I API (even if PAL may return an error in case initialization is failed)
    // The question remains, whether callback shall ba called at all in case of PAL error.
    if (m_locationContext->initializeCallback)
    {
        NB_QaLogApiCall(m_locationContext->internalContext, NB_QLAC_StartCallback, __FUNCTION__, "NB_LocationInitializeCallback");
        m_locationContext->initializeCallback((void*)m_locationContext->callbackData, m_locationContext);
        NB_QaLogApiCall(m_locationContext->internalContext, NB_QLAC_CallbackReturn, __FUNCTION__, "NB_LocationInitializeCallback");
    }
}

NB_Error AbpalGps::StartWarmupFix()
{
    if(IsGPSInitialized() == false)
    {
        return NE_GPS;
    }

    NB_Error retStatus = NE_OK;

    // It makes sense to ask for WUF only in case of a cold GPS with no warm-up fix in progress
    if (!m_locationContext->m_gpsLocationRequest->AlreadyStarted(StaticWarmupFixCallback, NULL) && 
        m_locationContext->m_gpsLocationRequest->IsGpsCold())
    {
        DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]Warm-up fix requested", 1));
        retStatus = m_locationContext->m_gpsLocationRequest->RequestStart(StaticWarmupFixCallback, NULL, NBFixType_Accurate);
    }

    // Reset the warm-up fix flag
    m_locationContext->locationConfig.warmUpFix = FALSE;

    return retStatus;
}

void AbpalGps::StaticGpsDeviceStateCallback(const void *userData, const char *stateChangeXml,
                                               PAL_Error error, const char *errorInfoXml)
{
    AbpalGps* me = (AbpalGps *)userData;
    if (me != NULL)
    {
        me->GpsDeviceStateCallback(stateChangeXml, error, errorInfoXml);
    }
}

void AbpalGps::GpsDeviceStateCallback(const char *stateChangeXml, PAL_Error error,
                                                   const char *errorInfoXml)
{
    // Currently, we do not use CCC's device state callback. Leaving it as a stub.
}

void AbpalGps::InitGpsCriteria(ABPAL_GpsCriteria& gpsCriteria, int timeout, int maxAge)
{
    gpsCriteria.desiredAccuracy      = m_locationContext->gpsDesiredAccuracy;
    gpsCriteria.desiredInformation   = PGI_Location | PGI_Velocity | PGI_Altitude;
    gpsCriteria.desiredInterval      = TRACKING_FIXES_INTERVAL_CONSTANT;
    gpsCriteria.desiredMaxAge        = maxAge;
    gpsCriteria.desiredNetworkAssist = 0;
    gpsCriteria.desiredPerformance   = 0;
    gpsCriteria.desiredSetting       = PGC_MaxAge;
    gpsCriteria.desiredTimeout       = timeout;
    gpsCriteria.desiredTotalFixes    = 0;
}

void AbpalGps::ConvertToNBLocation(NB_LS_Location& lsLocation, const ABPAL_GpsLocation& gpsLocation)
{
    lsLocation.altitude                                = gpsLocation.altitude;
    lsLocation.gpsTime                                 = gpsLocation.gpsTime;
    lsLocation.heading                                 = gpsLocation.heading;
    lsLocation.horizontalUncertaintyAlongAxis          = gpsLocation.horizontalUncertaintyAlongAxis;
    lsLocation.horizontalUncertaintyAlongPerpendicular = gpsLocation.horizontalUncertaintyAlongPerpendicular;
    lsLocation.horizontalUncertaintyAngleOfAxis        = gpsLocation.horizontalUncertaintyAngleOfAxis;
    lsLocation.horizontalVelocity                      = gpsLocation.horizontalVelocity;
    lsLocation.latitude                                = gpsLocation.latitude;
    lsLocation.longitude                               = gpsLocation.longitude;
    lsLocation.numberOfSatellites                      = gpsLocation.numberOfSatellites;
    lsLocation.status                                  = gpsLocation.status;
    lsLocation.utcOffset                               = gpsLocation.utcOffset;
    lsLocation.verticalUncertainty                     = gpsLocation.verticalUncertainty;
    lsLocation.verticalVelocity                        = gpsLocation.verticalVelocity;

    lsLocation.valid = gpsLocation.valid & ~NBPGV_UTCOffset;    // NBPGV_UTCOffset isn't always valid, so just clear it
    // ABPAL and NB GPS location structures use different flag position for satellite count
    lsLocation.valid = lsLocation.valid & ~PGV_SatelliteCount; // clear ABPAL satellite count flag
    if (gpsLocation.valid & PGV_SatelliteCount)
    {
        lsLocation.valid |= NBPGV_SatelliteCount; // set NB satellite count flag if valid
    }
}

void AbpalGps::ConvertToNbGpsLocation(const ABPAL_GpsLocation& gpsLocation, NB_GpsLocation& nbGpsLocation)
{
    nbGpsLocation.altitude                                = gpsLocation.altitude;
    nbGpsLocation.gpsTime                                 = gpsLocation.gpsTime;
    nbGpsLocation.heading                                 = gpsLocation.heading;
    nbGpsLocation.horizontalUncertaintyAlongAxis          = gpsLocation.horizontalUncertaintyAlongAxis;
    nbGpsLocation.horizontalUncertaintyAlongPerpendicular = gpsLocation.horizontalUncertaintyAlongPerpendicular;
    nbGpsLocation.horizontalUncertaintyAngleOfAxis        = gpsLocation.horizontalUncertaintyAngleOfAxis;
    nbGpsLocation.horizontalVelocity                      = gpsLocation.horizontalVelocity;
    nbGpsLocation.latitude                                = gpsLocation.latitude;
    nbGpsLocation.longitude                               = gpsLocation.longitude;
    nbGpsLocation.numberOfSatellites                      = gpsLocation.numberOfSatellites;
    nbGpsLocation.status                                  = (NB_Error)gpsLocation.status;
    nbGpsLocation.utcOffset                               = gpsLocation.utcOffset;
    nbGpsLocation.valid                                   = gpsLocation.valid;
    nbGpsLocation.verticalUncertainty                     = gpsLocation.verticalUncertainty;
    nbGpsLocation.verticalVelocity                        = gpsLocation.verticalVelocity;
}

void AbpalGps::StaticWarmupFixCallback(void* appData, NB_LocationContext* context, const NB_LS_Location* location, NB_Error error)
{
    if (location && location->valid)
    {
        DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]Warm-up fix: success", 1));
    }
    else
    {
        DEBUGLOG(LOG_SS_CORE, LOG_SEV_DEBUG, ("[NB_LS_LOG]Warm-up fix: error %d", error));
    }
}

/*! @} */
