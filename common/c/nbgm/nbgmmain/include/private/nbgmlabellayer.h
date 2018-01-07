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

    @file nbgmlabellayer.h
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

#ifndef _NBGM_LABEL_LAYER_H_
#define _NBGM_LABEL_LAYER_H_
#include "nbretypes.h"
#include "nbrescenemanager.h"
#include "nbrerenderengine.h"
#include "nbrebillboardnode.h"
#include "nbgmvectortile.h"
#include "nbgmvectortiledata.h"
#include "nbgmlayoutelement.h"
#include "nbgmmaplayer.h"
#include "nbgmcontext.h"
#include "nbgmlabellayertile.h"
#include "nbgmlayoutmanager.h"
#include "nbgmtaskqueue.h"
#include "nbgmdefaultoverlaymanager.h"

//@TODO: Uncomment this line to see tile names
//#define NBGM_SHOW_TILE_NAME
class NBGM_DebugPointLabel;
class NBGM_MapViewProfiler;
class NBGM_MapViewImpl;

/*! \addtogroup NBRE_Service
*  @{
*/

//! 
/** Label layer priority. 
Smaller value has higher priority.
Priority value should be positive number;
*/
struct NBGM_LabelLayerPriority
{
    NBGM_LabelLayerPriority()
        :roadLabel(3), areaLabel(8), shield(4), untexturedBuilding(9), landmark(7), pointLabel(1), pointShield(4), staticPoi(2), staticPoiLabel(6)
    {
    }

    int32 roadLabel;
    int32 areaLabel;
    int32 shield;
    int32 untexturedBuilding;
    int32 landmark;
    int32 pointLabel;
    int32 pointShield;
    int32 staticPoi;
    int32 staticPoiLabel;
};

/** Label layer manages all label & shield
*/
class NBGM_LabelLayer: public NBGM_IMapLayer,
    public NBGM_ILayoutProvider
{
private:
    typedef NBRE_Map<NBGM_VectorTileID, NBGM_LabelLayerTile*> TileMap;

public:
    NBGM_LabelLayer(NBGM_Context& nbgmContext, NBRE_Node& parentNode, int32 billboardOverlayId, NBRE_BillboardSet* billboardSet, NBRE_Overlay* textOverlay, nb_boolean useNavRoadLabel
        , NBRE_SurfaceSubView* subView, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, NBRE_CameraPtr mScreenCamera, NBGM_MapViewProfiler* mapViewProfiler, int32 layerStride, int32 layerMain
        , shared_ptr<WorkerTaskQueue> loadingThread, shared_ptr<NBGM_TaskQueue> renderThread, NBGM_MapViewImpl* mapViewImpl);
    virtual ~NBGM_LabelLayer();

public:
    ///Derive From NBGM_IMapLayer
    NBGM_IMapLayerTileCreator& GetTileCreator();
    void OnAddTile(const NBGM_VectorTileID& tileId, NBGM_IMapLayerTile* tile);
    void OnRemoveTile(const NBGM_VectorTileID& tileId);
    void OnActiveTile(const NBGM_VectorTileID& tileId);
    void OnDeactiveTile(const NBGM_VectorTileID& tileId);
    /// Set label element priority
    void SetLabelLayerPriority(const NBGM_LabelLayerPriority& priority) { mPriority = priority; }
    void UpdateLayoutElementVisibility(float viewPointDistance);
    void RefreshFontMaterial(const NBRE_String& materialCategoryName, const NBRE_Map<uint32, NBRE_FontMaterial>& materials);
    void GetFontMaterialIds(const NBRE_String& materialCategoryName, NBRE_Set<uint32>& materialIds);
    nb_boolean UseNavRoadLabel() { return mUseNavRoadLabel; }
    void NotifyExtend(const NBGM_TileMapBoundaryExtendType& extendType);
    virtual void UpdateLayoutList(NBGM_LayoutElementList& layoutElements);
    void OnLayoutElementsStart();
    nb_boolean OnLayoutElementsDone(uint64 id, int32 level, NBRE_Vector<NBGM_StaticLayoutResult*>* layoutResult);
    nb_boolean GetStaticPoiInfo(const NBRE_String& id, NBGM_StaticPoiInfo& info);
    void SelectStaticPoi(const NBRE_String& id, nb_boolean selected);
    void GetElementsAt(NBGM_LayoutElementType type, const NBRE_Point2f& screenPosition, float radius, NBRE_Vector<NBRE_String>& ids);
    void EnableLayerLayout(const NBRE_String& layerId, nb_boolean enable);

    const NBRE_FontMaterial* GetFontMaterial(const NBRE_String& category, uint32 id) const;
    float GetFontScale(void);

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_LabelLayer);

private:
    NBGM_Context&    mNBGMContext;
    TileMap mTileMap;
    NBRE_Node& mParentNode;
    NBGM_LabelLayerPriority mPriority;
    NBGM_LabelLayerTileCreator mLayerDataCreator;
    nb_boolean mUseNavRoadLabel;
    NBRE_SurfaceSubView* mSubView;
    NBRE_DefaultOverlayManager<DrawOrderStruct>& mOverlayManager;
    NBRE_CameraPtr mScreenCamera;
    NBGM_MapViewProfiler* mMapViewProfiler;
    int32 mLayerStride;
    int32 mLayerMain;
    NBGM_TileMapBoundaryExtendType mExtendType;
    shared_ptr<WorkerTaskQueue> mLoadingThread;
    shared_ptr<NBGM_TaskQueue> mRenderThread;
    NBGM_MapViewImpl* mMapViewImpl;
    NBRE_Map<NBRE_String, nb_boolean> mLayersLayoutState;
    NBRE_Map<NBRE_String, NBRE_Map<uint32, NBRE_FontMaterial> > mFontMaterials;

#ifdef NBGM_SHOW_TILE_NAME
    struct PosCompare
    {
        bool operator() (const NBRE_Vector3d& left, const NBRE_Vector3d& right) const
        {
            return memcmp(&left, &right, sizeof(NBRE_Vector3d)) < 0;
        }
    };
    typedef NBRE_Map<NBRE_Vector3d, NBGM_DebugPointLabel*, PosCompare> DebugLabelMap;
    DebugLabelMap mDebugLabels;
#endif
};

/*! @} */
#endif
