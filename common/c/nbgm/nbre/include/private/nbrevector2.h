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

    @file nbrevector2.h
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

#ifndef _NBRE_VECTOR2_H_
#define _NBRE_VECTOR2_H_

#include "paltypes.h"
/** \addtogroup NBRE_Core
*  @{
*/

//! 2D vector class
/** 
*/
template <typename Real>
class NBRE_Vector2
{
public:
    NBRE_Vector2();
    NBRE_Vector2(Real x, Real y);
    NBRE_Vector2(const Real* v);
    NBRE_Vector2(const NBRE_Vector2<Real>& vec);
    ~NBRE_Vector2();

public:
    NBRE_Vector2<Real>& operator = (const NBRE_Vector2<Real>& vec);
    nb_boolean operator == (const NBRE_Vector2<Real>& vec) const;
    nb_boolean operator != (const NBRE_Vector2<Real>& vec) const;

    NBRE_Vector2<Real> operator - () const;
    NBRE_Vector2<Real> operator + (const NBRE_Vector2<Real>& vec) const;
    NBRE_Vector2<Real> operator - (const NBRE_Vector2<Real>& vec ) const;
    NBRE_Vector2<Real> operator * (Real scalar) const;
    NBRE_Vector2<Real> operator / (Real scalar) const;

    NBRE_Vector2<Real>& operator += (const NBRE_Vector2<Real>& vec);
    NBRE_Vector2<Real>& operator -= (const NBRE_Vector2<Real>& vec);    
    NBRE_Vector2<Real>& operator *= (Real scalar);
    NBRE_Vector2<Real>& operator /= (Real scalar);

    Real Length() const;
    Real SquaredLength() const;
    void Normalise();
    Real DotProduct(const NBRE_Vector2<Real>& vec) const;
    Real CrossProduct(const NBRE_Vector2<Real>& p) const;
    NBRE_Vector2<Real> Perpendicular() const;
    Real DistanceTo(const NBRE_Vector2<Real>& vec) const;
    Real SquaredDistanceTo(const NBRE_Vector2<Real>& vec) const;
    nb_boolean IsNaN() const;
    Real GetDirection() const;

public:
    Real x;
    Real y;
};

template <typename Real>
NBRE_Vector2<Real> operator* (Real scalar, const NBRE_Vector2<Real>& vec);

#include "nbrevector2.inl"

typedef NBRE_Vector2<float> NBRE_Vector2f;
typedef NBRE_Vector2<double> NBRE_Vector2d;
typedef NBRE_Vector2<int32> NBRE_Vector2i;
typedef NBRE_Vector2<uint32> NBRE_Vector2u;

typedef NBRE_Vector2<float> NBRE_Point2f;
typedef NBRE_Vector2<double> NBRE_Point2d;
typedef NBRE_Vector2<int32> NBRE_Point2i;
typedef NBRE_Vector2<uint32> NBRE_Point2u;

typedef NBRE_Vector<NBRE_Point2f> Polygon2f;

template <typename Real>
nb_boolean NBRE_IsClockwise(const NBRE_Vector<NBRE_Vector2<Real> >& polyline)
{
    int32 length = polyline.size();
    if(length < 3)
    {
        return FALSE;
    }

    NBRE_Vector2<Real> pt1 = polyline[0];
    NBRE_Vector2<Real> pt2 = polyline[1];
    Real size = pt1.CrossProduct(pt2);
    for(int i = 1; i < length; ++i)
    {
        pt1 = polyline[i];
        if((i+1) < length)
        {
            pt2 = polyline[i+1];
        }
        else
        {
            pt2 = polyline[0];
        }
        size += pt1.CrossProduct(pt2);
    }
    return size > 0;
}

/** @} */
#endif
