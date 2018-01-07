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
#include "nbgmstaticpoi.h"
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

NBGM_StaticPoi::NBGM_StaticPoi(NBGM_Context& nbgmContext,
                               NBRE_Node* parentNode,
                               NBRE_SurfaceSubView* subView,
                               int32 overlayId,
                               const NBRE_Vector2f& size,
                               NBRE_ShaderPtr shader,
                               NBRE_ShaderPtr selectedShader,
                               NBRE_ShaderPtr unselectedShader,
                               const NBRE_Vector3d& position,
                               const NBRE_String& id,
                               nb_boolean showLabel,
                               const NBRE_Vector2f& imageOffset,
                               const NBRE_Vector2f& bubbleOffset,
                               const NBRE_String& text,
                               const NBRE_FontMaterial& fontMaterial,
                               uint32 materialId,
                               float distanceToPoi,
                               float distanceToLabel,
                               float distanceToAnother,
                               int32 labelPriority)
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
    ,mShowLabel(showLabel)
    ,mImageOffset(imageOffset)
    ,mBubbleOffset(bubbleOffset)
    ,mFontMaterial(fontMaterial)
    ,mFontMaterialId(materialId)
    ,mDistanceToPoi(distanceToPoi)
    ,mDistanceToLabel(distanceToLabel)
    ,mDistanceToAnother(distanceToAnother)
    ,mPoiLabel(NULL)
{
    nbre_assert(parentNode != NULL && subView != NULL);
    SetTypeId(NBGM_LET_STATIC_POI);
    mText = text;

    if (showLabel)
    {
        mPoiLabel = NBRE_NEW NBGM_StaticPoiLabel(nbgmContext, this, text, fontMaterial, materialId);
        mPoiLabel->SetPriority(labelPriority);
    }
}

NBGM_StaticPoi::~NBGM_StaticPoi()
{
    if (mPoiLabel)
    {
        NBRE_DELETE mPoiLabel;
        mPoiLabel = NULL;
    }

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
NBGM_StaticPoi::OnUpdate()
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
NBGM_StaticPoi::IsElementVisible()
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
NBGM_StaticPoi::UpdateRegion()
{
    NBGM_LayoutManager& layoutManager = *(mNBGMContext.layoutManager);
    layoutManager.GetLayoutBuffer().UpdateRegion(mScreenRect, STATIC_POI_UPDATE_MASK);
}

nb_boolean
NBGM_StaticPoi::IsElementRegionAvailable()
{
    mLabelAvailable = FALSE;

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
    nb_boolean result = layoutManager.GetLayoutBuffer().IsRegionAvailable(sr, POINT_LABEL_CHECK_MASK);
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

    mLabelAvailable = TRUE;
    return result;
}

NBRE_Vector3d
NBGM_StaticPoi::GetPosition()
{
    return mPosition;
}

void
NBGM_StaticPoi::SetColor(const NBRE_Color& color)
{
    mNode->SetColor(color);
}

double
NBGM_StaticPoi::UpdateScale(double pixelsPerUnit)
{
    return mSize.x / pixelsPerUnit;
}

void
NBGM_StaticPoi::SetSelected(nb_boolean selected)
{
    if (selected && !mSelected)
    {
        mOriginalPriority = GetLayoutPriority();
        SetLayoutPriority(HIGHEST_PRIORITY);
    }
    else if (!selected && mSelected)
    {
        SetLayoutPriority(mOriginalPriority);
    }

    if (mPoiLabel)
    {
        mPoiLabel->SetSelected(selected);
    }
    mSelected = selected;
}

nb_boolean
NBGM_StaticPoi::GetScreenPosition(NBRE_Vector2f& position)
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
NBGM_StaticPoi::GetScreenBubblePosition(NBRE_Vector2f& position)
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
NBGM_StaticPoi::GetScreenBubbleOffset()
{
    return NBRE_Vector2f(mBubbleOffset.x * mSize.x, mBubbleOffset.y * mSize.y);
}

nb_boolean
NBGM_StaticPoi::HitTest(const NBRE_Vector2f& screenPosition, float radius)
{
    if (!mIsInFrustum || !mNode.get() || !mNode->IsVisible())
    {
        return FALSE;
    }

    const NBRE_Vector2d& sp = NBRE_TypeConvertd::Convert(screenPosition);
    NBRE_Vector2d r(radius, radius);
    if (NBRE_Intersectiond::HitTest(mScreenRect, NBRE_AxisAlignedBox2d(sp - r, sp + r)))
    {
        return TRUE;
    }

    return FALSE;
}

void
NBGM_StaticPoi::AddToSence()
{
    NBGM_LayoutElement::AddToSence();

    NBRE_Vector3d worldPosition = mParentNode->WorldPosition() + mPosition;
    mBoundingBox.minExtend = worldPosition;
    mBoundingBox.maxExtend = worldPosition;
    mMinPixelPerUnit = 0;
}

void
NBGM_StaticPoi::SetVisible(nb_boolean value)
{
    if (mNode.get())
    {
        mNode->SetVisible(value);
    }
}

void
NBGM_StaticPoi::UpdateScreenRect(const NBRE_Vector2d& screenPosition, double pixelsPerUnit)
{
    NBRE_Vector2d hs(mSize.x * 0.5, mSize.y * 0.5);
    NBRE_Vector2d offset(mImageOffset.x * mSize.x, mImageOffset.y * mSize.y);
    mScreenRect.minExtend = screenPosition - hs + offset;
    mScreenRect.maxExtend = screenPosition + hs + offset;

    if (mPoiLabel)
    {
        mPoiLabel->UpdateScreenRect(screenPosition, pixelsPerUnit);
    }
}

nb_boolean
NBGM_StaticPoi::IsTooCloseTo(NBGM_LayoutElement* elem)
{
    NBGM_LayoutElementType t = elem->GetTypeId();

    switch (t)
    {
    case NBGM_LET_STATIC_POI:
        if (mScreenRect.GetCenter().DistanceTo(elem->GetScreenRect().GetCenter()) < mDistanceToPoi)
        {
            return TRUE;
        }
        break;
    case NBGM_LET_MAP_ROAD_LABEL:
    case NBGM_LET_NAV_ROAD_LABEL:
    case NBGM_LET_POINT_LABEL:
    case NBGM_LET_AREA_LABEL:
    case NBGM_LET_BUILDING_LABEL:
        if (mScreenRect.GetCenter().DistanceTo(elem->GetScreenRect().GetCenter()) < mDistanceToLabel)
        {
            return TRUE;
        }
        break;
    case NBGM_LET_SHIELD:
    case NBGM_LET_POINT_SHIELD:
    case NBGM_LET_POI:
        if (mScreenRect.GetCenter().DistanceTo(elem->GetScreenRect().GetCenter()) < mDistanceToAnother)
        {
            return TRUE;
        }
        break;
    default:
        break;
    }
    return FALSE;
}

void
NBGM_StaticPoi::RefreshFontMaterial(const NBRE_Map<uint32, NBRE_FontMaterial>& materials)
{
    if (mPoiLabel)
    {
        mPoiLabel->RefreshFontMaterial(materials);
    }
}

void
NBGM_StaticPoi::GetFontMaterialIds(NBRE_Set<uint32>& materialIds)
{
    if (mPoiLabel)
    {
        mPoiLabel->GetFontMaterialIds(materialIds);
    }
}

NBGM_StaticPoiLabel::NBGM_StaticPoiLabel(NBGM_Context& nbgmContext,
                                         NBGM_StaticPoi* poi,
                                         const NBRE_String& /*text*/,
                                         const NBRE_FontMaterial& fontMaterial,
                                         uint32 materialId)
    :NBGM_LayoutElement(nbgmContext)
    ,mPoi(poi)
    ,mLabelPlace(LP_NONE)
    ,mFontMaterial(fontMaterial)
    ,mFontMaterialId(materialId)
    ,mSelected(FALSE)
{
    SetTypeId(NBGM_LET_STATIC_POI_LABEL);
    mText = mPoi->mText;
    for (uint32 i = 0; i < 3; ++i)
    {
        mTextAreaEntity[i] = NULL;
    }
}

NBGM_StaticPoiLabel::~NBGM_StaticPoiLabel()
{

}

void NBGM_StaticPoiLabel::UpdateRegion()
{
    NBGM_LayoutManager& layoutManager = *(mNBGMContext.layoutManager);
    layoutManager.GetLayoutBuffer().UpdateRegion(mScreenRect, POINT_LABEL_UPDATE_MASK);
}

nb_boolean NBGM_StaticPoiLabel::IsElementRegionAvailable()
{
    const NBRE_Vector2f& mSize = mPoi->mSize;

    if (!mPoi->IsEnable() || !mPoi->mLabelAvailable)
    {
        return FALSE;
    }

    NBGM_LayoutManager& layoutManager = *(mNBGMContext.layoutManager);

    // find all visible pois
    nb_boolean isPoiVisible = FALSE;
    NBRE_Vector<NBGM_StaticPoi*> pois;
    NBRE_Vector<NBGM_LayoutElement*>& elems = layoutManager.GetVisibleSet();
    for (NBRE_Vector<NBGM_LayoutElement*>::iterator it = elems.begin(); it != elems.end(); ++it)
    {
        NBGM_LayoutElement* e = *it;
        if (e->GetTypeId() == NBGM_LET_STATIC_POI)
        {
            pois.push_back((NBGM_StaticPoi*)e);
            if (mPoi == e)
            {
                isPoiVisible = TRUE;
            }
        }
    }
    // associate poi not visible, skip
    if (!isPoiVisible)
    {
        return FALSE;
    }

    NBRE_Vector3d worldPosition = mPoi->mParentNode->WorldPosition() + mPoi->mPosition;
    NBRE_Vector2d screenPosition;
    if(!layoutManager.WorldToScreen(worldPosition, screenPosition))
    {
        return FALSE;
    }

    NBRE_Vector2d padding;
    NBRE_Vector2d halfSize(mPoi->mSize.x * 0.5, mPoi->mSize.y * 0.5);
    NBRE_Vector2d offset(mPoi->mSize.x * mPoi->mImageOffset.x, mPoi->mSize.y * mPoi->mImageOffset.y);

    if (mBaseStringSize.x == 0)
    {
        mBaseStringSize = mNBGMContext.renderingEngine->Context().mFontManager->GetStringSize(mFontMaterial, 1, mText, TRUE, PAL_TA_CENTER);
    }

    // Label preferred position order
    const static LabelPlacement LABEL_PLACES[] = {LP_BOTTOM, LP_TOP, LP_RIGHT, LP_LEFT};

    NBRE_Vector<LabelPlacement> lps;
    if (mLabelPlace != LP_NONE)
    {
        // Try last place first
        lps.push_back(mLabelPlace);
    }
    // Try other places by order
    for (uint32 i = 0; i < 4; ++i)
    {
        if (LABEL_PLACES[i] != mLabelPlace)
        {
            lps.push_back(LABEL_PLACES[i]);
        }
    }

    if (GetDrawIndex() == INVALID_DRAW_INDEX)
    {
        double pad = layoutManager.GetLayoutBuffer().CellSize() * 4.0;
        padding.x = pad;
        padding.y = pad;
    }

    nb_boolean positionFound = FALSE;
    for (uint32 i = 0; i < lps.size(); ++i)
    {
        LabelPlacement labelPlace = lps[i];
        mLabelOffset = GetLabelOffset(labelPlace);
        NBRE_Vector2d lo(mLabelOffset.x * mSize.x, mLabelOffset.y * mSize.y);
        halfSize.x = mBaseStringSize.x * (mFontMaterial.maxFontHeightPixels * 0.5);
        halfSize.y = mBaseStringSize.y * (mFontMaterial.maxFontHeightPixels * 0.5);
        NBRE_AxisAlignedBox2d mLabelScreenRect;
        mLabelScreenRect.minExtend = screenPosition - halfSize + lo;
        mLabelScreenRect.maxExtend = screenPosition + halfSize + lo;
        NBRE_AxisAlignedBox2d sr(mLabelScreenRect.minExtend - padding, mLabelScreenRect.maxExtend + padding);
        if (layoutManager.GetLayoutBuffer().IsRegionAvailable(sr, STATIC_POI_LABEL_CHECK_MASK))
        {
            nb_boolean hitIcons = FALSE;
            for (uint32 pIdx = 0; pIdx < pois.size(); ++pIdx)
            {
                NBGM_StaticPoi* poi = pois[pIdx];
                if (poi != mPoi && NBRE_Intersectiond::HitTest(poi->mScreenRect, mLabelScreenRect))
                {
                    hitIcons = TRUE;
                    break;
                }
            }

            if (!hitIcons)
            {
                positionFound = TRUE;
                mLabelPlace = labelPlace;
                mScreenRect = mLabelScreenRect;
                break;
            }
        }
    }

    if (!positionFound)
    {
        return FALSE;
    }

    PAL_TEXT_ALIGN ALIGN_TYPES[] = {PAL_TA_CENTER, PAL_TA_LEFT, PAL_TA_RIGHT};
    if (mPoi->mNode.get())
    {
        if (mTextAreaEntity[0] == NULL)
        {
            for (uint32 i = 0; i < 3; ++i)
            {
                mTextAreaEntity[i] = NBRE_NEW NBRE_TextAreaEntity(mNBGMContext.renderingEngine->Context(), mText, mFontMaterial, mFontMaterial.maxFontHeightPixels / mSize.x, ALIGN_TYPES[i]);
                mTextAreaEntity[i]->SetTransform(NBRE_Transformationd::BuildTranslateMatrix(NBRE_Vector3d(mLabelOffset.x, mLabelOffset.y, 0)));
                mPoi->mNode->AddEntity(NBRE_BillboardEntityPtr(mTextAreaEntity[i]));
                mTextAreaEntity[i]->SetVisible(FALSE);
            }
        }
        else
        {
            for (uint32 i = 0; i < 3; ++i)
            {
                mTextAreaEntity[i]->SetTransform(NBRE_Transformationd::BuildTranslateMatrix(NBRE_Vector3d(mLabelOffset.x, mLabelOffset.y, 0)));
            }
        }
    }

    return TRUE;
}

void NBGM_StaticPoiLabel::AddToSence()
{
    NBGM_LayoutElement::AddToSence();

    NBRE_Vector3d worldPosition = mPoi->mParentNode->WorldPosition() + mPoi->mPosition;
    mBoundingBox.minExtend = worldPosition;
    mBoundingBox.maxExtend = worldPosition;
    mMinPixelPerUnit = 0;
}

void NBGM_StaticPoiLabel::SetVisible(nb_boolean value)
{
    if (mTextAreaEntity[0])
    {
        PAL_TEXT_ALIGN ALIGN_TYPES[] = {PAL_TA_CENTER, PAL_TA_LEFT, PAL_TA_RIGHT};
        for (uint32 i = 0; i < 3; ++i)
        {
            PAL_TEXT_ALIGN align = ALIGN_TYPES[i];
            nb_boolean entityVisible = FALSE;

            if (value)
            {
                switch (mLabelPlace)
                {
                case LP_LEFT:
                    if (align == PAL_TA_RIGHT)
                    {
                        entityVisible = TRUE;
                    }
                    break;
                case LP_RIGHT:
                    if (align == PAL_TA_LEFT)
                    {
                        entityVisible = TRUE;
                    }
                    break;
                case LP_BOTTOM:
                case LP_TOP:
                default:
                    if (align == PAL_TA_CENTER)
                    {
                        entityVisible = TRUE;
                    }
                    break;
                }
            }

            mTextAreaEntity[i]->SetVisible(entityVisible);
        }
    }
}

NBRE_Vector2d
NBGM_StaticPoiLabel::GetLabelOffset(LabelPlacement placement)
{
    double padding = MM_TO_PIXEL(STATIC_POI_LABEL_PADDING_MM, NBGM_GetConfig()->dpi) / mPoi->mSize.x;
    double dx = 0;
    double dy = 0;
    const NBRE_Vector2f& mSize = mPoi->mSize;
    const NBRE_Vector2f& mImageOffset = mPoi->mImageOffset;

    switch (placement)
    {
    case LP_TOP:
        dy = (mSize.y / mSize.x + mBaseStringSize.y * mFontMaterial.maxFontHeightPixels / mSize.x) * 0.5 + padding;
        break;
    case LP_BOTTOM:
        dy = -((mSize.y / mSize.x + mBaseStringSize.y * mFontMaterial.maxFontHeightPixels / mSize.x) * 0.5 + padding);
        break;
    case LP_LEFT:
        dx = -((1.0 + mBaseStringSize.x * mFontMaterial.maxFontHeightPixels / mSize.x) * 0.5 + padding);
        break;
    case LP_RIGHT:
        dx = (1.0 + mBaseStringSize.x * mFontMaterial.maxFontHeightPixels / mSize.x) * 0.5 + padding;
        break;
    default:
        break;
    }

    return NBRE_Vector2d(mImageOffset.x + dx, mImageOffset.y + dy);
}

nb_boolean
NBGM_StaticPoiLabel::HitTest(const NBRE_Vector2d& screenPosition)
{
    if (NBRE_Intersectiond::HitTest(mScreenRect, screenPosition))
    {
        return TRUE;
    }
    return FALSE;
}

void
NBGM_StaticPoiLabel::UpdateScreenRect(const NBRE_Vector2d& screenPosition, double /*pixelsPerUnit*/)
{
    const NBRE_Vector2f& mSize = mPoi->mSize;
    NBRE_Vector2d hs(mSize.x * 0.5, mSize.y * 0.5);

    NBRE_Vector2d lo(mLabelOffset.x * mSize.x, mLabelOffset.y * mSize.y);
    hs.x = mBaseStringSize.x * (mFontMaterial.maxFontHeightPixels * 0.5);
    hs.y = mBaseStringSize.y * (mFontMaterial.maxFontHeightPixels * 0.5);
    mScreenRect.minExtend = screenPosition - hs + lo;
    mScreenRect.maxExtend = screenPosition + hs + lo;
}

void
NBGM_StaticPoiLabel::RefreshFontMaterial(const NBRE_Map<uint32, NBRE_FontMaterial>& materials)
{
    NBRE_Map<uint32, NBRE_FontMaterial>::const_iterator it = materials.find(mFontMaterialId);
    if (it == materials.end())
    {
        return;
    }

    mFontMaterial = it->second;
    if (mTextAreaEntity[0] != NULL)
    {
        for (uint32 i = 0; i < 3; ++i)
        {
            mTextAreaEntity[i]->SetFontMaterial(mFontMaterial);
        }
    }
}

void
NBGM_StaticPoiLabel::GetFontMaterialIds(NBRE_Set<uint32>& materialIds)
{
    materialIds.insert(mFontMaterialId);
}

void
NBGM_StaticPoiLabel::SetSelected(nb_boolean selected)
{
    if (selected && !mSelected)
    {
        mOriginalPriority = GetLayoutPriority();
        SetLayoutPriority(HIGHEST_PRIORITY);
    }
    else if (!selected && mSelected)
    {
        SetLayoutPriority(mOriginalPriority);

    }
    mSelected = selected;
}
