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
#include "nbgmpointlabel.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbrerenderpal.h"
#include "nbrehardwarebuffer.h"
#include "nbreintersection.h"
#include "nbremath.h"
#include "nbretypeconvert.h"
#include "nbretransformutil.h"

NBGM_PointLabel::NBGM_PointLabel(NBGM_Context& nbgmContext, NBRE_Node* parentNode, NBRE_SurfaceSubView* subView, int32 overlayId, const NBRE_WString& text, const NBRE_FontMaterial& material, uint32 materialId, float fontHeight, const NBRE_Vector3d& position, nb_boolean isWatermark)
:NBGM_LayoutElement(nbgmContext, text)
,mSubView(subView)
,mOverlayId(overlayId)
,mPosition(position)
,mFontHeight(fontHeight)
,mFontMaterial(material)
,mFontMaterialId(materialId)
,mScale(1.0f)
,mAlpha(1.0f)
,mParentNode(parentNode)
,mVisible(FALSE)
,mTextAreaEntity(NULL)
,mIsWatermark(isWatermark)
{
    nbre_assert(parentNode != NULL);
    SetTypeId(NBGM_LET_POINT_LABEL);
}

NBGM_PointLabel::~NBGM_PointLabel()
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

nb_boolean
NBGM_PointLabel::IsElementVisible()
{
    if (!IsEnable())
    {
        return FALSE;
    }

    NBGM_LayoutManager& layoutManager = *(mNBGMContext.layoutManager);

    NBRE_Vector3d worldPosition = mParentNode->WorldPosition() + mPosition;

    if (GetDrawIndex() != NBGM_LayoutElement::INVALID_DRAW_INDEX)
    {
        if (!layoutManager.IsInFrustum(mNode->AxisAlignedBox()))
        {
            return FALSE;
        }
    }
    else
    {
        if (!layoutManager.IsInFrustum(worldPosition))
        {
            return FALSE;
        }
    }

    return TRUE;
}

void
NBGM_PointLabel::UpdateRegion()
{
    NBGM_LayoutManager& layoutManager = *(mNBGMContext.layoutManager);
    layoutManager.AddPointLabel(mText);

    NBRE_Vector3d worldPosition = mParentNode->WorldPosition() + mPosition;
    NBRE_Vector2d screenPosition;
    if(!layoutManager.WorldToScreen(worldPosition, screenPosition))
    {
        return;
    }
    if (mBaseStringSize.x == 0)
    {
        mBaseStringSize = mNBGMContext.renderingEngine->Context().mFontManager->GetStringSize(mFontMaterial, 1, mText, TRUE, PAL_TA_CENTER);
    }
    NBRE_Vector2d halfSize((float)(mBaseStringSize.x * mFontMaterial.maxFontHeightPixels) * 0.5f
        ,(float)(mBaseStringSize.y * mFontMaterial.maxFontHeightPixels) * 0.5f);
    mScreenRect.minExtend = screenPosition - halfSize;
    mScreenRect.maxExtend = screenPosition + halfSize;

    layoutManager.GetLayoutBuffer().UpdateRegion(mScreenRect, mIsWatermark ? POINT_WATERMARK_UPDATE_MASK : POINT_LABEL_UPDATE_MASK);
}

nb_boolean
NBGM_PointLabel::IsElementRegionAvailable()
{
    if (!IsEnable())
    {
        return FALSE;
    }

    NBGM_LayoutManager& layoutManager = *(mNBGMContext.layoutManager);

    if (layoutManager.IsPointLabelDisplayed(mText))
    {
        return FALSE;
    }

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

    mScale = mFontMaterial.maxFontHeightPixels / pixelsPerUnit / mFontHeight;

    double pad = layoutManager.GetLayoutBuffer().CellSize();
    NBRE_Vector2d padding(pad, pad);
    if (GetDrawIndex() != NBGM_LayoutElement::INVALID_DRAW_INDEX)
    {
        padding.x = 0;
        padding.y = 0;
    }
    if (mBaseStringSize.x == 0)
    {
        mBaseStringSize = mNBGMContext.renderingEngine->Context().mFontManager->GetStringSize(mFontMaterial, 1, mText, TRUE);
    }
    NBRE_Vector2d halfSize((float)(mBaseStringSize.x * mFontMaterial.maxFontHeightPixels) * 0.5f
        ,(float)(mBaseStringSize.y * mFontMaterial.maxFontHeightPixels) * 0.5f);
    mScreenRect.minExtend = screenPosition - halfSize - padding;
    mScreenRect.maxExtend = screenPosition + halfSize + padding;

    if (!NBRE_Intersectiond::HitTest(mScreenRect, layoutManager.GetScreenRect()))
    {
        return FALSE;
    }

    nb_boolean result = layoutManager.GetLayoutBuffer().IsRegionAvailable(mScreenRect, mIsWatermark ? POINT_WATERMARK_CHECK_MASK : POINT_LABEL_CHECK_MASK);
    if (result)
    {
        if (!mNode)
        {
            NBRE_BillboardSet* bs = mSubView->GetBillboardSet(mOverlayId);
            if (!bs)
            {
                return FALSE;
            }
            mNode = bs->AddBillboard();
            mTextAreaEntity = NBRE_NEW NBRE_TextAreaEntity(mNBGMContext.renderingEngine->Context(), mText, mFontMaterial, mFontHeight, PAL_TA_CENTER);
            mNode->AddEntity(NBRE_BillboardEntityPtr(mTextAreaEntity));
            mNode->SetPosition(mPosition);
            mNode->SetVisible(FALSE);
            mNode->SetColor(NBRE_Color(1, 1, 1, mAlpha));
            if (mIsWatermark)
            {
                mNode->SetDrawIndex(-1);
            }
            mParentNode->AddChild(mNode);
        }
    }
    if (mNode)
    {
        mNode->SetScale(NBRE_Vector3f(static_cast<float>(mScale), static_cast<float>(mScale), static_cast<float>(mScale)));
        mBoundingBox = mNode->AxisAlignedBox();
    }
    
    return result;
}

void 
NBGM_PointLabel::SetColor(const NBRE_Color& color)
{
    if (color.a != mAlpha)
    {
        if (mNode)
        {
            mNode->SetColor(color);
        }
        mAlpha = color.a;
    }
}

double
NBGM_PointLabel::UpdateScale(double pixelsPerUnit)
{
    return mFontMaterial.maxFontHeightPixels / pixelsPerUnit;
}

void
NBGM_PointLabel::AddToSence()
{
    NBRE_Vector3d worldPosition = mParentNode->WorldPosition() + mPosition;
    mBoundingBox.minExtend = worldPosition;
    mBoundingBox.maxExtend = worldPosition;
    mMinPixelPerUnit = 0;
}

void 
NBGM_PointLabel::SetVisible(nb_boolean value)
{
    mVisible = value;
    if (mNode)
    {
        mNode->SetVisible(value);
    }
}

void
NBGM_PointLabel::RefreshFontMaterial(const NBRE_Map<uint32, NBRE_FontMaterial>& materials)
{
    NBRE_Map<uint32, NBRE_FontMaterial>::const_iterator it = materials.find(mFontMaterialId);
    if (it == materials.end())
    {
        return;
    }

    mFontMaterial = it->second;
    if (mTextAreaEntity != NULL)
    {
        mTextAreaEntity->SetFontMaterial(mFontMaterial);
    }
}

void
NBGM_PointLabel::GetFontMaterialIds(NBRE_Set<uint32>& materialIds)
{
    materialIds.insert(mFontMaterialId);
}

NBGM_LayoutElement*
NBGM_PointLabel::Clone(NBRE_Node* parentNode)
{
    NBGM_PointLabel* label = NBRE_NEW NBGM_PointLabel(mNBGMContext, parentNode, mSubView, mOverlayId
        , mText, mFontMaterial, mFontMaterialId, mFontHeight, mPosition, mIsWatermark);
    label->SetLayoutPriority(mPriority);

    return label;
}

void
NBGM_PointLabel::OnUpdate()
{
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

    mScale = mFontMaterial.maxFontHeightPixels / pixelsPerUnit / mFontHeight;
    mNode->SetScale(NBRE_Vector3f((float)mScale, (float)mScale, (float)mScale));
    mBoundingBox = mNode->AxisAlignedBox();
}
