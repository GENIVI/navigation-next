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

    @file nbgmnbmdata.h

*/
/*
(C) Copyright 2011 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef _NBRE_NBMLAYERDATA_H_
#define _NBRE_NBMLAYERDATA_H_

#include "nbgmvectortiledata.h"
#include "nbgmmapmaterialmanager.h"
#include "nbgmbuildutility.h"
#include "nbrepolyline2.h"

class NBGM_VectorVertexScale;
struct NBGM_NBMDataLoadInfo;

class NBGM_TextData
{
public:
    NBGM_TextData();
    ~NBGM_TextData();
public:
    void Initialize(NBRE_LayerTEXTData* data, const NBRE_MapMateriaGroup& materialGroup, uint16 materialFlag);
    uint16 GetPriority() const { return priority; }
public:
    NBRE_Vector< NBRE_Pair<uint32, NBRE_String> > textList;
    uint32 fontMaterialId;
    const NBRE_MapMaterial* fontMaterial;
    uint16 priority;
};

class NBGM_PointLabelData
{
public:
    NBGM_PointLabelData();
    ~NBGM_PointLabelData();
public:
    void Initialize(NBRE_LayerPNTSData* data, const NBRE_MapMateriaGroup& materialGroup, uint16 materialFlag);
    uint16 GetPriority() const { return priority; }
public:
    uint16 materialId;
    const NBRE_MapMaterial* material;
    uint16 fontMaterialId;
    const NBRE_MapMaterial* fontMaterial;
    NBRE_Vector3d position;
    NBGM_TextData* text;
    NBRE_String id;
    uint16 priority;
};

class NBGM_RoadLabelData
{
public:
    NBGM_RoadLabelData();
    ~NBGM_RoadLabelData();
public:
    void Initialize(NBRE_LayerTPTHData* data, const NBRE_MapMateriaGroup& materialGroup);
    uint16 GetPriority() const { return priority; }
public:
    float width;
    NBRE_Vector<NBRE_Point2f> track;
    NBGM_TextData* text;
    uint16 priority;
};

class NBGM_ShieldData
{
public:
    NBGM_ShieldData();
    ~NBGM_ShieldData();
public:
    void Initialize(NBRE_LayerTPSHData* data, const NBRE_MapMateriaGroup& materialGroup);
    uint16 GetPriority() const { return priority; }
public:
    uint16 materialId;
    const NBRE_MapMaterial* material;
    uint16 fontMaterialId;
    const NBRE_MapMaterial* fontMaterial;
    NBRE_Vector<NBRE_Point2f> track;
    NBGM_TextData* text;
    uint16 priority;
};

class NBGM_PathArrowData
{
public:
    NBGM_PathArrowData();
    ~NBGM_PathArrowData();
public:
    void Initialize(NBRE_LayerTPARData* data, const NBRE_MapMateriaGroup& materialGroup);
    uint16 GetPriority() const { return priority; }
public:
    uint16 materialId;
    const NBRE_MapMaterial* material;
    NBRE_Vector<NBRE_Point2f> track;
    uint16 priority;
};

class NBGM_PoiData
{
public:
    NBGM_PoiData();
    ~NBGM_PoiData();
public:
    void Initialize(NBRE_LayerPOISData* data, const NBRE_MapMateriaGroup& materialGroup);
public:
    uint16 pointId;
    NBRE_String id;
    NBGM_PointLabelData* pointLabel;
    uint16 selectedMaterialId;
    const NBRE_MapMaterial* selectedMaterial;
};

class NBGM_GpinData
{
public:
    NBGM_GpinData();
    ~NBGM_GpinData();
public:
    void Initialize(NBRE_LayerGPINData* data, const NBRE_MapMateriaGroup& materialGroup);
public:
    NBRE_Vector2f position;
    NBRE_String id;
    float radius;
    uint16 materialId;
    const NBRE_MapMaterial* material;
};

class NBGM_NBMLayerData
{
public:
    NBGM_NBMLayerData(const NBRE_LayerInfo& info);
    virtual ~NBGM_NBMLayerData();
public:
    virtual PAL_Error LoadLayerData(NBGM_ResourceContext& resourceContext, NBRE_Layer* layer, const NBRE_Point3d& refCenter, const NBGM_NBMDataLoadInfo& tileInfo) = 0;
    const NBRE_LayerInfo& GetLayerInfo() { return mLayerInfo; }
protected:
    NBRE_LayerInfo  mLayerInfo;
};

class NBGM_NBMData
{
public:
    NBGM_NBMData(const NBRE_String& id, const NBRE_Vector3d& center, const NBRE_String& groupName);
    ~NBGM_NBMData();
public:
    void LoadNBMData(NBGM_ResourceContext& resourceContext, NBRE_Mapdata* mapdata, const NBGM_NBMDataLoadInfo& tileInfo, uint16 materialFlag);
    const NBRE_Vector<NBGM_NBMLayerData*>& GetLayers() { return mLayers; }
    const NBRE_String& GetId() { return mId; }
    const NBRE_Vector3d& GetCenter() { return mCenter; }
    const NBRE_String& GetGroupName() { return mGroupName; }

private:
    NBGM_NBMLayerData* CreateLayerData(const NBRE_LayerInfo& info);
private:
    NBRE_String mId;
    NBRE_Vector3d mCenter;
    NBRE_String mGroupName;
    NBRE_Vector<NBGM_NBMLayerData*> mLayers;
};

struct NBGM_PolylineData
{
public:
    NBGM_PolylineData() {}
    ~NBGM_PolylineData() { NBRE_DELETE polyline; }
public:
    NBRE_Polyline2d* polyline;
    NBGM_ElementId id;
};

class NBGM_NBMLPTHLayerData : public NBGM_NBMLayerData
{
public:
    NBGM_NBMLPTHLayerData(const NBRE_LayerInfo& info);
    virtual ~NBGM_NBMLPTHLayerData();
public:
    virtual PAL_Error LoadLayerData(NBGM_ResourceContext& resourceContext, NBRE_Layer* layer, const NBRE_Point3d& refCenter, const NBGM_NBMDataLoadInfo& tileInfo);
    const NBRE_ModelPtr& GetRoadModel() { return mRoadModel; }
    const NBRE_ModelPtr& GetBackgroundRoadModel() { return mBackgroundRoadModel; }
    shared_ptr<NBGM_VectorVertexScale> GetRoadAnimation() { return mRoadAnimation; }
    shared_ptr<NBGM_VectorVertexScale> GetBackgroundRoadAnimation() { return mBackgroundRoadAnimation; }
    const NBRE_Set<NBRE_SubModel*>& GetAnimatedSubModels() { return mAnimatedSubModels; }
    const NBRE_Vector<NBGM_PolylineData*>& GetTracks() { return mTracks; }
private:
    NBRE_ModelPtr mRoadModel;
    NBRE_ModelPtr mBackgroundRoadModel;
    shared_ptr<NBGM_VectorVertexScale> mRoadAnimation;
    shared_ptr<NBGM_VectorVertexScale> mBackgroundRoadAnimation;
    NBRE_Set<NBRE_SubModel*> mAnimatedSubModels;
    NBRE_Vector<NBGM_PolylineData*> mTracks;
};

class NBGM_NBMLPDRLayerData : public NBGM_NBMLayerData
{
public:
    NBGM_NBMLPDRLayerData(const NBRE_LayerInfo& info);
    virtual ~NBGM_NBMLPDRLayerData();
public:
    virtual PAL_Error LoadLayerData(NBGM_ResourceContext& resourceContext, NBRE_Layer* layer, const NBRE_Point3d& refCenter, const NBGM_NBMDataLoadInfo& tileInfo);
    const NBRE_Map<uint16, NBRE_ModelPtr>& GetRoadModel() { return mRoadModel; }
    const NBRE_Map<uint16, NBRE_ModelPtr>& GetBackgroundRoadModel() { return mBackgroundRoadModel; }
    NBRE_Map<uint16, shared_ptr<NBGM_VectorVertexScale> >& GetRoadAnimation() { return mRoadAnimation; }
    NBRE_Map<uint16, shared_ptr<NBGM_VectorVertexScale> >& GetBackgroundRoadAnimation() { return mBackgroundRoadAnimation; }
    const NBRE_Map<uint16, NBRE_Set<NBRE_SubModel*>*>& GetAnimatedSubModels() { return mAnimatedSubModels; }
    const NBRE_Vector<NBGM_PolylineData*>& GetTracks() { return mTracks; }
private:
    NBRE_Map<uint16, NBRE_ModelPtr> mRoadModel;
    NBRE_Map<uint16, NBRE_ModelPtr> mBackgroundRoadModel;
    NBRE_Map<uint16, shared_ptr<NBGM_VectorVertexScale> > mRoadAnimation;
    NBRE_Map<uint16, shared_ptr<NBGM_VectorVertexScale> > mBackgroundRoadAnimation;
    NBRE_Map<uint16, NBRE_Set<NBRE_SubModel*>*> mAnimatedSubModels;
    NBRE_Vector<NBGM_PolylineData*> mTracks;
};

class NBGM_NBMAREALayerData : public NBGM_NBMLayerData
{
public:
    NBGM_NBMAREALayerData(const NBRE_LayerInfo& info);
    virtual ~NBGM_NBMAREALayerData();
public:
    virtual PAL_Error LoadLayerData(NBGM_ResourceContext& resourceContext, NBRE_Layer* layer, const NBRE_Point3d& refCenter, const NBGM_NBMDataLoadInfo& tileInfo);
    nb_boolean Is2DBuilding() { return mIs2DBuilding; }
    const NBRE_ModelPtr& GetAreaModel() { return mAreaModel; }
    const NBRE_ModelPtr& GetOutlineAreaModel() { return mOutlineAreaModel; }
    const NBRE_Vector<NBGM_PointLabelData*>& GetPointLabels() { return mPointLabels; }
private:
    nb_boolean CheckIfIs2DBuilding(NBGM_ResourceContext& resourceContext, NBRE_Layer* layerData, const NBRE_String& materialCategoryName);
private:
    nb_boolean mIs2DBuilding;
    NBRE_ModelPtr mAreaModel;
    NBRE_ModelPtr mOutlineAreaModel;
    NBRE_Vector<NBGM_PointLabelData*> mPointLabels;
};

class NBGM_NBMTPTHLayerData : public NBGM_NBMLayerData
{
public:
    NBGM_NBMTPTHLayerData(const NBRE_LayerInfo& info);
    virtual ~NBGM_NBMTPTHLayerData();
public:
    virtual PAL_Error LoadLayerData(NBGM_ResourceContext& resourceContext, NBRE_Layer* layer, const NBRE_Point3d& refCenter, const NBGM_NBMDataLoadInfo& tileInfo);
    const NBRE_Vector<NBGM_RoadLabelData*>& GetRoadLabels() { return mRoadLabels; }
private:
    NBRE_Vector<NBGM_RoadLabelData*> mRoadLabels;
};

class NBGM_NBMPNTSLayerData : public NBGM_NBMLayerData
{
public:
    NBGM_NBMPNTSLayerData(const NBRE_LayerInfo& info);
    virtual ~NBGM_NBMPNTSLayerData();
public:
    virtual PAL_Error LoadLayerData(NBGM_ResourceContext& resourceContext, NBRE_Layer* layer, const NBRE_Point3d& refCenter, const NBGM_NBMDataLoadInfo& tileInfo);
    const NBRE_Vector<NBGM_PointLabelData*>& GetPointLabels() { return mPointLabels; }
private:
    NBRE_Vector<NBGM_PointLabelData*> mPointLabels;
};

class NBGM_NBMMESHLayerData : public NBGM_NBMLayerData
{
public:
    NBGM_NBMMESHLayerData(const NBRE_LayerInfo& info);
    virtual ~NBGM_NBMMESHLayerData();
public:
    virtual PAL_Error LoadLayerData(NBGM_ResourceContext& resourceContext, NBRE_Layer* layer, const NBRE_Point3d& refCenter, const NBGM_NBMDataLoadInfo& tileInfo);
    const NBRE_Vector<NBRE_ModelPtr>& GetModels() { return mModels; }
    const NBRE_Vector<NBGM_PointLabelData*>& GetPointLabels() { return mPointLabels; }
private:
    void GetVertexCount(NBRE_Vector<NBRE_ModelPtr>* models, uint32& vertexCount, uint32& indexCount);
    void MergeModels(NBRE_IRenderPal& renderPal, NBRE_Vector<NBRE_ModelPtr>* models, NBRE_Vector<NBRE_ModelPtr>& outModels);
private:
    NBRE_Vector<NBRE_ModelPtr> mModels;
    NBRE_Vector<NBGM_PointLabelData*> mPointLabels;
};

class NBGM_NBMTPSHLayerData : public NBGM_NBMLayerData
{
public:
    NBGM_NBMTPSHLayerData(const NBRE_LayerInfo& info);
    virtual ~NBGM_NBMTPSHLayerData();
public:
    virtual PAL_Error LoadLayerData(NBGM_ResourceContext& resourceContext, NBRE_Layer* layer, const NBRE_Point3d& refCenter, const NBGM_NBMDataLoadInfo& tileInfo);
    const NBRE_Vector<NBGM_ShieldData*>& GetShields() { return mShields; }
private:
    NBRE_Vector<NBGM_ShieldData*> mShields;
};

class NBGM_NBMTPARLayerData : public NBGM_NBMLayerData
{
public:
    NBGM_NBMTPARLayerData(const NBRE_LayerInfo& info);
    virtual ~NBGM_NBMTPARLayerData();
public:
    virtual PAL_Error LoadLayerData(NBGM_ResourceContext& resourceContext, NBRE_Layer* layer, const NBRE_Point3d& refCenter, const NBGM_NBMDataLoadInfo& tileInfo);
    const NBRE_ModelPtr GetModel() { return mModel; }
    const NBRE_Vector<NBGM_PathArrowData*>& GetPathArrows() { return mPathArrows; }
private:
    void CreateMergedLayerData(const NBRE_Layer* layer, NBRE_Vector<NBGM_PathArrowData*>& pathArrowDataList, NBRE_MapMaterialManager& materialManager, const NBRE_String& materialCategoryName);
    nb_boolean MergeTrackPolyline(NBRE_Vector<NBRE_Point2f>& desArray, NBRE_Array* srcArray);
    PAL_Error BuildModel(NBGM_ResourceContext& resourceContext, const NBRE_Vector<NBGM_PathArrowData*>& pathArrowDataList, const NBRE_String& materialCategoryName);
    NBRE_String BuildPathArrowVertex(NBGM_ResourceContext& resourceContext, const NBRE_Vector<NBGM_PathArrowData*>& pathArrowDataList, int8 zoomlevel, NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& texcoordBuffer, NBGM_ShortBuffer& indexBuffer, const NBRE_String& materialCategoryName);
    NBRE_MeshPtr BuildPathArrowMesh(NBGM_ResourceContext& resourceContext, const NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& texcoordBuffer, const NBGM_ShortBuffer& indexBuffer);
private:
    NBRE_ModelPtr mModel;
    NBRE_Vector<NBGM_PathArrowData*> mPathArrows;
};

class NBGM_NBMPOISLayerData : public NBGM_NBMLayerData
{
public:
    NBGM_NBMPOISLayerData(const NBRE_LayerInfo& info);
    virtual ~NBGM_NBMPOISLayerData();
public:
    virtual PAL_Error LoadLayerData(NBGM_ResourceContext& resourceContext, NBRE_Layer* layer, const NBRE_Point3d& refCenter, const NBGM_NBMDataLoadInfo& tileInfo);
    const NBRE_Vector<NBGM_PoiData*>& GetPois() { return mPois; }
private:
    NBRE_Vector<NBGM_PoiData*> mPois;
};

class NBGM_NBMGPINLayerData : public NBGM_NBMLayerData
{
public:
    NBGM_NBMGPINLayerData(const NBRE_LayerInfo& info);
    virtual ~NBGM_NBMGPINLayerData();
public:
    virtual PAL_Error LoadLayerData(NBGM_ResourceContext& resourceContext, NBRE_Layer* layer, const NBRE_Point3d& refCenter, const NBGM_NBMDataLoadInfo& tileInfo);
    const NBRE_Vector<NBGM_GpinData*>& GetGpins() { return mGpins; }
private:
    NBRE_Vector<NBGM_GpinData*> mGpins;
};

#endif
