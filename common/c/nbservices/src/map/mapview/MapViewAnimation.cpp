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
 @file     MapViewAnimation.cpp
 */
/*
 (C) Copyright 2011 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
#include "MapViewAnimation.h"
#include "palclock.h"
#include "MapViewCameraListener.h"
#include "MapViewInterface.h"

using namespace nbmap;

MapViewAnimation::MapViewAnimation(MapViewCameraListener *listener):m_animationListener(listener),m_tracedAnimationId(0)
{
    for(uint32 i=0; i < AT_TOTAL; ++i)
    {
        m_animations[i] = NULL;
        m_startTime[i] = 0;
    }
}

MapViewAnimation::~MapViewAnimation()
{
    for(uint32 i=0; i < AT_TOTAL; ++i)
    {
        delete m_animations[i];
        m_animations[i] = NULL;
    }
}

bool
MapViewAnimation::IsAnimating() const
{
    for(uint32 i = 0; i < AT_TOTAL; ++i)
    {
        if(m_animations[i])
        {
            bool finished = m_animations[i]->Finished();
            if(!finished)
            {
                return true;
            }
        }
    }
    return false;
}

bool
MapViewAnimation::IsAnimating(AnimationType type) const
{
    if(m_animations[type])
    {
        return !m_animations[type]->Finished();
    }
    return false;
}

void
MapViewAnimation::StopAnimation(AnimationType type)
{
    if(m_animations[type])
    {
        m_animations[type]->Stop();
    }
}

void
MapViewAnimation::StopAnimations()
{
    UpdateTrancedAnimation();
    for(uint32 i=0; i < AT_TOTAL; ++i)
    {
        if(m_animations[i])
        {
            m_animations[i]->Stop();
        }
    }
}

void
MapViewAnimation::StopAnimations(const std::vector<AnimationType>& toStops)
{
    UpdateTrancedAnimation();
    for(uint32 i=0; i < toStops.size(); ++i)
    {
        if(m_animations[toStops[i]])
        {
            m_animations[toStops[i]]->Stop();
        }
    }
}

void
MapViewAnimation::StopAnimationsExcept(const std::vector<AnimationType>& noStops)
{
    if(noStops.size() >= static_cast<uint32>(AT_TOTAL))
    {
        return;
    }

    bool nostop[AT_TOTAL];
    nsl_memset(nostop, false, sizeof(bool)*AT_TOTAL);

    for(uint32 i=0; i < noStops.size(); ++i)
    {
        nostop[noStops[i]] = true;
    }

    for(uint32 i=0; i<AT_TOTAL; ++i)
    {
        if(!nostop[i])
        {
            if(m_animations[nostop[i]])
            {
                m_animations[nostop[i]]->Stop();
            }
        }
    }
}

void
MapViewAnimation::StartAnimation(AnimationType type, double start, double end, uint32 duration, NBGM_Interpolator<double>* interpolator)
{
    if(m_animations[type])
    {
        delete m_animations[type];
        m_animations[type] = NULL;
    }

    if(interpolator)
    {
        m_startTime[type] = PAL_ClockGetTimeMs();
        m_animations[type] = interpolator;
        interpolator->Start(start, end, duration);
    }
}

double
MapViewAnimation::GetInterpolation(AnimationType type, uint32 currentTime)
{
    if(m_animations[type])
    {
        uint32 interval = currentTime - m_startTime[type];
        m_startTime[type] = currentTime;
        return m_animations[type]->GetInterpolation(interval);
    }
    return 0.0;
}

void MapViewAnimation::TraceAnimation(int id)
{
    UpdateTrancedAnimation();
    m_tracedAnimationId = id;
}

void
MapViewAnimation::CheckAnimationStatus()
{
    if(m_tracedAnimationId != 0)
    {
        if(!IsAnimating())
        {
            m_animationListener->OnCameraAnimationDone(m_tracedAnimationId, MVAST_FINISHED);
            m_tracedAnimationId = 0;
        }
    }
}

void
MapViewAnimation::UpdateTrancedAnimation()
{
    if(m_tracedAnimationId != 0)
    {
         m_animationListener->OnCameraAnimationDone(m_tracedAnimationId, IsAnimating()?MVAST_INTERRUPTED:MVAST_FINISHED);
         m_tracedAnimationId = 0;
    }
}
