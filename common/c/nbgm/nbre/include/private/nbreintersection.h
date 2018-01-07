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

    @file nbreintersection.h
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

#ifndef _NBRE_INTERSECTION_H_
#define _NBRE_INTERSECTION_H_

#include "paltypes.h"
#include "palmath.h"
#include "nbrevector2.h"
#include "nbrevector3.h"
#include "nbreaxisalignedbox2.h"
#include "nbreaxisalignedbox3.h"
#include "nbreray2.h"
#include "nbreray3.h"
#include "nbretriangle2.h"
#include "nbretriangle3.h"
#include "nbrefrustum3.h"
#include "nbresegment2.h"
#include "nbresegment3.h"
#include "nbresphere3.h"
#include "nbrenoneaxisalignedbox3.h"

/** \addtogroup NBRE_Core
*  @{
*/

//! Geometry intersection algorithms
/** 
*/
template <typename Real>
class NBRE_Intersection
{
public:
    // Point-AABB
    static nb_boolean HitTest(const NBRE_AxisAlignedBox2<Real>& box, const NBRE_Vector2<Real>& point);
    static nb_boolean HitTest(const NBRE_AxisAlignedBox3<Real>& box, const NBRE_Vector3<Real>& point);
    // AABB-AABB
    static nb_boolean HitTest(const NBRE_AxisAlignedBox2<Real>& box1, const NBRE_AxisAlignedBox2<Real>& box2);
    static nb_boolean HitTest(const NBRE_AxisAlignedBox3<Real>& box1, const NBRE_AxisAlignedBox3<Real>& box2);
    static nb_boolean HitTest(const NBRE_NoneAxisAlignedBox3<Real>& box1, const NBRE_NoneAxisAlignedBox3<Real>& box2);

    // AABB-AABB CCD, for fast moving objects.
    //! Moving AABB collision test.
    /** If both box moving, select one box as static one,
        calculate relative offset of another box
    @param staticBox non-moving AABB
    @param movingBox moving AABB
    @param moveOffset offset of moving AABB in period
    @param pCollideT if not NULL, output collision time during period, its range is [0..1]
    @return nb_boolean TRUE if collision detected, FALSE if no collision
    */
    static nb_boolean HitTest(const NBRE_AxisAlignedBox2<Real>& staticBox, const NBRE_AxisAlignedBox2<Real>& movingBox, const NBRE_Vector2<Real>& moveOffset, Real* pCollideT);
    static nb_boolean HitTest(const NBRE_AxisAlignedBox2<Real>& staticBox, const NBRE_AxisAlignedBox2<Real>& movingBox, const NBRE_Vector2<Real>& moveOffset);
    static nb_boolean HitTest(const NBRE_AxisAlignedBox3<Real>& staticBox, const NBRE_AxisAlignedBox3<Real>& movingBox, const NBRE_Vector3<Real>& moveOffset, Real* pCollideT);
    static nb_boolean HitTest(const NBRE_AxisAlignedBox3<Real>& staticBox, const NBRE_AxisAlignedBox3<Real>& movingBox, const NBRE_Vector3<Real>& moveOffset);

    // Ray-AABB
    static nb_boolean HitTest(Real* minB, Real* maxB, Real* origin, Real* dir, Real* coord, uint32 dimension);
    static nb_boolean HitTest(const NBRE_AxisAlignedBox2<Real>& box, const NBRE_Ray2<Real>& ray, NBRE_Vector2<Real>* pIntersectPoint);
    static nb_boolean HitTest(const NBRE_AxisAlignedBox3<Real>& box, const NBRE_Ray3<Real>& ray, NBRE_Vector3<Real>* pIntersectPoint);

    // Ray-Plane
    static nb_boolean HitTest(const NBRE_Plane<Real>& plane, const NBRE_Ray3<Real>& ray, NBRE_Vector3<Real>* pIntersectPoint);
    // Line-Plane
    static nb_boolean HitTest(const NBRE_Plane<Real>& plane, const NBRE_Vector3<Real>& end, const NBRE_Vector3<Real>& start, NBRE_Vector3<Real>* pIntersectPoint, Real epsilon);
    // Plane-Plane
    static nb_boolean HitTest(const NBRE_Plane<Real>& plane1, const NBRE_Plane<Real>& plane2, NBRE_Ray3<Real>* ray);

    // Ray-Triangle
    static nb_boolean HitTest(const NBRE_Triangle3<Real>& triangle, const NBRE_Ray3<Real>& ray, NBRE_Point3f* pIntersectPoint);
    static nb_boolean HitTest(const NBRE_Triangle3<Real>& triangle, const NBRE_Ray3<Real>& ray, Real minT, nb_boolean isCCW);
    /// Ray-Ray
    static nb_boolean HitTest(const NBRE_Ray2<Real>& ray1, const NBRE_Ray2<Real>& ray2, NBRE_Vector2<Real>* pIntersectPoint);
    static nb_boolean HitTest(const NBRE_Ray3<Real>& ray1, const NBRE_Ray3<Real>& ray2, NBRE_Vector3<Real>* pIntersectPoint);
    /// Ray-Polyhedron
    static int32 HitTest(const NBRE_Ray3<Real>& ray, Real maxRayT, NBRE_Plane<Real>* normals, uint32 normalCount, Real *intersectRayT, NBRE_Plane<Real>* intersectNormal);
    // Three-Plane intersect point
    static nb_boolean HitTest(const NBRE_Plane<Real>& plane1, const NBRE_Plane<Real>& plane2, const NBRE_Plane<Real>& plane3, NBRE_Vector3<Real>* pIntersectPoint);

    // Point-Frustum
    static nb_boolean HitTest(const NBRE_Frustum3<Real>& frustum, const NBRE_Vector3<Real>& point);
    // Frustum-AABB
    static nb_boolean HitTest(const NBRE_Frustum3<Real>& frustum, const NBRE_AxisAlignedBox3<Real>& box);
    // @TODO:
    // Triangle-AABB

    static nb_boolean HitTest(const NBRE_Sphere3<Real>& sphere, const NBRE_Vector3<Real>& point);
    static nb_boolean HitTest(const NBRE_Sphere3<Real>& sphere1, const NBRE_Sphere3<Real>& sphere2);
    static nb_boolean HitTest(const NBRE_Sphere3<Real>& sphere, const NBRE_AxisAlignedBox3<Real>& box);
    static nb_boolean HitTest(const NBRE_Sphere3<Real>& sphere, const NBRE_Ray2<Real>& ray);

    static nb_boolean HitTest(const NBRE_Vector<NBRE_Vector2<Real> >& polyline1, const NBRE_Vector<NBRE_Vector2<Real> >& polyline2);

    // Project
    static NBRE_Vector3<Real> ProjectTo(const NBRE_Vector3<Real>& point, const NBRE_Plane<Real>& plane);
    static NBRE_Segment2<Real> ProjectTo(const NBRE_Segment2<Real>& segment, const NBRE_Ray2<Real>& ray);
    static NBRE_Segment3<Real> ProjectTo(const NBRE_Segment3<Real>& segment, const NBRE_Plane<Real>& plane);
    static NBRE_Segment3<Real> ProjectTo(const NBRE_Triangle3<Real>& triangle, const NBRE_Plane<Real>& plane);

    // Plane side
    static NBRE_PlaneSide GetPlaneSide(const NBRE_Segment3<Real>& segment, const NBRE_Plane<Real>& plane);
    static NBRE_PlaneSide GetPlaneSide(const NBRE_Triangle3<Real>& triangle, const NBRE_Plane<Real>& plane);
    static NBRE_PlaneSide GetPlaneSide(const NBRE_AxisAlignedBox3<Real>& box, const NBRE_Plane<Real>& plane);
    static NBRE_PlaneSide GetPlaneSide(const NBRE_Frustum3<Real>& frustum, const NBRE_Plane<Real>& plane);

    static nb_boolean Contain(const NBRE_Frustum3<Real>& frustum, const NBRE_AxisAlignedBox3<Real>& box);

    static nb_boolean Contain(const NBRE_Vector<NBRE_Vector2<Real> >& polyline, const NBRE_Vector2<Real>& point);
};

#include "nbreintersection.inl"

typedef NBRE_Intersection<float> NBRE_Intersectionf;
typedef NBRE_Intersection<double> NBRE_Intersectiond;

/** @} */
#endif
