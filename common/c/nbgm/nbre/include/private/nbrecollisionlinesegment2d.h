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

    @file nbrecollisionlinesegment2d.h
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
#ifndef _NBRE_COLLISION_LINE_SEGMENT_2D_H_
#define _NBRE_COLLISION_LINE_SEGMENT_2D_H_
#include "nbretypes.h"
#include "nbreicollisionprimitive.h"
#include "nbreaxisalignedbox2.h"

/*! \addtogroup NBRE_Scene
*  @{
*/
/** Line segment collision primitive.
 */
class NBRE_CollisionLineSegment2d:
    public NBRE_ICollisionPrimitive2d
{
    friend class NBRE_CollisionTest;
public:
    NBRE_CollisionLineSegment2d(const NBRE_Vector2d& p0, const NBRE_Vector2d& p1);
    virtual ~NBRE_CollisionLineSegment2d();
    virtual NBRE_CollisionPrimitiveType GetType() { return NBRE_CPT_LINESEGMENT2; }
    virtual nb_boolean HitTest(NBRE_ICollisionPrimitive2d* obj);
    virtual const NBRE_AxisAlignedBox2d& GetAABB() { return mAABB; }
    const NBRE_Vector2d* GetEndPoints() const { return mEndPoints; }
    double GetSquaredDistanceTo(const NBRE_Vector2d& p) const;

private:
    void UpdateAxis();

private:
    NBRE_Vector2d mEndPoints[2];
    NBRE_Vector2d mDir;
    double mLength;
    NBRE_AxisAlignedBox2d mAABB;
    NBRE_Vector<NBRE_Vector2d> mAxis;
    NBRE_Vector<NBRE_Vector2d> mAxisIntervals;
};

/*! @} */
#endif
