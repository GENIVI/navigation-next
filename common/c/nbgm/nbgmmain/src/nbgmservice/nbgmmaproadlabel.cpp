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
#include "nbgmmaproadlabel.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbrerenderpal.h"
#include "nbrehardwarebuffer.h"
#include "nbreintersection.h"
#include "nbremath.h"
#include "nbreaxisalignedbox2.h"
#include "nbretypeconvert.h"
#include "nbrerenderengine.h"
#include "nbgmlabeldebugutil.h"
#include "nbretransformutil.h"

NBGM_MapRoadLabel::NBGM_MapRoadLabel(NBGM_Context& nbgmContext, int32 overlayId, NBRE_Node* parentNode, const NBRE_WString& text, const NBRE_FontMaterial& material, uint32 materialId, const NBRE_Polyline2f& polyline, float fontHeight, float width, uint64 tileId)
:NBGM_LayoutElement(nbgmContext, text)
,mOverlayId(overlayId)
,mPolyline(polyline)
,mWorldPolyline(polyline.VertexCount())
,mFontHeight(fontHeight)
,mFontMaterial(material)
,mFontMaterialId(materialId)
,mWidth(width)
,mAlpha(1.0f)
,mParentNode(parentNode)
,mLayoutPolyline(NULL)
,mVisible(FALSE)
,mBaseStringWidth(0)
,mTileId(tileId)
,mScaleFontHeight(0)
{
	SetTypeId(NBGM_LET_MAP_ROAD_LABEL);
	mPriority = 1;
}

NBGM_MapRoadLabel::~NBGM_MapRoadLabel()
{
    if (mTextPathEntity)
    {
        mParentNode->DetachObject(mTextPathEntity.get());
    }
    
    NBRE_DELETE mLayoutPolyline;
}

nb_boolean
NBGM_MapRoadLabel::IsElementVisible()
{   
    NBGM_LayoutManager& layoutManager = *(mNBGMContext.layoutManager);

    if (!IsEnable())
    {
        return FALSE;
    }

    if (mTextPathEntity.get() != NULL && !layoutManager.IsInFrustum(mTextPathEntity->AxisAlignedBox()))
    {
        return FALSE;
    }
    else if (!layoutManager.IsInFrustum(mBoundingBox))
    {
        return FALSE;
    }

    return TRUE;
}

void 
NBGM_MapRoadLabel::UpdateRegion()
{
    NBGM_LayoutManager& layoutManager = *(mNBGMContext.layoutManager);
    layoutManager.UpdateRegion(mWorldPolyline, 3, MAP_ROAD_LABEL_UPDATE_MASK);
}

nb_boolean 
NBGM_MapRoadLabel::IsElementRegionAvailable()
{
    NBGM_LayoutManager& layoutManager = *(mNBGMContext.layoutManager);

    NBRE_Vector<NBRE_Polyline2d> screenPls;
    if (layoutManager.WorldToScreen(mWorldPolyline, screenPls))
    {
        nb_boolean isDisplayed = GetDrawIndex() == NBGM_LayoutElement::INVALID_DRAW_INDEX ? TRUE : FALSE;
        for (uint32 i = 0; i < screenPls.size(); ++ i)
        {
            if (!layoutManager.GetLayoutBuffer().IsRegionAvailable(screenPls[i], isDisplayed ? 7 : 3, MAP_ROAD_LABEL_CHECK_MASK) )
            {
                return FALSE;
            }
        }
    }

    if (!mTextPathEntity)
    {
        mTextPathEntity = NBRE_TextPath3dEntityPtr(NBRE_NEW NBRE_TextPath3dEntity(mNBGMContext.renderingEngine->Context(), mNBGMContext.transUtil, mOverlayId, mText, mFontMaterial, mFontHeight, &mPolyline, 0));
        mTextPathEntity->SetVisible(FALSE);
        mTextPathEntity->SetColor(NBRE_Color(1, 1, 1, mAlpha));
        mParentNode->AttachObject(mTextPathEntity);
    }

    return TRUE;
}

NBRE_Vector3d 
NBGM_MapRoadLabel::GetPosition()
{
    return mPosition;
}

void 
NBGM_MapRoadLabel::SetColor(const NBRE_Color& color)
{
    if (mAlpha != color.a)
    {
        if (mTextPathEntity)
        {
            mTextPathEntity->SetColor(color);
        }        
        mAlpha = color.a;
    }
}

void
NBGM_MapRoadLabel::AddToSence()
{
    mBoundingBox.SetNull();
    const NBRE_Vector3d& refCenter = mParentNode->WorldPosition();
    for (uint32 i = 0; i < mPolyline.VertexCount(); ++i)
    {
        const NBRE_Vector2f v = mPolyline.Vertex(i);
        mBoundingBox.Merge(refCenter.x + v.x, refCenter.y + v.y, refCenter.z);
        mWorldPolyline.SetVertex(i, NBRE_Vector3d(refCenter.x + v.x, refCenter.y + v.y, refCenter.z));
    }
    mBoundingBox.minExtend.x -= mWidth;
    mBoundingBox.minExtend.y -= mWidth;
    mBoundingBox.maxExtend.x += mWidth;
    mBoundingBox.maxExtend.y += mWidth;

    mWorldPolyline.UpdateLength();
    mPosition = mBoundingBox.GetCenter();
}

void
NBGM_MapRoadLabel::SetVisible(nb_boolean value)
{
    mVisible = value;
    if (mTextPathEntity)
    {
        mTextPathEntity->SetVisible(value);
    }
}

void
NBGM_MapRoadLabel::RefreshFontMaterial(const NBRE_Map<uint32, NBRE_FontMaterial>& materials)
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
NBGM_MapRoadLabel::GetFontMaterialIds(NBRE_Set<uint32>& materialIds)
{
    materialIds.insert(mFontMaterialId);
}
