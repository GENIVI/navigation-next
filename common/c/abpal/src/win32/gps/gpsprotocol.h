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

 GPS protocol

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
#ifndef GPS_PROTOCOL_H
#define GPS_PROTOCOL_H

#ifdef __cplusplus
extern "C"
	{
#endif


#include "abpalgps.h"
#include "gpsconfiguration.h"
#include "windows.h"

class GpsContextProtocol
{
public:
	virtual ~GpsContextProtocol() {};

    virtual PAL_Error initGpsConfig(const GpsConfiguration* config,
                                    ABPAL_GpsInitializeCallback* userCallback,
                                    ABPAL_GpsDeviceStateCallback* stateCallback,
                                    const void* data
                                    ) = 0;
    virtual ABPAL_GpsState handlerState() = 0;
    virtual PAL_Error getLocation(const ABPAL_GpsCriteria* criteria,
                                  ABPAL_GpsLocationCallback* callBack,
                                  const void* data
                                  ) = 0;
    virtual PAL_Error cancelGetLocation() = 0;
    virtual PAL_Error beginTracking(const ABPAL_GpsCriteria* criteria,
                                    ABPAL_GpsLocationCallback* callBack,
                                    const void* data,
                                    ABPAL_GpsTrackingInfo* trackInfo
                                    ) = 0;
    virtual PAL_Error endTracking(const ABPAL_GpsTrackingInfo* trackintInfo) = 0;
    virtual PAL_Error gpsSusPend(const ABPAL_GpsTrackingInfo* trackintInfo) = 0;
    virtual PAL_Error gpsResume(const ABPAL_GpsTrackingInfo* trackintInfo) = 0;
    virtual PAL_Error updateCriteria(const ABPAL_GpsCriteria* criteria,
                                     const ABPAL_GpsTrackingInfo* trackInfo
                                     ) = 0;
};

class GpsContextCallBackProtocol
{
public:
    virtual ~GpsContextCallBackProtocol() {};

    virtual void locationCallBack(ABPAL_GpsLocation* location,
                                  PAL_Error error,
                                  const char* errorInfo
                                  ) = 0;
    virtual ABPAL_GpsCriteria palCriteria() = 0;
    virtual ABPAL_GpsState requestState() = 0;
};

class GpsManagerProtocol
{
public:
    virtual ~GpsManagerProtocol() {};

    virtual void addObserver(GpsContextCallBackProtocol* observer) = 0;
    virtual PAL_Error start() = 0;
    virtual PAL_Error cancelRequest(GpsContextCallBackProtocol* observer) = 0;
    virtual PAL_Error setSimulationConfig(const GpsConfiguration* config) = 0;
    virtual void updateGpsCriteria() = 0;
    virtual PAL_Error cancelGetLocation() = 0;
    virtual PAL_Error endTracking() = 0;
    virtual BOOL isServiceEnable() = 0;
};

#ifdef __cplusplus
    }
#endif

#endif //GPS_PROTOCOL_H
