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
#include "nbreentity.h"
#include "nbrecommon.h"
#include "nbredefaultpass.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbretypeconvert.h"
#include "nbresurfacemanager.h"

NBRE_Entity::NBRE_Entity(NBRE_Context& context, int32 overlayId):
    mContext(context),
    mVisible(TRUE),
    mCurrentPass(0),
    mOverlayId(overlayId),
    mParentNode(NULL),
    mPickListener(NULL),
    mPriority(0)
{
}

NBRE_Entity::NBRE_Entity(NBRE_Context& context, const NBRE_ModelPtr& model, int32 overlayId):
    mContext(context),
    mModel(model),
    mVisible(TRUE),
    mCurrentPass(0),
    mOverlayId(overlayId),
    mParentNode(NULL),
    mPickListener(NULL),
    mPriority(0)
{
    for (uint32 i = 0; i < mModel->GetSubModelNumber(); ++i)
    {
        NBRE_SubEntity* subEntity = NBRE_NEW NBRE_SubEntity(mContext, &mModel->GetSubModel(i));
        mSubEntityList.push_back(subEntity);
    }
}

NBRE_Entity::~NBRE_Entity()
{
    for (NBRE_SubEntityList::iterator i = mSubEntityList.begin(); i != mSubEntityList.end(); ++i)
    {
        NBRE_DELETE *i;
    }
    mModel.reset();
}

const NBRE_AxisAlignedBox3d&
NBRE_Entity::AxisAlignedBox() const
{
    return mAxisAlignedBox;
}

const NBRE_AxisAlignedBox3d&
NBRE_Entity::AxisAlignedBox()
{
    mAxisAlignedBox = NBRE_TypeConvertd::Convert(mModel->GetMesh()->AxisAlignedBox());
    NBRE_Transformationd::Transform(mParentNode->WorldTransform(), mAxisAlignedBox);
    return mAxisAlignedBox;
}

const NBRE_NoneAxisAlignedBox3d&
NBRE_Entity::BoundingBox()const
{
    return mBoundingBox;
}

const NBRE_SubEntity&
NBRE_Entity::GetSubEntity(uint32 index) const
{
    nbre_assert(index < mSubEntityList.size());
    return *mSubEntityList[index];
}

NBRE_SubEntity&
NBRE_Entity::GetSubEntity(uint32 index)
{
    nbre_assert(index < mSubEntityList.size());
    return *mSubEntityList[index];
}

uint32
NBRE_Entity::GetSubEntityNumber() const
{
    return mSubEntityList.size();
}

void
NBRE_Entity::SetShader(const NBRE_ShaderPtr& shader)
{
    /// Set for all sub entities
    NBRE_SubEntityList::iterator i;
    for (i = mSubEntityList.begin(); i != mSubEntityList.end(); ++i)
    {
        (*i)->SetShader(shader);
    }
}

void
NBRE_Entity::SetShaderName(const NBRE_String& name)
{
    /// Set for all sub entities
    NBRE_SubEntityList::iterator i;
    for (i = mSubEntityList.begin(); i != mSubEntityList.end(); ++i)
    {
        (*i)->SetShaderName(name);
    }
}

nb_boolean
NBRE_Entity::Visible() const
{
    return mVisible;
}

void
NBRE_Entity::SetVisible( nb_boolean val )
{
    mVisible = val;
}

NBRE_PassPtr
NBRE_Entity::GetCachedPass()
{
    if(mCurrentPass < static_cast<int32>(mPassCaches.size()))
    {
        return mPassCaches[mCurrentPass++];
    }
    ++mCurrentPass;
    NBRE_PassPtr pass = NBRE_PassPtr(NBRE_NEW NBRE_Pass());
    mPassCaches.push_back(pass);
    return pass;
}

void
NBRE_Entity::GetRenderOperation(NBRE_SubEntity* subEntity, NBRE_RenderOperation& renderOp, const NBRE_PassPtr& pass)
{
    renderOp.SetIndexData(subEntity->GetSubModel()->GetSubMesh()->GetIndexData());
    renderOp.SetOperationType(subEntity->GetSubModel()->GetSubMesh()->GetOperationType());
    renderOp.SetTransform(&mParentNode->WorldTransform());
    if (subEntity->GetSubModel()->GetSubMesh()->UseSharedVertices())
    {
        renderOp.SetVertexData(mModel->GetMesh()->GetSharedVertexedData());
    }
    else
    {
        renderOp.SetVertexData(subEntity->GetSubModel()->GetSubMesh()->GetVertexData());
    }

    const NBRE_ITextureAnimation* passOp = subEntity->TextureAnimation();
    if (mPassOperations.size() > 0 || passOp != NULL)
    {
        NBRE_PassPtr passCache;
        passCache = GetCachedPass();
        *passCache = *pass;

        if(passOp != NULL)
        {
            passOp->ModifyPass(passCache.get());
        }

        for(NBRE_PassOperationList::const_iterator po = mPassOperations.begin(); po != mPassOperations.end(); ++po)
        {
            (*po)->ModifyPass(passCache.get());
        }
        renderOp.SetPass(passCache.get());
    }
    else
    {
        renderOp.SetPass(pass.get());
    }
}

void
NBRE_Entity::GetRenderOperation(NBRE_SubEntity* subEntity, NBRE_Overlay& overlay)
{
    const NBRE_ShaderPtr& shader = subEntity->GetShader();
    if(shader)
    {
        for(uint32 i = 0; i < shader->GetPassCount(); ++i)
        {
            GetRenderOperation(subEntity, overlay.CreateRenderOperation(), shader->GetPass(i));
        }
    }
    else
    {
        GetRenderOperation(subEntity, overlay.CreateRenderOperation(), mContext.mDefaultPass->GetPass());
    }
}

void
NBRE_Entity::AddPassOperation(NBRE_IPassOperation* passOperation)
{
    mPassOperations.push_back(passOperation);
}

void
NBRE_Entity::UpdateSurfaceList()
{
    for (SurfaceList::iterator i = mRefSurfaces.begin(); i != mRefSurfaces.end(); ++i)
    {
        mContext.mSurfaceManager->UpdateSurfaceList(*i);
    }
}

void
NBRE_Entity::NotifyAttached(NBRE_Node* node)
{
    mParentNode = node;
}

void
NBRE_Entity::NotifyDetatch()
{
    mParentNode = NULL;
}

void
NBRE_Entity::NotifyUpdatePosition()
{
    mAxisAlignedBox = NBRE_TypeConvertd::Convert(mModel->GetMesh()->AxisAlignedBox());
    mBoundingBox.center = mAxisAlignedBox.GetCenter();

    mBoundingBox.up = NBRE_Vector3d(0, 0, mAxisAlignedBox.maxExtend.z - mBoundingBox.center.z);
    mBoundingBox.forward = NBRE_Vector3d(0, mAxisAlignedBox.maxExtend.y - mBoundingBox.center.y, 0);
    mBoundingBox.right = NBRE_Vector3d(mAxisAlignedBox.maxExtend.x - mBoundingBox.center.x, 0, 0);

    NBRE_Transformationd::Transform(mParentNode->WorldTransform(), mAxisAlignedBox);
    NBRE_Transformationd::Transform(mParentNode->WorldTransform(), mBoundingBox);
}

NBRE_ISpatialObject*
NBRE_Entity::SpatialObject()
{
    return this;
}

void
NBRE_Entity::Accept(NBRE_Visitor& visitor)
{
    visitor.Visit(static_cast<NBRE_IRenderOperationProvider*>(this), mContext.mSurfaceManager);
    visitor.Visit(static_cast<NBRE_ISpatialObject*>(this));
    if(mPickListener)
    {
        visitor.Visit(static_cast<NBRE_IPickedUpObject*>(this));
    }
    visitor.Visit(static_cast<NBRE_IMovableObject*>(this));
}

void
NBRE_Entity::NotifyCamera(const NBRE_Camera& /*camera*/)
{
}

void
NBRE_Entity::UpdateRenderQueue(NBRE_Overlay& overlay)
{
    if(!mVisible)
    {
        return;
    }

    mCurrentPass = 0;

    mModel->UpdateVertexData();

    for (NBRE_SubEntityList::iterator i = mSubEntityList.begin(); i != mSubEntityList.end(); ++i)
    {
        GetRenderOperation(*i, overlay);
    }

    mModel->CompleteVertextAnimation();
}

int32
NBRE_Entity::OverlayId() const
{
    return mOverlayId;
}

void
NBRE_Entity::SetModel(const NBRE_ModelPtr& model)
{
    nbre_assert(!model);

    mModel = model;
    for (uint32 i = 0; i < mModel->GetSubModelNumber(); ++i)
    {
        NBRE_SubEntity* subEntity = NBRE_NEW NBRE_SubEntity(mContext, &mModel->GetSubModel(i));
        mSubEntityList.push_back(subEntity);
    }
}

const NBRE_ModelPtr&
NBRE_Entity::GetModel() const
{
    return mModel;
}

void
NBRE_Entity::SetOverlayId(int32 overlayId)
{
    mOverlayId = overlayId;
}

void
NBRE_Entity::OnPickedUp()
{
    mPickListener->OnPickedUp(this);
}

nb_boolean
NBRE_Entity::IsIntersect(const NBRE_Camera& /*camera*/, const NBRE_Ray3f& ray, NBRE_Point3f* pIntersectPoint)
{
    if (NBRE_Intersectionf::HitTest(NBRE_TypeConvertf::Convert(mAxisAlignedBox), ray, NULL))
    {
        NBRE_Ray3f r = ray;
        NBRE_Transformationf::Transform(NBRE_TypeConvertf::Convert(mParentNode->WorldTransform().Inverse()), r);
        nb_boolean result = mModel->GetMesh()->HitTest(r, pIntersectPoint);
        if (pIntersectPoint != NULL && result)
        {
            NBRE_Transformationf::Transform(NBRE_TypeConvertf::Convert(mParentNode->WorldTransform()), *pIntersectPoint);
        }
        return result;
    }
    else
    {
        return FALSE;
    }
}

void
NBRE_Entity::SetPickListener(NBRE_IEntityListener* listener)
{
    mPickListener = listener;
}

void
NBRE_Entity::SetVertexAnimation(shared_ptr<NBRE_IVertexAnimation> vertexAnimation)
{
    if(mModel)
    {
        mModel->SetVertexAnimation(vertexAnimation);
    }
}

void
NBRE_Entity::SetTextureAnimation(const NBRE_Set<NBRE_SubModel*>& animatedSubModels, NBRE_ITextureAnimation* textureAnimation)
{
    nbre_assert(textureAnimation != NULL);
    for (NBRE_SubEntityList::iterator i = mSubEntityList.begin(); i != mSubEntityList.end(); ++i)
    {
        if(animatedSubModels.find((*i)->GetSubModel()) != animatedSubModels.end())
        {
            (*i)->SetTextureAnimation(textureAnimation);
        }
    }
}

void
NBRE_Entity::AddRefSurface(NBRE_RenderSurface* surface)
{
    mRefSurfaces.push_back(surface);
}

void
NBRE_Entity::SetName( const NBRE_String& name )
{
    mEntityName = name;
}

NBRE_Entity*
NBRE_Entity::Duplicate()
{
    NBRE_Entity* entity = NBRE_NEW NBRE_Entity(mContext, mModel, mOverlayId);
    entity->mVisible = mVisible;
    entity->mPassOperations = mPassOperations;
    entity->mAxisAlignedBox = mAxisAlignedBox;
    entity->mPickListener = mPickListener;
    entity->mRefSurfaces = mRefSurfaces;
    entity->mEntityName = mEntityName + " Duplicate";
    entity->mPriority = mPriority;
    for(uint32 i = 0; i < GetSubEntityNumber(); ++i)
    {
        NBRE_SubEntity& subEntity = GetSubEntity(i);
        NBRE_SubEntity& duplicateSubEntity = entity->GetSubEntity(i);
        duplicateSubEntity.SetShaderName(subEntity.mShaderName);
        if(subEntity.mShaderName.empty())
        {
            duplicateSubEntity.SetShader(subEntity.GetShader());
        }
        duplicateSubEntity.SetVisible(subEntity.Visible());
    }
    return entity;
}
