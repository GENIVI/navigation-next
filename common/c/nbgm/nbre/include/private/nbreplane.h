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

    @file nbreplane.h
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

#ifndef _NBRE_PLANE_H_
#define _NBRE_PLANE_H_

#include "paltypes.h"
#include "nbrevector3.h"
/** \addtogroup NBRE_Core
*  @{
*/

enum NBRE_PlaneSide
{
    NBRE_PS_FRONT_SIDE,
    NBRE_PS_BACK_SIDE,
    NBRE_PS_SPANNING
};

//! 3D plane class
/** Equation: normal * pointOnPlane + d = 0
*/
template <typename Real>
class NBRE_Plane
{
public:
    NBRE_Plane();
    NBRE_Plane(const NBRE_Vector3<Real>& normal, Real d);
    NBRE_Plane(const NBRE_Vector3<Real>& normal, const NBRE_Vector3<Real>& p);
    NBRE_Plane(const NBRE_Vector3<Real>& p0, const NBRE_Vector3<Real>& p1, const NBRE_Vector3<Real>& p2);
    ~NBRE_Plane();

public:
    nb_boolean operator == (const NBRE_Plane& rhs) const;
    nb_boolean operator != (const NBRE_Plane& rhs) const;

    Real DistanceTo (const NBRE_Vector3<Real>& p) const;
    NBRE_Vector3<Real> GetClosestPointTo(const NBRE_Vector3<Real>& p) const;
    NBRE_PlaneSide GetSide (const NBRE_Vector3<Real>& p) const;
    void Normalize();

public:
    NBRE_Vector3<Real> normal;
    Real d;
};

#include "nbreplane.inl"

typedef NBRE_Plane<float> NBRE_Planef;
typedef NBRE_Plane<double> NBRE_Planed;
/** @} */
#endif
