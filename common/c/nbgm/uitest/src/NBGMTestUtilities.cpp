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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#include "stdafx.h"
#include "nbgmTestUtilities.h"
#include "nbremath.h"
#include "palmath.h"
#include "nbrehardwareindexbuffer.h"
#include "nbrehardwarevertexbuffer.h"
#include "nbrerendercommon.h"
#include "nbgmconst.h"
#include "nbgmmapscaleutility.h"

char* GetExePath()
{
    TCHAR exeFilename[MAX_PATH];
    exeFilename[0] = '\0';
    DWORD filenameLength = GetModuleFileName(NULL, exeFilename, (sizeof(exeFilename) / sizeof(exeFilename[0])));
    if (filenameLength > 0)
    {
        // Remove the executable name from the path
        TCHAR* delimiter = wcsrchr(exeFilename, '\\');
        if (delimiter != 0)
        {
            *++delimiter = '\0';
        }
    }
    TCHAR* name = exeFilename;
    DWORD dwNum = WideCharToMultiByte(CP_ACP,NULL,name,-1,NULL,0,NULL,FALSE);
    char *psText;
    psText= NBRE_NEW char[dwNum];
    WideCharToMultiByte (CP_ACP,NULL, name,-1,psText,dwNum,NULL,FALSE);
    return psText;
}

char* GetModelPath(const char* modelName)
{
    TCHAR exeFilename[MAX_PATH];
    exeFilename[0] = '\0';
    DWORD filenameLength = GetModuleFileName(NULL, exeFilename, (sizeof(exeFilename) / sizeof(exeFilename[0])));
    if (filenameLength > 0)
    {
        // Remove the executable name from the path
        TCHAR* delimiter = wcsrchr(exeFilename, '\\');
        if (delimiter != 0)
        {
            *++delimiter = '\0';
        }
    }

    TCHAR wideFullFilename[MAX_PATH];
    wideFullFilename[0] = '\0';
    // Convert to wide-char
    wchar_t wideFilename[MAX_PATH] = {0};
    MultiByteToWideChar(CP_ACP, 0, modelName, -1, wideFilename, sizeof(wideFilename) / sizeof(wchar_t));
    wcscat(exeFilename, wideFilename);

    TCHAR* name = exeFilename+2;
    DWORD dwNum = WideCharToMultiByte(CP_ACP,NULL,name,-1,NULL,0,NULL,FALSE);
    char *psText;
    psText= NBRE_NEW char[dwNum];

    WideCharToMultiByte (CP_ACP,NULL, name,-1,psText,dwNum,NULL,FALSE);

    return psText;
}

char* StringToChar(CString& str)
{
    DWORD dwNum = WideCharToMultiByte(CP_ACP,NULL,str,-1,NULL,0,NULL,FALSE);
    char *psText;
    psText= NBRE_NEW char[dwNum];

    WideCharToMultiByte (CP_ACP,NULL, str,-1,psText,dwNum,NULL,FALSE);
    return psText;
}

float TileToMercatorX(int tx, float px, int tz) 
{
    int tScale = (1 << (tz - 1));
    return static_cast<float>(((tx + px)/tScale - 1.0f) * PI);
}

float TileToMercatorY(int ty, float py, int tz) 
{
    int tScale = (1 << (tz - 1));
    return static_cast<float>((1.0f - (ty + py)/tScale) * PI);
}

void SpatialConvertLatLongToMercator(double latitude, double longitude, double& xMercator, double& yMercator)
{
	double rlat = 0.0;
	xMercator = NBRE_Math::DegToRad(longitude);
    rlat = NBRE_Math::DegToRad(latitude);
	yMercator = nsl_log(nsl_tan(rlat)+nsl_sec(rlat));
}

CString * SplitString(CString str, char split, int& iSubStrs)
{
    int iPos = 0;
    int iNums = 0;
    CString strTemp = str;
    CString strRight;
    while (iPos != -1)
    {
        iPos = strTemp.Find(split);
        if (iPos == -1)
        {
            break;
        }
        strRight = strTemp.Mid(iPos + 1, str.GetLength());
        strTemp = strRight;
        iNums++;
    }
    if (iNums == 0)
    {
        iSubStrs = 1; 
        return NULL;
    }
    iSubStrs = iNums + 1;
    CString* pStrSplit;
    pStrSplit= NBRE_NEW CString[iSubStrs];
    strTemp = str;
    CString strLeft;
    for (int i = 0; i < iNums; i++)
    {
        iPos = strTemp.Find(split);
        strLeft = strTemp.Left(iPos);
        strRight = strTemp.Mid(iPos + 1, strTemp.GetLength());
        strTemp = strRight;
        pStrSplit[i] = strLeft;
    }
    pStrSplit[iNums] = strTemp;
    return pStrSplit;
}

NBRE_Mesh* CreatePolygon3D(NBRE_IRenderPal& renderPal, float* vertices, uint16 vertexCount)
{
    if (vertices == NULL || vertexCount < 2)
    {
        return NULL;
    }

    uint16* indices= NBRE_NEW uint16[(vertexCount - 2) * 3];

    for (uint16 i = 0; i < vertexCount - 2; ++i)
    {
        uint16 index = i * 3;

        indices[index]     = 0;
        indices[index + 1] = 1 + i;
        indices[index + 2] = 2 + i;
    }

    NBRE_HardwareVertexBuffer* vertextBuff = renderPal.CreateVertexBuffer(sizeof(float) * 3, vertexCount, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    vertextBuff->WriteData(0, sizeof(float) * 3 * vertexCount, vertices, FALSE);

    NBRE_VertexDeclaration* decalration = renderPal.CreateVertexDeclaration();
    NBRE_VertexElement* posElem1= NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem1);

    NBRE_VertexData* vertexData= NBRE_NEW NBRE_VertexData(0);
    vertexData->AddBuffer(vertextBuff);
    vertexData->SetVertexDeclaration(decalration);

    NBRE_Mesh* mesh= NBRE_NEW NBRE_Mesh();

    NBRE_HardwareIndexBuffer* indexBuff = renderPal.CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, (vertexCount - 2) * 3, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    indexBuff->WriteData(0, (vertexCount - 2) * 3 * sizeof(uint16), indices, FALSE);
    NBRE_IndexData* indexData= NBRE_NEW NBRE_IndexData(indexBuff, 0, indexBuff->GetNumIndexes());

    mesh->CreateSubMesh(vertexData, indexData, NBRE_PMT_TRIANGLE_LIST);
    mesh->CalculateBoundingBox();

    NBRE_DELETE[] indices;

    return mesh;
}


NBRE_Mesh* CreateGround(NBRE_IRenderPal& renderPal, float width, int32 count, float r, float g, float b)
{

    int32 vertexCount = (count+1)*2*2;

    NBRE_HardwareVertexBuffer* vertextBuff = renderPal.CreateVertexBuffer(sizeof(float) * 2, vertexCount, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    NBRE_HardwareVertexBuffer* clrBuff = renderPal.CreateVertexBuffer(sizeof(float) * 3, vertexCount, FALSE, NBRE_HardwareBuffer::HBU_STATIC);

    float* vertex = (float*)vertextBuff->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY);
    float* colors = (float*)clrBuff->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY);

    float halfWholeWidth = width*count/2.f;
    for(int32 i = 0; i < count+1; ++i)
    {
        vertex[4*i + 0] = -halfWholeWidth;
        vertex[4*i + 1] = -halfWholeWidth + width*i;

        vertex[4*i + 2] = halfWholeWidth;
        vertex[4*i + 3] = -halfWholeWidth + width*i;
    }

    for(int32 i = 0; i < count+1; ++i)
    {
        vertex[4*(i+count+1) + 0] = -halfWholeWidth + width*i;
        vertex[4*(i+count+1) + 1] = halfWholeWidth;

        vertex[4*(i+count+1) + 2] = -halfWholeWidth + width*i;
        vertex[4*(i+count+1) + 3] = -halfWholeWidth;
    }

    for (int32 i = 0; i < vertexCount; ++i)
    {
        colors[3*i + 0] = r;
        colors[3*i + 1] = g;
        colors[3*i + 2] = b;
    }

    vertextBuff->UnLock();
    clrBuff->UnLock();

    NBRE_VertexDeclaration* decalration = renderPal.CreateVertexDeclaration();
    NBRE_VertexElement* posElem1= NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_POSITION, 0);
    NBRE_VertexElement* posElem2= NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_DIFFUSE, 0);

    decalration->GetVertexElementList().push_back(posElem1);
    decalration->GetVertexElementList().push_back(posElem2);


    NBRE_VertexData* vertexData = NBRE_NEW NBRE_VertexData(0);
    vertexData->AddBuffer(vertextBuff);
    vertexData->AddBuffer(clrBuff);

    vertexData->SetVertexDeclaration(decalration);

    NBRE_Mesh* mesh = NBRE_NEW NBRE_Mesh();

    NBRE_IndexData* indexData= NBRE_NEW NBRE_IndexData(NULL, 0, vertexCount);

    mesh->CreateSubMesh(vertexData, indexData, NBRE_PMT_LINE_LIST);
    mesh->CalculateBoundingBox();


    return mesh;
}

NBRE_Mesh* CreateRegularPrism(NBRE_IRenderPal& renderPal, uint16 edgeNum, float edgeHeight, float radius)
{
    if (edgeNum < 2 || edgeHeight < 0.0f || radius < 0.0f)
    {
        return NULL;
    }

    uint32 vertexCount = edgeNum * 2;
    uint32 indexCount = ((edgeNum - 3) * 3 + 3) * 2 + edgeNum * 6;
    uint32 index = 0;
    uint32 normalIndex = 0;
    uint16 step = 0;
    NBRE_Vector3f vTopPlane(0.0f, 0.0f, 1.0f);
    NBRE_Vector3f vBottomPlane(0.0f, 0.0f, -1.0f);
    NBRE_Vector3f v1;
    NBRE_Vector3f v2;
    NBRE_Vector3f v3;
    NBRE_Vector3f v4;

    typedef NBRE_Vector<NBRE_Vector3f*> NBRE_PointList;

    float* vertices= NBRE_NEW float[vertexCount * 3];
    float* normals= NBRE_NEW float[vertexCount * 3];
    NBRE_PointList pointList;

    for (uint32 i = 0; i < edgeNum; ++i)
    {
        float radian = static_cast<float>(i * TWO_PI / edgeNum );

        vertices[index++] = radius * nsl_cos(radian);
        vertices[index++] = edgeHeight * 0.5f; 
        vertices[index++] = radius * nsl_sin(radian);

        vertices[index++] = radius * nsl_cos(radian);
        vertices[index++] = -edgeHeight * 0.5f; 
        vertices[index++] = radius * nsl_sin(radian);
    }

    pointList.push_back(new NBRE_Vector3f(vertices[vertexCount * 3 - 3], vertices[vertexCount * 3 - 2], vertices[vertexCount * 3 - 1]));
    index = 0;
    while(index < vertexCount * 3)
    {
        pointList.push_back(new NBRE_Vector3f(vertices[index], vertices[index+1], vertices[index+2]));
        index += 3;
    }
    pointList.push_back(new NBRE_Vector3f(vertices[0], vertices[1], vertices[2]));
    pointList.push_back(new NBRE_Vector3f(vertices[3], vertices[4], vertices[5]));

    index = 1;
    for (uint32 i = 0; i < edgeNum; ++i)
    {
        v1 = *pointList[index + 1] - *pointList[index];
        v2 = *pointList[index + 3] - *pointList[index];
        v1 = v1.CrossProduct(v2);

        v2 = *pointList[index + 1] - *pointList[index];
        v3 = *pointList[index - 1] - *pointList[index];
        v3 = v3.CrossProduct(v2);

        v1 += v3;
        v4 = v1;

        v1 += vTopPlane;
        v1.Normalise();

        v4 += vBottomPlane;
        v4.Normalise();

        normals[normalIndex++] = v1.x;
        normals[normalIndex++] = v1.y;
        normals[normalIndex++] = v1.z;

        normals[normalIndex++] = v4.x;
        normals[normalIndex++] = v4.y;
        normals[normalIndex++] = v4.z;

        index += 2;
    }

    uint16* bottomSideIndices= NBRE_NEW uint16[(edgeNum - 3) * 3 + 3];
    index = 0;
    for (uint16 i = 0; i < edgeNum - 2; ++i)
    {
        uint16 step = i * 2;

        bottomSideIndices[index++] = 1;
        bottomSideIndices[index++] = 5 + step;
        bottomSideIndices[index++] = 3 + step;
    }

    uint16* sideIndices= NBRE_NEW uint16[edgeNum * 6];
    index = 0;
    for (uint16 i = 0; i < edgeNum - 1; ++i)
    {
        step = i * 2;

        sideIndices[index++] = 0 + step;
        sideIndices[index++] = 1 + step;
        sideIndices[index++] = 3 + step;

        sideIndices[index++] = 0 + step;
        sideIndices[index++] = 3 + step;
        sideIndices[index++] = 2 + step;
    }
    sideIndices[index++] = 2 + step;
    sideIndices[index++] = 3 + step;
    sideIndices[index++] = 1;

    sideIndices[index++] = 2 + step;
    sideIndices[index++] = 1;
    sideIndices[index++] = 0;

    uint16* topSideIndices= NBRE_NEW uint16[(edgeNum - 3) * 3 + 3];
    index = 0;
    for (uint16 i = 0; i < edgeNum - 2; ++i)
    {
        uint16 step = i * 2;

        topSideIndices[index++]     = 0;
        topSideIndices[index++] = 2 + step;
        topSideIndices[index++] = 4 + step;
    }

    NBRE_HardwareVertexBuffer* vertextBuff = renderPal.CreateVertexBuffer(sizeof(float) * 3, vertexCount, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    vertextBuff->WriteData(0, vertexCount * 3 * sizeof(float), vertices, FALSE);

    NBRE_HardwareVertexBuffer* normalBuff = renderPal.CreateVertexBuffer(sizeof(float) * 3, vertexCount, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    normalBuff->WriteData(0, vertexCount * 3 * sizeof(float), normals, FALSE);

    NBRE_VertexDeclaration* decalration = renderPal.CreateVertexDeclaration();
    NBRE_VertexElement* posElem1= NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem1);
    NBRE_VertexElement* normalElem= NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_NORMAL, 0);
    decalration->GetVertexElementList().push_back(normalElem);

    NBRE_VertexData* vertexData= NBRE_NEW NBRE_VertexData(0);
    vertexData->AddBuffer(vertextBuff);
    vertexData->AddBuffer(normalBuff);
    vertexData->SetVertexDeclaration(decalration);


    NBRE_Mesh* mesh= NBRE_NEW NBRE_Mesh();

    NBRE_HardwareIndexBuffer* indexBuff = renderPal.CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, indexCount, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    uint32 offset = 0;

    indexBuff->WriteData(0, ((edgeNum - 3) * 3 + 3) * sizeof(uint16), bottomSideIndices, FALSE);
    offset = ((edgeNum - 3) * 3 + 3) * sizeof(uint16);

    indexBuff->WriteData(offset, edgeNum * 6 * sizeof(uint16), sideIndices, FALSE);
    offset += edgeNum * 6 * sizeof(uint16);

    indexBuff->WriteData(offset, ((edgeNum - 3) * 3 + 3) * sizeof(uint16), topSideIndices, FALSE);

    NBRE_IndexData* indexData= NBRE_NEW NBRE_IndexData(indexBuff, 0, indexBuff->GetNumIndexes());

    mesh->CreateSubMesh(vertexData, indexData, NBRE_PMT_TRIANGLE_LIST);
    mesh->CalculateBoundingBox();

    NBRE_DELETE[] vertices;
    NBRE_DELETE[] normals;
    NBRE_DELETE[] topSideIndices;
    NBRE_DELETE[] bottomSideIndices;
    NBRE_DELETE[] sideIndices;

    NBRE_PointList::const_iterator iter;
    for (iter = pointList.begin(); iter != pointList.end(); ++iter)
    {
        NBRE_DELETE *iter;
    }

    return mesh;
}

NBRE_Mesh* CreateCube(NBRE_IRenderPal& renderPal, float length, float width, float height)
{
    float vertices[] = 
    {
        -length, -width, -height,
         length, -width, -height,
        -length,  width, -height,
         length,  width, -height,

        -length, -width,  height,
         length, -width,  height,
        -length,  width,  height,
         length,  width,  height,

    };

    float texcord[] = 
    {
        -length, -width, -height,
         length, -width, -height,
        -length,  width, -height,
         length,  width, -height,

        -length, -width,  height,
         length, -width,  height,
        -length,  width,  height,
         length,  width,  height,

    };


    static uint16 indices[] = 
    {
        0, 2, 1,
        1, 2, 3, 

        4, 5, 6,
        5, 7, 6, 

        0, 1, 4,
        1, 5, 4, 

        1, 3, 5,
        3, 7, 5, 

        2, 6, 3,
        3, 6, 7, 

        0, 4, 2,
        2, 4, 6, 
    };
    int32 vertexCount = sizeof(vertices)/(sizeof(float) * 3);
    int32 indexCount = sizeof(indices)/(sizeof(uint16));


    NBRE_HardwareVertexBuffer* vertextBuff = renderPal.CreateVertexBuffer(sizeof(float) * 3, vertexCount, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    vertextBuff->WriteData(0, sizeof(float) * 3 * vertexCount, vertices, FALSE);

    NBRE_HardwareVertexBuffer* texBuff = renderPal.CreateVertexBuffer(sizeof(float) * 3, vertexCount, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    texBuff->WriteData(0, sizeof(float) * 3 * vertexCount, texcord, FALSE);

    NBRE_VertexDeclaration* decalration = renderPal.CreateVertexDeclaration();
    NBRE_VertexElement* posElem1= NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem1);
    NBRE_VertexElement* texcoordElem1= NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
    decalration->GetVertexElementList().push_back(texcoordElem1);

    NBRE_VertexData* vertexData= NBRE_NEW NBRE_VertexData(0);
    vertexData->AddBuffer(vertextBuff);
    vertexData->AddBuffer(texBuff);
    vertexData->SetVertexDeclaration(decalration);

    NBRE_Mesh* mesh= NBRE_NEW NBRE_Mesh();

    NBRE_HardwareIndexBuffer* indexBuff = renderPal.CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, indexCount, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    indexBuff->WriteData(0, sizeof(indices), indices, FALSE);
    NBRE_IndexData* indexData= NBRE_NEW NBRE_IndexData(indexBuff, 0, indexBuff->GetNumIndexes());

    mesh->CreateSubMesh(vertexData, indexData, NBRE_PMT_TRIANGLE_LIST);
    mesh->CalculateBoundingBox();


    return mesh;
}


NBRE_Mesh* CreateSphere(NBRE_IRenderPal& renderPal, float radius)
{
    if (radius < 0.0f)
    {
        return NULL;
    }

    uint32 index = 0;
    int32 stepV = 10;
    int32 stepH = 10;
    uint16 verticesCountH = static_cast<uint16>(360 / stepH);
    uint16 CircleCount = static_cast<uint16>(180 / stepH + 1);
    uint16 vertexCount = 2 + (CircleCount - 2) * verticesCountH;
    uint16 indexCount = verticesCountH * 3 * 2 + (CircleCount - 3) * (verticesCountH * 6);

    float* vertices= NBRE_NEW float[vertexCount * (3 * 2 + 2)];
    NBRE_Vector3f vector;

    vector.x = vertices[index++] = 0.0f;
    vector.y = vertices[index++] = 0.0f;
    vector.z = vertices[index++] = -radius;

    vector.Normalise();
    vertices[index++] = vector.x;
    vertices[index++] = vector.y;
    vertices[index++] = vector.z;

    vertices[index++] = 0;
    vertices[index++] = 0;

    for(int32 vAngle=-90 + stepV; vAngle<=90 - stepV; vAngle=vAngle+stepV)
    {
        for(int32 hAngle=0; hAngle<360; hAngle=hAngle+stepH)
        {
            float xozRadius = radius * nsl_cos(NBRE_Math::DegToRad(static_cast<float>(vAngle)));

            float x = static_cast<float>(xozRadius * nsl_cos(NBRE_Math::DegToRad(static_cast<float>(hAngle))));
            float y = static_cast<float>(xozRadius * nsl_sin(NBRE_Math::DegToRad(static_cast<float>(hAngle))));
            float z = static_cast<float>(radius * nsl_sin(NBRE_Math::DegToRad(static_cast<float>(vAngle))));

            vector.x = vertices[index++] = x;
            vector.y = vertices[index++] = y;
            vector.z = vertices[index++] = z;

            vector.Normalise();
            vertices[index++] = vector.x;
            vertices[index++] = vector.y;
            vertices[index++] = vector.z;

            // Texcood
            vertices[index++] = hAngle / 360.0f;
            vertices[index++] = (vAngle + 90) / 180.0f;
        }
    }

    vector.x = vertices[index++] = 0.0f;
    vector.y = vertices[index++] = 0.0f;
    vector.z = vertices[index++] = radius;

    vector.Normalise();
    vertices[index++] = vector.x;
    vertices[index++] = vector.y;
    vertices[index++] = vector.z;

    vertices[index++] = 1.0f;
    vertices[index++] = 1.0f;

    index = 0;
    uint16* indices= NBRE_NEW uint16[indexCount];
    for (uint16 i = 0; i < verticesCountH - 1; ++i)
    {
        indices[index++] = 0;
        indices[index++] = 2 + i;
        indices[index++] = 1 + i;
    }
    indices[index++] = 0;
    indices[index++] = 1;
    indices[index++] = verticesCountH;

    for (uint16 i = 0; i < CircleCount - 3; ++i)
    {
        uint16 vertexIndex_1 = 1 + verticesCountH * i;
        uint16 vertexIndex_2 = 1 + verticesCountH * (i + 1);
        for (uint16 i = 0; i < verticesCountH - 1; ++i)
        {
            indices[index++] = vertexIndex_1 + i;
            indices[index++] = vertexIndex_1 + i + 1;
            indices[index++] = vertexIndex_2 + i;

            indices[index++] = vertexIndex_1 + i + 1;
            indices[index++] = vertexIndex_2 + i + 1;
            indices[index++] = vertexIndex_2 + i;
        }
        indices[index++] = vertexIndex_1 + verticesCountH - 1;
        indices[index++] = vertexIndex_1;
        indices[index++] = vertexIndex_2 + verticesCountH - 1;

        indices[index++] = vertexIndex_1;
        indices[index++] = vertexIndex_2;
        indices[index++] = vertexIndex_2 + verticesCountH - 1;
    }

    for (uint16 i = 0; i < verticesCountH - 1; ++i)
    {
        uint16 vertexIndex_1 = 1 + verticesCountH * (CircleCount - 3);
        indices[index++] = vertexIndex_1 + i;
        indices[index++] = vertexIndex_1 + i + 1;
        indices[index++] = vertexCount - 1;
    }
    indices[index++] = 1 + verticesCountH * (CircleCount - 3) + verticesCountH - 1;
    indices[index++] = 1 + verticesCountH * (CircleCount - 3);
    indices[index++] = vertexCount - 1;

    NBRE_HardwareVertexBuffer* vertextBuff = renderPal.CreateVertexBuffer(sizeof(float) * (3 * 2 + 2), vertexCount, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    vertextBuff->WriteData(0, vertexCount * (3 * 2 + 2) * sizeof(float), vertices, FALSE);

    NBRE_VertexDeclaration* decalration = renderPal.CreateVertexDeclaration();
    NBRE_VertexElement* posElem1= NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem1);
    NBRE_VertexElement* normalElem1= NBRE_NEW NBRE_VertexElement(0, 3*sizeof(float), NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_NORMAL, 0);
    decalration->GetVertexElementList().push_back(normalElem1);
    NBRE_VertexElement* texcoordElem1= NBRE_NEW NBRE_VertexElement(0, 3*2*sizeof(float), NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
    decalration->GetVertexElementList().push_back(texcoordElem1);

    NBRE_VertexData* vertexData= NBRE_NEW NBRE_VertexData(0);
    vertexData->AddBuffer(vertextBuff);
    vertexData->SetVertexDeclaration(decalration);

    NBRE_HardwareIndexBuffer* indexBuff = renderPal.CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, indexCount, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    indexBuff->WriteData(0, indexCount * sizeof(uint16), indices, FALSE);
    NBRE_IndexData* indexData= NBRE_NEW NBRE_IndexData(indexBuff, 0, indexBuff->GetNumIndexes());

    NBRE_Mesh* mesh= NBRE_NEW NBRE_Mesh();
    mesh->CreateSubMesh(vertexData, indexData, NBRE_PMT_TRIANGLE_LIST);
    mesh->CalculateBoundingBox();

    NBRE_DELETE[] vertices;
    NBRE_DELETE[] indices;

    return mesh;
}

NBRE_Mesh* Create3DTextureSphere(NBRE_IRenderPal& renderPal, float radius)
{
    if (radius < 0.0f)
    {
        return NULL;
    }

    uint32 index = 0;
    int32 stepV = 10;
    int32 stepH = 10;
    uint16 verticesCountH = static_cast<uint16>(360 / stepH);
    uint16 CircleCount = static_cast<uint16>(180 / stepH + 1);
    uint16 vertexCount = 2 + (CircleCount - 2) * verticesCountH;
    uint16 indexCount = verticesCountH * 3 * 2 + (CircleCount - 3) * (verticesCountH * 6);

    float* vertices= NBRE_NEW float[vertexCount * (3 * 2)];
    NBRE_Vector3f vector;

    vector.x = vertices[index++] = 0.0f;
    vector.y = vertices[index++] = 0.0f;
    vector.z = vertices[index++] = -radius;

    vector.Normalise();
    vertices[index++] = vector.x;
    vertices[index++] = vector.y;
    vertices[index++] = vector.z;


    for(int32 vAngle=-90 + stepV; vAngle<=90 - stepV; vAngle=vAngle+stepV)
    {
        for(int32 hAngle=0; hAngle<360; hAngle=hAngle+stepH)
        {
            float xozRadius = radius * nsl_cos(NBRE_Math::DegToRad(static_cast<float>(vAngle)));

            float x = static_cast<float>(xozRadius * nsl_cos(NBRE_Math::DegToRad(static_cast<float>(hAngle))));
            float y = static_cast<float>(xozRadius * nsl_sin(NBRE_Math::DegToRad(static_cast<float>(hAngle))));
            float z = static_cast<float>(radius * nsl_sin(NBRE_Math::DegToRad(static_cast<float>(vAngle))));

            vector.x = vertices[index++] = x;
            vector.y = vertices[index++] = y;
            vector.z = vertices[index++] = z;

            vector.Normalise();
            vertices[index++] = vector.x;
            vertices[index++] = vector.y;
            vertices[index++] = vector.z;

        }
    }

    vector.x = vertices[index++] = 0.0f;
    vector.y = vertices[index++] = 0.0f;
    vector.z = vertices[index++] = radius;

    vector.Normalise();
    vertices[index++] = vector.x;
    vertices[index++] = vector.y;
    vertices[index++] = vector.z;


    index = 0;
    uint16* indices= NBRE_NEW uint16[indexCount];
    for (uint16 i = 0; i < verticesCountH - 1; ++i)
    {
        indices[index++] = 0;
        indices[index++] = 2 + i;
        indices[index++] = 1 + i;
    }
    indices[index++] = 0;
    indices[index++] = 1;
    indices[index++] = verticesCountH;

    for (uint16 i = 0; i < CircleCount - 3; ++i)
    {
        uint16 vertexIndex_1 = 1 + verticesCountH * i;
        uint16 vertexIndex_2 = 1 + verticesCountH * (i + 1);
        for (uint16 i = 0; i < verticesCountH - 1; ++i)
        {
            indices[index++] = vertexIndex_1 + i;
            indices[index++] = vertexIndex_1 + i + 1;
            indices[index++] = vertexIndex_2 + i;

            indices[index++] = vertexIndex_1 + i + 1;
            indices[index++] = vertexIndex_2 + i + 1;
            indices[index++] = vertexIndex_2 + i;
        }
        indices[index++] = vertexIndex_1 + verticesCountH - 1;
        indices[index++] = vertexIndex_1;
        indices[index++] = vertexIndex_2 + verticesCountH - 1;

        indices[index++] = vertexIndex_1;
        indices[index++] = vertexIndex_2;
        indices[index++] = vertexIndex_2 + verticesCountH - 1;
    }

    for (uint16 i = 0; i < verticesCountH - 1; ++i)
    {
        uint16 vertexIndex_1 = 1 + verticesCountH * (CircleCount - 3);
        indices[index++] = vertexIndex_1 + i;
        indices[index++] = vertexIndex_1 + i + 1;
        indices[index++] = vertexCount - 1;
    }
    indices[index++] = 1 + verticesCountH * (CircleCount - 3) + verticesCountH - 1;
    indices[index++] = 1 + verticesCountH * (CircleCount - 3);
    indices[index++] = vertexCount - 1;

    NBRE_HardwareVertexBuffer* vertextBuff = renderPal.CreateVertexBuffer(sizeof(float) * (3 * 2), vertexCount, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    vertextBuff->WriteData(0, vertexCount * (3 * 2) * sizeof(float), vertices, FALSE);

    NBRE_VertexDeclaration* decalration = renderPal.CreateVertexDeclaration();
    NBRE_VertexElement* posElem1= NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem1);
    NBRE_VertexElement* normalElem1= NBRE_NEW NBRE_VertexElement(0, 3*sizeof(float), NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_NORMAL, 0);
    decalration->GetVertexElementList().push_back(normalElem1);
    NBRE_VertexElement* texcoordElem1= NBRE_NEW NBRE_VertexElement(0, 3*sizeof(float), NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
    decalration->GetVertexElementList().push_back(texcoordElem1);

    NBRE_VertexData* vertexData= NBRE_NEW NBRE_VertexData(0);
    vertexData->AddBuffer(vertextBuff);
    vertexData->SetVertexDeclaration(decalration);

    NBRE_HardwareIndexBuffer* indexBuff = renderPal.CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, indexCount, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    indexBuff->WriteData(0, indexCount * sizeof(uint16), indices, FALSE);
    NBRE_IndexData* indexData= NBRE_NEW NBRE_IndexData(indexBuff, 0, indexBuff->GetNumIndexes());

    NBRE_Mesh* mesh= NBRE_NEW NBRE_Mesh();
    mesh->CreateSubMesh(vertexData, indexData, NBRE_PMT_TRIANGLE_LIST);
    mesh->CalculateBoundingBox();

    NBRE_DELETE[] vertices;
    NBRE_DELETE[] indices;

    return mesh;
}



static PAL_Instance* gPal = NULL;

PAL_Instance* GetPal()
{
    if(gPal == NULL)
    {
        PAL_Config pc = {0};
        pc.multiThreaded = TRUE;
        gPal = PAL_Create(&pc);
    }
    return gPal;
}

void DestoryPal()
{
    if(gPal != NULL)
    {
        PAL_Destroy(gPal);
        gPal = NULL;
    }
}

void InitializeNBGM()
{
    char* path = GetExePath();

    NBGM_Config config = {0};
    config.pal = GetPal();
    config.workSpace = path;
    config.nbmOffset = 0;
    config.dpi = 320.f;
    config.useRoadBuildingVisibility = TRUE;
    config.relativeCameraDistance = (float)NBGM_MapScaleUtility::GetRelativeCameraDistance(); // camera height on zoom level 0, this value comes from Map Layering - Mapkit3D.xlsx.
    config.relativeZoomLevel = (int8)NBGM_MapScaleUtility::GetRelativeZoomLevel();
    config.metersPerPixelOnRelativeZoomLevel = (float)NBGM_MapScaleUtility::MetersPerPixelOnRelativeZoomLevel();
    NBGM_Initialize(&config);
    NBGM_SetDebugLogSeverity(PAL_LogSeverityDebug);
    NBRE_DELETE_ARRAY path;
}

NBRE_RenderEngine* CreateRenderEngine(NBRE_IRenderPal* renderer)
{
    NBRE_RenderEngine* eng = NBRE_NEW NBRE_RenderEngine(GetPal());
    char* path = GetExePath();
    NBRE_RenderConfig config;
    config.fontFolderPath = NBRE_String(NBGM_GetConfig()->workSpace);
    eng->Initialize(renderer, config);
    NBRE_DELETE[] path;
    return eng;
}
