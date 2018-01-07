/*!--------------------------------------------------------------------------

    @file nbrevector2.inl
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
#include "nbremath.h"

template <typename Real>
inline NBRE_Vector2<Real>::NBRE_Vector2(void)
:x(0),y(0)
{
}

template <typename Real>
inline NBRE_Vector2<Real>::NBRE_Vector2(Real x, Real y)
:x(x), y(y)
{
}

template <typename Real>
inline NBRE_Vector2<Real>::NBRE_Vector2(const Real* v)
:x(v[0]), y(v[1]) 
{
}

template <typename Real>
inline NBRE_Vector2<Real>::NBRE_Vector2(const NBRE_Vector2<Real>& vec)
:x(vec.x), y(vec.y)
{
}

template <typename Real>
inline NBRE_Vector2<Real>::~NBRE_Vector2(void)
{
}

template <typename Real> 
inline NBRE_Vector2<Real>& 
NBRE_Vector2<Real>::operator = (const NBRE_Vector2<Real>& vec)
{
    if(&vec != this)
    {
        x = vec.x;
        y = vec.y;
    }
    return *this;
}

template <typename Real> 
inline nb_boolean 
NBRE_Vector2<Real>::operator == (const NBRE_Vector2<Real>& vec) const
{
    return (x == vec.x && y == vec.y) ? TRUE : FALSE;
}

template <typename Real> 
inline nb_boolean 
NBRE_Vector2<Real>::operator != (const NBRE_Vector2<Real>& vec) const
{
    return (x == vec.x && y == vec.y) ? FALSE : TRUE;
}

template <typename Real> 
inline NBRE_Vector2<Real> 
NBRE_Vector2<Real>::operator - () const
{
    return NBRE_Vector2<Real>(-x, -y);
}

template <typename Real> 
inline NBRE_Vector2<Real> 
NBRE_Vector2<Real>::operator + (const NBRE_Vector2<Real>& vec) const
{
    return NBRE_Vector2<Real>(x + vec.x, y + vec.y);
}

template <typename Real> 
inline NBRE_Vector2<Real> 
NBRE_Vector2<Real>::operator - (const NBRE_Vector2<Real>& vec) const
{
    return NBRE_Vector2<Real>(x - vec.x, y - vec.y);
}

template <typename Real> 
inline NBRE_Vector2<Real> 
NBRE_Vector2<Real>::operator * (Real scalar) const
{
    return NBRE_Vector2<Real>(x * scalar, y * scalar);
}

template <typename Real> 
inline NBRE_Vector2<Real> 
NBRE_Vector2<Real>::operator / (Real scalar) const
{
    return NBRE_Vector2<Real>(x / scalar, y / scalar);
}

template <typename Real> 
inline NBRE_Vector2<Real>& 
NBRE_Vector2<Real>::operator += (const NBRE_Vector2<Real>& vec)
{
    x += vec.x;
    y += vec.y;
    return *this;
}

template <typename Real> 
inline NBRE_Vector2<Real>& 
NBRE_Vector2<Real>::operator -= (const NBRE_Vector2<Real>& vec)
{
    x -= vec.x;
    y -= vec.y;
    return *this;
}

template <typename Real> 
inline NBRE_Vector2<Real>& 
NBRE_Vector2<Real>::operator *= (Real scalar)
{
    x *= scalar;
    y *= scalar;
    return *this;
}

template <typename Real> 
inline NBRE_Vector2<Real>& 
NBRE_Vector2<Real>::operator /= (Real scalar)
{
    x /= scalar;
    y /= scalar;
    return *this;
}

template <typename Real> 
inline Real 
NBRE_Vector2<Real>::Length() const
{
    Real lq = x * x + y * y;
    return nsl_sqrt(lq);
}

template <typename Real> 
inline Real 
NBRE_Vector2<Real>::SquaredLength() const
{
    return x * x + y * y;
}

template <typename Real> 
inline void 
NBRE_Vector2<Real>::Normalise()
{
    Real length = Length();
    if (length != 0.0f)
    {
        Real inverseLength = 1.0f / length;
        x *= inverseLength;
        y *= inverseLength;
    }
}

template <typename Real> 
inline Real 
NBRE_Vector2<Real>::DotProduct(const NBRE_Vector2& vec) const
{
    return x * vec.x + y * vec.y;
}

template <typename Real> 
inline Real 
NBRE_Vector2<Real>::CrossProduct(const NBRE_Vector2& vec) const
{
    return x*vec.y - y*vec.x;
}

template <typename Real> 
inline NBRE_Vector2<Real> 
NBRE_Vector2<Real>::Perpendicular() const
{
    return NBRE_Vector2<Real>(-y, x);
}

template <typename Real> 
inline Real 
NBRE_Vector2<Real>::DistanceTo(const NBRE_Vector2<Real>& vec) const
{
    Real dx = x - vec.x;
    Real dy = y - vec.y;
    return static_cast<Real>(nsl_sqrt(static_cast<double>(dx * dx + dy * dy)));
}

template <typename Real> 
inline Real 
NBRE_Vector2<Real>::SquaredDistanceTo(const NBRE_Vector2<Real>& vec) const
{
    Real dx = x - vec.x;
    Real dy = y - vec.y;
    return dx * dx + dy * dy;
}

template <typename Real> 
inline nb_boolean
NBRE_Vector2<Real>::IsNaN() const
{
    return x != x || y != y ? TRUE : FALSE;
}

template <typename Real>
inline NBRE_Vector2<Real> 
operator* (Real scalar, const NBRE_Vector2<Real>& vec)
{
    return NBRE_Vector2<Real>(vec.x * scalar, vec.y * scalar);
}

template <typename Real>
inline Real
NBRE_Vector2<Real>::GetDirection() const
{
    if(NBRE_Math::IsZero(static_cast<float>(x), 1e-5f) && NBRE_Math::IsZero(static_cast<float>(y), 1e-5f))
    {
        return 0.0;
    }

    Real tempX = static_cast<Real>(nsl_fabs(x));
    Real tempY = static_cast<Real>(nsl_fabs(y));
    Real tan = 0.0;

    if(tempX > tempY)
    {
        tan = tempY / tempX;
    }
    else
    {
        tan = tempX / tempY;
    }

    Real theta = static_cast<Real>(NBRE_Math::RadToDeg(static_cast<Real>(nsl_atan(tan))));

    if(tempX < tempY)
    {
        theta = static_cast<Real>(90.0 - theta);
    }
    if(x < 0)
    {
        theta = static_cast<Real>(180.0 - theta);
    }
    if(y < 0)
    {
        theta = static_cast<Real>(360.0 - theta);
    }

    return theta;
}