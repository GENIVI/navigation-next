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
 
 @file nbgmdecelerateinterpolator.h
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
#ifndef _NBGM_DECELERATE_INTERPOLATOR_H_
#define _NBGM_DECELERATE_INTERPOLATOR_H_
#include "nbgminterpolator.h"

/*! \addtogroup NBGM_Manager
 *  @{
 */

/*! An interpolator defines the rate of change of an animation. This allows the
 basic animation effects (alpha, scale, translate, rotate) to be
 accelerated, decelerated, repeated, etc.
 */
template<typename T>
class NBGM_DecelerateInterpolator:public NBGM_Interpolator<T>
{
public:
    NBGM_DecelerateInterpolator(){;}
    
public:
    T GetInterpolation(uint32 interval)
    {
        this->m_CurrentTime += interval;
        if(this->m_CurrentTime >= this->m_duration)
        {
            this->m_CurrentTime = this->m_duration;
            this->m_Current = this->m_End;
        }
        else
        {
            T velocity = 2*(this->m_End - this->m_Start);
            T acc = -velocity;
            T t = this->m_CurrentTime/(T)this->m_duration;
            //s=vt + at^2/2
            this->m_Current = this->m_Start + velocity * t + static_cast<T>(0.5 * acc * t * t);
        }
        return this->m_Current;
    }
};

/*! @} */
#endif
