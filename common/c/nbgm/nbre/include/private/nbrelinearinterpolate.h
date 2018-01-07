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

    @file nbrelinearinterpolate.h
*/
/*
    (C) Copyright 2012 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef _NBRE_LINEAR_INTERPOLATE_H_
#define _NBRE_LINEAR_INTERPOLATE_H_

#include "paltypes.h"
#include "nbrevector2.h"
#include "nbrevector3.h"
#include "nbrevector4.h"
/** \addtogroup NBRE_Core
*  @{
*/

//! Linear interpolation algorithms
/** 
*/
template <typename T>
class NBRE_LinearInterpolate
{
public:    
    static T Lerp(T v0, T v1, T t);
    static NBRE_Vector2<T> Lerp(const NBRE_Vector2<T>& v0, const NBRE_Vector2<T>& v1, T t);
    static NBRE_Vector3<T> Lerp(const NBRE_Vector3<T>& v0, const NBRE_Vector3<T>& v1, T t);
    static NBRE_Vector4<T> Lerp(const NBRE_Vector4<T>& v0, const NBRE_Vector4<T>& v1, T t);
};

template <typename T>
inline T 
NBRE_LinearInterpolate<T>::Lerp(T v0, T v1, T t)
{   
    return v0 * static_cast<T>(1.0 - t) + v1 * t;
}

template <typename T>
inline NBRE_Vector2<T>
NBRE_LinearInterpolate<T>::Lerp(const NBRE_Vector2<T>& v0, const NBRE_Vector2<T>& v1, T t)
{
    T inverseT = static_cast<T>(1.0 - t);
    return NBRE_Vector2<T>(v0.x * inverseT + v1.x * t
                          ,v0.y * inverseT + v1.y * t);
}

template <typename T>
inline NBRE_Vector3<T>
NBRE_LinearInterpolate<T>::Lerp(const NBRE_Vector3<T>& v0, const NBRE_Vector3<T>& v1, T t)
{
    T inverseT = static_cast<T>(1.0 - t);
    return NBRE_Vector3<T>(v0.x * inverseT + v1.x * t
                          ,v0.y * inverseT + v1.y * t
                          ,v0.z * inverseT + v1.z * t);
}

template <typename T>
inline NBRE_Vector4<T>
NBRE_LinearInterpolate<T>::Lerp(const NBRE_Vector4<T>& v0, const NBRE_Vector4<T>& v1, T t)
{
    // Perspective projection using w value
    t = v0.w * t / (v1.w + (v0.w - v1.w) * t);
    T inverseT = static_cast<T>(1.0 - t);

    return NBRE_Vector4<T>(v0.x * inverseT + v1.x * t
                          ,v0.y * inverseT + v1.y * t
                          ,v0.z * inverseT + v1.z * t
                          ,v0.w * inverseT + v1.w * t);
}

typedef NBRE_LinearInterpolate<float> NBRE_LinearInterpolatef;
typedef NBRE_LinearInterpolate<double> NBRE_LinearInterpolated;

/** @} */
#endif
