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

    @file nbresegment2.h
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

#ifndef _NBRE_SEGMENT2_H_
#define _NBRE_SEGMENT2_H_

#include "paltypes.h"
#include "nbrevector2.h"
/** \addtogroup NBRE_Core
*  @{
*/

//! 2D line segment class
/** defined with start and end point
*/
template <typename Real>
class NBRE_Segment2
{
public:
    NBRE_Segment2();
    NBRE_Segment2(Real xStart, Real yStart, Real xEnd, Real yEnd);
    NBRE_Segment2(const NBRE_Vector2<Real>& start, const NBRE_Vector2<Real>& end);
    ~NBRE_Segment2();

public:
    Real Length() const;
    Real SquaredLength() const;
    NBRE_Vector2<Real> GetDirection() const;
    void Reverse();
    nb_boolean IsParallel(const NBRE_Segment2<Real>& line, Real epsilon) const;
    nb_boolean IsPerpendicular(const NBRE_Segment2<Real>& line, Real epsilon) const;
    NBRE_Vector2<Real> GetClosestPointTo(const NBRE_Vector2<Real>& point) const;
    Real DistanceTo(const NBRE_Vector2<Real>& point) const;
    Real GetClosestT(const NBRE_Vector2<Real>& point) const;

public:
    NBRE_Vector2<Real> startPoint;
    NBRE_Vector2<Real> endPoint;
};

#include "nbresegment2.inl"

typedef NBRE_Segment2<float> NBRE_Segment2f;
typedef NBRE_Segment2<double> NBRE_Segment2d;
/** @} */
#endif
