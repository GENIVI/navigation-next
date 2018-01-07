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

    @file nbreaxisalignedbox2.h
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

#ifndef _NBRE_AXIS_ALIGNED_BOX2_H_
#define _NBRE_AXIS_ALIGNED_BOX2_H_

#include "paltypes.h"
#include "nbrevector2.h"
#include "nbremath.h"

/** \addtogroup NBRE_Core
*  @{
*/
//! Axis aligned bounding box 2D
/** 
*/
template <typename Real>
class NBRE_AxisAlignedBox2
{
public:
    NBRE_AxisAlignedBox2();
    NBRE_AxisAlignedBox2(Real minX, Real minY, Real maxX, Real maxY);
    NBRE_AxisAlignedBox2(const NBRE_Vector2<Real>& minExtend, const NBRE_Vector2<Real>& maxExtend);
    NBRE_AxisAlignedBox2(const NBRE_AxisAlignedBox2<Real>& box);
    NBRE_AxisAlignedBox2& operator = (const NBRE_AxisAlignedBox2& box);
    ~NBRE_AxisAlignedBox2();

public:
    void Merge(Real x, Real y);
    void Merge(const NBRE_Vector2<Real>& point);
    void Merge(const NBRE_AxisAlignedBox2<Real>& box);
    NBRE_Vector2<Real> GetSize() const;
    NBRE_Vector2<Real> GetCenter() const;
    NBRE_Vector2<Real> GetClosestPointTo(const NBRE_Vector2<Real>& point) const;
    void GetCorners(NBRE_Vector2<Real>* resultPoints) const;
    nb_boolean IsNull() const;
    void SetNull();

public:
    NBRE_Vector2<Real> minExtend;
    NBRE_Vector2<Real> maxExtend;
};

#include "nbreaxisalignedbox2.inl"

typedef NBRE_AxisAlignedBox2<float> NBRE_AxisAlignedBox2f;
typedef NBRE_AxisAlignedBox2<double> NBRE_AxisAlignedBox2d;
#include "nbreaxisalignedbox2i.h"
/** @} */

#endif
