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

#include "entity_test.h"
#include "palstdlib.h"

#include "nbreentity.h"
#include "nbresubentity.h"
#include "nbrehardwarevertexbuffer.h"
#include "nbrehardwareindexbuffer.h"
#include "nbreglrenderpal.h"

static NBRE_IRenderPal* gRenderPal = NULL;

/* The suite initialization function.
 * Opens the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 * Case name is mesh_test.
 */
int init_suite_entity(void)
{
    gRenderPal = NBRE_NEW NBRE_GlRenderPal();
    return 0;
}

/* The suite cleanup function.
 * Closes the temporary file used by the tests.
 * Returns zero on success, non-zero otherwise.
 */
int clean_suite_entity(void)
{
    NBRE_DELETE gRenderPal;
    return 0;
}

/* Simple test of testEntity().
 */
void testEntity(void)
{
    /// Create a mesh which used the shared vertices
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

    NBRE_AxisAlignedBox3d boundingBox(-100.0f, -100.0f, 0.0f, 100.0f, 0.0f, 0.0f);

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

    /// Create a Entity
    NBRE_MeshPtr meshPrt;
    meshPrt.Bind(mesh_2, NBRE_SPFM_DELETE);

    NBRE_Context context;
    nsl_memset(&context, 0, sizeof(NBRE_Context));

    NBRE_Entity* entity = new NBRE_Entity(context, meshPrt, 0);

    CU_ASSERT(entity != NULL);
    CU_ASSERT(entity->GetSubEntityNumber() == 2);

    CU_ASSERT(entity->AxisAlignedBox().maxExtend == boundingBox.maxExtend);
    CU_ASSERT(entity->AxisAlignedBox().minExtend == boundingBox.minExtend);

    entity->SetVisible(FALSE);
    CU_ASSERT(entity->Visible() == FALSE);
    entity->SetVisible(TRUE);
    CU_ASSERT(entity->Visible() == TRUE);

    CU_ASSERT(entity->GetSubEntity(0).SubMesh()->GetIndexData() == indexData_3);
    CU_ASSERT(entity->GetSubEntity(1).SubMesh()->GetIndexData() == indexData_4);
    CU_ASSERT(entity->GetSubEntity(0).SubMesh()->GetVertexData() == NULL);
    CU_ASSERT(entity->GetSubEntity(1).SubMesh()->GetVertexData() == NULL);

    NBRE_Shader* shader = new NBRE_Shader();
    NBRE_ShaderPtr shaderPtr;
    shaderPtr.Bind(shader, NBRE_SPFM_DELETE);

    NBRE_Pass* pass = new NBRE_Pass();
    NBRE_PassPtr passPtr;
    passPtr.Bind(pass, NBRE_SPFM_DELETE);
    shaderPtr->AddPass(passPtr);

    entity->SetShader(shaderPtr);

    NBRE_DELETE entity;
}

EntityTest::EntityTest():NBRE_TestSuite("entity_test", init_suite_entity, clean_suite_entity)
{
    AddTestCase(new NBRE_TestCase("test_create_entity", testEntity));
}
