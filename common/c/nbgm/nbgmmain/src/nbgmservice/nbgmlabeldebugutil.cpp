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
#include "nbgmlabeldebugutil.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbrerenderpal.h"
#include "nbrehardwarebuffer.h"
#include "nbreintersection.h"
#include "nbremath.h"
#include "nbretypeconvert.h"
#include "nbreentity.h"
#include "nbrevertexindexdata.h"
#include "nbrepolyline2.h"

struct PolylineEntityVertex
{
    NBRE_Vector3f position;
    NBRE_Color color;
};

NBRE_Mesh*
CreateDebugPolylineEntity(NBRE_Context mContext, const NBRE_Polyline2f& mPolyline, const NBRE_Color& mColor)
{
    NBRE_VertexData* mVertexData = NULL;
    

    // Create vertex buffer
    NBRE_HardwareVertexBuffer* vertextBuff = mContext.mRenderPal->CreateVertexBuffer(sizeof(float) * (3 + 4), mPolyline.VertexCount(), FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    NBRE_VertexDeclaration* decalration = mContext.mRenderPal->CreateVertexDeclaration();
    NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem);
    NBRE_VertexElement* colorElem = NBRE_NEW NBRE_VertexElement(0, (3) * sizeof(float), NBRE_VertexElement::VET_FLOAT4, NBRE_VertexElement::VES_DIFFUSE, 0);
    decalration->GetVertexElementList().push_back(colorElem);

    mVertexData = NBRE_NEW NBRE_VertexData(0);
    mVertexData->AddBuffer(vertextBuff);
    mVertexData->SetVertexDeclaration(decalration);

    // Create index buffer
    NBRE_HardwareIndexBuffer* indexBuff = mContext.mRenderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, mPolyline.VertexCount(), FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    NBRE_IndexData* mIndexData = NBRE_NEW NBRE_IndexData(indexBuff, 0, indexBuff->GetNumIndexes());

    NBRE_HardwareIndexBuffer* indexBuffEndPoints = mContext.mRenderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, 2, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    NBRE_IndexData* mIndexDataEndPoints = NBRE_NEW NBRE_IndexData(indexBuffEndPoints, 0, indexBuffEndPoints->GetNumIndexes());

    // Update buffer data
    PolylineEntityVertex* pVertex = static_cast<PolylineEntityVertex*>(mVertexData->GetBuffer(0)->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));
    uint16* pIndex = static_cast<uint16*>(mIndexData->IndexBuffer()->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));
    uint16* pIndexEp = static_cast<uint16*>(mIndexDataEndPoints->IndexBuffer()->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));
    uint16 iEp = 0;

    for (uint16 i = 0; i < mPolyline.VertexCount(); ++i)
    {
        const NBRE_Vector2f& v2 = mPolyline.Vertex(i);
        pVertex[i].position.x = v2.x;
        pVertex[i].position.y = v2.y;
        pVertex[i].position.z = mPolyline.Length() * 0.02f;
        pVertex[i].color = mColor;
        pIndex[i] = i;
        if (i == 0 || i == mPolyline.VertexCount() - 1)
        {
            pIndexEp[iEp++] = i;
        }
    }

    vertextBuff->UnLock();
    indexBuff->UnLock();

    NBRE_Mesh* mesh = NBRE_NEW NBRE_Mesh(mVertexData);
    mesh->CreateSubMesh(mIndexData, NBRE_PMT_LINE_STRIP);
    mesh->CreateSubMesh(mIndexDataEndPoints, NBRE_PMT_POINT_LIST);
    mesh->CalculateBoundingBox();
    return mesh;
}

NBRE_Mesh*
CreateDebugRectEntity(NBRE_Context mContext, float x, float y, float sizeX, float sizeY, const NBRE_Color& mColor)
{
    // Create vertex buffer
    NBRE_VertexData* mVertexData = NBRE_NEW NBRE_VertexData(0);

    NBRE_VertexDeclaration* decalration = mContext.mRenderPal->CreateVertexDeclaration();
    decalration->GetVertexElementList().push_back(NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0));
    decalration->GetVertexElementList().push_back(NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT4, NBRE_VertexElement::VES_DIFFUSE, 0));
    mVertexData->SetVertexDeclaration(decalration);

    mVertexData->AddBuffer(mContext.mRenderPal->CreateVertexBuffer(sizeof(float) * 3, 4, FALSE, NBRE_HardwareBuffer::HBU_STATIC));
    mVertexData->AddBuffer(mContext.mRenderPal->CreateVertexBuffer(sizeof(float) * 4, 4, FALSE, NBRE_HardwareBuffer::HBU_STATIC));

    // Create index buffer
    NBRE_HardwareIndexBuffer* indexBuff = mContext.mRenderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, 5, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    NBRE_IndexData* mIndexData = NBRE_NEW NBRE_IndexData(indexBuff, 0, indexBuff->GetNumIndexes());

    // Update buffer data
    float* pVertex = static_cast<float*>(mVertexData->GetBuffer(0)->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));
    NBRE_Color* pColor = static_cast<NBRE_Color*>(mVertexData->GetBuffer(1)->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));
    uint16* pIndex = static_cast<uint16*>(mIndexData->IndexBuffer()->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));

    NBRE_Vector3f center(x, y, 0);
    NBRE_Vector3f half(sizeX * 0.5f, sizeY * 0.5f, 0);

    *pVertex++ = center.x - half.x;
    *pVertex++ = center.y - half.y;
    *pVertex++ = 0;
    
    *pVertex++ = center.x + half.x;
    *pVertex++ = center.y - half.y;
    *pVertex++ = 0;

    *pVertex++ = center.x + half.x;
    *pVertex++ = center.y + half.y;
    *pVertex++ = 0;

    *pVertex++ = center.x - half.x;
    *pVertex++ = center.y + half.y;
    *pVertex++ = 0;

    pColor[0] = mColor;
    pColor[1] = mColor;
    pColor[2] = mColor;
    pColor[3] = mColor;

    pIndex[0] = 0;
    pIndex[1] = 1;
    pIndex[2] = 2;
    pIndex[3] = 3;
    pIndex[4] = 0;

    mVertexData->GetBuffer(0)->UnLock();
    mVertexData->GetBuffer(1)->UnLock();
    indexBuff->UnLock();

    NBRE_Mesh* mesh = NBRE_NEW NBRE_Mesh(mVertexData);
    mesh->CreateSubMesh(mIndexData, NBRE_PMT_LINE_STRIP);
    mesh->CalculateBoundingBox();
    return mesh;
}

NBRE_Mesh*
CreateDebugPointEntity(NBRE_Context mContext, float x, float y)
{
    NBRE_Color mColor(1, 0, 0, 1);
    NBRE_VertexData* mVertexData = NULL;

    // Create vertex buffer
    NBRE_HardwareVertexBuffer* vertextBuff = mContext.mRenderPal->CreateVertexBuffer(sizeof(float) * (3 + 4), 1, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    NBRE_VertexDeclaration* decalration = mContext.mRenderPal->CreateVertexDeclaration();
    NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem);
    NBRE_VertexElement* colorElem = NBRE_NEW NBRE_VertexElement(0, (3) * sizeof(float), NBRE_VertexElement::VET_FLOAT4, NBRE_VertexElement::VES_DIFFUSE, 0);
    decalration->GetVertexElementList().push_back(colorElem);

    mVertexData = NBRE_NEW NBRE_VertexData(0);
    mVertexData->AddBuffer(vertextBuff);
    mVertexData->SetVertexDeclaration(decalration);

    // Create index buffer
    NBRE_HardwareIndexBuffer* indexBuff = mContext.mRenderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, 1, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    NBRE_IndexData* mIndexData = NBRE_NEW NBRE_IndexData(indexBuff, 0, indexBuff->GetNumIndexes());

    // Update buffer data
    PolylineEntityVertex* pVertex = static_cast<PolylineEntityVertex*>(mVertexData->GetBuffer(0)->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));
    uint16* pIndex = static_cast<uint16*>(mIndexData->IndexBuffer()->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));

    NBRE_Vector3f center(x, y, 0);

    pVertex[0].position.x = x;
    pVertex[0].position.y = y;
    pVertex[0].position.z = 0;
    pVertex[0].color = mColor;

    pIndex[0] = 0;

    vertextBuff->UnLock();
    indexBuff->UnLock();

    NBRE_PassPtr mPassCache(NBRE_NEW NBRE_Pass());
    mPassCache->SetEnableDepthTest(TRUE);
    mPassCache->SetEnableDepthWrite(TRUE);
    mPassCache->SetEnableBlend(TRUE);
    mPassCache->SetLineWidth(3.0f);
    NBRE_ShaderPtr shader(NBRE_NEW NBRE_Shader());
    shader->AddPass(mPassCache);

    NBRE_Mesh* mesh = NBRE_NEW NBRE_Mesh(mVertexData);
    mesh->CreateSubMesh(mIndexData, NBRE_PMT_POINT_LIST);
    mesh->CalculateBoundingBox();
    return mesh;
}

NBRE_Mesh*
CreateDebugWideLineEntity(NBRE_Context mContext, const NBRE_Polyline2f& mPolyline, const NBRE_Color& mColor, float width)
{
    uint32 segCount = mPolyline.VertexCount() - 1;
    NBRE_VertexData* mVertexData = NULL;

    // Create vertex buffer
    NBRE_HardwareVertexBuffer* vertextBuff = mContext.mRenderPal->CreateVertexBuffer(sizeof(float) * (3 + 4), segCount * 4, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    NBRE_VertexDeclaration* decalration = mContext.mRenderPal->CreateVertexDeclaration();
    NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem);
    NBRE_VertexElement* colorElem = NBRE_NEW NBRE_VertexElement(0, (3) * sizeof(float), NBRE_VertexElement::VET_FLOAT4, NBRE_VertexElement::VES_DIFFUSE, 0);
    decalration->GetVertexElementList().push_back(colorElem);

    mVertexData = NBRE_NEW NBRE_VertexData(0);
    mVertexData->AddBuffer(vertextBuff);
    mVertexData->SetVertexDeclaration(decalration);

    // Create index buffer
    NBRE_HardwareIndexBuffer* indexBuff = mContext.mRenderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, segCount * 8, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    NBRE_IndexData* mIndexData = NBRE_NEW NBRE_IndexData(indexBuff, 0, indexBuff->GetNumIndexes());

    // Update buffer data
    PolylineEntityVertex* pVertex = static_cast<PolylineEntityVertex*>(mVertexData->GetBuffer(0)->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));
    uint16* pIndex = static_cast<uint16*>(mIndexData->IndexBuffer()->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));
    uint16 baseIndex = 0;
    float z = mPolyline.Length() * 0.002f;

    for (uint32 i = 0; i < segCount; ++i)
    {
        const NBRE_Vector2f& p0 = mPolyline.Vertex(i);
        const NBRE_Vector2f& p1 = mPolyline.Vertex(i + 1);
        const NBRE_Vector2f& dir = p1 - p0;
        NBRE_Vector2f dirPerp = dir.Perpendicular();
        dirPerp.Normalise();
        dirPerp *= width * 0.5f;

        NBRE_Vector2f p = p0 - dirPerp;
        pVertex[0].position.x = p.x;
        pVertex[0].position.y = p.y;
        pVertex[0].position.z = z;
        pVertex[0].color = mColor;

        p = p1 - dirPerp;
        pVertex[1].position.x = p.x;
        pVertex[1].position.y = p.y;
        pVertex[1].position.z = z;
        pVertex[1].color = mColor;

        p = p1 + dirPerp;
        pVertex[2].position.x = p.x;
        pVertex[2].position.y = p.y;
        pVertex[2].position.z = z;
        pVertex[2].color = mColor;

        p = p0 + dirPerp;
        pVertex[3].position.x = p.x;
        pVertex[3].position.y = p.y;
        pVertex[3].position.z = z;
        pVertex[3].color = mColor;

        pIndex[0] = baseIndex + 0;
        pIndex[1] = baseIndex + 1;
        pIndex[2] = baseIndex + 1;
        pIndex[3] = baseIndex + 2;
        pIndex[4] = baseIndex + 2;
        pIndex[5] = baseIndex + 3;
        pIndex[6] = baseIndex + 3;
        pIndex[7] = baseIndex + 0;

        pVertex += 4;
        pIndex += 8;
        baseIndex += 4;
    }
    vertextBuff->UnLock();
    indexBuff->UnLock();

    NBRE_PassPtr mPassCache(NBRE_NEW NBRE_Pass());
    mPassCache->SetEnableDepthTest(FALSE);
    mPassCache->SetEnableDepthWrite(FALSE);
    mPassCache->SetEnableBlend(TRUE);
    mPassCache->SetLineWidth(3.0f);
    NBRE_ShaderPtr shader(NBRE_NEW NBRE_Shader());
    shader->AddPass(mPassCache);

    NBRE_Mesh* mesh = NBRE_NEW NBRE_Mesh(mVertexData);
    mesh->CreateSubMesh(mIndexData, NBRE_PMT_LINE_LIST);
    mesh->CalculateBoundingBox();
    return mesh;
}

NBRE_Mesh*
CreateDebugPolygonEntity(NBRE_Context mContext, const NBRE_Vector2d* vertices, uint32 count, const NBRE_Color& mColor)
{
    NBRE_VertexData* mVertexData = NULL;

    // Create vertex buffer
    NBRE_HardwareVertexBuffer* vertextBuff = mContext.mRenderPal->CreateVertexBuffer(sizeof(float) * (3 + 4), count, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    NBRE_VertexDeclaration* decalration = mContext.mRenderPal->CreateVertexDeclaration();
    NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem);
    NBRE_VertexElement* colorElem = NBRE_NEW NBRE_VertexElement(0, (3) * sizeof(float), NBRE_VertexElement::VET_FLOAT4, NBRE_VertexElement::VES_DIFFUSE, 0);
    decalration->GetVertexElementList().push_back(colorElem);

    mVertexData = NBRE_NEW NBRE_VertexData(0);
    mVertexData->AddBuffer(vertextBuff);
    mVertexData->SetVertexDeclaration(decalration);

    // Create index buffer
    NBRE_HardwareIndexBuffer* indexBuff = mContext.mRenderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, count + 1, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    NBRE_IndexData* mIndexData = NBRE_NEW NBRE_IndexData(indexBuff, 0, indexBuff->GetNumIndexes());

    // Update buffer data
    PolylineEntityVertex* pVertex = static_cast<PolylineEntityVertex*>(mVertexData->GetBuffer(0)->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));
    uint16* pIndex = static_cast<uint16*>(mIndexData->IndexBuffer()->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY));

    for (uint32 i = 0; i < count; ++i)
    {
        pVertex[i].position.x = (float)vertices[i].x;
        pVertex[i].position.y = (float)vertices[i].y;
        pVertex[i].position.z = 0;
        pVertex[i].color = mColor;
        pIndex[i] = (uint16)i;
    }
    pIndex[count] = 0;

    vertextBuff->UnLock();
    indexBuff->UnLock();

    NBRE_PassPtr mPassCache(NBRE_NEW NBRE_Pass());
    mPassCache->SetEnableDepthTest(TRUE);
    mPassCache->SetEnableDepthWrite(TRUE);
    mPassCache->SetEnableBlend(TRUE);
    mPassCache->SetLineWidth(1.0f);
    NBRE_ShaderPtr shader(NBRE_NEW NBRE_Shader());
    shader->AddPass(mPassCache);

    NBRE_Mesh* mesh = NBRE_NEW NBRE_Mesh(mVertexData);
    mesh->CreateSubMesh(mIndexData, NBRE_PMT_LINE_STRIP);
    mesh->CalculateBoundingBox();
    return mesh;
}

