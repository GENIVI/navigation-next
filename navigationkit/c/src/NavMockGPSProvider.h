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
    @file     NavMockGPSProvider.h
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef __NAVMOCKGPSPROVIDER_H__
#define __NAVMOCKGPSPROVIDER_H__

extern "C"
{
#include "nbspatial.h"
#include "nbnavigation.h"
#include "paltimer.h"
}

#include "NavApiTypes.h"
#include <vector>

namespace nbnav
{
/*! The mock GSP listener who providers the gps location updating every second.
 */
class NavMockGPSListener
{
public:
    virtual void OnMockGPSUpdate(NB_GpsLocation* location) = 0;
    virtual ~NavMockGPSListener() {};
};

/*! The mock gps provider who use the polyline point of route to generate gps point.
 */
class NavMockGPSProvider
{
public:
    NavMockGPSProvider(NavMockGPSListener* listener, PAL_Instance* pal, NB_Navigation* navigation);
    ~NavMockGPSProvider();

    /*! Set the speed of each gps point. */
    void SetSpeed(double speed);

    /*! Reset the provider when route changed. */
    void Reset();

    /*! Start the provider updating. */
    void Start();

    /* Stop the provider running when arrived. */
    void Stop();

private:
    void Update();
    static void PointUpdate(PAL_Instance* pal, void* userData, PAL_TimerCBReason reason);

private:
    NavMockGPSListener* m_listener;
    PAL_Instance* m_pal;
    NB_Navigation* m_navigation;
    bool m_isRunning;
    double m_speed;
    uint32 m_maneuverIndex;
    uint32 m_segmentIndex;
    NB_GpsLocation m_lastPosition;
};
}

#endif
/*! @} */