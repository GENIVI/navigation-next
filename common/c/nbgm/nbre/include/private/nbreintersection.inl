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
template <typename Real>
inline nb_boolean 
NBRE_Intersection<Real>::HitTest(const NBRE_AxisAlignedBox2<Real>& box, const NBRE_Vector2<Real>& point)
{
    return point.x < box.minExtend.x || point.y < box.minExtend.y 
        || point.x > box.maxExtend.x || point.y > box.maxExtend.y
        ? FALSE : TRUE;
}

template <typename Real>
inline nb_boolean 
NBRE_Intersection<Real>::HitTest(const NBRE_AxisAlignedBox3<Real>& box, const NBRE_Vector3<Real>& point)
{
    return point.x < box.minExtend.x || point.y < box.minExtend.y || point.z < box.minExtend.z 
        || point.x > box.maxExtend.x || point.y > box.maxExtend.y || point.z > box.maxExtend.z
        ? FALSE : TRUE;
}

template <typename Real>
inline nb_boolean 
NBRE_Intersection<Real>::HitTest(const NBRE_AxisAlignedBox2<Real>& box1, const NBRE_AxisAlignedBox2<Real>& box2)
{
    return box1.minExtend.x > box2.maxExtend.x || box1.minExtend.y > box2.maxExtend.y
        || box1.maxExtend.x < box2.minExtend.x || box1.maxExtend.y < box2.minExtend.y
        ? FALSE : TRUE;
}

template <typename Real>
inline nb_boolean 
NBRE_Intersection<Real>::HitTest(const NBRE_AxisAlignedBox3<Real>& box1, const NBRE_AxisAlignedBox3<Real>& box2)
{
    return box1.minExtend.x > box2.maxExtend.x || box1.minExtend.y > box2.maxExtend.y || box1.minExtend.z > box2.maxExtend.z
        || box1.maxExtend.x < box2.minExtend.x || box1.maxExtend.y < box2.minExtend.y || box1.maxExtend.z < box2.minExtend.z
        ? FALSE : TRUE;
}

template <typename Real>
inline nb_boolean
NBRE_Intersection<Real>::HitTest(const NBRE_NoneAxisAlignedBox3<Real>& box1, const NBRE_NoneAxisAlignedBox3<Real>& box2)
{
    if(box1.Contain(box2.center))
    {
        return TRUE;
    }
    if(box2.Contain(box1.center))
    {
        return TRUE;
    }

    NBRE_Vector3d corners[8];
    box1.GetCorners(corners);
    for(int i = 0; i < 8; ++i)
    {
        if(box2.Contain(corners[i]))
        {
            return TRUE;
        }
    }

    box2.GetCorners(corners);
    for(int i = 0; i < 8; ++i)
    {
        if(box1.Contain(corners[i]))
        {
            return TRUE;
        }
    }
    return FALSE;
}

template <typename Real>
nb_boolean 
NBRE_Intersection<Real>::HitTest(const NBRE_AxisAlignedBox2<Real>& staticBox, const NBRE_AxisAlignedBox2<Real>& movingBox, const NBRE_Vector2<Real>& moveOffset, Real* pCollideT)
{
    Real tEnter = 0;
    Real tLeave = 1;

    // x axis
    if (moveOffset.x == 0)
    {
        if (movingBox.minExtend.x > staticBox.maxExtend.x || movingBox.maxExtend.x < staticBox.minExtend.x)
        {
            return FALSE;
        }
    }
    else
    {
        // check enter & leave time on axis
        Real inverseOffset = 1 / moveOffset.x;
        Real xEnter = (staticBox.minExtend.x - movingBox.maxExtend.x) * inverseOffset;
        Real xLeave = (staticBox.maxExtend.x - movingBox.minExtend.x) * inverseOffset;
        
        if (xEnter > xLeave)
        {
            // swap
            Real temp = xEnter;
            xEnter = xLeave;
            xLeave = temp;
        }
        // update enter & leave time
        if (xEnter > tEnter)
        {
            tEnter = xEnter;
        }
        if (xLeave < tLeave)
        {
            tLeave = xLeave;
        }
        // check empty interval
        if (tEnter > tLeave)
        {
            return FALSE;
        }
    }

    // y axis
    if (moveOffset.y == 0)
    {
        if (movingBox.minExtend.y > staticBox.maxExtend.y || movingBox.maxExtend.y < staticBox.minExtend.y)
        {
            return FALSE;
        }
    }
    else
    {
        // check enter & leave time on axis
        Real inverseOffset = 1 / moveOffset.y;
        Real yEnter = (staticBox.minExtend.y - movingBox.maxExtend.y) * inverseOffset;
        Real yLeave = (staticBox.maxExtend.y - movingBox.minExtend.y) * inverseOffset;

        if (yEnter > yLeave)
        {
            // swap
            Real temp = yEnter;
            yEnter = yLeave;
            yLeave = temp;
        }
        // update enter & leave time
        if (yEnter > tEnter)
        {
            tEnter = yEnter;
        }
        if (yLeave < tLeave)
        {
            tLeave = yLeave;
        }
        // check empty interval
        if (tEnter > tLeave)
        {
            return FALSE;
        }
    }

    if (pCollideT != NULL)
    {
        *pCollideT = tEnter;
    }

    return TRUE;
}

template <typename Real>
nb_boolean 
NBRE_Intersection<Real>::HitTest(const NBRE_AxisAlignedBox3<Real>& staticBox, const NBRE_AxisAlignedBox3<Real>& movingBox, const NBRE_Vector3<Real>& moveOffset, Real* pCollideT)
{
    Real tEnter = 0;
    Real tLeave = 1;

    // x axis
    if (moveOffset.x == 0)
    {
        if (movingBox.minExtend.x > staticBox.maxExtend.x || movingBox.maxExtend.x < staticBox.minExtend.x)
        {
            return FALSE;
        }
    }
    else
    {
        // check enter & leave time on axis
        Real inverseOffset = 1 / moveOffset.x;
        Real xEnter = (staticBox.minExtend.x - movingBox.maxExtend.x) * inverseOffset;
        Real xLeave = (staticBox.maxExtend.x - movingBox.minExtend.x) * inverseOffset;

        if (xEnter > xLeave)
        {
            // swap
            Real temp = xEnter;
            xEnter = xLeave;
            xLeave = temp;
        }
        // update enter & leave time
        if (xEnter > tEnter)
        {
            tEnter = xEnter;
        }
        if (xLeave < tLeave)
        {
            tLeave = xLeave;
        }
        // check empty interval
        if (tEnter > tLeave)
        {
            return FALSE;
        }
    }

    // y axis
    if (moveOffset.y == 0)
    {
        if (movingBox.minExtend.y > staticBox.maxExtend.y || movingBox.maxExtend.y < staticBox.minExtend.y)
        {
            return FALSE;
        }
    }
    else
    {
        // check enter & leave time on axis
        Real inverseOffset = 1 / moveOffset.y;
        Real yEnter = (staticBox.minExtend.y - movingBox.maxExtend.y) * inverseOffset;
        Real yLeave = (staticBox.maxExtend.y - movingBox.minExtend.y) * inverseOffset;

        if (yEnter > yLeave)
        {
            // swap
            Real temp = yEnter;
            yEnter = yLeave;
            yLeave = temp;
        }
        // update enter & leave time
        if (yEnter > tEnter)
        {
            tEnter = yEnter;
        }
        if (yLeave < tLeave)
        {
            tLeave = yLeave;
        }
        // check empty interval
        if (tEnter > tLeave)
        {
            return FALSE;
        }
    }

    // z axis
    if (moveOffset.z == 0)
    {
        if (movingBox.minExtend.z > staticBox.maxExtend.z || movingBox.maxExtend.z < staticBox.minExtend.z)
        {
            return FALSE;
        }
    }
    else
    {
        // check enter & leave time on axis
        Real inverseOffset = 1 / moveOffset.z;
        Real zEnter = (staticBox.minExtend.z - movingBox.maxExtend.z) * inverseOffset;
        Real zLeave = (staticBox.maxExtend.z - movingBox.minExtend.z) * inverseOffset;

        if (zEnter > zLeave)
        {
            // swap
            Real temp = zEnter;
            zEnter = zLeave;
            zLeave = temp;
        }
        // update enter & leave time
        if (zEnter > tEnter)
        {
            tEnter = zEnter;
        }
        if (zLeave < tLeave)
        {
            tLeave = zLeave;
        }
        // check empty interval
        if (tEnter > tLeave)
        {
            return FALSE;
        }
    }

    if (pCollideT != NULL)
    {
        *pCollideT = tEnter;
    }

    return TRUE;
}

template <typename Real>
nb_boolean
NBRE_Intersection<Real>::HitTest(Real* minB, Real* maxB, Real* origin, Real* dir, Real* coord, uint32 dimension)
{
    const uint32 MIDDLE = 0;
    const uint32 LEFT = 1;
    const uint32 RIGHT = 2;
    const uint32 MAX_DIMESION = 3;

    nb_boolean inside = TRUE;
    uint32 whichPlane;
    uint32 quadrant[MAX_DIMESION];    
    Real maxT[MAX_DIMESION];
    Real candidatePlane[MAX_DIMESION];

    // Find candidate planes
    for (uint32 i = 0; i < dimension; i++)
    {
        if (origin[i] < minB[i])
        {
            quadrant[i] = LEFT;
            candidatePlane[i] = minB[i];
            inside = FALSE;
        }
        else if (origin[i] > maxB[i])
        {
            quadrant[i] = RIGHT;
            candidatePlane[i] = maxB[i];
            inside = FALSE;
        }
        else
        {
            quadrant[i] = MIDDLE;
        }
    }

    if (inside)
    {
        for (uint32 i = 0; i < dimension; i++)
        {
            coord[i] = origin[i];
        }
        return TRUE;
    }

    // Calculate T distances to candidate planes
    for (uint32 i = 0; i < dimension; i++)
    {
        if (quadrant[i] != MIDDLE && dir[i] !=0.)
        {
            maxT[i] = (candidatePlane[i] - origin[i]) / dir[i];
        }
        else
        {
            maxT[i] = -1.;
        }
    }

    // Get largest of the maxT's for final choice of intersection
    whichPlane = 0;
    for (uint32 i = 1; i < dimension; i++)
    {
        if (maxT[whichPlane] < maxT[i])
        {
            whichPlane = i;
        }
    }

    // Check final candidate actually inside box
    if (maxT[whichPlane] < 0)
    {
        return FALSE;
    }

    for (uint32 i = 0; i < dimension; i++)
    {
        if (whichPlane != i)
        {
            coord[i] = origin[i] + maxT[whichPlane] *dir[i];
            if (coord[i] < minB[i] || coord[i] > maxB[i])
            {
                return FALSE;
            }
        } 
        else 
        {
            coord[i] = candidatePlane[i];
        }
    }

    return TRUE;
}

template <typename Real>
nb_boolean 
NBRE_Intersection<Real>::HitTest(const NBRE_AxisAlignedBox2<Real>& box, const NBRE_Ray2<Real>& ray, NBRE_Vector2<Real>* pIntersectPoint)
{
    Real coord[] = {0, 0};
    Real minExt[] = {box.minExtend.x, box.minExtend.y};
    Real maxExt[] = {box.maxExtend.x, box.maxExtend.y};
    Real origin[] = {ray.origin.x, ray.origin.y};
    Real direction[] = {ray.direction.x, ray.direction.y};
    nb_boolean result = HitTest(minExt, maxExt, origin, direction, coord, 2);

    if (pIntersectPoint != NULL)
    {
        pIntersectPoint->x = coord[0];
        pIntersectPoint->y = coord[1];
    }
    return result;
}

template <typename Real>
nb_boolean 
NBRE_Intersection<Real>::HitTest(const NBRE_AxisAlignedBox3<Real>& box, const NBRE_Ray3<Real>& ray, NBRE_Vector3<Real>* pIntersectPoint)
{
    Real coord[] = {0, 0, 0};
    Real minExt[] = {box.minExtend.x, box.minExtend.y, box.minExtend.z};
    Real maxExt[] = {box.maxExtend.x, box.maxExtend.y, box.maxExtend.z};
    Real origin[] = {ray.origin.x, ray.origin.y, ray.origin.z};
    Real direction[] = {ray.direction.x, ray.direction.y, ray.direction.z};    
    nb_boolean result = HitTest(minExt, maxExt, origin, direction, coord, 3);

    if (pIntersectPoint != NULL)
    {
        pIntersectPoint->x = coord[0];
        pIntersectPoint->y = coord[1];
        pIntersectPoint->z = coord[2];
    }
    return result;
}

template <typename Real>
nb_boolean
NBRE_Intersection<Real>::HitTest(const NBRE_Plane<Real>& plane, const NBRE_Ray3<Real>& ray, NBRE_Vector3<Real>* pIntersectPoint)
{
    Real denominator = plane.normal.DotProduct(ray.direction);
    if (denominator >= 0)
    {
        return FALSE;
    }
    Real t = (ray.origin.DotProduct(plane.normal) + plane.d) / -denominator;
    if (t < 0)
    {
        return FALSE;
    }
    else
    {
        if (pIntersectPoint != NULL)
        {
            *pIntersectPoint = ray.origin + ray.direction * t;
        }
        return TRUE;
    }
}

template <typename Real>
nb_boolean
NBRE_Intersection<Real>::HitTest(const NBRE_Plane<Real>& plane, const NBRE_Vector3<Real>& end, const NBRE_Vector3<Real>& start, NBRE_Vector3<Real>* pIntersectPoint, Real epsilon)
{
    NBRE_Vector3<Real> dir =  end - start;
    Real denominator = plane.normal.DotProduct(dir);
    if (NBRE_Math::Abs(denominator) < epsilon)
    {
        return FALSE;
    }
    Real t = (start.DotProduct(plane.normal) + plane.d) / -denominator;
    if (t < 0 || t > 1)
    {
        return FALSE;
    }
    else
    {
        if (pIntersectPoint != NULL)
        {
            *pIntersectPoint = start + dir * t;
        }
        return TRUE;
    }
}

template <typename Real>
nb_boolean
NBRE_Intersection<Real>::HitTest(const NBRE_Plane<Real>& plane1, const NBRE_Plane<Real>& plane2, NBRE_Ray3<Real>* ray)
{
    if(ray == NULL)
    {
        return FALSE;
    }
    Real invdet;              /* inverse of 2x2 matrix determinant */
    NBRE_Vector3<Real> dir2;    /* holds the squares of the coordinates of dir */
    NBRE_Vector3<Real> dir = plane1.normal.CrossProduct(plane2.normal);
    NBRE_Vector3<Real> origin;
    dir2.x = dir.x * dir.x;
    dir2.y = dir.y * dir.y;
    dir2.z = dir.z * dir.z;
    if(dir2.z > dir2.y && dir2.z > dir2.x)
    {
        /* then get a point on the XY plane */
        invdet = 1 / dir.z;
        if(NBRE_Math::IsNaN(invdet))
        {
            return FALSE;
        }
        /* solve < pl1.x * xpt.x + pl1.y * xpt.y = - pl1.w >
             < pl2.x * xpt.x + pl2.y * xpt.y = - pl2.w > */
        origin = NBRE_Vector3<Real>(plane1.normal.y * plane2.d - plane2.normal.y * plane1.d,
            plane2.normal.x * plane1.d - plane1.normal.x * plane2.d, 0);
    }
    else if(dir2.y > dir2.x)
    {
        /* then get a point on the XZ plane */
        invdet = 1 / dir.y;
        if(NBRE_Math::IsNaN(invdet))
        {
            return FALSE;
        }
        /* solve < pl1.x * xpt.x + pl1.z * xpt.z = -pl1.w >
                 < pl2.x * xpt.x + pl2.z * xpt.z = -pl2.w > */
        origin = NBRE_Vector3<Real>(plane1.normal.z * plane2.d - plane2.normal.z * plane1.d, 0,
            plane2.normal.x * plane1.d - plane1.normal.x * plane2.d);
    }
    else
    {
        /* then get a point on the YZ plane */
        invdet = 1 / dir.x;
        if(NBRE_Math::IsNaN(invdet))
        {
            return FALSE;
        }
        /* solve < pl1.y * xpt.y + pl1.z * xpt.z = - pl1.w >
             < pl2.y * xpt.y + pl2.z * xpt.z = - pl2.w > */
        origin = NBRE_Vector3<Real>(0, plane1.normal.z * plane2.d - plane2.normal.z * plane1.d,
            plane2.normal.y * plane1.d - plane1.normal.y * plane2.d);
    }

    origin = invdet * origin;

    invdet = 1 / nsl_sqrt(dir2.x + dir2.y + dir2.z);
    dir = invdet * dir;

    ray->origin = origin;
    ray->direction = dir;

    return TRUE;
}

template <typename Real>
nb_boolean
NBRE_Intersection<Real>::HitTest(const NBRE_Triangle3<Real>& triangle, const NBRE_Ray3<Real>& ray, NBRE_Point3f* pIntersectPoint)
{
    NBRE_Vector3<Real> edge1(triangle.vertices[1] - triangle.vertices[0]);
    NBRE_Vector3<Real> edge2(triangle.vertices[2] - triangle.vertices[0]);
    NBRE_Vector3<Real> pvec = ray.direction.CrossProduct(edge2);
    Real inv_det = 1 / edge1.DotProduct(pvec);
    
    if (NBRE_Math::IsNaN<Real>(inv_det))
    {
        return FALSE;
    }

    NBRE_Vector3<Real> tvec(ray.origin - triangle.vertices[0]);
    Real u = tvec.DotProduct(pvec) * inv_det;
    
    if (u < 0 || u > 1)
    {
        return FALSE;
    }

    NBRE_Vector3<Real> qvec(tvec.CrossProduct(edge1));
    Real v = ray.direction.DotProduct(qvec) * inv_det;
    
    if (v < 0 || u + v > 1)
    {
        return FALSE;
    }

    if (pIntersectPoint != NULL)
    {
        *pIntersectPoint = triangle.vertices[0] * (1 - u - v) + triangle.vertices[1] * u + triangle.vertices[2] * v;
    }

    return TRUE;
}

template <typename Real>
nb_boolean
NBRE_Intersection<Real>::HitTest(const NBRE_Triangle3<Real>& triangle, const NBRE_Ray3<Real>& ray, Real minT, nb_boolean isCCW)
{
    const NBRE_Vector3<Real>& rayOrg = ray.origin;
    const NBRE_Vector3<Real>& rayDelta = ray.direction;
    const NBRE_Vector3<Real>& p0 = triangle.vertices[0];
    const NBRE_Vector3<Real>& p1 = triangle.vertices[1];
    const NBRE_Vector3<Real>& p2 = triangle.vertices[2];

    NBRE_Vector3<Real> e1 = p1 - p0;
    NBRE_Vector3<Real> e2 = p2 - p0;
    NBRE_Vector3<Real> n;
    
    if (isCCW)
    {
        n = e2.CrossProduct(e1);
    }
    else
    {
        n = e1.CrossProduct(e2);
    }
    
    Real dot = n.DotProduct(rayDelta);

    if (!(dot < 0))
    {
        return FALSE;
    }

    Real d = n.DotProduct(p0);
    Real t = d - n.DotProduct(rayOrg);
    if (!(t <= 0))
    {
        return FALSE;
    }

    if (!(t >= dot * minT))
    {
        return FALSE;
    }

    t /= dot;
    NBRE_Vector3<Real> intersectPoint = rayOrg + rayDelta * t;

    Real u0, u1, u2;
    Real v0, v1, v2;
    Real temp = 0;
    Real alpha = 0;
    Real beta = 0;
    Real gamma = 0;

    if (NBRE_Math::Abs<Real>(n.x) > NBRE_Math::Abs<Real>(n.y))
    {
        if (NBRE_Math::Abs<Real>(n.x) > NBRE_Math::Abs<Real>(n.z))
        {
            u0 = intersectPoint.y - p0.y;
            u1 = p1.y - p0.y;
            u2 = p2.y - p0.y;
            v0 = intersectPoint.z - p0.z;
            v1 = p1.z - p0.z;
            v2 = p2.z - p0.z;
        }
        else
        {
            u0 = intersectPoint.x - p0.x;
            u1 = p1.x - p0.x;
            u2 = p2.x - p0.x;
            v0 = intersectPoint.y - p0.y;
            v1 = p1.y - p0.y;
            v2 = p2.y - p0.y;
        }
    }
    else
    {
        if (NBRE_Math::Abs<Real>(n.y) > NBRE_Math::Abs<Real>(n.z))
        {
            u0 = intersectPoint.x - p0.x;
            u1 = p1.x - p0.x;
            u2 = p2.x - p0.x;
            v0 = intersectPoint.z - p0.z;
            v1 = p1.z - p0.z;
            v2 = p2.z - p0.z;
        }
        else
        {
            u0 = intersectPoint.x - p0.x;
            u1 = p1.x - p0.x;
            u2 = p2.x - p0.x;
            v0 = intersectPoint.y - p0.y;
            v1 = p1.y - p0.y;
            v2 = p2.y - p0.y;
        }
    }

    temp = u1 * v2 - v1 * u2;
    if (!(temp != 0))
    {
        return FALSE;
    }
    temp = 1.0f / temp;
    alpha = (u0 * v2 - v0 * u2) * temp;
    if (!(alpha >= 0))
    {
        return FALSE;
    }
    beta = (u1 * v0 - v1 * u0) * temp;
    if (!(beta >= 0))
    {
        return FALSE;
    }
    gamma = 1.0f - alpha - beta;
    if (!(gamma >= 0))
    {
        return FALSE;
    }
    return TRUE;
}

template <typename Real>
inline nb_boolean 
NBRE_Intersection<Real>::HitTest(const NBRE_Plane<Real>& plane1, const NBRE_Plane<Real>& plane2, const NBRE_Plane<Real>& plane3, NBRE_Vector3<Real>* pIntersectPoint)
{
    Real demoninator = plane1.normal.CrossProduct(plane2.normal).DotProduct(plane3.normal);
    if (demoninator == 0)
    {
        return FALSE;
    }

    if (pIntersectPoint != NULL)
    {
        *pIntersectPoint = 
            ( plane2.normal.CrossProduct(plane3.normal) * plane1.d 
            + plane3.normal.CrossProduct(plane1.normal) * plane2.d
            + plane1.normal.CrossProduct(plane2.normal) * plane3.d
            ) / -demoninator;
    }
    
    return TRUE;
}

template <typename Real>
inline nb_boolean 
NBRE_Intersection<Real>::HitTest(const NBRE_Frustum3<Real>& frustum, const NBRE_Vector3<Real>& point)
{
    return !HitTest(frustum.GetAABB(), point)
        || frustum.GetLeft().GetSide(point) == NBRE_PS_BACK_SIDE
        || frustum.GetRight().GetSide(point) == NBRE_PS_BACK_SIDE
        || frustum.GetBottom().GetSide(point) == NBRE_PS_BACK_SIDE
        || frustum.GetTop().GetSide(point) == NBRE_PS_BACK_SIDE
        || frustum.GetNear().GetSide(point) == NBRE_PS_BACK_SIDE
        || frustum.GetFar().GetSide(point) == NBRE_PS_BACK_SIDE
        ? FALSE : TRUE;
}

template <typename Real>
inline nb_boolean 
NBRE_Intersection<Real>::HitTest(const NBRE_Frustum3<Real>& frustum, const NBRE_AxisAlignedBox3<Real>& box)
{
    if (!HitTest(frustum.GetAABB(), box))
    {
        return FALSE;
    }

    NBRE_Plane<Real> frustumPlanes[6] = 
        { frustum.GetLeft(), frustum.GetRight()
        , frustum.GetBottom(), frustum.GetTop()
        , frustum.GetNear(), frustum.GetFar() };

    NBRE_Vector3<Real> boxCorners[8];
    box.GetCorners(boxCorners);

    for (uint32 i = 0; i < 6; ++i)
    {
        if (frustumPlanes[i].GetSide(boxCorners[0]) != NBRE_PS_BACK_SIDE)
        {
            continue;
        }
        if (frustumPlanes[i].GetSide(boxCorners[1]) != NBRE_PS_BACK_SIDE)
        {
            continue;
        }
        if (frustumPlanes[i].GetSide(boxCorners[2]) != NBRE_PS_BACK_SIDE)
        {
            continue;
        }
        if (frustumPlanes[i].GetSide(boxCorners[3]) != NBRE_PS_BACK_SIDE)
        {
            continue;
        }
        if (frustumPlanes[i].GetSide(boxCorners[4]) != NBRE_PS_BACK_SIDE)
        {
            continue;
        }
        if (frustumPlanes[i].GetSide(boxCorners[5]) != NBRE_PS_BACK_SIDE)
        {
            continue;
        }
        if (frustumPlanes[i].GetSide(boxCorners[6]) != NBRE_PS_BACK_SIDE)
        {
            continue;
        }
        if (frustumPlanes[i].GetSide(boxCorners[7]) != NBRE_PS_BACK_SIDE)
        {
            continue;
        }
        return FALSE;
    }

    return TRUE;
}

template <typename Real>
inline nb_boolean 
NBRE_Intersection<Real>::HitTest(const NBRE_Ray2<Real>& ray1, const NBRE_Ray2<Real>& ray2, NBRE_Vector2<Real>* pIntersectPoint)
{
    const NBRE_Vector2<Real>& d1 = ray1.direction;
    const NBRE_Vector2<Real>& d2 = ray2.direction;
    const NBRE_Vector2<Real>& d2Perpendicular = d2.Perpendicular();
    Real demoninator = d1.DotProduct(d2Perpendicular);
    if (demoninator == 0)
    {
        return FALSE;
    }

    const NBRE_Vector2<Real>& p1 = ray1.origin;
    const NBRE_Vector2<Real>& p2 = ray2.origin;
    const NBRE_Vector2<Real>& p1p2 = p2 - p1;
    Real t1 = p1p2.DotProduct(d2Perpendicular) / demoninator;
    if (pIntersectPoint != NULL)
    {
        *pIntersectPoint = p1 + d1 * t1;
    }

    return TRUE;
}

template <typename Real>
inline nb_boolean 
NBRE_Intersection<Real>::HitTest(const NBRE_Ray3<Real>& ray1, const NBRE_Ray3<Real>& ray2, NBRE_Vector3<Real>* pIntersectPoint)
{   
    const NBRE_Vector3<Real>& d1 = ray1.direction;
    const NBRE_Vector3<Real>& d2 = ray2.direction;
    const NBRE_Vector3<Real>& d1d2Cross = d1.CrossProduct(d2);
    Real d1d2CrossLengthSq = d1d2Cross.SquaredLength();

    if (d1d2CrossLengthSq == 0)
    {
        return FALSE;
    }

    const NBRE_Vector3<Real>& p1 = ray1.origin;
    const NBRE_Vector3<Real>& p2 = ray2.origin;
    const NBRE_Vector3<Real>& p1p2 = p2 - p1;

    Real t1 = p1p2.CrossProduct(d2).DotProduct(d1d2Cross) / d1d2CrossLengthSq;
    if (pIntersectPoint != NULL)
    {
        *pIntersectPoint = p1 + d1 * t1;
    }

    return TRUE;
}

template <typename Real>
inline  nb_boolean 
NBRE_Intersection<Real>::HitTest(const NBRE_Sphere3<Real>& sphere, const NBRE_Vector3<Real>& point)
{
    return point.SquaredDistanceTo(sphere.center) <= sphere.radius * sphere.radius;
}

template <typename Real>
inline  nb_boolean 
NBRE_Intersection<Real>::HitTest(const NBRE_Sphere3<Real>& sphere1, const NBRE_Sphere3<Real>& sphere2)
{
    Real totalR = sphere1.radius + sphere2.radius;
    return sphere1.center.SquaredDistanceTo(sphere2.center) <= totalR * totalR;
}

template <typename Real>
inline  nb_boolean 
NBRE_Intersection<Real>::HitTest(const NBRE_Sphere3<Real>& sphere, const NBRE_AxisAlignedBox3<Real>& box)
{
    return HitTest(sphere, box.GetClosestPointTo(sphere.center));
}

template <typename Real>
inline  nb_boolean 
NBRE_Intersection<Real>::HitTest(const NBRE_Sphere3<Real>& sphere, const NBRE_Ray2<Real>& ray)
{
    Real distance = ray.DistanceTo(sphere.center);
    if (distance > sphere.radius)
    {
        return FALSE;
    }

    return HitTest(sphere, ray.GetClosestPointTo(sphere.center));
}

template <typename Real>
inline  nb_boolean 
IsOneSide(const NBRE_Vector<NBRE_Vector2<Real> >& polyline1, const NBRE_Vector<NBRE_Vector2<Real> >& polyline2)
{
    int32 size1 = polyline1.size();
    int32 size2 = polyline2.size();
    if(size1 < 3)
    {
        return FALSE;
    }
    nb_boolean isClockwise = NBRE_IsClockwise(polyline1);
    for(int i = 0; i < size1; ++i)
    {
        NBRE_Vector2<Real> pt1 = polyline1[i];
        NBRE_Vector2<Real> pt2;
        if((i+1) < size1)
        {
            pt2 = polyline1[i+1];
        }
        else
        {
            pt2 = polyline1[0];
        }

        int j = 0;
        for(; j < size2; ++j)
        {
            const NBRE_Vector2<Real>& point = polyline2[j];
            if(isClockwise)
            {
                if((pt1-point).CrossProduct(pt2-point)>0)
                {
                    break;
                }
            }
            else
            {
                if((pt1-point).CrossProduct(pt2-point)<0)
                {
                    break;
                }
            }
        }
        if(j == size2)
        {
            return TRUE;
        }
    }
    return FALSE;
}

template <typename Real>
inline  nb_boolean 
NBRE_Intersection<Real>::HitTest(const NBRE_Vector<NBRE_Vector2<Real> >& polyline1, const NBRE_Vector<NBRE_Vector2<Real> >& polyline2)
{
    if(IsOneSide(polyline1, polyline2))
    {
        return FALSE;
    }
    if(IsOneSide(polyline2, polyline1))
    {
        return FALSE;
    }
    return TRUE;
}

template <typename Real>
inline NBRE_Segment2<Real> 
NBRE_Intersection<Real>::ProjectTo(const NBRE_Segment2<Real>& segment, const NBRE_Ray2<Real>& ray)
{
    return NBRE_Segment2<Real>(ray.GetClosestPointTo(segment.startPoint), ray.GetClosestPointTo(segment.endPoint));
}

template <typename Real>
inline NBRE_Vector3<Real> 
NBRE_Intersection<Real>::ProjectTo(const NBRE_Vector3<Real>& point, const NBRE_Plane<Real>& plane)
{
    return plane.GetClosestPointTo(point);
}

template <typename Real>
inline NBRE_Segment3<Real> 
NBRE_Intersection<Real>::ProjectTo(const NBRE_Segment3<Real>& segment, const NBRE_Plane<Real>& plane)
{
    return NBRE_Segment3<Real>(plane.GetClosestPointTo(segment.startPoint), plane.GetClosestPointTo(segment.endPoint));
}

template <typename Real>
inline NBRE_Segment3<Real> 
NBRE_Intersection<Real>::ProjectTo(const NBRE_Triangle3<Real>& triangle, const NBRE_Plane<Real>& plane)
{
    return NBRE_Triangle3<Real>(plane.GetClosestPointTo(triangle.vertices[0]), plane.GetClosestPointTo(triangle.vertices[1]), plane.GetClosestPointTo(triangle.vertices[2]));
}

template <typename Real>
inline NBRE_PlaneSide 
NBRE_Intersection<Real>::GetPlaneSide(const NBRE_Segment3<Real>& segment, const NBRE_Plane<Real>& plane)
{
    NBRE_PlaneSide startSide = plane.GetSide(segment.startPoint);
    NBRE_PlaneSide endSide = plane.GetSide(segment.endPoint);

    if (startSide == NBRE_PS_FRONT_SIDE && endSide == NBRE_PS_FRONT_SIDE)
    {
        return NBRE_PS_FRONT_SIDE;
    }
    else if (startSide == NBRE_PS_BACK_SIDE && endSide == NBRE_PS_BACK_SIDE)
    {
        return NBRE_PS_BACK_SIDE;
    }
    else
    {
        return NBRE_PS_SPANNING;
    }
}

template <typename Real>
inline NBRE_PlaneSide 
NBRE_Intersection<Real>::GetPlaneSide(const NBRE_Triangle3<Real>& triangle, const NBRE_Plane<Real>& plane)
{
    NBRE_PlaneSide result = plane.GetSide(triangle.vertices[0]);

    for (uint32 i = 1; i < 3; ++i)
    {
        if (plane.GetSide(triangle.vertices[i]) != result)
        {
            return NBRE_PS_SPANNING;
        }
    }
    return result;
}

template <typename Real>
inline NBRE_PlaneSide 
NBRE_Intersection<Real>::GetPlaneSide(const NBRE_AxisAlignedBox3<Real>& box, const NBRE_Plane<Real>& plane)
{
    NBRE_Vector3<Real> corners[8];
    box.GetCorners(corners);
    NBRE_PlaneSide result = plane.GetSide(corners[0]);

    for (uint32 i = 1; i < 8; ++i)
    {
        if (plane.GetSide(corners[i]) != result)
        {
            return NBRE_PS_SPANNING;
        }
    }
    return result;
}

template <typename Real>
inline NBRE_PlaneSide 
NBRE_Intersection<Real>::GetPlaneSide(const NBRE_Frustum3<Real>& frustum, const NBRE_Plane<Real>& plane)
{
    NBRE_Vector3<Real> corners[8];
    frustum.GetCorners(corners);
    NBRE_PlaneSide result = plane.GetSide(corners[0]);

    for (uint32 i = 1; i < 8; ++i)
    {
        if (plane.GetSide(corners[i]) != result)
        {
            return NBRE_PS_SPANNING;
        }
    }
    return result;
}

template <typename Real>
inline nb_boolean 
NBRE_Intersection<Real>::Contain(const NBRE_Frustum3<Real>& frustum, const NBRE_AxisAlignedBox3<Real>& box)
{
    if (!frustum.GetAABB().Contain(box))
    {
        return FALSE;
    }

    NBRE_Vector3<Real> boxCorners[8];
    box.GetCorners(boxCorners);
    for (uint32 i = 0; i < 8; ++i)
    {
        if(!NBRE_Intersection<Real>::HitTest(frustum, boxCorners[i]))
        {
            return FALSE;
        }
    }
    return TRUE;
}

//for convex polyline
template <typename Real>
inline nb_boolean 
Contain(const NBRE_Vector<NBRE_Vector2<Real> >& polyline, const NBRE_Vector2<Real>& point)
{
    int32 size = polyline.size();
    if(size < 3)
    {
        return FALSE;
    }
    NBRE_Vector2<Real> pt1 = polyline[0] - point;
    NBRE_Vector2<Real> pt2 = polyline[1] - point;
    nb_boolean inside = pt1.CrossProduct(pt2)>0;
    for(int i = 1; i < size; ++i)
    {
        pt1 = polyline[i] - point;
        if((i+1) < size)
        {
            pt2 = polyline[i+1] - point;
        }
        else
        {
            pt2 = polyline[0] - point;
        }
        nb_boolean ret = pt1.CrossProduct(pt2)>0;
        if(ret != inside)
        {
            return FALSE;
        }
    }
    return TRUE;
}

/// Ray-polyhedron intersection test.
/// Polyhedron is defined by face planes (with normal pointing to outside)
/// return 0 if no intersection found, 1 if ray intersect from outside, -1 if ray intersect from inside
template <typename Real> inline int32
NBRE_Intersection<Real>::HitTest(
    const NBRE_Ray3<Real>& ray,	/* ray */
    Real maxRayT,		/* maximum useful distance along ray */
    NBRE_Plane<Real>* normals,	/* list of planes in convex polyhedron */
    uint32 normalCount,	/* number of planes in convex polyhedron */
    Real *intersectRayT,	/* returned: distance of intersection along ray */
    NBRE_Plane<Real>* intersectNormal		/* returned: normal of face hit */
)
{
    /* front/back face # hit */
    uint32 frontNormalIndex;
    uint32 backNormalIndex;

    Real tnear = -NBRE_Math::Infinity;
    Real tfar = maxRayT;

    /* Test each plane in polyhedron */
    for ( uint32 i = 0; i < normalCount; ++i )
    {
        NBRE_Plane<Real>* pln = normals + i;
        /* Compute intersection point T and sidedness */
        // (RayOrg + t * RayNormal) * PlaneNormal + PlaneD = 0
        // t = (RayOrg * PlaneNormal + PlaneD) / -(RayNormal * PlaneNormal)
        // t = vn / -vd
        Real vd = ray.direction.DotProduct(pln->normal);
        Real vn = ray.origin.DotProduct(pln->normal) + pln->d;
        if ( vd == 0 )
        {
            /* ray is parallel to plane - check if ray origin is inside plane's
               half-space */
            if ( vn > 0 )
            {
                /* ray origin is outside half-space */
                return 0;
            }
        }
        else
        {
            /* ray not parallel - get distance to plane */
            Real t = -vn / vd ;
            if ( vd < 0 )
            {
                /* front face - T is a near point */
                if ( t > tfar )
                {
                    return 0;
                }
                if ( t > tnear )
                {
                    /* hit near face, update normal */
                    frontNormalIndex = normalCount ;
                    tnear = t ;
                }
            }
            else
            {
                /* back face - T is a far point */
                if ( t < tnear )
                {
                    return 0;
                }
                if ( t < tfar )
                {
                    /* hit far face, update normal */
                    backNormalIndex = normalCount ;
                    tfar = t ;
                }
            }
        }
    }

    /* survived all tests */
    /* Note: if ray originates on polyhedron, may want to change 0.0 to some
     * epsilon to avoid intersecting the originating face.
     */
    if ( tnear >= 0 )
    {
        /* outside, hitting front face */
        *intersectNormal = normals[frontNormalIndex];
        *intersectRayT = tnear ;
        return 1;
    }
    else
    {
        if ( tfar < maxRayT )
        {
            /* inside, hitting back face */
            *intersectNormal = normals[backNormalIndex];
            *intersectRayT = tfar;
            return -1;
        }
        else
        {
            /* inside, but back face beyond tmax */
            // Note: This may not be the wanted result if tmax is a small value.
            return 0;
        }
    }
}
