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

/*--------------------------------------------------------------------------

(C) Copyright 2012 by Networks In Motion, Inc.                

The information contained herein is confidential, proprietary 
to Networks In Motion, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of Networks In Motion is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/
#include "nbrecollisionlinesegment2d.h"
#include "nbreintersection.h"
#include "nbrecollisionaabb2d.h"
#include "nbrecollisiontest.h"
#include "nbrelinearinterpolate.h"

NBRE_CollisionLineSegment2d::NBRE_CollisionLineSegment2d(const NBRE_Vector2d& p0, const NBRE_Vector2d& p1)
{
    mEndPoints[0] = p0;
    mEndPoints[1] = p1;
    mAABB.Merge(p0);
    mAABB.Merge(p1);
    mDir = p1 - p0;
    mLength = mDir.Length();
    mDir.Normalise();
}

NBRE_CollisionLineSegment2d::~NBRE_CollisionLineSegment2d()
{
}

nb_boolean
NBRE_CollisionLineSegment2d::HitTest(NBRE_ICollisionPrimitive2d* obj)
{
    nb_boolean result = FALSE;
    NBRE_CollisionPrimitiveType type = obj->GetType();
    
    switch (type)
    {
    case NBRE_CPT_AABB2:
        {
            NBRE_CollisionAABB2d* aabbObj = (NBRE_CollisionAABB2d*)obj;
            result = NBRE_CollisionTest::HitTest(aabbObj, this);
        }
        break;
    case NBRE_CPT_POLYGON2:
        {
            NBRE_CollisionPolygon2d* polygonObj = (NBRE_CollisionPolygon2d*)obj;
            result = NBRE_CollisionTest::HitTest(polygonObj, this);
        }
        break;
    case NBRE_CPT_LINESEGMENT2:
        {
            NBRE_CollisionLineSegment2d* segmentObj = (NBRE_CollisionLineSegment2d*)obj;
            result = NBRE_CollisionTest::HitTest(this, segmentObj);
        }
        break;
    case NBRE_CPT_CIRCLE2:
        {
            NBRE_CollisionCircle2d* circleObj = (NBRE_CollisionCircle2d*)obj;
            result = NBRE_CollisionTest::HitTest(circleObj, this);
        }
        break;
    default:
        NBRE_DebugLog(PAL_LogSeverityDebug, "NBRE_CollisionLineSegment2d::HitTest, unsupported type %d", type);
    }

    return result;
}

double
NBRE_CollisionLineSegment2d::GetSquaredDistanceTo(const NBRE_Vector2d& p) const
{
    if (mLength == 0)
    {
        return p.SquaredDistanceTo(mEndPoints[0]);
    }

    double t = mDir.DotProduct(p - mEndPoints[0]) / mLength;
    if (t < 0)
    {
        return mEndPoints[0].SquaredDistanceTo(p);
    }
    else if (t > 1.0)
    {
        return mEndPoints[1].SquaredDistanceTo(p);
    }
    else
    {
        return p.SquaredDistanceTo(NBRE_LinearInterpolated::Lerp(mEndPoints[0], mEndPoints[1], t));
    }
}

void
NBRE_CollisionLineSegment2d::UpdateAxis()
{
    mAxis.clear();
    mAxisIntervals.clear();

    NBRE_Vector2d interval;
    NBRE_CollisionTest::GetIntervalOnAxis(mDir, mEndPoints, 2, interval.x, interval.y);
    mAxis.push_back(mDir);
    mAxisIntervals.push_back(interval);

    const NBRE_Vector2d& perp = mDir.Perpendicular();
    NBRE_CollisionTest::GetIntervalOnAxis(perp, mEndPoints, 1, interval.x, interval.y);
    mAxis.push_back(perp);
    mAxisIntervals.push_back(interval);
}
