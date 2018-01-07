/*!--------------------------------------------------------------------------

    @file nbreplane.inl
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
#include "palmath.h"

template <typename Real>
inline NBRE_Plane<Real>::NBRE_Plane ()
:normal(0, 1, 0), d(0)
{
}

template <typename Real>
inline NBRE_Plane<Real>::NBRE_Plane (const NBRE_Vector3<Real>& n, Real d)
:normal(n), d(d)
{
}

template <typename Real>
inline NBRE_Plane<Real>::NBRE_Plane (const NBRE_Vector3<Real>& n, const NBRE_Vector3<Real>& p)
:normal(n)
{
    normal.Normalise();
    d = -normal.DotProduct(p);
}

template <typename Real>
inline NBRE_Plane<Real>::NBRE_Plane (const NBRE_Vector3<Real>& p0, const NBRE_Vector3<Real>& p1, const NBRE_Vector3<Real>& p2)
{
    NBRE_Vector3<Real> edge1 = p1 - p0;
    NBRE_Vector3<Real> edge2 = p2 - p0;
    normal = edge1.CrossProduct(edge2);
    normal.Normalise();
    d = -normal.DotProduct(p0);
}

template <typename Real>
inline NBRE_Plane<Real>::~NBRE_Plane ()
{
}

template <typename Real>
inline nb_boolean 
NBRE_Plane<Real>::operator == (const NBRE_Plane<Real>& rhs) const
{
    return (normal == rhs.normal && d == rhs.d) ? TRUE : FALSE;
}

template <typename Real>
inline nb_boolean 
NBRE_Plane<Real>::operator != (const NBRE_Plane<Real>& rhs) const
{
    return (normal == rhs.normal && d == rhs.d) ? FALSE : TRUE;
}

template <typename Real>
inline Real 
NBRE_Plane<Real>::DistanceTo (const NBRE_Vector3<Real>& p) const
{
    return normal.DotProduct(p) + d;
}

template <typename Real>
inline NBRE_Vector3<Real> 
NBRE_Plane<Real>::GetClosestPointTo (const NBRE_Vector3<Real>& p) const
{
    return normal * -DistanceTo(p) + p;
}

template <typename Real>
inline NBRE_PlaneSide 
NBRE_Plane<Real>::GetSide (const NBRE_Vector3<Real>& p) const
{
    Real distance = DistanceTo(p);

    if (distance < 0)
    {
        return NBRE_PS_BACK_SIDE;
    }
    else if (distance > 0)
    {
        return NBRE_PS_FRONT_SIDE;
    }
    else
    {
        return NBRE_PS_SPANNING;
    }
}

template <typename Real>
inline void
NBRE_Plane<Real>::Normalize ()
{
    Real invLength = 1 / normal.Length();
    if (invLength == invLength)
    {
        normal *= invLength;
        d *= invLength;
    }
}

template class NBRE_Plane<float>;
template class NBRE_Plane<double>;
