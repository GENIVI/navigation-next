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
#include "nbgmshield.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbrerenderpal.h"
#include "nbrehardwarebuffer.h"
#include "nbreintersection.h"
#include "nbremath.h"
#include "nbretypeconvert.h"
#include "nbretransformutil.h"

NBGM_Shield::NBGM_Shield(NBGM_Context& nbgmContext, NBRE_Node* parentNode, NBRE_SurfaceSubView* subView, int32 overlayId, const NBRE_WString& text, const NBRE_FontMaterial& material, uint32 materialId, float fontHeight, NBRE_ShaderPtr image, const NBRE_Vector2f& textureSize, const NBRE_Vector2f& offset, const NBRE_Polyline3d& polyline)
:NBGM_LayoutElement(nbgmContext, text)
,mSubView(subView)
,mOverlayId(overlayId)
,mFontHeight(fontHeight)
,mFontMaterial(material)
,mFontMaterialId(materialId)
,mIsInitialized(FALSE)
,mScale(1.0f)
,mAlpha(1.0f)
,mParentNode(parentNode)
,mImage(image)
,mTextureSize(textureSize)
,mOffset(offset)
,mVisible(FALSE)
,mTextAreaEntity(NULL)
,mNeedUpdate(TRUE)
{
    nbre_assert(parentNode != NULL);
    SetTypeId(NBGM_LET_SHIELD);
    mPolyline = NBRE_NEW NBRE_Polyline3d(polyline);
}

NBGM_Shield::NBGM_Shield(NBGM_Context& nbgmContext, NBRE_Node* parentNode, NBRE_SurfaceSubView* subView, int32 overlayId, const NBRE_WString& text, const NBRE_FontMaterial& material, uint32 materialId, float fontHeight, NBRE_ShaderPtr image, const NBRE_Vector2f& textureSize, const NBRE_Vector2f& offset, const NBRE_Vector3d& position, nb_boolean needUpdate)
:NBGM_LayoutElement(nbgmContext, text)
,mSubView(subView)
,mOverlayId(overlayId)
,mPosition(position)
,mPolyline(NULL)
,mFontHeight(fontHeight)
,mFontMaterial(material)
,mFontMaterialId(materialId)
,mIsInitialized(FALSE)
,mScale(1.0f)
,mAlpha(1.0f)
,mParentNode(parentNode)
,mImage(image)
,mTextureSize(textureSize)
,mOffset(offset)
,mVisible(FALSE)
,mTextAreaEntity(NULL)
,mNeedUpdate(needUpdate)
{
    nbre_assert(parentNode != NULL);
    SetTypeId(NBGM_LET_POINT_SHIELD);
}

NBGM_Shield::~NBGM_Shield()
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

    if (mPolyline != NULL)
    {
        NBRE_DELETE mPolyline;
    }
}

nb_boolean
NBGM_Shield::IsElementVisible()
{
    if (!IsEnable())
    {
        return FALSE;
    }

    NBGM_LayoutManager& layoutManager = *(mNBGMContext.layoutManager);

    if (GetDrawIndex() != NBGM_LayoutElement::INVALID_DRAW_INDEX)
    {
    }
    else
    {
        if (!layoutManager.IsInFrustum(mBoundingBox))
        {
            return FALSE;
        }
    }

    return TRUE;
}

void
NBGM_Shield::UpdateRegion()
{
    NBGM_LayoutManager& layoutManager = *(mNBGMContext.layoutManager);
    NBRE_Vector2d screenPosition;
    if(!layoutManager.WorldToScreen(mPosition, screenPosition))
    {
        return;
    }

    double ppu = 0;
    if (!layoutManager.PixelsPerUnit(mPosition, ppu))
    {
        return;
    }
    float scale = (float)(mFontMaterial.minFontHeightPixels / ppu / mFontHeight);
    NBRE_Vector2d halfSize(mTextureSize.x * scale * ppu * 0.5, mTextureSize.y * scale * ppu * 0.5);
    mScreenRect.minExtend = screenPosition - halfSize;
    mScreenRect.maxExtend = screenPosition + halfSize;
    if (mPolyline)
    {
        layoutManager.AddShield(mText);
    }
    layoutManager.GetLayoutBuffer().UpdateRegion(mScreenRect, SHIELD_UPDATE_MASK);
}

nb_boolean
NBGM_Shield::IsElementRegionAvailable()
{
    if (!IsEnable())
    {
        return FALSE;
    }

    NBGM_LayoutManager& layoutManager = *(mNBGMContext.layoutManager);

    if (mPolyline)
    {
        if (layoutManager.IsShieldDisplayed(mText))
        {
            return FALSE;
        }

        // Get visible polyline parts
        NBGM_LayoutPolylineList pls;
        if (!layoutManager.WorldToScreen(*mPolyline, 1, 3, NAV_ROAD_LABEL_CHECK_MASK, 0, pls))
        {
            return FALSE;
        }

        // find best polyline
        NBGM_LayoutPolylinePosition bestPos;
        int32 bestIndex = -1;
        double minDistance = 0;
        for (uint32 i = 0; i < pls.size(); ++i)
        {
            NBGM_LayoutPolylinePosition pos;
            double dist = pls[i].DistanceTo(mLayoutRefPoint, &pos);
            if (bestIndex == -1 || dist < minDistance)
            {
                minDistance = dist;
                bestIndex = i;
                bestPos = pos;
            }
        }

        if (bestIndex == -1)
        {
            return FALSE;
        }

        NBGM_LayoutPolyline& layoutPolyline = pls[bestIndex];
        mPosition = layoutPolyline.PointAt(bestPos).WorldPosition();
    }

    NBRE_Vector3d position = mPosition;

    NBRE_Vector2d screenPosition;
    if(!layoutManager.WorldToScreen(position, screenPosition))
    {
        return FALSE;
    }

    double ppu = 0;
    if (!layoutManager.PixelsPerUnit(position, ppu))
    {
        return FALSE;
    }

    if (mNeedUpdate)
    {
        float strWidth = mNBGMContext.renderingEngine->Context().mFontManager->GetStringWidth(mFontMaterial, mFontHeight, mText);
        NBRE_Vector2f textureSize(mTextureSize.x * 2 * mFontHeight + strWidth, (mTextureSize.y * 2 + 1.0f) * mFontHeight);
        if (textureSize.x < textureSize.y)
        {
            textureSize.x = textureSize.y;
        }
        mTextureSize = textureSize;
        mOffset = NBRE_Vector2f(textureSize.x * mOffset.x, textureSize.y * mOffset.y);
        mNeedUpdate = FALSE;
    }
    float scale = (float)(mFontMaterial.minFontHeightPixels / ppu / mFontHeight);
    NBRE_Vector2d halfSize(mTextureSize.x * scale * ppu * 0.5, mTextureSize.y * scale * ppu * 0.5);
    NBRE_Vector2d padding(halfSize.x, halfSize.y);
    if (GetDrawIndex() != NBGM_LayoutElement::INVALID_DRAW_INDEX)
    {
        padding.x = 0;
        padding.y = 0;
    }
    mScreenRect.minExtend = screenPosition - halfSize - padding;
    mScreenRect.maxExtend = screenPosition + halfSize + padding;

    if (!NBRE_Intersectiond::HitTest(mScreenRect, layoutManager.GetScreenRect()))
    {
        return FALSE;
    }

    nb_boolean result = layoutManager.GetLayoutBuffer().IsRegionAvailable(mScreenRect, SHIELD_CHECK_MASK);
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
            mTextAreaEntity->SetTransform(NBRE_Transformationd::BuildTranslateMatrix(mOffset.x, mOffset.y, 0));
            NBRE_ImageEntity* imageEntity = NBRE_NEW NBRE_ImageEntity(mNBGMContext.renderingEngine->Context(), mTextureSize, mImage);
            mNode->AddEntity(NBRE_BillboardEntityPtr(imageEntity));
            mNode->AddEntity(NBRE_BillboardEntityPtr(mTextAreaEntity));
            mNode->SetPosition(mPosition - mParentNode->WorldPosition());
            mNode->SetVisible(FALSE);
            mParentNode->AddChild(mNode);

            mNode->SetColor(NBRE_Color(1, 1, 1, mAlpha));
            mNode->SetVisible(mVisible);
        }
        mNode->SetScale(NBRE_Vector3f(scale, scale, scale));
        mNode->SetPosition(mPosition - mParentNode->WorldPosition());
        mBoundingBox = mNode->AxisAlignedBox();
        
        return TRUE;
    }
    
    return FALSE;
}

void
NBGM_Shield::SetColor(const NBRE_Color& color)
{
    if (mAlpha != color.a)
    {   
        mAlpha = color.a;
        if (mNode)
        {
            mNode->SetColor(color);
        }
    }
}

NBRE_Vector3d 
NBGM_Shield::GetPosition()
{
    return mPosition;
}

void
NBGM_Shield::AddToSence()
{
    if (mPolyline)
    {
        mBoundingBox = mPolyline->AABB();
        const NBRE_AxisAlignedBox2d& screenRect = mNBGMContext.layoutManager->GetScreenRect();
        mLayoutRefPoint.x = (screenRect.minExtend.x + screenRect.maxExtend.x) * 0.5f;
        mLayoutRefPoint.y = screenRect.maxExtend.y;
    }
    else
    {
        mBoundingBox.minExtend = mPosition;
        mBoundingBox.maxExtend = mPosition;
    }
}

void
NBGM_Shield::SetVisible(nb_boolean value)
{
    mVisible = value;
    if (mNode)
    {
        mNode->SetVisible(value);
    }
}

void
NBGM_Shield::RefreshFontMaterial(const NBRE_Map<uint32, NBRE_FontMaterial>& materials)
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
NBGM_Shield::GetFontMaterialIds(NBRE_Set<uint32>& materialIds)
{
    materialIds.insert(mFontMaterialId);
}

void
NBGM_Shield::OnUpdate()
{
    double ppu = 0;
    if (mNBGMContext.transUtil->PixelsPerUnit(mPosition, ppu))
    {
        float scale = (float)(mFontMaterial.minFontHeightPixels / ppu / mFontHeight);
        if (mNode.get())
        {
            mNode->SetScale(NBRE_Vector3f(scale, scale, scale));
            mBoundingBox = mNode->AxisAlignedBox();
        }
    }
}
