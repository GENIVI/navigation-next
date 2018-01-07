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

    @file     hsgpsprovider.h
    @date     02/25/2014
    @defgroup HSGPSPROVIDER_H HS Gps

    HS Gps
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

#ifndef HSGPSPROVIDER_H
#define HSGPSPROVIDER_H

#include "gpsprovider.h"
#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

class HsGpsProvider: public GpsProvider
{
public:
    static HsGpsProvider* GetInstantce(PAL_Instance* pal);
    static HsGpsProvider* GetAvailableInstantce();

    virtual PAL_Error Initialize(const ABPAL_GpsConfig* gpsConfig, uint32 configCount);
    virtual void start();
    virtual void stop();
    virtual void updateCriteria(const ABPAL_GpsCriteria* criteria);
    virtual PAL_Error enableBackgroundGeolocation(nb_boolean isEnabled);
    virtual void RemoveInstance();
    void NotifyLocationInfomation();
    void LocationInfoProcess(ABPAL_GpsLocation &LocationInfo, bool isGpsFix);

private:
    HsGpsProvider(PAL_Instance* pal);
    virtual ~HsGpsProvider();
    bool logLocationAllowed(const ABPAL_GpsConfig* config, uint32 configCount);
    void RecordLocation(ABPAL_GpsLocation &location, ABPAL_GpsLocation &lastLocation, bool coarse);
    static void* GpsWorkerThreadFunc(void* user_data);
    bool IsLocationUpdated(ABPAL_GpsLocation &oldLocation, ABPAL_GpsLocation &newlastLocation);

    PAL_Instance*          m_pPal;

    // while navigating, the GPS fix still need get when app at background.
    // this bool mark that, whether should we stop when the stop event come in.
    bool                   m_backgroundEventsEnabled;
    bool                   m_logLocationAllowed;
    bool                   m_bThreadWorking;
    pthread_t              m_threadId;
    int                    m_serialHandle;
    bool                   m_gpsupdated;
    ABPAL_GpsLocation      m_gpsLocation;
    static uint32          m_Ref;
    static HsGpsProvider*  m_pHsGpsProvider;
    static pthread_mutex_t m_lock;
};

#endif  /* REALGPSPROVIDER_H */

/*! @} */
