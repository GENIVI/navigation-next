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

#include "animationparameters.h"
using namespace locationtoolkit;

/*! AnimationParameters constructor.
 Constructor to use when creating an AnimationParameters object.
 @param accelerationType The acceleration type of the animation.
 @param duration The duration of the animation in milliseconds(>= 0).
 */
AnimationParameters::AnimationParameters(AccelerationType accelerationType, int duration)
    :mAnimationId(0),
     mAccelerationType(accelerationType),
     mDuration(duration)
{
}

AnimationParameters::~AnimationParameters()
{

}


/*! Gets the acceleration type of the animation.
 @return The acceleration type: AccelerationType
*/
locationtoolkit::AnimationParameters::AccelerationType AnimationParameters::GetAccelerationType() const
{
    return mAccelerationType;
}

/*! Sets the acceleration type of the animation.
 @param accelerationType The acceleration type:
 @return None.
*/
AnimationParameters& AnimationParameters::SetAccelerationType(AccelerationType accelerationType)
{
    mAccelerationType = accelerationType;
    return *this;
}

/*! Gets the duration of the animation.
 @return The duration in milliseconds.
*/
int AnimationParameters::GetDuration() const
{
    return mDuration;
}

/*! Sets the duration of the animation.
 @param duration The duration in milliseconds.
 @return None.
*/
AnimationParameters& AnimationParameters::SetDuration(int duration)
{
    mDuration = duration;
    return *this;
}

/*! Sets the id of the animation.(The default id is 0)
 @param id If this id is set to a nonzero value, the end of the animation can be tracked.
 @return This AnimationParameters object.
*/
AnimationParameters& AnimationParameters::SetAnimationId(int id)
{
    mAnimationId = id;
    return *this;
}

/*! Gets the id of the animation.
 @return The animation id.
*/
int AnimationParameters::GetAnimationId() const
{
    return mAnimationId;
}
