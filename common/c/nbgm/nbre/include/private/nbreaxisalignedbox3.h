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

    @file nbreaxisalignedbox3.h
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

#ifndef _NBRE_AXIS_ALIGNED_BOX3_H_
#define _NBRE_AXIS_ALIGNED_BOX3_H_

#include "paltypes.h"
#include "nbrevector3.h"

/** \addtogroup NBRE_Core
*  @{
*/
//! Axis aligned bounding box 3D
/** 
*/
template <typename Real>
class NBRE_AxisAlignedBox3
{
public:
    NBRE_AxisAlignedBox3();
    NBRE_AxisAlignedBox3(Real minX, Real minY, Real minZ, Real maxX, Real maxY, Real maxZ);
    NBRE_AxisAlignedBox3(const NBRE_Vector3<Real>& minExtend, const NBRE_Vector3<Real>& maxExtend);
    NBRE_AxisAlignedBox3(const NBRE_AxisAlignedBox3<Real>& box);
    ~NBRE_AxisAlignedBox3();

public:
    void Merge(Real x, Real y, Real z);
    void Merge(const NBRE_Vector3<Real>& point);
    void Merge(const NBRE_AxisAlignedBox3<Real>& box);
    void Set(const NBRE_Vector3<Real>* points, uint32 pointsCount);
    NBRE_Vector3<Real> GetSize() const;
    NBRE_Vector3<Real> GetCenter() const;
    NBRE_Vector3<Real> GetClosestPointTo(const NBRE_Vector3<Real>& point) const;
    void GetCorners(NBRE_Vector3<Real>* resultPoints) const;
    nb_boolean IsNull() const;
    void SetNull();
    nb_boolean Contain(const NBRE_AxisAlignedBox3& box) const;
    Real DistanceTo(const NBRE_AxisAlignedBox3& box) const;

    NBRE_AxisAlignedBox3& operator = (const NBRE_AxisAlignedBox3<Real>& box);
public:
    NBRE_Vector3<Real> minExtend;
    NBRE_Vector3<Real> maxExtend;
};

#include "nbreaxisalignedbox3.inl"
#include "nbreaxisalignedbox3i.inl"

typedef NBRE_AxisAlignedBox3<float> NBRE_AxisAlignedBox3f;
typedef NBRE_AxisAlignedBox3<double> NBRE_AxisAlignedBox3d;
/** @} */

#endif
