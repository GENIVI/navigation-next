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

    @file     gpscontext.h
    @date     09/11/2012
    @defgroup GPSCONTEXT_H PAL Gps Context

    @brief    GPS context

*/
/*
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

#ifndef GPSCONTEXT_H
#define GPSCONTEXT_H

#include "palstdlib.h"
#include "abpalgps.h"
#include "gpsprovider.h"
#include "gpslistener.h"
#include "locationlistener.h"
#include "paltimer.h"
#include "pallock.h"
#include <list>

typedef struct TrackInformation TrackInformation;


class GpsContext: public LocationListner
{
    public:
        GpsContext(PAL_Instance* pal);
        ~GpsContext();
        PAL_Error initGpsConfig(const ABPAL_GpsConfig* gpsConfig,
                      uint32 configCount,
                      ABPAL_GpsInitializeCallback* initCallback,
                      ABPAL_GpsDeviceStateCallback* deviceStateCallback,
                      const void * cbData);
        PAL_Error getLocation(const ABPAL_GpsCriteria* criteria,
                    ABPAL_GpsLocationCallback* callBack,
                    const void * cbData);
        PAL_Error cancelGetLocation();
        PAL_Error beginTracking(const ABPAL_GpsCriteria* criteria,
                      ABPAL_GpsLocationCallback* callBack,
                      const void * data,
                      ABPAL_GpsTrackingInfo* trackInfo);
        PAL_Error endTracking(const ABPAL_GpsTrackingInfo* trackintInfo);
        PAL_Error updateCriteria(const ABPAL_GpsCriteria* criteria, const ABPAL_GpsTrackingInfo* trackInfo = NULL);
        PAL_Error gpsSuspend(const ABPAL_GpsTrackingInfo* trackintInfo);
        PAL_Error gpsResume(const ABPAL_GpsTrackingInfo* trackintInfo);
        ABPAL_GpsState getState();

        virtual void LocationCallback(PAL_Error error, const ABPAL_GpsLocation& location);

    private:
        void AddGpsListener(const ABPAL_GpsCriteria* criteria,
                          ABPAL_GpsLocationCallback* callBack,
                          const void * data,
                          ABPAL_GpsTrackingInfo* trackInfo = NULL);

        void RemoveGpsListener(const ABPAL_GpsTrackingInfo* trackintInfo);
        void NotifyGpsListeners(PAL_Error error, const ABPAL_GpsLocation& location);

        static void TimerCallback(PAL_Instance *pal,
                                  void *userData,
                                  PAL_TimerCBReason reason);

        PAL_Instance* m_pPal;
        GpsProvider* m_pGpsProvider;
        std::list<GpsListener*> m_pGpsListeners;

        ABPAL_GpsState m_state;

        ABPAL_GpsInitializeCallback* m_pInitCallback;
        ABPAL_GpsDeviceStateCallback* m_pDeviceStateCallback;
        void* m_pUserData;

        PAL_Lock* m_pLock;
        uint32 m_trackId;
        uint32 m_listCount;
        uint32 m_addTrackInfoTimer;
};
#endif
