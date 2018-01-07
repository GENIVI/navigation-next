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
#include "nbretextpath3dentity.h"
#include "nbrecommon.h"
#include "nbredefaultpass.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbretypeconvert.h"
#include "nbresurfacemanager.h"
#include "nbretransformutil.h"

static const float TEXT_PATH_MAX_ANGLE_TOLERANCE = 10.0f * NBRE_Math::Deg2Rad;

NBRE_TextPath3dEntity::NBRE_TextPath3dEntity(NBRE_Context& context, NBRE_TransformUtil* transUtil, int32 overlayId, const NBRE_WString& text,
        const NBRE_FontMaterial& fontMaterial, float fontHeight, NBRE_Polyline2f* polyline, float offset)
:mContext(context),
mTransUtil(transUtil),
mVisible(TRUE),
mOverlayId(overlayId),
mParentNode(NULL),
mPolyline(polyline),
mFontHeight(fontHeight)
{
    mTextEntity = new NBRE_TextPathEntity(context, text, fontMaterial, fontHeight, polyline, offset);
}

NBRE_TextPath3dEntity::~NBRE_TextPath3dEntity()
{
    NBRE_DELETE mTextEntity;
}

const NBRE_AxisAlignedBox3d&
NBRE_TextPath3dEntity::AxisAlignedBox() const
{
    return mBoundingBox;
}

nb_boolean
NBRE_TextPath3dEntity::Visible() const
{
    return mVisible;
}

void
NBRE_TextPath3dEntity::SetVisible( nb_boolean val )
{
    mVisible = val;
}

void
NBRE_TextPath3dEntity::UpdateSurfaceList()
{
}

void
NBRE_TextPath3dEntity::NotifyAttached(NBRE_Node* node)
{
    mParentNode = node;
}

void
NBRE_TextPath3dEntity::NotifyDetatch()
{
    mParentNode = NULL;
}

void
NBRE_TextPath3dEntity::NotifyUpdatePosition()
{
    mBoundingBox = NBRE_TypeConvertd::Convert(mTextEntity->AxisAlignedBox());
    NBRE_Transformationd::Transform(mParentNode->WorldTransform(), mBoundingBox);
    mTextEntity->SetTransform(mParentNode->WorldTransform());
}

NBRE_ISpatialObject*
NBRE_TextPath3dEntity::SpatialObject()
{
    return this;
}

void
NBRE_TextPath3dEntity::Accept(NBRE_Visitor& visitor)
{
    visitor.Visit(static_cast<NBRE_IRenderOperationProvider*>(this), mContext.mSurfaceManager);
    visitor.Visit(static_cast<NBRE_ISpatialObject*>(this));
    visitor.Visit(static_cast<NBRE_IMovableObject*>(this));
}

void 
NBRE_TextPath3dEntity::NotifyCamera(const NBRE_Camera& camera)
{
    if(!mTextEntity || !mVisible)
    {
        return;
    }

    NBRE_Vector3d pos = camera.Position();
    pos.Normalise();
    double cosValue = pos.DotProduct(NBRE_Vector3d(0, 0, 1));
    float tiltAngle = static_cast<float>(NBRE_Math::RadToDeg(nsl_acos(cosValue)));
    mTextEntity->UpdateTextRotate(mTransUtil->GetEyePosition()-mParentNode->WorldPosition(), mTransUtil->GetCameraRight(), tiltAngle);
}

void
NBRE_TextPath3dEntity::UpdateRenderQueue(NBRE_Overlay& overlay)
{
    if (!mVisible)
    {
        return;
    }
    Rebuild();
    mTextEntity->UpdateRenderQueue(overlay);
}

int32
NBRE_TextPath3dEntity::OverlayId() const
{
    return mOverlayId;
}

void
NBRE_TextPath3dEntity::Rebuild()
{
    const NBRE_Vector3d& refCenter = mParentNode->WorldPosition();
    const NBRE_Vector2f* worldEndPoints = mTextEntity->GetLayoutEndPoints();
    NBRE_Vector2d screenEndPoints[2];
    NBRE_Vector2d mUp(0, 1);
    nb_boolean isRevert = mTextEntity->IsRevert();
    nb_boolean isRevertOld = isRevert;

    if (!mTransUtil->WorldToScreen(refCenter + NBRE_Vector3d(worldEndPoints[0].x, worldEndPoints[0].y, 0), screenEndPoints[0]))
    {
        return;
    }

    if (!mTransUtil->WorldToScreen(refCenter + NBRE_Vector3d(worldEndPoints[1].x, worldEndPoints[1].y, 0), screenEndPoints[1]))
    {
        return;
    }

    NBRE_Vector2d dir = screenEndPoints[1] - screenEndPoints[0];
    dir.Normalise();

    // Check if direction in tolerance angle range
    double cosTolerance = nsl_cos(TEXT_PATH_MAX_ANGLE_TOLERANCE);
    double cosNewDirUp = dir.DotProduct(mUp);
    if (cosNewDirUp >= -cosTolerance || cosNewDirUp <= cosTolerance)
    {
        // angle not in tolerance range
        NBRE_Vector2d right(mUp.y, -mUp.x);
        isRevert = dir.DotProduct(right) >= 0 ? FALSE : TRUE;
    }
    mTextDirection = dir;
    if (isRevert != isRevertOld)
    {
        mTextEntity->Rebuild(mFontHeight, isRevert, mPolyline, 0);
    }
}

void
NBRE_TextPath3dEntity::Rebuild(float fontHeight, nb_boolean isRevert, NBRE_Polyline2f* polyline, float offset)
{
    mFontHeight = fontHeight;
    mPolyline = polyline;
    mTextEntity->Rebuild(fontHeight, isRevert, polyline, offset);
    NotifyUpdatePosition();
}

void
NBRE_TextPath3dEntity::SetColor(const NBRE_Color& color)
{
    mTextEntity->SetColor(color);
}

void
NBRE_TextPath3dEntity::SetFontMaterial(const NBRE_FontMaterial& fontMaterial)
{
    mTextEntity->SetFontMaterial(fontMaterial);
}

const NBRE_Vector2f*
NBRE_TextPath3dEntity::GetLayoutEndPoints()
{
    return mTextEntity->GetLayoutEndPoints();
}

nb_boolean
NBRE_TextPath3dEntity::IsRevert()
{
    return mTextEntity->IsRevert();
}
