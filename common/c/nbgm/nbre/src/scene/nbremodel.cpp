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

#include "nbremodel.h"
#include "nbresubmodel.h"
#include "nbrevertexanimation.h"

NBRE_Model::NBRE_Model()
    :mMesh(NBRE_NEW NBRE_Mesh())
    ,mAnimationLastUpdated(0)
{
    for (uint32 i = 0; i < mMesh->GetSubMeshNumber(); ++i)
    {
        NBRE_SubModel* subModel = NBRE_NEW NBRE_SubModel(mMesh->GetSubMesh(i));
        mSubModelList.push_back(subModel);
    }
}

NBRE_Model::NBRE_Model(const NBRE_MeshPtr& mesh)
    :mMesh(mesh)
    ,mAnimationLastUpdated(0)
{
    for (uint32 i = 0; i < mMesh->GetSubMeshNumber(); ++i)
    {
        NBRE_SubModel* subModel = NBRE_NEW NBRE_SubModel(mMesh->GetSubMesh(i));
        mSubModelList.push_back(subModel);
    }
}

NBRE_Model::~NBRE_Model()
{
    for (NBRE_SubModelList::iterator i = mSubModelList.begin(); i != mSubModelList.end(); ++i)
    {
        NBRE_DELETE *i;
    }
    mMesh.reset();
}

const NBRE_SubModel&
NBRE_Model::GetSubModel(uint32 index) const
{
    nbre_assert(index < mSubModelList.size());
    return *mSubModelList[index];
}

NBRE_SubModel&
NBRE_Model::GetSubModel(uint32 index)
{
    nbre_assert(index < mSubModelList.size());
    return *mSubModelList[index];
}

uint32
NBRE_Model::GetSubModelNumber() const
{
    return mSubModelList.size();
}

void
NBRE_Model::SetMesh(const NBRE_MeshPtr& mesh)
{
    mMesh = mesh;
    for (NBRE_SubModelList::iterator i = mSubModelList.begin(); i != mSubModelList.end(); ++i)
    {
        NBRE_DELETE *i;
    }
    mSubModelList.clear();
    for (uint32 i = 0; i < mMesh->GetSubMeshNumber(); ++i)
    {
        NBRE_SubModel* subModel = NBRE_NEW NBRE_SubModel(mMesh->GetSubMesh(i));
        mSubModelList.push_back(subModel);
    }
}

void
NBRE_Model::SetShader(const NBRE_ShaderPtr& shader)
{
    /// Set for all sub models
    NBRE_SubModelList::iterator i;
    for (i = mSubModelList.begin(); i != mSubModelList.end(); ++i)
    {
        (*i)->SetShader(shader);
    }
}

void
NBRE_Model::SetShaderName(const NBRE_String& name)
{
    /// Set for all sub models
    NBRE_SubModelList::iterator i;
    for (i = mSubModelList.begin(); i != mSubModelList.end(); ++i)
    {
        (*i)->SetShaderName(name);
    }
}

NBRE_SubModel*
NBRE_Model::CreateSubModel(NBRE_IndexData* indexData, NBRE_PrimitiveType opertationType)
{
    NBRE_SubMesh* subMesh = mMesh->CreateSubMesh(indexData, opertationType);
    NBRE_SubModel* subModel = NULL;
    if (subMesh != NULL)
    {
        subModel = NBRE_NEW NBRE_SubModel(subMesh);
        mSubModelList.push_back(subModel);
    }
    return subModel;
}

NBRE_SubModel*
NBRE_Model::CreateSubModel(NBRE_VertexData* vertexData, NBRE_IndexData* indexData, NBRE_PrimitiveType opertationType)
{
    NBRE_SubMesh* subMesh = mMesh->CreateSubMesh(vertexData, indexData, opertationType);
    NBRE_SubModel* subModel = NULL;
    if (subMesh != NULL)
    {
        subModel = NBRE_NEW NBRE_SubModel(subMesh);
        mSubModelList.push_back(subModel);
    }
    return subModel;
}

void
NBRE_Model::SetVertexAnimation(shared_ptr<NBRE_IVertexAnimation> vertexAnimation)
{
    mVertexAnimation = vertexAnimation;
}

void
NBRE_Model::UpdateVertexData()
{
    if(mVertexAnimation)
    {
        if(mAnimationLastUpdated != mVertexAnimation->GetDirtyFrameNumber())
        {
            for (NBRE_SubModelList::iterator i = mSubModelList.begin(); i != mSubModelList.end(); ++i)
            {
                NBRE_SubModel* subModel = *i;
                if(subModel->GetSubMesh()->GetVertexData())
                {
                    mVertexAnimation->Animate(*subModel->GetSubMesh()->GetVertexData(), *subModel->GetSubMesh()->GetIndexData());
                }
            }
            if(mMesh->GetSharedVertexedData())
            {
                mVertexAnimation->Animate(*mMesh->GetSharedVertexedData(), *mMesh->GetSubMesh(0)->GetIndexData());
            }
            mAnimationLastUpdated = mVertexAnimation->GetDirtyFrameNumber();
        }
    }
}

void
NBRE_Model::CompleteVertextAnimation()
{
    if(mVertexAnimation)
    {
        mVertexAnimation->AnimationComplete();
    }
}
