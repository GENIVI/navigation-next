/*!--------------------------------------------------------------------------

    @file nbresegment3.inl
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
inline NBRE_Segment3<Real>::NBRE_Segment3()
{
}

template <typename Real>
inline NBRE_Segment3<Real>::NBRE_Segment3(Real xStart, Real yStart, Real zStart, Real xEnd, Real yEnd, Real zEnd)
:startPoint(xStart, yStart, zStart)
,endPoint(xEnd, yEnd, zEnd)
{
}

template <typename Real>
inline NBRE_Segment3<Real>::NBRE_Segment3(const NBRE_Vector3<Real>& start, const NBRE_Vector3<Real>& end)
:startPoint(start)
,endPoint(end)
{
}

template <typename Real>
inline NBRE_Segment3<Real>::~NBRE_Segment3()
{
}

template <typename Real>
inline Real 
NBRE_Segment3<Real>::Length() const
{
    return startPoint.DistanceTo(endPoint);
}

template <typename Real>
inline Real 
NBRE_Segment3<Real>::SquaredLength() const
{
    return startPoint.SquaredDistanceTo(endPoint);
}

template <typename Real>
inline NBRE_Vector3<Real> 
NBRE_Segment3<Real>::GetDirection() const
{
    NBRE_Vector3<Real> result(endPoint - startPoint);
    result.Normalise();
    return result;
}

template <typename Real>
inline void 
NBRE_Segment3<Real>::Reverse()
{
    NBRE_Vector3<Real> temp(startPoint);
    startPoint = endPoint;
    endPoint = temp;
}

template <typename Real>
inline nb_boolean 
NBRE_Segment3<Real>::IsParallel(const NBRE_Segment3<Real>& line, Real epsilon) const
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
NBRE_Segment3<Real>::IsPerpendicular(const NBRE_Segment3<Real>& line, Real epsilon) const
{
    Real cosAngle = GetDirection().DotProduct(line.GetDirection());
    return cosAngle > -epsilon && cosAngle < epsilon ? TRUE : FALSE;
}

template <typename Real>
inline NBRE_Vector3<Real> 
NBRE_Segment3<Real>::GetClosestPointTo(const NBRE_Vector3<Real>& point) const
{
    if (startPoint == endPoint)
    {
        // Zero segment
        return startPoint;
    }
    else
    {
        const NBRE_Vector3<Real>& startToPoint = point - startPoint;
        NBRE_Vector3<Real> direction(endPoint - startPoint);
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
Real 
NBRE_Segment3<Real>::DistanceTo(const NBRE_Vector3<Real>& point) const
{
    point.DistanceTo(GetClosestPointTo(point));
}

template <typename Real>
inline Real 
NBRE_Segment3<Real>::GetClosestT(const NBRE_Vector3<Real>& point) const
{
    Real t = 0;
    if (startPoint == endPoint)
    {
        // Zero segment
        return t;
    }
    else
    {
        const NBRE_Vector3<Real>& startToPoint = point - startPoint;
        NBRE_Vector3<Real> direction(endPoint - startPoint);
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
