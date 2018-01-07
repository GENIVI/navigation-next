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

    @file nbrefrustum3.h
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

#ifndef _NBRE_FRUSTUM3_H_
#define _NBRE_FRUSTUM3_H_

#include "paltypes.h"
#include "nbrevector3.h"
#include "nbrematrix4x4.h"
#include "nbreaxisalignedbox3.h"
#include "nbreplane.h"

/** \addtogroup NBRE_Core
*  @{
*/

//! 3D frustum class
/** Defined by 6 planes. Cache corner points & AABB to improve performance.
*/
template <typename Real>
class NBRE_Frustum3
{
public:
    NBRE_Frustum3();
    NBRE_Frustum3(const NBRE_Matrix4x4<Real>& matrix);
    NBRE_Frustum3(const NBRE_Plane<Real>& left,   const NBRE_Plane<Real>& right,
                  const NBRE_Plane<Real>& bottom, const NBRE_Plane<Real>& top,
                  const NBRE_Plane<Real>& zNear,  const NBRE_Plane<Real>& zFar);
    ~NBRE_Frustum3();

public:
    void UpdateFrom(const NBRE_Matrix4x4<Real>& matrix);
    void Update();
    const NBRE_Plane<Real>& GetNear() const;
    const NBRE_Plane<Real>& GetFar() const;
    const NBRE_Plane<Real>& GetBottom() const;
    const NBRE_Plane<Real>& GetTop() const;
    const NBRE_Plane<Real>& GetLeft() const;
    const NBRE_Plane<Real>& GetRight() const;
    const NBRE_AxisAlignedBox3<Real>& GetAABB() const;
    void GetCorners(NBRE_Vector3<Real>* resultPoints) const;

private:
    NBRE_Plane<Real> mLeft;
    NBRE_Plane<Real> mRight;
    NBRE_Plane<Real> mBottom;
    NBRE_Plane<Real> mTop;
    NBRE_Plane<Real> mNear;
    NBRE_Plane<Real> mFar;
    NBRE_Vector3<Real> corners[8];
    NBRE_AxisAlignedBox3<Real> mAABB;
};

#include "nbrefrustum3.inl"

typedef NBRE_Frustum3<float> NBRE_Frustum3f;
typedef NBRE_Frustum3<double> NBRE_Frustum3d;
/** @} */
#endif
