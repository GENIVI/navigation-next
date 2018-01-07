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
#include "nbrecollisionobjectbuilder2d.h"
#include "nbrecollisionaabb2d.h"
#include "nbreprimitivecollisionobject2d.h"
#include "nbrecompositecollisionobject2d.h"
#include "nbrecollisionpolygon2d.h"
#include "nbrecollisionlinesegment2d.h"
#include "nbrevertexindexdata.h"
#include "nbrecollisioncircle2d.h"

NBRE_CollisionObject2d*
NBRE_CollisionObjectBuilder2d::CreateAABB2d(void* owner, const NBRE_AxisAlignedBox2d& aabb, uint32 mask)
{
    NBRE_PrimitiveCollisionObject2d* po = NBRE_NEW NBRE_PrimitiveCollisionObject2d(owner, NBRE_NEW NBRE_CollisionAABB2d(aabb));
    po->SetMask(mask);
    return po;
}

NBRE_CollisionObject2d*
NBRE_CollisionObjectBuilder2d::CreatePolygon2d(void* owner, const NBRE_Vector2d* vertices, uint32 vertexCount, uint32 mask)
{
    NBRE_PrimitiveCollisionObject2d* po = NBRE_NEW NBRE_PrimitiveCollisionObject2d(owner, NBRE_NEW NBRE_CollisionPolygon2d(vertices, vertexCount));
    po->SetMask(mask);
    return po;
}

NBRE_CollisionObject2d*
NBRE_CollisionObjectBuilder2d::CreatePolyline2d(void* owner, const NBRE_Polyline2d& polyline, double width, NBRE_IBoundTree2d* boundTree, uint32 mask)
{
    NBRE_CompositeCollisionObject2d* co = NBRE_NEW NBRE_CompositeCollisionObject2d(owner, boundTree);
    for (uint32 i = 1; i < polyline.VertexCount(); ++i)
    {
        const NBRE_Vector2d& p0 = polyline.Vertex(i - 1);
        const NBRE_Vector2d& p1 = polyline.Vertex(i);
        NBRE_Vector2d d((p1 - p0).Perpendicular());
        d.Normalise();
        d *= width * 0.5;
        NBRE_Vector2d vs[4];
        vs[0] = p0 - d;
        vs[1] = p1 - d;
        vs[2] = p1 + d;
        vs[3] = p0 + d;
        co->AddPrimitive(NBRE_NEW NBRE_CollisionPolygon2d(vs, 4));
    }
    co->Build();
    co->SetMask(mask);
    return co;
}

NBRE_CollisionObject2d*
NBRE_CollisionObjectBuilder2d::CreatePolyline2d(void* owner, const NBRE_Polyline2d& polyline, NBRE_IBoundTree2d* boundTree, uint32 mask)
{
    NBRE_CompositeCollisionObject2d* co = NBRE_NEW NBRE_CompositeCollisionObject2d(owner, boundTree);
    for (uint32 i = 1; i < polyline.VertexCount(); ++i)
    {
        const NBRE_Vector2d& p0 = polyline.Vertex(i - 1);
        const NBRE_Vector2d& p1 = polyline.Vertex(i);
        co->AddPrimitive(NBRE_NEW NBRE_CollisionLineSegment2d(p0, p1));
    }
    co->Build();
    co->SetMask(mask);
    return co;
}

NBRE_CollisionObject2d*
NBRE_CollisionObjectBuilder2d::CreateTriangleMesh2d(void* owner, NBRE_VertexData* vertexData, NBRE_IndexData* indexData, NBRE_IBoundTree2d* boundTree, uint32 mask)
{   
    uint32 indexCount = indexData->IndexCount();
    uint32 indexStart = indexData->IndexStart();
    NBRE_HardwareIndexBuffer* indexBuffer = indexData->IndexBuffer();
    NBRE_VertexDeclaration* decl = vertexData->GetVertexDeclaration();
    NBRE_VertexDeclaration::VertexElementList& decls = decl->GetVertexElementList();
    NBRE_HardwareVertexBuffer* vertexBuffer = NULL;
    uint32 vertexOffset = 0;
    for (NBRE_VertexDeclaration::VertexElementList::iterator it = decls.begin(); it != decls.end(); ++it)
    {
        NBRE_VertexElement* ve = *it;
        if (ve->Semantic() == NBRE_VertexElement::VES_POSITION)
        {
            if (ve->Type() == NBRE_VertexElement::VET_FLOAT2
                || ve->Type() == NBRE_VertexElement::VET_FLOAT3)
            {
                vertexBuffer = vertexData->GetBuffer(ve->Source());
                vertexOffset = ve->Offset();
            }
            break;
        }
    }

    if (vertexBuffer == NULL)
    {
        return NULL;
    }

    NBRE_CompositeCollisionObject2d* co = NBRE_NEW NBRE_CompositeCollisionObject2d(owner, boundTree);
    if (indexBuffer->GetType() == NBRE_HardwareIndexBuffer::IT_16BIT)
    {
        uint8* pVertex = (uint8*)vertexBuffer->Lock(0, NBRE_HardwareBuffer::HBL_READ_ONLY);
        uint32 vertexSize= vertexBuffer->GetVertexSize();
        uint16* pIndex = (uint16*)indexBuffer->Lock(indexStart * sizeof(uint16), NBRE_HardwareBuffer::HBL_READ_ONLY);
        for (uint32 i = 0; i < indexCount; i += 3)
        {
            NBRE_Vector2d vs[3];
            float* p0 = (float*)(pVertex + pIndex[0] * vertexSize + vertexOffset);
            float* p1 = (float*)(pVertex + pIndex[1] * vertexSize + vertexOffset);
            float* p2 = (float*)(pVertex + pIndex[2] * vertexSize + vertexOffset);
            vs[0].x = p0[0]; vs[0].y = p0[1];
            vs[1].x = p1[0]; vs[1].y = p1[1];
            vs[2].x = p2[0]; vs[2].y = p2[1];
            co->AddPrimitive(NBRE_NEW NBRE_CollisionPolygon2d(vs, 3));
            pIndex += 3;
        }
        indexBuffer->UnLock();
        vertexBuffer->UnLock();
    }

    co->Build();
    co->SetMask(mask);
    return co;
}

NBRE_CollisionObject2d*
NBRE_CollisionObjectBuilder2d::CreateCircle2d(void* owner, const NBRE_Vector2d& center, double radius, uint32 mask)
{
    NBRE_PrimitiveCollisionObject2d* po = NBRE_NEW NBRE_PrimitiveCollisionObject2d(owner, NBRE_NEW NBRE_CollisionCircle2d(center, radius));
    po->SetMask(mask);
    return po;
}
