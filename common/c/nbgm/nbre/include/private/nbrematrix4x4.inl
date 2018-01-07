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
inline NBRE_Matrix4x4<Real>::NBRE_Matrix4x4()
{
    Identity();
}

template <typename Real>
inline NBRE_Matrix4x4<Real>::NBRE_Matrix4x4(
                           Real m00, Real m01, Real m02, Real m03,
                           Real m10, Real m11, Real m12, Real m13,
                           Real m20, Real m21, Real m22, Real m23,
                           Real m30, Real m31, Real m32, Real m33)
{
    mBuffer[0][0]  = m00;
    mBuffer[0][1]  = m01;
    mBuffer[0][2]  = m02;
    mBuffer[0][3]  = m03;
    mBuffer[1][0]  = m10;
    mBuffer[1][1]  = m11;
    mBuffer[1][2]  = m12;
    mBuffer[1][3]  = m13;
    mBuffer[2][0]  = m20;
    mBuffer[2][1]  = m21;
    mBuffer[2][2]  = m22;
    mBuffer[2][3]  = m23;
    mBuffer[3][0]  = m30;
    mBuffer[3][1]  = m31;
    mBuffer[3][2]  = m32;
    mBuffer[3][3]  = m33;
}

template <typename Real>
inline NBRE_Matrix4x4<Real>::NBRE_Matrix4x4(const Real* m, nb_boolean isRowMajor)
{
    nsl_memcpy(mBuffer, m, sizeof(mBuffer));
    if (!isRowMajor)
    {
        Transpose();
    }
}

template <typename Real>
inline NBRE_Matrix4x4<Real>::NBRE_Matrix4x4(const Real m[4][4])
{
    nsl_memcpy(mBuffer, m, sizeof(mBuffer));
}

template <typename Real>
inline NBRE_Matrix4x4<Real>::NBRE_Matrix4x4(const NBRE_Matrix4x4<Real>& m1)
{
    nsl_memcpy(mBuffer, m1.mBuffer, sizeof(mBuffer));
}

template <typename Real>
inline NBRE_Matrix4x4<Real>::~NBRE_Matrix4x4()
{
}

template <typename Real>
inline NBRE_Matrix4x4<Real>& 
NBRE_Matrix4x4<Real>::operator = (const NBRE_Matrix4x4<Real>& m4)
{
    if(&m4 != this)
    {
        nsl_memcpy(mBuffer, m4.mBuffer, sizeof(mBuffer));         
    }
    return *this;
}

template <typename Real>
inline nb_boolean 
NBRE_Matrix4x4<Real>::operator == (const NBRE_Matrix4x4<Real>& m1) const
{
    if(nsl_memcmp(mBuffer, m1.mBuffer, sizeof(mBuffer)) == 0)
    {
        return TRUE;
    }
    return FALSE;
}

template <typename Real>
inline NBRE_Matrix4x4<Real> 
NBRE_Matrix4x4<Real>::operator + (const NBRE_Matrix4x4<Real>& m1) const
{
    NBRE_Matrix4x4<Real> result;

    for (uint16 row = 0; row < 4; ++row)
    {
        for (uint16 col = 0; col < 4; ++col)
        {
            result.mBuffer[row][col] = this->mBuffer[row][col] + m1.mBuffer[row][col];
        }
    }

    return result;
}

template <typename Real>
inline NBRE_Matrix4x4<Real>& 
NBRE_Matrix4x4<Real>::operator += (const NBRE_Matrix4x4<Real>& m1)
{
    for (uint16 row = 0; row < 4; ++row)
    {
        for (uint16 col = 0; col < 4; ++col)
        {
            this->mBuffer[row][col] = this->mBuffer[row][col] + m1.mBuffer[row][col];
        }
    }

    return *this;
}

template <typename Real>
inline NBRE_Matrix4x4<Real> 
NBRE_Matrix4x4<Real>::operator - (const NBRE_Matrix4x4<Real>& m1) const
{
    NBRE_Matrix4x4<Real> result;

    for (uint16 row = 0; row < 4; ++row)
    {
        for (uint16 col = 0; col < 4; ++col)
        {
            result.mBuffer[row][col] = this->mBuffer[row][col] - m1.mBuffer[row][col];
        }
    }

    return result;
}

template <typename Real>
inline NBRE_Matrix4x4<Real>& 
NBRE_Matrix4x4<Real>::operator -= (const NBRE_Matrix4x4<Real>& m1)
{
    for (uint16 row = 0; row < 4; ++row)
    {
        for (uint16 col = 0; col < 4; ++col)
        {
            this->mBuffer[row][col] = this->mBuffer[row][col] - m1.mBuffer[row][col];
        }
    }

    return *this;
}

template <typename Real>
inline NBRE_Matrix4x4<Real>& 
NBRE_Matrix4x4<Real>::operator - ()
{
    for (uint16 row = 0; row < 4; ++row)
    {
        for (uint16 col = 0; col < 4; ++col)
        {
            this->mBuffer[row][col] = -this->mBuffer[row][col];
        }
    }
    return *this;
}

template <typename Real>
inline NBRE_Matrix4x4<Real> 
NBRE_Matrix4x4<Real>::operator * (const NBRE_Matrix4x4<Real>& m1) const
{
    NBRE_Matrix4x4<Real> result;

    result.mBuffer[0][0] = mBuffer[0][0] * m1.mBuffer[0][0] + mBuffer[0][1] * m1.mBuffer[1][0] + mBuffer[0][2] * m1.mBuffer[2][0] + mBuffer[0][3] * m1.mBuffer[3][0];
    result.mBuffer[0][1] = mBuffer[0][0] * m1.mBuffer[0][1] + mBuffer[0][1] * m1.mBuffer[1][1] + mBuffer[0][2] * m1.mBuffer[2][1] + mBuffer[0][3] * m1.mBuffer[3][1];
    result.mBuffer[0][2] = mBuffer[0][0] * m1.mBuffer[0][2] + mBuffer[0][1] * m1.mBuffer[1][2] + mBuffer[0][2] * m1.mBuffer[2][2] + mBuffer[0][3] * m1.mBuffer[3][2];
    result.mBuffer[0][3] = mBuffer[0][0] * m1.mBuffer[0][3] + mBuffer[0][1] * m1.mBuffer[1][3] + mBuffer[0][2] * m1.mBuffer[2][3] + mBuffer[0][3] * m1.mBuffer[3][3];

    result.mBuffer[1][0] = mBuffer[1][0] * m1.mBuffer[0][0] + mBuffer[1][1] * m1.mBuffer[1][0] + mBuffer[1][2] * m1.mBuffer[2][0] + mBuffer[1][3] * m1.mBuffer[3][0];
    result.mBuffer[1][1] = mBuffer[1][0] * m1.mBuffer[0][1] + mBuffer[1][1] * m1.mBuffer[1][1] + mBuffer[1][2] * m1.mBuffer[2][1] + mBuffer[1][3] * m1.mBuffer[3][1];
    result.mBuffer[1][2] = mBuffer[1][0] * m1.mBuffer[0][2] + mBuffer[1][1] * m1.mBuffer[1][2] + mBuffer[1][2] * m1.mBuffer[2][2] + mBuffer[1][3] * m1.mBuffer[3][2];
    result.mBuffer[1][3] = mBuffer[1][0] * m1.mBuffer[0][3] + mBuffer[1][1] * m1.mBuffer[1][3] + mBuffer[1][2] * m1.mBuffer[2][3] + mBuffer[1][3] * m1.mBuffer[3][3];

    result.mBuffer[2][0] = mBuffer[2][0] * m1.mBuffer[0][0] + mBuffer[2][1] * m1.mBuffer[1][0] + mBuffer[2][2] * m1.mBuffer[2][0] + mBuffer[2][3] * m1.mBuffer[3][0];
    result.mBuffer[2][1] = mBuffer[2][0] * m1.mBuffer[0][1] + mBuffer[2][1] * m1.mBuffer[1][1] + mBuffer[2][2] * m1.mBuffer[2][1] + mBuffer[2][3] * m1.mBuffer[3][1];
    result.mBuffer[2][2] = mBuffer[2][0] * m1.mBuffer[0][2] + mBuffer[2][1] * m1.mBuffer[1][2] + mBuffer[2][2] * m1.mBuffer[2][2] + mBuffer[2][3] * m1.mBuffer[3][2];
    result.mBuffer[2][3] = mBuffer[2][0] * m1.mBuffer[0][3] + mBuffer[2][1] * m1.mBuffer[1][3] + mBuffer[2][2] * m1.mBuffer[2][3] + mBuffer[2][3] * m1.mBuffer[3][3];

    result.mBuffer[3][0] = mBuffer[3][0] * m1.mBuffer[0][0] + mBuffer[3][1] * m1.mBuffer[1][0] + mBuffer[3][2] * m1.mBuffer[2][0] + mBuffer[3][3] * m1.mBuffer[3][0];
    result.mBuffer[3][1] = mBuffer[3][0] * m1.mBuffer[0][1] + mBuffer[3][1] * m1.mBuffer[1][1] + mBuffer[3][2] * m1.mBuffer[2][1] + mBuffer[3][3] * m1.mBuffer[3][1];
    result.mBuffer[3][2] = mBuffer[3][0] * m1.mBuffer[0][2] + mBuffer[3][1] * m1.mBuffer[1][2] + mBuffer[3][2] * m1.mBuffer[2][2] + mBuffer[3][3] * m1.mBuffer[3][2];
    result.mBuffer[3][3] = mBuffer[3][0] * m1.mBuffer[0][3] + mBuffer[3][1] * m1.mBuffer[1][3] + mBuffer[3][2] * m1.mBuffer[2][3] + mBuffer[3][3] * m1.mBuffer[3][3];

    return result;
}

template <typename Real>
inline NBRE_Matrix4x4<Real>& 
NBRE_Matrix4x4<Real>::operator *= (const NBRE_Matrix4x4<Real>& m1)
{
    *this = *this * m1;
    return *this;
}

template <typename Real>
inline NBRE_Vector4<Real>
NBRE_Matrix4x4<Real>::operator * (const NBRE_Vector4<Real>& vec) const
{
    return NBRE_Vector4<Real>(mBuffer[0][0]*vec.x + mBuffer[0][1]*vec.y + mBuffer[0][2]*vec.z + mBuffer[0][3]*vec.w,
                              mBuffer[1][0]*vec.x + mBuffer[1][1]*vec.y + mBuffer[1][2]*vec.z + mBuffer[1][3]*vec.w,
                              mBuffer[2][0]*vec.x + mBuffer[2][1]*vec.y + mBuffer[2][2]*vec.z + mBuffer[2][3]*vec.w,
                              mBuffer[3][0]*vec.x + mBuffer[3][1]*vec.y + mBuffer[3][2]*vec.z + mBuffer[3][3]*vec.w);
}

template <typename Real>
inline NBRE_Vector3<Real> 
NBRE_Matrix4x4<Real>::operator * (const NBRE_Vector3<Real>& v) const
{
    Real fInvW = static_cast<Real>(1.0f / (mBuffer[3][0] + mBuffer[3][1] + mBuffer[3][2] + mBuffer[3][3]));

    Real x = ( mBuffer[0][0] * v.x + mBuffer[0][1] * v.y + mBuffer[0][2] * v.z + mBuffer[0][3] ) * fInvW;
    Real y = ( mBuffer[1][0] * v.x + mBuffer[1][1] * v.y + mBuffer[1][2] * v.z + mBuffer[1][3] ) * fInvW;
    Real z = ( mBuffer[2][0] * v.x + mBuffer[2][1] * v.y + mBuffer[2][2] * v.z + mBuffer[2][3] ) * fInvW;

    NBRE_Vector3<Real> vector(x, y, z);
    return vector;
}

template <typename Real>
inline const Real* 
NBRE_Matrix4x4<Real>::operator [] (uint8 row) const
{
    nbre_assert(row < 4);
    return mBuffer[row];
}

template <typename Real>
inline Real* 
NBRE_Matrix4x4<Real>::operator [] (uint8 row)
{
    nbre_assert(row < 4);
    return mBuffer[row];
}

template <typename Real>
inline void 
NBRE_Matrix4x4<Real>::LoadFrom(const Real* buffer, nb_boolean isRowMajor)
{
    nsl_memcpy(mBuffer, buffer, sizeof(mBuffer));
    if (!isRowMajor)
    {
        Transpose();
    }
}

template <typename Real>
inline void 
NBRE_Matrix4x4<Real>::CopyTo(Real* buffer, nb_boolean isRowMajor) const
{
    if (isRowMajor)
    {
    	//note: in release version it will crash
//        nsl_memcpy(buffer, mBuffer, sizeof(mBuffer));
		for (uint32 row = 0; row < 4; ++row)
        {
			for (uint32 col = 0; col < 4; ++col)
            {
                *buffer++ = mBuffer[row][col];
            }
        }
    }
    else
    {
        for (uint32 col = 0; col < 4; ++col)
        {
            for (uint32 row = 0; row < 4; ++row)
            {
                *buffer++ = mBuffer[row][col];
            }
        }
    }
}

template <typename Real>
inline void 
NBRE_Matrix4x4<Real>::Identity()
{
    nsl_memset(mBuffer, 0, sizeof(mBuffer));
    mBuffer[0][0] = mBuffer[1][1] = mBuffer[2][2] = mBuffer[3][3] = 1.0f;
}

template <typename Real>
inline void 
NBRE_Matrix4x4<Real>::Transpose()
{
    for (uint16 row = 0; row < 4; ++row) 
    {
        for(uint16 col = row; col < 4; ++col)
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
NBRE_Matrix4x4<Real>::Zero()
{
    nsl_memset(mBuffer, 0, sizeof(mBuffer));
}

template <typename Real>
inline NBRE_Matrix4x4<Real>
NBRE_Matrix4x4<Real>::Inverse() const
{
    Real m00 = mBuffer[0][0], m01 = mBuffer[0][1], m02 = mBuffer[0][2], m03 = mBuffer[0][3];
    Real m10 = mBuffer[1][0], m11 = mBuffer[1][1], m12 = mBuffer[1][2], m13 = mBuffer[1][3];
    Real m20 = mBuffer[2][0], m21 = mBuffer[2][1], m22 = mBuffer[2][2], m23 = mBuffer[2][3];
    Real m30 = mBuffer[3][0], m31 = mBuffer[3][1], m32 = mBuffer[3][2], m33 = mBuffer[3][3];

    Real v0 = m20 * m31 - m21 * m30;
    Real v1 = m20 * m32 - m22 * m30;
    Real v2 = m20 * m33 - m23 * m30;
    Real v3 = m21 * m32 - m22 * m31;
    Real v4 = m21 * m33 - m23 * m31;
    Real v5 = m22 * m33 - m23 * m32;

    Real t00 = + (v5 * m11 - v4 * m12 + v3 * m13);
    Real t10 = - (v5 * m10 - v2 * m12 + v1 * m13);
    Real t20 = + (v4 * m10 - v2 * m11 + v0 * m13);
    Real t30 = - (v3 * m10 - v1 * m11 + v0 * m12);

    Real invDet = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

    Real d00 = t00 * invDet;
    Real d10 = t10 * invDet;
    Real d20 = t20 * invDet;
    Real d30 = t30 * invDet;

    Real d01 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
    Real d11 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
    Real d21 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
    Real d31 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

    v0 = m10 * m31 - m11 * m30;
    v1 = m10 * m32 - m12 * m30;
    v2 = m10 * m33 - m13 * m30;
    v3 = m11 * m32 - m12 * m31;
    v4 = m11 * m33 - m13 * m31;
    v5 = m12 * m33 - m13 * m32;

    Real d02 = + (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
    Real d12 = - (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
    Real d22 = + (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
    Real d32 = - (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

    v0 = m21 * m10 - m20 * m11;
    v1 = m22 * m10 - m20 * m12;
    v2 = m23 * m10 - m20 * m13;
    v3 = m22 * m11 - m21 * m12;
    v4 = m23 * m11 - m21 * m13;
    v5 = m23 * m12 - m22 * m13;

    Real d03 = - (v5 * m01 - v4 * m02 + v3 * m03) * invDet;
    Real d13 = + (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
    Real d23 = - (v4 * m00 - v2 * m01 + v0 * m03) * invDet;
    Real d33 = + (v3 * m00 - v1 * m01 + v0 * m02) * invDet;

    return NBRE_Matrix4x4<Real>(
        d00, d01, d02, d03,
        d10, d11, d12, d13,
        d20, d21, d22, d23,
        d30, d31, d32, d33);
}
