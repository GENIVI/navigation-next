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

    @file nbretypeconvert.h
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
#ifndef _NBRE_TYEP_CONVERT_H_
#define _NBRE_TYEP_CONVERT_H_

#include "paltypes.h"
#include "nbrevector3.h"
#include "nbreaxisalignedbox3.h"
#include "nbrematrix4x4.h"
#include "nbreray3.h"
#include "nbrepolyline2.h"
#include "nbreaxisalignedbox2.h"

/** \addtogroup NBRE_Core
*  @{
*/

//! Type convert class
/** 
*/
template <typename Real>
class NBRE_TypeConvert
{
public:
    template <typename OtherReal>
    static NBRE_Vector3<Real> Convert(const NBRE_Vector3<OtherReal>& vec)
    {
        return NBRE_Vector3<Real>( static_cast<Real>(vec.x), static_cast<Real>(vec.y), static_cast<Real>(vec.z) );
    }

    template <typename OtherReal>
    static NBRE_Vector2<Real> Convert(const NBRE_Vector2<OtherReal>& vec)
    {
        return NBRE_Vector2<Real>( static_cast<Real>(vec.x), static_cast<Real>(vec.y) );
    }

    template <typename OtherReal>
    static NBRE_AxisAlignedBox3<Real> Convert(const NBRE_AxisAlignedBox3<OtherReal>& box)
    {
        return NBRE_AxisAlignedBox3<Real>(Convert(box.minExtend), Convert(box.maxExtend));
    }

    template <typename OtherReal>
    static NBRE_AxisAlignedBox2<Real> Convert(const NBRE_AxisAlignedBox2<OtherReal>& box)
    {
        return NBRE_AxisAlignedBox2<Real>(Convert(box.minExtend), Convert(box.maxExtend));
    }

    template <typename OtherReal>
    static NBRE_Matrix4x4<Real> Convert(const NBRE_Matrix4x4<OtherReal>& m)
    {
        return NBRE_Matrix4x4<Real>(
            static_cast<Real>(m[0][0]), static_cast<Real>(m[0][1]), static_cast<Real>(m[0][2]), static_cast<Real>(m[0][3]), 
            static_cast<Real>(m[1][0]), static_cast<Real>(m[1][1]), static_cast<Real>(m[1][2]), static_cast<Real>(m[1][3]), 
            static_cast<Real>(m[2][0]), static_cast<Real>(m[2][1]), static_cast<Real>(m[2][2]), static_cast<Real>(m[2][3]), 
            static_cast<Real>(m[3][0]), static_cast<Real>(m[3][1]), static_cast<Real>(m[3][2]), static_cast<Real>(m[3][3]) 
            );
    }

    template <typename OtherReal>
    static NBRE_Ray3<Real> Convert(const NBRE_Ray3<OtherReal>& r)
    {
        return NBRE_Ray3<Real>(Convert(r.origin), Convert(r.direction));
    }

    template <typename OtherReal>
    static NBRE_Polyline2<Real> Convert(const NBRE_Polyline2<OtherReal>& pl)
    {
        NBRE_Vector<NBRE_Vector2<Real> > vs;
        for (uint32 i = 0; i < pl.VertexCount(); ++i)
        {
            const NBRE_Vector2<OtherReal>& v = pl.Vertex(i);
            vs.push_back(NBRE_Vector2<Real>(static_cast<Real>(v.x), static_cast<Real>(v.y)));
        }
        return NBRE_Polyline2<Real>(vs);
    }
};

typedef NBRE_TypeConvert<float> NBRE_TypeConvertf;
typedef NBRE_TypeConvert<double> NBRE_TypeConvertd;

/** @} */
#endif
