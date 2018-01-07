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

    @file nbrevector3.h
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
#ifndef _NBRE_VECTOR3_H_
#define _NBRE_VECTOR3_H_

#include "paltypes.h"
#include "nbrevector2.h"
/** \addtogroup NBRE_Core
*  @{
*/

//! 3D vector class
/** 
*/
template <typename Real>
class NBRE_Vector3
{
public:
    NBRE_Vector3();
    NBRE_Vector3(Real x, Real y, Real z);
    NBRE_Vector3(const Real* v);
    NBRE_Vector3(const NBRE_Vector3<Real>& vec);
    NBRE_Vector3(const NBRE_Vector2<Real>& vec);
    ~NBRE_Vector3();

public:
    NBRE_Vector3<Real>& operator = (const NBRE_Vector3<Real>& vec);
    nb_boolean operator == (const NBRE_Vector3<Real>& vec) const;
    nb_boolean operator != (const NBRE_Vector3<Real>& vec) const;

    NBRE_Vector3<Real> operator - () const;
    
    NBRE_Vector3<Real> operator + (const NBRE_Vector3<Real>& vec) const;
    NBRE_Vector3<Real> operator - (const NBRE_Vector3<Real>& vec) const;
    NBRE_Vector3<Real> operator * (Real scalar) const;
    NBRE_Vector3<Real> operator / (Real scalar) const;

    NBRE_Vector3<Real>& operator += (const NBRE_Vector3<Real>& vec);
    NBRE_Vector3<Real>& operator -= (const NBRE_Vector3<Real>& vec);
    NBRE_Vector3<Real>& operator *= (Real scalar);
    NBRE_Vector3<Real>& operator /= (Real scalar);

    Real DotProduct(const NBRE_Vector3& vec) const;
    NBRE_Vector3<Real> CrossProduct(const NBRE_Vector3<Real>& p) const;

    void Normalise();
    Real Length() const;
    Real SquaredLength() const;

    Real DistanceTo(const NBRE_Vector3<Real>& vec) const;
    Real SquaredDistanceTo(const NBRE_Vector3<Real>& vec) const;

    nb_boolean IsNaN() const;

public:
    Real x;
    Real y;
    Real z;
};

template <typename Real>
NBRE_Vector3<Real> operator* (Real scalar, const NBRE_Vector3<Real>& vec);

#include "nbrevector3.inl"

typedef NBRE_Vector3<float> NBRE_Vector3f;
typedef NBRE_Vector3<double> NBRE_Vector3d;

typedef NBRE_Vector3<float> NBRE_Point3f;
typedef NBRE_Vector3<double> NBRE_Point3d;

/** @} */
#endif
