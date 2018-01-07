/*!--------------------------------------------------------------------------

    @file nbrevector4.inl
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
NBRE_Vector4<Real>::NBRE_Vector4()
:x(0),y(0),z(0), w(0) 
{
}

template <typename Real>
NBRE_Vector4<Real>::NBRE_Vector4(Real x, Real y, Real z, Real w)
:x(x), y(y), z(z), w(w) 
{
}

template <typename Real>
NBRE_Vector4<Real>::NBRE_Vector4(const Real* v)
:x(v[0]), y(v[1]), z(v[2]), w(v[3]) 
{
}

template <typename Real>
NBRE_Vector4<Real>::NBRE_Vector4(const NBRE_Vector4<Real>& vec)
:x(vec.x), y(vec.y), z(vec.z), w(vec.w) 
{

}

template <typename Real>
NBRE_Vector4<Real>::NBRE_Vector4(const NBRE_Vector3<Real>& rec3, Real s)
    :x(rec3.x), y(rec3.y), z(rec3.z), w(s)
{

}

template <typename Real>
NBRE_Vector4<Real>::NBRE_Vector4(const NBRE_Vector3<Real>& vec)
:x(vec.x), y(vec.y), z(vec.z), w(1) 
{
}

template <typename Real>
NBRE_Vector4<Real>::~NBRE_Vector4()
{
}

template <typename Real> 
inline NBRE_Vector4<Real>& 
NBRE_Vector4<Real>::operator = (const NBRE_Vector4<Real>& vec)
{
    if(&vec != this)
    {
        x = vec.x;
        y = vec.y;
        z = vec.z;
        w = vec.w;
    }
    return *this;
}

template <typename Real> 
inline nb_boolean 
NBRE_Vector4<Real>::operator == (const NBRE_Vector4<Real>& vec) const
{
    return (x == vec.x && y == vec.y && z == vec.z && w == vec.w) ? TRUE : FALSE;
}

template <typename Real> 
inline nb_boolean 
NBRE_Vector4<Real>::operator != (const NBRE_Vector4<Real>& vec) const
{
    return (x == vec.x && y == vec.y && z == vec.z && w == vec.w) ? FALSE : TRUE;
}

template <typename Real> 
inline NBRE_Vector4<Real> 
NBRE_Vector4<Real>::operator - () const
{
    return NBRE_Vector4<Real>(-x, -y, -z, -w);
}

template <typename Real> 
inline NBRE_Vector4<Real> 
NBRE_Vector4<Real>::operator + (const NBRE_Vector4<Real>& vec) const
{
    return NBRE_Vector4<Real>(x + vec.x, y + vec.y, z + vec.z, w + vec.w);
}

template <typename Real> 
inline NBRE_Vector4<Real> 
NBRE_Vector4<Real>::operator - (const NBRE_Vector4<Real>& vec) const
{
    return NBRE_Vector4<Real>(x - vec.x, y - vec.y, z - vec.z, w - vec.w);
}

template <typename Real> 
inline NBRE_Vector4<Real> 
NBRE_Vector4<Real>::operator * (Real scalar) const
{
    return NBRE_Vector4<Real>(x * scalar, y * scalar, z * scalar, w * scalar);
}

template <typename Real> 
inline NBRE_Vector4<Real> 
NBRE_Vector4<Real>::operator / (Real scalar) const
{
    return NBRE_Vector4<Real>(x / scalar, y / scalar, z / scalar, w / scalar);
}

template <typename Real> 
inline NBRE_Vector4<Real>& 
NBRE_Vector4<Real>::operator += (const NBRE_Vector4<Real>& vec)
{
    x += vec.x;
    y += vec.y;
    z += vec.z;
    w += vec.w;
    return *this;
}

template <typename Real> 
inline NBRE_Vector4<Real>& 
NBRE_Vector4<Real>::operator -= (const NBRE_Vector4<Real>& vec)
{
    x -= vec.x;
    y -= vec.y;
    z -= vec.z;
    w -= vec.w;
    return *this;
}

template <typename Real> 
inline NBRE_Vector4<Real>& 
NBRE_Vector4<Real>::operator *= (Real scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
    return *this;
}

template <typename Real> 
inline NBRE_Vector4<Real>& 
NBRE_Vector4<Real>::operator /= (Real scalar)
{
    x /= scalar;
    y /= scalar;
    z /= scalar;
    w /= scalar;
    return *this;
}

template <typename Real> 
inline Real 
NBRE_Vector4<Real>::DotProduct(const NBRE_Vector4& vec) const
{
    return x * vec.x + y * vec.y + z * vec.z + w * vec.w;
}

template <typename Real> 
inline nb_boolean
NBRE_Vector4<Real>::IsNaN() const
{
    return (x != x || y != y || z != z || w != w) ? TRUE : FALSE;
}

template <typename Real>
inline NBRE_Vector4<Real> 
operator* (Real scalar, const NBRE_Vector4<Real>& vec)
{
    return NBRE_Vector4<Real>(vec.x * scalar, vec.y * scalar, vec.z * scalar, vec.w * scalar);
}
