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
#include "nbgmroadlabel.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbrerenderpal.h"
#include "nbrehardwarebuffer.h"
#include "nbreintersection.h"
#include "nbremath.h"
#include "nbreaxisalignedbox2.h"
#include "nbretypeconvert.h"
#include "nbrerenderengine.h"
#include "nbretransformutil.h"

static const float TEXT_PATH_MAX_ANGLE_TOLERANCE = 10.0f * NBRE_Math::Deg2Rad;
static const double MAX_DIRECTION_CHANGE = nsl_cos(30.0);

static nb_boolean
IsCurvyRoad(const NBRE_Polyline2d& polyline)
{
    if (polyline.VertexCount() > 2)
    {
        NBRE_Vector2d initDir(polyline.Vertex(1) - polyline.Vertex(0));
        initDir.Normalise();
        NBRE_Vector2d lastDir = initDir;

        for (uint32 i = 2; i < polyline.VertexCount(); ++i)
        {
            NBRE_Vector2d curDir(polyline.Vertex(i) - polyline.Vertex(i - 1));
            curDir.Normalise();

            double dotCurInit = curDir.DotProduct(initDir);
            if (dotCurInit < MAX_DIRECTION_CHANGE)
            {
                return TRUE;
            }
            if (curDir.DotProduct(lastDir) < MAX_DIRECTION_CHANGE)
            {
                return TRUE;
            }
            lastDir = curDir;
        }
    }
    return FALSE;
}


NBGM_RoadLabel::NBGM_RoadLabel(NBGM_Context& nbgmContext, NBRE_Node* parentNode, NBRE_SurfaceSubView* subView, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, int32 overlayId, const NBRE_WString& text, const NBRE_FontMaterial& material, uint32 materialId, const NBRE_Polyline2f& polyline, float width)
:NBGM_LayoutElement(nbgmContext, text)
,mSubView(subView)
,mOverlayManager(overlayManager)
,mOverlayId(overlayId)
,mPolyline(polyline.VertexCount())
,mFontHeight(material.minFontHeightPixels)
,mFontMaterial(material)
,mFontMaterialId(materialId)
,mTextPathEntity(NULL)
,mAlign(NBRE_FA_BEGIN)
,mWidth(width)
,mLayoutPolyline(&nbgmContext, polyline.VertexCount())
,mScreenPolyline(polyline.VertexCount())
,mAlpha(1.0f)
,mVisible(FALSE)
,mTextDirection(1, 0)
,mParentNode(parentNode)
,mBaseStringWidth(0)
{
    mPriority = 1;

	mLayoutOffset[0] = 0.0;
    mLayoutOffset[1] = 0.0;

    SetTypeId(NBGM_LET_NAV_ROAD_LABEL);
    const NBRE_Vector3d& refPoint = parentNode->WorldPosition();
    for (uint32 i = 0; i < polyline.VertexCount(); ++i)
    {
        const NBRE_Vector2f& v = polyline.Vertex(i);
        mPolyline.SetVertex(i, NBRE_Vector3d(v.x + refPoint.x, v.y + refPoint.y, refPoint.z));
    }
    mPolyline.UpdateLength();
}

NBGM_RoadLabel::~NBGM_RoadLabel()
{
    if(mTextPathEntity)
    {
        NBRE_Overlay* overlay = mOverlayManager.FindOverlay(mOverlayId);
        if (overlay)
        {
            overlay->RemoveRenderProvider(mTextPathEntity);
        }
        NBRE_DELETE mTextPathEntity;
    }
}

nb_boolean
NBGM_RoadLabel::IsElementVisible()
{
    NBGM_LayoutManager& layoutManager = *(mNBGMContext.layoutManager);

    if (!IsEnable())
    {
        return FALSE;
    }

    if (GetDrawIndex() != NBGM_LayoutElement::INVALID_DRAW_INDEX)
    {
        if (!layoutManager.IsInFrustum(mLayoutPolyline.GetWorldPolyline()))
        {
            return FALSE;
        }

        if (!layoutManager.IsInNearPlaneFrontSide(mLayoutPolyline.GetWorldPolyline()))
        {
            return FALSE;
        }

        mLayoutPolyline.UpdateScreenPositionFromWorldPosition();

        double pixelPerUnit = 0;
        if (!layoutManager.PixelsPerUnit(mPosition, pixelPerUnit))
        {
            return FALSE;
        }

        mFontHeight = (float)NBRE_Math::Clamp<double>(pixelPerUnit * mWidth, mFontMaterial.minFontHeightPixels, mFontMaterial.maxFontHeightPixels);

        // split layout polyline
        double strWidth = mFontHeight * mBaseStringWidth;
        double plLength = mLayoutPolyline.GetScreenPolyline().Length();
        
        if (mAlign == NBRE_FA_END)
        {
            mLayoutPolyline = mLayoutPolyline.SubPolyline(plLength - strWidth, plLength);
        }
        else
        {
            mLayoutPolyline = mLayoutPolyline.SubPolyline(0, strWidth);
        }

        mScreenPolyline = NBRE_TypeConvertf::Convert(mLayoutPolyline.GetScreenPolyline());
        mTextPathEntity->Rebuild(mFontHeight, IsRevert(), &mScreenPolyline, 0);

        const NBGM_LayoutVertex& vertex = mLayoutPolyline.PointAt(mLayoutPolyline.GetScreenPolyline().Length() * 0.5);
        mPosition = vertex.WorldPosition();
    }
    else
    {
        if (!layoutManager.IsInFrustum(mPolyline))
        {
            return FALSE;
        }
    }

    return TRUE;
}

void
NBGM_RoadLabel::UpdateRegion()
{
    NBGM_LayoutManager& layoutManager = *(mNBGMContext.layoutManager);
    const NBRE_Polyline2d& pl = mLayoutPolyline.GetScreenPolyline();
    layoutManager.GetLayoutBuffer().UpdateRegion(pl, 3, 0x1);
    layoutManager.AddRoadLabel(mText);
}

void
NBGM_RoadLabel::OnUpdate()
{
    if (GetDrawIndex() != NBGM_LayoutElement::INVALID_DRAW_INDEX)
    {
        if (!mNBGMContext.transUtil->IsInFrustum(mLayoutPolyline.GetWorldPolyline()))
        {
            RemoveFromVisibleSet();
            return;
        }

        if (!mNBGMContext.transUtil->IsInNearPlaneFrontSide(mLayoutPolyline.GetWorldPolyline()))
        {
            RemoveFromVisibleSet();
            return;
        }

        mLayoutPolyline.UpdateScreenPositionFromWorldPosition();

        double pixelPerUnit = 0;
        if (!mNBGMContext.transUtil->PixelsPerUnit(mPosition, pixelPerUnit))
        {
            RemoveFromVisibleSet();
            return;
        }

        mFontHeight = (float)NBRE_Math::Clamp<double>(pixelPerUnit * mWidth, mFontMaterial.minFontHeightPixels, mFontMaterial.maxFontHeightPixels);

        // split layout polyline
        double strWidth = mFontHeight * mBaseStringWidth;
        double plLength = mLayoutPolyline.GetScreenPolyline().Length();

        if (mAlign == NBRE_FA_END)
        {
            mLayoutPolyline = mLayoutPolyline.SubPolyline(plLength - strWidth, plLength);
        }
        else
        {
            mLayoutPolyline = mLayoutPolyline.SubPolyline(0, strWidth);
        }

        mScreenPolyline = NBRE_TypeConvertf::Convert(mLayoutPolyline.GetScreenPolyline());
        mTextPathEntity->Rebuild(mFontHeight, IsRevert(), &mScreenPolyline, 0);
    }
}

nb_boolean
NBGM_RoadLabel::IsElementRegionAvailable()
{
    NBGM_LayoutManager& layoutManager = *(mNBGMContext.layoutManager);

    if (!IsEnable())
    {
        return FALSE;
    }

    if (layoutManager.IsRoadLabelDisplayed(mText))
    {
        return FALSE;
    }

    double ppuMax = 0;
    for (uint32 i = 0; i < mPolyline.VertexCount(); ++i)
    {
        double ppu = 0;
        if (layoutManager.PixelsPerUnit(mPolyline.Vertex(i), ppu))
        {
            if (ppu > ppuMax)
            {
                ppuMax = ppu;
            }
        }
    }

    mFontHeight = (float)NBRE_Math::Clamp<double>(ppuMax * mWidth, mFontMaterial.minFontHeightPixels, mFontMaterial.maxFontHeightPixels);
    double padding = mFontHeight;
    double strWidth = mFontHeight * mBaseStringWidth + padding * 2;
    NBGM_LayoutPolylineList pls;
    if (!layoutManager.WorldToScreen(mPolyline, mWidth, 3, 0x3, strWidth, pls))
    {
        return FALSE;
    }

    // find best polyline
    double bestDist = 0;
    int32 bestIndex = -1;
    NBGM_LayoutPolylinePosition bestPos;
    for (uint32 i = 0; i < pls.size(); ++i)
    {
        const NBGM_LayoutPolyline& pl = pls[i];
        if (pl.GetScreenPolyline().Length() < strWidth
            || IsCurvyRoad(pl.GetScreenPolyline()))
        {
            continue;
        }

        NBGM_LayoutPolylinePosition pos;
        double dist = pl.DistanceTo(mLayoutRefPoint, &pos);
        if (bestIndex == -1 || dist < bestDist)
        {
            bestIndex = i;
            bestDist = dist;
            bestPos = pos;
        }
    }

    if (bestIndex == -1)
    {
        return FALSE;
    }

    mLayoutPolyline = pls[bestIndex];
    
    double bestOffset = mLayoutPolyline.ConvertParameterCoordinateToOffset(bestPos);
    double plLen = mLayoutPolyline.GetScreenPolyline().Length();

    double centerOffset = bestOffset;
    if (centerOffset - strWidth * 0.5 < 0)
    {
        centerOffset = strWidth * 0.5;
    }
    if (centerOffset + strWidth * 0.5 > plLen)
    {
        centerOffset = plLen - strWidth * 0.5;
    }
    if (centerOffset >= bestOffset)
    {
        mAlign = NBRE_FA_BEGIN;
    }
    else
    {
        mAlign = NBRE_FA_END;
    }
    double w = (strWidth - padding * 2) * 0.5;
    mLayoutPolyline = mLayoutPolyline.SubPolyline(centerOffset - w, centerOffset + w);

    const NBGM_LayoutVertex& vertex = mLayoutPolyline.PointAt(mLayoutPolyline.GetScreenPolyline().Length() * 0.5);
    mPosition = vertex.WorldPosition();

    mScreenPolyline = NBRE_TypeConvertf::Convert(mLayoutPolyline.GetScreenPolyline());

    if (mTextPathEntity == NULL)
    {   
        NBRE_Overlay* overlay = mOverlayManager.FindOverlay(mOverlayId);
        if (!overlay)
        {
            return FALSE;
        }
        mTextPathEntity = NBRE_NEW NBRE_TextPathEntity(mNBGMContext.renderingEngine->Context(), mText, mFontMaterial, mFontHeight, &mScreenPolyline, 0);
        overlay->AppendRenderProvider(mTextPathEntity);
    }
    return TRUE;
}

NBRE_Vector3d 
NBGM_RoadLabel::GetPosition()
{
    return mPosition;
}

void 
NBGM_RoadLabel::SetColor(const NBRE_Color& color)
{
    if (mAlpha != color.a)
    {
        if (mTextPathEntity != NULL)
        {
            mTextPathEntity->SetColor(color);
        }
        mAlpha = color.a;
    }
}

void
NBGM_RoadLabel::AddToSence()
{
    NBGM_LayoutElement::AddToSence();

    const NBRE_AxisAlignedBox2d& screenRect = mNBGMContext.layoutManager->GetScreenRect();
    mLayoutRefPoint.x = (screenRect.minExtend.x + screenRect.maxExtend.x) * 0.5f;
    mLayoutRefPoint.y = screenRect.maxExtend.y;

    mPosition = mPolyline.PointAt(mPolyline.Length() * 0.5f);
    mBoundingBox = mPolyline.AABB();

    mBaseStringWidth = NBRE_TextEntity::GetStringWidth(mNBGMContext.renderingEngine->Context(), mFontMaterial, 1, mText);
    mMinPixelPerUnit = (float)(mBaseStringWidth * mFontMaterial.minFontHeightPixels / mPolyline.Length());
}

void
NBGM_RoadLabel::SetVisible(nb_boolean value)
{
    mVisible = value;
    if (mTextPathEntity != NULL)
    {
        mTextPathEntity->SetVisible(value);
    }
}

void
NBGM_RoadLabel::RefreshFontMaterial(const NBRE_Map<uint32, NBRE_FontMaterial>& materials)
{
    NBRE_Map<uint32, NBRE_FontMaterial>::const_iterator it = materials.find(mFontMaterialId);
    if (it == materials.end())
    {
        return;
    }

    mFontMaterial = it->second;
    if (mTextPathEntity)
    {
        mTextPathEntity->SetFontMaterial(mFontMaterial);
    }
}

void
NBGM_RoadLabel::GetFontMaterialIds(NBRE_Set<uint32>& materialIds)
{
    materialIds.insert(mFontMaterialId);
}

nb_boolean 
NBGM_RoadLabel::IsRevert()
{
    NBRE_Vector2f up(0, 1);
    nb_boolean isRevert = mTextPathEntity->IsRevert();

    NBRE_Vector2f dir = mScreenPolyline.Vertex(mScreenPolyline.VertexCount() - 1) - mScreenPolyline.Vertex(0);
    dir.Normalise();

    // Check if direction in tolerance angle range
    float cosTolerance = (float)nsl_cos(TEXT_PATH_MAX_ANGLE_TOLERANCE);
    float cosOldDirUp = mTextDirection.DotProduct(up);
    float cosNewDirUp = dir.DotProduct(up);
    if (!((cosOldDirUp >= cosTolerance && cosNewDirUp >= cosTolerance) ||
        (cosOldDirUp <= -cosTolerance && cosNewDirUp <= cosTolerance)))
    {
        // angle not in tolerance range
        NBRE_Vector2f right(up.y, -up.x);
        isRevert = dir.DotProduct(right) >= 0 ? FALSE : TRUE;
    }
    mTextDirection = dir;
    return isRevert;
}

NBRE_Polyline2f
NBGM_RoadLabel::GetPolyline()
{
    const NBRE_Vector3d& refPoint = mParentNode->WorldPosition();
    NBRE_Polyline2f result(mPolyline.VertexCount());

    for (uint32 i = 0; i < mPolyline.VertexCount(); ++i)
    {
        const NBRE_Vector3d& v = mPolyline.Vertex(i);
        result.SetVertex(i, NBRE_Vector2f((float)(v.x - refPoint.x), (float)(v.y - refPoint.y)));
    }
    result.UpdateLength();

    return result;
}
