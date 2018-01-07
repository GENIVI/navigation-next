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

    @file nbretriangle3.h
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

#ifndef _NBRE_TRIANGLE3_H_
#define _NBRE_TRIANGLE3_H_

#include "paltypes.h"
#include "nbreplane.h"
/** \addtogroup NBRE_Core
*  @{
*/

//! 3D triangle class
/** defined with start and end point
*/
template <typename Real>
class NBRE_Triangle3
{
public:
    NBRE_Triangle3();
    NBRE_Triangle3(const NBRE_Vector3<Real>* vertices);
    NBRE_Triangle3(const NBRE_Vector3<Real>& p0, const NBRE_Vector3<Real>& p1, const NBRE_Vector3<Real>& p2);
    ~NBRE_Triangle3();

public:
    NBRE_Vector3<Real> GetPlane() const;
    NBRE_Vector3<Real> GetNormal() const;
public:
    NBRE_Vector3<Real> vertices[3];
};

template <typename Real>
inline NBRE_Triangle3<Real>::NBRE_Triangle3()
{
}

template <typename Real>
inline NBRE_Triangle3<Real>::NBRE_Triangle3(const NBRE_Vector3<Real>* vertices)
{
    nsl_memcpy(this->vertices, vertices, sizeof(NBRE_Vector3<Real>) * 3);
}

template <typename Real>
inline NBRE_Triangle3<Real>::NBRE_Triangle3(const NBRE_Vector3<Real>& p0, const NBRE_Vector3<Real>& p1, const NBRE_Vector3<Real>& p2)
{
    vertices[0] = p0;
    vertices[1] = p1; 
    vertices[2] = p2;
}

template <typename Real>
inline NBRE_Triangle3<Real>::~NBRE_Triangle3()
{
}

template <typename Real>
inline NBRE_Vector3<Real> 
NBRE_Triangle3<Real>::GetPlane() const
{
    return NBRE_Plane<Real>(vertices[0], vertices[1], vertices[2]);
}

template <typename Real>
inline NBRE_Vector3<Real> 
NBRE_Triangle3<Real>::GetNormal() const
{
    NBRE_Plane<Real> plane(vertices[0], vertices[1], vertices[2]);
    return plane.normal;
}

typedef NBRE_Triangle3<float> NBRE_Triangle3f;
typedef NBRE_Triangle3<double> NBRE_Triangle3d;
/** @} */
#endif
