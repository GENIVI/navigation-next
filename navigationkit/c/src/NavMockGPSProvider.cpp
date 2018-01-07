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

/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#include "NavMockGPSProvider.h"

using namespace nbnav;
using namespace std;

const static double DEFAULT_SPEED = 10;

NavMockGPSProvider::NavMockGPSProvider(NavMockGPSListener* listener, PAL_Instance* pal, NB_Navigation* navigation) :
    m_listener(listener),
    m_pal(pal),
    m_navigation(navigation),
    m_isRunning(false),
    m_speed(DEFAULT_SPEED),
    m_maneuverIndex(0),
    m_segmentIndex(0)
{
    nsl_memset(&m_lastPosition, 0, sizeof(NB_GpsLocation));
    m_lastPosition.latitude = INVALID_LATLON;
}

NavMockGPSProvider::~NavMockGPSProvider()
{
    Stop();
}

void NavMockGPSProvider::SetSpeed(double speed)
{
    m_speed = speed;
    if (m_speed <= 0)
    {
        m_speed = DEFAULT_SPEED;
    }
}

void NavMockGPSProvider::Reset()
{
    m_maneuverIndex = 0;
    m_segmentIndex = 0;
    m_lastPosition.latitude = INVALID_LATLON;
}

void NavMockGPSProvider::Stop()
{
    if (m_isRunning)
    {
        m_isRunning = false;
        PAL_TimerCancel(m_pal, &NavMockGPSProvider::PointUpdate, this);
    }
}

void NavMockGPSProvider::Start()
{
    if (!m_isRunning)
    {
        m_isRunning = true;
        PAL_TimerSet(m_pal, 1000, &NavMockGPSProvider::PointUpdate , this);
    }
}

void NavMockGPSProvider::Update()
{
    if (NB_NavigationGetNextMockPoint(m_navigation, &m_maneuverIndex, &m_segmentIndex,
        m_speed, m_lastPosition.latitude == INVALID_LATLON ? NULL : &m_lastPosition, &m_lastPosition) == NE_OK)
    {
        if (m_listener)
        {
            m_listener->OnMockGPSUpdate(&m_lastPosition);
        }
    }
}

void NavMockGPSProvider::PointUpdate(PAL_Instance* pal, void* userData, PAL_TimerCBReason reason)
{
    if (reason != PTCBR_TimerFired)
    {
        return;
    }

    NavMockGPSProvider* me = static_cast<NavMockGPSProvider*>(userData);
    if (!me->m_isRunning)
    {
        return;
    }
    me->Update();
    //Update can triger stop event, so need recheck m_isRunning
    if(me->m_isRunning)
    {
        PAL_TimerSet(me->m_pal, 1000, NavMockGPSProvider::PointUpdate, me);
    }
}
