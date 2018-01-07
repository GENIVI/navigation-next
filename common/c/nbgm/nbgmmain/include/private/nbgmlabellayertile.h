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

/*!--------------------------------------------------------------------------

    @file nbgmlabellayertile.h
*/
/*
    (C) Copyright 2012 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef _NBGM_LABEL_LAYER_TILE_H_
#define _NBGM_LABEL_LAYER_TILE_H_
#include "nbgmmaplayer.h"
#include "nbrebillboardnode.h"
#include "nbgmlayoutelement.h"
#include "nbgmtaskqueue.h"
#include "nbgmdefaultoverlaymanager.h"

struct NBGM_LabelLayerPriority;
typedef NBRE_Map<int32, NBRE_BillboardSet*> DrawOrderBillboardSetMap;
class NBGM_StaticLabelTile;
class NBGM_LabelLayer;
class NBGM_StaticLayoutResult;

/*! \addtogroup NBRE_Service
*  @{
*/
struct NBGM_PathArrowLayerData
{
    NBGM_PathArrowLayerData() {};
    ~NBGM_PathArrowLayerData();
    NBGM_PathArrowLayerData* Clone();
    
    typedef NBRE_Vector<NBRE_Point2f> MergedPolyline;
    typedef NBRE_Vector<MergedPolyline*> OneLayerPolylines;

    float              mNearVisibility;
    float              mFarVisibility;
    OneLayerPolylines  mLayerData;
};

class NBGM_LabelLayerTile : public NBGM_IMapLayerTile
{
public:
    NBGM_LabelLayerTile(NBGM_Context* nbgmContext, const NBGM_LabelLayer* layer, const NBRE_String& id, NBRE_SurfaceSubView* subView, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, NBRE_Overlay* textOverlay, 
                        const NBGM_LabelLayerPriority* priority, const NBRE_Point3d& tileCenter, nb_boolean useNavRoadLabel, uint8 labelDrawOrder, uint8 baseDrawOrder, int32 subDrawOrder, const NBRE_String& materialCategoryName);
    ~NBGM_LabelLayerTile();

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_LabelLayerTile);

public:
    void CreateTileLabels(NBGM_NBMLayerData* layer, const NBRE_Point3d& refCenter, uint8 drawOrder, int32 subDrawOrder, uint8 labelDrawOrder, const NBRE_String& materialCategoryName, const NBRE_String& groupName);
    void OnActive();
    void OnDeactive();
    void AddToScene();
    NBRE_NodePtr Node();
    void UpdateLayoutElementVisibility(float viewPointDistance);
    void AddTileNameLabel();
    void RefreshFontMaterial(const NBRE_Map<uint32, NBRE_FontMaterial>& materials);
    const NBRE_String& GetMaterialCategoryName() const { return mMaterialCategoryName; }
    void GetFontMaterialIds(NBRE_Set<uint32>& materialIds);
    void NotifyExtend(const NBGM_TileMapBoundaryExtendType& extendType);
    uint8 GetBaseDrawOrder() { return mBaseDrawOrder; }
    int32 GetSubDrawOrder() { return mSubDrawOrder; }
    uint8 GetLabelDrawOrder() { return mLabelDrawOrder; }
    void UpdateLayoutList(NBGM_LayoutElementList& layoutElements, shared_ptr<WorkerTaskQueue> loadingThread, shared_ptr<NBGM_TaskQueue> renderThread, NBGM_LabelLayer* labelLayer);
    nb_boolean OnLayoutElementsDone(uint64 id, int32 level, NBRE_Vector<NBGM_StaticLayoutResult*>* layoutResult);
    void ResolveStaticConflicts(int32 level, NBGM_LabelLayerTile* tile, double epsilon);
    nb_boolean GetStaticPoiInfo(const NBRE_String& id, NBGM_StaticPoiInfo& info);
    void SelectStaticPoi(const NBRE_String& id, nb_boolean selected);
    void GetElementsAt(NBGM_LayoutElementType type, const NBRE_Point2f& screenPosition, float radius, NBRE_Vector<NBRE_String>& ids);
    void UpdateLayersLayoutState(NBRE_Map<NBRE_String, nb_boolean>& layersLayoutState);
    void EnableLayerLayout(const NBRE_String& layerId, nb_boolean enable);

    const char* GetLabel(const NBGM_TextData& text, uint32& langCode);
private:
    void CreateTileAreaLabels(NBGM_NBMLayerData* layer, const NBRE_Point3d& refCenter, uint8 drawOrder, int32 subDrawOrder, uint8 labelDrawOrder, const NBRE_String& materialCategoryName);
    void CreateTileRoadLabels(NBGM_NBMLayerData* layer, const NBRE_Point3d& refCenter, uint8 drawOrder, int32 subDrawOrder, uint8 labelDrawOrder, const NBRE_String& materialCategoryName);
    void CreateTileBuildingLabels(NBGM_NBMLayerData* layer, const NBRE_Point3d& refCenter, uint8 drawOrder, int32 subDrawOrder, uint8 labelDrawOrder, const NBRE_String& materialCategoryName);
    void CreateTilePointLabels(NBGM_NBMLayerData* layer, const NBRE_Point3d& refCenter, uint8 drawOrder, int32 subDrawOrder, uint8 labelDrawOrder, const NBRE_String& materialCategoryName);
    void CreateTileShields(NBGM_NBMLayerData* layer, const NBRE_Point3d& refCenter, uint8 drawOrder, int32 subDrawOrder, uint8 labelDrawOrder, const NBRE_String& materialCategoryName);
    void CreatePathArrows(NBGM_NBMLayerData* layer, const NBRE_Point3d& refCenter, uint8 drawOrder, int32 subDrawOrder, uint8 labelDrawOrder, const NBRE_String& materialCategoryName);
    NBGM_LayoutElement* CreateTilePointShield(const NBRE_MapMaterial& shieldMat, const NBRE_FontMaterial& fontMaterial, uint32 fontMaterialId, const char* str, uint32 langCode, const NBRE_Vector3d& position, int32 overlayId);
    NBGM_LayoutElement* CreateTileStaticPoi(const NBRE_String& id, const NBRE_MapMaterial& poiMat, const NBRE_FontMaterial* fontMaterial, uint32 fontMaterialId, const char* str, uint32 langCode, const NBRE_Vector3d& position, int32 overlayId, int32 labelPriority);
    void CreateTileCustomPin(NBGM_NBMLayerData* layer, const NBRE_Point3d& refCenter, uint8 drawOrder, int32 subDrawOrder, uint8 labelDrawOrder, const NBRE_String& materialCategoryName, const NBRE_String& layerId);
    void CreateRepeatNode();
    void ResetRepeatNodes();

private:

    struct ElementAssistInfo
    {
        ElementAssistInfo(): mNearVisibility(0.0f), mFarVisibility(0.0f), mType(NBRE_LDT_LPTH), mVisible(TRUE){}

        float                   mNearVisibility;
        float                   mFarVisibility;
        NBRE_LayerDataType      mType;
        nb_boolean              mVisible;
        NBGM_LayoutElementList  mLayoutElementList;
    };
    typedef NBRE_Vector<ElementAssistInfo> ElementAssistInfoList;

    NBGM_PathArrowLayerData* CreateMergedLayerData(NBGM_NBMLayerData* layer);

    struct RoadTrackInfo
    {
        NBRE_WString text;
        const NBRE_MapMaterial* material;
        uint32 materialId;
        float width;
        NBRE_Vector<NBRE_Point2f> polyline;
        uint16 priority;
    };
    typedef NBRE_Vector<RoadTrackInfo*> RoadTrackList;
    void MergeRoadTracks(NBGM_NBMLayerData* layer, RoadTrackList& result);
    nb_boolean TryMergeRoadTracks(RoadTrackInfo* dst, RoadTrackInfo* src);

private:
    NBGM_Context* mNBGMContext;
    const NBGM_LabelLayer* mLayer;
    NBRE_SurfaceSubView* mSubView;
    NBRE_DefaultOverlayManager<DrawOrderStruct>& mOverlayManager;
    NBRE_NodePtr mNode;
    NBRE_NodePtr mSubNode;
    NBRE_String mId;
    NBGM_LayoutElementList mElements;
    NBGM_StaticLabelTile* mStaticLabelTile;
    NBRE_AxisAlignedBox3d mAABB;
    uint32 mLoadedLayerFlag;
    const NBGM_LabelLayerPriority* mPriority;
    nb_boolean mIsNav;
    uint8 mLabelDrawOrder;
    uint8 mBaseDrawOrder;
    int32 mSubDrawOrder;
    NBRE_NodePtr mRepeatSubNode[3];
    NBGM_TileMapBoundaryExtendType mExtendType;
    NBRE_Point3d mTileCenter;
    ElementAssistInfoList   mElementAssistInfoList;
    nb_boolean mActive;
    nb_boolean mHaveStaticPoi;
    NBRE_String mMaterialCategoryName;
};

class NBGM_LabelLayerTileCreator : public NBGM_IMapLayerTileCreator
{
public:
    NBGM_LabelLayerTileCreator(NBGM_Context& nbgmContext, const NBGM_LabelLayer* layer, int32 billboardOverlayId, NBRE_BillboardSet* billboardSet, NBRE_Overlay* textOverlay,
                                const NBGM_LabelLayerPriority* priority, nb_boolean useNavRoadLabel, NBRE_SurfaceSubView* subView);
    ~NBGM_LabelLayerTileCreator();

public:
    NBGM_IMapLayerTile* CreateMapLayerTile(const NBGM_VectorTileID& tileId, NBGM_NBMData* nbmData, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, uint8 drawOrder, int32 subDrawOrder, uint8 labelDrawOrder, nb_boolean enablePicking, uint32 selectMask, const NBRE_String& materialCategoryName);

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_LabelLayerTileCreator);

private:
    NBGM_Context&       mNBGMContext;
    const NBGM_LabelLayer* mLayer;
    NBRE_BillboardSet*  mBillboardSet;
    NBRE_Overlay*       mTextOverlay;
    const NBGM_LabelLayerPriority* mPriority;
    nb_boolean          mUseNavRoadLabel;
    NBRE_SurfaceSubView* mSubView;
};

/*! @} */
#endif
