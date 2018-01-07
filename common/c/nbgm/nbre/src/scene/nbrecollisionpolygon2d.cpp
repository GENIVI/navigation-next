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
#include "nbrecollisionpolygon2d.h"
#include "nbreintersection.h"
#include "nbrecollisionaabb2d.h"
#include "nbrecollisiontest.h"

static const double COS_ANGLE_UPPER_LIMIT = 0.99999999;
static const double COS_ANGLE_LOWER_LIMIT = -0.99999999;

static nb_boolean
IsSameAxis(const NBRE_Vector2d& axis1, const NBRE_Vector2d& axis2)
{
    double cosAngle = axis1.x * axis2.x + axis1.y * axis2.y;
    return cosAngle > COS_ANGLE_UPPER_LIMIT || cosAngle < COS_ANGLE_LOWER_LIMIT ? TRUE : FALSE;
}

NBRE_CollisionPolygon2d::NBRE_CollisionPolygon2d(const NBRE_Vector2d* vertices, uint32 vertexCount)
{
    mVertices.push_back(vertices[0]);
    mAABB.Merge(vertices[0]);
    for (uint32 i = 1; i < vertexCount; ++i)
    {
        const NBRE_Vector2d& v = vertices[i];
        if (v != mVertices.back())
        {
            mVertices.push_back(v);
            mAABB.Merge(v);
        }
    }
}

NBRE_CollisionPolygon2d::~NBRE_CollisionPolygon2d()
{
}

nb_boolean
NBRE_CollisionPolygon2d::HitTest(NBRE_ICollisionPrimitive2d* obj)
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
            result = NBRE_CollisionTest::HitTest(this, polygonObj);
        }
        break;
    case NBRE_CPT_LINESEGMENT2:
        {
            NBRE_CollisionCircle2d* segmentObj = (NBRE_CollisionCircle2d*)obj;
            result = NBRE_CollisionTest::HitTest(this, segmentObj);
        }
        break;
    case NBRE_CPT_CIRCLE2:
        {
            NBRE_CollisionCircle2d* circleObj = (NBRE_CollisionCircle2d*)obj;
            result = NBRE_CollisionTest::HitTest(this, circleObj);
        }
        break;
    default:
        NBRE_DebugLog(PAL_LogSeverityDebug, "NBRE_CollisionPolygon2d::HitTest, unsupported type %d", type);
    }

    return result;
}

void
NBRE_CollisionPolygon2d::UpdateAxis()
{
    mAxis.clear();
    mAxisIntervals.clear();
    uint32 n = mVertices.size();
    for (uint32 i = 0; i < n; ++i)
    {
        const NBRE_Vector2d& edge = mVertices[i] - mVertices[(i + 1) % n];
        NBRE_Vector2d axis(edge.Perpendicular());
        axis.Normalise();
        nb_boolean isDuplicatedAxis = FALSE;
        for (uint32 j = 0; j < mAxis.size(); ++j)
        {
            if (IsSameAxis(axis, mAxis[j]))
            {
                isDuplicatedAxis = TRUE;
                break;
            }
        }
        if (!isDuplicatedAxis)
        {
            NBRE_Vector2d interval;
            NBRE_CollisionTest::GetIntervalOnAxis(axis, &mVertices[0], mVertices.size(), interval.x, interval.y);
            mAxis.push_back(axis);
            mAxisIntervals.push_back(interval);
        }
    }
}
