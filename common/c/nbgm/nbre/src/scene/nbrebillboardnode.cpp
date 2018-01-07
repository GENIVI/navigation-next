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
#include <algorithm>
#include "nbrebillboardnode.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbrerenderpal.h"
#include "nbrehardwarebuffer.h"
#include "nbreintersection.h"
#include "nbremath.h"
#include "nbrefontmanager.h"
#include "nbretypeconvert.h"

//************************************NBRE_BillBoardNode***********************************
void
NBRE_BillboardNode::UpdatePosition()
{
    NBRE_Node::UpdatePosition();

    mBoundingBox.SetNull();
    for (NBRE_BillboardEntityList::iterator i = mEntities.begin();
        i != mEntities.end(); ++i)
    {
        mBoundingBox.Merge(NBRE_TypeConvertd::Convert((*i)->RenderBoundingBox()));
    }

    NBRE_Vector3d center = mBoundingBox.GetCenter();
    NBRE_Vector3d size = mBoundingBox.GetSize();
    double d = nsl_sqrt(size.x * size.x + size.y * size.y + size.z * size.z);
    size.x = d * 0.5;
    size.y = size.x;
    size.z = size.x;
    mBoundingBox.minExtend = center - size;
    mBoundingBox.maxExtend = center + size;

    NBRE_Transformationd::Transform(WorldTransform(), mBoundingBox);

}

const NBRE_AxisAlignedBox3d&
NBRE_BillboardNode::AxisAlignedBox() const
{
    return mBoundingBox;
}

void
NBRE_BillboardNode::Update(const NBRE_Camera& camera)
{
    const NBRE_Vector3d& worldPosition = WorldPosition();
    //// update center distance to eye
    const NBRE_Vector3d& eyePosition = camera.WorldPosition();
    mSquaredDistanceToEye = worldPosition.SquaredDistanceTo(eyePosition);

    mModelViewMatrix = camera.ExtractMatrix();
    //// cancel rotation
    NBRE_Math::Swap<double, double>(mModelViewMatrix[0][1], mModelViewMatrix[1][0]);
    NBRE_Math::Swap<double, double>(mModelViewMatrix[0][2], mModelViewMatrix[2][0]);
    NBRE_Math::Swap<double, double>(mModelViewMatrix[2][1], mModelViewMatrix[1][2]);
    mModelViewMatrix[0][3] = worldPosition.x;
    mModelViewMatrix[1][3] = worldPosition.y;
    mModelViewMatrix[2][3] = worldPosition.z;
    mModelViewMatrix *= NBRE_Transformationd::BuildScaleMatrix(NBRE_TypeConvertd::Convert(mScale));
}

void
NBRE_BillboardNode::UpdateRenderQueue(NBRE_Overlay& overlay)
{
    if (!mVisible)
    {
        return;
    }

    for (NBRE_BillboardEntityList::iterator i = mEntities.begin();
        i != mEntities.end(); ++i)
    {
        (*i)->UpdateRenderQueue(overlay, mModelViewMatrix);
    }
}

void
NBRE_BillboardNode::AddEntity(NBRE_BillboardEntityPtr entity)
{
    mEntities.push_back(entity);
}

void
NBRE_BillboardNode::RemoveEntity(NBRE_BillboardEntityPtr entity)
{
    for (NBRE_BillboardEntityList::iterator i = mEntities.begin();
        i != mEntities.end(); ++i)
    {
        if (*i == entity)
        {
            mEntities.erase(i);
            break;
        }
    }
}

void
NBRE_BillboardNode::SetColor(const NBRE_Color& color)
{
    for (NBRE_BillboardEntityList::iterator i = mEntities.begin();
        i != mEntities.end(); ++i)
    {
        (*i)->SetColor(color);
    }
}

int
NBRE_BillboardNode::CompareBillboardsByZOrder(const void* v1, const void* v2)
{
    const NBRE_BillboardNodePtr& b1 = *(const NBRE_BillboardNodePtr*)v1;
    const NBRE_BillboardNodePtr& b2 = *(const NBRE_BillboardNodePtr*)v2;

    if (b1->mDrawIndex == b2->mDrawIndex)
    {
        // Render from the billboard from far to near
        return static_cast<int>(b2->mSquaredDistanceToEye - b1->mSquaredDistanceToEye);
    }
    else
    {
        return static_cast<int>(b1->mDrawIndex - b2->mDrawIndex);
    }
}

//************************************NBRE_BillboardSet***********************************

NBRE_BillboardSet::NBRE_BillboardSet(NBRE_Context& context, int32 overlayId):
    mContext(context),
    mOverlayId(overlayId),
    mEnableZOrder(FALSE)
{
    mAABB.minExtend.x = -NBRE_Math::Infinity;
    mAABB.minExtend.y = -NBRE_Math::Infinity;
    mAABB.minExtend.z = -NBRE_Math::Infinity;
    mAABB.maxExtend.x = NBRE_Math::Infinity;
    mAABB.maxExtend.y = NBRE_Math::Infinity;
    mAABB.maxExtend.z = NBRE_Math::Infinity;
}

NBRE_BillboardSet::~NBRE_BillboardSet()
{
}

NBRE_BillboardNodePtr
NBRE_BillboardSet::AddBillboard()
{
    NBRE_BillboardNodePtr billboard(NBRE_NEW NBRE_BillboardNode());
    mBillboards.push_back(billboard);
    return billboard;
}

void
NBRE_BillboardSet::RemoveBillboard(NBRE_BillboardNodePtr value)
{
    for (NBRE_BillboardNodeArray::iterator i = mBillboards.begin(); i != mBillboards.end(); ++i)
    {
        if (*i == value)
        {
            mBillboards.erase(i);
            break;
        }
    }

    for (NBRE_BillboardNodeArray::iterator i = mVisibleSet.begin(); i != mVisibleSet.end(); ++i)
    {
        if (*i == value)
        {
            mVisibleSet.erase(i);
            break;
        }
    }
}

void
NBRE_BillboardSet::FindVisibleSet(const NBRE_Camera& camera)
{
    mVisibleSet.clear();
    NBRE_Frustum3d frustum(NBRE_TypeConvertd::Convert(camera.GetFrustum().ExtractMatrix())*camera.ExtractMatrix());
    for (NBRE_BillboardNodeArray::iterator i = mBillboards.begin(); i != mBillboards.end(); ++i)
    {
        NBRE_BillboardNodePtr& billboard = *i;
        if (!billboard->IsVisible())
        {
            continue;
        }
        if (NBRE_Intersectiond::HitTest(frustum, billboard->AxisAlignedBox()))
        {
            billboard->Update(camera);
            mVisibleSet.push_back(billboard);
        }
    }
}

void
NBRE_BillboardSet::NotifyCamera(const NBRE_Camera& camera)
{
    // Find visible billboards
    FindVisibleSet(camera);

    uint32 numBillboards = mVisibleSet.size();
    if (numBillboards <= 0)
    {
        return;
    }

    // Sort by z order and shader
    if (numBillboards > 1)
    {
        if (mEnableZOrder)
        {
            nsl_qsort(&mVisibleSet[0], numBillboards, sizeof(NBRE_BillboardNodePtr), NBRE_BillboardNode::CompareBillboardsByZOrder);
        }
        else
        {
            std::stable_sort(mVisibleSet.begin(), mVisibleSet.end(), NBRE_BillboardNodeLessThan());
        }
    }
}

void
NBRE_BillboardSet::UpdateRenderQueue(NBRE_Overlay& overlay)
{
    for (NBRE_BillboardNodeArray::iterator i = mVisibleSet.begin(); i != mVisibleSet.end(); ++i)
    {
        NBRE_BillboardNodePtr& billboard = *i;
        billboard->UpdateRenderQueue(overlay);
    }
}

int32
NBRE_BillboardSet::OverlayId() const
{
    return mOverlayId;
}

nb_boolean
NBRE_BillboardSet::Visible() const
{
    return TRUE;
}

void
NBRE_BillboardSet::Accept(NBRE_Visitor& visitor)
{
    visitor.Visit(static_cast<NBRE_IRenderOperationProvider*>(this), mContext.mSurfaceManager);
}
