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
#include "nbgmlabellayer.h"
#include "nbgmmapmaterialmanager.h"
#include "nbgmpointlabel.h"
#include "nbgmshield.h"
#include "nbgmroadlabel.h"
#include "nbgmmaproadlabel.h"
#include "nbremath.h"
#include "nbretypeconvert.h"
#include "nbgmconst.h"
#include "nbgm.h"
#include "nbgmdebugpointlabel.h"
#include "nbgmmapviewprofiler.h"
#include "nbgmmapviewimpl.h"

NBGM_LabelLayer::NBGM_LabelLayer(NBGM_Context& nbgmContext, NBRE_Node& parentNode, int32 billboardOverlayId, NBRE_BillboardSet* billboardSet, NBRE_Overlay* textOverlay, nb_boolean useNavRoadLabel
                                 , NBRE_SurfaceSubView* subView, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, NBRE_CameraPtr screenCamera, NBGM_MapViewProfiler* mapViewProfiler, int32 layerStride, int32 layerMain
                                 , shared_ptr<WorkerTaskQueue> loadingThread, shared_ptr<NBGM_TaskQueue> renderThread, NBGM_MapViewImpl* mapViewImpl)
    :mNBGMContext(nbgmContext)
    ,mParentNode(parentNode)
    ,mLayerDataCreator(nbgmContext, this, billboardOverlayId, billboardSet, textOverlay, &mPriority, useNavRoadLabel, subView)
    ,mUseNavRoadLabel(useNavRoadLabel)
    ,mSubView(subView)
    ,mOverlayManager(overlayManager)
    ,mScreenCamera(screenCamera)
    ,mMapViewProfiler(mapViewProfiler)
    ,mLayerStride(layerStride)
    ,mLayerMain(layerMain)
    ,mExtendType(NBGM_TMBET_NONE)
    ,mLoadingThread(loadingThread)
    ,mRenderThread(renderThread)
    ,mMapViewImpl(mapViewImpl)
{
    mNBGMContext.layoutManager->AddLayoutProvider(this);
}

NBGM_LabelLayer::~NBGM_LabelLayer()
{
    TileMap::iterator pTileMapEnd = mTileMap.end();
    TileMap::iterator iter;
    for (iter = mTileMap.begin(); iter != pTileMapEnd; ++iter)
    {
        if(iter->second->Node())
        {
            mParentNode.RemoveChild(iter->second->Node().get());
        }
        NBRE_DELETE iter->second;
    }

#ifdef NBGM_SHOW_TILE_NAME
    for (DebugLabelMap::iterator i = mDebugLabels.begin(); i != mDebugLabels.end(); ++i)
    {
        NBRE_DELETE i->second;
    }
#endif
    mNBGMContext.layoutManager->RemoveLayoutProvider(this);
}

#ifdef NBGM_SHOW_TILE_NAME
static NBRE_WString 
GetTileName(const NBGM_VectorTileID& tileId)
{
    NBRE_WString str = NBRE_Font::ToUnicode(tileId.c_str());
    uint32 p = str.find_last_of(L'/');
    if (p != NBRE_WString::npos)
    {
        str = str.substr(p + 1);
    }
    p = str.find_last_of(L'\\');
    if (p != NBRE_WString::npos)
    {
        str = str.substr(p + 1);
    }
    return str;
}
#endif

NBGM_IMapLayerTileCreator&
NBGM_LabelLayer::GetTileCreator()
{
    return mLayerDataCreator;
}

void
NBGM_LabelLayer::OnAddTile(const NBGM_VectorTileID& tileId, NBGM_IMapLayerTile* tile)
{
    if(tile != NULL)
    {
        nbre_assert(mTileMap.find(tileId) == mTileMap.end());
        NBGM_LabelLayerTile* tileData = static_cast<NBGM_LabelLayerTile*> (tile);

        int32 layerId = mOverlayManager.AssignOverlayId(DrawOrderStruct(tileData->GetLabelDrawOrder(), 0, 0));
        mSubView->AddBillboardSet(layerId);

//        mSubView->Overlay(layerId)->SetClearFlag(NBRE_CLF_CLEAR_DEPTH);
        mMapViewProfiler->AddRenderQueue(tileData->GetLabelDrawOrder(), *mOverlayManager.FindOverlay(layerId));

        layerId = mOverlayManager.AssignOverlayId(DrawOrderStruct(tileData->GetBaseDrawOrder(), tileData->GetSubDrawOrder(), 0));
        mSubView->AddBillboardSet(layerId);

        mOverlayManager.AssignOverlayId(DrawOrderStruct(tileData->GetBaseDrawOrder(), tileData->GetSubDrawOrder(), 2));

        if (mUseNavRoadLabel)
        {
            layerId = mOverlayManager.AssignOverlayId(DrawOrderStruct(tileData->GetLabelDrawOrder(), 0, 1));
            NBRE_Overlay* ol = mOverlayManager.FindOverlay(layerId);
            ol->SetCamera(mScreenCamera);
            ol->SetAutoClearRenderProvider(FALSE);
        }
        tileData->UpdateLayersLayoutState(mLayersLayoutState);

        tileData->AddToScene();
        mParentNode.AddChild(tileData->Node());
        mTileMap[tileId] = tileData;

    #ifdef NBGM_SHOW_TILE_NAME
        for (DebugLabelMap::iterator i = mDebugLabels.begin(); i != mDebugLabels.end(); ++i)
        {
            i->second->AddToScene();
        }
    #endif
    }

}

void
NBGM_LabelLayer::OnRemoveTile(const NBGM_VectorTileID& tileId)
{
    TileMap::iterator i = mTileMap.find(tileId);
    if (i != mTileMap.end())
    {
        if(i->second->Node())
        {
            mParentNode.RemoveChild(i->second->Node().get());
        }
        NBRE_DELETE i->second;
        mTileMap.erase(i);
    }
}

void 
NBGM_LabelLayer::OnActiveTile(const NBGM_VectorTileID& tileId)
{
    TileMap::iterator i = mTileMap.find(tileId);
    if (i != mTileMap.end())
    {
        i->second->OnActive();
    }
}

void 
NBGM_LabelLayer::OnDeactiveTile(const NBGM_VectorTileID& tileId)
{
    TileMap::iterator i = mTileMap.find(tileId);
    if (i != mTileMap.end())
    {
        i->second->OnDeactive();
    }
}

void
NBGM_LabelLayer::UpdateLayoutElementVisibility(float viewPointDistance)
{
    TileMap::iterator pTileMap = mTileMap.begin();
    TileMap::iterator pTileMapEnd = mTileMap.end();
    for (; pTileMap != pTileMapEnd; ++pTileMap)
    {
        pTileMap->second->UpdateLayoutElementVisibility(viewPointDistance);
    }
}

void
NBGM_LabelLayer::RefreshFontMaterial(const NBRE_String& materialCategoryName, const NBRE_Map<uint32, NBRE_FontMaterial>& materials)
{
    mFontMaterials[materialCategoryName] = materials;

    for (TileMap::iterator i = mTileMap.begin(); i != mTileMap.end(); ++i)
    {
        if (i->second->GetMaterialCategoryName() == materialCategoryName)
        {
            i->second->RefreshFontMaterial(materials);
        }
    }
    mNBGMContext.layoutManager->Invalidate();
}

void
NBGM_LabelLayer::GetFontMaterialIds(const NBRE_String& materialCategoryName, NBRE_Set<uint32>& materialIds)
{
    for (TileMap::iterator i = mTileMap.begin(); i != mTileMap.end(); ++i)
    {
        if (i->second->GetMaterialCategoryName() == materialCategoryName)
        {
            i->second->GetFontMaterialIds(materialIds);
        }
    }
}

void
NBGM_LabelLayer::NotifyExtend(const NBGM_TileMapBoundaryExtendType& extendType)
{
    if (mExtendType != extendType)
    {
        for (TileMap::iterator i = mTileMap.begin(); i != mTileMap.end(); ++i)
        {
            i->second->NotifyExtend(extendType);
        }
        mExtendType = extendType;
    }
}

void
NBGM_LabelLayer::UpdateLayoutList(NBGM_LayoutElementList& layoutElements)
{
    TileMap::iterator pTileMap = mTileMap.begin();
    TileMap::iterator pTileMapEnd = mTileMap.end();
    for (; pTileMap != pTileMapEnd; ++pTileMap)
    {
        pTileMap->second->UpdateLayoutList(layoutElements, mLoadingThread, mRenderThread, this);
    }
}

void 
NBGM_LabelLayer::OnLayoutElementsStart()
{
    mMapViewImpl->AddLayoutTaskRef();
}
nb_boolean
NBGM_LabelLayer::OnLayoutElementsDone(uint64 id, int32 level, NBRE_Vector<NBGM_StaticLayoutResult*>* layoutResult)
{
    NBGM_LayoutManager& layoutManager = *mNBGMContext.layoutManager;
    double epsilon = layoutManager.GetLayoutLevels().GetLevelMercatorPerPixel(level)
                   * layoutManager.GetLayoutBuffer().CellSize() * 2.0;
    TileMap::iterator pTileMap = mTileMap.begin();
    TileMap::iterator pTileMapEnd = mTileMap.end();
    for (; pTileMap != pTileMapEnd; ++pTileMap)
    {
        NBGM_LabelLayerTile* tile = pTileMap->second;
        if (tile->OnLayoutElementsDone(id, level, layoutResult))
        {
            for (TileMap::iterator it = mTileMap.begin(); it != mTileMap.end(); ++it)
            {
                NBGM_LabelLayerTile* tile2 = it->second;
                if (tile != tile2)
                {   
                    tile->ResolveStaticConflicts(level, tile2, epsilon);
                }
            }
            mMapViewImpl->InvalidateLayout();
            mMapViewImpl->ReleaseLayoutTaskRef();
            return TRUE;
        }
    }
    mMapViewImpl->ReleaseLayoutTaskRef();
    return FALSE;
}

nb_boolean
NBGM_LabelLayer::GetStaticPoiInfo(const NBRE_String& id, NBGM_StaticPoiInfo& info)
{
    for (TileMap::iterator it = mTileMap.begin(); it != mTileMap.end(); ++it)
    {
        NBGM_LabelLayerTile* tileData = it->second;
        if (tileData && tileData->GetStaticPoiInfo(id, info))
        {
            return TRUE;
        }
    }
    return FALSE;
}

void
NBGM_LabelLayer::SelectStaticPoi(const NBRE_String& id, nb_boolean selected)
{
    for (TileMap::iterator it = mTileMap.begin(); it != mTileMap.end(); ++it)
    {
        NBGM_LabelLayerTile* tileData = it->second;
        if (tileData)
        {
            tileData->SelectStaticPoi(id, selected);
        }
    }
}

void
NBGM_LabelLayer::GetElementsAt(NBGM_LayoutElementType type, const NBRE_Point2f& screenPosition, float radius, NBRE_Vector<NBRE_String>& ids)
{
    NBRE_Point2f sp(screenPosition.x, screenPosition.y);
    for (TileMap::iterator it = mTileMap.begin(); it != mTileMap.end(); ++it)
    {
        NBGM_LabelLayerTile* tileData = it->second;
        if(tileData)
        {
            tileData->GetElementsAt(type, sp, radius, ids);
        }
    }
}

void
NBGM_LabelLayer::EnableLayerLayout(const NBRE_String& layerId, nb_boolean enable)
{
    mLayersLayoutState[layerId] = enable;
    
    for (TileMap::iterator it = mTileMap.begin(); it != mTileMap.end(); ++it)
    {
        NBGM_LabelLayerTile* tileData = it->second;
        if(tileData)
        {
            tileData->EnableLayerLayout(layerId, enable);
        }
    }
}

const NBRE_FontMaterial*
NBGM_LabelLayer::GetFontMaterial(const NBRE_String& category, uint32 id) const
{
    NBRE_Map<NBRE_String, NBRE_Map<uint32, NBRE_FontMaterial> >::const_iterator it = mFontMaterials.find(category);
    if (it != mFontMaterials.end())
    {
        NBRE_Map<uint32, NBRE_FontMaterial>::const_iterator it2 = it->second.find(id & 0xffff7fff);
        if (it2 != it->second.end())
        {
            return &it2->second;
        }
    }
    return NULL;
}

float NBGM_LabelLayer::GetFontScale(void)
{
    return mMapViewImpl->GetFontScale();
}

