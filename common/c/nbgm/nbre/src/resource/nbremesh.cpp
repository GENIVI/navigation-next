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
#include "nbremesh.h"
#include "nbrehardwarevertexbuffer.h"
#include "nbreintersection.h"
#include "nbremath.h"
#include "nbrelog.h"

NBRE_Mesh::~NBRE_Mesh()
{
    NBRE_SubMeshList::iterator iter;
    for (iter = mSubMeshList.begin(); iter != mSubMeshList.end(); ++iter)
    {
        NBRE_DELETE *iter;
    }
    NBRE_DELETE mSharedVertexData;
    NBRE_DELETE mTriangleIterator;
}

uint32
NBRE_Mesh::GetSubMeshNumber() const
{
    return mSubMeshList.size();
}

NBRE_SubMesh*
NBRE_Mesh::GetSubMesh(uint32 index) const
{
    nbre_assert(index < mSubMeshList.size());
    return mSubMeshList[index];
}

NBRE_SubMesh*
NBRE_Mesh::CreateSubMesh(NBRE_IndexData* indexData, NBRE_PrimitiveType opertationType)
{
    NBRE_SubMesh* subMesh = NBRE_NEW NBRE_SubMesh(indexData, opertationType);
    if (subMesh != NULL)
    {
        mSubMeshList.push_back(subMesh);
    }
    return subMesh;
}

NBRE_SubMesh*
NBRE_Mesh::CreateSubMesh(NBRE_VertexData* vertexData, NBRE_IndexData* indexData, NBRE_PrimitiveType opertationType)
{
    NBRE_SubMesh* subMesh = NBRE_NEW NBRE_SubMesh(vertexData, indexData, opertationType);
    if (subMesh != NULL)
    {
        mSubMeshList.push_back(subMesh);
    }
    return subMesh;
}

const NBRE_AxisAlignedBox3f&
NBRE_Mesh::AxisAlignedBox() const
{
    return mBoundingBox;
}

const NBRE_VertexData*
NBRE_Mesh::GetSharedVertexedData() const
{
    return mSharedVertexData;
}

NBRE_VertexData*
NBRE_Mesh::GetSharedVertexedData()
{
    return mSharedVertexData;
}

void 
NBRE_Mesh::CalculateBoundingBox()
{
    /// calculate the boundingbox of sub meshes when not using share vertices
    if (mSharedVertexData == NULL)
    {
        for (uint32 i = 0; i < mSubMeshList.size(); ++i)
        {
            NBRE_SubMesh* subMesh = mSubMeshList[i];
            NBRE_VertexData* vertexData = subMesh->GetVertexData();

            const NBRE_VertexDeclaration* vertexDeclaration = subMesh->GetVertexData()->GetVertexDeclaration();
            NBRE_VertexDeclaration::VertexElementList vertexElementList = vertexDeclaration->GetVertexElementList();

            NBRE_VertexDeclaration::VertexElementList::const_iterator iter;
            for (iter = vertexElementList.begin(); iter != vertexElementList.end(); ++iter)
            {
                if ((*iter)->Semantic() == NBRE_VertexElement::VES_POSITION)
                {
                    NBRE_HardwareVertexBuffer* vertexBuffer = vertexData->GetBuffer((*iter)->Source());
                    uint32 verteicsCount = vertexBuffer->GetNumVertices();
                    uint32 stride = vertexBuffer->GetVertexSize();
                    uint8* dataBuff = static_cast<uint8*>(vertexBuffer->Lock((*iter)->Offset(), NBRE_HardwareBuffer::HBL_READ_ONLY));

                    if ((*iter)->Type() == NBRE_VertexElement::VET_FLOAT3)
                    {
                        for (uint32 i = 0; i < verteicsCount; ++i)
                        {
                            float* floatBuff = reinterpret_cast<float*>(dataBuff);
                            mBoundingBox.Merge(floatBuff[0], floatBuff[1], floatBuff[2]);
                            dataBuff += stride;
                        }
                    }
                    else if ((*iter)->Type() == NBRE_VertexElement::VET_FLOAT2)
                    {
                        for (uint32 i = 0; i < verteicsCount; ++i)
                        {
                            float* floatBuff = reinterpret_cast<float*>(dataBuff);
                            mBoundingBox.Merge(floatBuff[0], floatBuff[1], 0);
                            dataBuff += stride;
                        }
                    }

                    vertexBuffer->UnLock();
                }
            }
        }
    }
    /// calculate the bounding box of mesh which use the shared vertices
    else
    {
        const NBRE_VertexDeclaration* vertexDeclaration = mSharedVertexData->GetVertexDeclaration();
        NBRE_VertexDeclaration::VertexElementList vertexElementList = vertexDeclaration->GetVertexElementList();

        NBRE_VertexDeclaration::VertexElementList::const_iterator iter;
        for (iter = vertexElementList.begin(); iter != vertexElementList.end(); ++iter)
        {
            if ((*iter)->Semantic() == NBRE_VertexElement::VES_POSITION)
            {
                NBRE_HardwareVertexBuffer* vertexBuffer = mSharedVertexData->GetBuffer((*iter)->Source());
                uint32 verteicsCount = vertexBuffer->GetNumVertices();
                uint32 stride = vertexBuffer->GetVertexSize();
                uint8* dataBuff = static_cast<uint8*>(vertexBuffer->Lock((*iter)->Offset(), NBRE_HardwareBuffer::HBL_READ_ONLY));

                if ((*iter)->Type() == NBRE_VertexElement::VET_FLOAT3)
                {
                    for (uint32 i = 0; i < verteicsCount; ++i)
                    {
                        float* floatBuff = reinterpret_cast<float*>(dataBuff);
                        mBoundingBox.Merge(floatBuff[0], floatBuff[1], floatBuff[2]);
                        dataBuff += stride;
                    }
                }
                else if ((*iter)->Type() == NBRE_VertexElement::VET_FLOAT2)
                {
                    for (uint32 i = 0; i < verteicsCount; ++i)
                    {
                        float* floatBuff = reinterpret_cast<float*>(dataBuff);
                        mBoundingBox.Merge(floatBuff[0], floatBuff[1], 0);
                        dataBuff += stride;
                    }
                }

                vertexBuffer->UnLock();
            }
        }
    }
}

nb_boolean
NBRE_Mesh::HitTest(const NBRE_Ray3f& ray, NBRE_Point3f* pIntersectPoint)
{
    if (!NBRE_Intersectionf::HitTest(mBoundingBox, ray, NULL))
    {
        return FALSE;
    }

    if (mTriangleIterator == NULL)
    {
        mTriangleIterator = NBRE_NEW TriangleIterator(*this);
    }
    mTriangleIterator->Reset();

    NBRE_Triangle3f tr;
    while (mTriangleIterator->Next(tr))
    {
        if (NBRE_Intersectionf::HitTest(tr, ray, pIntersectPoint))
        {
            return TRUE;
        }
    }
    return FALSE;
}

NBRE_Mesh::TriangleIterator::TriangleIterator(NBRE_Mesh& mesh)
:mIndex(0)
{
    if (mesh.mSharedVertexData == NULL)
    {
        for (uint32 subMeshIndex = 0; subMeshIndex < mesh.mSubMeshList.size(); ++subMeshIndex)
        {
            NBRE_SubMesh* subMesh = mesh.mSubMeshList[subMeshIndex];
            // @TODO: Add other primitive type support if needed
            if (subMesh->GetOperationType() != NBRE_PMT_TRIANGLE_LIST)
            {
                continue;
            }
            uint32 baseIndex = mVertices.size();
            NBRE_VertexData* vertexData = subMesh->GetVertexData();

            const NBRE_VertexDeclaration* vertexDeclaration = subMesh->GetVertexData()->GetVertexDeclaration();
            NBRE_VertexDeclaration::VertexElementList vertexElementList = vertexDeclaration->GetVertexElementList();

            NBRE_VertexDeclaration::VertexElementList::const_iterator iter;
            for (iter = vertexElementList.begin(); iter != vertexElementList.end(); ++iter)
            {
                if ((*iter)->Semantic() == NBRE_VertexElement::VES_POSITION)
                {
                    NBRE_HardwareVertexBuffer* vertexBuffer = vertexData->GetBuffer((*iter)->Source());
                    uint32 verteicsCount = vertexBuffer->GetNumVertices();
                    uint32 stride = vertexBuffer->GetVertexSize();
                    uint8* dataBuff = static_cast<uint8*>(vertexBuffer->Lock((*iter)->Offset(), NBRE_HardwareBuffer::HBL_READ_ONLY));

                    if ((*iter)->Type() == NBRE_VertexElement::VET_FLOAT3)
                    {
                        for (uint32 i = 0; i < verteicsCount; ++i)
                        {
                            float* floatBuff = reinterpret_cast<float*>(dataBuff);
                            mVertices.push_back(NBRE_Vector3f(floatBuff[0], floatBuff[1], floatBuff[2]));
                            dataBuff += stride;
                        }
                    }
                    else if ((*iter)->Type() == NBRE_VertexElement::VET_FLOAT2)
                    {
                        for (uint32 i = 0; i < verteicsCount; ++i)
                        {
                            float* floatBuff = reinterpret_cast<float*>(dataBuff);
                            mVertices.push_back(NBRE_Vector3f(floatBuff[0], floatBuff[1], 0));
                            dataBuff += stride;
                        }
                    }

                    vertexBuffer->UnLock();
                }
            }

            NBRE_HardwareIndexBuffer* indexBuffer = subMesh->GetIndexData()->IndexBuffer();
            switch (indexBuffer->GetType())
            {
            case NBRE_HardwareIndexBuffer::IT_32BIT:
                {
                    uint32* pIndex = (uint32*)indexBuffer->Lock(0, NBRE_HardwareBuffer::HBL_READ_ONLY);
                    uint32 indexCount = indexBuffer->GetNumIndexes();
                    for (uint32 i = 0; i < indexCount; ++i)
                    {
                        mIndices.push_back(baseIndex + pIndex[i]);
                    }
                    indexBuffer->UnLock();
                }
                break;
            case NBRE_HardwareIndexBuffer::IT_16BIT:
                {
                    uint16* pIndex = (uint16*)indexBuffer->Lock(0, NBRE_HardwareBuffer::HBL_READ_ONLY);
                    uint32 indexCount = indexBuffer->GetNumIndexes();
                    for (uint32 i = 0; i < indexCount; ++i)
                    {
                        mIndices.push_back(baseIndex + pIndex[i]);
                    }
                    indexBuffer->UnLock();
                }
                break;
            case NBRE_HardwareIndexBuffer::IT_8BIT:
                {
                    uint8* pIndex = (uint8*)indexBuffer->Lock(0, NBRE_HardwareBuffer::HBL_READ_ONLY);
                    uint32 indexCount = indexBuffer->GetNumIndexes();
                    for (uint32 i = 0; i < indexCount; ++i)
                    {
                        mIndices.push_back(baseIndex + pIndex[i]);
                    }
                    indexBuffer->UnLock();
                }
                break;
            default:
                // Unknown format
                NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_Mesh::TriangleIterator::TriangleIterator construct failed, type=$d", indexBuffer->GetType());
                nbre_assert(0);
                break;
            }
        }
    }
    else
    {
        const NBRE_VertexDeclaration* vertexDeclaration = mesh.mSharedVertexData->GetVertexDeclaration();
        NBRE_VertexDeclaration::VertexElementList vertexElementList = vertexDeclaration->GetVertexElementList();

        NBRE_VertexDeclaration::VertexElementList::const_iterator iter;
        for (iter = vertexElementList.begin(); iter != vertexElementList.end(); ++iter)
        {
            if ((*iter)->Semantic() == NBRE_VertexElement::VES_POSITION)
            {
                NBRE_HardwareVertexBuffer* vertexBuffer = mesh.mSharedVertexData->GetBuffer((*iter)->Source());
                uint32 verteicsCount = vertexBuffer->GetNumVertices();
                uint32 stride = vertexBuffer->GetVertexSize();
                uint8* dataBuff = static_cast<uint8*>(vertexBuffer->Lock((*iter)->Offset(), NBRE_HardwareBuffer::HBL_READ_ONLY));

                if ((*iter)->Type() == NBRE_VertexElement::VET_FLOAT3)
                {
                    for (uint32 i = 0; i < verteicsCount; ++i)
                    {
                        float* floatBuff = reinterpret_cast<float*>(dataBuff);
                        mVertices.push_back(NBRE_Vector3f(floatBuff[0], floatBuff[1], floatBuff[2]));
                        dataBuff += stride;
                    }
                }
                else if ((*iter)->Type() == NBRE_VertexElement::VET_FLOAT2)
                {
                    for (uint32 i = 0; i < verteicsCount; ++i)
                    {
                        float* floatBuff = reinterpret_cast<float*>(dataBuff);
                        mVertices.push_back(NBRE_Vector3f(floatBuff[0], floatBuff[1], 0));
                        dataBuff += stride;
                    }
                }

                vertexBuffer->UnLock();
            }
        }

        for (uint32 subMeshIndex = 0; subMeshIndex < mesh.mSubMeshList.size(); ++subMeshIndex)
        {
            NBRE_SubMesh* subMesh = mesh.mSubMeshList[subMeshIndex];
            // @TODO: Add other primitive type support if needed
            if (subMesh->GetOperationType() != NBRE_PMT_TRIANGLE_LIST)
            {
                continue;
            }

            NBRE_HardwareIndexBuffer* indexBuffer = subMesh->GetIndexData()->IndexBuffer();
            switch (indexBuffer->GetType())
            {
            case NBRE_HardwareIndexBuffer::IT_32BIT:
                {
                    uint32* pIndex = (uint32*)indexBuffer->Lock(0, NBRE_HardwareBuffer::HBL_READ_ONLY);
                    uint32 indexCount = indexBuffer->GetNumIndexes();
                    for (uint32 i = 0; i < indexCount; ++i)
                    {
                        mIndices.push_back(pIndex[i]);
                    }
                    indexBuffer->UnLock();
                }
                break;
            case NBRE_HardwareIndexBuffer::IT_16BIT:
                {
                    uint16* pIndex = (uint16*)indexBuffer->Lock(0, NBRE_HardwareBuffer::HBL_READ_ONLY);
                    uint32 indexCount = indexBuffer->GetNumIndexes();
                    for (uint32 i = 0; i < indexCount; ++i)
                    {
                        mIndices.push_back(pIndex[i]);
                    }
                    indexBuffer->UnLock();
                }
                break;
            case NBRE_HardwareIndexBuffer::IT_8BIT:
                {
                    uint8* pIndex = (uint8*)indexBuffer->Lock(0, NBRE_HardwareBuffer::HBL_READ_ONLY);
                    uint32 indexCount = indexBuffer->GetNumIndexes();
                    for (uint32 i = 0; i < indexCount; ++i)
                    {
                        mIndices.push_back(pIndex[i]);
                    }
                    indexBuffer->UnLock();
                }
                break;
            default:
                // Unknown format
                NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_Mesh::TriangleIterator::TriangleIterator construct failed, type=$d", indexBuffer->GetType());
                nbre_assert(0);
                break;
            }
        }
    }
}
