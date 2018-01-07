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

(C) Copyright 2010 by Networks In Motion, Inc.

The information contained herein is confidential, proprietary
to Networks In Motion, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of Networks In Motion is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#include "nbgmvectortilebuilder.h"
#include "nbgmvectortiledata.h"
#include "nbgmdom.h"
#include "nbrelog.h"
#include "nbgmmapmaterialmanager.h"
#include "nbgmmapmaterialfactory.h"
#include "nbreentity.h"
#include "nbremesh.h"
#include "nbrevertexindexdata.h"
#include "nbrerenderpal.h"
#include "nbreshadermanager.h"
#include "nbgmbuildmodelcontext.h"
#include "nbrecontext.h"
#include "nbgmcontext.h"
#include "nbrerenderengine.h"
#include "nbgmcommon.h"

#define TAN0         0.00000000000000000000000000000000f
#define TAN5         0.08748866352592400522201866943496f
#define TAN10        0.17632698070846497347109038686862f
#define TAN15        0.26794919243112270647255365849413f
#define TAN20        0.36397023426620236135104788277683f
#define TAN25        0.46630765815499859283000619479956f
#define TAN30        0.57735026918962576450914878050196f
#define TAN35        0.70020753820970977945852271944483f
#define TAN40        0.83909963117728001176312729812318f
#define TAN45        1.00000000000000000000000000000000f

static float
LowPrecision(NBGM_ResourceContext& resourceContext, double /*offset*/, double /*orgOffset*/, float x)
{
    return resourceContext.WorldToModel(x);
}

static NBRE_VertexData*
CreateVertexDataFromDomGeomerty(NBGM_ResourceContext& rc, NBRE_IRenderPal& renderPal, const NBRE_DOM_Geometry& domGeom,
                                LayerDataCrateStruct* cs)
{
    NBRE_VertexData* vertexData = NBRE_NEW NBRE_VertexData(0);
    NBRE_VertexDeclaration* decl = renderPal.CreateVertexDeclaration();
    if(vertexData == NULL || decl == NULL)
    {
        NBRE_DELETE vertexData;
        NBRE_DELETE decl;
        return NULL;
    }
    vertexData->SetVertexDeclaration(decl);

    uint32 source = 0;
    if(domGeom.position != NULL)
    {
        uint32 posBufSize = domGeom.size*sizeof(float)*3;
        NBRE_HardwareVertexBuffer* posBuf = renderPal.CreateVertexBuffer(sizeof(float)*3, domGeom.size, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
        NBRE_VertexElement* elem = NBRE_NEW NBRE_VertexElement(source++, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
        float* posPoint = NBRE_NEW float[posBufSize];
        if(vertexData == NULL || decl == NULL || posPoint == NULL)
        {
            NBRE_DELETE posBuf;
            NBRE_DELETE elem;
            NBRE_DELETE posPoint;
            NBRE_DELETE vertexData;
            return NULL;
        }

        if(cs->layerType == NBRE_DLT_Raster)
        {
            for(uint16 i = 0; i < domGeom.size*3; i+=3)
            {
                posPoint[i] = LowPrecision(rc, cs->offset.x, cs->orgOffset.x, domGeom.position[i]);
                posPoint[i + 1] = LowPrecision(rc, cs->offset.y, cs->orgOffset.y, domGeom.position[i + 1]);
                posPoint[i + 2] = rc.WorldToModel(domGeom.position[i + 2]);
            }
        }
        else
        {
            for(uint16 i = 0; i < domGeom.size*3; ++i)
            {
                posPoint[i] = rc.WorldToModel(domGeom.position[i]);
            }
        }

        if (renderPal.GetCapabilities().GetSupportHBAO())
        {
            for(uint16 i = 0; i < domGeom.size*3; i += 3)
            {
                float z = MERCATER_TO_METER(rc.ModelToWorld(posPoint[i + 2]));
                if (cs->layerType == NBRE_DLT_UnTextureBuilding ||
                    cs->layerType == NBRE_DLT_UnTextureLandmarkBuilding)
                {
                    if (z > 3.0f)
                    {
                        z = (z - 3.0f) / 0.3f;
                    }
                    posPoint[i + 2] = rc.WorldToModel(METER_TO_MERCATOR(z));
                }
                else if (cs->layerType == NBRE_DLT_LandmarkBuilding)
                {
                    posPoint[i + 2] /= 0.7f;
                }
            }
        }

        posBuf->WriteData(0, posBufSize, posPoint, TRUE);

        vertexData->AddBuffer(posBuf);
        decl->GetVertexElementList().push_back(elem);

        NBRE_DELETE_ARRAY posPoint;
    }

    if(domGeom.texCoord != NULL)
    {
        NBRE_HardwareVertexBuffer* texCoordBuf = renderPal.CreateVertexBuffer(sizeof(float)*2, domGeom.size, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
        NBRE_VertexElement* elem = NBRE_NEW NBRE_VertexElement(source++, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
        if(vertexData == NULL || decl == NULL)
        {
            NBRE_DELETE texCoordBuf;
            NBRE_DELETE elem;
            NBRE_DELETE vertexData;
            return NULL;
        }

        uint32 texCoordBufSize = domGeom.size*sizeof(float)*2;
        texCoordBuf->WriteData(0, texCoordBufSize, domGeom.texCoord, TRUE);
        vertexData->AddBuffer(texCoordBuf);
        decl->GetVertexElementList().push_back(elem);
    }

    if(domGeom.normal != NULL)
    {
        NBRE_HardwareVertexBuffer* norBuf = renderPal.CreateVertexBuffer(sizeof(float)*3, domGeom.size, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
        NBRE_VertexElement* elem = NBRE_NEW NBRE_VertexElement(source++, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_NORMAL, 0);
        if(vertexData == NULL || decl == NULL)
        {
            NBRE_DELETE norBuf;
            NBRE_DELETE elem;
            NBRE_DELETE vertexData;
            return NULL;
        }

        uint32 norBufSize = domGeom.size*sizeof(float)*3;
        norBuf->WriteData(0, norBufSize, domGeom.normal, TRUE);
        vertexData->AddBuffer(norBuf);
        decl->GetVertexElementList().push_back(elem);
    }

    if(cs->layerType == NBRE_DLT_Raster)
    {
        NBRE_HardwareVertexBuffer* colorBuf = renderPal.CreateVertexBuffer(sizeof(float)*4, domGeom.size, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
        NBRE_VertexElement* elem = NBRE_NEW NBRE_VertexElement(source++, 0, NBRE_VertexElement::VET_FLOAT4, NBRE_VertexElement::VES_DIFFUSE, 0);
        if(vertexData == NULL || decl == NULL)
        {
            NBRE_DELETE colorBuf;
            NBRE_DELETE elem;
            NBRE_DELETE vertexData;
            return NULL;
        }

        float* colorData = (float*)colorBuf->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY);
        for (uint32 i = 0; i < domGeom.size; ++i)
        {
            float* pColor = colorData + i * 4;
            pColor[0] = 1.0f;
            pColor[1] = 1.0f;
            pColor[2] = 1.0f;
            pColor[3] = 1.0f;
        }
        colorBuf->UnLock();

        vertexData->AddBuffer(colorBuf);
        decl->GetVertexElementList().push_back(elem);
    }

    return vertexData;
}

static NBRE_PrimitiveType
GetPrimitiveType(uint8 triangleFlag)
{
    switch(triangleFlag)
    {
    case 0:
        return NBRE_PMT_TRIANGLE_LIST;
    case 1:
        return NBRE_PMT_TRIANGLE_STRIP;
    case 2:
        return  NBRE_PMT_TRIANGLE_FAN;
    default:
        NBRE_DebugLog(PAL_LogSeverityCritical, "GetPrimitiveType failed, triangleFlag=%u", triangleFlag);
        nbre_assert(0);
    }
    return NBRE_PMT_TRIANGLE_LIST;
}

static NBRE_IndexData*
CreateIndexDataFromDomMesh(NBRE_IRenderPal& renderPal, const NBRE_DOM_Mesh& domMesh)
{
    NBRE_HardwareIndexBuffer* indexBuffer = renderPal.CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, domMesh.indicesCount, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    if(indexBuffer == NULL)
    {
        return NULL;
    }

    indexBuffer->WriteData(0, sizeof(uint16)*domMesh.indicesCount, domMesh.indices, TRUE);

    return NBRE_NEW NBRE_IndexData(indexBuffer, 0, domMesh.indicesCount);
}


//static NBRE_Vector3f
//CalcNormal(float* vertex, uint16* index, NBGM_Vector3d* normal)
//{
//    NBRE_Vector3f a(vertex[index[1]*3] - vertex[index[0]*3],
//                    vertex[index[1]*3+1] - vertex[index[0]*3+1],
//                    vertex[index[1]*3+2] - vertex[index[0]*3+2]);
//
//    NBRE_Vector3f b(vertex[index[2]*3] - vertex[index[0]*3],
//                    vertex[index[2]*3+1] - vertex[index[0]*3+1],
//                    vertex[index[2]*3+2] - vertex[index[0]*3+2]);
//
//    return a.CrossProduct(b);
//}

static void
GetTextureCoordinateFromNormal(const NBRE_Vector3f& normal, NBRE_Vector2f& texcord)
{
    float x = nsl_fabs(normal.x);
    float y = nsl_fabs(normal.y);

    if((normal.z > x) && (normal.z > y))
    {
        texcord.x = 0.5f;
        texcord.y = 0.75f;
    }
    else
    {
        float tan = 0.0f;

        texcord.x = 0.0f;
        texcord.y = 0.25f;
        if(x > y)
        {
            tan = y/x;
        }
        else
        {
            tan = x/y;
        }

        if(tan > TAN45)
        {
            texcord.x = 0.125f;
        }
        else if(tan > TAN40)
        {
            texcord.x = 0.1125f;
        }
        else if(tan > TAN35)
        {
            texcord.x = 0.1f;
        }
        else if(tan > TAN30)
        {
            texcord.x = 0.0875f;
        }
        else if(tan > TAN25)
        {
            texcord.x = 0.075f;
        }
        else if(tan > TAN20)
        {
            texcord.x = 0.0625f;
        }
        else if(tan > TAN15)
        {
            texcord.x = 0.05f;
        }
        else if(tan > TAN10)
        {
            texcord.x = 0.0375f;
        }
        else if(tan > TAN5)
        {
            texcord.x = 0.025f;
        }
        else if(tan > TAN0)
        {
            texcord.x = 0.0125f;
        }

        if(x>y)
        {
            texcord.x = 0.25f - texcord.x;
        }
        if(normal.y < 0)
        {
            texcord.x = 0.5f - texcord.x;
        }
        if(normal.x < 0)
        {
            texcord.x = 1.0f - texcord.x;
        }
    }
}

static void
UntexturedBuildingAddTextureCoordinate(NBGM_ResourceContext& resourceContext, NBRE_SubMesh* subMesh)
{
    NBGM_FloatBuffer& vertexBuffer = resourceContext.buildModelContext->mVetexBuffer;
    NBGM_FloatBuffer& normalBuffer = resourceContext.buildModelContext->mNormalBuffer1;
    NBGM_FloatBuffer& texcoordBuffer = resourceContext.buildModelContext->mTexcoordBuffer1;
    NBGM_ShortBuffer& indexBuffer = resourceContext.buildModelContext->mIndexBuffer;

    vertexBuffer.clear();
    texcoordBuffer.clear();
    normalBuffer.clear();
    indexBuffer.clear();

    NBRE_VertexData* vertexData = subMesh->GetVertexData();
    NBRE_IndexData* indexData = subMesh->GetIndexData();

    NBRE_VertexDeclaration* vertexDeclaration = subMesh->GetVertexData()->GetVertexDeclaration();
    NBRE_VertexDeclaration::VertexElementList vertexElementList = vertexDeclaration->GetVertexElementList();

    NBRE_VertexDeclaration::VertexElementList::iterator iter;
    nb_boolean hasNormal = FALSE;
    NBRE_HardwareVertexBuffer* hardwareNormalBuffer = NULL;
    uint32 normalOffset = 0;
    for (iter = vertexElementList.begin(); iter != vertexElementList.end(); ++iter)
    {
        if ((*iter)->Semantic() == NBRE_VertexElement::VES_NORMAL)
        {
            hardwareNormalBuffer = vertexData->GetBuffer((*iter)->Source());
            normalOffset = (*iter)->Offset();
            hasNormal = TRUE;
            break;
        }
    }

    for (iter = vertexElementList.begin(); iter != vertexElementList.end(); ++iter)
    {
        if ((*iter)->Semantic() == NBRE_VertexElement::VES_POSITION)
        {
            NBRE_HardwareVertexBuffer* hardwareVertexBuffer = vertexData->GetBuffer((*iter)->Source());
            uint32 stride = hardwareVertexBuffer->GetVertexSize();
            uint8* dataBuff = static_cast<uint8*>(hardwareVertexBuffer->Lock((*iter)->Offset(), NBRE_HardwareBuffer::HBL_WRITE_DISCARD));
            float* vertex = NULL;
            uint16* index = static_cast<uint16*>(indexData->IndexBuffer()->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_DISCARD));

            float* normal = NULL;
            uint32 normalStride = 0;
            uint8* normalDataBuff = NULL;
            if (hasNormal)
            {
                normalStride = hardwareNormalBuffer->GetVertexSize();
                normalDataBuff = static_cast<uint8*>(hardwareNormalBuffer->Lock(normalOffset, NBRE_HardwareBuffer::HBL_WRITE_DISCARD));
            }

            if(subMesh->GetOperationType() == NBRE_PMT_TRIANGLE_LIST)
            {
                for (uint16 i = 0; i < indexData->IndexBuffer()->GetNumIndexes(); i += 3)
                {
                    vertex = reinterpret_cast<float*>(dataBuff+index[i+1]*stride);
                    NBRE_Vector3f a(vertex[0], vertex[1], vertex[2]);

                    vertex = reinterpret_cast<float*>(dataBuff+index[i+2]*stride);
                    NBRE_Vector3f b(vertex[0], vertex[1], vertex[2]);

                    vertex = reinterpret_cast<float*>(dataBuff+index[i]*stride);
                    NBRE_Vector3f c(vertex[0], vertex[1], vertex[2]);

                    if (hasNormal)
                    {
                        normal = reinterpret_cast<float*>(normalDataBuff+index[i+1]*normalStride);
                        NBRE_Vector3f na(normal[0], normal[1], normal[2]);

                        normal = reinterpret_cast<float*>(normalDataBuff+index[i+2]*normalStride);
                        NBRE_Vector3f nb(normal[0], normal[1], normal[2]);

                        normal = reinterpret_cast<float*>(normalDataBuff+index[i]*normalStride);
                        NBRE_Vector3f nc(normal[0], normal[1], normal[2]);

                        normalBuffer.push_back(nc.x);
                        normalBuffer.push_back(nc.y);
                        normalBuffer.push_back(nc.z);
                        normalBuffer.push_back(na.x);
                        normalBuffer.push_back(na.y);
                        normalBuffer.push_back(na.z);
                        normalBuffer.push_back(nb.x);
                        normalBuffer.push_back(nb.y);
                        normalBuffer.push_back(nb.z);
                    }

                    NBRE_Vector2f texcord;
                    GetTextureCoordinateFromNormal((a-c).CrossProduct(b-c), texcord);

                    vertexBuffer.push_back(c.x);
                    vertexBuffer.push_back(c.y);
                    vertexBuffer.push_back(c.z);
                    vertexBuffer.push_back(a.x);
                    vertexBuffer.push_back(a.y);
                    vertexBuffer.push_back(a.z);
                    vertexBuffer.push_back(b.x);
                    vertexBuffer.push_back(b.y);
                    vertexBuffer.push_back(b.z);

                    texcoordBuffer.push_back(texcord.x);
                    texcoordBuffer.push_back(texcord.y);
                    texcoordBuffer.push_back(texcord.x);
                    texcoordBuffer.push_back(texcord.y);
                    texcoordBuffer.push_back(texcord.x);
                    texcoordBuffer.push_back(texcord.y);

                    indexBuffer.push_back(i);
                    indexBuffer.push_back(i+1);
                    indexBuffer.push_back(i+2);
                }
            }

            hardwareVertexBuffer->UnLock();
            indexData->IndexBuffer()->UnLock();

            vertexDeclaration->ClearVertexElementList();

            NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
            vertexDeclaration->GetVertexElementList().push_back(posElem);

            NBRE_VertexElement* texElem = NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
            vertexDeclaration->GetVertexElementList().push_back(texElem);

            vertexData->SetVertexCount(vertexBuffer.size()/3);

            if (hasNormal)
            {
                NBRE_VertexElement* normalElem = NBRE_NEW NBRE_VertexElement(2, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_NORMAL, 0);
                vertexDeclaration->GetVertexElementList().push_back(normalElem);

                hardwareNormalBuffer->Resize(normalBuffer.size()*sizeof(float));
                hardwareNormalBuffer->WriteData(0, normalBuffer.size()*sizeof(float), &normalBuffer.front(), TRUE);
            }

            hardwareVertexBuffer->Resize(vertexBuffer.size()*sizeof(float));
            hardwareVertexBuffer->WriteData(0, vertexBuffer.size()*sizeof(float), &vertexBuffer.front(), TRUE);

            NBRE_IRenderPal& rp = *(resourceContext.renderPal);
            NBRE_HardwareVertexBuffer* texcordBuf = rp.CreateVertexBuffer(sizeof(float) * 2, texcoordBuffer.size()/2, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
            texcordBuf->WriteData(0, texcoordBuffer.size()*sizeof(float), &texcoordBuffer.front(), TRUE);
            vertexData->AddBuffer(1, texcordBuf);

            indexData->IndexBuffer()->WriteData(0, indexBuffer.size()*sizeof(uint16), &indexBuffer.front(), TRUE);

            break;
        }
    }
}

static NBRE_Mesh*
CreateMeshFromMapdata(NBGM_ResourceContext& resourceContext, const NBRE_DOM_Mesh& domMesh, const NBRE_DOM_Geometry& domGeometry, const NBRE_MapMaterial& mat, LayerDataCrateStruct* cs)
{
    NBRE_Mesh* result = NBRE_NEW NBRE_Mesh(NULL);
    NBRE_IRenderPal& renderPal = *(resourceContext.renderPal);
    NBRE_VertexData* vertexData = CreateVertexDataFromDomGeomerty(resourceContext, renderPal, domGeometry, cs);
    NBRE_IndexData* indexData = CreateIndexDataFromDomMesh(renderPal, domMesh);
    if(result == NULL || vertexData == NULL || indexData == NULL)
    {
        NBRE_DELETE result;
        NBRE_DELETE vertexData;
        NBRE_DELETE indexData;
        return NULL;
    }

    NBRE_PrimitiveType primitiveType = GetPrimitiveType(domMesh.flag);
    NBRE_SubMesh* subMesh = result->CreateSubMesh(vertexData, indexData, primitiveType);
    if(subMesh == NULL)
    {
        NBRE_DELETE result;
        NBRE_DELETE vertexData;
        NBRE_DELETE indexData;
        return NULL;
    }

    if(mat.Type() == NBRE_MMT_HorizontalLightSourceMaterial)
    {
        UntexturedBuildingAddTextureCoordinate(resourceContext, subMesh);
    }

    result->CalculateBoundingBox();
    return result;
}

static NBRE_ModelPtr
CreateModelFromMapdata(NBGM_ResourceContext& resourceContext, NBRE_DOM* dom, const NBRE_DOM_Mesh& domMesh, const NBRE_DOM_Geometry& domGeometry, NBRE_Layer* layer, LayerDataCrateStruct* cs)
{
    if(!VERIFY_NBM_INDEX(domMesh.materialIndex))
    {
        return NBRE_ModelPtr();
    }

    NBRE_MapMaterial* createdMat = NULL;
    const NBRE_MapMaterial* mat = NULL;
    if(VERIFY_NBM_INTERNAL_INDEX(domMesh.materialIndex))
    {
        NBRE_MapMaterialFactory::CreateMapMaterialFromDom(resourceContext.palInstance, *resourceContext.renderPal,
                                                          *resourceContext.textureManager, *dom, domMesh.materialIndex, FALSE, &createdMat);
        mat = createdMat;
    }
    else
    {
        uint16 id = MAKE_NBM_INDEX(domMesh.materialIndex);
        mat = &resourceContext.mapMaterialManager->GetCurrentGroup(cs->materialCategoryName).GetMapMaterial(id);
    }

    if(mat == NULL)
    {
        return NBRE_ModelPtr();
    }
    cs->layerType = layer->info.layerType;

    NBRE_Mesh* mesh = CreateMeshFromMapdata(resourceContext, domMesh, domGeometry, *mat, cs);
    if(mesh == NULL)
    {
        NBRE_DELETE createdMat;
        return NBRE_ModelPtr();
    }
    NBRE_ModelPtr model(NBRE_NEW NBRE_Model(NBRE_MeshPtr(mesh)));

    NBRE_ShaderPtr shader = mat->GetShader(0);
    shader->GetPass(0)->SetEnableDepthWrite(FALSE);
    if(layer->info.layerType == NBRE_DLT_Raster)
    {
        shader->GetPass(0)->GetTextureUnits()[0].SetState(NBRE_TextureState(NBRE_TFT_MIPMAP_LINEAR, NBRE_TFT_LINEAR, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, FALSE));
    }
    else if(layer->info.layerType == NBRE_DLT_LandmarkBuilding ||
            layer->info.layerType == NBRE_DLT_UnTextureBuilding ||
            layer->info.layerType == NBRE_DLT_UnTextureLandmarkBuilding ||
            layer->info.layerType == NBRE_DLT_ECM)
    {
        shader->GetPass(0)->SetEnableDepthWrite(TRUE);
    }

    if(layer->info.layerType == NBRE_DLT_LandmarkBuilding)
    {
        shader->GetPass(0)->SetGpuProgram("light3d");
        shader->GetPass(0)->SetGpuProgramParam("mvp", NBRE_GpuProgramParam(NBRE_GpuProgramParam::PV_ModelViewProjectionMatrix));
        shader->GetPass(0)->SetGpuProgramParam("normalMat", NBRE_GpuProgramParam(NBRE_GpuProgramParam::PV_NormalMatrix));
        shader->GetPass(0)->SetGpuProgramParam("u_texSampler_1", NBRE_GpuProgramParam(0));
    }

    shader->GetPass(0)->SetEnableLighting(FALSE);
    shader->GetPass(0)->SetEnableDepthTest(TRUE);
    shader->GetPass(0)->SetEnableCullFace(TRUE);
    shader->GetPass(0)->SetEnableBlend(TRUE);
    if(createdMat != NULL)
    {
        mat = NULL;
        model->SetShader(shader);
        NBRE_DELETE createdMat;
        nbre_assert(shader.use_count() == 2);
    }
    else
    {
        NBRE_String shaderID = GenerateID(cs->materialCategoryName, mat->GetShaderName(0));
        model->SetShaderName(shaderID);
    }

    return model;
}

void
CreateModelsFromMapdata(NBGM_ResourceContext& resoureceContext, NBRE_DOM* dom, const NBRE_DOM_MESH& meshCunk, const NBRE_DOM_GEOM& geomChunk, NBRE_Vector<NBRE_ModelPtr>& models, NBRE_Layer* layer, LayerDataCrateStruct* cs)
{
    for(uint16 i=0; i<meshCunk.count; ++i)
    {
        NBRE_DOM_Mesh& domMesh = meshCunk.meshes[i];
        if(domMesh.geometryIndex >= geomChunk.count)
        {
            continue;
        }

        NBRE_DOM_Geometry& domGeometry = geomChunk.geometries[domMesh.geometryIndex];
        NBRE_ModelPtr model = CreateModelFromMapdata(resoureceContext, dom, domMesh, domGeometry, layer, cs);
        if(model != NULL)
        {
            models.push_back(model);
        }
    }
}
