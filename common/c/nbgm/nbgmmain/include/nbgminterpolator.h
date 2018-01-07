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

    @file nbgminterpolator.h
*/
/*
    (C) Copyright 2010 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _NBGM_INTERPOLATOR_H_
#define _NBGM_INTERPOLATOR_H_
#include "paltypes.h"

/*! \addtogroup NBGM_Manager
*  @{
*/

/*! An interpolator defines the rate of change of an animation. This allows the
    basic animation effects (alpha, scale, translate, rotate) to be
    accelerated, decelerated, repeated, etc.
*/
template<typename T>
class NBGM_Interpolator
{
public:
    NBGM_Interpolator():m_Start(0), m_End(0), m_Current(0), m_CurrentTime(0), m_duration(0){;}
    virtual ~NBGM_Interpolator(){;}
public:
    void Start(T start, T end, uint32 duration)
    {
        m_Start = start;
        //combine the previous animation if any
        m_End = end+(m_End - m_Current);

        m_duration = duration;
        m_CurrentTime = 0;
    }

    void Stop()
    {
        m_Start = 0;
        m_End = 0;
        m_Current = 0;
        m_CurrentTime = 0;
        m_duration = 0;
    }

    bool Finished() const
    {
        return m_CurrentTime>=m_duration;
    }

    /*!Maps a value representing the elapsed fraction of an animation to a
     value that represents the interpolated fraction. This interpolated
     value is then multiplied by the change in value of an animation to
     derive the animated value at the current elapsed animation time.
     @param input A value between 0 and 1.0 indicating our current point
     in the animation where 0 represents the start and 1.0 represents the end
     @return The interpolation value. This value can be more than 1.0
     for interpolators which overshoot their targets, or less than 0 for
     interpolators that undershoot their targets.
     */
    virtual T GetInterpolation(uint32 interval) = 0;

protected:
    T m_Start;
    T m_End;
    T m_Current;
    
    uint32 m_CurrentTime;
    uint32 m_duration;
};

/*! @} */
#endif
