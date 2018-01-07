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

#include "nbrevertexindexdata.h"
#include "nbrelog.h"

NBRE_VertexElement::NBRE_VertexElement(uint32 source, int offset, VertexElementType type, VertexElementSemantic semantic, uint32 index)
    :mSource(source), mOffset(offset), mType(type), mSemantic(semantic), mIndex(index)
{

};

NBRE_VertexElement::~NBRE_VertexElement()
{

}

uint16
NBRE_VertexElement::GetTypeCount(NBRE_VertexElement::VertexElementType etype)
{
    switch (etype)
    {
    case NBRE_VertexElement::VET_COLOUR:
    case NBRE_VertexElement::VET_COLOUR_ABGR:
    case NBRE_VertexElement::VET_COLOUR_ARGB:
        return 1;
    case NBRE_VertexElement::VET_FLOAT1:
        return 1;
    case NBRE_VertexElement::VET_FLOAT2:
        return 2;
    case NBRE_VertexElement::VET_FLOAT3:
        return 3;
    case NBRE_VertexElement::VET_FLOAT4:
        return 4;
    case NBRE_VertexElement::VET_SHORT1:
        return 1;
    case NBRE_VertexElement::VET_SHORT2:
        return 2;
    case NBRE_VertexElement::VET_SHORT3:
        return 3;
    case NBRE_VertexElement::VET_SHORT4:
        return 4;
    case NBRE_VertexElement::VET_UBYTE4:
        return 4;
    }
    NBRE_DebugLog(PAL_LogSeverityCritical, "NBRE_VertexElement::GetTypeCount failed, etype=%d", etype); 
    nbre_assert(FALSE);
    return 0;
}

NBRE_VertexDeclaration::NBRE_VertexDeclaration()
{

}

NBRE_VertexDeclaration::~NBRE_VertexDeclaration()
{
    NBRE_VertexDeclaration::VertexElementList::iterator elem, elemEnd;
    elemEnd = mElementList.end();
    for (elem = mElementList.begin(); elem != elemEnd;)
    {
        NBRE_DELETE (*elem);
        mElementList.erase( elem ++);
    }
}

const NBRE_VertexDeclaration::VertexElementList&
NBRE_VertexDeclaration::GetVertexElementList() const
{
    return mElementList;
}

NBRE_VertexDeclaration::VertexElementList&
NBRE_VertexDeclaration::GetVertexElementList()
{
    return mElementList;
}

void 
NBRE_VertexDeclaration::ClearVertexElementList()
{
    NBRE_VertexDeclaration::VertexElementList::iterator elemEnd = mElementList.end();
    for (NBRE_VertexDeclaration::VertexElementList::iterator elem = mElementList.begin(); elem != elemEnd; ++elem)
    {
        NBRE_DELETE (*elem);
    }
    mElementList.clear();
}

NBRE_IndexData::NBRE_IndexData(NBRE_HardwareIndexBuffer* dataBuffer, uint32 start, uint32 count)
    :mIndexBuffer(dataBuffer), mIndexStart(start), mIndexCount(count)
{

}

NBRE_IndexData::~NBRE_IndexData()
{
    NBRE_DELETE mIndexBuffer;
}

NBRE_VertexData::NBRE_VertexData(uint32 vertexCount)
    :mVertexDeclaration(NULL),mVertexStart(0),mVertexCount(vertexCount)
{

}

NBRE_VertexData::~NBRE_VertexData()
{
    for(uint32 i=0; i<mBufferList.size(); ++i)
    {
        NBRE_DELETE mBufferList[i];
    }
    delete mVertexDeclaration;
}

const NBRE_HardwareVertexBuffer*
NBRE_VertexData::GetBuffer(uint32 source) const
{
    if(source<mBufferList.size())
    {
        return mBufferList[source];
    }
    NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_VertexData::GetBuffer, out of range: size=%d, source=%d", mBufferList.size(), source);
    return NULL;
}

NBRE_HardwareVertexBuffer*
NBRE_VertexData::GetBuffer(uint32 source)
{
    if(source<mBufferList.size())
    {
        return mBufferList[source];
    }
    NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_VertexData::GetBuffer, out of range: size=%d, source=%d", mBufferList.size(), source);
    return NULL;
}

uint32
NBRE_VertexData::GetBufferCount() const
{
    return mBufferList.size();
}


void
NBRE_VertexData::AddBuffer(NBRE_HardwareVertexBuffer* buffer)
{
    mBufferList.push_back(buffer);
    if(this->mVertexCount == 0)
    {
        mVertexCount = buffer->GetNumVertices();
    }
}

void
NBRE_VertexData::AddBuffer(uint32 index, NBRE_HardwareVertexBuffer* buffer)
{
    nbre_assert(index >= 0 && index <= mBufferList.size());
    BufferList::iterator it = mBufferList.begin();
    it += index;
    mBufferList.insert(it, buffer);
    if(this->mVertexCount == 0)
    {
        mVertexCount = buffer->GetNumVertices();
    }
}

void
NBRE_VertexData::SetBuffer(NBRE_HardwareVertexBuffer* buffer, uint32 source)
{
    if(source<mBufferList.size())
    {
        NBRE_DELETE mBufferList[source];
        mBufferList[source] = buffer;
    }
    else
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_VertexData::GetBuffer, out of range: size=%d, source=%d", mBufferList.size(), source);
    }
}

const
NBRE_VertexDeclaration* NBRE_VertexData::GetVertexDeclaration() const
{
    return mVertexDeclaration;
}


NBRE_VertexDeclaration* NBRE_VertexData::GetVertexDeclaration()
{
    return mVertexDeclaration;
}

void
NBRE_VertexData::SetVertexDeclaration(NBRE_VertexDeclaration* decl)
{
    NBRE_DELETE mVertexDeclaration;
    mVertexDeclaration = decl;
}
