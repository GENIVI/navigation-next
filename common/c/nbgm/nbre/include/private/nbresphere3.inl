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

template <typename Real> 
inline
NBRE_Sphere3<Real>::NBRE_Sphere3():
radius(0)
{
}

template <typename Real> 
inline
NBRE_Sphere3<Real>::NBRE_Sphere3(Real x, Real y, Real z, Real r)
:center(x, y, z)
,radius(r)
{
}

template <typename Real> 
inline
NBRE_Sphere3<Real>::~NBRE_Sphere3()
{
}

template <typename Real> 
inline NBRE_Vector3<Real> 
NBRE_Sphere3<Real>::GetClosestPointTo(const NBRE_Vector3<Real>& point) const
{
    Real distance = point.DistanceTo(center);
    if (distance <= radius)
    {
        return point;
    }
    else
    {
        Real t = radius / distance;
        return center * (1 - t) + point * t;
    }
}
