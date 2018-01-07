/*!--------------------------------------------------------------------------

    @file nbrevector3.inl
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
NBRE_Vector3<Real>::NBRE_Vector3()
:x(0),y(0),z(0) 
{
}

template <typename Real>
NBRE_Vector3<Real>::NBRE_Vector3(Real x, Real y, Real z)
:x(x), y(y), z(z) 
{
}

template <typename Real>
NBRE_Vector3<Real>::NBRE_Vector3(const Real* v)
:x(v[0]), y(v[1]), z(v[2]) 
{
}

template <typename Real>
NBRE_Vector3<Real>::NBRE_Vector3(const NBRE_Vector3<Real>& vec)
:x(vec.x), y(vec.y), z(vec.z) 
{

}

template <typename Real>
NBRE_Vector3<Real>::NBRE_Vector3(const NBRE_Vector2<Real>& vec)
:x(vec.x), y(vec.y), z(0) 
{
}

template <typename Real>
NBRE_Vector3<Real>::~NBRE_Vector3()
{
}

template <typename Real> 
inline NBRE_Vector3<Real>& 
NBRE_Vector3<Real>::operator = (const NBRE_Vector3<Real>& vec)
{
    if(&vec != this)
    {
        x = vec.x;
        y = vec.y;
        z = vec.z;
    }
    return *this;
}

template <typename Real> 
inline nb_boolean 
NBRE_Vector3<Real>::operator == (const NBRE_Vector3<Real>& vec) const
{
    return (x == vec.x && y == vec.y && z == vec.z) ? TRUE : FALSE;
}

template <typename Real> 
inline nb_boolean 
NBRE_Vector3<Real>::operator != (const NBRE_Vector3<Real>& vec) const
{
    return (x == vec.x && y == vec.y && z == vec.z) ? FALSE : TRUE;
}

template <typename Real> 
inline NBRE_Vector3<Real> 
NBRE_Vector3<Real>::operator - () const
{
    return NBRE_Vector3<Real>(-x, -y, -z);
}

template <typename Real> 
inline NBRE_Vector3<Real> 
NBRE_Vector3<Real>::operator + (const NBRE_Vector3<Real>& vec) const
{
    return NBRE_Vector3<Real>(x + vec.x, y + vec.y, z + vec.z);
}

template <typename Real> 
inline NBRE_Vector3<Real> 
NBRE_Vector3<Real>::operator - (const NBRE_Vector3<Real>& vec) const
{
    return NBRE_Vector3<Real>(x - vec.x, y - vec.y, z - vec.z);
}

template <typename Real> 
inline NBRE_Vector3<Real> 
NBRE_Vector3<Real>::operator * (Real scalar) const
{
    return NBRE_Vector3<Real>(x * scalar, y * scalar, z * scalar);
}

template <typename Real> 
inline NBRE_Vector3<Real> 
NBRE_Vector3<Real>::operator / (Real scalar) const
{
    return NBRE_Vector3<Real>(x / scalar, y / scalar, z / scalar);
}

template <typename Real> 
inline NBRE_Vector3<Real>& 
NBRE_Vector3<Real>::operator += (const NBRE_Vector3<Real>& vec)
{
    x += vec.x;
    y += vec.y;
    z += vec.z;
    return *this;
}

template <typename Real> 
inline NBRE_Vector3<Real>& 
NBRE_Vector3<Real>::operator -= (const NBRE_Vector3<Real>& vec)
{
    x -= vec.x;
    y -= vec.y;
    z -= vec.z;
    return *this;
}

template <typename Real> 
inline NBRE_Vector3<Real>& 
NBRE_Vector3<Real>::operator *= (Real scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

template <typename Real> 
inline NBRE_Vector3<Real>& 
NBRE_Vector3<Real>::operator /= (Real scalar)
{
    x /= scalar;
    y /= scalar;
    z /= scalar;
    return *this;
}

template <typename Real> 
inline Real 
NBRE_Vector3<Real>::Length() const
{
    Real lq = x * x + y * y + z * z;
    return nsl_sqrt(lq);
}

template <typename Real> 
inline Real 
NBRE_Vector3<Real>::SquaredLength() const
{
    return x * x + y * y + z * z;
}

template <typename Real> 
inline void 
NBRE_Vector3<Real>::Normalise()
{
    Real length = Length();
    if (length != 0.0f)
    {
        Real inverseLength = static_cast<Real>(1.0 / length);
        x *= inverseLength;
        y *= inverseLength;
        z *= inverseLength;
    }
}

template <typename Real> 
inline Real 
NBRE_Vector3<Real>::DotProduct(const NBRE_Vector3& vec) const
{
    return x * vec.x + y * vec.y + z * vec.z;
}

template <typename Real> 
inline NBRE_Vector3<Real> 
NBRE_Vector3<Real>::CrossProduct(const NBRE_Vector3<Real>& p) const
{
    return NBRE_Vector3<Real>(y * p.z - z * p.y, z * p.x - x * p.z, x * p.y - y * p.x);
}

template <typename Real> 
inline Real 
NBRE_Vector3<Real>::DistanceTo(const NBRE_Vector3<Real>& vec) const
{
    Real dx = x - vec.x;
    Real dy = y - vec.y;
    Real dz = z - vec.z;
    return nsl_sqrt(dx * dx + dy * dy + dz * dz);
}

template <typename Real> 
inline Real 
NBRE_Vector3<Real>::SquaredDistanceTo(const NBRE_Vector3<Real>& vec) const
{
    Real dx = x - vec.x;
    Real dy = y - vec.y;
    Real dz = z - vec.z;
    return dx * dx + dy * dy + dz * dz;
}

template <typename Real> 
inline nb_boolean
NBRE_Vector3<Real>::IsNaN() const
{
    return (x != x || y != y || z != z) ? TRUE : FALSE;
}

template <typename Real>
inline NBRE_Vector3<Real> 
operator* (Real scalar, const NBRE_Vector3<Real>& vec)
{
    return NBRE_Vector3<Real>(vec.x * scalar, vec.y * scalar, vec.z * scalar);
}
