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

    @file nbreobjectbox3.h
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

#ifndef _NBRE_OBJECT_BOX3_H_
#define _NBRE_OBJECT_BOX3_H_

#include "paltypes.h"
#include "nbrevector3.h"
#include "nbremath.h"

/** \addtogroup NBRE_Core
*  @{
*/
//! Axis aligned bounding box 3D
/** 
*/
template <typename Real>
class NBRE_NoneAxisAlignedBox3
{
public:
    NBRE_NoneAxisAlignedBox3(){};
    ~NBRE_NoneAxisAlignedBox3(){};

public:
    void GetCorners(NBRE_Vector3<Real>* resultPoints) const
    {
        resultPoints[0] = center - up - forward - right;
        resultPoints[1] = center - up - forward + right;
        resultPoints[2] = center - up + forward + right;
        resultPoints[3] = center - up + forward - right;
        resultPoints[4] = center + up - forward - right;
        resultPoints[5] = center + up - forward + right;
        resultPoints[6] = center + up + forward + right;
        resultPoints[7] = center + up + forward - right;
    }

    nb_boolean Contain(NBRE_Vector3<Real> pt) const
    {
        pt -= center;
        return (up.DotProduct(up)>=NBRE_Math::Abs(pt.DotProduct(up)))&&
               (forward.DotProduct(forward)>=NBRE_Math::Abs(pt.DotProduct(forward)))&&
               (right.DotProduct(right)>=NBRE_Math::Abs(pt.DotProduct(right)));
    }

public:
    NBRE_Vector3<Real> center;
    NBRE_Vector3<Real> up;
    NBRE_Vector3<Real> forward;
    NBRE_Vector3<Real> right;
};


typedef NBRE_NoneAxisAlignedBox3<float> NBRE_NoneAxisAlignedBox3f;
typedef NBRE_NoneAxisAlignedBox3<double> NBRE_NoneAxisAlignedBox3d;
/** @} */

#endif
