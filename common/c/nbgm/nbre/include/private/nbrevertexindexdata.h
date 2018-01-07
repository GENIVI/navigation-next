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

/*!--------------------------------------------------------------------------

    @file nbrevertexindexdata.h
*/
/*
    (C) Copyright 2010 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef _NBRE_INDEX_VERTEX_DATA_H_
#define _NBRE_INDEX_VERTEX_DATA_H_
#include "nbretypes.h"
#include "nbrehardwarevertexbuffer.h"
#include "nbrehardwareindexbuffer.h"
#include "nbrecommon.h"

/** \addtogroup NBRE_RenderPal
*  @{
*/
/** This class declares the usage of a single vertex buffer as a component
    of a complete NBRE_VertexDeclaration.
*/
class NBRE_VertexElement 
{
public:
	enum VertexElementSemantic 
    {
		/// Position, 3 reals per vertex
		VES_POSITION = 1,
		/// Blending weights
		VES_BLEND_WEIGHTS = 2,
        /// Blending indices
        VES_BLEND_INDICES = 3,
		/// Normal, 3 reals per vertex
		VES_NORMAL = 4,
		/// Diffuse colors
		VES_DIFFUSE = 5,
		/// Specular colors
		VES_SPECULAR = 6,
		/// Texture coordinates
		VES_TEXTURE_COORDINATES = 7,
        /// Binormal (Y axis if normal is Z)
        VES_BINORMAL = 8,
        /// Tangent (X axis if normal is Z)
        VES_TANGENT = 9,
        ///
        VES_USER_DEFINE

	};

    /// Vertex element type, used to identify the base types of the vertex contents
    enum VertexElementType
    {
        VET_FLOAT1 = 0,
        VET_FLOAT2 = 1,
        VET_FLOAT3 = 2,
        VET_FLOAT4 = 3,
        /// alias to more specific color type - use the current rendersystem's color packing
		VET_COLOUR = 4,
		VET_SHORT1 = 5,
		VET_SHORT2 = 6,
		VET_SHORT3 = 7,
		VET_SHORT4 = 8,
        VET_UBYTE4 = 9,
        /// D3D style compact color
        VET_COLOUR_ARGB = 10,
        /// GL style compact color
        VET_COLOUR_ABGR = 11
    };

public:
    NBRE_VertexElement(uint32 source, int offset, VertexElementType type, VertexElementSemantic semantic, uint32 index);
    ~NBRE_VertexElement();

public:
    static uint16 GetTypeCount(VertexElementType etype);

public:
    uint32 Source() const { return mSource; }
    uint32 Offset() const { return mOffset; }
    VertexElementType Type() const { return mType; }
    VertexElementSemantic Semantic() const { return mSemantic; }
    uint32 Index() const { return mIndex; }

private:
    /// The source vertex buffer, as bound to an index using VertexBufferBinding
    uint32 mSource;
    /// The offset in the buffer that this element starts at
    uint32 mOffset;
    /// The type of element
    VertexElementType mType;
    /// The meaning of the element
    VertexElementSemantic mSemantic;
    /// Index of the item, only applicable for some elements like texture coords
    uint32 mIndex;
};

/** This class declares the format of a set of vertex inputs, which
    can be issued to the rendering API through a RenderOperation.
*/
class NBRE_VertexDeclaration
{
public:
    typedef NBRE_List<NBRE_VertexElement*> VertexElementList;

public:
    NBRE_VertexDeclaration();
    virtual ~NBRE_VertexDeclaration();

public:
    const VertexElementList& GetVertexElementList() const;
    VertexElementList& GetVertexElementList();
    void ClearVertexElementList();

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_VertexDeclaration);

protected:
    VertexElementList mElementList;
};

/** Summary class collecting together index data source information. */
class NBRE_IndexData
{
public:
    NBRE_IndexData(NBRE_HardwareIndexBuffer* dataBuffer, uint32 start, uint32 count);
    ~NBRE_IndexData();

public:
    NBRE_HardwareIndexBuffer* IndexBuffer() const { return mIndexBuffer; }
    uint32 IndexStart() const { return mIndexStart; }
    uint32 IndexCount() const { return mIndexCount; }
    void SetIndexCount(uint32 count) { mIndexCount = count; }
    void SetIndexStart(uint32 start) { mIndexStart = start; }

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_IndexData);

private:
    /// pointer to the HardwareIndexBuffer to use, must be specified if useIndexes = true
    NBRE_HardwareIndexBuffer* mIndexBuffer;
    /// index in the buffer to start from for this operation
    uint32 mIndexStart;
    /// The number of indexes to use from the buffer
    uint32 mIndexCount;
};

/** Summary class collecting together vertex source information. */
class NBRE_VertexData
{
public:
    explicit NBRE_VertexData(uint32 vertexCount);
    ~NBRE_VertexData();

public:
    const NBRE_HardwareVertexBuffer* GetBuffer(uint32 source) const;
    NBRE_HardwareVertexBuffer* GetBuffer(uint32 source);
    uint32 GetBufferCount() const;

    void AddBuffer(NBRE_HardwareVertexBuffer* buffer);
    void AddBuffer(uint32 index, NBRE_HardwareVertexBuffer* buffer);
    void SetBuffer(NBRE_HardwareVertexBuffer* buffer, uint32 source);

    void SetVertexDeclaration(NBRE_VertexDeclaration* decl);
    const NBRE_VertexDeclaration* GetVertexDeclaration() const;
    NBRE_VertexDeclaration* GetVertexDeclaration();

    void SetVertexStart(uint32 start)  { mVertexStart = start; }
    void SetVertexCount(uint32 count)  { mVertexCount = count; }
    uint32 GetVertexStart()const       { return mVertexStart; }
    uint32 GetVertexCount()const       { return mVertexCount; }

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_VertexData);

private:
    typedef NBRE_Vector<NBRE_HardwareVertexBuffer*> BufferList;

	NBRE_VertexDeclaration* mVertexDeclaration;
    BufferList mBufferList;
    /// The base vertex index to start from
    uint32 mVertexStart;
    /// The number of vertices used in this operation
    uint32 mVertexCount;
};

/** @} */
#endif
