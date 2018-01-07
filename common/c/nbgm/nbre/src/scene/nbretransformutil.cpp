/*
Copyright (c) 2018, TeleCommunication Systems, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
   * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the TeleCommunication Systems, Inc., nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED. IN NO EVENT SHALL TELECOMMUNICATION SYSTEMS, INC.BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
#include "nbretransformutil.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbrerenderpal.h"
#include "nbrehardwarebuffer.h"
#include "nbreintersection.h"
#include "nbremath.h"
#include "nbreclipping.h"
#include "nbretypeconvert.h"
#include "nbrelog.h"
#include "palclock.h"

NBRE_TransformUtil::NBRE_TransformUtil(NBRE_Viewport* viewport)
:mViewport(viewport)
,mEyeToScreenCenterDistance(0)
{

}

NBRE_TransformUtil::~NBRE_TransformUtil()
{

}

nb_boolean
NBRE_TransformUtil::IsInNearPlaneFrontSide(const NBRE_Vector3d& position)
{
    return mFrustum.GetNear().GetSide(position) != NBRE_PS_BACK_SIDE ? TRUE : FALSE;
}

nb_boolean
NBRE_TransformUtil::IsInNearPlaneFrontSide(const NBRE_Polyline3d& polyline)
{
    const NBRE_Planed& nearPlane = mFrustum.GetNear();

    for (uint32 i = 0; i < polyline.VertexCount(); ++i)
    {
        if (nearPlane.GetSide(polyline.Vertex(i)) == NBRE_PS_BACK_SIDE)
        {
            return FALSE;
        }
    }
    return TRUE;
}

nb_boolean
NBRE_TransformUtil::IsInFrustum(const NBRE_Vector3d& position)
{
    return NBRE_Intersectiond::HitTest(mFrustum, position);
}

nb_boolean
NBRE_TransformUtil::IsInFrustum(const NBRE_AxisAlignedBox3d& aabb)
{
    return NBRE_Intersectiond::HitTest(mFrustum, aabb);
}

nb_boolean
NBRE_TransformUtil::IsInFrustum(const NBRE_Polyline3d& polyline)
{
    return NBRE_Intersectiond::HitTest(mFrustum, polyline.AABB());
}

nb_boolean
NBRE_TransformUtil::WorldToScreen(const NBRE_Vector3d& position, NBRE_Vector2d& screenPosition)
{
    if (!IsInNearPlaneFrontSide(position))
    {
        return FALSE;
    }
    screenPosition = NBRE_Transformationd::WorldToScreen(mViewProjectViewportMatrix, position);
    return TRUE;
}

nb_boolean
NBRE_TransformUtil::WorldToScreen(const NBRE_Vector3d& position, NBRE_Vector2d& screenPosition, double& w)
{
    if (!IsInNearPlaneFrontSide(position))
    {
        return FALSE;
    }
    screenPosition = NBRE_Transformationd::WorldToScreen(mViewProjectViewportMatrix, position, &w);
    return TRUE;
}

nb_boolean
NBRE_TransformUtil::WorldToScreen(const NBRE_Polyline3d& polyline, NBRE_Polyline2d& screenPolyline)
{
    screenPolyline = NBRE_Polyline2d(polyline.VertexCount());
    if (!IsInNearPlaneFrontSide(polyline))
    {
        return FALSE;
    }

    for (uint32 i = 0; i < polyline.VertexCount(); ++i)
    {
        screenPolyline.SetVertex(i, NBRE_Transformationd::WorldToScreen(mViewProjectViewportMatrix, polyline.Vertex(i)));
    }
    screenPolyline.UpdateLength();

    return TRUE;
}

nb_boolean
NBRE_TransformUtil::PixelsPerUnit(const NBRE_Vector3d& position, double& pixelsPerUnit)
{
    if (!IsInNearPlaneFrontSide(position))
    {
        return FALSE;
    }
    NBRE_Vector3d left(position);
    NBRE_Vector3d right(position);

    right.x += mViewMatrix[0][0];
    right.y += mViewMatrix[0][1];
    right.z += mViewMatrix[0][2];
    pixelsPerUnit = NBRE_Transformationd::WorldToScreen(mViewProjectViewportMatrix, left).DistanceTo(NBRE_Transformationd::WorldToScreen(mViewProjectViewportMatrix, right));
    return TRUE;
}

nb_boolean
NBRE_TransformUtil::UpdateCameraMatrix()
{
    nb_boolean result = FALSE;
    mViewMatrix = mViewport->Camera()->ExtractMatrix();
    mProjectMatrix = NBRE_TypeConvertd::Convert(mViewport->Camera()->GetFrustum().ExtractMatrix());
    mViewportMatrix = NBRE_TypeConvertd::Convert(mViewport->ExtractMatrix());
    NBRE_Matrix4x4d compositeMatrix = mViewportMatrix * mProjectMatrix * mViewMatrix;
    if (!(compositeMatrix == mViewProjectViewportMatrix))
    {
        result = TRUE;
    }
    mViewProjectViewportMatrix = compositeMatrix;

    mFrustum = NBRE_Frustum3d(mProjectMatrix * mViewMatrix);
    const NBRE_Matrix4x4d& cameraMatrix = mViewport->Camera()->WorldTransform();
    mCameraRight.x = cameraMatrix[0][0];
    mCameraRight.y = cameraMatrix[1][0];
    mCameraRight.z = cameraMatrix[2][0];

    mEyePosition = mViewport->Camera()->WorldPosition();
    mEyeToScreenCenterDistance = mViewport->Camera()->Position().Length();
    UpdatePixelScale();

    NBRE_AxisAlignedBox2i rc(mViewport->GetRect());
    mScreenRect.minExtend.x = static_cast<double>(rc.minExtend.x);
    mScreenRect.minExtend.y = static_cast<double>(rc.minExtend.y);
    mScreenRect.maxExtend.x = static_cast<double>(rc.maxExtend.x);
    mScreenRect.maxExtend.y = static_cast<double>(rc.maxExtend.y);
    return result;
}

void
NBRE_TransformUtil::UpdatePixelScale()
{
    NBRE_Vector3d frutumCorners[8];
    mFrustum.GetCorners(frutumCorners);
    NBRE_Vector3d nearCenter = (frutumCorners[0] + frutumCorners[2]) * 0.5;
    NBRE_Vector3d farCenter = (frutumCorners[4] + frutumCorners[6]) * 0.5;
    double t = 0.5;

    if (nearCenter.z * farCenter.z > 0)
    {
        return;
    }
    t = NBRE_Math::Abs<double>(nearCenter.z / (farCenter.z - nearCenter.z));
    NBRE_Vector3d screenCenter = NBRE_LinearInterpolated::Lerp(nearCenter, farCenter, t);
    if (!PixelsPerUnit(screenCenter, mScreenCenterPixelsPerUnit))
    {
        return;
    }
}
