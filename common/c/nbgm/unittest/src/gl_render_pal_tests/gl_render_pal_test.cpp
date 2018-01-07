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

#include "gl_render_pal_test.h"
#include "nbrehardwareindexbuffer.h"
#include "nbrehardwarevertexbuffer.h"
#include "nbrevertexindexdata.h"
#include "private\nbreglhardwareindexbuffer.h"
#include "private\nbreglhardwarevertexbuffer.h"

int SetUp(void)
{
    return 0;
}

int CleanUpTest(void)
{
    return 0;
}

void TestGLIndexHardWareBufferCreate(void)
{
    NBRE_GLHardwareIndexBuffer indexBuffer0(NBRE_GLHardwareIndexBuffer::IT_8BIT, 3);
    CU_ASSERT(indexBuffer0.GetType() == NBRE_GLHardwareIndexBuffer::IT_8BIT);
    CU_ASSERT(indexBuffer0.GetNumIndexes() == 3);
    CU_ASSERT(indexBuffer0.GetIndexSize() == 1);

    NBRE_GLHardwareIndexBuffer indexBuffer1(NBRE_GLHardwareIndexBuffer::IT_16BIT, 6);
    CU_ASSERT(indexBuffer1.GetType() == NBRE_GLHardwareIndexBuffer::IT_16BIT);
    CU_ASSERT(indexBuffer1.GetNumIndexes() == 6);
    CU_ASSERT(indexBuffer1.GetIndexSize() == 2);

    NBRE_GLHardwareIndexBuffer indexBuffer2(NBRE_GLHardwareIndexBuffer::IT_32BIT, 9);
    CU_ASSERT(indexBuffer2.GetType() == NBRE_GLHardwareIndexBuffer::IT_32BIT);
    CU_ASSERT(indexBuffer2.GetNumIndexes() == 9);
    CU_ASSERT(indexBuffer2.GetIndexSize() == 4);
}

void TestGLVertexHardWareBufferCreate(void)
{
    NBRE_GLHardwareVertexBuffer vertexBuffer0(
    sizeof(float)*3, 3);
    CU_ASSERT(vertexBuffer0.GetVertexSize() == 12);
    CU_ASSERT(vertexBuffer0.GetNumVertices() == 3);
}

void TestGLIndexHardWareBufferReadWrite()
{
    static uint16 index_data[] = {
        0, 1, 2,
        3, 4, 5,
        6, 7, 8
    };

    NBRE_HardwareBuffer* indexBuffer0 = new NBRE_GLHardwareIndexBuffer(NBRE_GLHardwareIndexBuffer::IT_16BIT, 3);
    uint16 readData[9] = {0} ;

    for(int i=0; i<3*2; i+=2)
    {
        indexBuffer0->WriteData(i, 3*2-i, index_data+i/2);
        indexBuffer0->ReadData(i, 3*2-i, &readData);
        CU_ASSERT( nsl_memcmp(index_data+i/2, readData, 3*2-i) == 0);
    }

    indexBuffer0->WriteData(0, 3*2, index_data);
    for(int j=0; j<3*2; j+=2)
    {
        indexBuffer0->ReadData(j, 3*2-j, &readData);
        CU_ASSERT( nsl_memcmp(index_data+j/2, readData, 3*2-j) == 0);
    }

    delete indexBuffer0;
}

void TestGLVertexHardWareBufferReadWrite()
{
    static float vertex_data[] = {
        0, 1, 2, 3,
        4, 5, 6, 7,
        8, 9, 10, 11
    };

    NBRE_HardwareBuffer* indexBuffer0 = new NBRE_GLHardwareVertexBuffer(4*sizeof(float), 3);
    float readData[12] = {0} ;

    for(int i=0; i<3*4*sizeof(float); i+=sizeof(float))
    {
        indexBuffer0->WriteData(i, 3*4*sizeof(float)-i, vertex_data+i/sizeof(float));
        indexBuffer0->ReadData(i, 3*4*sizeof(float)-i, &readData);
        CU_ASSERT( nsl_memcmp(vertex_data+i/sizeof(float), readData, 3*4*sizeof(float)-i) == 0);
    }

    indexBuffer0->WriteData(0, 3*4*sizeof(float), vertex_data);
    for(int j=0; j<3*4*sizeof(float); j+=sizeof(float))
    {
        indexBuffer0->ReadData(j, 3*4*sizeof(float)-j, &readData);
        CU_ASSERT( nsl_memcmp(vertex_data+j/sizeof(float), readData, 3*4*sizeof(float)-j) == 0);
    }

    delete indexBuffer0;
}

void TestVertexElement()
{
    NBRE_VertexElement elem(2, 3, NBRE_VertexElement::VET_FLOAT4, NBRE_VertexElement::VES_DIFFUSE, 0);
    CU_ASSERT(elem.Source() == 2);
    CU_ASSERT(elem.Offset() == 3);
    CU_ASSERT(elem.Type() == NBRE_VertexElement::VET_FLOAT4);
    CU_ASSERT(elem.Semantic() == NBRE_VertexElement::VES_DIFFUSE);
    CU_ASSERT(NBRE_VertexElement::GetTypeCount(elem.Type()) == 4);
}

void TestVertexDeclaration()
{
    NBRE_VertexDeclaration decl;
    NBRE_VertexElement* elem = NBRE_NEW NBRE_VertexElement(2, 3, NBRE_VertexElement::VET_FLOAT4, NBRE_VertexElement::VES_DIFFUSE, 0);
    decl.GetVertexElementList().push_back(elem);
    CU_ASSERT(decl.GetVertexElementList().size() == 1);
}

void TestIndexData()
{
    static uint16 index_data[] = {
        0, 1, 2,
        3, 4, 5,
        6, 7, 8
    };

    NBRE_HardwareIndexBuffer* indexBuffer = new NBRE_GLHardwareIndexBuffer(NBRE_GLHardwareIndexBuffer::IT_16BIT, 9);

    NBRE_IndexData index(indexBuffer, 0, 9);
    CU_ASSERT(index.IndexBuffer() == indexBuffer);
    CU_ASSERT(index.IndexStart() == 0);
    CU_ASSERT(index.IndexCount() == 9);
}

void TestVertexData()
{
    static float color_data[] = {
        1, 0, 0, 1,
        0, 1, 0, 1,
        0, 0, 1, 1,
    };
    static float pos_data[] = {
        -0.5, -0.5, -10,
        0.5, -0.5, -10,
        0,    1, -10,
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

    NBRE_GLHardwareVertexBuffer* colorBuffer0 =    NBRE_NEW NBRE_GLHardwareVertexBuffer(sizeof(float)*4, 3);
    NBRE_GLHardwareVertexBuffer* posBuffer0 =      NBRE_NEW NBRE_GLHardwareVertexBuffer(sizeof(float)*3, 3);
    NBRE_GLHardwareVertexBuffer* normalBuffer0 =   NBRE_NEW NBRE_GLHardwareVertexBuffer(sizeof(float)*3, 3);
    NBRE_GLHardwareVertexBuffer* texBuffer0 =      NBRE_NEW NBRE_GLHardwareVertexBuffer(sizeof(float)*2, 3);

    colorBuffer0->   WriteData(0, sizeof(color_data),    color_data);
    posBuffer0->     WriteData(0, sizeof(pos_data),      pos_data);
    normalBuffer0->  WriteData(0, sizeof(normal_data),   normal_data);
    texBuffer0->     WriteData(0, sizeof(tex_data),      tex_data);

    NBRE_VertexElement* colorElem0 =     NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT4,  NBRE_VertexElement::VES_DIFFUSE, 0);
    NBRE_VertexElement* posElem0 =       NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT3,  NBRE_VertexElement::VES_POSITION, 0);
    NBRE_VertexElement* normalElem0 =    NBRE_NEW NBRE_VertexElement(2, 0, NBRE_VertexElement::VET_FLOAT3,  NBRE_VertexElement::VES_NORMAL, 0);
    NBRE_VertexElement* texElem0 =       NBRE_NEW NBRE_VertexElement(3, 0, NBRE_VertexElement::VET_FLOAT2,  NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);

    NBRE_VertexDeclaration* decl0 = NBRE_NEW NBRE_VertexDeclaration();
    decl0->GetVertexElementList().push_back(colorElem0);
    decl0->GetVertexElementList().push_back(posElem0);
    decl0->GetVertexElementList().push_back(normalElem0);
    decl0->GetVertexElementList().push_back(texElem0);

    NBRE_VertexData* vertexData0 = NBRE_NEW NBRE_VertexData(3);
    vertexData0->AddBuffer(colorBuffer0);
    vertexData0->AddBuffer(posBuffer0);
    vertexData0->AddBuffer(normalBuffer0);
    vertexData0->AddBuffer(texBuffer0);
    vertexData0->SetVertexDeclaration(decl0);


    static float vertex_data[] = {
        1, 0, 0, 1,  0.5f, -0.5f, -10, 0, 0, 1,    0,   0,
        0, 1, 0, 1,  1.5f, -0.5f, -10, 0, 0, 1,    1,   0,
        0, 0, 1, 1,     1,     1, -10, 0, 0, 1, 0.5f,   1
    };
    NBRE_GLHardwareVertexBuffer* vertexBuffer1 = NBRE_NEW NBRE_GLHardwareVertexBuffer(sizeof(float)*12, 3);
    vertexBuffer1->WriteData(0, sizeof(vertex_data), vertex_data);
    NBRE_VertexElement* colorElem1 =    NBRE_NEW NBRE_VertexElement(0, 0,                   NBRE_VertexElement::VET_FLOAT4,  NBRE_VertexElement::VES_DIFFUSE, 0);
    NBRE_VertexElement* posElem1 =      NBRE_NEW NBRE_VertexElement(0, 4*sizeof(float),     NBRE_VertexElement::VET_FLOAT3,  NBRE_VertexElement::VES_POSITION, 0);
    NBRE_VertexElement* normalElem1 =   NBRE_NEW NBRE_VertexElement(0, 7*sizeof(float),     NBRE_VertexElement::VET_FLOAT3,  NBRE_VertexElement::VES_NORMAL, 0);
    NBRE_VertexElement* texElem1 =      NBRE_NEW NBRE_VertexElement(0, 10*sizeof(float),    NBRE_VertexElement::VET_FLOAT2,  NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
    NBRE_VertexDeclaration* decl1 = NBRE_NEW NBRE_VertexDeclaration();
    decl1->GetVertexElementList().push_back(colorElem1);
    decl1->GetVertexElementList().push_back(posElem1);
    decl1->GetVertexElementList().push_back(normalElem1);
    decl1->GetVertexElementList().push_back(texElem1);
    NBRE_VertexData* vertexData1 = NBRE_NEW NBRE_VertexData(3);
    vertexData1->AddBuffer(vertexBuffer1);
    vertexData1->SetVertexDeclaration(decl1);

    CU_ASSERT(vertexData0->GetBuffer(0) == colorBuffer0);
    CU_ASSERT(vertexData0->GetBuffer(1) == posBuffer0);
    CU_ASSERT(vertexData0->GetBuffer(2) == normalBuffer0);
    CU_ASSERT(vertexData0->GetBuffer(3) == texBuffer0);
    CU_ASSERT(vertexData0->GetVertexDeclaration() == decl0);

    CU_ASSERT(vertexData1->GetBuffer(1) == vertexBuffer1);
    CU_ASSERT(vertexData1->GetVertexDeclaration() == decl1);

    delete vertexData0;
    delete vertexData1;
}

GLRenderPalTest::GLRenderPalTest():NBRE_TestSuite("gl_render_pal_test", SetUp, CleanUpTest)
{
    AddTestCase(new NBRE_TestCase("Test_GL_Index_HardWare_Buffer_Create", TestGLIndexHardWareBufferCreate));
    AddTestCase(new NBRE_TestCase("Test_GL_Vertex_HardWare_Buffer_Create", TestGLVertexHardWareBufferCreate));

    AddTestCase(new NBRE_TestCase("Test_GL_Index_HardWare_Buffer_ReadWrite", TestGLIndexHardWareBufferReadWrite));    
    AddTestCase(new NBRE_TestCase("Test_GL_Vertex_HardWare_Buffer_ReadWrite", TestGLVertexHardWareBufferReadWrite));    

    AddTestCase(new NBRE_TestCase("Test_VertexElement", TestVertexElement));    
    AddTestCase(new NBRE_TestCase("Test_VertexDeclaration", TestVertexDeclaration));    

    AddTestCase(new NBRE_TestCase("Test_IndexData", TestIndexData));    
}
