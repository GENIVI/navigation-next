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
#include "nbrelog.h"
#include "nbreentity.h"
#include "nbgmmapmaterialmanager.h"
#include "nbrerenderengine.h"
#include "nbgmcommon.h"
#include "nbgmbuildutility.h"

static NBRE_VertexData*
CreateVertexData(NBRE_IRenderPal& rp)
{
    NBRE_VertexDeclaration* decalration = rp.CreateVertexDeclaration();

    NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem);

    NBRE_VertexElement* texElem = NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
    decalration->GetVertexElementList().push_back(texElem);

    NBRE_VertexData* vertexData = NBRE_NEW NBRE_VertexData(0);
    vertexData->SetVertexDeclaration(decalration);

    return vertexData;
}

static const NBRE_MapMaterial*
GetMaterial(NBGM_ResourceContext& resourceContext, const NBRE_LayerAREAData* data, nb_boolean& isInternalMat, const NBRE_String& materialCategoryName)
{
    if (data->material == NULL)
    {
        const NBRE_MapMateriaGroup& materiaGroup = resourceContext.mapMaterialManager->GetCurrentGroup(materialCategoryName);
        isInternalMat = FALSE;
        return &materiaGroup.GetMapMaterial(data->materialId);
    }
    else
    {
        isInternalMat = TRUE;
        return static_cast<const NBRE_MapMaterial*>(data->material);
    }
}

static NBRE_IndexData*
CreateIndexData(NBRE_IRenderPal& rp, NBGM_ShortBuffer& indexBuffer)
{
    NBRE_HardwareIndexBuffer* indexBuf = rp.CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, indexBuffer.size(), TRUE, NBRE_HardwareBuffer::HBU_STATIC);

    indexBuf->WriteData(0, indexBuffer.size()*sizeof(uint16), &indexBuffer.front(), TRUE);
    return NBRE_NEW NBRE_IndexData(indexBuf, 0, indexBuf->GetNumIndexes());
}

static void
FillVerticesFromBuffer(NBRE_IRenderPal& rp, NBRE_VertexData* vertexData, NBGM_FloatBuffer& vertexBuffer, NBGM_FloatBuffer& texcoordBuffer)
{
    NBRE_HardwareVertexBuffer* vertextBuf = rp.CreateVertexBuffer(sizeof(float) * 2, vertexBuffer.size()/2, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    NBRE_HardwareVertexBuffer* texcordBuf = rp.CreateVertexBuffer(sizeof(float) * 2, texcoordBuffer.size()/2, TRUE, NBRE_HardwareBuffer::HBU_STATIC);

    vertexData->AddBuffer(vertextBuf);
    vertexData->AddBuffer(texcordBuf);

    vertextBuf->WriteData(0, vertexBuffer.size()*sizeof(float), &vertexBuffer.front(), TRUE);
    texcordBuf->WriteData(0, texcoordBuffer.size()*sizeof(float), &texcoordBuffer.front(), TRUE);
}

static NBRE_Model*
BuildAreaModel(NBGM_ResourceContext& resourceContext, const NBRE_Point3d& tileCenter, NBRE_Layer* areaLayer, const NBRE_String& materialCategoryName)
{
    NBGM_FloatBuffer& vertexBuffer = resourceContext.buildModelContext->mVetexBuffer;
    NBGM_FloatBuffer& texcoordBuffer = resourceContext.buildModelContext->mTexcoordBuffer1;
    NBGM_ShortBuffer& indexBuffer = resourceContext.buildModelContext->mIndexBuffer;

    vertexBuffer.clear();
    texcoordBuffer.clear();

    // Convert areas to drawing data
    NBRE_IRenderPal& rp = *resourceContext.renderPal;
    NBRE_VertexData* vertexData = CreateVertexData(rp);

    //create area mesh
    NBRE_Mesh* mesh = NBRE_NEW NBRE_Mesh(vertexData);
    NBRE_Model* model = NBRE_NEW NBRE_Model(NBRE_MeshPtr(mesh));

    for (uint32 i = 0; i < areaLayer->dataCount; ++i)
    {
        NBRE_String shaderName;
        nb_boolean isInternalMat = FALSE;
        NBRE_ShaderPtr shader;
        NBRE_LayerAREAData* areaData = static_cast<NBRE_LayerAREAData*>(areaLayer->data[i]);
        const NBRE_MapMaterial* material = GetMaterial(resourceContext, areaData, isInternalMat, materialCategoryName);

        indexBuffer.clear();

        switch(material->Type())
        {
        case NBRE_MMT_StandardTexturedMaterial:
            {
                const NBRE_MapStandardTexturedMaterial* textureMaterial = static_cast<const NBRE_MapStandardTexturedMaterial*>(material);
                if (isInternalMat)
                {
                    shader = textureMaterial->Shader();
                }
                else
                {
                    shaderName = textureMaterial->ShaderName();
                }
                NBGM_BuildModelUtility::PolygonPack2FloatArray(resourceContext, tileCenter, areaData->polygon, vertexBuffer, texcoordBuffer, indexBuffer);
            }
            break;

        case NBRE_MMT_StandardColorMaterial:
            {
                const NBRE_MapStandardColorMaterial* colorMaterial = static_cast<const NBRE_MapStandardColorMaterial*>(material);
                if (isInternalMat)
                {
                    shader = colorMaterial->GetShader(0);
                }
                else
                {
                    shaderName = colorMaterial->GetShaderName(0);
                }
                NBGM_BuildModelUtility::PolygonPack2FloatArray(resourceContext, tileCenter, areaData->polygon, vertexBuffer, texcoordBuffer, indexBuffer);
            }
            break;

        case NBRE_MMT_OutlinedComplexColorFillMaterial:
            {
                const NBRE_OutlinedComplexColorFillMaterial* outlineComplexMaterial = static_cast<const NBRE_OutlinedComplexColorFillMaterial*>(material);
                if (isInternalMat)
                {
                    shader = outlineComplexMaterial->InteriorShader();
                }
                else
                {
                    shaderName = outlineComplexMaterial->InteriorShaderName();
                }
                NBGM_BuildModelUtility::PolygonPack2FloatArray(resourceContext, tileCenter, areaData->polygon, vertexBuffer, texcoordBuffer, indexBuffer);
            }
            break;

        case NBRE_MMT_OutlinedSimpleColorFillMaterial:
            {
                const NBRE_OutlinedSimpleColorFillMaterial* outlineSimpleMaterial = static_cast<const NBRE_OutlinedSimpleColorFillMaterial*>(material);
                if (isInternalMat)
                {
                    shader = outlineSimpleMaterial->InteriorShader();
                }
                else
                {
                    shaderName = outlineSimpleMaterial->InteriorShaderName();
                }
                NBGM_BuildModelUtility::ConvexPolygonPack2FloatArray(resourceContext, tileCenter, areaData->polygon, vertexBuffer, texcoordBuffer, indexBuffer);
            }
            break;
        case NBRE_MMT_OutlinedHoleyMaterial:
            {
                const NBRE_OutlinedHoleyMaterial* outlineComplexMaterial = static_cast<const NBRE_OutlinedHoleyMaterial*>(material);
                if (isInternalMat)
                {
                    shader = outlineComplexMaterial->InteriorShader();
                }
                else
                {
                    shaderName = outlineComplexMaterial->InteriorShaderName();
                }
                NBGM_BuildModelUtility::HoleyPolygonPack2FloatArray(resourceContext, tileCenter, areaData->polygon, vertexBuffer, texcoordBuffer, indexBuffer);
            }
            break;

        default:
            {
                if(shaderName == "")
                {
                    NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_BuildAreaModel: unknown material->maetrialType is %d", material->Type());
                    continue;
                }
            }
        }

        if (indexBuffer.size() > 0)
        {
            NBRE_IndexData* indexData = CreateIndexData(rp, indexBuffer);
            NBRE_SubModel* subModel = model->CreateSubModel(indexData, NBRE_PMT_TRIANGLE_LIST);

            if (isInternalMat)
            {
                subModel->SetShader(shader);
            }
            else
            {
                subModel->SetShaderName(GenerateID(materialCategoryName, shaderName));
            }
        }
    }

    if(vertexBuffer.size() == 0 || texcoordBuffer.size() == 0)
    {
        NBRE_DELETE model;
        model = NULL;
        NBRE_DebugLog(PAL_LogSeverityMajor, "Invalid data!");
        return model;
    }

    FillVerticesFromBuffer(rp, vertexData, vertexBuffer, texcoordBuffer);
    model->GetMesh()->CalculateBoundingBox();

    return model;
}

static NBRE_Model*
BuildAreaWireFrameModel(NBGM_ResourceContext& resourceContext, const NBRE_Point3d& tileCenter, NBRE_Layer* areaLayer, const NBRE_String& materialCategoryName)
{
    NBGM_FloatBuffer& vertexBuffer = resourceContext.buildModelContext->mVetexBuffer;
    NBGM_FloatBuffer& texcoordBuffer = resourceContext.buildModelContext->mTexcoordBuffer1;
    NBGM_ShortBuffer& indexBuffer = resourceContext.buildModelContext->mIndexBuffer;

    vertexBuffer.clear();
    texcoordBuffer.clear();

    NBRE_IRenderPal& rp = *resourceContext.renderPal;
    NBRE_VertexData* vertexData = CreateVertexData(rp);

    //create area mesh
    NBRE_Mesh* mesh = NBRE_NEW NBRE_Mesh(vertexData);
    NBRE_Model* model = NBRE_NEW NBRE_Model(NBRE_MeshPtr(mesh));

    for (uint32 i = 0; i < areaLayer->dataCount; ++i)
    {
        NBRE_String shaderName;
        nb_boolean isInternalMat = FALSE;
        NBRE_ShaderPtr shader;
        NBRE_LayerAREAData* areaData = static_cast<NBRE_LayerAREAData*>(areaLayer->data[i]);
        const NBRE_MapMaterial* material = GetMaterial(resourceContext, areaData, isInternalMat, materialCategoryName);

        indexBuffer.clear();

        switch(material->Type())
        {
        case NBRE_MMT_OutlinedComplexColorFillMaterial:
            {
                const NBRE_OutlinedComplexColorFillMaterial* outlineComplexMaterial = static_cast<const NBRE_OutlinedComplexColorFillMaterial*>(material);
                if (isInternalMat)
                {
                    shader = outlineComplexMaterial->OutlinedShader();
                }
                else
                {
                    shaderName = outlineComplexMaterial->OutlinedShaderName();
                }
            }
            break;

        case NBRE_MMT_OutlinedSimpleColorFillMaterial:
            {
                const NBRE_OutlinedSimpleColorFillMaterial* outlineSimpleMaterial = static_cast<const NBRE_OutlinedSimpleColorFillMaterial*>(material);
                if (isInternalMat)
                {
                    shader = outlineSimpleMaterial->OutlinedShader();
                }
                else
                {
                    shaderName = outlineSimpleMaterial->OutlinedShaderName();
                }
            }
            break;
        case NBRE_MMT_OutlinedHoleyMaterial:
            {
                const NBRE_OutlinedHoleyMaterial* outlineComplexMaterial = static_cast<const NBRE_OutlinedHoleyMaterial*>(material);
                if (isInternalMat)
                {
                    shader = outlineComplexMaterial->OutlinedShader();
                }
                else
                {
                    shaderName = outlineComplexMaterial->OutlinedShaderName();
                }
            }
            break;

        default:
            {
                if(shaderName == "")
                {
                    NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_BuildAreaModel: unknown material->maetrialType is %d", material->Type());
                    continue;
                }
            }
            break;
        }

        NBGM_BuildModelUtility::PolygonWireFramePack2FloatArray(tileCenter, areaData->polygon, vertexBuffer, texcoordBuffer, indexBuffer);

        NBRE_IndexData* indexData = CreateIndexData(rp, indexBuffer);
        NBRE_SubModel* subModel = model->CreateSubModel(indexData, NBRE_PMT_LINE_LIST);

        if (isInternalMat)
        {
            subModel->SetShader(shader);
        }
        else
        {
            subModel->SetShaderName(GenerateID(materialCategoryName, shaderName));
        }
    }

    if(vertexBuffer.size() == 0 || texcoordBuffer.size() == 0)
    {
        NBRE_DELETE model;
        model = NULL;
        NBRE_DebugLog(PAL_LogSeverityMajor, "Invalid data!");
        return model;
    }

    FillVerticesFromBuffer(rp, vertexData, vertexBuffer, texcoordBuffer);
    mesh->CalculateBoundingBox();

    return model;
}

NBRE_Model*
NBGM_VectorTileBuilder::CreateVectorAreaModel(NBGM_ResourceContext& resourceContext, NBRE_Layer* dataTile, const NBRE_Point3d& tileCenter, const NBRE_String& materialCategoryName)
{
    NBRE_Model* model = BuildAreaModel(resourceContext, tileCenter, dataTile, materialCategoryName);
    return model;
}

NBRE_Model*
NBGM_VectorTileBuilder::CreateVectorAreaWireFrameModel(NBGM_ResourceContext& resourceContext, NBRE_Layer* dataTile, const NBRE_Point3d& tileCenter, const NBRE_String& materialCategoryName)
{
    NBRE_Model* model = BuildAreaWireFrameModel(resourceContext, tileCenter, dataTile, materialCategoryName);
    return model;
}
