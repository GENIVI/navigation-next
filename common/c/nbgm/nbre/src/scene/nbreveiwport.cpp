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
#include "nbreveiwport.h"
#include "nbrerenderpal.h"
#include "nbretransformation.h"
#include "nbretypeconvert.h"

NBRE_Viewport::NBRE_Viewport(int32 left, int32 top, uint32 width, uint32 height)
    : mLeftBottomX(left), mLeftBottomY(top), mWidth(width), mHeight(height)
{

}

NBRE_Viewport::~NBRE_Viewport()
{
}

void NBRE_Viewport::Apply(NBRE_IRenderPal& renderPal)
{
    renderPal.SetViewPort(mLeftBottomX, mLeftBottomY, mWidth, mHeight);

    mCurrentCamera->UpdatePosition();

    renderPal.SetProjectionTransform(mCurrentCamera->GetFrustum().ExtractMatrix());

    renderPal.SetViewTransform(mCurrentCamera->ExtractMatrix());
}

void NBRE_Viewport::SetCamera(const NBRE_CameraPtr& camera)
{
    nbre_assert(camera.get());
    mCurrentCamera = camera;
}

const NBRE_CameraPtr& NBRE_Viewport::Camera()
{
    return mCurrentCamera;
}

void NBRE_Viewport::Update(int32 left, int32 top, uint32 width, uint32 height)
{
    mLeftBottomX = left;
    mLeftBottomY = top;
    mWidth = width;
    mHeight = height;
    mViewportMatrix = NBRE_Transformationf::BuildViewportMatrix(
        static_cast<float>(left),
        static_cast<float>(top),
        static_cast<float>(width),
        static_cast<float>(height));
}

NBRE_AxisAlignedBox2i NBRE_Viewport::GetRect() const
{
    return NBRE_AxisAlignedBox2i(mLeftBottomX, mLeftBottomY, mLeftBottomX + static_cast<int32>(mWidth), mLeftBottomY + static_cast<int32>(mHeight));
}

const NBRE_Matrix4x4f& NBRE_Viewport::ExtractMatrix() const
{
    return mViewportMatrix;
}

const NBRE_Matrix4x4f& NBRE_Viewport::GetViewProjectionViewportMatrix()
{
    mViewProjectionViewportMatrix = mViewportMatrix * mCurrentCamera->GetFrustum().ExtractMatrix() * NBRE_TypeConvertf::Convert(mCurrentCamera->ExtractMatrix());
    return mViewProjectionViewportMatrix;
}

static void TransformPoint(const NBRE_Frustum& frustum, const NBRE_AxisAlignedBox2i& rect, NBRE_Point3d& pt)
{
    const NBRE_PerspectiveConfig& pers = frustum.GetPerspectiveConfig();
    double top = nsl_tan(pers.mFov * 0.5 * NBRE_Math::Pi64 / 180.0) * frustum.GetZNear();
    double right = top * pers.mAspect;
    pt.x = 2*pt.x*right/rect.GetSize().x;
    pt.y = 2*pt.y*top/rect.GetSize().y;
    pt.z = -frustum.GetZNear();
}

NBRE_Ray3d NBRE_Viewport::GetRay(float x, float y)
{
    NBRE_AxisAlignedBox2i rect = GetRect();

    NBRE_Point3d pt(x - rect.GetSize().x/2, rect.GetSize().y/2 - y, 0);
    TransformPoint(mCurrentCamera->GetFrustum(), rect, pt);

    NBRE_Transformationd::Transform(mCurrentCamera->WorldTransform(), pt);
    return NBRE_Ray3d(mCurrentCamera->WorldPosition(), pt - mCurrentCamera->WorldPosition());
}

NBRE_Viewport* NBRE_Viewport::Clone()
{
    NBRE_Viewport* viewport = NBRE_NEW NBRE_Viewport(mLeftBottomX, mLeftBottomY, mWidth, mHeight);
    if (viewport != NULL)
    {
        viewport->SetCamera(NBRE_CameraPtr(mCurrentCamera->CreateWorldCamera()));
    }

    return viewport;
}

NBRE_Vector3d NBRE_Viewport::GetWorldPosition(float x, float y)
{
    NBRE_AxisAlignedBox2i rect = GetRect();

    NBRE_Point3d pt(x - rect.GetSize().x/2, rect.GetSize().y/2 - y, 0);
    TransformPoint(mCurrentCamera->GetFrustum(), rect, pt);

    NBRE_Transformationd::Transform(mCurrentCamera->WorldTransform(), pt);

    return pt;
}