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

    @file     locationtracker.h
    @date     05/05/2011
    @defgroup LOCATIONSERVICES_LOCATIONREQUEST_GROUP

*/
/*
    LocationTracker class declaration. Tracking implementation

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

#include "locationcontext.h"
#include "base.h"

class GpsLocationTracker;
class NetworkLocationTracker;
class EmuLocationTracker;

/*! Class LocationTracker - class for tracking requests to NavBuilder
*/
class LocationTracker : public Base
{
public:
    LocationTracker(NB_LocationContext* locationContext, NB_Context* nbContext);
    virtual ~LocationTracker();

    void GpsInitialized(void);

    NB_LS_LocationState GetState(void);
    NB_Error BeginTracking(NB_LS_LocationCallback callback, void* userData, NB_LocationTrackingInfo** trackingInfo);
    NB_Error EndTracking(NB_LocationTrackingInfo* trackingInfo);

private:
    static void StaticGpsTrackerCallback(void* userData, NB_LocationContext* context,
        const NB_LS_Location* location, NB_Error error);
    void GpsTrackerCallback(const NB_LS_Location* location, NB_Error error);

    static void StaticNetworkTrackerCallback(void* userData, NB_LocationContext* context,
        const NB_LS_Location* location, NB_Error error);
    void NetworkTrackerCallback(const NB_LS_Location* location, NB_Error error);

    static void StaticEmuTrackerCallback(void* userData, NB_LocationContext* context,
        const NB_LS_Location* location, NB_Error error);
    void EmuTrackerCallback(const NB_LS_Location* location, NB_Error error);

    NB_LocationContext*         m_locationContext;
    GpsLocationTracker*      m_gpsTracker;
    NetworkLocationTracker*  m_networkTracker;
    EmuLocationTracker*      m_emuTracker;
    NB_LS_Location              m_gpsTrackingLocation;
    NB_LS_Location              m_networkTrackingLocation;
    NB_LS_Location              m_emuTrackingLocation;

    bool                        m_gotGpsFix;

    NB_LS_LocationCallback          m_callback;
    void*                       m_userData;
};

/*! @} */
