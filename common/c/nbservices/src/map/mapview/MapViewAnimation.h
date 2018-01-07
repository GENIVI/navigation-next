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
 @file     MapViewAnimation.h
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
/*! @{ */

#ifndef _MAP_VIEW_ANIMATION_H_
#define _MAP_VIEW_ANIMATION_H_
#include "nbgminterpolator.h"
#include "paltypes.h"
#include "palstdlib.h"

#include <vector>

namespace nbmap
{

class MapViewCameraListener;

/*! Described animation of all map transformation */
class MapViewAnimation
{
public:
    MapViewAnimation(MapViewCameraListener *listener);
    ~MapViewAnimation();

public:
    /*! All supported animation type */
    enum AnimationType
    {
        AT_TILT = 0,
        AT_ROTATE,
        AT_ZOOM,
        AT_SET_CENTER_X,
        AT_SET_CENTER_Y,
        AT_TOTAL
    };

    /*! Start a animation */
    void StartAnimation(AnimationType type, double start, double end, uint32 duration, NBGM_Interpolator<double>* interpolator);
    /*! Get interpolation of an animation */
    double GetInterpolation(AnimationType type, uint32 currentTime);

    /*! Stop animations */
    void StopAnimation(AnimationType type);
    void StopAnimations();
    void StopAnimations(const std::vector<AnimationType>& toStops);
    void StopAnimationsExcept(const std::vector<AnimationType>& noStops);

    /*! Get animation state */
    bool IsAnimating() const;
    bool IsAnimating(AnimationType type) const;

    void CheckAnimationStatus();
    void TraceAnimation(int id);

private:
    void UpdateTrancedAnimation();

private:
    NBGM_Interpolator<double>* m_animations[AT_TOTAL];
    uint32 m_startTime[AT_TOTAL];
    MapViewCameraListener* m_animationListener;
    int m_tracedAnimationId;
};
}


#endif //_MAP_VIEW_ANIMATION_H_

/*! @} */
