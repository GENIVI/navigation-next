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
#include "nbrescenemanager.h"
#include "nbreintersection.h"
#include "nbredefaultspatialdividemanager.h"
#include "nbrebillboardnode.h"
#include "nbretypeconvert.h"
#include "nbresurfacemanager.h"

NBRE_SceneManager::NBRE_SceneManager() :
    mRootNode(NULL),
    mSpatialDivideStrategy(NULL)
{
    Construct();
}

NBRE_SceneManager::~NBRE_SceneManager()
{
    NBRE_DELETE mRootNode;
    NBRE_DELETE mSpatialDivideStrategy;
}

void NBRE_SceneManager::Construct()
{
    mRootNode = NBRE_NEW NBRE_Node();
    RefreshLights();
}

NBRE_Node* NBRE_SceneManager::RootSceneNode()
{
    return mRootNode;
}

void NBRE_SceneManager::UpdatePosition()
{
    mRootNode->UpdatePosition();
}

void NBRE_SceneManager::UpdateOverlay(const NBRE_Camera& camera, NBRE_IOverlayManager& overlayMap)
{
    UpdatePosition();

    NBRE_Frustum3f frustum(camera.GetFrustum().ExtractMatrix()*NBRE_TypeConvertf::Convert(camera.ExtractMatrix()));

    RenderQueueVisitor visitor(overlayMap, camera, frustum);

    FrustumHitTest hittest(frustum);
    mSpatialDivideStrategy->WalkHitTestSpatialObject(&hittest, visitor);

    //for(VisitableList::iterator pVisitable = mVisitables.begin(); pVisitable != mVisitables.end(); ++pVisitable)
    //{
    //    (*pVisitable)->Accept(visitor);
    //}
}

class RayHitTest: public NBRE_ISpaceHitTest
{
public:
    RayHitTest(const NBRE_Ray3f& ray): mRay(ray)
    {
    }

    virtual nb_boolean Intersect(const NBRE_AxisAlignedBox3f& box)
    {
        return NBRE_Intersectionf::HitTest(box, mRay, NULL);
    }

    virtual nb_boolean Contain(const NBRE_AxisAlignedBox3f& /*box*/)
    {
        return FALSE;
    }

private:
    NBRE_Ray3f mRay;
};

void NBRE_SceneManager::QueryObjects(const NBRE_Camera& /*camera*/, const NBRE_Ray3f& ray, NBRE_MovableObjectList& movableObjs)
{
    UpdatePosition();

    SpatialQueryVisitor visitor(movableObjs);

    RayHitTest hittest(ray);
    mSpatialDivideStrategy->WalkHitTestSpatialObject(&hittest, visitor);
}

void NBRE_SceneManager::SpatialQueryVisitor::Visit(NBRE_IMovableObject* movableObject)
{
    mMovableObjectList.push_back(movableObject);
}

void NBRE_SceneManager::PickupObject(const NBRE_Camera& camera, const NBRE_Ray3f& ray, nb_boolean closestObjectOnly)
{
    UpdatePosition();

    PickObjectList pickObjs;
    SpatialPickupVisitor visitor(pickObjs);

    RayHitTest hittest(ray);
    mSpatialDivideStrategy->WalkHitTestSpatialObject(&hittest, visitor);

    if (pickObjs.size() == 0)
    {
        return;
    }
    if (closestObjectOnly)
    {
        NBRE_Point3f point;
        NBRE_IPickedUpObject* closeObj = NULL;
        float minDistSq = 0;
        for(PickObjectList::iterator pObj = pickObjs.begin(); pObj != pickObjs.end(); ++pObj)
        {
            NBRE_IPickedUpObject* po = *pObj;
            if (po->IsIntersect(camera, ray, &point))
            {
                float distSq = ray.origin.SquaredDistanceTo(point);
                if (closeObj == NULL || distSq < minDistSq)
                {
                    closeObj = po;
                    minDistSq = distSq;
                }
            }
        }
        if (closeObj != NULL)
        {
            closeObj->OnPickedUp();
        }
    }
    else
    {
        for(PickObjectList::iterator pObj = pickObjs.begin(); pObj != pickObjs.end(); ++pObj)
        {
            NBRE_IPickedUpObject* po = *pObj;
            if (po->IsIntersect(camera, ray, NULL))
            {
                po->OnPickedUp();
            }
        }
    }
}

void NBRE_SceneManager::SpatialPickupVisitor::Visit(NBRE_IPickedUpObject* pickObject)
{
    mPickObjectList.push_back(pickObject);
}

void NBRE_SceneManager::WalkNodeTree(NBRE_Node* node, NBRE_Visitor& visitor)
{
    node->Accept(visitor);

    const NBRE_NodeList& children = node->GetChildren();
    for(NBRE_NodeList::const_iterator pNode = children.begin(); pNode != children.end(); ++pNode)
    {
        WalkNodeTree((*pNode).get(), visitor);
    }
}

void NBRE_SceneManager::RenderQueueVisitor::Visit(NBRE_IRenderOperationProvider* updateRenderQueue, NBRE_SurfaceManager* /*surfaceManager*/)
{
    if(updateRenderQueue->Visible())
    {
        NBRE_Overlay* overlay = mOverlayManager.FindOverlay(updateRenderQueue->OverlayId());

        if(overlay)
        {
            updateRenderQueue->UpdateSurfaceList();
            overlay->AppendRenderProvider(updateRenderQueue);
        }
    }
}

void NBRE_SceneManager::LightVisitor::Visit(NBRE_ILightProvider* updateLight)
{
    updateLight->UpdateLights(mLights);
}

void NBRE_SceneManager::RefreshLights()
{
    mNeedUpdateLights = TRUE;
}

const NBRE_LightList& NBRE_SceneManager::GetLights()
{
    if (mNeedUpdateLights)
    {
        mLights.clear();
        LightVisitor visitor(mLights);
        WalkNodeTree(mRootNode, visitor);
        mNeedUpdateLights = FALSE;
    }
    return mLights;
}


void NBRE_SceneManager::SpatialVisitor::Visit(NBRE_ISpatialObject* spatialObject)
{
    mSpatialDivideStrategy->RemoveSpatialObject(spatialObject);
    mSpatialDivideStrategy->AddSpatialObject(spatialObject);
}

void NBRE_SceneManager::SetSpatialDivideStrategy(NBRE_ISpatialDivideStrategy* strategy)
{
    mRootNode->SetSpatialDivideStrategy(strategy);
    NBRE_DELETE mSpatialDivideStrategy;
    mSpatialDivideStrategy = strategy;
}
