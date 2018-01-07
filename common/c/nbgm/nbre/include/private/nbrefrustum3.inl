/*!--------------------------------------------------------------------------

    @file nbrefrustum3.inl
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
inline NBRE_Frustum3<Real>::NBRE_Frustum3()
{
}

template <typename Real>
inline NBRE_Frustum3<Real>::NBRE_Frustum3(const NBRE_Matrix4x4<Real>& matrix)
{
    UpdateFrom(matrix);
}

template <typename Real>
inline NBRE_Frustum3<Real>::NBRE_Frustum3(const NBRE_Plane<Real>& left,   const NBRE_Plane<Real>& right
                                        , const NBRE_Plane<Real>& bottom, const NBRE_Plane<Real>& top
                                        , const NBRE_Plane<Real>& nearZ,   const NBRE_Plane<Real>& farZ)
:mLeft(left), mRight(right), mBottom(bottom), mTop(top), mNear(nearZ), mFar(farZ)
{
    Update();
}

template <typename Real>
inline NBRE_Frustum3<Real>::~NBRE_Frustum3()
{
}

template <typename Real>
inline const NBRE_Plane<Real>&
NBRE_Frustum3<Real>::GetNear() const 
{
    return mNear; 
}

template <typename Real>
inline const NBRE_Plane<Real>&
NBRE_Frustum3<Real>::GetFar() const 
{
    return mFar; 
}

template <typename Real>
inline const NBRE_Plane<Real>& 
NBRE_Frustum3<Real>::GetBottom() const 
{
    return mBottom; 
}

template <typename Real>
inline const NBRE_Plane<Real>&
NBRE_Frustum3<Real>::GetTop() const 
{
    return mTop; 
}

template <typename Real>
inline const NBRE_Plane<Real>& 
NBRE_Frustum3<Real>::GetLeft() const 
{
    return mLeft; 
}

template <typename Real>
inline const NBRE_Plane<Real>& 
NBRE_Frustum3<Real>::GetRight() const 
{
    return mRight; 
}

template <typename Real>
inline const NBRE_AxisAlignedBox3<Real>&
NBRE_Frustum3<Real>::GetAABB() const 
{
    return mAABB; 
}

template <typename Real>
inline void
NBRE_Frustum3<Real>::UpdateFrom(const NBRE_Matrix4x4<Real>& matrix)
{
    Real m[4][4];
    matrix.CopyTo(&m[0][0], TRUE);

    mLeft.normal.x = m[3][0] + m[0][0];
    mLeft.normal.y = m[3][1] + m[0][1];
    mLeft.normal.z = m[3][2] + m[0][2];
    mLeft.d        = m[3][3] + m[0][3];

    mRight.normal.x = m[3][0] - m[0][0];
    mRight.normal.y = m[3][1] - m[0][1];
    mRight.normal.z = m[3][2] - m[0][2];
    mRight.d        = m[3][3] - m[0][3];

    mBottom.normal.x = m[3][0] + m[1][0];
    mBottom.normal.y = m[3][1] + m[1][1];
    mBottom.normal.z = m[3][2] + m[1][2];
    mBottom.d        = m[3][3] + m[1][3];

    mTop.normal.x = m[3][0] - m[1][0];
    mTop.normal.y = m[3][1] - m[1][1];
    mTop.normal.z = m[3][2] - m[1][2];
    mTop.d        = m[3][3] - m[1][3];

    mNear.normal.x = m[3][0] + m[2][0];
    mNear.normal.y = m[3][1] + m[2][1];
    mNear.normal.z = m[3][2] + m[2][2];
    mNear.d        = m[3][3] + m[2][3];

    mFar.normal.x = m[3][0] - m[2][0];
    mFar.normal.y = m[3][1] - m[2][1];
    mFar.normal.z = m[3][2] - m[2][2];
    mFar.d        = m[3][3] - m[2][3];

    mLeft.Normalize();
    mRight.Normalize();
    mBottom.Normalize();
    mTop.Normalize();
    mNear.Normalize();
    mFar.Normalize();

    Update();
}

template <typename Real>
static NBRE_Vector3<Real>
IntersectPlanes(const NBRE_Plane<Real>& plane1, const NBRE_Plane<Real>& plane2, const NBRE_Plane<Real>& plane3)
{
    Real demoninator = (plane1.normal.CrossProduct(plane2.normal)).DotProduct(plane3.normal);
    return  ( plane2.normal.CrossProduct(plane3.normal) * plane1.d 
            + plane3.normal.CrossProduct(plane1.normal) * plane2.d
            + plane1.normal.CrossProduct(plane2.normal) * plane3.d
            ) / -demoninator;
}

template <typename Real>
inline void 
NBRE_Frustum3<Real>::Update()
{
    corners[0] = IntersectPlanes<Real>(mLeft, mBottom, mNear);
    corners[1] = IntersectPlanes<Real>(mRight, mBottom, mNear);
    corners[2] = IntersectPlanes<Real>(mRight, mTop, mNear);
    corners[3] = IntersectPlanes<Real>(mLeft, mTop, mNear);
    corners[4] = IntersectPlanes<Real>(mLeft, mBottom, mFar);
    corners[5] = IntersectPlanes<Real>(mRight, mBottom, mFar);
    corners[6] = IntersectPlanes<Real>(mRight, mTop, mFar);
    corners[7] = IntersectPlanes<Real>(mLeft, mTop, mFar);
    mAABB.Set(corners, 8);
}

template <typename Real>
inline void
NBRE_Frustum3<Real>::GetCorners(NBRE_Vector3<Real>* resultPoints) const
{
    for (uint32 i = 0; i < 8; ++i)
    {
        resultPoints[i] = corners[i];
    }
}
