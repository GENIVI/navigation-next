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
#include "nbgmtilemaplayertile.h"
#include "nbgmtilemaplayer.h"
#include "nbgmvectortilebuilder.h"
#include "nbretypeconvert.h"
#include "nbgmconst.h"
#include "nbgmvectortiledata.h"
#include "nbgmmapmaterialmanager.h"
#include "nbreuniformgridspatialpartition2d.h"
#include "nbreaabbtree2d.h"
#include "nbgmlabeldebugutil.h"
#include "nbrecompositecollisionobject2d.h"
#include "nbrecollisionpolygon2d.h"
#include "nbrecollisionlinesegment2d.h"
#include "nbrecollisiontypes.h"

//#define ENABLE_COLLISION_DETECT_DEBUG_DRAW

NBGM_TileMapLayerTile::NBGM_TileMapLayerTile(NBGM_Context& nbgmContext, const NBRE_Point3d& center, const NBGM_VectorTileID& tileId, uint8 drawOrder):
    mNBGMContext(nbgmContext),
    mNode(NBRE_NEW NBRE_Node),
    mExtendNode(NBRE_NEW NBRE_Node),
    mExtendType(NBGM_TMBET_NONE),
    mTileCenter(center),
    mTileID(tileId),
    mIsActive(TRUE),
    mOpacity(1.0f),
    mInitOpacityAnimation(FALSE),
    mDrawOrder(drawOrder),
    mIsBuildingTile(FALSE),
    mIsRoadTile(FALSE),
    mIsPolylineTile(FALSE),
    mZoomLevel(0)
{
    mNode->SetPosition(center);
}

NBGM_TileMapLayerTile::~NBGM_TileMapLayerTile()
{
    for (LayerCollisionSpaceTable::iterator it = mCollisionLayers.begin(); it != mCollisionLayers.end(); ++it)
    {
        NBRE_DELETE it->second;
    }

    if(mExtendType != NBGM_TMBET_NONE)
    {
        mNode->RemoveChild(mExtendNode.get());
    }
}

void
NBGM_TileMapLayerTile::SetCurrentZoomLevel(int8 zoomlevel)
{
    for(uint32 i=0; i<mRoadAnimations.size(); ++i)
    {
        mRoadAnimations[i]->SetCurrentZoomLevel(zoomlevel);
    }
    mZoomLevel = zoomlevel;
}

void
NBGM_TileMapLayerTile::CreatePolygonCollisionObjects(LayerCollisionSpace* lcs, NBRE_ModelPtr ent, uint32 selectMask)
{
    if (lcs)
    {
        if (lcs->mStaticSpace == NULL)
        {
            lcs->mStaticSpace = NBRE_NEW NBRE_UniformGridSpatialPartition2d(lcs->mAABB, NBRE_Vector2i(10, 10));
        }

        uint32 subMeshNum = ent->GetSubModelNumber();
        for (uint32 iSubMesh = 0; iSubMesh < subMeshNum; ++iSubMesh)
        {
            NBRE_SubModel& subEntity = ent->GetSubModel(iSubMesh);
            NBRE_SubMesh* subMesh = subEntity.GetSubMesh();
            if (subMesh->GetOperationType() == NBRE_PMT_TRIANGLE_LIST)
            {
                NBRE_VertexData* vertexData = subMesh->UseSharedVertices() ? ent->GetMesh()->GetSharedVertexedData() : subMesh->GetVertexData();
                NBRE_AABBTree2d* aabbTree = NBRE_NEW NBRE_AABBTree2d;
                NBRE_CollisionObject2d* co = NBRE_CollisionObjectBuilder2d::CreateTriangleMesh2d(NULL, vertexData, subMesh->GetIndexData(), aabbTree, selectMask);
                if (co)
                {
                    lcs->mStaticSpace->AddCollisionObject(co);
#ifdef ENABLE_COLLISION_DETECT_DEBUG_DRAW
                    NBRE_Vector<NBRE_AxisAlignedBox2d> treeAABBs;
                    aabbTree->GetNodeAABBs(treeAABBs);
                    for (uint32 i = 0; i < treeAABBs.size(); ++i)
                    {
                        NBRE_AxisAlignedBox2f box = NBRE_TypeConvertf::Convert(treeAABBs[i]);
                        NBRE_Model* model = NBRE_NEW NBRE_Model(NBRE_MeshPtr(
                            CreateDebugRectEntity(mNBGMContext.renderingEngine->Context()
                            , box.GetCenter().x, box.GetCenter().y, box.GetSize().x, box.GetSize().y
                            , NBRE_Color(0, 1, 0, 1))));
                        mNode->AttachObject(NBRE_EntityPtr(NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), NBRE_ModelPtr(model), 65843)));
                    }

                    NBRE_Vector<NBRE_ICollisionPrimitive2d*> triList;
                    ((NBRE_CompositeCollisionObject2d*)co)->GetPrimitives(triList);
                    for (uint32 i = 0; i < triList.size(); ++i)
                    {
                        NBRE_CollisionPolygon2d* tri = (NBRE_CollisionPolygon2d*)triList[i];
                        NBRE_Model* model = NBRE_NEW NBRE_Model(NBRE_MeshPtr(
                            CreateDebugPolygonEntity(mNBGMContext.renderingEngine->Context()
                            , &tri->GetVertices()[0], tri->GetVertices().size(),
                            NBRE_Color(0, 0, 1, 1))
                            ));
                        mNode->AttachObject(NBRE_EntityPtr(NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), NBRE_ModelPtr(model), 10)));
                    }
#endif
                }
            }
        }

    }
}

void
NBGM_TileMapLayerTile::CreatePolylineCollisionObjects(NBGM_Context& /*NBGMContext*/,
                                                      const NBRE_Vector<NBGM_PolylineData*>& tracks,
                                                      LayerCollisionSpace* lcs,
                                                      uint32 selectMask)
{
    if (lcs == NULL || lcs->mAABB.IsNull())
    {
        return;
    }

    if (tracks.size() > 0)
    {
        if (lcs->mDynamicCollisionSpace == NULL)
        {
            lcs->mDynamicCollisionSpace = NBRE_NEW NBRE_UniformGridSpatialPartition2d(lcs->mAABB, NBRE_Vector2i(10, 10));
        }

        for (uint32 i = 0; i < tracks.size(); ++i)
        {
            NBRE_CollisionObject2d* co = NBRE_CollisionObjectBuilder2d::CreatePolyline2d((void*)&tracks[i]->id, *tracks[i]->polyline, NBRE_NEW NBRE_AABBTree2d, selectMask);
            if (co)
            {
                lcs->mDynamicCollisionSpace->AddCollisionObject(co);

#ifdef ENABLE_COLLISION_DETECT_DEBUG_DRAW
                NBRE_CompositeCollisionObject2d* cco = (NBRE_CompositeCollisionObject2d*)co;
                NBRE_Vector<NBRE_ICollisionPrimitive2d*> pms;
                cco->GetPrimitives(pms);
                for (uint32 j = 0; j < pms.size(); ++j)
                {
                    NBRE_CollisionLineSegment2d* seg = (NBRE_CollisionLineSegment2d*)pms[j];
                    NBRE_Polyline2d plSeg(seg->GetEndPoints(), 2);

                    NBRE_Model* model = NBRE_NEW NBRE_Model(NBRE_MeshPtr(
                        CreateDebugPolylineEntity(mNBGMContext.renderingEngine->Context()
                        , NBRE_TypeConvertf::Convert(plSeg), NBRE_Color(0,0,1,0.3f))
                        ));
                    mNode->AttachObject(NBRE_EntityPtr(NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), NBRE_ModelPtr(model), 65843)));
                }
#endif
            }
        }
    }
}

void
NBGM_TileMapLayerTile::Build2DBuildingEntities(NBGM_NBMAREALayerData* areaLayer, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager,
                                               uint8 drawOrder, int32 subDrawOrder, nb_boolean enablePicking, uint32 selectMask, uint32 index)
{
    if(areaLayer->GetAreaModel().get() == NULL || areaLayer->GetOutlineAreaModel().get() == NULL)
    {
        return;
    }

    NBRE_Entity* ent = NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), areaLayer->GetAreaModel(), 0);
    NBRE_Entity* entOutline = NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), areaLayer->GetOutlineAreaModel(), 0);
    if(ent == NULL || entOutline == NULL)
    {
        NBRE_DELETE ent;
        NBRE_DELETE entOutline;
        return;
    }

    int32 overlayId = overlayManager.AssignOverlayId(DrawOrderStruct(drawOrder, subDrawOrder, 1));
    mOverlayIds.insert(overlayId);
    ent->SetOverlayId(overlayId);
    NBRE_EntityPtr extEnt = AttachEntity(NBRE_EntityPtr(ent));

    float nearVisibility = areaLayer->GetLayerInfo().nearVisibility;
    float farVisibility = areaLayer->GetLayerInfo().farVisibility;

    EntityAssistInfo info(ent, extEnt.get(), nearVisibility, farVisibility, "B2D", areaLayer->GetLayerInfo().layerType);
    mEntityAssistInfoList.push_back(info);

    overlayId = overlayManager.AssignOverlayId(DrawOrderStruct(drawOrder, subDrawOrder, 0));
    mOverlayIds.insert(overlayId);
    entOutline->SetOverlayId(overlayId);
    NBRE_EntityPtr extEntOutline = AttachEntity(NBRE_EntityPtr(entOutline));
    EntityAssistInfo infoWireFrame(entOutline, extEntOutline.get(), nearVisibility, farVisibility, "B2D_Outline", areaLayer->GetLayerInfo().layerType);
    mEntityAssistInfoList.push_back(infoWireFrame);

    if (enablePicking)
    {
        LayerCollisionSpace* lcs = GetLayerCollisionSpace(areaLayer, index);
        CreatePolygonCollisionObjects(lcs, areaLayer->GetAreaModel(), selectMask);
    }
}

void
NBGM_TileMapLayerTile::BuildAreaEntities(NBGM_NBMAREALayerData* areaLayer, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager,
                                         uint8 drawOrder, int32 subDrawOrder, nb_boolean enablePicking, uint32 selectMask, uint32 index)
{
    if(areaLayer->GetAreaModel().get() == NULL)
    {
        return;
    }

    NBRE_Entity* ent = NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), areaLayer->GetAreaModel(), 0);
    if(ent == NULL)
    {
        return;
    }

    int32 overlayId = overlayManager.AssignOverlayId(DrawOrderStruct(drawOrder, subDrawOrder, 0));
    mOverlayIds.insert(overlayId);
    ent->SetOverlayId(overlayId);

    NBRE_EntityPtr extEnt = AttachEntity(NBRE_EntityPtr(ent));

    float nearVisibility = areaLayer->GetLayerInfo().nearVisibility;
    float farVisibility = areaLayer->GetLayerInfo().farVisibility;

    EntityAssistInfo info(ent, extEnt.get(), nearVisibility, farVisibility, "AREA", areaLayer->GetLayerInfo().layerType);
    mEntityAssistInfoList.push_back(info);

    if (enablePicking)
    {
        LayerCollisionSpace* lcs = GetLayerCollisionSpace(areaLayer, index);
        CreatePolygonCollisionObjects(lcs, areaLayer->GetAreaModel(), selectMask);
    }
}

void
NBGM_TileMapLayerTile::BuildRouteEntities(NBGM_NBMLPTHLayerData* lpthLayer, NBRE_ITextureAnimation& texcoorAnimation,
                                          int8 zoomLevel, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager,
                                          uint8 drawOrder, int32 subDrawOrder, nb_boolean enablePicking, uint32 selectMask, uint32 index, int32 glowOverlay)
{
    if(lpthLayer->GetRoadModel().get() == NULL)
    {
        return;
    }

    NBRE_Entity* ent = NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), lpthLayer->GetRoadModel(), 0);
    if(ent == NULL)
    {
        return;
    }

    int32 overlayId = overlayManager.AssignOverlayId(DrawOrderStruct(drawOrder, subDrawOrder, 0));
    mOverlayIds.insert(overlayId);
    ent->SetOverlayId(overlayId);

    shared_ptr<NBGM_VectorVertexScale> anim = lpthLayer->GetRoadAnimation();
    anim->SetCurrentZoomLevel(zoomLevel);
    mRoadAnimations.push_back(anim);

    ent->SetTextureAnimation(lpthLayer->GetAnimatedSubModels(), &texcoorAnimation);

    NBRE_EntityPtr extendForEntityPtr = AttachEntity(NBRE_EntityPtr(ent));
    extendForEntityPtr->SetTextureAnimation(lpthLayer->GetAnimatedSubModels(), &texcoorAnimation);

    float nearVisibility = lpthLayer->GetLayerInfo().nearVisibility;
    float farVisibility = lpthLayer->GetLayerInfo().farVisibility;

    EntityAssistInfo info(ent, extendForEntityPtr.get(), nearVisibility, farVisibility, "ROUTE", lpthLayer->GetLayerInfo().layerType);
    mEntityAssistInfoList.push_back(info);

    if (enablePicking)
    {
        LayerCollisionSpace* lcs = GetLayerCollisionSpace(lpthLayer, index);
        CreatePolylineCollisionObjects(mNBGMContext, lpthLayer->GetTracks(), lcs, selectMask);
    }

    if (glowOverlay != -1)
    {
        NBRE_Entity* ent = NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), lpthLayer->GetRoadModel(), 0);
        ent->SetOverlayId(glowOverlay);

        shared_ptr<NBGM_VectorVertexScale> anim = lpthLayer->GetRoadAnimation();
        anim->SetCurrentZoomLevel(zoomLevel);
        mRoadAnimations.push_back(anim);

        ent->SetTextureAnimation(lpthLayer->GetAnimatedSubModels(), &texcoorAnimation);

        NBRE_EntityPtr extendForEntityPtr = AttachEntity(NBRE_EntityPtr(ent));
        extendForEntityPtr->SetTextureAnimation(lpthLayer->GetAnimatedSubModels(), &texcoorAnimation);

        float nearVisibility = lpthLayer->GetLayerInfo().nearVisibility;
        float farVisibility = lpthLayer->GetLayerInfo().farVisibility;

        EntityAssistInfo info(ent, extendForEntityPtr.get(), nearVisibility, farVisibility, "ROUTE", lpthLayer->GetLayerInfo().layerType);
        mEntityAssistInfoList.push_back(info);
    }
}

void
NBGM_TileMapLayerTile::BuildRoadEntities(NBGM_NBMLPTHLayerData* lpthLayer, NBRE_ITextureAnimation& texcoorAnimation,
                                         int8 zoomLevel, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager,
                                         uint8 drawOrder, int32 subDrawOrder, nb_boolean enablePicking, uint32 selectMask, uint32 index, int32 glowOverlay)
{
    if(lpthLayer->GetRoadModel().get() == NULL || lpthLayer->GetBackgroundRoadModel().get() == NULL)
    {
        return;
    }

    if (!enablePicking)
    {
        mIsRoadTile = TRUE;
    }

    // foreground road
    NBRE_Entity* foreEnt = NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), lpthLayer->GetRoadModel(), 0);
    NBRE_Entity* backEnt = NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), lpthLayer->GetBackgroundRoadModel(), 0);
    if(foreEnt == NULL || backEnt == NULL)
    {
        return;
    }

    int32 overlayId = overlayManager.AssignOverlayId(DrawOrderStruct(drawOrder, subDrawOrder, NBGM_RoadForegroundLayer));
    mOverlayIds.insert(overlayId);
    foreEnt->SetOverlayId(overlayId);
    foreEnt->SetPriority(index);

    if (mIsRoadTile)
    {
        mRoadFgEntityOverlapMap[foreEnt] = overlayId;
    }

    shared_ptr<NBGM_VectorVertexScale> foreAnim = lpthLayer->GetRoadAnimation();
    foreAnim->SetCurrentZoomLevel(zoomLevel);
    mRoadAnimations.push_back(foreAnim);
    foreEnt->SetTextureAnimation(lpthLayer->GetAnimatedSubModels(), &texcoorAnimation);

    NBRE_EntityPtr extendForEntityPtr = AttachEntity(NBRE_EntityPtr(foreEnt));
    extendForEntityPtr->SetTextureAnimation(lpthLayer->GetAnimatedSubModels(), &texcoorAnimation);

    float nearVisibility = lpthLayer->GetLayerInfo().nearVisibility;
    float farVisibility = lpthLayer->GetLayerInfo().farVisibility;

    EntityAssistInfo infoFore(foreEnt, extendForEntityPtr.get(), nearVisibility, farVisibility, "ROAD", lpthLayer->GetLayerInfo().layerType);
    mEntityAssistInfoList.push_back(infoFore);

    overlayId = overlayManager.AssignOverlayId(DrawOrderStruct(drawOrder, subDrawOrder, NBGM_RoadBackgroundLayer));
    mOverlayIds.insert(overlayId);
    backEnt->SetOverlayId(overlayId);
    backEnt->SetPriority(index);
    if (mIsRoadTile)
    {
        mRoadBgEntityOverlapMap[backEnt] = overlayId;
    }


    shared_ptr<NBGM_VectorVertexScale> backAnim = lpthLayer->GetBackgroundRoadAnimation();
    backAnim->SetCurrentZoomLevel(zoomLevel);
    mRoadAnimations.push_back(backAnim);

    NBRE_EntityPtr extendBackEntityPtr = AttachEntity(NBRE_EntityPtr(backEnt));

    EntityAssistInfo infoBack(backEnt, extendBackEntityPtr.get(), nearVisibility, farVisibility, "ROAD", lpthLayer->GetLayerInfo().layerType);
    mEntityAssistInfoList.push_back(infoBack);

    if (enablePicking)
    {
        LayerCollisionSpace* lcs = GetLayerCollisionSpace(lpthLayer, index);
        CreatePolylineCollisionObjects(mNBGMContext, lpthLayer->GetTracks(), lcs, selectMask);
    }

    if (glowOverlay != -1)
    {
        // foreground road
        NBRE_Entity* foreEnt = NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), lpthLayer->GetRoadModel(), 0);
        NBRE_Entity* backEnt = NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), lpthLayer->GetBackgroundRoadModel(), 0);

        backEnt->SetOverlayId(glowOverlay);
        backEnt->SetPriority(index);


        shared_ptr<NBGM_VectorVertexScale> backAnim = lpthLayer->GetBackgroundRoadAnimation();
        backAnim->SetCurrentZoomLevel(zoomLevel);
        mRoadAnimations.push_back(backAnim);

        NBRE_EntityPtr extendBackEntityPtr = AttachEntity(NBRE_EntityPtr(backEnt));

        EntityAssistInfo infoBack(backEnt, extendBackEntityPtr.get(), nearVisibility, farVisibility, "ROAD", lpthLayer->GetLayerInfo().layerType);
        mEntityAssistInfoList.push_back(infoBack);
        
        foreEnt->SetOverlayId(glowOverlay);
        foreEnt->SetPriority(index);

        shared_ptr<NBGM_VectorVertexScale> foreAnim = lpthLayer->GetRoadAnimation();
        foreAnim->SetCurrentZoomLevel(zoomLevel);
        mRoadAnimations.push_back(foreAnim);
        foreEnt->SetTextureAnimation(lpthLayer->GetAnimatedSubModels(), &texcoorAnimation);

        NBRE_EntityPtr extendForEntityPtr = AttachEntity(NBRE_EntityPtr(foreEnt));
        extendForEntityPtr->SetTextureAnimation(lpthLayer->GetAnimatedSubModels(), &texcoorAnimation);

        float nearVisibility = lpthLayer->GetLayerInfo().nearVisibility;
        float farVisibility = lpthLayer->GetLayerInfo().farVisibility;

        EntityAssistInfo infoFore(foreEnt, extendForEntityPtr.get(), nearVisibility, farVisibility, "ROAD", lpthLayer->GetLayerInfo().layerType);
        mEntityAssistInfoList.push_back(infoFore);

        
    }
}

void
NBGM_TileMapLayerTile::BuildRouteEntities(NBGM_NBMLPDRLayerData* lpdrLayer,
                                          NBRE_ITextureAnimation& texcoorAnimation,
                                          int8 zoomLevel,
                                          NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager,
                                          uint8 drawOrder,
                                          int32 subDrawOrder,
                                          nb_boolean enablePicking,
                                          uint32 selectMask,
                                          uint32 index)
{
    if(lpdrLayer->GetRoadModel().size() == 0)
    {
        return;
    }

    const NBRE_Map<uint16, NBRE_ModelPtr>& roadModel = lpdrLayer->GetRoadModel();
    NBRE_Map<uint16, shared_ptr<NBGM_VectorVertexScale> >& foreAnims = lpdrLayer->GetRoadAnimation();
    const NBRE_Map<uint16, NBRE_Set<NBRE_SubModel*>*>& animSubModels = lpdrLayer->GetAnimatedSubModels();

    for (NBRE_Map<uint16, NBRE_ModelPtr>::const_iterator it = roadModel.begin(); it != roadModel.end(); ++it)
    {
        uint16 roadOrder = it->first;
        NBRE_Entity* foreEnt = NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), it->second, 0);
        if (foreEnt == NULL)
        {
            return;
        }

        int32 overlayId = overlayManager.AssignOverlayId(DrawOrderStruct(drawOrder, (subDrawOrder << 16) + roadOrder, NBGM_RoadForegroundLayer));
        mOverlayIds.insert(overlayId);
        foreEnt->SetOverlayId(overlayId);
        foreEnt->SetPriority(index);

        NBRE_Map<uint16, shared_ptr<NBGM_VectorVertexScale> >::iterator itForeAnim = foreAnims.find(roadOrder);
        if (itForeAnim != foreAnims.end())
        {
            shared_ptr<NBGM_VectorVertexScale> foreAnim = itForeAnim->second;
            foreAnim->SetCurrentZoomLevel(zoomLevel);
            mRoadAnimations.push_back(foreAnim);
        }
        NBRE_EntityPtr extendForEntityPtr = AttachEntity(NBRE_EntityPtr(foreEnt));

        NBRE_Map<uint16, NBRE_Set<NBRE_SubModel*>*>::const_iterator itAnimSubModel = animSubModels.find(roadOrder);
        if (itAnimSubModel != animSubModels.end())
        {
            foreEnt->SetTextureAnimation(*itAnimSubModel->second, &texcoorAnimation);
            extendForEntityPtr->SetTextureAnimation(*itAnimSubModel->second, &texcoorAnimation);
        }

        float nearVisibility = lpdrLayer->GetLayerInfo().nearVisibility;
        float farVisibility = lpdrLayer->GetLayerInfo().farVisibility;

        EntityAssistInfo infoFore(foreEnt, extendForEntityPtr.get(), nearVisibility, farVisibility, "ROAD", lpdrLayer->GetLayerInfo().layerType);
        mEntityAssistInfoList.push_back(infoFore);
    }

    if (enablePicking)
    {
        LayerCollisionSpace* lcs = GetLayerCollisionSpace(lpdrLayer, index);
        CreatePolylineCollisionObjects(mNBGMContext, lpdrLayer->GetTracks(), lcs, selectMask);
    }
}

void
NBGM_TileMapLayerTile::BuildRoadEntities(NBGM_NBMLPDRLayerData* lpdrLayer,
                                         NBRE_ITextureAnimation& texcoorAnimation,
                                         int8 zoomLevel,
                                         NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager,
                                         uint8 drawOrder,
                                         int32 subDrawOrder,
                                         nb_boolean enablePicking,
                                         uint32 selectMask,
                                         uint32 index)
{
    if(lpdrLayer->GetRoadModel().size() == 0 || lpdrLayer->GetBackgroundRoadModel().size() == 0)
    {
        return;
    }

    if (!enablePicking)
    {
        mIsRoadTile = TRUE;
    }

    const NBRE_Map<uint16, NBRE_ModelPtr>& roadModel = lpdrLayer->GetRoadModel();
    const NBRE_Map<uint16, NBRE_ModelPtr>& bgModel = lpdrLayer->GetBackgroundRoadModel();
    NBRE_Map<uint16, shared_ptr<NBGM_VectorVertexScale> >& foreAnims = lpdrLayer->GetRoadAnimation();
    NBRE_Map<uint16, shared_ptr<NBGM_VectorVertexScale> >& bgAnims = lpdrLayer->GetBackgroundRoadAnimation();
    const NBRE_Map<uint16, NBRE_Set<NBRE_SubModel*>*>& animSubModels = lpdrLayer->GetAnimatedSubModels();

    for (NBRE_Map<uint16, NBRE_ModelPtr>::const_iterator it = roadModel.begin(); it != roadModel.end(); ++it)
    {
        uint16 roadOrder = it->first;

        NBRE_Entity* foreEnt = NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), it->second, 0);
        if (foreEnt == NULL)
        {
            return;
        }

        NBRE_Map<uint16, NBRE_ModelPtr>::const_iterator itBgModel = bgModel.find(roadOrder);
        if (itBgModel == bgModel.end())
        {
            return;
        }
        NBRE_Entity* backEnt = NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), itBgModel->second, 0);
        if (backEnt == NULL)
        {
            return;
        }

        int32 overlayId = overlayManager.AssignOverlayId(DrawOrderStruct(drawOrder, (subDrawOrder << 16) + (roadOrder), NBGM_RoadForegroundLayer));
        mOverlayIds.insert(overlayId);
        foreEnt->SetOverlayId(overlayId);
        foreEnt->SetPriority(index);

        if (mIsRoadTile)
        {
            mRoadFgEntityOverlapMap[foreEnt] = overlayId;
        }

        NBRE_Map<uint16, shared_ptr<NBGM_VectorVertexScale> >::iterator itForeAnim = foreAnims.find(roadOrder);
        if (itForeAnim != foreAnims.end())
        {
            shared_ptr<NBGM_VectorVertexScale> foreAnim = itForeAnim->second;
            foreAnim->SetCurrentZoomLevel(zoomLevel);
            mRoadAnimations.push_back(foreAnim);
        }
        NBRE_EntityPtr extendForEntityPtr = AttachEntity(NBRE_EntityPtr(foreEnt));

        overlayId = overlayManager.AssignOverlayId(DrawOrderStruct(drawOrder, (subDrawOrder << 16) + (roadOrder), NBGM_RoadBackgroundLayer));
        mOverlayIds.insert(overlayId);
        backEnt->SetOverlayId(overlayId);
        backEnt->SetPriority(index);
        if (mIsRoadTile)
        {
            mRoadBgEntityOverlapMap[backEnt] = overlayId;
        }

        NBRE_Map<uint16, shared_ptr<NBGM_VectorVertexScale> >::iterator itBackAnim = bgAnims.find(roadOrder);
        if (itBackAnim != bgAnims.end())
        {
            shared_ptr<NBGM_VectorVertexScale> backAnim = itBackAnim->second;
            backAnim->SetCurrentZoomLevel(zoomLevel);
            mRoadAnimations.push_back(backAnim);
        }
        NBRE_EntityPtr extendBackEntityPtr = AttachEntity(NBRE_EntityPtr(backEnt));

        NBRE_Map<uint16, NBRE_Set<NBRE_SubModel*>*>::const_iterator itAnimSubModel = animSubModels.find(roadOrder);
        if (itAnimSubModel != animSubModels.end())
        {
            foreEnt->SetTextureAnimation(*itAnimSubModel->second, &texcoorAnimation);
            extendForEntityPtr->SetTextureAnimation(*itAnimSubModel->second, &texcoorAnimation);
            backEnt->SetTextureAnimation(*itAnimSubModel->second, &texcoorAnimation);
            extendBackEntityPtr->SetTextureAnimation(*itAnimSubModel->second, &texcoorAnimation);
        }

        float nearVisibility = lpdrLayer->GetLayerInfo().nearVisibility;
        float farVisibility = lpdrLayer->GetLayerInfo().farVisibility;

        EntityAssistInfo infoFore(foreEnt, extendForEntityPtr.get(), nearVisibility, farVisibility, "ROAD", lpdrLayer->GetLayerInfo().layerType);
        mEntityAssistInfoList.push_back(infoFore);

        EntityAssistInfo infoBack(backEnt, extendBackEntityPtr.get(), nearVisibility, farVisibility, "ROAD", lpdrLayer->GetLayerInfo().layerType);
        mEntityAssistInfoList.push_back(infoBack);
    }

    if (enablePicking)
    {
        LayerCollisionSpace* lcs = GetLayerCollisionSpace(lpdrLayer, index);
        CreatePolylineCollisionObjects(mNBGMContext, lpdrLayer->GetTracks(), lcs, selectMask);
    }
}

void
NBGM_TileMapLayerTile::BuildUT3DEntities(NBGM_NBMMESHLayerData* meshLayer, int32 buildingOverlay)
{
    const NBRE_Vector<NBRE_ModelPtr>& models = meshLayer->GetModels();
    for(NBRE_Vector<NBRE_ModelPtr>::const_iterator iter = models.begin(); iter != models.end(); ++iter)
    {
        NBRE_Entity* entity = NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), *iter, -1);
        entity->SetOverlayId(buildingOverlay);
        NBRE_EntityPtr extEnt = AttachEntity(NBRE_EntityPtr(entity));

        float nearVisibility = meshLayer->GetLayerInfo().nearVisibility;
        float farVisibility = meshLayer->GetLayerInfo().farVisibility;

        EntityAssistInfo info(entity, extEnt.get(), nearVisibility, farVisibility, "UT3D", meshLayer->GetLayerInfo().layerType);
        mEntityAssistInfoList.push_back(info);
    }
    mIsBuildingTile = TRUE;
}

void
NBGM_TileMapLayerTile::BuildUTLM3DEntities(NBGM_NBMMESHLayerData* meshLayer, int32 buildingOverlay)
{
    const NBRE_Vector<NBRE_ModelPtr>& models = meshLayer->GetModels();
    for(NBRE_Vector<NBRE_ModelPtr>::const_iterator iter = models.begin(); iter != models.end(); ++iter)
    {
        NBRE_Entity* entity = NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), *iter, -1);
        entity->SetOverlayId(buildingOverlay);
        NBRE_EntityPtr extEnt = AttachEntity(NBRE_EntityPtr(entity));

        float nearVisibility = meshLayer->GetLayerInfo().nearVisibility;
        float farVisibility = meshLayer->GetLayerInfo().farVisibility;

        EntityAssistInfo info(entity, extEnt.get(), nearVisibility, farVisibility, "UT3DLM", meshLayer->GetLayerInfo().layerType);
        mEntityAssistInfoList.push_back(info);
    }
    mIsBuildingTile = TRUE;
}

void
NBGM_TileMapLayerTile::BuildLM3DEntities(NBGM_NBMMESHLayerData* meshLayer, int32 buildingOverlay)
{
    const NBRE_Vector<NBRE_ModelPtr>& models = meshLayer->GetModels();
    for(NBRE_Vector<NBRE_ModelPtr>::const_iterator iter = models.begin(); iter != models.end(); ++iter)
    {
        NBRE_EntityPtr entity(NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), *iter, -1));
        entity->SetOverlayId(buildingOverlay);

        NBRE_EntityPtr extEnt = AttachEntity(entity);
        float nearVisibility = meshLayer->GetLayerInfo().nearVisibility;
        float farVisibility = meshLayer->GetLayerInfo().farVisibility;

        EntityAssistInfo info(entity.get(), extEnt.get(), nearVisibility, farVisibility, "LM3D", meshLayer->GetLayerInfo().layerType);
        mEntityAssistInfoList.push_back(info);
    }
    mIsBuildingTile = TRUE;
}

void
NBGM_TileMapLayerTile::BuildRasterEntities(NBGM_NBMMESHLayerData* meshLayer, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager,
                                           uint8 drawOrder, int32 subDrawOrder)
{
    const NBRE_Vector<NBRE_ModelPtr>& models = meshLayer->GetModels();
    for(NBRE_Vector<NBRE_ModelPtr>::const_iterator iter = models.begin(); iter != models.end(); ++iter)
    {
        NBRE_EntityPtr entity(NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), *iter, -1));
        int32 overlayId = overlayManager.AssignOverlayId(DrawOrderStruct(drawOrder, subDrawOrder, 0));
        mOverlayIds.insert(overlayId);
        entity->SetOverlayId(overlayId);

        NBRE_EntityPtr extEnt = AttachEntity(entity);
        float nearVisibility = meshLayer->GetLayerInfo().nearVisibility;
        float farVisibility = meshLayer->GetLayerInfo().farVisibility;

        EntityAssistInfo info(entity.get(), extEnt.get(), nearVisibility, farVisibility, "RASTER", meshLayer->GetLayerInfo().layerType);
        mEntityAssistInfoList.push_back(info);
    }
}

static bool IsPolyline(NBGM_NBMData* nbmData)
{
    return nbmData->GetId().find("GEOGRAPHIC_") != string::npos;
}

void
NBGM_TileMapLayerTile::LoadMapdata(NBGM_NBMData* nbmData, NBRE_ITextureAnimation& texcoorAnimation, int8 zoomLevel, int32 buildingOverlay, int32 glowOverlay, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, uint8 drawOrder, int32 subDrawOrder, nb_boolean enablePicking, uint32 selectMask)
{
    mZoomLevel = zoomLevel;
    mIsBuildingTile = FALSE;
    mIsPolylineTile = IsPolyline(nbmData);

    for(uint32 i = 0; i < nbmData->GetLayers().size(); ++i)
    {
        NBGM_NBMLayerData* layer = nbmData->GetLayers()[i];
        if(layer == NULL)
        {
            continue;
        }

        if(layer->GetLayerInfo().layerType == NBRE_DLT_Associated && layer->GetLayerInfo().layerDataType != NBRE_LDT_TPSH)
        {
            continue;
        }

        switch(layer->GetLayerInfo().layerDataType)
        {
        case NBRE_LDT_AREA:
            {
                NBGM_NBMAREALayerData* areaLayer = static_cast<NBGM_NBMAREALayerData*> (layer);
                if (areaLayer->Is2DBuilding())
                {
                    Build2DBuildingEntities(areaLayer, overlayManager, drawOrder, subDrawOrder, enablePicking, selectMask, i);
                }
                else
                {
                    BuildAreaEntities(areaLayer, overlayManager, drawOrder, subDrawOrder, enablePicking, selectMask, i);
                }
            }
            break;
        case NBRE_LDT_LPTH:
            {
                NBGM_NBMLPTHLayerData* lpthLayer = static_cast<NBGM_NBMLPTHLayerData*> (layer);
                if (layer->GetLayerInfo().layerType == NBRE_DLT_Route)
                {
                    BuildRouteEntities(lpthLayer, texcoorAnimation, zoomLevel, overlayManager, drawOrder,
                                       subDrawOrder, enablePicking, selectMask, i, mIsPolylineTile ? glowOverlay : -1);
                }
                else
                {
                    BuildRoadEntities(lpthLayer, texcoorAnimation, zoomLevel, overlayManager, drawOrder,
                                      subDrawOrder, enablePicking, selectMask, i, mIsPolylineTile ? glowOverlay : -1);
                }
            }
            break;
        case NBRE_LDT_LPDR:
            {
                NBGM_NBMLPDRLayerData* lpdrLayer = static_cast<NBGM_NBMLPDRLayerData*> (layer);
                if (layer->GetLayerInfo().layerType == NBRE_DLT_Route)
                {
                    BuildRouteEntities(lpdrLayer, texcoorAnimation, zoomLevel, overlayManager, drawOrder,
                                       subDrawOrder, enablePicking, selectMask, i);
                }
                else
                {
                    BuildRoadEntities(lpdrLayer, texcoorAnimation, zoomLevel, overlayManager, drawOrder,
                                      subDrawOrder, enablePicking, selectMask, i);
                }
            }
            break;
        case NBRE_LDT_MESH:
            {
                NBGM_NBMMESHLayerData* meshLayer = static_cast<NBGM_NBMMESHLayerData*> (layer);
                if (layer->GetLayerInfo().layerType == NBRE_DLT_UnTextureBuilding)
                {
                    BuildUT3DEntities(meshLayer, buildingOverlay);
                }
                else if(layer->GetLayerInfo().layerType == NBRE_DLT_UnTextureLandmarkBuilding)
                {
                    BuildUTLM3DEntities(meshLayer, buildingOverlay);
                }
                else if(layer->GetLayerInfo().layerType == NBRE_DLT_LandmarkBuilding)
                {
                    BuildLM3DEntities(meshLayer, buildingOverlay);
                }
                else if(layer->GetLayerInfo().layerType == NBRE_DLT_Raster)
                {
                    BuildRasterEntities(meshLayer, overlayManager, drawOrder, subDrawOrder);
                }
            }
            break;
        default:
            break;
        }
    }
}

void NBGM_TileMapLayerTile::Active()
{
    EntityAssistInfoList::iterator iter = mEntityAssistInfoList.begin();
    EntityAssistInfoList::iterator iterEnd = mEntityAssistInfoList.end();

    for(; iter != iterEnd; ++iter)
    {
        iter->mEntity->SetVisible(TRUE);
        iter->mExtendEntity->SetVisible(TRUE);
    }
    mIsActive = TRUE;
}

void NBGM_TileMapLayerTile::Deactive()
{
    EntityAssistInfoList::iterator iter = mEntityAssistInfoList.begin();
    EntityAssistInfoList::iterator iterEnd = mEntityAssistInfoList.end();

    for(; iter != iterEnd; ++iter)
    {
        iter->mEntity->SetVisible(FALSE);
        iter->mExtendEntity->SetVisible(FALSE);
    }
    mIsActive = FALSE;
}

void NBGM_TileMapLayerTile::UpdateEntityVisibility(const NBRE_Planef& cameraPlane)
{
    if(!mIsActive)
    {
        return;
    }

    float distance = NBRE_Math::Abs(cameraPlane.DistanceTo(NBRE_TypeConvertf::Convert(mTileCenter)));
    distance = mNBGMContext.ModelToWorld(MERCATER_TO_METER(distance));

    EntityAssistInfoList::iterator iter = mEntityAssistInfoList.begin();
    EntityAssistInfoList::iterator iterEnd = mEntityAssistInfoList.end();

//    NBRE_DebugLog(PAL_LogSeverityDebug, "==========NBGM_TileMapLayer::VectorMapTile::UpdateEntityVisibility=========\rTile ID = %s\rlayers count is = %d\rcamera distance    = %20.10f m"
//                  ,mTileID.c_str(), static_cast<uint32>(mEntityAssistInfoList.size()), mNBGMContext.ModelToWorld(MERCATER_TO_METER(distance)));

//    uint32 layerIndex = 0;
    for (; iter != iterEnd; ++iter)
    {
//        NBRE_DebugLog(PAL_LogSeverityDebug, "-----------------------------------------------------------------------\r"
//                                            "layer index = %d, layer type = %s\r"
//                                            "tile near distance = %20.10f m\r"
//                                            "tile far distance  = %20.10f m",
//                                            layerIndex++,
//                                            iter->mType.c_str(),
//                                            mNBGMContext.ModelToWorld(MERCATER_TO_METER(iter->mNearVisibility)),
//                                            mNBGMContext.ModelToWorld(MERCATER_TO_METER(iter->mFarVisibility)));


        nb_boolean isVisible = CheckNearFarVisibility(distance, iter->mNearVisibility, iter->mFarVisibility);
        iter->mEntity->SetVisible(isVisible);
        iter->mExtendEntity->SetVisible(isVisible);
    }

    for (LayerCollisionSpaceTable::iterator it = mCollisionLayers.begin(); it != mCollisionLayers.end(); ++it)
    {
        LayerCollisionSpace* lcs = it->second;
        lcs->mVisible = CheckNearFarVisibility(distance, lcs->mLayerInfo.nearVisibility, lcs->mLayerInfo.farVisibility);
    }
}

NBRE_NodePtr NBGM_TileMapLayerTile::Node()
{
    return mNode;
}

NBRE_EntityPtr NBGM_TileMapLayerTile::AttachEntity(const NBRE_EntityPtr& entity)
{
    mNode->AttachObject(entity);
    NBRE_Entity* extendEntity = entity.get()->Duplicate();
    NBRE_EntityPtr exntendEntityPtr(extendEntity);
    mExtendNode->AttachObject(exntendEntityPtr);

    return exntendEntityPtr;
}

void NBGM_TileMapLayerTile::NotifyExtend(const NBGM_TileMapBoundaryExtendType& extendType)
{
    if (!mIsActive)
    {
        return;
    }

    if(mExtendType != extendType)
    {
        switch(extendType)
        {
        case NBGM_TMBET_NONE:
            mNode->RemoveChild(mExtendNode.get());
            break;
        case NBGM_TMBET_LEFT:
            mExtendNode->SetPosition(NBRE_Point3f(mNBGMContext.WorldToModel(-NBRE_Math::PiTwo), 0.0f, 0.0f));
            if(mExtendType == NBGM_TMBET_NONE)
            {
                mNode->AddChild(mExtendNode);
            }
            break;
        case NBGM_TMBET_RIGHT:
            mExtendNode->SetPosition(NBRE_Point3f(mNBGMContext.WorldToModel(NBRE_Math::PiTwo), 0.0f, 0.0f));
            if(mExtendType == NBGM_TMBET_NONE)
            {
                mNode->AddChild(mExtendNode);
            }
            break;
        default:
            break;
        }
        mExtendType = extendType;
    }
}

void
NBGM_TileMapLayerTile::SetOpacity(float opacity)
{
    if (mOpacity == opacity)
    {
        return;
    }

    if(!mInitOpacityAnimation)
    {
        int32 entityCount = mNode->AttachedObjectsNumber();
        for (int32 i = 0; i < entityCount; ++i)
        {
            NBRE_Entity* entity = (NBRE_Entity*)mNode->GetAttachedObject(i);
            if(entity)
            {
                shared_ptr<NBGM_VertextOpacityAnimation> animation = shared_ptr<NBGM_VertextOpacityAnimation>(NBRE_NEW NBGM_VertextOpacityAnimation(opacity));
                entity->SetVertexAnimation(animation);
                mOpacityAnimations.push_back(animation);
            }
        }
        mInitOpacityAnimation = TRUE;
    }
    else
    {
        for (NBRE_Vector<shared_ptr<NBGM_VertextOpacityAnimation> >::iterator iter = mOpacityAnimations.begin(); iter != mOpacityAnimations.end(); ++iter)
        {
            (*iter)->SetOpacity(opacity);
        }
    }

    mOpacity = opacity;
}

NBGM_TileMapLayerTile::NBGM_VertextOpacityAnimation::NBGM_VertextOpacityAnimation(float opacity)
    :mOpacity(opacity), mDirtyFrameNumber(1)
{
}

NBGM_TileMapLayerTile::NBGM_VertextOpacityAnimation::~NBGM_VertextOpacityAnimation()
{
}

void
NBGM_TileMapLayerTile::NBGM_VertextOpacityAnimation::Animate(NBRE_VertexData& vertexData, NBRE_IndexData& /*indexData*/)
{
    NBRE_VertexDeclaration::VertexElementList& declList = vertexData.GetVertexDeclaration()->GetVertexElementList();
    for (NBRE_VertexDeclaration::VertexElementList::iterator declIt = declList.begin(); declIt != declList.end(); ++declIt)
    {
        NBRE_VertexElement* declElem = *declIt;
        if (declElem->Semantic() == NBRE_VertexElement::VES_DIFFUSE)
        {
            NBRE_HardwareVertexBuffer* vertexBuffer = vertexData.GetBuffer(declElem->Source());
            float* pVertex = (float*)vertexBuffer->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY);
            uint32 vertexCount = vertexBuffer->GetNumVertices();
            for (uint32 vIdx = 0; vIdx < vertexCount; ++vIdx)
            {
                float* pV = pVertex + vIdx * 4;
                pV[3] = mOpacity;
            }
            vertexBuffer->UnLock();
        }
    }
}

void
NBGM_TileMapLayerTile::NBGM_VertextOpacityAnimation::SetOpacity(float opacity)
{
    if(mOpacity != opacity)
    {
        mOpacity = opacity;
        ++mDirtyFrameNumber;
    }
}

nb_boolean
NBGM_TileMapLayerTile::HitTest(const NBRE_Vector2d& center, double radius, uint32 selectMask, NBRE_Set<NBGM_ElementId>& ids)
{
    nb_boolean hit = FALSE;

    if (!mIsActive)
    {
        return FALSE;
    }

    // Polygon check region, converted to tile local coordinates
    NBRE_Vector2d tileCenter(mNode->WorldPosition().x, mNode->WorldPosition().y);
    NBRE_Vector2d rd(radius, radius);
    NBRE_AxisAlignedBox2d localBox(center - tileCenter - rd, center - tileCenter + rd);
    float roadScale = GetPolylineScaleFactor(mZoomLevel, TRUE);
    NBRE_CollisionObject2d* co = NBRE_CollisionObjectBuilder2d::CreateCircle2d(NULL, center - tileCenter, radius, selectMask);

    NBRE_CollisionObject2dList cos;
    for (LayerCollisionSpaceTable::iterator it = mCollisionLayers.begin(); it != mCollisionLayers.end(); ++it)
    {
        LayerCollisionSpace* lcs = it->second;
        if (!lcs->mVisible || !NBRE_Intersectiond::HitTest(lcs->mAABB, localBox))
        {
            continue;
        }

        if (lcs->mStaticSpace)
        {
            if (NBRE_CollisionDetector2d::HitTest(lcs->mStaticSpace, co, selectMask, cos))
            {
                hit = TRUE;
                //@TODO: Use element id instead of layer id after NBM file is ready
                //ids.insert(it->first);
            }
        }

        NBRE_ISpatialPartition2d* dynamicSpace = lcs->mDynamicCollisionSpace;
        if (dynamicSpace)
        {
            NBRE_CollisionObject2d* co2 = NBRE_CollisionObjectBuilder2d::CreateCircle2d(NULL, center - tileCenter, radius + lcs->mWidth * 0.5 * roadScale, selectMask);
            if (NBRE_CollisionDetector2d::HitTest(dynamicSpace, co2, selectMask, cos))
            {
                hit = TRUE;
            }
            NBRE_DELETE co2;
        }
    }

    if (hit)
    {
        for (uint32 coIdx = 0; coIdx < cos.size(); ++coIdx)
        {
            NBGM_ElementId* eid = (NBGM_ElementId*)cos[coIdx]->GetOwner();
            if (eid)
            {
                ids.insert(*eid);
            }
        }
    }

#ifdef ENABLE_COLLISION_DETECT_DEBUG_DRAW
    for (uint32 i = 0; i < cos.size(); ++i)
    {
        NBRE_CollisionObject2d* c = cos[i];
        NBRE_Vector<NBRE_ICollisionPrimitive2d*> polygonList;
        if (c->GetType() == NBRE_COT_SINGLE)
        {
            continue;
        }

        ((NBRE_CompositeCollisionObject2d*)c)->GetPrimitives(polygonList);
        for (uint32 k = 0; k < polygonList.size(); ++k)
        {
            if (polygonList[k]->GetType() == NBRE_CPT_POLYGON2)
            {
                NBRE_CollisionPolygon2d* tri = (NBRE_CollisionPolygon2d*)polygonList[k];
                NBRE_Model* model = NBRE_NEW NBRE_Model(NBRE_MeshPtr(
                    CreateDebugPolygonEntity(mNBGMContext.renderingEngine->Context()
                    , &tri->GetVertices()[0], tri->GetVertices().size(),
                    NBRE_Color(1, 0, 0, 1))
                    ));
                mNode->AttachObject(NBRE_EntityPtr(NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), NBRE_ModelPtr(model), 65843)));
            }
            else if (polygonList[k]->GetType() == NBRE_CPT_LINESEGMENT2)
            {
                NBRE_CollisionLineSegment2d* tri = (NBRE_CollisionLineSegment2d*)polygonList[k];
                NBRE_Polyline2d coPl(tri->GetEndPoints(), 2);
                NBRE_Model* model = NBRE_NEW NBRE_Model(NBRE_MeshPtr(
                    CreateDebugPolylineEntity(mNBGMContext.renderingEngine->Context()
                    , NBRE_TypeConvertf::Convert(coPl), NBRE_Color(1,0,0,1))
                    ));
                mNode->AttachObject(NBRE_EntityPtr(NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), NBRE_ModelPtr(model), 65843)));
            }
        }
    }
#endif

    NBRE_DELETE co;
    return hit;
}

void
NBGM_TileMapLayerTile::GetPolylineCollisionSpace(LayerCollisionSpace* lcs,
                                                 const NBRE_Vector<NBGM_PolylineData*>& pls)
{
    for (uint32 i = 0; i < pls.size(); ++i)
    {
        NBRE_Polyline2d* pl = pls[i]->polyline;
        uint32 n = pl->VertexCount();
        for (uint32 j = 0; j < n; ++j)
        {
            lcs->mAABB.Merge(pl->Vertex(j));
        }
        float plWidth = (float)pl->GetWidth();
        if (lcs->mWidth < plWidth)
        {
            lcs->mWidth = plWidth;
        }
    }
    if (!lcs->mAABB.IsNull())
    {
        NBRE_Vector2d extend(lcs->mWidth * 0.5, lcs->mWidth * 0.5);
        lcs->mAABB.minExtend -= extend;
        lcs->mAABB.maxExtend += extend;
    }
}

NBGM_TileMapLayerTile::LayerCollisionSpace*
NBGM_TileMapLayerTile::GetLayerCollisionSpace(NBGM_NBMLayerData* layer, uint32 index)
{
    LayerCollisionSpace* lcs = NULL;
    LayerCollisionSpaceTable::iterator it = mCollisionLayers.find(index);
    if (it == mCollisionLayers.end())
    {
        lcs = NBRE_NEW LayerCollisionSpace(layer->GetLayerInfo());
        mCollisionLayers[index] = lcs;

        switch(layer->GetLayerInfo().layerDataType)
        {
        case NBRE_LDT_AREA:
            {
                NBGM_NBMAREALayerData* areaLayer = static_cast<NBGM_NBMAREALayerData*> (layer);
                NBRE_ModelPtr model = areaLayer->GetAreaModel();
                const NBRE_AxisAlignedBox3f& aabb3 = model->GetMesh()->AxisAlignedBox();
                NBRE_AxisAlignedBox2d aabb2(aabb3.minExtend.x, aabb3.minExtend.y, aabb3.maxExtend.x, aabb3.maxExtend.y);
                lcs->mAABB.Merge(aabb2);
            }
            break;
        case NBRE_LDT_LPTH:
            {
                NBGM_NBMLPTHLayerData* lpthLayer = static_cast<NBGM_NBMLPTHLayerData*> (layer);
                const NBRE_Vector<NBGM_PolylineData*>& pls = lpthLayer->GetTracks();
                GetPolylineCollisionSpace(lcs, pls);
            }
            break;
        case NBRE_LDT_LPDR:
            {
                NBGM_NBMLPDRLayerData* lpdrLayer = static_cast<NBGM_NBMLPDRLayerData*> (layer);
                const NBRE_Vector<NBGM_PolylineData*>& pls = lpdrLayer->GetTracks();
                GetPolylineCollisionSpace(lcs, pls);
            }
            break;
        default:
            break;
        }
    }
    else
    {
        lcs = it->second;
    }
    return lcs;
}

NBGM_TileMapLayerTile::LayerCollisionSpace::LayerCollisionSpace(const NBRE_LayerInfo& layerInfo)
:mLayerInfo(layerInfo)
,mStaticSpace(NULL)
,mWidth(0)
,mDynamicCollisionSpace(NULL)
,mVisible(FALSE)
{
}

NBGM_TileMapLayerTile::LayerCollisionSpace::~LayerCollisionSpace()
{
    NBRE_DELETE mStaticSpace;
    NBRE_DELETE mDynamicCollisionSpace;
}

nb_boolean
NBGM_TileMapLayerTile::CheckIfHasLM3D()
{
    EntityAssistInfoList::iterator iter = mEntityAssistInfoList.begin();
    EntityAssistInfoList::iterator iterEnd = mEntityAssistInfoList.end();
    for(; iter != iterEnd; ++iter)
    {
        if(iter->mLayerType == NBRE_DLT_LandmarkBuilding)
        {
            return TRUE;
        }
    }
    return FALSE;
}

void
NBGM_TileMapLayerTile::DisableUnTexturedLandMarks()
{
    EntityAssistInfoList::iterator iter = mEntityAssistInfoList.begin();
    EntityAssistInfoList::iterator iterEnd = mEntityAssistInfoList.end();
    for(; iter != iterEnd; ++iter)
    {
        if(iter->mLayerType == NBRE_DLT_UnTextureLandmarkBuilding)
        {
            iter->mEntity->SetVisible(FALSE);
            iter->mExtendEntity->SetVisible(FALSE);
        }
    }
}

void
NBGM_TileMapLayerTile::SwitchRoadOverlays(nb_boolean useFrameBuffer, int32 transparentRoadBgOverlay, int32 transparentRoadFgOverlay)
{
    for (NBRE_Map<NBRE_Entity*, int32>::iterator it = mRoadBgEntityOverlapMap.begin(); it != mRoadBgEntityOverlapMap.end(); ++it)
    {
        it->first->SetOverlayId(useFrameBuffer ? transparentRoadBgOverlay : it->second);
    }

    for (NBRE_Map<NBRE_Entity*, int32>::iterator it = mRoadFgEntityOverlapMap.begin(); it != mRoadFgEntityOverlapMap.end(); ++it)
    {
        it->first->SetOverlayId(useFrameBuffer ? transparentRoadFgOverlay : it->second);
    }
}

NBGM_TileMapLayerTileCreator::NBGM_TileMapLayerTileCreator(NBGM_Context& nbgmContext, NBRE_ITextureAnimation* texcoorAnimation, int8* zoomLevel, int32* transparentRoadBgOverlay, int32* buildingOverlay, int32* glowOverlay)
:mNBGMContext(nbgmContext)
,mTexcoorAnimation(texcoorAnimation)
,mZoomLevel(zoomLevel)
,mTransparentRoadBgOverlay(transparentRoadBgOverlay)
,mBuildingOverlay(buildingOverlay)
,mGlowOverlay(glowOverlay)
{
}

NBGM_TileMapLayerTileCreator::~NBGM_TileMapLayerTileCreator()
{
}

NBGM_IMapLayerTile*
NBGM_TileMapLayerTileCreator::CreateMapLayerTile(const NBGM_VectorTileID& tileId, NBGM_NBMData* nbmData, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, uint8 drawOrder, int32 subDrawOrder, uint8 /*labelDrawOrder*/, nb_boolean enablePicking, uint32 selectMask, const NBRE_String& /*materialCategoryName*/)
{
    nbre_assert(nbmData);

    nb_boolean hasVectorTile = FALSE;

    for(NBRE_Vector<NBGM_NBMLayerData*>::const_iterator iter = nbmData->GetLayers().begin(); iter != nbmData->GetLayers().end(); ++iter)
    {
        NBGM_NBMLayerData* layer = *iter;
        if(layer->GetLayerInfo().layerDataType == NBRE_LDT_AREA || layer->GetLayerInfo().layerDataType == NBRE_LDT_LPTH
            || layer->GetLayerInfo().layerDataType == NBRE_LDT_MESH ||  layer->GetLayerInfo().layerDataType == NBRE_LDT_LPDR)
        {
            hasVectorTile = TRUE;
            break;
        }
    }

    if (hasVectorTile)
    {
        NBGM_TileMapLayerTile* tile = NBRE_NEW NBGM_TileMapLayerTile(mNBGMContext, nbmData->GetCenter(), tileId, drawOrder);
        tile->LoadMapdata(nbmData, *mTexcoorAnimation, *mZoomLevel, *mBuildingOverlay, *mGlowOverlay, overlayManager, drawOrder, subDrawOrder, enablePicking, selectMask);
        return tile;
    }
    return NULL;
}
