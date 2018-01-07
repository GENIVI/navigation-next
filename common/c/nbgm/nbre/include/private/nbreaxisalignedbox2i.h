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

    @file nbreaxisalignedbox2i.h
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

#ifndef _NBRE_AXIS_ALIGNED_BOX2I_H_
#define _NBRE_AXIS_ALIGNED_BOX2I_H_

#include "paltypes.h"
#include "nbrevector2.h"
#include "nbremath.h"

/** \addtogroup NBRE_Core
*  @{
*/
//! Axis aligned bounding box 2D
/** 
*/
template <>
class NBRE_AxisAlignedBox2<int32>
{
public:
    NBRE_AxisAlignedBox2();
    NBRE_AxisAlignedBox2(int32 minX, int32 minY, int32 maxX, int32 maxY);
    NBRE_AxisAlignedBox2(const NBRE_Vector2<int32>& minExtend, const NBRE_Vector2<int32>& maxExtend);
    NBRE_AxisAlignedBox2(const NBRE_AxisAlignedBox2<int32>& box);
    NBRE_AxisAlignedBox2& operator = (const NBRE_AxisAlignedBox2& box);
    ~NBRE_AxisAlignedBox2();

public:
    void Merge(int32 x, int32 y);
    void Merge(const NBRE_Vector2<int32>& point);
    void Merge(const NBRE_AxisAlignedBox2<int32>& box);
    NBRE_Vector2<int32> GetSize() const;
    NBRE_Vector2<int32> GetCenter() const;
    NBRE_Vector2<int32> GetClosestPointTo(const NBRE_Vector2<int32>& point) const;
    void GetCorners(NBRE_Vector2<int32>* resultPoints) const;
    nb_boolean IsNull() const;
    void SetNull();

public:
    NBRE_Vector2<int32> minExtend;
    NBRE_Vector2<int32> maxExtend;
};

#include "nbreaxisalignedbox2i.inl"

typedef NBRE_AxisAlignedBox2<int32> NBRE_AxisAlignedBox2i;
/** @} */

#endif
