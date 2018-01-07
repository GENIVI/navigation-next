/*!--------------------------------------------------------------------------

    @file nbreray2.inl
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
#include "paltypes.h"

template <typename Real>
inline NBRE_Ray2<Real>::NBRE_Ray2()
{
}

template <typename Real>
inline NBRE_Ray2<Real>::NBRE_Ray2(const NBRE_Vector2<Real>& origin, const NBRE_Vector2<Real>& direction)
:origin(origin)
,direction(direction)
{
    this->direction.Normalise();
}

template <typename Real>
inline NBRE_Ray2<Real>::~NBRE_Ray2()
{
}

template <typename Real>
inline void 
NBRE_Ray2<Real>::Reverse()
{
    direction *= -1;
}

template <typename Real>
inline nb_boolean 
NBRE_Ray2<Real>::IsParallel(const NBRE_Ray2<Real>& line, Real epsilon) const
{
    Real cosAngle = direction.DotProduct(line.direction);
    if (cosAngle < 0)
    {
        cosAngle = -cosAngle;
    }
    return 1 - cosAngle < epsilon ? TRUE : FALSE;
}

template <typename Real>
inline nb_boolean 
NBRE_Ray2<Real>::IsPerpendicular(const NBRE_Ray2<Real>& line, Real epsilon) const
{
    Real cosAngle = direction.DotProduct(line.direction);
    return cosAngle > -epsilon && cosAngle < epsilon ? TRUE : FALSE;
}

template <typename Real>
Real 
NBRE_Ray2<Real>::DistanceTo(const NBRE_Vector2<Real>& point) const
{
    NBRE_Vector2<Real>& startToPoint = point - origin;
    NBRE_Vector2<Real> closestPoint(origin);
    closestPoint += direction * startToPoint.DotProduct(direction);
    return closestPoint.DistanceTo(point);
}

template <typename Real>
NBRE_Vector2<Real> 
NBRE_Ray2<Real>::GetClosestPointTo(const NBRE_Vector2<Real>& point) const
{
    NBRE_Vector2<Real>& startToPoint = point - origin;
    NBRE_Vector2<Real> closestPoint(origin);
    closestPoint += startToPoint.DotProduct(direction);
    return closestPoint;
}

template <typename Real>
inline void 
NBRE_Ray2<Real>::Normalize()
{
    direction.Normalise();
}
