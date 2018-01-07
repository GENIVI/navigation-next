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
#include "nbgmcustompin.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbrerenderpal.h"
#include "nbrehardwarebuffer.h"
#include "nbreintersection.h"
#include "nbremath.h"
#include "nbretypeconvert.h"
#include "nbretransformutil.h"
#include "nbgmlabeldebugutil.h"
#include "nbgmcommon.h"
#include "nbgmconst.h"

NBGM_CustomPin::NBGM_CustomPin(NBGM_Context& nbgmContext,
                               NBRE_Node* parentNode,
                               NBRE_SurfaceSubView* subView,
                               int32 overlayId,
                               const NBRE_Vector2f& size,
                               NBRE_ShaderPtr shader,
                               NBRE_ShaderPtr selectedShader,
                               NBRE_ShaderPtr unselectedShader,
                               const NBRE_Vector3d& position,
                               const NBRE_String& id,
                               const NBRE_Vector2f& imageOffset,
                               const NBRE_Vector2f& bubbleOffset,
                               float distanceToPoi,
                               const NBRE_String& layerId,
                               nb_boolean enableCollisionDetection)
    :NBGM_LayoutElement(nbgmContext)
    ,mSubView(subView)
    ,mOverlayId(overlayId)
    ,mSize(size)
    ,mPosition(position)
    ,mIsInFrustum(FALSE)
    ,mScale(1.0f)
    ,mId(id)
    ,mSelected(FALSE)
    ,mParentNode(parentNode)
    ,mShader(shader)
    ,mSelectedShader(selectedShader)
    ,mUnselectedShader(unselectedShader)
    ,mBaseSize(1.0)
    ,mImageOffset(imageOffset)
    ,mBubbleOffset(bubbleOffset)
    ,mDistanceToPoi(distanceToPoi)
    ,mLayerId(layerId)
    ,mEnableCollisionDetection(enableCollisionDetection)
{
    nbre_assert(parentNode != NULL && subView != NULL);
    SetTypeId(NBGM_LET_CUSTOM_PIN);
}

NBGM_CustomPin::~NBGM_CustomPin()
{
    if(mNode)
    {
        mParentNode->RemoveChild(mNode.get());
        NBRE_BillboardSet* bs = mSubView->GetBillboardSet(mOverlayId);
        if (bs)
        {
            bs->RemoveBillboard(mNode);
        }
    }
}

void
NBGM_CustomPin::OnUpdate()
{
    mIsInFrustum = FALSE;
    if (mNode.get() == NULL)
    {
        return;
    }

    NBRE_Vector3d worldPosition = mParentNode->WorldPosition() + mPosition;
    double pixelsPerUnit = 0;
    if (!mNBGMContext.transUtil->PixelsPerUnit(worldPosition, pixelsPerUnit))
    {
        return;
    }

    mScale = mSize.x / pixelsPerUnit;
    mNode->SetScale(NBRE_Vector3f((float)mScale, (float)mScale, (float)mScale));
    mBoundingBox = mNode->AxisAlignedBox();

    NBRE_Vector2d screenPosition;
    if (mNBGMContext.transUtil->WorldToScreen(worldPosition, screenPosition))
    {
        UpdateScreenRect(screenPosition, pixelsPerUnit);
        mIsInFrustum = GetDrawIndex() != INVALID_DRAW_INDEX ? TRUE : FALSE;
    }
}

nb_boolean
NBGM_CustomPin::IsElementVisible()
{
    NBGM_LayoutManager& layoutManager = *(mNBGMContext.layoutManager);

    {
        if (!IsEnable())
        {
            return FALSE;
        }

        if (!layoutManager.IsInFrustum(mNode->WorldPosition()))
        {
            return FALSE;
        }

        if (GetDrawIndex() != NBGM_LayoutElement::INVALID_DRAW_INDEX)
        {
            double pixelsPerUnit = 0;
            if (!layoutManager.PixelsPerUnit(mNode->WorldPosition(), pixelsPerUnit))
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

void
NBGM_CustomPin::UpdateRegion()
{
    if (!mEnableCollisionDetection)
    {
        return;
    }
    NBGM_LayoutManager& layoutManager = *(mNBGMContext.layoutManager);
    layoutManager.GetLayoutBuffer().UpdateRegion(mScreenRect, CUSTOM_PIN_UPDATE_MASK);
}

nb_boolean
NBGM_CustomPin::IsElementRegionAvailable()
{
    if (!IsEnable())
    {
        return FALSE;
    }

    NBGM_LayoutManager& layoutManager = *(mNBGMContext.layoutManager);

    NBRE_Vector3d worldPosition = mParentNode->WorldPosition() + mPosition;
    NBRE_Vector2d screenPosition;
    if(!layoutManager.WorldToScreen(worldPosition, screenPosition))
    {
        return FALSE;
    }
    double pixelsPerUnit = 0;
    if (!layoutManager.PixelsPerUnit(worldPosition, pixelsPerUnit))
    {
        return FALSE;
    }

    mScale = mSize.x / pixelsPerUnit;

    double pad = layoutManager.GetLayoutBuffer().CellSize() * 4.0;
    NBRE_Vector2d padding(pad, pad);
    if (GetDrawIndex() != NBGM_LayoutElement::INVALID_DRAW_INDEX)
    {
        padding.x = 0;
        padding.y = 0;
    }

    NBRE_Vector2d halfSize(mSize.x * 0.5, mSize.y * 0.5);
    NBRE_Vector2d offset(mSize.x * mImageOffset.x, mSize.y * mImageOffset.y);
    mScreenRect.minExtend = screenPosition - halfSize + offset;
    mScreenRect.maxExtend = screenPosition + halfSize + offset;

    if (!NBRE_Intersectiond::HitTest(mScreenRect, layoutManager.GetScreenRect()))
    {
        return FALSE;
    }

    NBRE_AxisAlignedBox2d sr(mScreenRect.minExtend - padding, mScreenRect.maxExtend + padding);
    nb_boolean result = layoutManager.GetLayoutBuffer().IsRegionAvailable(sr, CUSTOM_PIN_CHECK_MASK);
    if (result && mNode.get() == NULL)
    {
        NBRE_BillboardSet* bs = mSubView->GetBillboardSet(mOverlayId);
        if (!bs)
        {
            return FALSE;
        }
        mNode = bs->AddBillboard();

        NBRE_Vector2f imgSize(1.0f, mSize.y / mSize.x);
        NBRE_ImageEntity* imageEntity = NBRE_NEW NBRE_ImageEntity(mNBGMContext.renderingEngine->Context(), imgSize, mShader, mImageOffset);
        mNode->AddEntity(NBRE_BillboardEntityPtr(imageEntity));

        mNode->SetPosition(mPosition);
        mNode->SetVisible(FALSE);
        mParentNode->AddChild(mNode);
    }

    if (mNode.get())
    {
        mNode->SetScale(NBRE_Vector3f(static_cast<float>(mScale), static_cast<float>(mScale), static_cast<float>(mScale)));
        mBoundingBox = mNode->AxisAlignedBox();
    }
    return result;
}

NBRE_Vector3d
NBGM_CustomPin::GetPosition()
{
    return mPosition;
}

void
NBGM_CustomPin::SetColor(const NBRE_Color& color)
{
    mNode->SetColor(color);
}

double
NBGM_CustomPin::UpdateScale(double pixelsPerUnit)
{
    return mSize.x / pixelsPerUnit;
}

void
NBGM_CustomPin::SetSelected(nb_boolean val)
{
    mSelected = val;
}

nb_boolean
NBGM_CustomPin::GetScreenPosition(NBRE_Vector2f& position)
{
    if(!mIsInFrustum)
    {
        return FALSE;
    }
    NBRE_Vector2d screenPosition;
    if (mNBGMContext.transUtil->WorldToScreen(mNode->WorldPosition(),screenPosition))
    {
        NBRE_AxisAlignedBox2d box = mNBGMContext.transUtil->GetScreenRect();
        if(screenPosition.x > box.GetSize().x || screenPosition.y > box.GetSize().y || screenPosition.x < 0.f || screenPosition.y < 0.f)
        {
            return FALSE;
        }
        position.x = static_cast<float>(screenPosition.x);
        position.y = static_cast<float>(screenPosition.y);
        return TRUE;
    }
    return FALSE;
}

nb_boolean
NBGM_CustomPin::GetScreenBubblePosition(NBRE_Vector2f& position)
{
    NBRE_Vector2f pos;
    if(GetScreenPosition(pos))
    {
        position.x = pos.x + mBubbleOffset.x*mSize.x;
        position.y = pos.y + mBubbleOffset.y*mSize.y;
        return TRUE;
    }
    return FALSE;
}

NBRE_Vector2f
NBGM_CustomPin::GetScreenBubbleOffset()
{
    return NBRE_Vector2f(mBubbleOffset.x * mSize.x, mBubbleOffset.y * mSize.y);
}

nb_boolean
NBGM_CustomPin::HitTest(const NBRE_Vector2f& screenPosition)
{
    if (!mIsInFrustum || !mNode.get() || !mNode->IsVisible())
    {
        return FALSE;
    }

    const NBRE_Vector2d& sp = NBRE_TypeConvertd::Convert(screenPosition);
    if (NBRE_Intersectiond::HitTest(mScreenRect, sp))
    {
        return TRUE;
    }
    return FALSE;
}

void
NBGM_CustomPin::AddToSence()
{
    NBGM_LayoutElement::AddToSence();

    NBRE_Vector3d worldPosition = mParentNode->WorldPosition() + mPosition;
    mBoundingBox.minExtend = worldPosition;
    mBoundingBox.maxExtend = worldPosition;
    mMinPixelPerUnit = 0;
}

void
NBGM_CustomPin::SetVisible(nb_boolean value)
{
    if (mNode.get())
    {
        mNode->SetVisible(value);
    }
}

void
NBGM_CustomPin::UpdateScreenRect(const NBRE_Vector2d& screenPosition, double /*pixelsPerUnit*/)
{
    NBRE_Vector2d hs(mSize.x * 0.5, mSize.y * 0.5);
    NBRE_Vector2d offset(mImageOffset.x * mSize.x, mImageOffset.y * mSize.y);
    mScreenRect.minExtend = screenPosition - hs + offset;
    mScreenRect.maxExtend = screenPosition + hs + offset;
}

nb_boolean
NBGM_CustomPin::IsTooCloseTo(NBGM_LayoutElement* elem)
{
    if (!mEnableCollisionDetection)
    {
        return FALSE;
    }
    NBGM_LayoutElementType t = elem->GetTypeId();

    switch (t)
    {
    case NBGM_LET_CUSTOM_PIN:
        {
            NBGM_CustomPin* p = (NBGM_CustomPin*)elem;
            if (p->mLayerId == mLayerId &&
                mScreenRect.GetCenter().DistanceTo(elem->GetScreenRect().GetCenter()) < mDistanceToPoi)
            {
                return TRUE;
            }
        }
        break;
    default:
        break;
    }
    return FALSE;
}

void NBGM_CustomPin::EnableCollsionDetection(nb_boolean value)
{
    if (value == mEnableCollisionDetection)
    {
        return;
    }
    mEnableCollisionDetection = value;
    InvalidateLayout();
}


NBGM_LayoutElement*
NBGM_CustomPin::Clone(NBRE_Node* parentNode)
{
    NBGM_CustomPin* icon = NBRE_NEW NBGM_CustomPin(mNBGMContext, parentNode, mSubView, mOverlayId, mSize, mShader, mSelectedShader, mUnselectedShader,
        mPosition, mId, mImageOffset, mBubbleOffset, mDistanceToPoi, mLayerId, mEnableCollisionDetection);
    icon->SetPriority(mPriority);
    return icon;
}
