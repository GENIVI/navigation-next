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

    @file nbremodel.h
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

#ifndef _NBRE_MODEL_H_
#define _NBRE_MODEL_H_

#include "nbresharedptr.h"
#include "nbremesh.h"
#include "nbresubmodel.h"

class NBRE_IVertexAnimation;
/*! Defines an instance of a discrete object described its shape and surface.
@remarks
    This class contains all the information included mesh and shader which is
    created during the build model process. From original data, we used this
    class to keep the information to describe this object. However, one model
    may have special vertex data and independent shader in some parts of it. 
    Therefor it contains a list of NBRE_SubModel to keep up these information
    as an independent object.
*/ 

class NBRE_Model
{
public:
    NBRE_Model();
    NBRE_Model(const NBRE_MeshPtr& mesh);
    ~NBRE_Model();

    const NBRE_SubModel& GetSubModel(uint32 index) const;
    NBRE_SubModel& GetSubModel(uint32 index);
    uint32 GetSubModelNumber() const;

    NBRE_MeshPtr GetMesh() { return mMesh; }
    const NBRE_MeshPtr& GetMesh() const { return mMesh; }
    void SetMesh(const NBRE_MeshPtr& mesh);
    void SetShader(const NBRE_ShaderPtr& shader);
    void SetShaderName(const NBRE_String& name);

    /// Create a sub model using shared vertices.
    NBRE_SubModel* CreateSubModel(NBRE_IndexData* indexData, NBRE_PrimitiveType opertationType);

    /// Create a sub model not using shared vertices.
    NBRE_SubModel* CreateSubModel(NBRE_VertexData* vertexData, NBRE_IndexData* indexData, NBRE_PrimitiveType opertationType);

    void SetVertexAnimation(shared_ptr<NBRE_IVertexAnimation> vertexAnimation);

    void UpdateVertexData();

    void CompleteVertextAnimation();

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_Model);

private:
    /*! The Mesh that this Model is based on.
    */
    NBRE_MeshPtr mMesh;

    /*! The list contained submodel
    */
    NBRE_SubModelList mSubModelList;

	///it's used to do vertex animation.
    shared_ptr<NBRE_IVertexAnimation> mVertexAnimation;
    uint32 mAnimationLastUpdated;
};

typedef shared_ptr<NBRE_Model> NBRE_ModelPtr;

#endif