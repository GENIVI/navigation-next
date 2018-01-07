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

    @file     lsabpalgps.h
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

#pragma once
#include "base.h"
#include "locationcontext.h"

extern "C"
{
  #include "abpalgps.h"
  #include <nbgpstypes.h>
}

#define ABPAL_GPS_FIX_MINIMAL_MAXAGE  1

class AbpalGps : public Base
{
public:
    static const int TRACKING_FIXES_INTERVAL_CONSTANT;

    AbpalGps(NB_LocationContext* locationContext);
    virtual ~AbpalGps(void);
    NB_Error Initialize(const NB_LS_LocationConfig* config, const ABPAL_GpsConfig* gpsConfig, uint32 gpsConfigCount);
    bool IsGPSInitialized();
    bool IsGPSEnabled();
    ABPAL_GpsContext* GpsContext(void);

    void InitGpsCriteria(ABPAL_GpsCriteria& gpsCriteria, int timeout, int maxAge=ABPAL_GPS_FIX_MINIMAL_MAXAGE); // minimal caching, by default
    static void ConvertToNBLocation(NB_LS_Location& lsLocation, const ABPAL_GpsLocation& gpsLocation);
    static void ConvertToNbGpsLocation(const ABPAL_GpsLocation& gpsLocation, NB_GpsLocation& nbGpsLocation);

    NB_Error StartWarmupFix();

private:
    static void StaticGpsInitializeCallback(const void* userData, PAL_Error error, const char *errorInfoXml);
    void GpsInitializeCallback(PAL_Error error, const char *errorInfoXml);

    static void StaticGpsDeviceStateCallback(const void *userData, const char *stateChangeXml, PAL_Error error, const char *errorInfoXml);
    void GpsDeviceStateCallback(const char *stateChangeXml, PAL_Error error, const char *errorInfoXml);

    static void StaticWarmupFixCallback(void* appData, NB_LocationContext* context, const NB_LS_Location* location, NB_Error error);

private:
    NB_LocationContext*   m_locationContext;
    ABPAL_GpsContext*     m_gpsContext;
    bool                  m_initialized;
    bool                  m_gpsInitCompleted;
};

/*! @} */
