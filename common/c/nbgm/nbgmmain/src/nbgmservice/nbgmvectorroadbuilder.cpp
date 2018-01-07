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
#include "nbgmvectortilebuilder.h"
#include "nbgmbuildmodelcontext.h"
#include "nbgmmapmaterial.h"
#include "nbreentity.h"
#include "nbrelog.h"
#include "nbgmmapmaterialmanager.h"
#include "nbrerenderengine.h"
#include "nbgmcommon.h"
#include "nbgmbuildutility.h"

static NBRE_SubModel*
CreateSubMeshWithMultiTex(NBGM_ResourceContext& resourceContext, NBRE_Model* model, NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& normalBuffer, NBGM_FloatBuffer& texcoordBuffer1, NBGM_FloatBuffer& texcoordBuffer2, NBGM_ShortBuffer& indexBuffer)
{
    NBRE_IRenderPal& rp = *(resourceContext.renderPal);
    NBRE_VertexDeclaration* decalration = rp.CreateVertexDeclaration();

    NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem);

    NBRE_VertexElement* texElem1 = NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
    decalration->GetVertexElementList().push_back(texElem1);

	NBRE_VertexElement* texElem2 = NBRE_NEW NBRE_VertexElement(2, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 1);
	decalration->GetVertexElementList().push_back(texElem2);

    NBRE_VertexElement* normalElem = NBRE_NEW NBRE_VertexElement(3, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_USER_DEFINE, 0);
    decalration->GetVertexElementList().push_back(normalElem);

    NBRE_VertexData* vertexData = NBRE_NEW NBRE_VertexData(vertexBuffer.size()/2);
    vertexData->SetVertexDeclaration(decalration);

    NBRE_HardwareVertexBuffer* vertextBuf = rp.CreateVertexBuffer(sizeof(float) * 2, vertexBuffer.size()/2, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    NBRE_HardwareVertexBuffer* texcoordBuf1 = rp.CreateVertexBuffer(sizeof(float) * 2, texcoordBuffer1.size()/2, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    NBRE_HardwareVertexBuffer* texcoordBuf2 = rp.CreateVertexBuffer(sizeof(float) * 2, texcoordBuffer2.size()/2, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    NBRE_HardwareVertexBuffer* normalBuf = rp.CreateVertexBuffer(sizeof(float) * 2, normalBuffer.size()/2, FALSE, NBRE_HardwareBuffer::HBU_STATIC);

    vertexData->AddBuffer(vertextBuf);
    vertexData->AddBuffer(texcoordBuf2);
    vertexData->AddBuffer(texcoordBuf1);
    vertexData->AddBuffer(normalBuf);

    vertextBuf->WriteData(0, vertexBuffer.size()*sizeof(float), &vertexBuffer.front(), FALSE);
    texcoordBuf1->WriteData(0, texcoordBuffer1.size()*sizeof(float), &texcoordBuffer1.front(), FALSE);
    texcoordBuf2->WriteData(0, texcoordBuffer2.size()*sizeof(float), &texcoordBuffer2.front(), FALSE);
    normalBuf->WriteData(0, normalBuffer.size()*sizeof(float), &normalBuffer.front(), FALSE);

    NBRE_HardwareIndexBuffer* indexBuf = rp.CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, indexBuffer.size(), TRUE, NBRE_HardwareBuffer::HBU_STATIC);

    indexBuf->WriteData(0, indexBuffer.size()*sizeof(uint16), &indexBuffer.front(), FALSE);
    NBRE_IndexData* indexData = NBRE_NEW NBRE_IndexData(indexBuf, 0, indexBuf->GetNumIndexes());
    return model->CreateSubModel(vertexData, indexData, NBRE_PMT_TRIANGLE_LIST);
}

static NBRE_SubModel*
CreateSubMesh(NBGM_ResourceContext& resourceContext, NBRE_Model* model, NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& normalBuffer, NBGM_FloatBuffer& texcoordBuffer, NBGM_ShortBuffer& indexBuffer)
{
    NBRE_IRenderPal& rp = *(resourceContext.renderPal);
    NBRE_VertexDeclaration* decalration = rp.CreateVertexDeclaration();

    NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem);

    NBRE_VertexElement* texElem1 = NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
    decalration->GetVertexElementList().push_back(texElem1);

    NBRE_VertexElement* normalElem = NBRE_NEW NBRE_VertexElement(2, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_USER_DEFINE, 0);
    decalration->GetVertexElementList().push_back(normalElem);

    NBRE_VertexData* vertexData = NBRE_NEW NBRE_VertexData(vertexBuffer.size()/2);
    vertexData->SetVertexDeclaration(decalration);

    NBRE_HardwareVertexBuffer* vertextBuf = rp.CreateVertexBuffer(sizeof(float) * 2, vertexBuffer.size()/2, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    NBRE_HardwareVertexBuffer* texcoordBuf = rp.CreateVertexBuffer(sizeof(float) * 2, texcoordBuffer.size()/2, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    NBRE_HardwareVertexBuffer* normalBuf = rp.CreateVertexBuffer(sizeof(float) * 2, normalBuffer.size()/2, TRUE, NBRE_HardwareBuffer::HBU_STATIC);

    vertexData->AddBuffer(vertextBuf);
    vertexData->AddBuffer(texcoordBuf);
    vertexData->AddBuffer(normalBuf);

    vertextBuf->WriteData(0, vertexBuffer.size()*sizeof(float), &vertexBuffer.front(), FALSE);
    texcoordBuf->WriteData(0, texcoordBuffer.size()*sizeof(float), &texcoordBuffer.front(), FALSE);
    normalBuf->WriteData(0, normalBuffer.size()*sizeof(float), &normalBuffer.front(), FALSE);

    NBRE_HardwareIndexBuffer* indexBuf = rp.CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, indexBuffer.size(), TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    indexBuf->WriteData(0, indexBuffer.size()*sizeof(uint16), &indexBuffer.front(), FALSE);
    NBRE_IndexData* indexData = NBRE_NEW NBRE_IndexData(indexBuf, 0, indexBuf->GetNumIndexes());

    return model->CreateSubModel(vertexData, indexData, NBRE_PMT_TRIANGLE_LIST);
}

static void
AssignSubModelShader(NBRE_SubModel* subModel, NBRE_MapMaterial* material, const NBRE_String& shaderName, nb_boolean internal, const NBRE_String& materialCategoryName)
{
    if(subModel == NULL)
    {
        return;
    }
    if(internal && material && material->GetShaderCount() > 0)
    {
        if(material->Type() == NBRE_MMT_StandardColorMaterial)
        {
            subModel->SetShader(material->GetShader(1));
        }
        else
        {
            subModel->SetShader(material->GetShader(0));
        }
    }
    else
    {
        subModel->SetShaderName(GenerateID(materialCategoryName, shaderName));
    }
}

typedef NBRE_Set<NBRE_SubModel*> AnimatedSubModelList;

static void
CreateSubModel(NBGM_ResourceContext& resourceContext, NBRE_Model* model, NBRE_MapMaterial* previousMaterial, NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& normalBuffer, NBGM_FloatBuffer& texcoordBuffer1, NBGM_FloatBuffer& texcoordBuffer2,
               NBGM_ShortBuffer& indexBuffer, AnimatedSubModelList& subModelList, const NBRE_String& shaderName, nb_boolean internal, const NBRE_String& materialCategoryName)
{
    if(vertexBuffer.size() == 0 || previousMaterial == NULL)
    {
        return;
    }

    NBRE_SubModel* subModel = NULL;

    switch(previousMaterial->Type())
    {
    case NBRE_MMT_OutlinedColorMaterial:
        {
            subModel = CreateSubMeshWithMultiTex(resourceContext, model, vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2, indexBuffer);
        }
        break;

    case NBRE_MMT_LinePatternMaterial:
    case NBRE_MMT_DashOutlineMaterial:
        {
            subModel = CreateSubMesh(resourceContext, model, vertexBuffer, normalBuffer, texcoordBuffer1, indexBuffer);
            subModelList.insert(subModel);
        }
        break;

    case NBRE_MMT_StandardColorMaterial:
        {
            subModel = CreateSubMeshWithMultiTex(resourceContext, model, vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2, indexBuffer);
        }
        break;

    case NBRE_MMT_PolylineCapMaterial:
        {
            subModel = CreateSubMeshWithMultiTex(resourceContext, model, vertexBuffer, normalBuffer, texcoordBuffer1, texcoordBuffer2, indexBuffer);
        }
        break;

    case NBRE_MMT_BitMapPatternMaterial:
        {
            subModel = CreateSubMesh(resourceContext, model, vertexBuffer, normalBuffer, texcoordBuffer1, indexBuffer);
            NBRE_BitMapPatternMaterial* bitMapPatternMat = static_cast<NBRE_BitMapPatternMaterial*>(previousMaterial);
            if(bitMapPatternMat->GetDistance() == 0)
            {
                subModelList.insert(subModel);
            }
        }
        break;

    default:
        return;
    }

    AssignSubModelShader(subModel, previousMaterial, shaderName, internal, materialCategoryName);
}

static void
CreateSubModel(NBGM_ResourceContext& rc,
               NBGM_BuildModelContext* buildModelContext,
               NBRE_Model* foregroundModel,
               NBRE_Model* backgroundModel,
               NBRE_MapMaterial* previousMaterial,
               AnimatedSubModelList& subModelList,
               const NBRE_String& shaderName,
               nb_boolean internal,
               const NBRE_String& materialCategoryName)
{
    NBGM_BuildModelContext* bc = rc.buildModelContext;

    if(buildModelContext)
    {
        bc = buildModelContext;
    }

    if(bc->mVetexBuffer.size() == 0 || previousMaterial == NULL)
    {
        return;
    }

    const uint16 MAX_INT16 = 65535;
    const uint16 VALID_VERTEX_COUNT = MAX_INT16 - 1;    // Because vertex is coupled.

    if(bc->mVetexBuffer.size() > VALID_VERTEX_COUNT)
    {
        // Add a crash intendionally to track Bug 191303 - Dashed traffic flow embedded into polyline appeared outside of polyline.
        // It is very difficult to reproduce this bug and there is also no evidence to point that the bug is caused by vertexBuffer.size() > 65534.
        // So just make a crash for QA to track this issue if it is caused by vertexBuffer.size() > 65534.
        NBRE_DebugLog(PAL_LogSeverityMajor, "CreateSubModel: mVetexBuffer.size() > 65534", bc->mVetexBuffer.size());
    }

    NBGM_FloatBuffer& vertexBuffer = bc->mVetexBuffer;
    NBGM_FloatBuffer& normalBuffer1 = bc->mNormalBuffer1;
    NBGM_FloatBuffer& normalBuffer2 = bc->mNormalBuffer2;
    NBGM_ShortBuffer& indexBuffer = bc->mIndexBuffer;
    NBGM_FloatBuffer& texcoordBuffer1 = bc->mTexcoordBuffer1;
    NBGM_FloatBuffer& texcoordBuffer2 = bc->mTexcoordBuffer2;
    NBGM_FloatBuffer& texcoordBuffer3 = bc->mTexcoordBuffer3;

    switch(previousMaterial->Type())
    {
    case NBRE_MMT_OutlinedColorMaterial:
        {
            CreateSubModel(rc, foregroundModel, previousMaterial, vertexBuffer, normalBuffer1, texcoordBuffer1, texcoordBuffer2, indexBuffer, subModelList, shaderName, internal, materialCategoryName);
            if(backgroundModel)
            {
                CreateSubModel(rc, backgroundModel, previousMaterial, vertexBuffer, normalBuffer2, texcoordBuffer1, texcoordBuffer3, indexBuffer, subModelList, shaderName, internal, materialCategoryName);
            }
        }
        break;

    case NBRE_MMT_LinePatternMaterial:
    case NBRE_MMT_DashOutlineMaterial:
    case NBRE_MMT_StandardColorMaterial:
    case NBRE_MMT_BitMapPatternMaterial:
        {
            CreateSubModel(rc, foregroundModel, previousMaterial, vertexBuffer, normalBuffer1, texcoordBuffer1, texcoordBuffer2, indexBuffer, subModelList, shaderName, internal, materialCategoryName);
        }
        break;

    case NBRE_MMT_PolylineCapMaterial:
        {
            CreateSubModel(rc, foregroundModel, previousMaterial, vertexBuffer, normalBuffer1, texcoordBuffer1,
                           texcoordBuffer2, indexBuffer, subModelList, shaderName, internal, materialCategoryName);
            CreateSubModel(rc, backgroundModel, previousMaterial, vertexBuffer, normalBuffer2, texcoordBuffer1,
                           texcoordBuffer3, indexBuffer, subModelList, shaderName, internal, materialCategoryName);
        }
        break;

    default:
        return;
    }

    bc->ClearAll();
}

typedef NBRE_Pair<const NBRE_MapMaterial*, nb_boolean>  MaterialInfo;

static MaterialInfo
GetMaterial(NBGM_ResourceContext& resourceContext,
            const NBRE_MapMaterial* material,
            uint16 materialId,
            const NBRE_Layer* roadLayer,
            const NBRE_String& materialCategoryName)
{
    if(material != NULL)
    {
        if (roadLayer->info.layerType == NBRE_DLT_Route
            || (material->GetShaderCount() != 0 && materialId == 0))
        {
            return MaterialInfo(material, TRUE);
        }
        return MaterialInfo(NULL, FALSE);
    }
    else
    {
        const NBRE_MapMateriaGroup& materiaGroup = resourceContext.mapMaterialManager->GetCurrentGroup(materialCategoryName);
        return MaterialInfo(&materiaGroup.GetMapMaterial(materialId), FALSE);
    }
}

static MaterialInfo
GetMaterial(NBGM_ResourceContext& resourceContext,
            const NBRE_LayerLPTHData* data,
            const NBRE_Layer* roadLayer,
            const NBRE_String& materialCategoryName)
{
    if(data == NULL)
    {
        return MaterialInfo(NULL, FALSE);
    }

    return GetMaterial(resourceContext,
                       data->material,
                       data->materialId,
                       roadLayer,
                       materialCategoryName);
}

static MaterialInfo
GetMaterial(NBGM_ResourceContext& resourceContext,
            const NBRE_LayerLPDRData* data,
            const NBRE_Layer* roadLayer,
            const NBRE_String& materialCategoryName)
{
    if(data == NULL)
    {
        return MaterialInfo(NULL, FALSE);
    }

    return GetMaterial(resourceContext,
                       data->material,
                       data->materialId,
                       roadLayer,
                       materialCategoryName);
}

static void
RecordTextureAnimatedSubModel(AnimatedSubModelList& subModelList, NBRE_Set<NBRE_SubModel*>& subModels)
{
    for(AnimatedSubModelList::iterator iter = subModelList.begin(); iter != subModelList.end(); ++iter)
    {
        subModels.insert(*iter);
    }
}

static void
FillDataToBuffer(const NBGM_FloatBuffer& src, NBGM_FloatBuffer& dst, uint32 index, float scale)
{
    uint32 size = src.size();
    for(uint32 j = index; j < size; ++j)
    {
        dst.push_back(src[j] * scale);
    }
}

static void
FillDataToBuffer(const NBGM_FloatBuffer& src, NBGM_FloatBuffer& dst, uint32 index, const NBRE_Point2f& texCoord)
{
    uint32 size = src.size();
    for(uint32 j = index; j < size; j += 2)
    {
        dst.push_back(texCoord.x);
        dst.push_back(texCoord.y);
    }
}

static PAL_Error
BuildPolylineCap(NBGM_ResourceContext& rc, NBGM_FloatBuffer& vertexBuffer,
                 NBGM_FloatBuffer& normalBuffer1, NBGM_FloatBuffer& normalBuffer2, NBGM_ShortBuffer& indexBuffer,
                 NBGM_FloatBuffer& texcoordBuffer1, NBGM_FloatBuffer& texcoordBuffer2, NBGM_FloatBuffer& texcoordBuffer3,
                 NBGM_PolylineCapType type, float arrowWidth, float arrowLength,
                 float radius, float polylineOutlineWidth, const NBRE_Point2f& lastPt, const NBRE_Point2f& preLastPt,
                 const NBRE_Point2f& outlineTexCoord, const NBRE_Point2f& interiorTexCoord)
{
    PAL_Error err = PAL_Ok;

    switch(type)
    {
    case NBGM_PCT_Arrow:
        {
            // build interior cap
            float halfHeadAngle = nsl_atan(arrowWidth * 0.5f / arrowLength);
            float interiorHeadToOutlineHead = polylineOutlineWidth / nsl_sin(halfHeadAngle);
            float interiorHeadLen = arrowLength - interiorHeadToOutlineHead - polylineOutlineWidth;
            float interiorWidth = 2.0f * nsl_tan(halfHeadAngle) * interiorHeadLen;

            uint32 textureIndex = texcoordBuffer2.size();
            err = NBGM_BuildModelUtility::BuildPolylineArrowCap(interiorWidth, interiorHeadLen, lastPt, preLastPt,
                                                                interiorTexCoord, vertexBuffer, normalBuffer1,
                                                                texcoordBuffer1, texcoordBuffer2, indexBuffer);
            if (err == PAL_Ok)
            {
                // build outlined cap
                float temp = (arrowWidth - interiorWidth) * 0.5f;
                float interiorLeftToOutlineLeft = nsl_sqrt(temp * temp + polylineOutlineWidth * polylineOutlineWidth);
                float heightScale = 1.0f + interiorHeadToOutlineHead / (interiorHeadLen * 2.0f / 3.0f);

                uint32 size = normalBuffer1.size();

                NBRE_Point2f leftNormal(normalBuffer1.at(size - 6), normalBuffer1.at(size - 5));
                NBRE_Point2f rightNormal(normalBuffer1.at(size - 4), normalBuffer1.at(size - 3));
                NBRE_Point2f headNormal(normalBuffer1.at(size - 2), normalBuffer1.at(size - 1));

                NBRE_Point2f leftPt = lastPt + leftNormal;
                NBRE_Point2f rightPt = lastPt + rightNormal;
                NBRE_Point2f headPt = lastPt + headNormal;

                NBRE_Point2f center = (leftPt + rightPt + headPt) / 3.0f;

                NBRE_Vector2f centerToLeft = leftPt - center;
                float widthScale = 1.0f + interiorLeftToOutlineLeft / centerToLeft.Length();

                float lLen = centerToLeft.Length() * widthScale;
                centerToLeft.Normalise();

                NBRE_Vector2f centerToRight = rightPt - center;
                float rLen = centerToRight.Length() * widthScale;
                centerToRight.Normalise();

                NBRE_Vector2f centerToHead = headPt - center;
                float hLen = centerToHead.Length() * heightScale;
                centerToHead.Normalise();

                leftPt = center + centerToLeft * lLen;
                rightPt = center + centerToRight * rLen;
                headPt = center + centerToHead * hLen;

                leftNormal = leftPt - lastPt;
                rightNormal = rightPt - lastPt;
                headNormal = headPt - lastPt;

                normalBuffer2.push_back(leftNormal.x);
                normalBuffer2.push_back(leftNormal.y);
                normalBuffer2.push_back(rightNormal.x);
                normalBuffer2.push_back(rightNormal.y);
                normalBuffer2.push_back(headNormal.x);
                normalBuffer2.push_back(headNormal.y);
                FillDataToBuffer(texcoordBuffer2, texcoordBuffer3, textureIndex, outlineTexCoord);
            }
        }
        break;
    case NBGM_PCT_Circle:
        {
            // build outlined cap
            uint32 normalIndex = normalBuffer2.size();
            uint32 textureIndex = texcoordBuffer3.size();

            err = NBGM_BuildModelUtility::BuildPolylineCircleCap(radius, lastPt, outlineTexCoord, vertexBuffer, normalBuffer2,
                                                                 texcoordBuffer1, texcoordBuffer3, indexBuffer);
            if (err == PAL_Ok)
            {
                // build interior cap
                float scale = 1.0f - polylineOutlineWidth / radius;
                FillDataToBuffer(normalBuffer2, normalBuffer1, normalIndex, scale);
                FillDataToBuffer(texcoordBuffer3, texcoordBuffer2, textureIndex, interiorTexCoord);
            }
        }
        break;
    default:
        break;
    }

    return err;
}

static PAL_Error
BuildRoadModel(NBGM_ResourceContext& resourceContext,
               NBRE_Layer* roadLayer,
               NBGM_DrawOrderModelMap* foregroundModels,
               NBGM_DrawOrderModelMap* backgroundModels,
               NBGM_DrawOrderSubModelMap& textureAnimatedSubModels,
               const NBRE_String& materialCategoryName)
{
    PAL_Error err = PAL_Ok;

    NBRE_Map<uint16, NBGM_BuildModelContext*> buildModelcontexts;
    NBRE_Map<uint16, NBRE_MapMaterial*> lastMats;
    NBRE_Map<uint16, nb_boolean> lastInternals;

    AnimatedSubModelList* subModelList = NULL;
    NBRE_MapMaterial** lastMat = NULL;
    nb_boolean* lastInternal = FALSE;
    NBRE_Model* foregroundModel = NULL;
    NBRE_Model* backgroundModel = NULL;
    NBRE_String* shaderName = NULL;

    NBRE_Map<uint16, NBRE_String> shaderNames;
    double patternRoadSegsTotalLen = 0.0;
    double dashRoadSegsTotalLen = 0.0;
    double bitmapPatternSegsTotalLen = 0.0;

    for (uint32 i = 0; i < roadLayer->dataCount; ++i)
    {
        NBRE_LayerLPDRData* roadData = static_cast<NBRE_LayerLPDRData*>(roadLayer->data[i]);
        if(roadData == NULL)
        {
            continue;
        }

        MaterialInfo info = GetMaterial(resourceContext, roadData, roadLayer, materialCategoryName);
        NBRE_MapMaterial* material = const_cast<NBRE_MapMaterial*>(info.first);
        if (material == NULL)
        {
            continue;
        }

        uint16 roadOrder = roadData->drawOrder;
        NBGM_BuildModelContext* bc = NULL;
        {
            NBRE_Map<uint16, NBGM_BuildModelContext*>::iterator it = buildModelcontexts.find(roadOrder);
            if (it == buildModelcontexts.end())
            {
                NBGM_BuildModelContext* ctx = NBRE_NEW NBGM_BuildModelContext;
                buildModelcontexts[roadOrder] = ctx;
                bc = ctx;
            }
            else
            {
                bc = it->second;
            }
        }
        NBGM_FloatBuffer& vertexBuffer = bc->mVetexBuffer;
        NBGM_FloatBuffer& normalBuffer1 = bc->mNormalBuffer1;
        NBGM_FloatBuffer& normalBuffer2 = bc->mNormalBuffer2;
        NBGM_ShortBuffer& indexBuffer = bc->mIndexBuffer;
        NBGM_FloatBuffer& texcoordBuffer1 = bc->mTexcoordBuffer1;
        NBGM_FloatBuffer& texcoordBuffer2 = bc->mTexcoordBuffer2;
        NBGM_FloatBuffer& texcoordBuffer3 = bc->mTexcoordBuffer3;

        if (foregroundModels)
        {
            NBRE_Map<uint16, NBRE_Model*>::iterator it = foregroundModels->find(roadOrder);
            if (it != foregroundModels->end())
            {
                foregroundModel = it->second;
            }
            else
            {
                foregroundModel = NBRE_NEW NBRE_Model(NBRE_MeshPtr(NBRE_NEW NBRE_Mesh()));
                (*foregroundModels)[roadOrder] = foregroundModel;
            }
        }

        if (backgroundModels)
        {
            NBRE_Map<uint16, NBRE_Model*>::iterator it = backgroundModels->find(roadOrder);
            if (it != backgroundModels->end())
            {
                backgroundModel = it->second;
            }
            else
            {
                backgroundModel = NBRE_NEW NBRE_Model(NBRE_MeshPtr(NBRE_NEW NBRE_Mesh()));
                (*backgroundModels)[roadOrder] = backgroundModel;
            }
        }

        {
            NBRE_Map<uint16, NBRE_Set<NBRE_SubModel*>*>::iterator it = textureAnimatedSubModels.find(roadOrder);
            if (it != textureAnimatedSubModels.end())
            {
                subModelList = it->second;
            }
            else
            {
                subModelList = NBRE_NEW NBRE_Set<NBRE_SubModel*>();
                textureAnimatedSubModels[roadOrder] = subModelList;
            }
        }

        NBRE_Map<uint16, NBRE_String>::iterator shaderIt = shaderNames.find(roadOrder);
        if (shaderIt != shaderNames.end())
        {
            shaderName = &(shaderIt->second);
        }
        else
        {
            NBRE_String name;
            shaderNames[roadOrder] = name;
            shaderIt = shaderNames.find(roadOrder);
            shaderName = &(shaderIt->second);
        }

        NBRE_Map<uint16, NBRE_MapMaterial*>::iterator lastMatIt = lastMats.find(roadOrder);
        if (lastMatIt != lastMats.end())
        {
            lastMat = &(lastMatIt->second);
        }
        else
        {
            NBRE_MapMaterial* mat = NULL;
            lastMats[roadOrder] = mat;
            lastMatIt = lastMats.find(roadOrder);
            lastMat = &(lastMatIt->second);
        }
        NBRE_Map<uint16, nb_boolean>::iterator itLastInternals = lastInternals.find(roadOrder);
        if (itLastInternals != lastInternals.end())
        {
            lastInternal = &(itLastInternals->second);
        }
        else
        {
            nb_boolean interval = FALSE;
            lastInternals[roadOrder] = interval;
            itLastInternals = lastInternals.find(roadOrder);
            lastInternal = &(itLastInternals->second);
        }

        switch(material->Type())
        {
        case NBRE_MMT_LinePatternMaterial:
            {
                NBRE_MapPatternLineMaterial* patternMaterial = static_cast<NBRE_MapPatternLineMaterial*>(material);
                if(*shaderName != patternMaterial->ShaderName())
                {
                    CreateSubModel(resourceContext, bc,
                                   foregroundModel, backgroundModel, *lastMat, *subModelList,
                                   *shaderName, info.second, materialCategoryName);
                }
                *shaderName = patternMaterial->ShaderName();

                err = NBGM_BuildModelUtility::PatternPolylinePack2Tristripe(resourceContext, roadData->polyline, roadData->width, 1.0f/8, vertexBuffer, normalBuffer1, texcoordBuffer1, indexBuffer, patternRoadSegsTotalLen);
                if (err != PAL_Ok)
                {
                    continue;
                }
                *lastMat = material;
                *lastInternal = info.second;
            }
            break;
        case NBRE_MMT_OutlinedColorMaterial:
            {
                NBRE_MapOutlinedColorsMaterial* outlineColorMaterial = static_cast<NBRE_MapOutlinedColorsMaterial*>(material);

                if(*shaderName != outlineColorMaterial->ShaderName())
                {
                    CreateSubModel(resourceContext, bc,
                                   foregroundModel, backgroundModel, *lastMat, *subModelList,
                                   *shaderName, info.second, materialCategoryName);
                }
                *shaderName = outlineColorMaterial->ShaderName();

                int32 normalIndex = normalBuffer2.size();
                int32 textureIndex = texcoordBuffer3.size();
                err = NBGM_BuildModelUtility::PolylinePack2TristripeWithRoundEndpoint(roadData->polyline, roadData->width, outlineColorMaterial->OutlineTexcoord().x, outlineColorMaterial->OutlineTexcoord().y,
                                                                                      vertexBuffer, normalBuffer2, texcoordBuffer1, texcoordBuffer3, indexBuffer);
                if (err != PAL_Ok)
                {
                    continue;
                }

                float scale = 1-outlineColorMaterial->OutlineWidth();
                FillDataToBuffer(normalBuffer2, normalBuffer1, normalIndex, scale);
                FillDataToBuffer(texcoordBuffer3, texcoordBuffer2, textureIndex, outlineColorMaterial->InteriorTexcoord());

                if ((roadData->roadFlag & 0x1) == 0)
                {
                    // make outline zero width
                    for(uint32 j = normalIndex; j < normalBuffer2.size(); j++)
                    {
                        normalBuffer2[j] = 0;
                    }
                }

                *shaderName = outlineColorMaterial->ShaderName();
                *lastMat = material;
                *lastInternal = info.second;
            }
            break;
        case NBRE_MMT_DashOutlineMaterial:
            {
                NBRE_MapDashLineMaterial* dashMaterial = static_cast<NBRE_MapDashLineMaterial*>(material);
                if(*shaderName != dashMaterial->ShaderName())
                {
                    CreateSubModel(resourceContext, bc,
                                   foregroundModel, backgroundModel, *lastMat, *subModelList,
                                   *shaderName, info.second, materialCategoryName);
                }
                *shaderName = dashMaterial->ShaderName();

                err = NBGM_BuildModelUtility::PatternPolylinePack2Tristripe(resourceContext, roadData->polyline, roadData->width, 1.0f/8, vertexBuffer,
                                                                            normalBuffer1, texcoordBuffer1, indexBuffer, dashRoadSegsTotalLen);
                if (err != PAL_Ok)
                {
                    continue;
                }
                *lastMat = material;
                *lastInternal = info.second;
            }
            break;
        case NBRE_MMT_StandardColorMaterial:
            {
                NBRE_MapStandardColorMaterial* colorMaterial = static_cast<NBRE_MapStandardColorMaterial*>(material);

                if(*shaderName != colorMaterial->GetShaderName(1))
                {
                    CreateSubModel(resourceContext, bc,
                                   foregroundModel, backgroundModel, *lastMat, *subModelList,
                                   *shaderName, info.second, materialCategoryName);
                }
                *shaderName = colorMaterial->GetShaderName(1);

                err = NBGM_BuildModelUtility::PolylinePack2TristripeWithRoundEndpoint(roadData->polyline, roadData->width, 0.5f, 0.5f,
                                                                                      vertexBuffer, normalBuffer1, texcoordBuffer1, texcoordBuffer2, indexBuffer);
                if (err != PAL_Ok)
                {
                    continue;
                }
                *lastMat = material;
                *lastInternal = info.second;
            }
            break;
        case NBRE_MMT_StandardTexturedMaterial:
            {
                *lastMat = material;
                *lastInternal = info.second;
            }
            break;
        case NBRE_MMT_PolylineCapMaterial:
            {
                NBRE_PolylineCapMaterial* mat = static_cast<NBRE_PolylineCapMaterial*>(material);

                if(*shaderName != mat->ShaderName())
                {
                    CreateSubModel(resourceContext, bc,
                        foregroundModel, backgroundModel, *lastMat, *subModelList,
                        *shaderName, info.second, materialCategoryName);
                }
                *shaderName = mat->ShaderName();

                uint32 normalIndex = normalBuffer2.size();
                uint32 textureIndex = texcoordBuffer3.size();

                // build body
                // outlined body
                // Now, when the type of cap is LineJoint, we still consider it as circle cap.
                if(mat->GetStartCap() == NBGM_PCT_LineJoin)
                {
                    err = NBGM_BuildModelUtility::PolylinePack2TristripeWithRoundEndpoint(roadData->polyline,
                                                                                          roadData->width,
                                                                                          mat->OutlineTexcoord().x,
                                                                                          mat->OutlineTexcoord().y,
                                                                                          vertexBuffer,
                                                                                          normalBuffer2,
                                                                                          texcoordBuffer1,
                                                                                          texcoordBuffer3,
                                                                                          indexBuffer);
                }
                else
                {

                    err = NBGM_BuildModelUtility::PolylinePack2TristripeWithFlatEndpoint(roadData->polyline,roadData->width,
                                                                                        mat->OutlineTexcoord().x, mat->OutlineTexcoord().y,
                                                                                        vertexBuffer, normalBuffer2, texcoordBuffer1,
                                                                                        texcoordBuffer3, indexBuffer);
                }

                if (err != PAL_Ok)
                {
                    continue;
                }

                // interior body
                float scale = 1.0f - mat->GetOutLineWidthPercentage();
                FillDataToBuffer(normalBuffer2, normalBuffer1, normalIndex, scale);
                FillDataToBuffer(texcoordBuffer3, texcoordBuffer2, textureIndex, mat->InteriorTexcoord());

                // build cap
                float polylineOutlineWidth = mat->GetOutLineWidthPercentage() * roadData->width * 0.5f;
                float radius = roadData->width * mat->GetRadiusPercentage();
                float arrowWidth = roadData->width * mat->GetShapeWidthPercentage();
                float arrowLength = roadData->width * mat->GetShapeHeightPercentage();

                // build start cap
                NBRE_Point2f* beginPt = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(roadData->polyline, 0));
                NBRE_Point2f* nextBeginPt = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(roadData->polyline, 1));

                err = BuildPolylineCap(resourceContext, vertexBuffer, normalBuffer1, normalBuffer2, indexBuffer,
                                       texcoordBuffer1, texcoordBuffer2, texcoordBuffer3,
                                       mat->GetStartCap(), arrowWidth, arrowLength, radius,
                                       polylineOutlineWidth, *beginPt, *nextBeginPt, mat->OutlineTexcoord(), mat->InteriorTexcoord());
                if (err != PAL_Ok)
                {
                    continue;
                }

                // build end cap
                uint32 count = NBRE_ArrayGetSize(roadData->polyline);
                NBRE_Point2f* lastPt = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(roadData->polyline, count - 1));
                NBRE_Point2f* prevLastPt = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(roadData->polyline, count - 2));

                err = BuildPolylineCap(resourceContext, vertexBuffer, normalBuffer1, normalBuffer2, indexBuffer,
                                       texcoordBuffer1, texcoordBuffer2, texcoordBuffer3,
                                       mat->GetEndCap(), arrowWidth, arrowLength, radius,
                                       polylineOutlineWidth, *lastPt, *prevLastPt, mat->OutlineTexcoord(), mat->InteriorTexcoord());

                if (err != PAL_Ok)
                {
                    continue;
                }

                *shaderName = mat->ShaderName();
                *lastMat = material;
                *lastInternal = info.second;
            }
            break;
        case NBRE_MMT_BitMapPatternMaterial:
            {
                NBRE_BitMapPatternMaterial* bitMapPatternMat = static_cast<NBRE_BitMapPatternMaterial*>(material);
                if(*shaderName != bitMapPatternMat->ShaderName())
                {
                    CreateSubModel(resourceContext, bc,
                        foregroundModel, backgroundModel, *lastMat, *subModelList,
                        *shaderName, info.second, materialCategoryName);
                }
                *shaderName = bitMapPatternMat->ShaderName();

                uint16 width = bitMapPatternMat->GetWidth();
                uint16 height = bitMapPatternMat->GetHeight();
                uint16 distance = bitMapPatternMat->GetDistance();
                nb_boolean distanceIsInvalid = FALSE;
                if(distance == 0)
                {
                    distanceIsInvalid = TRUE;
                }

                float mercatorHeight =  static_cast<float>(height) / static_cast<float>(width) * roadData->width;
                float mercatorDistance = static_cast<float>(distance) / static_cast<float>(width) * roadData->width + mercatorHeight;

                err = NBGM_BuildModelUtility::BitmapPatternPolyline2Tristripe(resourceContext,
                                                                              roadData->polyline,
                                                                              roadData->width,
                                                                              mercatorHeight,
                                                                              mercatorDistance,
                                                                              static_cast<float>(bitMapPatternMat->GetHeight())/bitMapPatternMat->GetWidth(),
                                                                              distanceIsInvalid,
                                                                              vertexBuffer,
                                                                              normalBuffer1,
                                                                              texcoordBuffer1,
                                                                              indexBuffer,
                                                                              bitmapPatternSegsTotalLen);
                if (err != PAL_Ok)
                {
                    continue;
                }
                *lastMat = material;
                *lastInternal = info.second;
            }
            break;
        default:
            {
                NBRE_DebugLog(PAL_LogSeverityMajor, "BuildRoadModel: unknown material->maetrialType is %d", material->Type());
                continue;
            }
        }
    }

    for (NBRE_Map<uint16, NBGM_BuildModelContext*>::iterator it = buildModelcontexts.begin(); it != buildModelcontexts.end(); ++it)
    {
        uint16 roadOrder = it->first;
        NBGM_BuildModelContext* bc = it->second;

        if (bc->mVetexBuffer.size() == 0)
        {
            continue;
        }

        if (foregroundModels)
        {
            NBRE_Map<uint16, NBRE_Model*>::iterator it = foregroundModels->find(roadOrder);
            if (it != foregroundModels->end())
            {
                foregroundModel = it->second;
            }
            else
            {
                continue;
            }
        }

        if (backgroundModels)
        {
            NBRE_Map<uint16, NBRE_Model*>::iterator it = backgroundModels->find(roadOrder);
            if (it != backgroundModels->end())
            {
                backgroundModel = it->second;
            }
            else
            {
                continue;
            }
        }

        {
            NBRE_Map<uint16, NBRE_Set<NBRE_SubModel*>*>::iterator it = textureAnimatedSubModels.find(roadOrder);
            if (it != textureAnimatedSubModels.end())
            {
                subModelList = it->second;
            }
            else
            {
                continue;
            }
        }

        NBRE_Map<uint16, NBRE_String>::iterator shaderIt = shaderNames.find(roadOrder);
        if (shaderIt != shaderNames.end())
        {
            shaderName = &(shaderIt->second);
        }
        else
        {
            continue;
        }

        NBRE_Map<uint16, NBRE_MapMaterial*>::iterator lastMatIt = lastMats.find(roadOrder);
        if (lastMatIt != lastMats.end())
        {
            lastMat = &(lastMatIt->second);
        }
        else
        {
            continue;
        }
        NBRE_Map<uint16, nb_boolean>::iterator itLastInternals = lastInternals.find(roadOrder);
        if (itLastInternals != lastInternals.end())
        {
            lastInternal = &(itLastInternals->second);
        }
        else
        {
            continue;
        }

        CreateSubModel(resourceContext, bc,
            foregroundModel, backgroundModel, *lastMat, *subModelList,
            *shaderName, *lastInternal, materialCategoryName);

        NBRE_DELETE it->second;
    }

    for (NBRE_Map<uint16, NBRE_Model*>::iterator it = foregroundModels->begin(); it != foregroundModels->end(); ++it)
    {
        NBRE_Model* model = it->second;
        if (model)
        {
            model->GetMesh()->CalculateBoundingBox();
        }
    }

    if (backgroundModels)
    {
        for (NBRE_Map<uint16, NBRE_Model*>::iterator it = backgroundModels->begin(); it != backgroundModels->end(); ++it)
        {
            NBRE_Model* model = it->second;
            if (model)
            {
                model->GetMesh()->CalculateBoundingBox();
            }
        }
    }

    return err;
}

static PAL_Error
BuildRoadModel(NBGM_ResourceContext& resourceContext,
               NBRE_Layer* roadLayer,
               NBRE_Model* foregroundModel,
               NBRE_Model* backgroundModel,
               NBRE_Set<NBRE_SubModel*>& textureAnimatedSubModels,
               const NBRE_String& materialCategoryName)
{
    PAL_Error err = PAL_Ok;

    NBGM_FloatBuffer& vertexBuffer = resourceContext.buildModelContext->mVetexBuffer;
    NBGM_FloatBuffer& normalBuffer1 = resourceContext.buildModelContext->mNormalBuffer1;
    NBGM_FloatBuffer& normalBuffer2 = resourceContext.buildModelContext->mNormalBuffer2;
    NBGM_ShortBuffer& indexBuffer = resourceContext.buildModelContext->mIndexBuffer;
    NBGM_FloatBuffer& texcoordBuffer1 = resourceContext.buildModelContext->mTexcoordBuffer1;
    NBGM_FloatBuffer& texcoordBuffer2 = resourceContext.buildModelContext->mTexcoordBuffer2;
    NBGM_FloatBuffer& texcoordBuffer3 = resourceContext.buildModelContext->mTexcoordBuffer3;

    resourceContext.buildModelContext->ClearAll();

    foregroundModel->SetMesh(NBRE_MeshPtr(NBRE_NEW NBRE_Mesh()));
    if(backgroundModel)
    {
        backgroundModel->SetMesh(NBRE_MeshPtr(NBRE_NEW NBRE_Mesh()));
    }

    AnimatedSubModelList subModelList;
    NBRE_MapMaterial* lastMat = NULL;
    nb_boolean lastInternal = FALSE;

    NBRE_String shaderName;
    double patternRoadSegsTotalLen = 0.0;
    double dashRoadSegsTotalLen = 0.0;
    double bitmapPatternSegsTotalLen = 0.0;

    for (uint32 i = 0; i < roadLayer->dataCount; ++i)
    {
        NBRE_LayerLPTHData* roadData = static_cast<NBRE_LayerLPTHData*>(roadLayer->data[i]);
        if(roadData == NULL)
        {
            continue;
        }

        MaterialInfo info = GetMaterial(resourceContext, roadData, roadLayer, materialCategoryName);
        NBRE_MapMaterial* material = const_cast<NBRE_MapMaterial*>(info.first);
        if (material == NULL)
        {
            continue;
        }

        switch(material->Type())
        {
        case NBRE_MMT_LinePatternMaterial:
            {
                NBRE_MapPatternLineMaterial* patternMaterial = static_cast<NBRE_MapPatternLineMaterial*>(material);
                if(shaderName != patternMaterial->ShaderName())
                {
                    CreateSubModel(resourceContext, NULL, foregroundModel, backgroundModel, lastMat, subModelList,
                                   shaderName, info.second, materialCategoryName);
                }
                shaderName = patternMaterial->ShaderName();

                err = NBGM_BuildModelUtility::PatternPolylinePack2Tristripe(resourceContext, roadData->polyline, roadData->width, 1.0f/8, vertexBuffer, normalBuffer1, texcoordBuffer1, indexBuffer, patternRoadSegsTotalLen);
                if (err != PAL_Ok)
                {
                    continue;
                }
                lastMat = material;
                lastInternal = info.second;
            }
            break;
        case NBRE_MMT_OutlinedColorMaterial:
            {
                NBRE_MapOutlinedColorsMaterial* outlineColorMaterial = static_cast<NBRE_MapOutlinedColorsMaterial*>(material);

                if(shaderName != outlineColorMaterial->ShaderName())
                {
                    CreateSubModel(resourceContext, NULL, foregroundModel, backgroundModel, lastMat, subModelList,
                                   shaderName, info.second, materialCategoryName);
                }
                shaderName = outlineColorMaterial->ShaderName();

                int32 normalIndex = normalBuffer2.size();
                int32 textureIndex = texcoordBuffer3.size();
                err = NBGM_BuildModelUtility::PolylinePack2TristripeWithRoundEndpoint(roadData->polyline, roadData->width, outlineColorMaterial->OutlineTexcoord().x, outlineColorMaterial->OutlineTexcoord().y,
                                                                                      vertexBuffer, normalBuffer2, texcoordBuffer1, texcoordBuffer3, indexBuffer);
                if (err != PAL_Ok)
                {
                    continue;
                }

                float scale = 1-outlineColorMaterial->OutlineWidth();
                FillDataToBuffer(normalBuffer2, normalBuffer1, normalIndex, scale);
                FillDataToBuffer(texcoordBuffer3, texcoordBuffer2, textureIndex, outlineColorMaterial->InteriorTexcoord());

                shaderName = outlineColorMaterial->ShaderName();
                lastMat = material;
                lastInternal = info.second;
            }
            break;
        case NBRE_MMT_DashOutlineMaterial:
            {
                NBRE_MapDashLineMaterial* dashMaterial = static_cast<NBRE_MapDashLineMaterial*>(material);
                if(shaderName != dashMaterial->ShaderName())
                {
                    CreateSubModel(resourceContext, NULL, foregroundModel, backgroundModel, lastMat, subModelList,
                                   shaderName, info.second, materialCategoryName);
                }
                shaderName = dashMaterial->ShaderName();

                err = NBGM_BuildModelUtility::PatternPolylinePack2Tristripe(resourceContext, roadData->polyline, roadData->width, 1.0f/8, vertexBuffer,
                                                                            normalBuffer1, texcoordBuffer1, indexBuffer, dashRoadSegsTotalLen);
                if (err != PAL_Ok)
                {
                    continue;
                }
                lastMat = material;
                lastInternal = info.second;
            }
            break;
        case NBRE_MMT_StandardColorMaterial:
            {
                NBRE_MapStandardColorMaterial* colorMaterial = static_cast<NBRE_MapStandardColorMaterial*>(material);

                if(shaderName != colorMaterial->GetShaderName(1))
                {
                    CreateSubModel(resourceContext, NULL, foregroundModel, backgroundModel, lastMat, subModelList,
                                   shaderName, info.second, materialCategoryName);
                }
                shaderName = colorMaterial->GetShaderName(1);

                err = NBGM_BuildModelUtility::PolylinePack2TristripeWithRoundEndpoint(roadData->polyline, roadData->width, 0.5f, 0.5f,
                                                                                      vertexBuffer, normalBuffer1, texcoordBuffer1, texcoordBuffer2, indexBuffer);
                if (err != PAL_Ok)
                {
                    continue;
                }
                lastMat = material;
                lastInternal = info.second;
            }
            break;
        case NBRE_MMT_StandardTexturedMaterial:
            {
                lastMat = material;
                lastInternal = info.second;
            }
            break;
        case NBRE_MMT_PolylineCapMaterial:
            {
                NBRE_PolylineCapMaterial* mat = static_cast<NBRE_PolylineCapMaterial*>(material);

                if(shaderName != mat->ShaderName())
                {
                    CreateSubModel(resourceContext, NULL, foregroundModel, backgroundModel, lastMat,
                                   subModelList, shaderName, info.second, materialCategoryName);
                }
                shaderName = mat->ShaderName();

                uint32 normalIndex = normalBuffer2.size();
                uint32 textureIndex = texcoordBuffer3.size();

                float polylineOutlineWidth = mat->GetOutLineWidthPercentage() * roadData->width * 0.5f;
                float radius = roadData->width * mat->GetRadiusPercentage();
                float arrowWidth = roadData->width * mat->GetShapeWidthPercentage();
                float arrowLength = roadData->width * mat->GetShapeHeightPercentage();
                float scale = 1.0f - mat->GetOutLineWidthPercentage();
                uint32 count = NBRE_ArrayGetSize(roadData->polyline);

                // build body
                // outlined body

                // If cap is radius and radius is equel to half of road width,
                // build cap and body at the same time.
                if((mat->GetStartCap() == NBGM_PCT_Circle)
                    && (mat->GetEndCap() == NBGM_PCT_Circle)
                    && (mat->GetRadiusPercentage() == 0.5f))
                {
                    err = NBGM_BuildModelUtility::PolylinePack2TristripeWithRoundEndpoint(roadData->polyline,
                                                                                          roadData->width,
                                                                                          mat->OutlineTexcoord().x,
                                                                                          mat->OutlineTexcoord().y,
                                                                                          vertexBuffer,
                                                                                          normalBuffer2,
                                                                                          texcoordBuffer1,
                                                                                          texcoordBuffer3,
                                                                                          indexBuffer);
                    // interior body
                    FillDataToBuffer(normalBuffer2, normalBuffer1, normalIndex, scale);
                    FillDataToBuffer(texcoordBuffer3, texcoordBuffer2, textureIndex, mat->InteriorTexcoord());

                }
                else if((mat->GetStartCap() == NBGM_PCT_Circle)
                        && (mat->GetRadiusPercentage() == 0.5f))
                {
                    err = NBGM_BuildModelUtility::PolylinePack2TristripeWithRoundStartCapFlatEndCap(roadData->polyline,
                                                                                                    roadData->width,
                                                                                                    mat->OutlineTexcoord().x,
                                                                                                    mat->OutlineTexcoord().y,
                                                                                                    vertexBuffer,
                                                                                                    normalBuffer2,
                                                                                                    texcoordBuffer1,
                                                                                                    texcoordBuffer3,
                                                                                                    indexBuffer);

                    if (err != PAL_Ok)
                    {
                        continue;
                    }

                    FillDataToBuffer(normalBuffer2, normalBuffer1, normalIndex, scale);
                    FillDataToBuffer(texcoordBuffer3, texcoordBuffer2, textureIndex, mat->InteriorTexcoord());

                    // build end cap
                    NBRE_Point2f* lastPt = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(roadData->polyline, count - 1));
                    NBRE_Point2f* prevLastPt = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(roadData->polyline, count - 2));

                    err = BuildPolylineCap(resourceContext,
                                           vertexBuffer,
                                           normalBuffer1,
                                           normalBuffer2,
                                           indexBuffer,
                                           texcoordBuffer1,
                                           texcoordBuffer2,
                                           texcoordBuffer3,
                                           mat->GetEndCap(),
                                           arrowWidth,
                                           arrowLength,
                                           radius,
                                           polylineOutlineWidth,
                                           *lastPt,
                                           *prevLastPt,
                                           mat->OutlineTexcoord(),
                                           mat->InteriorTexcoord());
                }
                else if((mat->GetEndCap() == NBGM_PCT_Circle)
                        && (mat->GetRadiusPercentage() == 0.5f))
                {
                    err = NBGM_BuildModelUtility::PolylinePack2TristripeWithFlatStartCapRoundEndCap(roadData->polyline,
                                                                                                    roadData->width,
                                                                                                    mat->OutlineTexcoord().x,
                                                                                                    mat->OutlineTexcoord().y,
                                                                                                    vertexBuffer,
                                                                                                    normalBuffer2,
                                                                                                    texcoordBuffer1,
                                                                                                    texcoordBuffer3,
                                                                                                    indexBuffer);
                    if (err != PAL_Ok)
                    {
                        continue;
                    }

                    FillDataToBuffer(normalBuffer2, normalBuffer1, normalIndex, scale);
                    FillDataToBuffer(texcoordBuffer3, texcoordBuffer2, textureIndex, mat->InteriorTexcoord());

                    // build start cap
                    NBRE_Point2f* beginPt = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(roadData->polyline, 0));
                    NBRE_Point2f* nextBeginPt = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(roadData->polyline, 1));

                    err = BuildPolylineCap(resourceContext,
                                           vertexBuffer,
                                           normalBuffer1,
                                           normalBuffer2,
                                           indexBuffer,
                                           texcoordBuffer1,
                                           texcoordBuffer2,
                                           texcoordBuffer3,
                                           mat->GetStartCap(),
                                           arrowWidth,
                                           arrowLength,
                                           radius,
                                           polylineOutlineWidth,
                                           *beginPt,
                                           *nextBeginPt,
                                           mat->OutlineTexcoord(),
                                           mat->InteriorTexcoord());
                }
                else
                {
                    err = NBGM_BuildModelUtility::PolylinePack2TristripeWithFlatEndpoint(roadData->polyline,
                                                                                         roadData->width,
                                                                                         mat->OutlineTexcoord().x,
                                                                                         mat->OutlineTexcoord().y,
                                                                                         vertexBuffer,
                                                                                         normalBuffer2,
                                                                                         texcoordBuffer1,
                                                                                         texcoordBuffer3,
                                                                                         indexBuffer);
                    if (err != PAL_Ok)
                    {
                        continue;
                    }

                    FillDataToBuffer(normalBuffer2, normalBuffer1, normalIndex, scale);
                    FillDataToBuffer(texcoordBuffer3, texcoordBuffer2, textureIndex, mat->InteriorTexcoord());

                    // build start cap
                    NBRE_Point2f* beginPt = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(roadData->polyline, 0));
                    NBRE_Point2f* nextBeginPt = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(roadData->polyline, 1));

                    err = BuildPolylineCap(resourceContext,
                                           vertexBuffer,
                                           normalBuffer1,
                                           normalBuffer2,
                                           indexBuffer,
                                           texcoordBuffer1,
                                           texcoordBuffer2,
                                           texcoordBuffer3,
                                           mat->GetStartCap(),
                                           arrowWidth,
                                           arrowLength,
                                           radius,
                                           polylineOutlineWidth,
                                           *beginPt,
                                           *nextBeginPt,
                                           mat->OutlineTexcoord(),
                                          mat->InteriorTexcoord());
                    if (err != PAL_Ok)
                    {
                        continue;
                    }

                    // build end cap
                    NBRE_Point2f* lastPt = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(roadData->polyline, count - 1));
                    NBRE_Point2f* prevLastPt = static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(roadData->polyline, count - 2));

                    err = BuildPolylineCap(resourceContext,
                                           vertexBuffer,
                                           normalBuffer1,
                                           normalBuffer2,
                                           indexBuffer,
                                           texcoordBuffer1,
                                           texcoordBuffer2,
                                           texcoordBuffer3,
                                           mat->GetEndCap(),
                                           arrowWidth,
                                           arrowLength,
                                           radius,
                                           polylineOutlineWidth,
                                           *lastPt,
                                           *prevLastPt,
                                           mat->OutlineTexcoord(),
                                           mat->InteriorTexcoord());
                }

                if (err != PAL_Ok)
                {
                    continue;
                }

                shaderName = mat->ShaderName();
                lastMat = material;
                lastInternal = info.second;
            }
            break;
        case NBRE_MMT_BitMapPatternMaterial:
            {
                NBRE_BitMapPatternMaterial* bitMapPatternMat = static_cast<NBRE_BitMapPatternMaterial*>(material);
                if(shaderName != bitMapPatternMat->ShaderName())
                {
                    CreateSubModel(resourceContext, NULL, foregroundModel, backgroundModel, lastMat, subModelList,
                                   shaderName, info.second, materialCategoryName);
                }
                shaderName = bitMapPatternMat->ShaderName();

                uint16 width = bitMapPatternMat->GetWidth();
                uint16 height = bitMapPatternMat->GetHeight();
                uint16 distance = bitMapPatternMat->GetDistance();
                nb_boolean distanceIsInvalid = FALSE;
                if(distance == 0)
                {
                    distanceIsInvalid = TRUE;
                }

                float mercatorHeight =  static_cast<float>(height) / static_cast<float>(width) * roadData->width;
                float mercatorDistance = static_cast<float>(distance) / static_cast<float>(width) * roadData->width + mercatorHeight;

                err = NBGM_BuildModelUtility::BitmapPatternPolyline2Tristripe(resourceContext,
                                                                              roadData->polyline,
                                                                              roadData->width,
                                                                              mercatorHeight,
                                                                              mercatorDistance,
                                                                              static_cast<float>(bitMapPatternMat->GetHeight())/bitMapPatternMat->GetWidth(),
                                                                              distanceIsInvalid,
                                                                              vertexBuffer,
                                                                              normalBuffer1,
                                                                              texcoordBuffer1,
                                                                              indexBuffer,
                                                                              bitmapPatternSegsTotalLen);
                if (err != PAL_Ok)
                {
                    continue;
                }
                lastMat = material;
                lastInternal = info.second;
            }
            break;
        default:
            {
                NBRE_DebugLog(PAL_LogSeverityMajor, "BuildRoadModel: unknown material->maetrialType is %d", material->Type());
                continue;
            }
        }
    }
    CreateSubModel(resourceContext, NULL, foregroundModel, backgroundModel, lastMat, subModelList, shaderName, lastInternal, materialCategoryName);

    foregroundModel->GetMesh()->CalculateBoundingBox();
    if(backgroundModel)
    {
        backgroundModel->GetMesh()->CalculateBoundingBox();
    }

    RecordTextureAnimatedSubModel(subModelList, textureAnimatedSubModels);

    return err;
}

PAL_Error
NBGM_VectorTileBuilder::CreateVectorRoadModel(NBGM_ResourceContext& resourceContext,
                                              NBRE_Layer* dataTile,
                                              NBGM_DrawOrderModelMap* foreground,
                                              NBGM_DrawOrderModelMap* background,
                                              NBGM_DrawOrderSubModelMap& textureAnimatedSubModels,
                                              const NBRE_String& materialCategoryName)
{
    textureAnimatedSubModels.clear();
    PAL_Error err = BuildRoadModel(resourceContext,
                                   dataTile,
                                   foreground,
                                   background,
                                   textureAnimatedSubModels,
                                   materialCategoryName);
    if(err != PAL_Ok)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM_VectorTileBuilder::CreateVectorRoadModel has error: 0x%08x", err);
    }
    return PAL_Ok;
}

PAL_Error
NBGM_VectorTileBuilder::CreateVectorRoadModel(NBGM_ResourceContext& resourceContext,
                                              NBRE_Layer* dataTile,
                                              NBRE_Model* foreground,
                                              NBRE_Model* background,
                                              NBRE_Set<NBRE_SubModel*>& textureAnimatedSubModels,
                                              const NBRE_String& materialCategoryName)
{
    textureAnimatedSubModels.clear();
    PAL_Error err = BuildRoadModel(resourceContext,
                                   dataTile,
                                   foreground,
                                   background,
                                   textureAnimatedSubModels,
                                   materialCategoryName);
    if(err != PAL_Ok)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM_VectorTileBuilder::CreateVectorRoadModel has error: 0x%08x", err);
    }
    return PAL_Ok;
}
