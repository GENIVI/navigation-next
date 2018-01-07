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

    @file     gpslocationrequest.h
    @date     05/05/2011
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
    NBSearchRequest class declaration

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
#include "locationrequest.h"
#include "lsabpalgps.h"

extern "C" {
  #include "abpalgps.h"
}

class AbpalGps;

class GpsLocationRequest : public LocationRequest
{
public:
    GpsLocationRequest(NB_LocationContext* locationContext, AbpalGps* gps);
    virtual ~GpsLocationRequest(void);

    virtual NB_Error RequestStart(NB_LS_LocationCallback callback, void* applicationData, NB_LS_FixType fixType = NBFixType_Normal);

    virtual NB_Error GetState(NB_LS_LocationState * state);
    void GpsInitialized(void);
    virtual bool IsCachedLocationValid(NB_LS_FixType fixType);
    void CancelOutstandingRequests();
    bool IsGpsCold() { return m_gpsCold; }

protected:
    virtual NB_Error StartLocationRequest();
    virtual bool IsInProgress();
    virtual void CancelLocationRequest();
    virtual NB_LS_Location* RetrieveLocation();
    virtual int GetSpecificTimeout(NB_LS_FixType fixType);
    virtual void LocationCallback(NB_Error error);

private:
    static void StaticGpsLocationCallback(const void* userData, PAL_Error error, const ABPAL_GpsLocation* location, const char* errorInfoXml);
    void GpsLocationCallback(PAL_Error error, const ABPAL_GpsLocation* location, const char* errorInfoXml);
    static void StaticColdGPSTimerCallback (PAL_Instance *pal, void *userData, PAL_TimerCBReason reason);
    void ColdGPSTimerCallback ();

private:
    AbpalGps*        m_gps;
    NB_LS_Location         m_gpsLocation;
    NB_LS_LocationState    m_state;
    bool                   m_isInProgress;
    bool                   m_gpsCold;
};

/*! @} */
