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

    @file nbgmtilemaplayertile.h
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

#ifndef _NBGM_TILE_MAP_LAYER_TILE_H_
#define _NBGM_TILE_MAP_LAYER_TILE_H_
#include "nbgmmaplayer.h"
#include "nbgmvectorvertexscale.h"
#include "nbrerenderengine.h"
#include "nbrecollisiondetector2d.h"
#include "nbreuniformgridspatialpartition2d.h"
#include "nbgmdefaultoverlaymanager.h"

class NBGM_TileMapLayer;

/*! \addtogroup NBRE_Service
*  @{
*/

class NBGM_TileMapLayerTile : public NBGM_IMapLayerTile
{

public:
    explicit NBGM_TileMapLayerTile(NBGM_Context& nbgmContext, const NBRE_Point3d& center, const NBGM_VectorTileID& tileId, uint8 drawOrder);
    ~NBGM_TileMapLayerTile();

public:
    void SetCurrentZoomLevel(int8 zoomlevel);
    void LoadMapdata(NBGM_NBMData* nbmData, NBRE_ITextureAnimation& texcoorAnimation, int8 zoomLevel, int32 buildingOverlay, int32 glowOverlay, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, uint8 drawOrder, int32 subDrawOrder, nb_boolean enablePicking, uint32 selectMask);
    NBRE_NodePtr Node();
    void Active();
    void Deactive();
    void UpdateEntityVisibility(const NBRE_Planef& cameraPlane);
    void NotifyExtend(const NBGM_TileMapBoundaryExtendType& extendType);
    void SetOpacity(float opacity);
    nb_boolean HitTest(const NBRE_Vector2d& center, double radius, uint32 selectMask, NBRE_Set<NBGM_ElementId>& ids);
    float GetOpactiy() const { return mOpacity; }
    uint8 GetDrawOrder() const { return mDrawOrder; }
    nb_boolean IsBuildingTile() const { return mIsBuildingTile; }
    nb_boolean IsRoadTile() const { return mIsRoadTile; }
    nb_boolean IsPolylineTile() const { return mIsPolylineTile; }
    const NBRE_Set<int32>& GetOverlayIds() const { return mOverlayIds; }
    nb_boolean CheckIfHasLM3D();
    void DisableUnTexturedLandMarks();
    void SwitchRoadOverlays(nb_boolean useFrameBuffer, int32 transparentRoadBgOverlay, int32 transparentRoadFgOverlay);

private:
    NBRE_EntityPtr AttachEntity(const NBRE_EntityPtr& entity);
    void Build2DBuildingEntities(NBGM_NBMAREALayerData* areaLayer, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager,
                                 uint8 drawOrder, int32 subDrawOrder, nb_boolean enablePicking, uint32 selectMask, uint32 index);

    void BuildAreaEntities(NBGM_NBMAREALayerData* areaLayer, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager,
                           uint8 drawOrder, int32 subDrawOrder, nb_boolean enablePicking, uint32 selectMask, uint32 index);

    void BuildRouteEntities(NBGM_NBMLPTHLayerData* lpthLayer, NBRE_ITextureAnimation& texcoorAnimation, int8 zoomLevel,
                            NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, uint8 drawOrder,
                            int32 subDrawOrder, nb_boolean enablePicking, uint32 selectMask, uint32 index, int32 glowOverlay);

    void BuildRoadEntities(NBGM_NBMLPTHLayerData* lpthLayer, NBRE_ITextureAnimation& texcoorAnimation, int8 zoomLevel,
                           NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, uint8 drawOrder,
                           int32 subDrawOrder, nb_boolean enablePicking, uint32 selectMask, uint32 index, int32 glowOverlay);

    void BuildRouteEntities(NBGM_NBMLPDRLayerData* lpdrLayer, NBRE_ITextureAnimation& texcoorAnimation, int8 zoomLevel,
                            NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, uint8 drawOrder,
                            int32 subDrawOrder, nb_boolean enablePicking, uint32 selectMask, uint32 index);

    void BuildRoadEntities(NBGM_NBMLPDRLayerData* lpdrLayer, NBRE_ITextureAnimation& texcoorAnimation, int8 zoomLevel,
                           NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, uint8 drawOrder,
                           int32 subDrawOrder, nb_boolean enablePicking, uint32 selectMask, uint32 index);

    void BuildUT3DEntities(NBGM_NBMMESHLayerData* meshLayer, int32 buildingOverlay);
    void BuildUTLM3DEntities(NBGM_NBMMESHLayerData* meshLayer, int32 buildingOverlay);
    void BuildLM3DEntities(NBGM_NBMMESHLayerData* meshLayer, int32 buildingOverlay);
    void BuildRasterEntities(NBGM_NBMMESHLayerData* meshLayer, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager,
                             uint8 drawOrder, int32 subDrawOrder);
private:
    DISABLE_COPY_AND_ASSIGN(NBGM_TileMapLayerTile);

private:
    struct EntityAssistInfo
    {
        EntityAssistInfo(){}
        EntityAssistInfo(NBRE_Entity* entity, NBRE_Entity* extnedEntity, float nearVisibility, float farVisibility, const NBRE_String& type, NBRE_DOM_LayerType layerType)
            :mEntity(entity), mExtendEntity(extnedEntity), mNearVisibility(nearVisibility)
            ,mFarVisibility(farVisibility), mType(type), mLayerType(layerType)
        {
        }

        NBRE_Entity*        mEntity;
        NBRE_Entity*        mExtendEntity;
        float               mNearVisibility;
        float               mFarVisibility;
        NBRE_String         mType;
        NBRE_DOM_LayerType  mLayerType;
    };
    typedef NBRE_Vector<EntityAssistInfo> EntityAssistInfoList;

    typedef NBRE_Map<int8, NBRE_UniformGridSpatialPartition2d*> ZoomLevelSpaceTable;
    struct LayerCollisionSpace
    {
        LayerCollisionSpace(const NBRE_LayerInfo& layerInfo);
        ~LayerCollisionSpace();

        NBRE_LayerInfo mLayerInfo;
        NBRE_UniformGridSpatialPartition2d* mStaticSpace;
        float mWidth;
        NBRE_UniformGridSpatialPartition2d* mDynamicCollisionSpace;
        nb_boolean mVisible;
        NBRE_AxisAlignedBox2d mAABB;
    };
    typedef NBRE_Map<uint32, LayerCollisionSpace*> LayerCollisionSpaceTable;

    LayerCollisionSpace* GetLayerCollisionSpace(NBGM_NBMLayerData* layer, uint32 index);
    void CreatePolygonCollisionObjects(LayerCollisionSpace* lcs, NBRE_ModelPtr ent, uint32 selectMask);
    void CreatePolylineCollisionObjects(NBGM_Context& mNBGMContext,
                                        const NBRE_Vector<NBGM_PolylineData*>& tracks,
                                        LayerCollisionSpace* lcs,
                                        uint32 selectMask);

    void GetPolylineCollisionSpace(LayerCollisionSpace* lcs,
                                   const NBRE_Vector<NBGM_PolylineData*>& pls);

private:
    class NBGM_VertextOpacityAnimation : public NBRE_IVertexAnimation
    {
    public:
        NBGM_VertextOpacityAnimation(float opacity);
        ~NBGM_VertextOpacityAnimation();

    public:
        virtual void Animate(NBRE_VertexData& vertexData, NBRE_IndexData& indexData);
        virtual uint32 GetDirtyFrameNumber() const {return mDirtyFrameNumber;}
        void AnimationComplete() {}

        void SetOpacity(float opacity);
    private:
        float mOpacity;
        uint32 mDirtyFrameNumber;
    };

private:
    NBGM_Context& mNBGMContext;
    NBRE_NodePtr mNode;
    NBRE_NodePtr mExtendNode;
    NBGM_TileMapBoundaryExtendType mExtendType;
    NBRE_Point3d mTileCenter;
    NBRE_Vector<shared_ptr<NBGM_VectorVertexScale> > mRoadAnimations;
    NBGM_VectorTileID mTileID;
    nb_boolean mIsActive;
    float mOpacity;
    nb_boolean mInitOpacityAnimation;
    NBRE_Vector<shared_ptr<NBGM_VertextOpacityAnimation> > mOpacityAnimations;
    uint8 mDrawOrder;
    nb_boolean mIsBuildingTile;
    nb_boolean mIsRoadTile;
    nb_boolean mIsPolylineTile;
    NBRE_Set<int32> mOverlayIds;
    EntityAssistInfoList mEntityAssistInfoList;
    LayerCollisionSpaceTable mCollisionLayers;
    int8 mZoomLevel;
    NBRE_Map<NBRE_Entity*, int32> mRoadBgEntityOverlapMap;
    NBRE_Map<NBRE_Entity*, int32> mRoadFgEntityOverlapMap;
};

class NBGM_TileMapLayerTileCreator : public NBGM_IMapLayerTileCreator
{
public:
    NBGM_TileMapLayerTileCreator(NBGM_Context& nbgmContext, NBRE_ITextureAnimation* texcoorAnimation, int8* zoomLevel, int32* transparentRoadBgOverlay, int32* buildingOverlay, int32* glowOverlay);
    virtual ~NBGM_TileMapLayerTileCreator();

public:
    NBGM_IMapLayerTile* CreateMapLayerTile(const NBGM_VectorTileID& tileId, NBGM_NBMData* nbmData, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, uint8 drawOrder, int32 subDrawOrder, uint8 labelDrawOrder, nb_boolean enablePicking, uint32 selectMask, const NBRE_String& materialCategoryName);

private:
    DISABLE_COPY_AND_ASSIGN(NBGM_TileMapLayerTileCreator);

private:
    NBGM_Context&               mNBGMContext;
    NBRE_ITextureAnimation*     mTexcoorAnimation;
    int8*                       mZoomLevel;
    int32*                      mTransparentRoadBgOverlay;
    int32*                      mBuildingOverlay;
    int32*                      mGlowOverlay;
};


/*! @} */

#endif
