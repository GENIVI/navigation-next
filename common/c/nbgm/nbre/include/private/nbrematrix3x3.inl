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
#include "palmath.h"
#include "palstdlib.h"
#include "nbremath.h"

template <typename Real>
inline NBRE_Matrix3x3<Real>::NBRE_Matrix3x3()
{
    Identity();
}

template <typename Real>
inline NBRE_Matrix3x3<Real>::NBRE_Matrix3x3(
                           Real m00, Real m01, Real m02,
                           Real m10, Real m11, Real m12,
                           Real m20, Real m21, Real m22)
{
    mBuffer[0][0]  = m00;
    mBuffer[0][1]  = m01;
    mBuffer[0][2]  = m02;
    mBuffer[1][0]  = m10;
    mBuffer[1][1]  = m11;
    mBuffer[1][2]  = m12;
    mBuffer[2][0]  = m20;
    mBuffer[2][1]  = m21;
    mBuffer[2][2]  = m22;
}

template <typename Real>
inline NBRE_Matrix3x3<Real>::NBRE_Matrix3x3(const Real* m, nb_boolean isRowMajor)
{
    nsl_memcpy(mBuffer, m, sizeof(mBuffer));
    if (!isRowMajor)
    {
        Transpose();
    }
}

template <typename Real>
inline NBRE_Matrix3x3<Real>::NBRE_Matrix3x3(const Real m[3][3])
{
    nsl_memcpy(mBuffer, m, sizeof(mBuffer));
}

template <typename Real>
inline NBRE_Matrix3x3<Real>::NBRE_Matrix3x3(const NBRE_Matrix3x3<Real>& m1)
{
    nsl_memcpy(mBuffer, m1.mBuffer, sizeof(mBuffer));
}

template <typename Real>
inline NBRE_Matrix3x3<Real>::~NBRE_Matrix3x3()
{
}

template <typename Real>
inline NBRE_Matrix3x3<Real>& 
NBRE_Matrix3x3<Real>::operator = (const NBRE_Matrix3x3<Real>& m)
{
    if(&m != this)
    {
        nsl_memcpy(mBuffer, m.mBuffer, sizeof(mBuffer));         
    }
    return *this;
}

template <typename Real>
inline nb_boolean 
NBRE_Matrix3x3<Real>::operator == (const NBRE_Matrix3x3<Real>& m1) const
{
    if(nsl_memcmp(mBuffer, m1.mBuffer, sizeof(mBuffer)) == 0)
    {
        return TRUE;
    }
    return FALSE;
}

template <typename Real>
inline NBRE_Matrix3x3<Real> 
NBRE_Matrix3x3<Real>::operator + (const NBRE_Matrix3x3<Real>& m1) const
{
    NBRE_Matrix3x3<Real> result;

    for (uint32 row = 0; row < 3; ++row)
    {
        for (uint32 col = 0; col < 3; ++col)
        {
            result.mBuffer[row][col] = this->mBuffer[row][col] + m1.mBuffer[row][col];
        }
    }

    return result;
}

template <typename Real>
inline NBRE_Matrix3x3<Real>& 
NBRE_Matrix3x3<Real>::operator += (const NBRE_Matrix3x3<Real>& m1)
{
    for (uint32 row = 0; row < 3; ++row)
    {
        for (uint32 col = 0; col < 3; ++col)
        {
            this->mBuffer[row][col] = this->mBuffer[row][col] + m1.mBuffer[row][col];
        }
    }

    return *this;
}

template <typename Real>
inline NBRE_Matrix3x3<Real> 
NBRE_Matrix3x3<Real>::operator - (const NBRE_Matrix3x3<Real>& m1) const
{
    NBRE_Matrix3x3<Real> result;

    for (uint32 row = 0; row < 3; ++row)
    {
        for (uint32 col = 0; col < 3; ++col)
        {
            result.mBuffer[row][col] = this->mBuffer[row][col] - m1.mBuffer[row][col];
        }
    }

    return result;
}

template <typename Real>
inline NBRE_Matrix3x3<Real>& 
NBRE_Matrix3x3<Real>::operator -= (const NBRE_Matrix3x3<Real>& m1)
{
    for (uint32 row = 0; row< 3; ++row)
    {
        for (uint32 col = 0; col< 3; ++col)
        {
            this->mBuffer[row][col] = this->mBuffer[row][col] - m1.mBuffer[row][col];
        }
    }

    return *this;
}

template <typename Real>
inline NBRE_Matrix3x3<Real>& 
NBRE_Matrix3x3<Real>::operator - ()
{
    for (uint32 row = 0; row< 3; ++row)
    {
        for (uint32 col = 0; col< 3; ++col)
        {
            this->mBuffer[row][col] = -this->mBuffer[row][col];
        }
    }
    return *this;
}

template <typename Real>
inline NBRE_Matrix3x3<Real> 
NBRE_Matrix3x3<Real>::operator * (const NBRE_Matrix3x3<Real>& m1) const
{
    NBRE_Matrix3x3<Real> result;

    result.mBuffer[0][0] = mBuffer[0][0] * m1.mBuffer[0][0] + mBuffer[0][1] * m1.mBuffer[1][0] + mBuffer[0][2] * m1.mBuffer[2][0];
    result.mBuffer[0][1] = mBuffer[0][0] * m1.mBuffer[0][1] + mBuffer[0][1] * m1.mBuffer[1][1] + mBuffer[0][2] * m1.mBuffer[2][1];
    result.mBuffer[0][2] = mBuffer[0][0] * m1.mBuffer[0][2] + mBuffer[0][1] * m1.mBuffer[1][2] + mBuffer[0][2] * m1.mBuffer[2][2];

    result.mBuffer[1][0] = mBuffer[1][0] * m1.mBuffer[0][0] + mBuffer[1][1] * m1.mBuffer[1][0] + mBuffer[1][2] * m1.mBuffer[2][0];
    result.mBuffer[1][1] = mBuffer[1][0] * m1.mBuffer[0][1] + mBuffer[1][1] * m1.mBuffer[1][1] + mBuffer[1][2] * m1.mBuffer[2][1];
    result.mBuffer[1][2] = mBuffer[1][0] * m1.mBuffer[0][2] + mBuffer[1][1] * m1.mBuffer[1][2] + mBuffer[1][2] * m1.mBuffer[2][2];

    result.mBuffer[2][0] = mBuffer[2][0] * m1.mBuffer[0][0] + mBuffer[2][1] * m1.mBuffer[1][0] + mBuffer[2][2] * m1.mBuffer[2][0];
    result.mBuffer[2][1] = mBuffer[2][0] * m1.mBuffer[0][1] + mBuffer[2][1] * m1.mBuffer[1][1] + mBuffer[2][2] * m1.mBuffer[2][1];
    result.mBuffer[2][2] = mBuffer[2][0] * m1.mBuffer[0][2] + mBuffer[2][1] * m1.mBuffer[1][2] + mBuffer[2][2] * m1.mBuffer[2][2];

    return result;
}

template <typename Real>
inline NBRE_Matrix3x3<Real>& 
NBRE_Matrix3x3<Real>::operator *= (const NBRE_Matrix3x3<Real>& m1)
{
    *this = *this * m1;
    return *this;
}

template <typename Real>
inline NBRE_Vector3<Real>
NBRE_Matrix3x3<Real>::operator * (const NBRE_Vector3<Real>& vec) const
{
    return NBRE_Vector3<Real>(mBuffer[0][0]*vec.x + mBuffer[0][1]*vec.y + mBuffer[0][2]*vec.z,
                              mBuffer[1][0]*vec.x + mBuffer[1][1]*vec.y + mBuffer[1][2]*vec.z,
                              mBuffer[2][0]*vec.x + mBuffer[2][1]*vec.y + mBuffer[2][2]*vec.z);
}

template <typename Real>
inline NBRE_Vector2<Real> 
NBRE_Matrix3x3<Real>::operator * (const NBRE_Vector2<Real>& v) const
{
    Real fInvW = static_cast<Real>(1.0 / (mBuffer[2][0] + mBuffer[2][1] + mBuffer[2][2]));
    Real x = ( mBuffer[0][0] * v.x + mBuffer[0][1] * v.y + mBuffer[0][2] ) * fInvW;
    Real y = ( mBuffer[1][0] * v.x + mBuffer[1][1] * v.y + mBuffer[1][2] ) * fInvW;
    Real z = ( mBuffer[2][0] * v.x + mBuffer[2][1] * v.y + mBuffer[2][2] ) * fInvW;   
    return NBRE_Vector2<Real>(x, y);
}

template <typename Real>
inline const Real* 
NBRE_Matrix3x3<Real>::operator [] (uint8 row) const
{
    nbre_assert(row< 3);
    return mBuffer[row];
}

template <typename Real>
inline Real* 
NBRE_Matrix3x3<Real>::operator [] (uint8 row)
{
    nbre_assert(row< 3);
    return mBuffer[row];
}

template <typename Real>
inline void 
NBRE_Matrix3x3<Real>::LoadFrom(const Real* buffer, nb_boolean isRowMajor)
{
    nsl_memcpy(mBuffer, buffer, sizeof(mBuffer));
    if (!isRowMajor)
    {
        Transpose();
    }
}

template <typename Real>
inline void 
NBRE_Matrix3x3<Real>::CopyTo(Real* buffer, nb_boolean isRowMajor) const
{
    if (isRowMajor)
    {
        nsl_memcpy(buffer, mBuffer, sizeof(mBuffer));
    }
    else
    {
        for (uint32 col = 0; col< 3; ++col)
        {
            for (uint32 row = 0; row< 3; ++row)
            {
                *buffer++ = mBuffer[row][col];
            }
        }
    }
}

template <typename Real>
inline void 
NBRE_Matrix3x3<Real>::Identity()
{
    nsl_memset(mBuffer, 0, sizeof(mBuffer));
    mBuffer[0][0] = mBuffer[1][1] = mBuffer[2][2] = 1;
}

template <typename Real>
inline void 
NBRE_Matrix3x3<Real>::Transpose()
{
    for (uint32 row = 0; row< 3; ++row) 
    {
        for(uint32 col = row; col< 3; ++col)
        {
            if (row != col)
            {
                Real temp = mBuffer[row][col];
                mBuffer[row][col] = mBuffer[col][row];
                mBuffer[col][row] = temp;
            }
        }
    }
}

template <typename Real>
inline void
NBRE_Matrix3x3<Real>::Zero()
{
    nsl_memset(mBuffer, 0, sizeof(mBuffer));
}

template <typename Real>
inline NBRE_Matrix3x3<Real>
NBRE_Matrix3x3<Real>::Inverse() const
{
    NBRE_Matrix3x3<Real> rkInverse;

    rkInverse[0][0] = mBuffer[1][1]*mBuffer[2][2] - mBuffer[1][2]*mBuffer[2][1];
    rkInverse[0][1] = mBuffer[0][2]*mBuffer[2][1] - mBuffer[0][1]*mBuffer[2][2];
    rkInverse[0][2] = mBuffer[0][1]*mBuffer[1][2] - mBuffer[0][2]*mBuffer[1][1];
    rkInverse[1][0] = mBuffer[1][2]*mBuffer[2][0] - mBuffer[1][0]*mBuffer[2][2];
    rkInverse[1][1] = mBuffer[0][0]*mBuffer[2][2] - mBuffer[0][2]*mBuffer[2][0];
    rkInverse[1][2] = mBuffer[0][2]*mBuffer[1][0] - mBuffer[0][0]*mBuffer[1][2];
    rkInverse[2][0] = mBuffer[1][0]*mBuffer[2][1] - mBuffer[1][1]*mBuffer[2][0];
    rkInverse[2][1] = mBuffer[0][1]*mBuffer[2][0] - mBuffer[0][0]*mBuffer[2][1];
    rkInverse[2][2] = mBuffer[0][0]*mBuffer[1][1] - mBuffer[0][1]*mBuffer[1][0];

    Real det =
        mBuffer[0][0]*rkInverse[0][0]+
        mBuffer[0][1]*rkInverse[1][0]+
        mBuffer[0][2]*rkInverse[2][0];
    if ( det == 0 )
    {
        return NBRE_Matrix3x3<Real>();
    }
    Real fInvDet = 1 / det;

    for (uint32 row = 0; row < 3; row++)
    {
        for (uint32 col = 0; col < 3; col++)
        {
            rkInverse[row][col] *= fInvDet;
        }   
    }
}
