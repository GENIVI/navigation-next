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
#include "nbrecollisiontest.h"
#include "nbreintersection.h"

void
NBRE_CollisionTest::GetIntervalOnAxis(const NBRE_Vector2d& axis, const NBRE_Vector2d* pts, uint32 ptCount, double& minValue, double& maxValue)
{
    minValue = maxValue = pts[0].DotProduct(axis);
    for (uint32 i = 1; i < ptCount; ++i)
    {
        double proj = pts[i].DotProduct(axis);
        if (proj < minValue)
        {
            minValue = proj;
        }
        else if (proj > maxValue)
        {
            maxValue = proj;
        }
    }
}

void
NBRE_CollisionTest::GetIntervalOnAxis(const NBRE_Vector2d& axis, const NBRE_Vector2d& center, double radius, double& minValue, double& maxValue)
{
    NBRE_Vector2d offset(axis);
    offset *= radius;
    minValue = axis.DotProduct(center - offset);
    maxValue = axis.DotProduct(center + offset);
}

nb_boolean
NBRE_CollisionTest::HitTest(NBRE_CollisionAABB2d* aabb, NBRE_CollisionPolygon2d* polygon)
{
    if (NBRE_Intersectiond::HitTest(polygon->GetAABB(), aabb->mAABB))
    {
        if (polygon->mAxis.size() == 0)
        {
            polygon->UpdateAxis();
        }
        nsl_assert(polygon->mAxis.size() > 0);

        for (uint32 i = 0; i < polygon->mAxis.size(); ++i)
        {
            const NBRE_Vector2d& axis = polygon->mAxis[i];
            const NBRE_Vector2d& interval = polygon->mAxisIntervals[i];
            double tMin;
            double tMax;
            GetIntervalOnAxis(axis, &aabb->mVertices[0], 4, tMin, tMax);
            if (tMin > interval.y || tMax < interval.x)
            {
                return FALSE;
            }
        }
        return TRUE;
    }
    return FALSE;
}

nb_boolean
NBRE_CollisionTest::HitTest(NBRE_CollisionPolygon2d* polygon1, NBRE_CollisionPolygon2d* polygon2)
{
    if (NBRE_Intersectiond::HitTest(polygon1->mAABB, polygon2->mAABB))
    {
        if (polygon1->mAxis.size() == 0)
        {
            polygon1->UpdateAxis();
        }
        if (polygon2->mAxis.size() == 0)
        {
            polygon2->UpdateAxis();
        }

        for (uint32 i = 0; i < polygon1->mAxis.size(); ++i)
        {
            const NBRE_Vector2d& axis = polygon1->mAxis[i];
            const NBRE_Vector2d& interval = polygon1->mAxisIntervals[i];
            double tMin;
            double tMax;
            GetIntervalOnAxis(axis, &polygon2->mVertices[0], polygon2->mVertices.size(), tMin, tMax);
            if (tMin > interval.y || tMax < interval.x)
            {
                return FALSE;
            }
        }
        for (uint32 i = 0; i < polygon2->mAxis.size(); ++i)
        {
            const NBRE_Vector2d& axis = polygon2->mAxis[i];
            const NBRE_Vector2d& interval = polygon2->mAxisIntervals[i];
            double tMin;
            double tMax;
            GetIntervalOnAxis(axis, &polygon1->mVertices[0], polygon1->mVertices.size(), tMin, tMax);
            if (tMin > interval.y || tMax < interval.x)
            {
                return FALSE;
            }
        }
        return TRUE;
    }
    return FALSE;
}

nb_boolean
NBRE_CollisionTest::HitTest(NBRE_CollisionAABB2d* aabb, NBRE_CollisionCircle2d* circle)
{
    double closestX = circle->mCenter.x;
    double closestY = circle->mCenter.y;
    const NBRE_AxisAlignedBox2d& box = aabb->GetAABB();
    if (closestX < box.minExtend.x)
    {
        closestX = box.minExtend.x;
    }
    else if (closestX > box.maxExtend.x)
    {
        closestX = box.maxExtend.x;
    }

    if (closestY < box.minExtend.y)
    {
        closestY = box.minExtend.y;
    }
    else if (closestY > box.maxExtend.y)
    {
        closestY = box.maxExtend.y;
    }
    double dx = closestX - circle->mCenter.x;
    double dy = closestY - circle->mCenter.y;
    return dx * dx + dy * dy <= circle->mRadius * circle->mRadius ? TRUE : FALSE;
}

nb_boolean
NBRE_CollisionTest::HitTest(NBRE_CollisionAABB2d* aabb, NBRE_CollisionLineSegment2d* lineSegment)
{
    if (NBRE_Intersectiond::HitTest(aabb->GetAABB(), lineSegment->GetAABB()))
    {
        if (lineSegment->mAxis.size() == 0)
        {
            lineSegment->UpdateAxis();
        }
        nsl_assert(lineSegment->mAxis.size() > 0);

        for (uint32 i = 0; i < lineSegment->mAxis.size(); ++i)
        {
            const NBRE_Vector2d& axis = lineSegment->mAxis[i];
            const NBRE_Vector2d& interval = lineSegment->mAxisIntervals[i];
            double tMin;
            double tMax;
            GetIntervalOnAxis(axis, &aabb->mVertices[0], 4, tMin, tMax);
            if (tMin > interval.y || tMax < interval.x)
            {
                return FALSE;
            }
        }
        return TRUE;
    }
    return FALSE;
}


nb_boolean
NBRE_CollisionTest::HitTest(NBRE_CollisionPolygon2d* polygon, NBRE_CollisionCircle2d* circle)
{
    if (NBRE_Intersectiond::HitTest(polygon->GetAABB(), circle->GetAABB()))
    {
        if (polygon->mAxis.size() == 0)
        {
            polygon->UpdateAxis();
        }
        nsl_assert(polygon->mAxis.size() > 0);

        for (uint32 i = 0; i < polygon->mAxis.size(); ++i)
        {
            const NBRE_Vector2d& axis = polygon->mAxis[i];
            const NBRE_Vector2d& interval = polygon->mAxisIntervals[i];
            double tMin;
            double tMax;
            GetIntervalOnAxis(axis, circle->mCenter, circle->mRadius, tMin, tMax);
            if (tMin > interval.y || tMax < interval.x)
            {
                return FALSE;
            }
        }

        uint32 vs = polygon->mVertices.size();
        if (vs > 0)
        {
            double minDistSq = polygon->mVertices.front().SquaredDistanceTo(circle->mCenter);
            uint32 minIdx = 0;
            for (uint32 i = 1; i < vs; ++i)
            {
                double distSq = polygon->mVertices[i].SquaredDistanceTo(circle->mCenter);
                if (distSq < minDistSq)
                {
                    minDistSq = distSq;
                    minIdx = i;
                }
            }

            // Project polygon & circle on "closest point to circle center" axis
            const NBRE_Vector2d& axis = polygon->mVertices[minIdx] - circle->mCenter;
            NBRE_Vector2d interval;
            GetIntervalOnAxis(axis, &polygon->mVertices[0], vs, interval.x, interval.y);
            double tMin;
            double tMax;
            GetIntervalOnAxis(axis, circle->mCenter, circle->mRadius, tMin, tMax);
            if (tMin > interval.y || tMax < interval.x)
            {
                return FALSE;
            }
        }
        
        return TRUE;
    }
    return FALSE;
}

nb_boolean
NBRE_CollisionTest::HitTest(NBRE_CollisionPolygon2d* polygon, NBRE_CollisionLineSegment2d* lineSegment)
{
    if (NBRE_Intersectiond::HitTest(polygon->GetAABB(), lineSegment->GetAABB()))
    {
        if (polygon->mAxis.size() == 0)
        {
            polygon->UpdateAxis();
        }
        nsl_assert(polygon->mAxis.size() > 0);

        for (uint32 i = 0; i < polygon->mAxis.size(); ++i)
        {
            const NBRE_Vector2d& axis = polygon->mAxis[i];
            const NBRE_Vector2d& interval = polygon->mAxisIntervals[i];
            double tMin;
            double tMax;
            GetIntervalOnAxis(axis, lineSegment->mEndPoints, 2, tMin, tMax);
            if (tMin > interval.y || tMax < interval.x)
            {
                return FALSE;
            }
        }

        if (lineSegment->mLength > 0)
        {
            if (lineSegment->mAxis.size() == 0)
            {
                lineSegment->UpdateAxis();
            }
            nsl_assert(lineSegment->mAxis.size() > 0);

            for (uint32 i = 0; i < lineSegment->mAxis.size(); ++i)
            {
                const NBRE_Vector2d& axis = lineSegment->mAxis[i];
                const NBRE_Vector2d& interval = lineSegment->mAxisIntervals[i];
                double tMin;
                double tMax;
                GetIntervalOnAxis(axis, &polygon->mVertices[0], polygon->mVertices.size(), tMin, tMax);
                if (tMin > interval.y || tMax < interval.x)
                {
                    return FALSE;
                }
            }
        }

        return TRUE;
    }
    return FALSE;
}

nb_boolean
NBRE_CollisionTest::HitTest(NBRE_CollisionCircle2d* circle, NBRE_CollisionCircle2d* circle2)
{
    double dx = circle2->mCenter.x - circle->mCenter.x;
    double dy = circle2->mCenter.y - circle->mCenter.y;
    double dr = circle2->mRadius + circle->mRadius;
    return dx * dx + dy * dy <= dr * dr;
}

nb_boolean
NBRE_CollisionTest::HitTest(NBRE_CollisionCircle2d* circle, NBRE_CollisionLineSegment2d* lineSegment)
{
    if (NBRE_Intersectiond::HitTest(circle->GetAABB(), lineSegment->GetAABB()))
    {
        double distSq = lineSegment->GetSquaredDistanceTo(circle->mCenter);
        return distSq <= circle->mRadius * circle->mRadius ? TRUE : FALSE;
    }
    return FALSE;
}

nb_boolean
NBRE_CollisionTest::FindSeperationPlane(NBRE_CollisionLineSegment2d* lineSegment, const NBRE_Vector2d* vertices, uint32 vertexCount)
{
    if (lineSegment->mAxis.size() == 0)
    {
        lineSegment->UpdateAxis();
    }
    nsl_assert(lineSegment->mAxis.size() > 0);

    for (uint32 i = 0; i < lineSegment->mAxis.size(); ++i)
    {
        const NBRE_Vector2d& axis = lineSegment->mAxis[i];
        const NBRE_Vector2d& interval = lineSegment->mAxisIntervals[i];
        double tMin;
        double tMax;
        GetIntervalOnAxis(axis, vertices, vertexCount, tMin, tMax);
        if (tMin > interval.y || tMax < interval.x)
        {
            return TRUE;
        }
    }
    return FALSE;
}

nb_boolean
NBRE_CollisionTest::HitTest(NBRE_CollisionLineSegment2d* lineSegment, NBRE_CollisionLineSegment2d* lineSegment2)
{
    if (lineSegment->mLength == 0)//is line a point?
    {
        if (lineSegment2->mLength == 0)//is line a point?
        {
            return lineSegment->mEndPoints[0] == lineSegment2->mEndPoints[0] ? TRUE : FALSE;
        }
        else
        {
            return FindSeperationPlane(lineSegment2, &lineSegment->mEndPoints[0], 1) ? FALSE : TRUE;
        }
    }
    else
    {
        if (lineSegment2->mLength == 0)//is line a point?
        {
            return FindSeperationPlane(lineSegment, &lineSegment2->mEndPoints[0], 1) ? FALSE : TRUE;
        }
        else
        {
            return FindSeperationPlane(lineSegment, &lineSegment2->mEndPoints[0], 2)
                || FindSeperationPlane(lineSegment2, &lineSegment->mEndPoints[0], 2) ? FALSE : TRUE;
        }
    }
}
