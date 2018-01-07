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
#include "nbreutility.h"
#include "nbgmcontext.h"
#include "nbrecommon.h"
#include "nbremesh.h"
#include "nbrerenderengine.h"

#define URI_NAME_LEN 256

NBRE_String
NBRE_Utility::FormatString(const char* fmt, ...) 
{ 
    va_list argptr; 
    char* buf = NBRE_NEW char[URI_NAME_LEN]; 
    nsl_memset(buf, 0, URI_NAME_LEN);

    va_start(argptr, fmt); 
    nsl_vsnprintf(buf, URI_NAME_LEN-1, fmt, argptr); 
    va_end(argptr); 

    NBRE_String s(buf); 
    delete[] buf;     
    return s; 
}

NBRE_Mesh*
NBRE_Utility::CreateRect3D(NBGM_Context& context, float length, float width, float height)
{
    const uint32 VERTEX_COUNT = 4;
    const uint32 INDEX_COUNT = 6;

    const float LENGHT = length * 0.5f; 
    const float WIDTH = width;
    const float HEIGHT = height * 0.5f;

    float vertices[] =
    {
       -LENGHT, -HEIGHT, WIDTH,
        LENGHT, -HEIGHT, WIDTH,
        LENGHT,  HEIGHT, WIDTH,
       -LENGHT,  HEIGHT, WIDTH
    };

    float texCoords[] =
    {
        1.f, 0.f,
        1.f, 1.f,
        0.f, 1.f,
        0.f, 0.f
    };

    uint16 indices[] =
    {
        0, 1, 2,
        0, 2, 3
    };

    NBRE_IRenderPal *renderPal = context.renderingEngine->Context().mRenderPal;

    NBRE_VertexDeclaration* decalration = renderPal->CreateVertexDeclaration();
    NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem);
    NBRE_VertexElement* texCoordElem = NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
    decalration->GetVertexElementList().push_back(texCoordElem);

    NBRE_HardwareVertexBuffer* vertextBuff = renderPal->CreateVertexBuffer(sizeof(float) * 3, VERTEX_COUNT, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    vertextBuff->WriteData(0, sizeof(float) * 3 * VERTEX_COUNT, vertices, TRUE);
    NBRE_HardwareVertexBuffer* texCoordBuff = renderPal->CreateVertexBuffer(sizeof(float) * 2, VERTEX_COUNT, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    texCoordBuff->WriteData(0, sizeof(float) *  2* VERTEX_COUNT, texCoords, TRUE);

    NBRE_VertexData* vertexData = NBRE_NEW NBRE_VertexData(0);
    vertexData->AddBuffer(vertextBuff);
    vertexData->AddBuffer(texCoordBuff);
    vertexData->SetVertexDeclaration(decalration);

    NBRE_HardwareIndexBuffer* indexBuff = renderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, INDEX_COUNT, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    indexBuff->WriteData(0, sizeof(uint16) * INDEX_COUNT, indices, TRUE);
    NBRE_IndexData* indexData = NBRE_NEW NBRE_IndexData(indexBuff, 0, indexBuff->GetNumIndexes());

    NBRE_Mesh* mesh = NBRE_NEW NBRE_Mesh(vertexData);
    mesh->CreateSubMesh(indexData, NBRE_PMT_TRIANGLE_LIST);
    mesh->CalculateBoundingBox();

    return mesh;
}
