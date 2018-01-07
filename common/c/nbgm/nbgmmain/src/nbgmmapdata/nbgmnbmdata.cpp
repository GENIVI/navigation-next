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

#include "nbgmnbmdata.h"
#include "nbgmvectortilebuilder.h"
#include "nbgmmapmaterial.h"
#include "nbgmcommon.h"
#include "nbgmconst.h"
#include "nbgmvectorvertexscale.h"
#include "nbgmmapviewimpltask.h"

NBGM_TextData::NBGM_TextData()
    :fontMaterialId(NBRE_INVALID_SHORT_INDEX)
    ,fontMaterial(NULL)
    ,priority(0xFFFF)
{
}

NBGM_TextData::~NBGM_TextData()
{
    if(VERIFY_NBM_INDEX(fontMaterialId) && VERIFY_NBM_INTERNAL_INDEX((uint16)fontMaterialId) && fontMaterial != NULL)
    {
        NBRE_DELETE fontMaterial;
    }
}

void
NBGM_TextData::Initialize(NBRE_LayerTEXTData* data, const NBRE_MapMateriaGroup& materialGroup, uint16 materialFlag)
{
    if(data != NULL)
    {
        uint32 size = NBRE_ArrayGetSize(data->labelArray);
        for(uint32 i = 0; i < size; ++i)
        {
            NBRE_LayerLabelData* labelData = (NBRE_LayerLabelData*)NBRE_ArrayGetAt(data->labelArray, i);
            if(labelData != NULL && labelData->label != NULL)
            {
                NBRE_Pair<uint32, NBRE_String> pair(labelData->languageCode, NBRE_String(labelData->label));
                textList.push_back(pair);
            }
        }
        if(VERIFY_NBM_INDEX(data->materialId))
        {
            fontMaterialId = ((uint32)materialFlag << 16) + data->materialId;
            if(VERIFY_NBM_INTERNAL_INDEX(data->materialId))
            {
                fontMaterial = data->material;
            }
            else
            {
                const NBRE_MapMaterial& mapMat = materialGroup.GetMapMaterial(materialFlag, MAKE_NBM_INDEX(data->materialId));
                if(mapMat.Type() != NBRE_MMT_FontMaterial)
                {
                    NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM_TextData::Initialize  UnMatched Material Type!, Type = %d, Index = %d", mapMat.Type(), data->materialId);
                    fontMaterial = NULL;
                }
                else
                {
                    fontMaterial = &mapMat;
                }
                priority = materialGroup.GetMapMaterialPriority(materialFlag, MAKE_NBM_INDEX(data->materialId));
            }
        }
    }
}


NBGM_PointLabelData::NBGM_PointLabelData()
    :materialId(NBRE_INVALID_SHORT_INDEX)
    ,material(NULL)
    ,fontMaterialId(NBRE_INVALID_SHORT_INDEX)
    ,fontMaterial(NULL)
    ,text(NULL)
    ,priority(0xFFFF)
{
}

NBGM_PointLabelData::~NBGM_PointLabelData()
{
    //TODO: Always get internal materialId, but there is no internal material in data.

    //if(VERIFY_NBM_INDEX(fontMaterialId) && VERIFY_NBM_INTERNAL_INDEX(fontMaterialId) && fontMaterial != NULL)
    //{
    //    NBRE_DELETE fontMaterial;
    //}

    if(VERIFY_NBM_INDEX(materialId) && VERIFY_NBM_INTERNAL_INDEX(materialId) && material != NULL)
    {
        NBRE_DELETE material;
    }

    if(text != NULL)
    {
        NBRE_DELETE text;
    }
}

void
NBGM_PointLabelData::Initialize(NBRE_LayerPNTSData* data, const NBRE_MapMateriaGroup& materialGroup, uint16 materialFlag)
{
    if(data != NULL)
    {
        if(data->label != NULL)
        {
            text = NBRE_NEW NBGM_TextData();
            text->Initialize(data->label, materialGroup, materialFlag);
        }
        position = NBRE_Vector3d(data->position.x, data->position.y, 0);
        if(VERIFY_NBM_INDEX(data->materialId))
        {
            materialId = data->materialId;
            if(VERIFY_NBM_INTERNAL_INDEX(data->materialId))
            {
                material = data->material;
            }
            else
            {
                material = &(materialGroup.GetMapMaterial(materialFlag, MAKE_NBM_INDEX(data->materialId)));
                priority = materialGroup.GetMapMaterialPriority(materialFlag, MAKE_NBM_INDEX(data->materialId));
            }
        }
        if(material != NULL && material->Type() == NBRE_MMT_ShieldMaterial)
        {
            const NBRE_MapShieldMaterial* shieldMaterial = static_cast<const NBRE_MapShieldMaterial*> (material);
            if(VERIFY_NBM_INDEX(shieldMaterial->GetFontMaterialIndex()))
            {
                fontMaterialId = shieldMaterial->GetFontMaterialIndex();

                //TODO: Always get internal materialId, but there is no internal material in data.

                //if(VERIFY_NBM_INTERNAL_INDEX(fontMaterialId))
                //{
                //    fontMaterial = NULL;
                //    NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM_PointLabelData::Initialize, wrong font material Id");
                //    nbre_assert(0);
                //}
                //else
                {
                    fontMaterial = &(materialGroup.GetMapMaterial(materialFlag, MAKE_NBM_INDEX(fontMaterialId)));
                }
            }
        }
        if (data->id)
        {
            id = *data->id;
        }
    }
}

NBGM_RoadLabelData::NBGM_RoadLabelData()
    :width(0)
    ,text(NULL)
    ,priority(0xFFFF)
{
}

NBGM_RoadLabelData::~NBGM_RoadLabelData()
{
    if(text != NULL)
    {
        NBRE_DELETE text;
    }
}

void
NBGM_RoadLabelData::Initialize(NBRE_LayerTPTHData* data, const NBRE_MapMateriaGroup& materialGroup)
{
    if(data != NULL)
    {
        if(data->label != NULL)
        {
            text = NBRE_NEW NBGM_TextData();
            text->Initialize(data->label, materialGroup, 0);
            priority = text->GetPriority();
        }
        width = data->width;
        uint32 vertexCount = NBRE_ArrayGetSize(data->polyline);
        for(uint32 i = 0; i < vertexCount; ++i)
        {
            NBRE_Point2f* pt = (NBRE_Point2f*)NBRE_ArrayGetAt(data->polyline, i);
            track.push_back(*pt);
        }
    }
}

NBGM_ShieldData::NBGM_ShieldData()
    :materialId(NBRE_INVALID_SHORT_INDEX)
    ,material(NULL)
    ,fontMaterialId(NBRE_INVALID_SHORT_INDEX)
    ,fontMaterial(NULL)
    ,text(NULL)
    ,priority(0xFFFF)
{
}

NBGM_ShieldData::~NBGM_ShieldData()
{
    //TODO: Always get internal materialId, but there is no internal material in data.
    //if(VERIFY_NBM_INDEX(fontMaterialId) && VERIFY_NBM_INTERNAL_INDEX(fontMaterialId) && fontMaterial != NULL)
    //{
    //    NBRE_DELETE fontMaterial;
    //}

    if(VERIFY_NBM_INDEX(materialId) && VERIFY_NBM_INTERNAL_INDEX(materialId) && material != NULL)
    {
        NBRE_DELETE material;
    }

    if(text != NULL)
    {
        NBRE_DELETE text;
    }
}

void
NBGM_ShieldData::Initialize(NBRE_LayerTPSHData* data, const NBRE_MapMateriaGroup& materialGroup)
{
    if(data != NULL)
    {
        if(data->label != NULL)
        {
            text = NBRE_NEW NBGM_TextData();
            text->Initialize(data->label, materialGroup, 0);
        }

        if(VERIFY_NBM_INDEX(data->materialId))
        {
            materialId = data->materialId;
            if(VERIFY_NBM_INTERNAL_INDEX(data->materialId))
            {
                material = NULL;
                NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM_ShieldData::Initialize, wrong material Id");
                nbre_assert(0);
            }
            else
            {
                material = &(materialGroup.GetMapMaterial(MAKE_NBM_INDEX(data->materialId)));
                priority = materialGroup.GetMapMaterialPriority(0, MAKE_NBM_INDEX(data->materialId));
            }
        }

        if(material != NULL && material->Type() == NBRE_MMT_ShieldMaterial)
        {
            const NBRE_MapShieldMaterial* shieldMaterial = static_cast<const NBRE_MapShieldMaterial*> (material);
            if(VERIFY_NBM_INDEX(shieldMaterial->GetFontMaterialIndex()))
            {
                fontMaterialId = shieldMaterial->GetFontMaterialIndex();
                //TODO: Always get internal materialId, but there is no internal material in data.
                //if(VERIFY_NBM_INTERNAL_INDEX(fontMaterialId))
                //{
                //    fontMaterial = NULL;
                //    NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM_ShieldData::Initialize, wrong font material Id");
                //    nbre_assert(0);
                //}
                //else
                {
                    fontMaterial = &(materialGroup.GetMapMaterial(MAKE_NBM_INDEX(fontMaterialId)));
                }
            }
        }

        uint32 vertexCount = NBRE_ArrayGetSize(data->polyline);
        for (uint32 i = 0; i < vertexCount; ++i)
        {
            NBRE_Point2f* pt = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(data->polyline, i));
            track.push_back(*pt);
        }
    }
}

NBGM_PathArrowData::NBGM_PathArrowData()
    :materialId(NBRE_INVALID_SHORT_INDEX)
    ,material(NULL)
    ,priority(0xFFFF)
{
}

NBGM_PathArrowData::~NBGM_PathArrowData()
{
    //if(VERIFY_NBM_INDEX(materialId) && VERIFY_NBM_INTERNAL_INDEX(materialId) && material != NULL)
    //{
    //    NBRE_DELETE material;
    //}
}

void
NBGM_PathArrowData::Initialize(NBRE_LayerTPARData* data, const NBRE_MapMateriaGroup& materialGroup)
{
    if(data != NULL)
    {
        if(VERIFY_NBM_INDEX(data->materialId))
        {
            materialId = data->materialId;

            //TODO: current nbm data is incorrect, the index refers to internal materials

            //if(VERIFY_NBM_INTERNAL_INDEX(data->materialId))
            //{
            //    material = NULL;
            //    NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM_PathArrowData::Initialize, wrong material Id");
            //    nbre_assert(0);
            //}
            //else
            {
                material = &(materialGroup.GetMapMaterial(MAKE_NBM_INDEX(data->materialId)));
                priority = materialGroup.GetMapMaterialPriority(0, MAKE_NBM_INDEX(data->materialId));
            }
        }
        uint32 vertexCount = NBRE_ArrayGetSize(data->polyline);
        for (uint32 i = 0; i < vertexCount; ++i)
        {
            NBRE_Point2f* pt = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(data->polyline, i));
            track.push_back(*pt);
        }
    }
}

NBGM_PoiData::NBGM_PoiData()
    :pointId(0)
    ,pointLabel(NULL)
    ,selectedMaterialId(NBRE_INVALID_SHORT_INDEX)
    ,selectedMaterial(NULL)
{
}

NBGM_PoiData::~NBGM_PoiData()
{
    if(VERIFY_NBM_INDEX(selectedMaterialId) && VERIFY_NBM_INTERNAL_INDEX(selectedMaterialId) && selectedMaterial != NULL)
    {
        NBRE_DELETE selectedMaterial;
    }

    if(pointLabel != NULL)
    {
        NBRE_DELETE pointLabel;
    }
}

void
NBGM_PoiData::Initialize(NBRE_LayerPOISData* data, const NBRE_MapMateriaGroup& materialGroup)
{
    if(data != NULL)
    {
        pointId = data->pointId;
        id = NBRE_String(data->identifier);
        if(data->point != NULL)
        {
            pointLabel = NBRE_NEW NBGM_PointLabelData();
            pointLabel->Initialize(data->point, materialGroup, 0);
        }
        if(VERIFY_NBM_INDEX(data->selectedMaterialId))
        {
            selectedMaterialId = data->selectedMaterialId;
            if(VERIFY_NBM_INTERNAL_INDEX(data->selectedMaterialId))
            {
                selectedMaterial = data->selectedMaterial;
            }
            else
            {
                selectedMaterial = &(materialGroup.GetMapMaterial(MAKE_NBM_INDEX(data->selectedMaterialId)));
            }
        }
    }
}

NBGM_GpinData::NBGM_GpinData()
    :radius(0.0f)
    ,materialId(NBRE_INVALID_SHORT_INDEX)
    ,material(NULL)
{
}

NBGM_GpinData::~NBGM_GpinData()
{
    if(VERIFY_NBM_INDEX(materialId) && VERIFY_NBM_INTERNAL_INDEX(materialId) && material != NULL)
    {
        NBRE_DELETE material;
    }
}

void
NBGM_GpinData::Initialize(NBRE_LayerGPINData* data, const NBRE_MapMateriaGroup& materialGroup)
{
    if(data != NULL)
    {
        id = NBRE_String(data->identifier);
        position = data->location;
        radius = data->radius;
        if(VERIFY_NBM_INDEX(data->materialId))
        {
            materialId = data->materialId;
            if(VERIFY_NBM_INTERNAL_INDEX(data->materialId))
            {
                material = data->material;
            }
            else
            {
                material = &(materialGroup.GetMapMaterial(MAKE_NBM_INDEX(data->materialId)));
            }
        }
    }
}

NBGM_NBMLayerData::NBGM_NBMLayerData(const NBRE_LayerInfo& info)
    :mLayerInfo(info)
{
}

NBGM_NBMLayerData::~NBGM_NBMLayerData()
{
}

NBGM_NBMData::NBGM_NBMData(const NBRE_String& id, const NBRE_Vector3d& center, const NBRE_String& groupName)
    :mId(id)
    ,mCenter(center)
    ,mGroupName(groupName)
{
}

NBGM_NBMData::~NBGM_NBMData()
{
    for(NBRE_Vector<NBGM_NBMLayerData*>::iterator iter = mLayers.begin(); iter != mLayers.end(); ++iter)
    {
        NBRE_DELETE (*iter);
    }
}

void
NBGM_NBMData::LoadNBMData(NBGM_ResourceContext& resourceContext, NBRE_Mapdata* mapdata, const NBGM_NBMDataLoadInfo& tileInfo, uint16 materialFlag)
{
    uint32 count = NBRE_ArrayGetSize(mapdata->layerArray);
    for(uint32 i = 0; i < count; ++i)
    {
        NBRE_Layer* layer = static_cast<NBRE_Layer*>(NBRE_ArrayGetAt(mapdata->layerArray, i));
        layer->info.materialFlag = materialFlag;
        NBGM_NBMLayerData* layerData = CreateLayerData(layer->info);
        if(layerData)
        {
            PAL_Error err = layerData->LoadLayerData(resourceContext, layer, mapdata->refCenter, tileInfo);
            if(err == PAL_Ok)
            {
                mLayers.push_back(layerData);
            }
            else
            {
                NBRE_DELETE layerData;
                NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM_NBMData::LoadNBMData, LoadLayerData Failed, layer type = %d, index = %d, err = 0x%08x", layer->info.layerType, i, err);
            }
        }
    }
}

NBGM_NBMLayerData*
NBGM_NBMData::CreateLayerData(const NBRE_LayerInfo& info)
{
    NBGM_NBMLayerData* layerData = NULL;

    switch(info.layerDataType)
    {
    case NBRE_LDT_LPTH:
        layerData = NBRE_NEW NBGM_NBMLPTHLayerData(info);
        break;
    case NBRE_LDT_AREA:
        layerData = NBRE_NEW NBGM_NBMAREALayerData(info);
        break;
    case NBRE_LDT_TPTH:
        layerData = NBRE_NEW NBGM_NBMTPTHLayerData(info);
        break;
    case NBRE_LDT_PNTS:
        layerData = NBRE_NEW NBGM_NBMPNTSLayerData(info);
        break;
    case NBRE_LDT_MESH:
        layerData = NBRE_NEW NBGM_NBMMESHLayerData(info);
        break;
    case NBRE_LDT_TPSH:
        layerData = NBRE_NEW NBGM_NBMTPSHLayerData(info);
        break;
    case NBRE_LDT_TPAR:
        layerData = NBRE_NEW NBGM_NBMTPARLayerData(info);
        break;
    case NBRE_LDT_POIS:
        layerData = NBRE_NEW NBGM_NBMPOISLayerData(info);
        break;
    case NBRE_LDT_GPIN:
        layerData = NBRE_NEW NBGM_NBMGPINLayerData(info);
        break;
    case NBRE_LDT_LPDR:
        layerData = NBRE_NEW NBGM_NBMLPDRLayerData(info);
        break;
    default:
        break;
    }

    return layerData;
}


NBGM_NBMLPTHLayerData::NBGM_NBMLPTHLayerData(const NBRE_LayerInfo& info)
    :NBGM_NBMLayerData(info)
{
}

NBGM_NBMLPTHLayerData::~NBGM_NBMLPTHLayerData()
{
    for (uint32 i = 0; i < mTracks.size(); ++i)
    {
        NBRE_DELETE mTracks[i];
    }
}

PAL_Error
NBGM_NBMLPTHLayerData::LoadLayerData(NBGM_ResourceContext& resourceContext, NBRE_Layer* layer, const NBRE_Point3d& /*refCenter*/, const NBGM_NBMDataLoadInfo& tileInfo)
{
    PAL_Error err = PAL_Ok;
    if(mLayerInfo.layerType == NBRE_DLT_Route)
    {
        NBRE_Model* foreModel = NBRE_NEW NBRE_Model();
        err = NBGM_VectorTileBuilder::CreateVectorRoadModel(resourceContext, layer, foreModel, NULL, mAnimatedSubModels, tileInfo.materialCategoryName);
        if(err == PAL_Ok)
        {
            mRoadModel.reset(foreModel);
            mRoadAnimation = shared_ptr<NBGM_VectorVertexScale>(NBRE_NEW NBGM_RoadPolylineVertexScale());
            mRoadModel->SetVertexAnimation(mRoadAnimation);
        }
        else
        {
            NBRE_DELETE foreModel;
        }
    }
    else
    {
        NBRE_Model* foreModel = NBRE_NEW NBRE_Model();
        NBRE_Model* backModel = NBRE_NEW NBRE_Model();
        err = NBGM_VectorTileBuilder::CreateVectorRoadModel(resourceContext, layer, foreModel, backModel, mAnimatedSubModels, tileInfo.materialCategoryName);
        if(err == PAL_Ok)
        {
            if(tileInfo.enablePicking)
            {
                mRoadModel.reset(foreModel);
                mRoadAnimation = shared_ptr<NBGM_VectorVertexScale>(NBRE_NEW NBGM_GeoPolylineVertexScale());
                mRoadModel->SetVertexAnimation(mRoadAnimation);
                mBackgroundRoadModel.reset(backModel);
                mBackgroundRoadAnimation = shared_ptr<NBGM_VectorVertexScale>(NBRE_NEW NBGM_GeoPolylineVertexScale());
                mBackgroundRoadModel->SetVertexAnimation(mBackgroundRoadAnimation);
            }
            else
            {
                mRoadModel.reset(foreModel);
                mRoadAnimation = shared_ptr<NBGM_VectorVertexScale>(NBRE_NEW NBGM_RoadPolylineVertexScale());
                mRoadModel->SetVertexAnimation(mRoadAnimation);
                mBackgroundRoadModel.reset(backModel);
                mBackgroundRoadAnimation = shared_ptr<NBGM_VectorVertexScale>(NBRE_NEW NBGM_RoadPolylineVertexScale());
                mBackgroundRoadModel->SetVertexAnimation(mBackgroundRoadAnimation);
            }
        }
        else
        {
            NBRE_DELETE foreModel;
            NBRE_DELETE backModel;
        }
    }

    for (uint32 i = 0; i < layer->dataCount; ++i)
    {
        NBRE_LayerLPTHData* roadData = static_cast<NBRE_LayerLPTHData*>(layer->data[i]);
        uint32 vs = NBRE_ArrayGetSize(roadData->polyline);
        if (vs >= 2)
        {
            NBRE_Polyline2d* pl = NBRE_NEW NBRE_Polyline2d(vs);
            for (uint32 j = 0; j < vs; ++j)
            {
                NBRE_Point2f* pt = (NBRE_Point2f*)NBRE_ArrayGetAt(roadData->polyline, j);
                pl->SetVertex(j, NBRE_Vector2d(pt->x, pt->y));
            }
            pl->UpdateLength();
            pl->SetWidth(roadData->width);

            NBGM_PolylineData* pd = NBRE_NEW NBGM_PolylineData;
            if (roadData->elementId)
            {
                pd->id = *roadData->elementId;
            }
            pd->polyline = pl;
            mTracks.push_back(pd);
        }
    }
    return err;
}

NBGM_NBMLPDRLayerData::NBGM_NBMLPDRLayerData(const NBRE_LayerInfo& info)
    :NBGM_NBMLayerData(info)
{
}

NBGM_NBMLPDRLayerData::~NBGM_NBMLPDRLayerData()
{
    for (NBRE_Map<uint16, NBRE_Set<NBRE_SubModel*>*>::iterator it = mAnimatedSubModels.begin(); it != mAnimatedSubModels.end(); ++it)
    {
        NBRE_DELETE it->second;
    }
    for (uint32 i = 0; i < mTracks.size(); ++i)
    {
        NBRE_DELETE mTracks[i];
    }
}

PAL_Error
NBGM_NBMLPDRLayerData::LoadLayerData(NBGM_ResourceContext& resourceContext, NBRE_Layer* layer, const NBRE_Point3d& /*refCenter*/, const NBGM_NBMDataLoadInfo& tileInfo)
{
    PAL_Error err = PAL_Ok;
    if(mLayerInfo.layerType == NBRE_DLT_Route)
    {
        NBRE_Map<uint16, NBRE_Model*> foreModel;

        err = NBGM_VectorTileBuilder::CreateVectorRoadModel(resourceContext, layer, &foreModel, NULL, mAnimatedSubModels, tileInfo.materialCategoryName);
        if(err == PAL_Ok)
        {
            for (NBRE_Map<uint16, NBRE_Model*>::iterator it = foreModel.begin(); it != foreModel.end(); ++it)
            {
                NBRE_ModelPtr roadModel(it->second);
                shared_ptr<NBGM_VectorVertexScale> roadAnimation = shared_ptr<NBGM_VectorVertexScale>(NBRE_NEW NBGM_RoadPolylineVertexScale());
                mRoadAnimation[it->first] = roadAnimation;
                roadModel->SetVertexAnimation(roadAnimation);
                mRoadModel[it->first] = roadModel;
            }
        }
        else
        {
            for (NBRE_Map<uint16, NBRE_Model*>::iterator it = foreModel.begin(); it != foreModel.end(); ++it)
            {
                NBRE_DELETE it->second;
            }
        }
    }
    else
    {
        NBRE_Map<uint16, NBRE_Model*> foreModel;
        NBRE_Map<uint16, NBRE_Model*> backModel;
        err = NBGM_VectorTileBuilder::CreateVectorRoadModel(resourceContext, layer, &foreModel, &backModel, mAnimatedSubModels, tileInfo.materialCategoryName);
        if(err == PAL_Ok)
        {
            if (foreModel.size() > 1)
            {
                if (foreModel.size())
                {
                }
            }
            for (NBRE_Map<uint16, NBRE_Model*>::iterator it = foreModel.begin(); it != foreModel.end(); ++it)
            {
                NBRE_ModelPtr roadModel(it->second);
                shared_ptr<NBGM_VectorVertexScale> roadAnimation = shared_ptr<NBGM_VectorVertexScale>(NBRE_NEW NBGM_RoadPolylineVertexScale());
                mRoadAnimation[it->first] = roadAnimation;
                roadModel->SetVertexAnimation(roadAnimation);
                mRoadModel[it->first] = roadModel;
            }

            for (NBRE_Map<uint16, NBRE_Model*>::iterator it = backModel.begin(); it != backModel.end(); ++it)
            {
                NBRE_ModelPtr roadModel(it->second);
                shared_ptr<NBGM_VectorVertexScale> roadAnimation = shared_ptr<NBGM_VectorVertexScale>(NBRE_NEW NBGM_RoadPolylineVertexScale());
                mBackgroundRoadAnimation[it->first] = roadAnimation;
                roadModel->SetVertexAnimation(roadAnimation);
                mBackgroundRoadModel[it->first] = roadModel;
            }
        }
        else
        {
            for (NBRE_Map<uint16, NBRE_Model*>::iterator it = foreModel.begin(); it != foreModel.end(); ++it)
            {
                NBRE_DELETE it->second;
            }
            for (NBRE_Map<uint16, NBRE_Model*>::iterator it = backModel.begin(); it != backModel.end(); ++it)
            {
                NBRE_DELETE it->second;
            }
        }
    }

    for (uint32 i = 0; i < layer->dataCount; ++i)
    {
        NBRE_LayerLPDRData* roadData = static_cast<NBRE_LayerLPDRData*>(layer->data[i]);
        uint32 vs = NBRE_ArrayGetSize(roadData->polyline);
        if (vs >= 2)
        {
            NBRE_Polyline2d* pl = NBRE_NEW NBRE_Polyline2d(vs);
            for (uint32 j = 0; j < vs; ++j)
            {
                NBRE_Point2f* pt = (NBRE_Point2f*)NBRE_ArrayGetAt(roadData->polyline, j);
                pl->SetVertex(j, NBRE_Vector2d(pt->x, pt->y));
            }
            pl->UpdateLength();
            pl->SetWidth(roadData->width);

            NBGM_PolylineData* pd = NBRE_NEW NBGM_PolylineData;
            if (roadData->elementId)
            {
                pd->id = *roadData->elementId;
            }
            pd->polyline = pl;
            mTracks.push_back(pd);
        }
    }
    return err;
}

NBGM_NBMAREALayerData::NBGM_NBMAREALayerData(const NBRE_LayerInfo& info)
    :NBGM_NBMLayerData(info)
    ,mIs2DBuilding(FALSE)
{
}

NBGM_NBMAREALayerData::~NBGM_NBMAREALayerData()
{
    for(NBRE_Vector<NBGM_PointLabelData*>::iterator iter = mPointLabels.begin(); iter != mPointLabels.end(); ++iter)
    {
        NBGM_PointLabelData* pointLabelData = *iter;
        NBRE_DELETE pointLabelData;
    }
}

nb_boolean
NBGM_NBMAREALayerData::CheckIfIs2DBuilding(NBGM_ResourceContext& resourceContext, NBRE_Layer* layerData, const NBRE_String& materialCategoryName)
{
    for (uint32 i = 0; i < layerData->dataCount; ++i)
    {
        NBRE_LayerAREAData* areaData = static_cast<NBRE_LayerAREAData*>(layerData->data[i]);
        NBRE_MapMaterialType type = NBRE_MMT_None;

        if (areaData->material == NULL)
        {
            const NBRE_MapMateriaGroup& materiaGroup = resourceContext.mapMaterialManager->GetCurrentGroup(materialCategoryName);
            const NBRE_MapMaterial& material = materiaGroup.GetMapMaterial(areaData->materialId);
            type = material.Type();
        }
        else
        {
            type = areaData->material->Type();
        }

        if (type == NBRE_MMT_OutlinedComplexColorFillMaterial ||
            type == NBRE_MMT_OutlinedSimpleColorFillMaterial ||
            type == NBRE_MMT_OutlinedHoleyMaterial)
        {
            return TRUE;
        }
    }

    return FALSE;
}

PAL_Error
NBGM_NBMAREALayerData::LoadLayerData(NBGM_ResourceContext& resourceContext, NBRE_Layer* layer, const NBRE_Point3d& refCenter, const NBGM_NBMDataLoadInfo& tileInfo)
{
    PAL_Error err = PAL_Ok;

    mIs2DBuilding = CheckIfIs2DBuilding(resourceContext, layer, tileInfo.materialCategoryName);

    if(mIs2DBuilding)
    {
        NBRE_Model* areaModel = NBGM_VectorTileBuilder::CreateVectorAreaModel(resourceContext, layer, refCenter, tileInfo.materialCategoryName);
        NBRE_Model* outLineModel = NBGM_VectorTileBuilder::CreateVectorAreaWireFrameModel(resourceContext, layer, refCenter, tileInfo.materialCategoryName);
        if(areaModel == NULL || outLineModel == NULL)
        {
            err = PAL_Failed;
        }
        mAreaModel.reset(areaModel);
        mOutlineAreaModel.reset(outLineModel);
    }
    else
    {
        NBRE_Model* areaModel = NBGM_VectorTileBuilder::CreateVectorAreaModel(resourceContext, layer, refCenter, tileInfo.materialCategoryName);
        if(areaModel == NULL)
        {
            err = PAL_Failed;
        }
        mAreaModel.reset(areaModel);
    }

    NBRE_MapMaterialManager& materialMgr = *(resourceContext.mapMaterialManager);
    for(uint32 i = 0; i < layer->dataCount; ++i)
    {
        NBRE_LayerAREAData* area = static_cast<NBRE_LayerAREAData*>(layer->data[i]);
        if (area == NULL || area->label == NULL || !VERIFY_NBM_INDEX(area->label->label->materialId))
        {
            continue;
        }
        NBGM_PointLabelData* pointLabelData = NBRE_NEW NBGM_PointLabelData();
        pointLabelData->Initialize(area->label, materialMgr.GetCurrentGroup(tileInfo.materialCategoryName), 0);
        mPointLabels.push_back(pointLabelData);
    }

    return err;
}

NBGM_NBMTPTHLayerData::NBGM_NBMTPTHLayerData(const NBRE_LayerInfo& info)
    :NBGM_NBMLayerData(info)
{
}

NBGM_NBMTPTHLayerData::~NBGM_NBMTPTHLayerData()
{
    for(NBRE_Vector<NBGM_RoadLabelData*>::iterator iter = mRoadLabels.begin(); iter != mRoadLabels.end(); ++iter)
    {
        NBRE_DELETE (*iter);
    }
}

PAL_Error
NBGM_NBMTPTHLayerData::LoadLayerData(NBGM_ResourceContext& resourceContext, NBRE_Layer* layer, const NBRE_Point3d& /*refCenter*/, const NBGM_NBMDataLoadInfo& tileInfo)
{
    NBRE_MapMaterialManager& materialMgr = *(resourceContext.mapMaterialManager);
    for(uint32 i = 0; i < layer->dataCount; ++i)
    {
        NBRE_LayerTPTHData* track = static_cast<NBRE_LayerTPTHData*>(layer->data[i]);
        if (track == NULL || track->label == NULL || !VERIFY_NBM_INDEX(track->label->materialId))
        {
            continue;
        }
        NBGM_RoadLabelData* roadLabel = NBRE_NEW NBGM_RoadLabelData();
        roadLabel->Initialize(track, materialMgr.GetCurrentGroup(tileInfo.materialCategoryName));
        mRoadLabels.push_back(roadLabel);
    }
    return PAL_Ok;
}

NBGM_NBMPNTSLayerData::NBGM_NBMPNTSLayerData(const NBRE_LayerInfo& info)
    :NBGM_NBMLayerData(info)
{
}

NBGM_NBMPNTSLayerData::~NBGM_NBMPNTSLayerData()
{
    for(NBRE_Vector<NBGM_PointLabelData*>::iterator iter = mPointLabels.begin(); iter != mPointLabels.end(); ++iter)
    {
        NBGM_PointLabelData* pointLabelData = *iter;
        NBRE_DELETE pointLabelData;
    }
}

PAL_Error
NBGM_NBMPNTSLayerData::LoadLayerData(NBGM_ResourceContext& resourceContext, NBRE_Layer* layer, const NBRE_Point3d& /*refCenter*/, const NBGM_NBMDataLoadInfo& tileInfo)
{
    NBRE_MapMaterialManager& materialMgr = *(resourceContext.mapMaterialManager);
    for (uint32 i = 0; i < layer->dataCount; ++i)
    {
        NBRE_LayerPNTSData* pnts = (NBRE_LayerPNTSData*)layer->data[i];
        if (pnts == NULL || pnts->label == NULL)
        {
            continue;
        }
        NBGM_PointLabelData* pointLabelData = NBRE_NEW NBGM_PointLabelData();
        pointLabelData->Initialize(pnts, materialMgr.GetCurrentGroup(tileInfo.materialCategoryName), layer->info.materialFlag);
        mPointLabels.push_back(pointLabelData);
    }
    return PAL_Ok;
}

NBGM_NBMMESHLayerData::NBGM_NBMMESHLayerData(const NBRE_LayerInfo& info)
    :NBGM_NBMLayerData(info)
{
}

NBGM_NBMMESHLayerData::~NBGM_NBMMESHLayerData()
{
    for(NBRE_Vector<NBGM_PointLabelData*>::iterator iter = mPointLabels.begin(); iter != mPointLabels.end(); ++iter)
    {
        NBGM_PointLabelData* pointLabelData = *iter;
        NBRE_DELETE pointLabelData;
    }
}

void
NBGM_NBMMESHLayerData::GetVertexCount(NBRE_Vector<NBRE_ModelPtr>* models, uint32& vertexCount, uint32& indexCount)
{
    for (uint32 i = 0; i < models->size(); ++i)
    {
        NBRE_ModelPtr& en = models->at(i);
        NBRE_VertexData* shareVertexData = NULL;
        for (uint32 j = 0; j < en->GetSubModelNumber(); ++j)
        {
            NBRE_SubModel& subModel = en->GetSubModel(j);
            NBRE_SubMesh* subMesh = subModel.GetSubMesh();
            NBRE_IndexData* indexData = subMesh->GetIndexData();
            if(subMesh->UseSharedVertices())
            {
                shareVertexData = subMesh->GetVertexData();
            }
            else
            {
                NBRE_VertexData* vertexData = subMesh->GetVertexData();
                vertexCount += vertexData->GetVertexCount();
            }
            indexCount += indexData->IndexCount();
        }
        if(shareVertexData)
        {
            vertexCount += shareVertexData->GetVertexCount();
        }
    }
}

void
NBGM_NBMMESHLayerData::MergeModels(NBRE_IRenderPal& renderPal, NBRE_Vector<NBRE_ModelPtr>* models, NBRE_Vector<NBRE_ModelPtr>& outModels)
{
    static const uint32 MaxVextexCount = 65536;

    NBRE_String shaderName;
    uint32 vertexCount = 0;
    uint32 indexCount = 0;
    GetVertexCount(models, vertexCount, indexCount);

    NBRE_VertexData* vertexData = NULL;

    NBRE_Vector3f* pPosition = NULL;
    NBRE_Vector2f* pTexcoord = NULL;
    NBRE_Vector3f* pNormal = NULL;
    uint16 baseIndex = 0;
    uint16 indexOffset = 0;

    NBRE_IndexData* indexData = NULL;
    uint16* pIndex = NULL;

    NBRE_Mesh* mesh = NULL;

    NBRE_PrimitiveType pType = NBRE_PMT_POINT_LIST;

    nb_boolean hasNormal = FALSE;
    if (models->size() > 0)
    {
        NBRE_ModelPtr m = models->at(0);
        if (m->GetSubModelNumber() > 0)
        {
            NBRE_SubMesh* sm = m->GetSubModel(0).GetSubMesh();
            if (sm)
            {
                NBRE_VertexDeclaration* decl = sm->GetVertexData()->GetVertexDeclaration();
                NBRE_VertexDeclaration::VertexElementList& dl = decl->GetVertexElementList();
                for (NBRE_VertexDeclaration::VertexElementList::iterator it = dl.begin(); it != dl.end(); ++it)
                {
                    NBRE_VertexElement* ve = *it;
                    if (ve->Semantic() == NBRE_VertexElement::VES_NORMAL)
                    {
                        hasNormal = TRUE;
                        break;
                    }
                }
            }
        }
    }

    for (uint32 i = 0; i < models->size(); ++i)
    {
        NBRE_ModelPtr& en = models->at(i);
        for (uint32 j = 0; j < en->GetSubModelNumber(); )
        {
            if(mesh == NULL)
            {
                NBRE_VertexDeclaration* decl = renderPal.CreateVertexDeclaration();
                decl->GetVertexElementList().push_back(NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0));
                decl->GetVertexElementList().push_back(NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0));
                if (hasNormal)
                {
                    decl->GetVertexElementList().push_back(NBRE_NEW NBRE_VertexElement(2, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_NORMAL, 0));
                }
                vertexData = NBRE_NEW NBRE_VertexData(0);
                vertexData->SetVertexDeclaration(decl);

                if(vertexCount > MaxVextexCount)
                {
                    vertexData->AddBuffer(renderPal.CreateVertexBuffer(sizeof(NBRE_Vector3f), MaxVextexCount, TRUE, NBRE_HardwareBuffer::HBU_STATIC));
                    vertexData->AddBuffer(renderPal.CreateVertexBuffer(sizeof(NBRE_Vector2f), MaxVextexCount, TRUE, NBRE_HardwareBuffer::HBU_STATIC));
                    if (hasNormal)
                    {
                        vertexData->AddBuffer(renderPal.CreateVertexBuffer(sizeof(NBRE_Vector3f), MaxVextexCount, TRUE, NBRE_HardwareBuffer::HBU_STATIC));
                    }
                }
                else
                {
                    vertexData->AddBuffer(renderPal.CreateVertexBuffer(sizeof(NBRE_Vector3f), vertexCount, TRUE, NBRE_HardwareBuffer::HBU_STATIC));
                    vertexData->AddBuffer(renderPal.CreateVertexBuffer(sizeof(NBRE_Vector2f), vertexCount, TRUE, NBRE_HardwareBuffer::HBU_STATIC));
                    if (hasNormal)
                    {
                        vertexData->AddBuffer(renderPal.CreateVertexBuffer(sizeof(NBRE_Vector3f), vertexCount, TRUE, NBRE_HardwareBuffer::HBU_STATIC));
                    }
                }

                pPosition = (NBRE_Vector3f*)vertexData->GetBuffer(0)->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY);
                pTexcoord = (NBRE_Vector2f*)vertexData->GetBuffer(1)->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY);
                if (hasNormal)
                {
                    pNormal = (NBRE_Vector3f*)vertexData->GetBuffer(2)->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY);
                }
                baseIndex = 0;

                indexData = NBRE_NEW NBRE_IndexData(renderPal.CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, indexCount, TRUE, NBRE_HardwareBuffer::HBU_STATIC), 0, indexCount);

                pIndex = (uint16*)indexData->IndexBuffer()->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY);
                indexOffset = 0;

                mesh = NBRE_NEW NBRE_Mesh(vertexData);
            }

            NBRE_SubModel& subModel = en->GetSubModel(j);
            NBRE_SubMesh* subMesh = subModel.GetSubMesh();

            if (i == 0 && j == 0)
            {
                shaderName = subModel.ShaderName();
                pType = subMesh->GetOperationType();
            }

            NBRE_VertexData* vd = subMesh->GetVertexData();
            NBRE_IndexData* id = subMesh->GetIndexData();
            uint16 vc = (uint16)vd->GetVertexCount();
            uint16 ic = (uint16)id->IndexCount();

            if((baseIndex + vc) > MaxVextexCount)
            {
                vertexData->GetBuffer(0)->UnLock();
                vertexData->GetBuffer(1)->UnLock();
                if (hasNormal)
                {
                    vertexData->GetBuffer(2)->UnLock();
                }
                indexData->IndexBuffer()->UnLock();
                indexData->SetIndexCount(indexOffset);
                indexCount -= indexOffset;

                mesh->CreateSubMesh(indexData, pType);
                mesh->CalculateBoundingBox();
                NBRE_ModelPtr model(NBRE_NEW NBRE_Model(NBRE_MeshPtr(mesh)));
                model->SetShaderName(shaderName);
                outModels.push_back(model);
                mesh = NULL;
                vertexCount -= baseIndex;
                continue;
            }

            NBRE_Vector3f* pP = (NBRE_Vector3f*)vd->GetBuffer(0)->Lock(0, NBRE_HardwareBuffer::HBL_READ_ONLY);
            NBRE_Vector2f* pT = (NBRE_Vector2f*)vd->GetBuffer(1)->Lock(0, NBRE_HardwareBuffer::HBL_READ_ONLY);
            NBRE_Vector3f* pN = NULL;
            if (hasNormal)
            {
                pN = (NBRE_Vector3f*)vd->GetBuffer(2)->Lock(0, NBRE_HardwareBuffer::HBL_READ_ONLY);
            }
            uint16* pI = (uint16*)id->IndexBuffer()->Lock(0, NBRE_HardwareBuffer::HBL_READ_ONLY);

            nsl_memcpy(pPosition, pP, sizeof(NBRE_Vector3f) * vc);
            nsl_memcpy(pTexcoord, pT, sizeof(NBRE_Vector2f) * vc);
            if (hasNormal)
            {
                nsl_memcpy(pNormal, pN, sizeof(NBRE_Vector3f) * vc);
            }
            nsl_memcpy(pIndex, pI, sizeof(uint16) * vc);

            for (uint16 k = 0; k < ic; ++k)
            {
                pIndex[k] = pI[k] + baseIndex;
            }

            vd->GetBuffer(0)->UnLock();
            vd->GetBuffer(1)->UnLock();
            if (hasNormal)
            {
                vd->GetBuffer(2)->UnLock();
            }
            id->IndexBuffer()->UnLock();

            pPosition += vc;
            pTexcoord += vc;
            if (hasNormal)
            {
                pNormal += vc;
            }
            pIndex += ic;
            baseIndex = baseIndex + vc;
            indexOffset = indexOffset + ic;

            ++j;
        }
    }

    if(mesh)
    {
        vertexData->GetBuffer(0)->UnLock();
        vertexData->GetBuffer(1)->UnLock();
        if (hasNormal)
        {
            vertexData->GetBuffer(2)->UnLock();
        }
        indexData->IndexBuffer()->UnLock();
        indexData->SetIndexCount(indexOffset);

        mesh->CreateSubMesh(indexData, pType);
        mesh->CalculateBoundingBox();
        NBRE_ModelPtr model(NBRE_NEW NBRE_Model(NBRE_MeshPtr(mesh)));
        model->SetShaderName(shaderName);
        outModels.push_back(model);
    }
}

PAL_Error
NBGM_NBMMESHLayerData::LoadLayerData(NBGM_ResourceContext& resourceContext, NBRE_Layer* layer, const NBRE_Point3d& /*refCenter*/, const NBGM_NBMDataLoadInfo& tileInfo)
{
    NBRE_MapMaterialManager& materialMgr = *(resourceContext.mapMaterialManager);
    for (uint32 i = 0; i < layer->dataCount; ++i)
    {
        NBRE_LayerMESHData* meshData = (NBRE_LayerMESHData*)layer->data[i];
        if(meshData == NULL)
        {
            continue;
        }

        if(meshData->models != NULL)
        {
            if (mLayerInfo.layerType == NBRE_DLT_UnTextureBuilding || mLayerInfo.layerType == NBRE_DLT_UnTextureLandmarkBuilding)
            {
                MergeModels(*(resourceContext.renderPal), meshData->models, mModels);
            }
            else
            {
                for(NBRE_Vector<NBRE_ModelPtr>::iterator iter = meshData->models->begin(); iter != meshData->models->end(); ++iter)
                {
                    mModels.push_back(*iter);
                }
            }
        }

        if (meshData->labelArray == NULL)
        {
            continue;
        }

        for (uint32 j = 0; j < meshData->labelCount; ++j)
        {
            NBRE_LayerPNTSData* pnts = meshData->labelArray[j];
            if (pnts == NULL || pnts->label == NULL || !VERIFY_NBM_INDEX(pnts->label->materialId))
            {
                continue;
            }
            NBGM_PointLabelData* pointLabelData = NBRE_NEW NBGM_PointLabelData();
            pointLabelData->Initialize(pnts, materialMgr.GetCurrentGroup(tileInfo.materialCategoryName), 0);
            mPointLabels.push_back(pointLabelData);
        }
    }
    return PAL_Ok;
}

NBGM_NBMTPSHLayerData::NBGM_NBMTPSHLayerData(const NBRE_LayerInfo& info)
    :NBGM_NBMLayerData(info)
{
}

NBGM_NBMTPSHLayerData::~NBGM_NBMTPSHLayerData()
{
    for(NBRE_Vector<NBGM_ShieldData*>::iterator iter = mShields.begin(); iter != mShields.end(); ++iter)
    {
        NBGM_ShieldData* shieldData = *iter;
        NBRE_DELETE shieldData;
    }
}

PAL_Error
NBGM_NBMTPSHLayerData::LoadLayerData(NBGM_ResourceContext& resourceContext, NBRE_Layer* layer, const NBRE_Point3d& /*refCenter*/, const NBGM_NBMDataLoadInfo& tileInfo)
{
    NBRE_MapMaterialManager& materialMgr = *(resourceContext.mapMaterialManager);
    for(uint32 i = 0; i < layer->dataCount; ++i)
    {
        NBRE_LayerTPSHData* shield = static_cast<NBRE_LayerTPSHData*>(layer->data[i]);
        if (shield == NULL || shield->label == NULL || !VERIFY_NBM_INDEX(shield->materialId))
        {
            continue;
        }
        NBGM_ShieldData* shieldData = NBRE_NEW NBGM_ShieldData();
        shieldData->Initialize(shield, materialMgr.GetCurrentGroup(tileInfo.materialCategoryName));
        mShields.push_back(shieldData);
    }
    return PAL_Ok;
}

NBGM_NBMTPARLayerData::NBGM_NBMTPARLayerData(const NBRE_LayerInfo& info)
    :NBGM_NBMLayerData(info)
{
}

NBGM_NBMTPARLayerData::~NBGM_NBMTPARLayerData()
{
    for(NBRE_Vector<NBGM_PathArrowData*>::iterator iter = mPathArrows.begin(); iter != mPathArrows.end(); ++iter)
    {
        NBGM_PathArrowData* pathArrow = *iter;
        NBRE_DELETE pathArrow;
    }
}

PAL_Error
NBGM_NBMTPARLayerData::LoadLayerData(NBGM_ResourceContext& resourceContext, NBRE_Layer* layer, const NBRE_Point3d& /*refCenter*/, const NBGM_NBMDataLoadInfo& tileInfo)
{
    if(layer == NULL)
    {
        return PAL_ErrBadParam;
    }
    NBRE_MapMaterialManager& materialMgr = *(resourceContext.mapMaterialManager);
    CreateMergedLayerData(layer, mPathArrows, materialMgr, tileInfo.materialCategoryName);
    BuildModel(resourceContext, mPathArrows, tileInfo.materialCategoryName);
    return PAL_Ok;
}

void
NBGM_NBMTPARLayerData::CreateMergedLayerData(const NBRE_Layer* layer, NBRE_Vector<NBGM_PathArrowData*>& pathArrowDataList, NBRE_MapMaterialManager& materialManager, const NBRE_String& materialCategoryName)
{
    NBRE_LayerTPARData* tparData = static_cast<NBRE_LayerTPARData*>(layer->data[0]);

    NBGM_PathArrowData* pathArrowData = NBRE_NEW NBGM_PathArrowData();
    pathArrowData->Initialize(tparData, materialManager.GetCurrentGroup(materialCategoryName));
    pathArrowData->track.clear();
    MergeTrackPolyline(pathArrowData->track, tparData->polyline);

    for (uint32 i = 1; i < layer->dataCount; ++i)
    {
        tparData = static_cast<NBRE_LayerTPARData*>(layer->data[i]);
        if (MergeTrackPolyline(pathArrowData->track, tparData->polyline))
        {
            continue;
        }
        else
        {
            pathArrowDataList.push_back(pathArrowData);
            pathArrowData = NBRE_NEW NBGM_PathArrowData();
            pathArrowData->Initialize(tparData, materialManager.GetCurrentGroup(materialCategoryName));
            pathArrowData->track.clear();
            MergeTrackPolyline(pathArrowData->track, tparData->polyline);
        }
    }

    pathArrowDataList.push_back(pathArrowData);
}

nb_boolean
NBGM_NBMTPARLayerData::MergeTrackPolyline(NBRE_Vector<NBRE_Point2f>& desArray, NBRE_Array* srcArray )
{
    uint32 count = NBRE_ArrayGetSize(srcArray);

    if (desArray.empty())
    {
        for (uint32 i = 0; i < count; ++i)
        {
            NBRE_Point2f* pos = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(srcArray, i));
            desArray.push_back(*pos);
        }
        return TRUE;
    }

    NBRE_Point2f& lastPos = desArray.back();
    NBRE_Point2f& startPos = *static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(srcArray, 0));

    NBRE_Point2f result = startPos - lastPos;

    float delta = result.Length();
    if (NBRE_Math::IsZero(delta, 1.0e-5f))
    {
        for (uint32 i = 0; i < count; ++i)
        {
            NBRE_Point2f* pos = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(srcArray, i));
            desArray.push_back(*pos);
        }
        return TRUE;
    }
    return FALSE;
}

PAL_Error
NBGM_NBMTPARLayerData::BuildModel(NBGM_ResourceContext& resourceContext, const NBRE_Vector<NBGM_PathArrowData*>& pathArrowDataList, const NBRE_String& materialCategoryName)
{
    NBGM_FloatBuffer& vertexBuffer = resourceContext.buildModelContext->mVetexBuffer;
    NBGM_FloatBuffer& texcoordBuffer = resourceContext.buildModelContext->mTexcoordBuffer1;
    NBGM_ShortBuffer& indexBuffer = resourceContext.buildModelContext->mIndexBuffer;

    vertexBuffer.clear();
    texcoordBuffer.clear();
    indexBuffer.clear();

    NBRE_String shaderName = BuildPathArrowVertex(resourceContext, pathArrowDataList,  MAP_PATH_ARROW_ZOOMLEVEL_RANGE[0], vertexBuffer, texcoordBuffer, indexBuffer, materialCategoryName);

    if (shaderName.empty() || vertexBuffer.size() == 0)
    {
        return PAL_ErrBadParam;
    }

    NBRE_MeshPtr mesh = BuildPathArrowMesh(resourceContext, vertexBuffer, texcoordBuffer, indexBuffer);
    NBRE_Model* model = NBRE_NEW NBRE_Model(mesh);
    shaderName = GenerateID(materialCategoryName, shaderName);
    model->SetShaderName(shaderName);
    mModel.reset(model);
    return PAL_Ok;
}

NBRE_String
NBGM_NBMTPARLayerData::BuildPathArrowVertex(NBGM_ResourceContext& resourceContext, const NBRE_Vector<NBGM_PathArrowData*>& pathArrowDataList, int8 /*zoomlevel*/, NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& texcoordBuffer, NBGM_ShortBuffer& indexBuffer, const NBRE_String& materialCategoryName)
{
    const NBRE_MapMateriaGroup& materiaGroup = resourceContext.mapMaterialManager->GetCurrentGroup(materialCategoryName);
    const NBRE_MapMaterial& material = materiaGroup.GetMapMaterial(MAKE_NBM_INDEX(pathArrowDataList[0]->materialId));

    if (material.Type() != NBRE_MMT_PathArrowMaterial)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM_NBMTPARLayerData::BuildPathArrowVertex. Mismatched material type!  type = %d, index = %d", material.Type(), pathArrowDataList[0]->materialId);
        return "";
    }

    NBRE_String shaderName = material.GetShaderName(0);
    const NBRE_MapPathArrowMaterial& paMat = static_cast<const NBRE_MapPathArrowMaterial&>(material);

    float length = resourceContext.WorldToModel(paMat.GetLength());
    float repeat = resourceContext.WorldToModel(paMat.GetRepeat());

    for (uint32 i = 0; i < pathArrowDataList.size(); ++i)
    {
        NBGM_BuildModelUtility::PathArrowPack2Tristripe(pathArrowDataList[i]->track, length, length, repeat, vertexBuffer, texcoordBuffer, indexBuffer);
    }

    return shaderName;
}

NBRE_MeshPtr
NBGM_NBMTPARLayerData::BuildPathArrowMesh(NBGM_ResourceContext& resourceContext, const NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& texcoordBuffer, const NBGM_ShortBuffer& indexBuffer)
{
    NBRE_IRenderPal* rp = resourceContext.renderPal;
    NBRE_VertexDeclaration* decalration = rp->CreateVertexDeclaration();


    NBRE_VertexData* vertexData = NBRE_NEW NBRE_VertexData(vertexBuffer.size()/2);
    NBRE_HardwareVertexBuffer* vertextBuf = rp->CreateVertexBuffer(sizeof(float) * 2, vertexBuffer.size()/2, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    vertextBuf->WriteData(0, vertexBuffer.size()*sizeof(float), &vertexBuffer.front(), TRUE);
    vertexData->AddBuffer(vertextBuf);

    NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem);

    NBRE_VertexElement* texElem1 = NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
    decalration->GetVertexElementList().push_back(texElem1);

    NBRE_HardwareVertexBuffer* texcoordBuf = rp->CreateVertexBuffer(sizeof(float) * 2, texcoordBuffer.size()/2, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    texcoordBuf->WriteData(0, texcoordBuffer.size()*sizeof(float), &texcoordBuffer.front(), TRUE);
    vertexData->AddBuffer(texcoordBuf);

    vertexData->SetVertexDeclaration(decalration);

    NBRE_HardwareIndexBuffer* indexBuf = rp->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, indexBuffer.size(), TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    indexBuf->WriteData(0, indexBuffer.size()*sizeof(uint16), &indexBuffer.front(), TRUE);
    NBRE_IndexData* indexData = NBRE_NEW NBRE_IndexData(indexBuf, 0, indexBuf->GetNumIndexes());

    NBRE_Mesh* mesh = NBRE_NEW NBRE_Mesh(vertexData);
    mesh->CreateSubMesh(indexData, NBRE_PMT_TRIANGLE_LIST);
    mesh->CalculateBoundingBox();

    return NBRE_MeshPtr(mesh);
}

NBGM_NBMPOISLayerData::NBGM_NBMPOISLayerData(const NBRE_LayerInfo& info)
    :NBGM_NBMLayerData(info)
{
}

NBGM_NBMPOISLayerData::~NBGM_NBMPOISLayerData()
{
    for (NBRE_Vector<NBGM_PoiData*>::iterator iter = mPois.begin(); iter != mPois.end(); ++iter)
    {
        NBRE_DELETE *iter;
    }
}

PAL_Error
NBGM_NBMPOISLayerData::LoadLayerData(NBGM_ResourceContext& resourceContext, NBRE_Layer* layer, const NBRE_Point3d& /*refCenter*/, const NBGM_NBMDataLoadInfo& tileInfo)
{
    for (uint32 i = 0; i < layer->dataCount; ++i)
    {
        NBRE_LayerPOISData* poiData = (NBRE_LayerPOISData*)layer->data[i];

        const NBRE_MapMateriaGroup& materialGroup = resourceContext.mapMaterialManager->GetCurrentGroup(tileInfo.materialCategoryName);

        if (poiData == NULL || poiData->point == NULL)
        {
            continue;
        }

        if (!VERIFY_NBM_INDEX(poiData->point->materialId) || !VERIFY_NBM_INDEX(poiData->selectedMaterialId))
        {
            NBRE_DebugLog(PAL_LogSeverityMajor,"NBGM_NBMPOISLayerData::LoadLayerData, invalid icon index(normal=0x%04x, selected=0x%04x)!",
                          poiData->point->materialId, poiData->selectedMaterialId);
            continue;
        }

        NBGM_PoiData* poi = NBRE_NEW NBGM_PoiData();
        poi->Initialize(poiData, materialGroup);
        mPois.push_back(poi);
    }
    return PAL_Ok;
}

NBGM_NBMGPINLayerData::NBGM_NBMGPINLayerData(const NBRE_LayerInfo& info)
    :NBGM_NBMLayerData(info)
{
}

NBGM_NBMGPINLayerData::~NBGM_NBMGPINLayerData()
{
    for (NBRE_Vector<NBGM_GpinData*>::iterator iter = mGpins.begin(); iter != mGpins.end(); ++iter)
    {
        NBRE_DELETE *iter;
    }
}

PAL_Error
NBGM_NBMGPINLayerData::LoadLayerData(NBGM_ResourceContext& resourceContext, NBRE_Layer* layer, const NBRE_Point3d& /*refCenter*/, const NBGM_NBMDataLoadInfo& tileInfo)
{
    for (uint32 i = 0; i < layer->dataCount; ++i)
    {
        NBRE_LayerGPINData* gpinData = (NBRE_LayerGPINData*)layer->data[i];

        const NBRE_MapMateriaGroup& materialGroup = resourceContext.mapMaterialManager->GetCurrentGroup(tileInfo.materialCategoryName);

        if (gpinData == NULL)
        {
            continue;
        }

        if (!VERIFY_NBM_INDEX(gpinData->materialId))
        {
            NBRE_DebugLog(PAL_LogSeverityMajor,"NBGM_NBMGPINLayerData::LoadLayerData, invalid material index = 0x%04x", gpinData->materialId);
            continue;
        }

        NBGM_GpinData* gpin = NBRE_NEW NBGM_GpinData();
        gpin->Initialize(gpinData, materialGroup);
        mGpins.push_back(gpin);
    }
    return PAL_Ok;
}
