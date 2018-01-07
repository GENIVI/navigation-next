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

    @file nbresegment3.h
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

#ifndef _NBRE_SEGMENT3_H_
#define _NBRE_SEGMENT3_H_

#include "paltypes.h"
#include "nbrevector3.h"
/** \addtogroup NBRE_Core
*  @{
*/

//! 3D segment class
/** defined with start and end point
*/
template <typename Real>
class NBRE_Segment3
{
public:
    NBRE_Segment3();
    NBRE_Segment3(Real xStart, Real yStart, Real zStart, Real xEnd, Real yEnd, Real zEnd);
    NBRE_Segment3(const NBRE_Vector3<Real>& start, const NBRE_Vector3<Real>& end);
    ~NBRE_Segment3();

public:
    Real Length() const;
    Real SquaredLength() const;
    NBRE_Vector3<Real> GetDirection() const;
    void Reverse();
    nb_boolean IsParallel(const NBRE_Segment3<Real>& line, Real epsilon) const;
    nb_boolean IsPerpendicular(const NBRE_Segment3<Real>& line, Real epsilon) const;
    NBRE_Vector3<Real> GetClosestPointTo(const NBRE_Vector3<Real>& point) const;
    Real DistanceTo(const NBRE_Vector3<Real>& point) const;
    Real GetClosestT(const NBRE_Vector3<Real>& point) const;

public:
    NBRE_Vector3<Real> startPoint;
    NBRE_Vector3<Real> endPoint;
};

#include "nbresegment3.inl"

typedef NBRE_Segment3<float> NBRE_Segment3f;
typedef NBRE_Segment3<double> NBRE_Segment3d;
/** @} */
#endif
