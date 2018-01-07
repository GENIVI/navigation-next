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

    @file     gpscontext.cpp
    @date     09/11/2012
    @defgroup GPSCONTEXT_H PAL Gps Context

    @brief    GPS context

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

#include "gpscontext.h"
#include "gpsfactory.h"

#define GPS_TIMEOUT 15000            //GPS time out in milliseconds

GpsContext::GpsContext(PAL_Instance* pal):
    m_pPal(pal),
    m_pGpsProvider(NULL),
    m_state(PGS_Undefined),
    m_pInitCallback(NULL),
    m_pDeviceStateCallback(NULL),
    m_pUserData(NULL),
    m_pLock(NULL),
    m_trackId(0),
    m_listCount(0),
    m_addTrackInfoTimer(0),
    m_pGpsListeners()
{
    PAL_LockCreate(pal, &m_pLock);
}

GpsContext::~GpsContext()
{
    PAL_TimerCancel(m_pPal, TimerCallback, this);

    if (m_pGpsProvider != NULL)
    {
        m_pGpsProvider->RemoveListner(this);
        m_pGpsProvider->RemoveInstance();
        m_pGpsProvider = NULL;
    }
    PAL_LockLock(m_pLock);
  	m_pGpsListeners.clear();
  	PAL_LockUnlock(m_pLock);

    PAL_LockDestroy(m_pLock);
}

PAL_Error GpsContext::initGpsConfig(const ABPAL_GpsConfig* gpsConfig, uint32 configCount,
                      ABPAL_GpsInitializeCallback* initCallback,
                      ABPAL_GpsDeviceStateCallback* deviceStateCallback,
                      const void * cbData)
{
    PAL_Error error = PAL_Ok;
    if (m_pGpsProvider != NULL)
    {
        m_pGpsProvider->RemoveInstance();
        m_pGpsProvider = NULL;
    }

    m_pGpsProvider = GpsProviderFactory::getGPSProvider(m_pPal, gpsConfig, configCount);
    if (m_pGpsProvider == NULL)
    {
        return PAL_ErrNoInit;
    }

    error = m_pGpsProvider->Initialize(gpsConfig, configCount);

    if (error == PAL_Ok)
    {
        m_pGpsProvider->AddListner(this);

        m_pInitCallback = initCallback;
        m_pDeviceStateCallback = deviceStateCallback;
        m_pUserData = (void*)(cbData);
        PAL_TimerSet(m_pPal, 200, TimerCallback, this);
    }

    return error;
}

PAL_Error GpsContext::getLocation(const ABPAL_GpsCriteria* criteria,
                    ABPAL_GpsLocationCallback* callBack,
                    const void * cbData)
{
    if (m_pGpsProvider == NULL)
    {
        return PAL_ErrNoInit;
    }

    AddGpsListener(criteria, callBack, cbData);

    if (m_state != PGS_Tracking)
    {
        m_state = PGS_Oneshot;
    }

    return PAL_Ok;
}

PAL_Error GpsContext::cancelGetLocation()
{
    return PAL_Ok;
}

PAL_Error GpsContext::beginTracking(const ABPAL_GpsCriteria* criteria,
                      ABPAL_GpsLocationCallback* callBack,
                      const void * data,
                      ABPAL_GpsTrackingInfo* trackInfo)
{
    if (m_pGpsProvider == NULL)
    {
        return PAL_ErrNoInit;
    }

    AddGpsListener(criteria, callBack, data, trackInfo);

    m_state = PGS_Tracking;

    return PAL_Ok;
}

PAL_Error GpsContext::endTracking(const ABPAL_GpsTrackingInfo* trackintInfo)
{
	RemoveGpsListener(trackintInfo);
    return PAL_Ok;
}

PAL_Error GpsContext::gpsSuspend(const ABPAL_GpsTrackingInfo* trackintInfo)
{
    return PAL_Ok;
}

PAL_Error GpsContext::gpsResume(const ABPAL_GpsTrackingInfo* trackintInfo)
{
    return PAL_Ok;
}

PAL_Error GpsContext::updateCriteria(const ABPAL_GpsCriteria* criteria, const ABPAL_GpsTrackingInfo* trackInfo)
{
    if (criteria == NULL)
    {
        return PAL_ErrBadParam;
    }

    if (m_pGpsProvider == NULL)
    {
        return PAL_ErrNoInit;
    }

    m_pGpsProvider->setPeriod(criteria->desiredInterval);

    return PAL_Ok;
}

void GpsContext::TimerCallback(PAL_Instance *pal, void *userData, PAL_TimerCBReason reason)
{
    GpsContext* pThis = reinterpret_cast<GpsContext*>(userData);

    if (reason == PTCBR_TimerFired)
    {
        pThis->m_state = PGS_Initialized;
        if (pThis->m_pInitCallback)
        {
            pThis->m_pInitCallback(pThis->m_pUserData, PAL_Ok, NULL);
        }
    }
}

ABPAL_GpsState GpsContext::getState()
{
    return m_state;
}

void GpsContext::AddGpsListener(const ABPAL_GpsCriteria* criteria,
                          ABPAL_GpsLocationCallback* callBack,
                          const void * data,
                          ABPAL_GpsTrackingInfo* trackInfo)
{
    PAL_LockLock(m_pLock);
    uint32 trackID = 0;
    if (trackInfo != NULL)
    {
        trackInfo->Id = trackID = ++m_trackId;
    }

    m_pGpsListeners.push_front(new GpsListener(criteria, callBack, const_cast<void*>(data), trackID));

    m_pGpsProvider->setPeriod(criteria->desiredInterval);

    if (m_listCount == 0)
    {
        m_pGpsProvider->start();
    }
    m_listCount++;

    PAL_LockUnlock(m_pLock);
}

void GpsContext::RemoveGpsListener(const ABPAL_GpsTrackingInfo* trackintInfo)
{
    PAL_LockLock(m_pLock);
    std::list<GpsListener*>::iterator listIterator;
    for (listIterator = m_pGpsListeners.begin(); listIterator != m_pGpsListeners.end(); ++listIterator)
    {
        if((*listIterator)->getTrackID() == trackintInfo->Id){
        	m_pGpsListeners.remove(*listIterator);
        	m_listCount--;
        	break;
        }
    }
    if (m_listCount == 0)
    {
        m_pGpsProvider->stop();
    }
    PAL_LockUnlock(m_pLock);
}

void GpsContext::NotifyGpsListeners(PAL_Error error, const ABPAL_GpsLocation& location)
{
    PAL_LockLock(m_pLock);
    std::list<GpsListener*>::iterator listIterator;
    for (listIterator = m_pGpsListeners.begin(); listIterator != m_pGpsListeners.end(); ++listIterator)
    {
    	(*listIterator)->Notify(error,location);
    }
    PAL_LockUnlock(m_pLock);
}

void GpsContext::LocationCallback(PAL_Error error, const ABPAL_GpsLocation& location)
{
	NotifyGpsListeners(error, location);
}

/*! @} */
