/*!--------------------------------------------------------------------------

    @file nbresegment2.inl
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
inline NBRE_Segment2<Real>::NBRE_Segment2()
{
}

template <typename Real>
inline NBRE_Segment2<Real>::NBRE_Segment2(Real xStart, Real yStart, Real xEnd, Real yEnd)
:startPoint(xStart, yStart)
,endPoint(xEnd, yEnd)
{
}

template <typename Real>
inline NBRE_Segment2<Real>::NBRE_Segment2(const NBRE_Vector2<Real>& start, const NBRE_Vector2<Real>& end)
:startPoint(start)
,endPoint(end)
{
}

template <typename Real>
inline NBRE_Segment2<Real>::~NBRE_Segment2()
{
}

template <typename Real>
inline Real 
NBRE_Segment2<Real>::Length() const
{
    return startPoint.DistanceTo(endPoint);
}

template <typename Real>
inline Real 
NBRE_Segment2<Real>::SquaredLength() const
{
    return startPoint.SquaredDistanceTo(endPoint);
}

template <typename Real>
inline NBRE_Vector2<Real> 
NBRE_Segment2<Real>::GetDirection() const
{
    return endPoint - startPoint;
}

template <typename Real>
inline void 
NBRE_Segment2<Real>::Reverse()
{
    NBRE_Vector2<Real> temp(startPoint);
    startPoint = endPoint;
    endPoint = temp;
}

template <typename Real>
inline nb_boolean 
NBRE_Segment2<Real>::IsParallel(const NBRE_Segment2<Real>& line, Real epsilon) const
{
    Real cosAngle = GetDirection().DotProduct(line.GetDirection());
    if (cosAngle < 0)
    {
        cosAngle = -cosAngle;
    }
    return 1 - cosAngle < epsilon ? TRUE : FALSE;
}

template <typename Real>
inline nb_boolean 
NBRE_Segment2<Real>::IsPerpendicular(const NBRE_Segment2<Real>& line, Real epsilon) const
{
    Real cosAngle = GetDirection().DotProduct(line.GetDirection());
    return cosAngle > -epsilon && cosAngle < epsilon ? TRUE : FALSE;
}

template <typename Real>
inline NBRE_Vector2<Real> 
NBRE_Segment2<Real>::GetClosestPointTo(const NBRE_Vector2<Real>& point) const
{
    if (startPoint == endPoint)
    {
        // Zero segment
        return startPoint;
    }
    else
    {
        const NBRE_Vector2<Real>& startToPoint = point - startPoint;
        NBRE_Vector2<Real> direction(endPoint - startPoint);
        direction.Normalise();
        Real t = startToPoint.DotProduct(direction);
        // clamp to segment range
        if (t < 0)
        {
            t = 0;
        }
        else if (t > 1)
        {
            t = 1;
        }
        return startPoint + direction * t;
    }
}

template <typename Real>
inline Real 
NBRE_Segment2<Real>::DistanceTo(const NBRE_Vector2<Real>& point) const
{
    return point.DistanceTo(GetClosestPointTo(point));
}

template <typename Real>
inline Real
NBRE_Segment2<Real>::GetClosestT(const NBRE_Vector2<Real>& point) const
{
    Real t = 0;
    if (startPoint == endPoint)
    {
        // Zero segment
        return t;
    }
    else
    {
        const NBRE_Vector2<Real>& startToPoint = point - startPoint;
        NBRE_Vector2<Real> direction(endPoint - startPoint);
        direction.Normalise();
        t = startToPoint.DotProduct(direction);
        // clamp to segment range
        if (t < 0)
        {
            t = 0;
        }
        else if (t > 1)
        {
            t = 1;
        }
        return t;
    }
}
