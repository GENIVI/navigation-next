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
#include "resource.h"
#include "TestTexture.h"
#include "nbrehardwareindexbuffer.h"
#include "nbrehardwarevertexbuffer.h"
#include "nbrecommon.h"
#include "nbretransformation.h"
#include "nbrepngtextureimage.h"
#include "nbgmTestUtilities.h"
#include "nbretexture.h"
#include "nbretextureunit.h"
#include "nbretransformation.h"
#include "nbreglrenderpal.h"
#include "nbretypes.h"
#include "nbrecommon.h"

static NBRE_Matrix4x4f gIdentity;

CTestTexture::CTestTexture()
    :m_ScreenHeight(1)
    ,m_ScreenWidth(1)
    ,m_VertexData1(NULL)
    ,m_VertexData2(NULL)
    ,m_VertexData3(NULL)
    ,m_Texture1(NULL)
    ,m_Texture2(NULL)
    ,m_Texture3(NULL)
    ,m_TextureState1(NBRE_TFT_LINEAR, NBRE_TFT_LINEAR, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, FALSE)
    ,m_TextureState2(NBRE_TFT_MIPMAP_LINEAR, NBRE_TFT_LINEAR, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, FALSE)
    ,m_TextureState3(NBRE_TFT_LINEAR, NBRE_TFT_LINEAR, NBRE_TAM_REPEAT, NBRE_TAM_REPEAT, FALSE)
    ,m_IndexData(NULL)
{
}

CTestTexture::~CTestTexture()
{
    NBRE_DELETE m_IndexData;
    NBRE_DELETE m_VertexData1;
    NBRE_DELETE m_VertexData2;
    NBRE_DELETE m_VertexData3;
    NBRE_DELETE m_Texture1;
    NBRE_DELETE m_Texture2;
    NBRE_DELETE m_Texture3;
}


int CTestTexture::OnCreate()
{
    CreateTestData();
    return 0;
}

void CTestTexture::InitRenderState()
{
    m_RenderPal->SetClearColor(0, 0, 0, 1);
    m_RenderPal->EnableTexture(0, TRUE);
    m_RenderPal->DisableTextureUnitsFrom(1);
    m_RenderPal->EnableLighting(FALSE);
    m_RenderPal->EnableCullFace(TRUE);
}

void CTestTexture::OnSize(UINT /*nType*/, int cx, int cy)
{
    m_RenderPal->SetViewPort(0, 0, cx, cy);
    m_RenderPal->SetProjectionTransform(NBRE_Transformationf::BuildPerspectiveMatrix(60.0f, (float)cx/(float)cy, 0.1f, 100.0f));
    NBRE_Matrix4x4d identity;
    m_RenderPal->SetWorldTransform(identity);
    m_RenderPal->SetViewTransform(identity);
}

static NBRE_Texture*
CreateCreateColorTexture(NBRE_IRenderPal* renderer, const NBRE_Color& c)
{
    NBRE_Image** images = NBRE_NEW NBRE_Image*[1];

    uint8* pixels = NBRE_NEW uint8[3];

    pixels[0] = static_cast<uint8>(c.r*255);
    pixels[1] = static_cast<uint8>(c.g*255);
    pixels[2] = static_cast<uint8>(c.b*255);

    images[0] = NBRE_NEW NBRE_Image(1, 1, NBRE_PF_R8G8B8, pixels);

    return renderer->CreateTexture(images, 1, 1, FALSE, NBRE_Texture::TT_2D, "CreateCreateColorTexture");
}

void CTestTexture::OnRender()
{
    GetWindow()->OnRenderBegine();
    uint32 flags = 0;
    flags |= NBRE_CLF_CLEAR_COLOR;
    flags |= NBRE_CLF_CLEAR_DEPTH;
    m_RenderPal->Clear(flags);

    m_RenderPal->BeginScene();
    if(m_IndexData)
    {
        m_RenderPal->SelectTexture(0, m_Texture1);
        m_RenderPal->SetTextureState(0, m_TextureState1);
        m_RenderPal->SetTextureTransform(0, gIdentity);
        m_RenderPal->SetTextureCoordSet(0, 0);
        m_RenderPal->DisableTextureUnitsFrom(1);
        m_RenderPal->SetVertexBuffer(m_VertexData1);
        m_RenderPal->DrawIndexedPrimitive(NBRE_PMT_TRIANGLE_LIST, m_IndexData);

        m_RenderPal->SelectTexture(0, m_Texture2);
        m_RenderPal->SetTextureState(0, m_TextureState2);
        m_RenderPal->SetTextureTransform(0, gIdentity);
        m_RenderPal->SetTextureCoordSet(0, 0);
        m_RenderPal->DisableTextureUnitsFrom(1);
        m_RenderPal->SetVertexBuffer(m_VertexData2);
        m_RenderPal->DrawIndexedPrimitive(NBRE_PMT_TRIANGLE_LIST, m_IndexData);

        m_RenderPal->SelectTexture(0, m_Texture3);
        m_RenderPal->SetTextureState(0, m_TextureState3);
        m_RenderPal->SetTextureTransform(0, m_Matrix);
        m_RenderPal->SetTextureCoordSet(0, 0);
        m_RenderPal->SelectTexture(1, m_Texture1);
        m_RenderPal->SetTextureState(1, m_TextureState1);
        m_RenderPal->SetTextureTransform(1, gIdentity);
        m_RenderPal->SetTextureCoordSet(1, 0);
        m_RenderPal->SelectTexture(2, m_Texture2);
        m_RenderPal->SetTextureState(2, m_TextureState2);
        m_RenderPal->SetTextureTransform(2, gIdentity);
        m_RenderPal->SetTextureCoordSet(2, 0);
        m_RenderPal->DisableTextureUnitsFrom(3);
        m_RenderPal->SetVertexBuffer(m_VertexData3);
        m_RenderPal->DrawIndexedPrimitive(NBRE_PMT_TRIANGLE_LIST, m_IndexData);
    }
    m_RenderPal->EndScene();
    GetWindow()->OnRenderEnd();
}

static void GenVertexData(NBRE_VertexData **data,  const float *buffer, uint32 count, NBRE_IRenderPal* renderPal )
{
    ASSERT(data != NULL && buffer != NULL);
    NBRE_HardwareVertexBuffer* vertexBuffer = renderPal->CreateVertexBuffer(sizeof(float)*8, 3, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    vertexBuffer->WriteData(0, count, buffer, FALSE);
    NBRE_VertexElement* posElem    = NBRE_NEW NBRE_VertexElement(0, 0,  NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    NBRE_VertexElement* normalElem = NBRE_NEW NBRE_VertexElement(0, 3*sizeof(float),  NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_NORMAL, 0);
    NBRE_VertexElement* texElem    = NBRE_NEW NBRE_VertexElement(0, 6*sizeof(float), NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
    NBRE_VertexDeclaration* decl   = renderPal->CreateVertexDeclaration();
    decl->GetVertexElementList().push_back(posElem);
    decl->GetVertexElementList().push_back(normalElem);
    decl->GetVertexElementList().push_back(texElem);
    *data = NBRE_NEW NBRE_VertexData(0);
    (*data)->AddBuffer(vertexBuffer);
    (*data)->SetVertexDeclaration(decl);
    (*data)->SetVertexStart(0);
    (*data)->SetVertexCount(3);
}

void CTestTexture::CreateTestData()
{
    static float vertex_data1[] = {
        -4.f,  0.f,  -10, 0, 0, 1,    0,  0,
        -2.f,  0.f,  -10, 0, 0, 1,    1,  0,
        -1.f,  3.0f, -14, 0, 0, 1, 0.5f,  1
    };

    static float vertex_data2[] = {
        2.f,  0.f,  -10, 0, 0, 1,    0,  0,
        4.f,  0.f,  -10, 0, 0, 1,    1,  0,
        1.f,  3.0f, -14, 0, 0, 1, 0.5f,  1
    };

    static float vertex_data3[] = {
        -1.f,  0.f,  -10, 0, 0, 1,    0,  0,
         1.f,  0.f,  -10, 0, 0, 1,    1,  0,
         0.f,  2.2f, -10, 0, 0, 1,  0.5f, 1
    };

    //Create vertex data
    GenVertexData(&m_VertexData1, vertex_data1, sizeof(vertex_data1), m_RenderPal);
    GenVertexData(&m_VertexData2, vertex_data2, sizeof(vertex_data2), m_RenderPal);
    GenVertexData(&m_VertexData3, vertex_data3, sizeof(vertex_data3), m_RenderPal);

    //Create index data
    static uint16 indics[3] = { 0, 1, 2 };
    NBRE_HardwareIndexBuffer* indexBuffer = this->m_RenderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, 3, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    static_cast<NBRE_HardwareBuffer*>(indexBuffer)->WriteData(0, sizeof(indics), indics, FALSE);
    m_IndexData = NBRE_NEW NBRE_IndexData(indexBuffer, 0, 3);

     //Create texture
    char path1[MAX_PATH + 1] = {0};
    char path2[MAX_PATH + 1] = {0};
    char path3[MAX_PATH + 1] = {0};

    char *folder = GetExePath();
    sprintf_s(path1, MAX_PATH + 1, "%s\\%s", folder, "TestTex1.jpg");
    sprintf_s(path2, MAX_PATH + 1, "%s\\%s", folder, "ACC.png");
    sprintf_s(path3, MAX_PATH + 1, "%s\\%s", folder, "TestTex2.jpg");
    NBRE_DELETE []folder;

    m_Texture1 = CreateCreateColorTexture(m_RenderPal, NBRE_Color(1, 0, 0, 1));
    m_Texture1->Load();
    m_Texture2 = m_RenderPal->CreateTexture(NBRE_NEW NBRE_PngTextureImage(*GetPal(), path2, 0, TRUE), NBRE_Texture::TT_2D, TRUE, "m_Texture1");
    m_Texture2->Load();
    m_Texture3 = m_RenderPal->CreateTexture(NBRE_NEW NBRE_PngTextureImage(*GetPal(), path3, 0, TRUE), NBRE_Texture::TT_2D, FALSE, "m_Texture2");
    m_Texture3->Load();
}

void CTestTexture::OnTimer(UINT_PTR /*nIDEvent*/)
{
    clock_t c = clock();
    m_Matrix = NBRE_Transformation<float>::BuildRotateDegreesMatrix((float)((c/10)%360), 0.f, 0.f, 1.f);
}
