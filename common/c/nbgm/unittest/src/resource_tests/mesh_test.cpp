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

#include "mesh_test.h"
#include "palstdlib.h"

#include "nbremesh.h"
#include "nbresubmesh.h"
#include "nbrehardwarevertexbuffer.h"
#include "nbrehardwareindexbuffer.h"
#include "nbreaxisalignedbox3.h"
#include "nbreglrenderpal.h"

static NBRE_IRenderPal* gRenderPal = NULL;
/* The suite initialization function.
 * Opens the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 * Case name is mesh_test.
 */
int init_suite_mesh(void)
{
    gRenderPal = NBRE_NEW NBRE_GlRenderPal();
    return 0;
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int clean_suite_mesh(void)
{
    NBRE_DELETE gRenderPal;
    return 0;
}

void testMeshNotUseSharedVertices(void)
{
    /// use for creating submesh_1
    float vertices_1[9] = 
    {
        -100.0f, -100.0f, 0.0f,
        100.0f, -100.0f, 0.0f,
        0.0f,  0.0f, 0.0f,
    };

    uint16 indices_1[3] = {0, 1, 2};

    /// use for creating submesh_2
    float vertices_2[9] = 
    {
        -50.0f, -50.0f, 0.0f,
        50.0f, -50.0f, 0.0f,
        10.0f,  0.0f, 0.0f,
    };

    uint16 indices_2[3] = {0, 1, 2};

    NBRE_AxisAlignedBox3f boundingBox(-100.0f, -100.0f, 0.0f, 100.0f, 0.0f, 0.0f);

    /** Test mesh which do not use shared vertices, 
        the sub meshes are 1 and 2
    */

    NBRE_Mesh* mesh_1 = new NBRE_Mesh();

    /// Create sub mesh 1
    NBRE_HardwareVertexBuffer* vertextBuff_1 = gRenderPal->CreateVertexBuffer(sizeof(float) * 3, 3, FALSE);
    vertextBuff_1->WriteData(0, sizeof(vertices_1), vertices_1);

    NBRE_VertexDeclaration* decalration_1 = new NBRE_VertexDeclaration();
    NBRE_VertexElement* vertexElement_1 = new NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    decalration_1->GetVertexElementList().push_back(vertexElement_1);

    NBRE_VertexData* vertexData_1 = new NBRE_VertexData(3);
    vertexData_1->AddBuffer(vertextBuff_1);
    vertexData_1->SetVertexDeclaration(decalration_1);

    NBRE_HardwareIndexBuffer* indexBuff_1 = gRenderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, 3, FALSE);
    indexBuff_1->WriteData(0, sizeof(indices_1), indices_1);
    NBRE_IndexData* indexData_1 = new NBRE_IndexData(indexBuff_1, 0, indexBuff_1->GetNumIndexes());

    mesh_1->CreateSubMesh(vertexData_1, indexData_1, NBRE_PMT_TRIANGLE_LIST);

    /// Create sub mesh 2
    NBRE_HardwareVertexBuffer* vertextBuff_2 = gRenderPal->CreateVertexBuffer(sizeof(float) * 3, 3, FALSE);
    vertextBuff_2->WriteData(0, sizeof(vertices_2), vertices_2);

    NBRE_VertexDeclaration* decalration_2 = new NBRE_VertexDeclaration();
    NBRE_VertexElement* vertexElement_2 = new NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    decalration_2->GetVertexElementList().push_back(vertexElement_2);

    NBRE_VertexData* vertexData_2 = new NBRE_VertexData(3);
    vertexData_2->AddBuffer(vertextBuff_2);
    vertexData_2->SetVertexDeclaration(decalration_2);

    NBRE_HardwareIndexBuffer* indexBuff_2 = gRenderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, 3, FALSE);
    indexBuff_2->WriteData(0, sizeof(indices_2), indices_2);
    NBRE_IndexData* indexData_2 = new NBRE_IndexData(indexBuff_2, 0, indexBuff_2->GetNumIndexes());

    mesh_1->CreateSubMesh(vertexData_2, indexData_2, NBRE_PMT_TRIANGLE_LIST);

    mesh_1->CalculateBoundingBox();

    CU_ASSERT(mesh_1 != NULL);
    CU_ASSERT(mesh_1->GetSubMeshNumber() == 2);

    CU_ASSERT(mesh_1->GetSubMesh(0) != NULL);
    CU_ASSERT(mesh_1->GetSubMesh(0)->GetVertexData() == vertexData_1);
    CU_ASSERT(mesh_1->GetSubMesh(0)->GetIndexData() == indexData_1);
    CU_ASSERT(mesh_1->GetSubMesh(0)->UseSharedVertices() == FALSE);
    CU_ASSERT(mesh_1->GetSubMesh(0)->GetOperationType() == NBRE_PMT_TRIANGLE_LIST);

    CU_ASSERT(mesh_1->GetSubMesh(1) != NULL);
    CU_ASSERT(mesh_1->GetSubMesh(1)->GetVertexData() == vertexData_2);
    CU_ASSERT(mesh_1->GetSubMesh(1)->GetIndexData() == indexData_2);
    CU_ASSERT(mesh_1->GetSubMesh(1)->UseSharedVertices() == FALSE);
    CU_ASSERT(mesh_1->GetSubMesh(1)->GetOperationType() == NBRE_PMT_TRIANGLE_LIST);

    CU_ASSERT(mesh_1->GetSharedVertexedData() == NULL);

    CU_ASSERT(mesh_1->AxisAlignedBox().maxExtend == boundingBox.maxExtend);
    CU_ASSERT(mesh_1->AxisAlignedBox().minExtend == boundingBox.minExtend);

    delete mesh_1;
}

void testMeshUseSharedVertices(void)
{
    /// use for creating submesh_3,4 they use the shared vertices
    float vertices_3_4[18] = 
    {
        -100.0f, -100.0f, 0.0f,
        100.0f, -100.0f, 0.0f,
        0.0f,  0.0f, 0.0f,

        -50.0f, -50.0f, 0.0f,
        50.0f, -50.0f, 0.0f,
        10.0f,  0.0f, 0.0f,
    };

    uint16 indices_3[3] = {0, 1, 2};
    uint16 indices_4[3] = {3, 4, 5};

    NBRE_AxisAlignedBox3f boundingBox(-100.0f, -100.0f, 0.0f, 100.0f, 0.0f, 0.0f);

    /** Test mesh which use shared vertices, 
        the sub meshes are 3 and 4
    */

    /// Create sub mesh 3 and 4
    NBRE_HardwareVertexBuffer* vertextBuff_3_4 = gRenderPal->CreateVertexBuffer(sizeof(float) * 3, 6, FALSE);
    vertextBuff_3_4->WriteData(0, sizeof(vertices_3_4), vertices_3_4);

    NBRE_VertexDeclaration* decalration_3_4 = new NBRE_VertexDeclaration();
    NBRE_VertexElement* vertexElement_3_4 = new NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    decalration_3_4->GetVertexElementList().push_back(vertexElement_3_4);

    NBRE_VertexData* vertexData_3_4 = new NBRE_VertexData(6);
    vertexData_3_4->AddBuffer(vertextBuff_3_4);
    vertexData_3_4->SetVertexDeclaration(decalration_3_4);

    NBRE_Mesh* mesh_2 = new NBRE_Mesh(vertexData_3_4);

    NBRE_HardwareIndexBuffer* indexBuff_3 = gRenderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, 3, FALSE);
    indexBuff_3->WriteData(0, sizeof(indices_3), indices_3);
    NBRE_IndexData* indexData_3 = new NBRE_IndexData(indexBuff_3, 0, indexBuff_3->GetNumIndexes());

    NBRE_HardwareIndexBuffer* indexBuff_4 = gRenderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, 3, FALSE);
    indexBuff_4->WriteData(0, sizeof(indices_4), indices_4);
    NBRE_IndexData* indexData_4 = new NBRE_IndexData(indexBuff_4, 0, indexBuff_4->GetNumIndexes());

    mesh_2->CreateSubMesh(indexData_3, NBRE_PMT_TRIANGLE_LIST);
    mesh_2->CreateSubMesh(indexData_4, NBRE_PMT_TRIANGLE_LIST);

    mesh_2->CalculateBoundingBox();

    CU_ASSERT(mesh_2 != NULL);
    CU_ASSERT(mesh_2->GetSubMeshNumber() == 2);

    CU_ASSERT(mesh_2->GetSubMesh(0) != NULL);
    CU_ASSERT(mesh_2->GetSubMesh(0)->GetVertexData() == NULL);
    CU_ASSERT(mesh_2->GetSubMesh(0)->GetIndexData() == indexData_3);
    CU_ASSERT(mesh_2->GetSubMesh(0)->UseSharedVertices() == TRUE);
    CU_ASSERT(mesh_2->GetSubMesh(0)->GetOperationType() == NBRE_PMT_TRIANGLE_LIST);

    CU_ASSERT(mesh_2->GetSubMesh(1) != NULL);
    CU_ASSERT(mesh_2->GetSubMesh(1)->GetVertexData() == NULL);
    CU_ASSERT(mesh_2->GetSubMesh(1)->GetIndexData() == indexData_4);
    CU_ASSERT(mesh_2->GetSubMesh(1)->UseSharedVertices() == TRUE);
    CU_ASSERT(mesh_2->GetSubMesh(1)->GetOperationType() == NBRE_PMT_TRIANGLE_LIST);

    CU_ASSERT(mesh_2->GetSharedVertexedData() == vertexData_3_4);

    CU_ASSERT(mesh_2->AxisAlignedBox().maxExtend == boundingBox.maxExtend);
    CU_ASSERT(mesh_2->AxisAlignedBox().minExtend == boundingBox.minExtend);

    delete mesh_2;
}

void testMeshUseSharedVerticesAndMixedVertices(void)
{
    float vertices[] = 
    {
        1, 0, 0, 1,  0.5f, -0.5f, -10, 0, 0, 1,    0,   0,
        0, 1, 0, 1,  1.5f, -0.5f, -10, 0, 0, 1,    1,   0,
        0, 0, 1, 1,     1,     1, -10, 0, 0, 1, 0.5f,   1
    };

    uint16 indices[3] = {0, 1, 2};

    NBRE_AxisAlignedBox3f boundingBox(0.5f, -0.5f, -10.0f, 1.5f, 1.0f, -10.0f);

    NBRE_HardwareVertexBuffer* vertextBuff = gRenderPal->CreateVertexBuffer(sizeof(float) * 12, 3, FALSE);
    vertextBuff->WriteData(0, sizeof(vertices), vertices);

    NBRE_VertexDeclaration* decalration = new NBRE_VertexDeclaration();
    NBRE_VertexElement* colorElem1 =    new NBRE_VertexElement(0, 0,                   NBRE_VertexElement::VET_FLOAT4,  NBRE_VertexElement::VES_DIFFUSE, 0);
    NBRE_VertexElement* posElem1 =      new NBRE_VertexElement(0, 4*sizeof(float),     NBRE_VertexElement::VET_FLOAT3,  NBRE_VertexElement::VES_POSITION, 0);
    NBRE_VertexElement* normalElem1 =   new NBRE_VertexElement(0, 7*sizeof(float),     NBRE_VertexElement::VET_FLOAT3,  NBRE_VertexElement::VES_NORMAL, 0);
    NBRE_VertexElement* texElem1 =      new NBRE_VertexElement(0, 10*sizeof(float),    NBRE_VertexElement::VET_FLOAT2,  NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
    decalration->GetVertexElementList().push_back(colorElem1);
    decalration->GetVertexElementList().push_back(posElem1);
    decalration->GetVertexElementList().push_back(normalElem1);
    decalration->GetVertexElementList().push_back(texElem1);

    NBRE_VertexData* vertexData = new NBRE_VertexData(3);
    vertexData->AddBuffer(vertextBuff);
    vertexData->SetVertexDeclaration(decalration);

    NBRE_Mesh* mesh = new NBRE_Mesh(vertexData);

    NBRE_HardwareIndexBuffer* indexBuff = gRenderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, 3, FALSE);
    indexBuff->WriteData(0, sizeof(indices), indices);
    NBRE_IndexData* indexData = new NBRE_IndexData(indexBuff, 0, indexBuff->GetNumIndexes());

    mesh->CreateSubMesh(indexData, NBRE_PMT_TRIANGLE_LIST);
    mesh->CalculateBoundingBox();

    CU_ASSERT(mesh != NULL);
    CU_ASSERT(mesh->GetSubMeshNumber() == 1);

    CU_ASSERT(mesh->GetSubMesh(0) != NULL);
    CU_ASSERT(mesh->GetSubMesh(0)->GetVertexData() == NULL);
    CU_ASSERT(mesh->GetSubMesh(0)->GetIndexData() == indexData);
    CU_ASSERT(mesh->GetSubMesh(0)->UseSharedVertices() == TRUE);
    CU_ASSERT(mesh->GetSubMesh(0)->GetOperationType() == NBRE_PMT_TRIANGLE_LIST);

    CU_ASSERT(mesh->GetSharedVertexedData() == vertexData);

    CU_ASSERT(mesh->AxisAlignedBox().maxExtend == boundingBox.maxExtend);
    CU_ASSERT(mesh->AxisAlignedBox().minExtend == boundingBox.minExtend);

    delete mesh;
}

MeshTest::MeshTest():NBRE_TestSuite("mesh_test", init_suite_mesh, clean_suite_mesh)
{
    AddTestCase(new NBRE_TestCase("test_create_mesh_not_use_shared_vertices", testMeshNotUseSharedVertices));
    AddTestCase(new NBRE_TestCase("test_create_mesh_use_shared_vertices", testMeshUseSharedVertices));
    AddTestCase(new NBRE_TestCase("test_create_mesh_use_shared_vertices_mixed_vertices", testMeshUseSharedVerticesAndMixedVertices));
}
