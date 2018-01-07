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
#include "TestGLPal.h"
#include "nbrecommon.h"
#include "nbretransformation.h"
#include "nbrecommon.h"

CTestGLPal::CTestGLPal():m_GLPal(NULL), m_VertexData0(NULL), m_VertexData1(NULL), m_IndexData(NULL)
{

}

CTestGLPal::~CTestGLPal()
{
    NBRE_DELETE m_IndexData;
    NBRE_DELETE m_VertexData0;
    NBRE_DELETE m_VertexData1;
}


int CTestGLPal::OnCreate()
{
    m_GLPal = dynamic_cast<NBRE_GLES2RenderPal*>(m_RenderPal);
    CreateTestData();
    return 0;
}

void CTestGLPal::OnSize(UINT /*nType*/, int cx, int cy)
{
    m_GLPal->SetViewPort(0, 0, cx, cy);

    m_GLPal->SetWorldTransform(NBRE_Transformationd::BuildRotateDegreesMatrix(0, 1, 0, 0));

    NBRE_Vector3d eye(0.f, 0.f, 5.f);
    NBRE_Vector3d target(0.f, 0.0f, 0.f);
    NBRE_Vector3d up(0.f, 1.f, 0.f);
    NBRE_Matrix4x4d viewMatrix = NBRE_Transformationd::BuildLookAtMatrix(eye, target, up);;
    m_GLPal->SetViewTransform(viewMatrix);
    m_GLPal->SetProjectionTransform(NBRE_Transformationf::BuildPerspectiveMatrix(90, (float)cx/(float)cy, 1.0f, 1000.0f));
}

void CTestGLPal::OnRender()
{
    GetWindow()->OnRenderBegine();
    m_GLPal->SetClearColor(0, 0, 0, 1);
    m_GLPal->Clear(NBRE_CLF_CLEAR_COLOR);
    if(m_IndexData)
    {
        m_GLPal->SetVertexBuffer(m_VertexData0);
        m_GLPal->DrawIndexedPrimitive(NBRE_PMT_TRIANGLE_LIST, this->m_IndexData);
        m_GLPal->SetVertexBuffer(m_VertexData1);
        m_GLPal->DrawIndexedPrimitive(NBRE_PMT_TRIANGLE_LIST, this->m_IndexData);
    }
    GetWindow()->OnRenderEnd();
}


void CTestGLPal::CreateTestData()
{
    static float color_data[] = {
        1, 0, 0, 1,
        0, 1, 0, 1,
        0, 0, 1, 1,
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

    NBRE_HardwareVertexBuffer* colorBuffer0 =    m_GLPal->CreateVertexBuffer(sizeof(float)*4, 3, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    NBRE_HardwareVertexBuffer* posBuffer0 =      m_GLPal->CreateVertexBuffer(sizeof(float)*3, 3, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    NBRE_HardwareVertexBuffer* normalBuffer0 =   m_GLPal->CreateVertexBuffer(sizeof(float)*3, 3, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    NBRE_HardwareVertexBuffer* texBuffer0 =      m_GLPal->CreateVertexBuffer(sizeof(float)*2, 3, FALSE, NBRE_HardwareBuffer::HBU_STATIC);

    colorBuffer0->  WriteData(0, sizeof(color_data),    color_data, FALSE);
    posBuffer0->    WriteData(0, sizeof(pos_data),      pos_data, FALSE);
    normalBuffer0-> WriteData(0, sizeof(normal_data),   normal_data, FALSE);
    texBuffer0->    WriteData(0, sizeof(tex_data),      tex_data, FALSE);

    NBRE_VertexElement* colorElem0  = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT4,  NBRE_VertexElement::VES_DIFFUSE, 0);
    NBRE_VertexElement* posElem0    = NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT3,  NBRE_VertexElement::VES_POSITION, 0);
    NBRE_VertexElement* normalElem0 = NBRE_NEW NBRE_VertexElement(2, 0, NBRE_VertexElement::VET_FLOAT3,  NBRE_VertexElement::VES_NORMAL, 0);
    NBRE_VertexElement* texElem0    = NBRE_NEW NBRE_VertexElement(3, 0, NBRE_VertexElement::VET_FLOAT2,  NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);

    NBRE_VertexDeclaration* decl0 = m_GLPal->CreateVertexDeclaration();
    decl0->GetVertexElementList().push_back(colorElem0);
    decl0->GetVertexElementList().push_back(posElem0);
    decl0->GetVertexElementList().push_back(normalElem0);
    decl0->GetVertexElementList().push_back(texElem0);

    m_VertexData0 = NBRE_NEW NBRE_VertexData(0);
    m_VertexData0->AddBuffer(colorBuffer0);
    m_VertexData0->AddBuffer(posBuffer0);
    m_VertexData0->AddBuffer(normalBuffer0);
    m_VertexData0->AddBuffer(texBuffer0);
    m_VertexData0->SetVertexDeclaration(decl0);

    static float vertex_data[] = {
        1, 0, 0, 1,  0.5,-0.5,0, 0, 0, 1,    0,   0,
        0, 1, 0, 1,  1.5,-0.5,0, 0, 0, 1,    1,   0,
        0, 0, 1, 1,   1.0,1.0,0, 0, 0, 1, 0.5f,   1
    };
    NBRE_HardwareVertexBuffer* vertexBuffer1 = m_GLPal->CreateVertexBuffer(sizeof(float)*12, 3, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    vertexBuffer1->WriteData(0, sizeof(vertex_data), vertex_data, FALSE);
    NBRE_VertexElement* colorElem1 =  NBRE_NEW NBRE_VertexElement(0, 0,                   NBRE_VertexElement::VET_FLOAT4,  NBRE_VertexElement::VES_DIFFUSE, 0);
    NBRE_VertexElement* posElem1 =    NBRE_NEW NBRE_VertexElement(0, 4*sizeof(float),     NBRE_VertexElement::VET_FLOAT3,  NBRE_VertexElement::VES_POSITION, 0);
    NBRE_VertexElement* normalElem1 = NBRE_NEW NBRE_VertexElement(0, 7*sizeof(float),     NBRE_VertexElement::VET_FLOAT3,  NBRE_VertexElement::VES_NORMAL, 0);
    NBRE_VertexElement* texElem1 =    NBRE_NEW NBRE_VertexElement(0, 10*sizeof(float),    NBRE_VertexElement::VET_FLOAT2,  NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
    NBRE_VertexDeclaration* decl1 =   m_GLPal->CreateVertexDeclaration();
    decl1->GetVertexElementList().push_back(colorElem1);
    decl1->GetVertexElementList().push_back(posElem1);
    decl1->GetVertexElementList().push_back(normalElem1);
    decl1->GetVertexElementList().push_back(texElem1);
    m_VertexData1 = NBRE_NEW NBRE_VertexData(0);
    m_VertexData1->AddBuffer(vertexBuffer1);
    m_VertexData1->SetVertexDeclaration(decl1);

    //create index data
    static uint16 indics[3] =
    {
        0, 1, 2
    };
    NBRE_HardwareIndexBuffer* indexBuffer = m_GLPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, 3, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    static_cast<NBRE_HardwareBuffer*>(indexBuffer)->WriteData(0, sizeof(indics), indics, FALSE);
    m_IndexData = NBRE_NEW NBRE_IndexData(indexBuffer, 0, 3);
}