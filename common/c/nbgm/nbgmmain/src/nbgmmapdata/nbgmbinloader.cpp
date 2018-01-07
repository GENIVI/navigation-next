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
#include "nbgmbinloader.h"
#include "pal.h"
#include "paltypes.h"
#include "palfile.h"
#include "nbrefilestream.h"
#include "palerror.h"
#include "nbrepngtextureimage.h"
#include "nbrerenderpal.h"
#include "nbretexture.h"
#include "nbretexturemanager.h"
#include "nbrelog.h"
#include "nbgm.h"
#include "nbrerenderengine.h"
#include "nbgmbuildutility.h"
#include "nbremodel.h"

#define SUPPORTED_BIN_VERSION 1

static PAL_Error FloatArray3DPacked(NBRE_IOStream* stream, uint32 count, float* floatArray);
static PAL_Error FloatArray2DPacked(NBRE_IOStream* stream, uint32 count, float* floatArray);

static NBRE_Model* ModelGeneratorReadGeometryFromFile(NBRE_IRenderPal& renderPal, float scaleFactor, NBRE_IOStream* stream, const NBRE_Vector<NBRE_ShaderPtr>& shaders);
static NBRE_IndexData* ModelGeneratorReadTriangleFromFile(NBRE_IRenderPal& renderer, NBRE_IOStream* stream, uint32* textureIndex);
static char* CreateFileTextureName(const char* texName);
static NBRE_TexturePtr CreateSceneTexture(NBRE_IRenderPal& renderPal, PAL_Instance* palInstance, char* texName);

typedef enum
{
    NBGM_ITS_None = 0,
    NBGM_ITS_UnsignedByteSize = 1,
    NBGM_ITS_UnsignedShortSize = 2,
    NBGM_ITS_UnsignedIntSize = 4
}NBGM_IndicesTypeSize;

static char*
MakeFullPath(const char* folder, const char* fileName)
{
    char* fullPath = NULL;
    if(fileName != NULL)
    {
        int pathLength=0;
        const char* file = fileName;

        //make a full path
        if((fileName[0]=='/') || (fileName[1] == ':' ))
        {
            fullPath = nsl_strdup(fileName);
        }
        else
        {
            if(fileName[0]=='.' && (fileName[1] == '\\' || fileName[1] == '/'))
            {
                file = fileName+2;
            }
            pathLength = nsl_strlen(folder) + nsl_strlen(file) +1;
            fullPath = (char*)nsl_malloc(pathLength);
            if(fullPath == NULL)
            {
                return NULL;
            }
            nsl_memset(fullPath, 0, pathLength);
            nsl_strncpy(fullPath, folder, nsl_strlen(folder));
            nsl_strlcat(fullPath, file, pathLength);
        }
    }
    return fullPath;
}

static PAL_Error
ModelGeneratorReadSceneFromFile_v1(NBRE_IRenderPal& renderPal, PAL_Instance* palInstance, float scaleFactor, NBRE_IOStream* stream, const char* textureFolder, uint32 binFlag, NBRE_Vector<NBRE_ModelPtr>& models)
{
    NBRE_Vector<NBRE_ShaderPtr> shaders;
    uint32 textureImagesCount = 0;
    uint32 geometriesCount = 0;
    float XMax = 0;
    float XMin = 0;
    float YMax = 0;
    float YMin = 0;
    uint32 i=0;
    uint32 readCount = 0;
    PAL_Error err = PAL_Ok;
    uint32 count = 0;

    //NOTE: just support little-endian data
    if(binFlag != 0)
    {
        err = PAL_ErrUnsupported;
        goto HandleError;
    }

    models.clear();

    // get x max
    err = stream->Read((uint8*)&XMax, sizeof(XMax), &readCount);
    XMax = scaleFactor * XMax;

    // get x min
    err = (err != PAL_Ok) ? err : stream->Read((uint8*)&XMin, sizeof(XMin), &readCount);
    XMin = scaleFactor * XMin;

    // get y max
    err = (err != PAL_Ok) ? err : stream->Read((uint8*)&YMax, sizeof(YMax), &readCount);
    YMax = scaleFactor * YMax;

    // get y min
    err = (err != PAL_Ok) ? err : stream->Read((uint8*)&YMin, sizeof(YMin), &readCount);
    YMin = scaleFactor * YMin;

    err = (err != PAL_Ok) ? err : stream->Read((uint8*)&textureImagesCount, sizeof(textureImagesCount), &readCount);
    err = (err != PAL_Ok) ? err : stream->Read((uint8*)&geometriesCount, sizeof(geometriesCount), &readCount);

    if(err != PAL_Ok)
    {
        goto HandleError;
    }

    for(i=0; i<textureImagesCount; ++i)
    {
        unsigned char imageNameLen = 0;
        char* imageName = NULL;
        char* texName = NULL;
        char* fullPath = NULL;

        err = stream->Read((uint8*)&imageNameLen, sizeof(imageNameLen), &readCount);
        if(err != PAL_Ok)
        {
            goto HandleError;
        }
        if (imageNameLen != 0)
        {
            count = (imageNameLen + 1) * sizeof(char);
            imageName = (char *)nsl_malloc(count);
            if (imageName == NULL)
            {
                err = PAL_ErrNoMem;
                goto HandleError;
            }
            nsl_memset(imageName, 0, count);

            err = stream->Read((uint8*)imageName, count - sizeof(char), &readCount);
            if(err != PAL_Ok)
            {
                nsl_free(imageName);
                goto HandleError;
            }
        }
        else
        {
            err = PAL_ErrBadParam;
            goto HandleError;
        }

        if(textureFolder)
        {
            fullPath = MakeFullPath(textureFolder, imageName);
            texName = CreateFileTextureName(fullPath);
            nsl_free(fullPath);
        }
        else
        {
            texName = CreateFileTextureName(imageName);
        }
        NBRE_TexturePtr texture = CreateSceneTexture(renderPal, palInstance, texName);
        nsl_free(imageName);
        nsl_free(texName);

        NBRE_PassPtr pass(NBRE_NEW NBRE_Pass());
        pass->SetEnableDepthTest(TRUE);
        pass->SetEnableDepthWrite(TRUE);
        pass->SetEnableCullFace(TRUE);
        pass->SetEnableBlend(TRUE);
        NBRE_TextureUnit unit;
        unit.SetState(NBRE_TextureState(NBRE_TFT_MIPMAP_TRILINEAR, NBRE_TFT_LINEAR, NBRE_TAM_REPEAT, NBRE_TAM_REPEAT, FALSE));
        unit.SetTexture(texture);
        pass->GetTextureUnits().push_back(unit);
        NBRE_ShaderPtr shader(NBRE_NEW NBRE_Shader());
        shader->AddPass(pass);
        shaders.push_back(shader);
    }

    count = 0;
    for( i=0; i<geometriesCount; ++i)
    {
        NBRE_Model* model = ModelGeneratorReadGeometryFromFile(renderPal, scaleFactor, stream, shaders);
        if (model == NULL)
        {
            err = PAL_ErrFileFailed;
            goto HandleError;
        }
        models.push_back(NBRE_ModelPtr(model));
        ++count;
    }

    return PAL_Ok;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ModelGeneratorReadSceneFromFile_v1, Error! err = 0x%x", err);
    return err;
}

static NBRE_Model*
ModelGeneratorReadGeometryFromFile(NBRE_IRenderPal& renderPal, float scaleFactor, NBRE_IOStream* stream, const NBRE_Vector<NBRE_ShaderPtr>& shaders)
{
    //NOTE: just support little-endian data
    uint32* subMeshTextureIndices = NULL;
    uint32 vertexCount = 0;
    uint32 textureCoordinatesCount = 0;
    uint32 trianglesCount = 0;
    uint32 i = 0;
    uint32 readCount = 0;
    PAL_Error err = PAL_Ok;

    NBRE_VertexDeclaration* decalration = NULL;
    NBRE_VertexData* vertexData= NULL;
    NBRE_VertexElement* posElem1 = NULL;
    NBRE_HardwareVertexBuffer* posBuff = NULL;
    NBRE_VertexElement* normalElem1 = NULL;
    NBRE_HardwareVertexBuffer* normalBuff = NULL;
    NBRE_Mesh* mesh = NULL;
    NBRE_Model* model = NULL;
    float* buf = NULL;

    err =  stream->Read((uint8*)&vertexCount, sizeof(vertexCount), &readCount);
    err = (err != PAL_Ok) ? err : stream->Read((uint8*)&textureCoordinatesCount, sizeof(textureCoordinatesCount), &readCount);
    err = (err != PAL_Ok) ? err : stream->Read((uint8*)&trianglesCount, sizeof(trianglesCount), &readCount);

    if (err != PAL_Ok)
    {
        goto HandleError;
    }

    subMeshTextureIndices = NBRE_NEW uint32[trianglesCount];

    decalration = renderPal.CreateVertexDeclaration();
    vertexData= NBRE_NEW NBRE_VertexData(0);

    posElem1= NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem1);
    posBuff = renderPal.CreateVertexBuffer(sizeof(float) * 3, vertexCount, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    buf = (float*)posBuff->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_DISCARD);

    err = FloatArray3DPacked(stream, vertexCount, buf);
    if (err != PAL_Ok)
    {
        goto HandleError;
    }

    for(uint32 i=0; i<vertexCount; ++i)
    {
		uint32 j = i*3;
        buf[j] = scaleFactor * buf[j];
        buf[j+1] = scaleFactor * buf[j+1];
        buf[j+2] = scaleFactor * buf[j+2];
    }
    vertexData->AddBuffer(posBuff);

    normalElem1= NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_NORMAL, 0);
    decalration->GetVertexElementList().push_back(normalElem1);
    normalBuff = renderPal.CreateVertexBuffer(sizeof(float) * 3, vertexCount, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    buf = (float*)normalBuff->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_DISCARD);

    err = FloatArray3DPacked(stream, vertexCount, buf);
    if (err != PAL_Ok)
    {
        goto HandleError;
    }

    vertexData->AddBuffer(normalBuff);

    if (textureCoordinatesCount > 0)
    {
        float* texcoord = NBRE_NEW float[2*textureCoordinatesCount];
        NBRE_VertexElement* texcoordElem1= NBRE_NEW NBRE_VertexElement(2, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
        decalration->GetVertexElementList().push_back(texcoordElem1);
        NBRE_HardwareVertexBuffer* texcoordBuff = renderPal.CreateVertexBuffer(sizeof(float) * 2, vertexCount, TRUE, NBRE_HardwareBuffer::HBU_STATIC);

        err = FloatArray2DPacked(stream, textureCoordinatesCount, texcoord);
        if (err != PAL_Ok)
        {
            NBRE_DELETE[] texcoord;
            NBRE_DELETE texcoordElem1;
            NBRE_DELETE texcoordBuff;
            goto HandleError;
        }
        texcoordBuff->WriteData(0, sizeof(float) * 2 * vertexCount, texcoord, TRUE);

        vertexData->AddBuffer(texcoordBuff);

        NBRE_DELETE_ARRAY texcoord;
        texcoord = NULL;
    }
    else
    {
        // Call this to skip headers, don't remove it
        float placeHolder[1];
        err = FloatArray2DPacked(stream, textureCoordinatesCount, placeHolder);
        if (err != PAL_Ok)
        {
            goto HandleError;
        }
    }

    vertexData->SetVertexDeclaration(decalration);
    mesh = NBRE_NEW NBRE_Mesh(vertexData);

    for (i = 0; i < trianglesCount; i++)
    {
        NBRE_IndexData* triangle = ModelGeneratorReadTriangleFromFile(renderPal, stream, &subMeshTextureIndices[i]);
        mesh->CreateSubMesh(triangle, NBRE_PMT_TRIANGLE_LIST);
    }

    mesh->CalculateBoundingBox();
    model = NBRE_NEW NBRE_Model(NBRE_MeshPtr(mesh));
    nbre_assert(model->GetSubModelNumber() == trianglesCount);
    for (i = 0; i < trianglesCount; i++)
    {   
        NBRE_SubModel& subModel = model->GetSubModel(i);
        nbre_assert(subMeshTextureIndices[i] < shaders.size());
        subModel.SetShader(shaders[subMeshTextureIndices[i]]);
    }
    NBRE_DELETE_ARRAY subMeshTextureIndices;
    return model;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ModelGeneratorReadGeometryFromFile, Error! err = 0x%x", err);
    NBRE_DELETE_ARRAY subMeshTextureIndices;
    NBRE_DELETE decalration;
    NBRE_DELETE posElem1;
    NBRE_DELETE posBuff;
    NBRE_DELETE normalElem1;
    NBRE_DELETE normalBuff;

    decalration = NULL;
    posBuff = NULL;
    normalBuff = NULL;
    NBRE_DELETE vertexData;

    return NULL;
}

static NBRE_IndexData*
ModelGeneratorReadTriangleFromFile(NBRE_IRenderPal& renderer, NBRE_IOStream* stream, uint32* textureIndex)
{
    //NOTE: just support little-endian data
    uint32 vertexIndicesCount = 0;
    NBGM_IndicesTypeSize vertexIndiceSize = NBGM_ITS_UnsignedShortSize;
    uint32 textureImageIndex = 0;
    uint32 readSize = 0;
    uint32 readCount = 0;
    PAL_Error err = PAL_Ok;
    NBRE_HardwareIndexBuffer::IndexType indexType;
    NBRE_HardwareIndexBuffer* indexBuffer = NULL;
    NBRE_IndexData* result = NULL;

    err = stream->Read((uint8*)&textureImageIndex, sizeof(textureImageIndex), &readCount);
    err = (err != PAL_Ok) ? err : stream->Read((uint8*)&vertexIndicesCount, sizeof(vertexIndicesCount), &readCount);
    err = (err != PAL_Ok) ? err : stream->Read((uint8*)&vertexIndiceSize, sizeof(uint8), &readCount);

    if (err != PAL_Ok)
    {
        goto HandleError;
    }

    indexType = NBRE_HardwareIndexBuffer::IT_16BIT;
    indexBuffer = renderer.CreateIndexBuffer(indexType, vertexIndicesCount, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    if (indexBuffer == NULL)
    {
        err = PAL_ErrNoMem;
        goto HandleError;
    }
    readSize = vertexIndicesCount* vertexIndiceSize * sizeof(uint8);

    switch (vertexIndiceSize)
    {
    case NBGM_ITS_UnsignedByteSize:
        {
            uint8* index = NBRE_NEW uint8[readSize];
            err = stream->Read(index, readSize, &readCount);
            if (err != PAL_Ok)
            {
                goto HandleError;
            }
            uint16* pIndex = static_cast<uint16*>(indexBuffer->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_DISCARD));
            for(uint32 i = 0; i < readCount; ++i)
            {
                pIndex[i] = index[i];
            }
            indexBuffer->UnLock();
        }
        break;
    case NBGM_ITS_UnsignedShortSize:
        {
            uint8* pIndex = static_cast<uint8*>(indexBuffer->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_DISCARD));
            err = stream->Read(pIndex, readSize, &readCount);
            if (err != PAL_Ok)
            {
                goto HandleError;
            }
            indexBuffer->UnLock();
        }
        break;
    case NBGM_ITS_UnsignedIntSize:
    default:
        NBRE_DebugLog(PAL_LogSeverityCritical, "ModelGeneratorReadTriangleFromFile failed, vertexIndiceSize=%d", vertexIndiceSize);
        nbre_assert(FALSE);
        break;
    }

    result = NBRE_NEW NBRE_IndexData(indexBuffer, 0, indexBuffer->GetNumIndexes());
    if (result == NULL)
    {
        err = PAL_ErrNoMem;
        goto HandleError;
    }
    *textureIndex = textureImageIndex;

    return result;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "ModelGeneratorReadTriangleFromFile, Error! err = 0x%x", err);
    return NULL;
}

static PAL_Error
FloatArray3DPacked(NBRE_IOStream* stream, uint32 count, float* floatArray)
{
    //NOTE: just support little-endian data
    float x_min = 0.0f;
    float y_min = 0.0f;
    float z_min = 0.0f;
    float x_step = 0.0f;
    float y_step = 0.0f;
    float z_step = 0.0f;
    uint8 packTypeSize = 1;
    uint32 i = 0;
    uint32 readCount = 0;
    uint32 bufferSize = 0;
    PAL_Error err = PAL_Ok;

    if(floatArray == NULL)
    {
        err = PAL_ErrBadParam;
        goto HandleError;
    }

    err = stream->Read((uint8*)&x_min, sizeof(x_min), &readCount);
    err = (err != PAL_Ok) ? err : stream->Read((uint8*)&y_min, sizeof(y_min), &readCount);
    err = (err != PAL_Ok) ? err : stream->Read((uint8*)&z_min, sizeof(z_min), &readCount);
    err = (err != PAL_Ok) ? err : stream->Read((uint8*)&x_step, sizeof(x_step), &readCount);
    err = (err != PAL_Ok) ? err : stream->Read((uint8*)&y_step, sizeof(y_step), &readCount);
    err = (err != PAL_Ok) ? err : stream->Read((uint8*)&z_step, sizeof(z_step), &readCount);
    err = (err != PAL_Ok) ? err : stream->Read((uint8*)&packTypeSize, sizeof(packTypeSize), &readCount);

    if (err != PAL_Ok)
    {
        goto HandleError;
    }

    if(packTypeSize != NBGM_ITS_UnsignedByteSize && packTypeSize!=NBGM_ITS_UnsignedShortSize && packTypeSize!=NBGM_ITS_UnsignedIntSize)
    {
        err = PAL_ErrWrongFormat;
        goto HandleError;
    }

    bufferSize = sizeof(uint8) * packTypeSize;
    for (i=0; i < count; i++)
    {
        uint32 x_count = 0;
        uint32 y_count = 0;
        uint32 z_count = 0;

        err = stream->Read((uint8*)&x_count, bufferSize, &readCount);
        err = (err != PAL_Ok) ? err : stream->Read((uint8*)&y_count, bufferSize, &readCount);
        err = (err != PAL_Ok) ? err : stream->Read((uint8*)&z_count, bufferSize, &readCount);

        if (err != PAL_Ok)
        {
            goto HandleError;
        }

        floatArray[i*3 + 0] = x_min + x_step * x_count;
        floatArray[i*3 + 1] = y_min + y_step * y_count;
        floatArray[i*3 + 2] = z_min + z_step * z_count;
    }

    return err;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "FloatArray3DPacked, Error! err = 0x%x", err);
    return err;
}

static PAL_Error
FloatArray2DPacked(NBRE_IOStream* stream, uint32 count, float* floatArray)
{
    //NOTE: just support little-endian data
    float s_min = 0.0f;
    float t_min = 0.0f;
    float s_step = 0.0f;
    float t_step = 0.0f;
    uint8 packTypeSize = 0;

    uint32 i = 0;
    uint32 readCount = 0;
    uint32 bufferSize = 0;
    PAL_Error err = PAL_Ok;

    if(floatArray == NULL)
    {
        err = PAL_ErrBadParam;
        goto HandleError;
    }

    err = stream->Read((uint8*)&s_min, sizeof(s_min), &readCount);
    err = (err != PAL_Ok) ? err : stream->Read((uint8*)&t_min, sizeof(t_min), &readCount);
    err = (err != PAL_Ok) ? err : stream->Read((uint8*)&s_step, sizeof(s_step), &readCount);
    err = (err != PAL_Ok) ? err : stream->Read((uint8*)&t_step, sizeof(t_step), &readCount);
    err = (err != PAL_Ok) ? err : stream->Read((uint8*)&packTypeSize, sizeof(packTypeSize), &readCount);

    if (err != PAL_Ok)
    {
        goto HandleError;
    }

    if(packTypeSize != NBGM_ITS_UnsignedByteSize && packTypeSize!=NBGM_ITS_UnsignedShortSize && packTypeSize!=NBGM_ITS_UnsignedIntSize)
    {
        return PAL_ErrWrongFormat;
    }

    bufferSize = sizeof(uint8)*packTypeSize;
    for (i=0; i < count; i++)
    {
        uint32 s_count = 0;
        uint32 t_count = 0;

        err = stream->Read((uint8*)&s_count, bufferSize, &readCount);
        err = (err != PAL_Ok) ? err : stream->Read((uint8*)&t_count, bufferSize, &readCount);

        if (err != PAL_Ok)
        {
            goto HandleError;
        }

        floatArray[i*2 + 0] = s_min + s_step * s_count;
        floatArray[i*2 + 1] = t_min + t_step * t_count;
    }

    return err;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "FloatArray3DPacked, Error! err = 0x%x", err);
    return err;
}

static char*
CreateFileTextureName(const char* texName)
{
    return nsl_strdup(texName);
}

static NBRE_TexturePtr
CreateSceneTexture(NBRE_IRenderPal& renderPal, PAL_Instance* palInstance, char* texName)
{
    NBRE_TexturePtr texture = NBGM_BuildTextureUtility::ConstructImageTexture(renderPal, palInstance, texName, 0, FALSE, NBRE_Texture::TT_2D, TRUE);
    return texture;
}

PAL_Error
NBRE_LoadSceneFromStream(NBRE_IRenderPal& renderPal, PAL_Instance* palInstance, float scaleFactor, NBRE_IOStream* stream, const char* textureFolder, NBRE_Vector<NBRE_ModelPtr>& model)
{
    uint8 binVersion = 0;
    uint8 binFlag = 0;
    uint32 readCount = 0;
    PAL_Error err = PAL_ErrFileFailed;

    if(stream == NULL)
    {
        err = PAL_ErrBadParam;
        goto HandleError;
    }

    err = stream->Read((uint8*)&binVersion, sizeof(binVersion), &readCount);
    err = (err != PAL_Ok) ? err : stream->Read((uint8*)&binFlag, sizeof(binFlag), &readCount);
    if(err != PAL_Ok)
    {
        goto HandleError;
    }

    if(binVersion>SUPPORTED_BIN_VERSION)
    {
        err = PAL_ErrWrongFormat;
        goto HandleError;
    }

    if(binVersion == 0)
    {
        // do nothing here
    }
    else if(binVersion == 1)
    {
        err = ModelGeneratorReadSceneFromFile_v1(renderPal, palInstance, scaleFactor, stream, textureFolder, binFlag, model);
    }
    else
    {
        err = PAL_ErrNotFound;
    }

    return err;

HandleError:
    NBRE_DebugLog(PAL_LogSeverityMajor, "LoadSceneFromFile, Error! err = 0x%x", err);
    return err;

}

PAL_Error
NBGM_LoadBinFile(NBRE_IRenderPal& renderPal, PAL_Instance* palInstance, float scaleFactor, const char* path, NBRE_Vector<NBRE_ModelPtr>& model)
{
    NBRE_FileStream fs(palInstance, path, 2048);
    const char* workSpace = NBGM_GetConfig()->workSpace;
    return NBRE_LoadSceneFromStream(renderPal, palInstance, scaleFactor, &fs, workSpace, model);
}
