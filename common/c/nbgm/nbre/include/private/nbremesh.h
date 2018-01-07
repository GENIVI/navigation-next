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

    @file nbremesh.h
*/
/*
    (C) Copyright 2012 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _NBRE_MESH_H_
#define _NBRE_MESH_H_

#include "nbretypes.h"
#include "nbresharedptr.h"
#include "nbreaxisalignedbox3.h"
#include "nbrevertexindexdata.h"
#include "nbresubmesh.h"
#include "nbretriangle3.h"

/*! \addtogroup NBRE_Resource
*  @{
*/
/*! Resource holding data about 3D mesh.
    @remarks
        This class holds the data used to represent a discrete
        3-dimensional object. NBRE_Mesh data usually contains more
        than just vertices and triangle information; it also
        includes references to materials (and the faces which use them),
        level-of-detail reduction information, and will add
        skeleton/bones information, key frame animation etc.
        However, it is important to note the emphasis on the word
        'discrete' here. This class does not cover the large-scale
        sprawling geometry found in level / landscape data.
    @par
        Multiple world objects can (indeed should) be created from a
        single mesh object - see the NBRE_Entity class for more info.
        The mesh object will have it's own default
        material properties, but potentially each world instance may
        wish to customize the materials from the original. When the object
        is instantiated into a scene node, the mesh material properties
        will be taken by default but may be changed. These properties
        are actually held at the SubMesh level since a single mesh may
        have parts with different materials.
*/

class NBRE_Mesh
{
    typedef NBRE_Vector<NBRE_SubMesh*> NBRE_SubMeshList;

public:
    /*! Create a mesh.
    @remarks
    When all the sub meshes in the mesh not using the shared vertices,
    use this constructor.
    mSharedVertexData will useless.
    */
    NBRE_Mesh():mSharedVertexData(NULL), mTriangleIterator(NULL) {}

    /*! Create a mesh.
    @remarks
    When all the sub meshes in the mesh using the shared vertices,
    use this constructor.
    The shared vertices pointer will be binded to mSharedVertexData.
    */
    explicit NBRE_Mesh(NBRE_VertexData* sharedVertexData):mSharedVertexData(sharedVertexData), mTriangleIterator(NULL) {}

    ~NBRE_Mesh();

public:
    /** Creates a new SubMesh.
        @remarks
            Method for manually creating geometry for the mesh.
            Note - use with extreme caution - you must be sure that
            you have set up the geometry properly.
    */

    /// Create a sub mesh using shared vertices.
    NBRE_SubMesh* CreateSubMesh(NBRE_IndexData* indexData, NBRE_PrimitiveType opertationType);

    /// Create a sub mesh not using shared vertices.
    NBRE_SubMesh* CreateSubMesh(NBRE_VertexData* vertexData, NBRE_IndexData* indexData, NBRE_PrimitiveType opertationType);

    uint32 GetSubMeshNumber() const;

    NBRE_SubMesh* GetSubMesh(uint32 index) const;

    void CalculateBoundingBox();

    const NBRE_AxisAlignedBox3f& AxisAlignedBox() const;

    const NBRE_VertexData* GetSharedVertexedData() const;
    NBRE_VertexData* GetSharedVertexedData();

    /// Ray-mesh hit test. Note ray must be transformed to mesh model coordinates.
    nb_boolean HitTest(const NBRE_Ray3f& ray, NBRE_Point3f* pIntersectPoint);

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_Mesh);

private:
    /** Mesh triangle iterator. To improve performance,
        it should be constructed once and used multiple times.
    */
    class TriangleIterator
    {
        /// Current index to index buffer
        uint32 mIndex;
        /// Vertex buffer
        NBRE_Vector<NBRE_Vector3f> mVertices;
        /// Index buffer
        NBRE_Vector<uint32> mIndices;

    public:
        TriangleIterator(NBRE_Mesh& mesh);
        
        /// Reset iterator pointer to beginning
        void Reset()
        {
            mIndex = 0;
        }

        /// Get next triangle, return FALSE when reach end, else return TRUE
        nb_boolean Next(NBRE_Triangle3f& tri)
        {
            if (mIndex < mIndices.size())
            {
                tri.vertices[0] = mVertices[mIndices[mIndex + 0]];
                tri.vertices[1] = mVertices[mIndices[mIndex + 1]];
                tri.vertices[2] = mVertices[mIndices[mIndex + 2]];
                mIndex += 3;
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
    };
    friend class TriangleIterator;

private:
    /*! Shared vertex data.
        @remarks
            This vertex data can be shared among multiple submeshes. SubMeshes may not have
            their own VertexData, they may share this one.
    */
    NBRE_VertexData* mSharedVertexData;

    /*! A list of submeshes which make up this mesh.
        @remarks
        Each mesh is made up of 1 or more submeshes, which
        are each based on a single material and can have their
        own vertex data (they may not - they can share vertex data
        from the Mesh, depending on preference).
    */
    NBRE_SubMeshList mSubMeshList;

    NBRE_AxisAlignedBox3f mBoundingBox;

    TriangleIterator* mTriangleIterator;
};

typedef shared_ptr<NBRE_Mesh> NBRE_MeshPtr;

/*! @} */
#endif
