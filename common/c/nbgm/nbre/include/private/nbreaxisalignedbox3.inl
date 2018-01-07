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
#include "nbremath.h"

template <typename Real> 
inline
NBRE_AxisAlignedBox3<Real>::NBRE_AxisAlignedBox3()
{
    SetNull();
}

template <typename Real> 
inline
NBRE_AxisAlignedBox3<Real>::NBRE_AxisAlignedBox3(Real minX, Real minY, Real minZ, Real maxX, Real maxY, Real maxZ)
:minExtend(minX, minY, minZ)
,maxExtend(maxX, maxY, maxZ)
{
}

template <typename Real> 
inline
NBRE_AxisAlignedBox3<Real>::NBRE_AxisAlignedBox3(const NBRE_Vector3<Real>& minExtend, const NBRE_Vector3<Real>& maxExtend)
:minExtend(minExtend)
,maxExtend(maxExtend)
{
}

template <typename Real> 
inline
NBRE_AxisAlignedBox3<Real>::NBRE_AxisAlignedBox3(const NBRE_AxisAlignedBox3<Real>& box)
:minExtend(box.minExtend)
,maxExtend(box.maxExtend)
{
}

template <typename Real> 
inline
NBRE_AxisAlignedBox3<Real>::~NBRE_AxisAlignedBox3()
{
}

template <typename Real> 
inline NBRE_AxisAlignedBox3<Real>&
NBRE_AxisAlignedBox3<Real>::operator = (const NBRE_AxisAlignedBox3<Real>& box)
{
    if(this != &box)
    {
        this->minExtend = box.minExtend;
        this->maxExtend = box.maxExtend;
    }
    return *this;
}

template <typename Real> 
inline void 
NBRE_AxisAlignedBox3<Real>::Merge(Real x, Real y, Real z)
{
    if (IsNull())
    {
        minExtend.x = maxExtend.x = x;
        minExtend.y = maxExtend.y = y;
        minExtend.z = maxExtend.z = z;
    }
    else
    {
        if (x < minExtend.x)
        {
            minExtend.x = x;
        }
        if (x > maxExtend.x)
        {
            maxExtend.x = x;
        }

        if (y < minExtend.y)
        {
            minExtend.y = y;
        }
        if (y > maxExtend.y)
        {
            maxExtend.y = y;
        }

        if (z < minExtend.z)
        {
            minExtend.z = z;
        }
        if (z > maxExtend.z)
        {
            maxExtend.z = z;
        }
    }
}

template <typename Real> 
inline void 
NBRE_AxisAlignedBox3<Real>::Merge(const NBRE_Vector3<Real>& point)
{
    Merge(point.x, point.y, point.z);
}

template <typename Real> 
inline void 
NBRE_AxisAlignedBox3<Real>::Merge(const NBRE_AxisAlignedBox3<Real>& box)
{
    Merge(box.minExtend);
    Merge(box.maxExtend);
}

template <typename Real> 
inline void 
NBRE_AxisAlignedBox3<Real>::Set(const NBRE_Vector3<Real>* points, uint32 pointsCount)
{
    if (pointsCount > 0)
    {
        minExtend = points[0];
        maxExtend = minExtend;
        for (uint32 i = 1; i < pointsCount; ++i)
        {
            Merge(points[i]);
        }
    }
}

template <typename Real> 
inline NBRE_Vector3<Real>
NBRE_AxisAlignedBox3<Real>::GetSize() const
{
    return maxExtend - minExtend;
}

template <typename Real> 
inline NBRE_Vector3<Real>
NBRE_AxisAlignedBox3<Real>::GetCenter() const
{
    return (maxExtend + minExtend) / 2;
}

template <typename Real> 
inline NBRE_Vector3<Real> 
NBRE_AxisAlignedBox3<Real>::GetClosestPointTo(const NBRE_Vector3<Real>& point) const
{
    NBRE_Vector3<Real> closestPoint(point);
    if (closestPoint.x < minExtend.x)
    {
        closestPoint.x = minExtend.x;
    }
    else if (closestPoint.x > maxExtend.x)
    {
        closestPoint.x = maxExtend.x;
    }

    if (closestPoint.y < minExtend.y)
    {
        closestPoint.y = minExtend.y;
    }
    else if (closestPoint.y > maxExtend.y)
    {
        closestPoint.y = maxExtend.y;
    }

    if (closestPoint.z < minExtend.z)
    {
        closestPoint.z = minExtend.z;
    }
    else if (closestPoint.z > maxExtend.z)
    {
        closestPoint.z = maxExtend.z;
    }

    return closestPoint;
}

template <typename Real> 
inline void 
NBRE_AxisAlignedBox3<Real>::GetCorners(NBRE_Vector3<Real>* p) const
{
    p[0].x = minExtend.x; p[0].y = minExtend.y; p[0].z = minExtend.z;
    p[1].x = maxExtend.x; p[1].y = minExtend.y; p[1].z = minExtend.z;
    p[2].x = maxExtend.x; p[2].y = maxExtend.y; p[2].z = minExtend.z;
    p[3].x = minExtend.x; p[3].y = maxExtend.y; p[3].z = minExtend.z;
    p[4].x = minExtend.x; p[4].y = minExtend.y; p[4].z = maxExtend.z;
    p[5].x = maxExtend.x; p[5].y = minExtend.y; p[5].z = maxExtend.z;
    p[6].x = maxExtend.x; p[6].y = maxExtend.y; p[6].z = maxExtend.z;
    p[7].x = minExtend.x; p[7].y = maxExtend.y; p[7].z = maxExtend.z;
}

template <typename Real> 
inline nb_boolean 
NBRE_AxisAlignedBox3<Real>::IsNull() const
{
    return !(minExtend.x <= maxExtend.x 
          && minExtend.y <= maxExtend.y 
          && minExtend.z <= maxExtend.z);
}

template <typename Real> 
inline void
NBRE_AxisAlignedBox3<Real>::SetNull()
{
    Real nan = NBRE_Math::NaN<Real>();
    minExtend.x = minExtend.y = minExtend.z = nan;
    maxExtend.x = maxExtend.y = maxExtend.z = nan;
}

template <typename Real> 
inline nb_boolean 
NBRE_AxisAlignedBox3<Real>::Contain(const NBRE_AxisAlignedBox3& box) const
{
    return minExtend.x <= box.minExtend.x && minExtend.y <= box.minExtend.y && minExtend.z <= box.minExtend.z
        && maxExtend.x >= box.maxExtend.x && maxExtend.y >= box.maxExtend.y && maxExtend.z >= box.maxExtend.z
        ? TRUE : FALSE;
}

template <typename Real> 
inline Real
NBRE_AxisAlignedBox3<Real>::DistanceTo(const NBRE_AxisAlignedBox3& box) const
{
    Real dx = minExtend.x - box.maxExtend.x;
    if (dx < 0)
    {
        dx = box.minExtend.x - maxExtend.x;
    }

    Real dy = minExtend.y - box.maxExtend.y;
    if (dy < 0)
    {
        dy = box.minExtend.y - maxExtend.y;
    }

    Real dz = minExtend.z - box.maxExtend.z;
    if (dz < 0)
    {
        dz = box.minExtend.z - maxExtend.z;
    }

    return static_cast<Real>(nsl_sqrt(
        (dx > 0 ? dx * dx : 0) +
        (dy > 0 ? dy * dy : 0) +
        (dz > 0 ? dz * dz : 0)
        ));
}
