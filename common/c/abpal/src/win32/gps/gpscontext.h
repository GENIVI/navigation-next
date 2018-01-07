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

 @file gpscontext.h
 @date 4/18/12

 GPS context

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

#ifndef GPSCONTEXT_H
#define GPSCONTEXT_H

#ifdef __cplusplus
extern "C"
	{
#endif

#include "abpalgps.h"
#include "gpsprotocol.h"

typedef enum
{
    Gps_Simulation,
    Gps_Real,

}GPSFileType;

class GPSContext : public GpsContextProtocol, public GpsContextCallBackProtocol
{
public:
    GPSContext(PAL_Instance* pal);
    ~GPSContext();

    //GpsContextProtocol interfaces
    virtual PAL_Error initGpsConfig(const GpsConfiguration* config,
		                    ABPAL_GpsInitializeCallback* userCallback,
                                    ABPAL_GpsDeviceStateCallback* stateCallback,
                                    const void* data
                                    ) ;
    virtual ABPAL_GpsState handlerState();
    virtual PAL_Error getLocation(const ABPAL_GpsCriteria* criteria,
		                  ABPAL_GpsLocationCallback* callBack,
                                  const void* data
                                  );
    virtual PAL_Error cancelGetLocation();
    virtual PAL_Error beginTracking(const ABPAL_GpsCriteria* criteria,
		                    ABPAL_GpsLocationCallback* callBack,
                                    const void* data,
                                    ABPAL_GpsTrackingInfo* trackInfo
                                    );
    virtual PAL_Error endTracking(const ABPAL_GpsTrackingInfo* trackintInfo);
    virtual PAL_Error gpsSusPend(const ABPAL_GpsTrackingInfo* trackintInfo);
    virtual PAL_Error gpsResume(const ABPAL_GpsTrackingInfo* trackintInfo);
    virtual PAL_Error updateCriteria(const ABPAL_GpsCriteria* criteria,
		                     const ABPAL_GpsTrackingInfo* trackInfo
                                     );

    //GpsContextCallBackProtocol interfaces
    virtual void locationCallBack(ABPAL_GpsLocation* location,
                                  PAL_Error error,
                                  const char* errorInfo
                                  );
    virtual ABPAL_GpsCriteria palCriteria();
    virtual ABPAL_GpsState requestState();
public:
    ABPAL_GpsDeviceStateCallback*       stateChangeCallBack;
    ABPAL_GpsInitializeCallback*        initlizeCallBack;
    ABPAL_GpsLocationCallback*          gpsCallBack;

    PAL_Instance*                       pal_Instance;
    const void*                         userData;
    ABPAL_GpsCriteria                   palcriteria;
    GPSFileType                         gps_Type;

    GpsManagerProtocol*                 gpsManager;

    UINT_PTR                            timer;
    UINT_PTR                            timeout_timer;
    ABPAL_GpsState                      gpsState;

    BOOL                                hasFix;
    ABPAL_GpsLocation*                  lastLocation;
    PAL_Error                           lastLocationError;
    char*                               lastLocationInfo;

    UINT                                trackingInterval;

    BOOL                                hasCallBack; // for tracking
    BOOL                                firstTimeTracking;  // only for tracking
};

#ifdef __cplusplus
    }
#endif

#endif //GPSCONTEXT_H