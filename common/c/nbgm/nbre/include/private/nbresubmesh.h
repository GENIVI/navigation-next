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

    @file nbresubmesh.h
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
#ifndef _NBRE_SUB_MESH_H_
#define _NBRE_SUB_MESH_H_

#include "nbretypes.h"
#include "nbrerenderpal.h"
#include "nbrevertexindexdata.h"

/*! \addtogroup NBRE_Resource
*  @{
*/
/*! Defines a part of a complete mesh.
    @remarks
        Meshes which make up the definition of a discrete 3D object
        are made up of potentially multiple parts. This is because
        different parts of the mesh may use different materials or
        use different vertex formats, such that a rendering state
        change is required between them.
    @par
        Like the NBRE_Mesh class, instantiations of 3D objects in the scene
        share the NBRE_SubMesh instances, and have the option of overriding
        their material differences on a per-object basis if required.
        See the SubEntity class for more information.
*/
class NBRE_SubMesh
{
friend class NBRE_Mesh;

public:
    const NBRE_VertexData* GetVertexData() const {return mVertexData;}
    const NBRE_IndexData* GetIndexData() const {return mIndexData;}

    NBRE_VertexData* GetVertexData() {return mVertexData;}
    NBRE_IndexData* GetIndexData() {return mIndexData;}

    nb_boolean UseSharedVertices() const {return mUseSharedVertices;}
    NBRE_PrimitiveType GetOperationType() const {return mOpertationType;}
    


private:
    /// Create a sub mesh using shared vertices.
    NBRE_SubMesh(NBRE_IndexData* indexData, NBRE_PrimitiveType opertationType);

    /// Create a sub mesh not using shared vertices.
    NBRE_SubMesh(NBRE_VertexData* vertexData, NBRE_IndexData* indexData, NBRE_PrimitiveType opertationType);

    ~NBRE_SubMesh();

    DISABLE_COPY_AND_ASSIGN(NBRE_SubMesh);

private:

    /// The vertices used to render this sub mesh
    NBRE_VertexData* mVertexData;
    /// Indicates if this sub mesh shares vertex data with other sub meshes or whether it has it's own vertices.
    nb_boolean mUseSharedVertices;

    /// Face index data
    NBRE_IndexData* mIndexData;

    /// The render operation type used to render this sub mesh
    NBRE_PrimitiveType mOpertationType;

};

/*! @} */
#endif
