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
#include "nbgmtest.h"
#include "resource.h"
#include "TestD3D9Pal.h"
#include "nbrecolor.h"
#include "nbred3d9renderpal.h"
#include "nbrecommon.h"
#include "nbretransformation.h"
#include "D3D9View.h"
#include "nbrecommon.h"

CTestD3D9Pal::CTestD3D9Pal():m_D3DPal(NULL), m_VertexData0(NULL), m_VertexData1(NULL), m_IndexData(NULL)
{
}

CTestD3D9Pal::~CTestD3D9Pal()
{
    NBRE_DELETE m_IndexData;
    NBRE_DELETE m_VertexData0;
    NBRE_DELETE m_VertexData1;

    for (NBRE_LightList::iterator it = m_LightList.begin(); it != m_LightList.end(); ++it)
    {
        NBRE_DELETE *it;
    }
}

void CTestD3D9Pal::InitRenderState()
{
    m_D3DPal->SetClearColor(0.f, 0.f, 0.f, 1.f);
    m_D3DPal->SetClearDepth(1.f);
    m_D3DPal->EnableDepthTest(TRUE);
    m_D3DPal->SetDepthFunc(NBRE_CF_LESS_EQUAL);
    m_D3DPal->EnableFog(FALSE);
    m_D3DPal->EnableCullFace(TRUE);
    m_D3DPal->EnableLighting(TRUE);
    m_D3DPal->UseLights(m_LightList);
}

int CTestD3D9Pal::OnCreate()
{
    m_D3DPal = dynamic_cast<NBRE_D3D9RenderPal*>(m_RenderPal);
    ASSERT(m_D3DPal);
    CreateTestData();
    return 0;
}

void CTestD3D9Pal::OnSize(UINT /*nType*/, int cx, int cy)
{
    m_D3DPal->SetViewPort(0, 0, cx, cy);

    m_D3DPal->SetWorldTransform(NBRE_Transformationd::BuildRotateDegreesMatrix(0, 1, 0, 0));

    NBRE_Vector3d eye(0.f, 0.f, 5.f);
    NBRE_Vector3d target(0.f, 0.0f, 0.f);
    NBRE_Vector3d up(0.f, 1.f, 0.f);
    NBRE_Matrix4x4d viewMatrix = NBRE_Transformationd::BuildLookAtMatrix(eye, target, up);;
    m_D3DPal->SetViewTransform(viewMatrix);
    m_D3DPal->SetProjectionTransform(NBRE_Transformationf::BuildPerspectiveMatrix(90, (float)cx/(float)cy, 1.0f, 1000.0f));
}

void CTestD3D9Pal::OnRender()
{
    GetWindow()->OnRenderBegine();
    m_D3DPal->Clear(NBRE_CLF_CLEAR_COLOR|NBRE_CLF_CLEAR_DEPTH);
    m_D3DPal->BeginScene();
    if(m_IndexData)
    {
        m_D3DPal->SetVertexBuffer(m_VertexData0);
        m_D3DPal->DrawIndexedPrimitive(NBRE_PMT_TRIANGLE_LIST, m_IndexData);
        m_D3DPal->SetVertexBuffer(m_VertexData1);
        m_D3DPal->DrawIndexedPrimitive(NBRE_PMT_TRIANGLE_LIST, m_IndexData);
    }
    m_D3DPal->EndScene();
    GetWindow()->OnRenderEnd();
}

struct TestDataUnit
{
    float position[3];
    DWORD color;
    float normal[3];

    TestDataUnit(float r, float g, float b, float a, float x, float y, float z,float nx, float ny, float nz)
    {
        color =  NBRE_Color(r,g,b,a).GetAsARGB();
        position[0] = x;
        position[1] = y;
        position[2] = z;
        normal[0] = nx;
        normal[1] = ny;
        normal[2] = nz;
    }
};

void CTestD3D9Pal::CreateTestData()
{
    static float color_data[] = {
        1, 0, 0,1,
        0, 1, 0,1,
        0, 0, 1,1
    };

    static float pos_data[] = {
       -2.0f, -0.5f,  0.f,
       -1.0f, -0.5f,  0.f,
       -1.5f,   1.f,  0.f
    };
    static float normal_data[] = {
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
    };
    static float tex_data[] = {
        0,   0,
        1,   0,
        0.5, 1,
    };

    NBRE_HardwareVertexBuffer* colorBuffer0  = m_D3DPal->CreateVertexBuffer(sizeof(float)*4, 3, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    NBRE_HardwareVertexBuffer* posBuffer0    = m_D3DPal->CreateVertexBuffer(sizeof(float)*3, 3, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    NBRE_HardwareVertexBuffer* normalBuffer0 = m_D3DPal->CreateVertexBuffer(sizeof(float)*3, 3, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    NBRE_HardwareVertexBuffer* texBuffer0    = m_D3DPal->CreateVertexBuffer(sizeof(float)*2, 3, FALSE, NBRE_HardwareBuffer::HBU_STATIC);

    colorBuffer0->  WriteData(0, sizeof(color_data),  color_data, FALSE);
    posBuffer0->    WriteData(0, sizeof(pos_data),    pos_data, FALSE);
    normalBuffer0-> WriteData(0, sizeof(normal_data), normal_data, FALSE);
    texBuffer0->    WriteData(0, sizeof(tex_data),    tex_data, FALSE);

    NBRE_VertexElement* colorElem0  = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT4,  NBRE_VertexElement::VES_DIFFUSE, 0);
    NBRE_VertexElement* posElem0    = NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT3,  NBRE_VertexElement::VES_POSITION, 0);
    NBRE_VertexElement* normalElem0 = NBRE_NEW NBRE_VertexElement(2, 0, NBRE_VertexElement::VET_FLOAT3,  NBRE_VertexElement::VES_NORMAL, 0);
    NBRE_VertexElement* texElem0    = NBRE_NEW NBRE_VertexElement(3, 0, NBRE_VertexElement::VET_FLOAT2,  NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);

    NBRE_VertexDeclaration* decl0 = m_D3DPal->CreateVertexDeclaration();
    decl0->GetVertexElementList().push_back(colorElem0);
    decl0->GetVertexElementList().push_back(posElem0);
    decl0->GetVertexElementList().push_back(normalElem0);
    decl0->GetVertexElementList().push_back(texElem0);

    m_VertexData0 = NBRE_NEW NBRE_VertexData(3);
    m_VertexData0->AddBuffer(colorBuffer0);
    m_VertexData0->AddBuffer(posBuffer0);
    m_VertexData0->AddBuffer(normalBuffer0);
    m_VertexData0->AddBuffer(texBuffer0);
    m_VertexData0->SetVertexDeclaration(decl0);
        
    TestDataUnit vertex_data1[] = {
        TestDataUnit(1,0,0,0, 0.5,-0.5,0,  0,0,1),
        TestDataUnit(0,1,0,0, 1.5,-0.5,0,  0,0,1),
        TestDataUnit(0,0,1,1, 1.0,1.0,0,   0,0,1),
    };

    NBRE_HardwareVertexBuffer* vertexBuffer1 = m_D3DPal->CreateVertexBuffer(sizeof(TestDataUnit), 3, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    vertexBuffer1->WriteData(0, sizeof(vertex_data1), vertex_data1, FALSE);
    NBRE_VertexElement* posElem1   = NBRE_NEW NBRE_VertexElement(0, 0,    NBRE_VertexElement::VET_FLOAT3,  NBRE_VertexElement::VES_POSITION, 0);
    NBRE_VertexElement* colorElem1 = NBRE_NEW NBRE_VertexElement(0, sizeof(float)*3,  NBRE_VertexElement::VET_COLOUR,  NBRE_VertexElement::VES_DIFFUSE, 0);
    NBRE_VertexElement* nomElem1   = NBRE_NEW NBRE_VertexElement(0, sizeof(float)*4,  NBRE_VertexElement::VET_FLOAT3,  NBRE_VertexElement::VES_NORMAL, 0);
    NBRE_VertexDeclaration* decl1  = m_D3DPal->CreateVertexDeclaration();

    decl1->GetVertexElementList().push_back(posElem1);
    decl1->GetVertexElementList().push_back(colorElem1);
    decl1->GetVertexElementList().push_back(nomElem1);

    m_VertexData1 = NBRE_NEW NBRE_VertexData(0);
    m_VertexData1->AddBuffer(vertexBuffer1);
    m_VertexData1->SetVertexDeclaration(decl1);

    //create index data
    static uint16 indics[3] =
    {
        0, 1, 2
    };

    NBRE_HardwareIndexBuffer* indexBuffer = m_D3DPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, 3, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    static_cast<NBRE_HardwareBuffer*>(indexBuffer)->WriteData(0, sizeof(indics), indics, FALSE);
    m_IndexData = NBRE_NEW NBRE_IndexData(indexBuffer, 0, 3);

    NBRE_Light *lt = NBRE_NEW NBRE_Light();
    lt->SetType(NBRE_LT_DIRECTIONAL);
    lt->SetDirection(NBRE_Vector3f(0,0,-1));
    lt->SetDiffuse(NBRE_Color(1.f, 1.f, 1.f,1.f));
    lt->SetAmbient(NBRE_Color(1.f, 1.f, 1.f,1.f));
    lt->SetSpecular(NBRE_Color(1.f, 1.f, 1.f,1.f));

    m_LightList.push_back(lt);
}
