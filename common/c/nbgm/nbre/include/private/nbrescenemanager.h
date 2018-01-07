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

    @file nbrescenemanager.h
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
#ifndef _NBRE_SCENEMANAGER_H_
#define _NBRE_SCENEMANAGER_H_
#include "nbretypes.h"
#include "nbrenode.h"
#include "nbreoverlay.h"
#include "nbresharedptr.h"
#include "nbrevisitor.h"
#include "nbrefrustum3.h"
#include "nbrespatialdividestrategy.h"
#include "nbreintersection.h"
#include "nbrecontext.h"
#include "nbreoverlaymanager.h"

class NBRE_Camera;

/** \addtogroup NBRE_Scene
*  @{
*/

/** Manage and organize space object.
@remarks
    This class defines a scene manager, it manage all scene relatived object, 
    include entity, light etc. It can be instanced multi times. 
 */

class NBRE_SceneManager
{
public:
    explicit NBRE_SceneManager();
    ~NBRE_SceneManager();

public:

    /** Gets the SceneNode at the root of the scene hierarchy.
        @remarks
            The entire scene is held as a hierarchy of nodes, which
            allows things like relative transforms, general changes in
            rendering state etc (See the SceneNode class for more info).
            In this basic SceneManager class, the application using
            Ogre is free to structure this hierarchy however it likes,
            since it has no real significance apart from making transforms
            relative to each node (more specialised subclasses will
            provide utility methods for building specific node structures
            e.g. loading a BSP tree).
        @par
            However, in all cases there is only ever one root node of
            the hierarchy, and this method returns a pointer to it.
    */
    NBRE_Node* RootSceneNode();

    void SetSpatialDivideStrategy(NBRE_ISpatialDivideStrategy* strategy);

    void UpdatePosition();
    void UpdateOverlay(const NBRE_Camera& camera, NBRE_IOverlayManager& overlayMap);
    void RefreshLights();
    const NBRE_LightList& GetLights();

    void QueryObjects(const NBRE_Camera& camera, const NBRE_Ray3f& ray, NBRE_MovableObjectList& movableObjs);

    void PickupObject(const NBRE_Camera& camera, const NBRE_Ray3f& ray, nb_boolean closestObjectOnly);

private:

    class SpatialVisitor: public NBRE_Visitor
    {
    public:
        SpatialVisitor(NBRE_ISpatialDivideStrategy* spatialStg):mSpatialDivideStrategy(spatialStg){};

    public:
        virtual void Visit(NBRE_ISpatialObject* spatialObject) ;

    private:
        DISABLE_COPY_AND_ASSIGN(SpatialVisitor);

    private:
        NBRE_ISpatialDivideStrategy* mSpatialDivideStrategy;
    };

    /*! A vistor class, visit a NBRE_IRenderOperationProvider to get 
        NBRE_RenderOperation objects which will be used by NBRE_RenderPal later.
        @remarks
            Normally, NBRE_IRenderOperationProvider is inherited by NBRE_Node 
            as NBRE_EntityNode. But any other Nodes can be a NBRE_IRenderOperationProvider
            if it wants to be rendered by NBRE_SceneManager automatically
    */

    class RenderQueueVisitor: public NBRE_Visitor
    {
    public:
        /*! Create a RenderQueueVisitor.
        @param overlayMap An NBRE_OverlayMap instance, which stores all Overlay objects as well as their IDs.
            The NBRE_Overlay objects in this overlayMap is the place where save the visit results.
        @param camera Current camera object, tell the NBRE_IRenderOperationProvider current camera setting.
        @param frustum Camera frustum used by frustum culling.
        */
        RenderQueueVisitor(NBRE_IOverlayManager& overlayMap, const NBRE_Camera& camera, const NBRE_Frustum3f& frustum):
              mOverlayManager(overlayMap),
              mCamera(camera),
              mFrustum(frustum)
              {};

    public:
        /*! Visit a NBRE_IRenderOperationProvider instance to get NBRE_RenderOperation.
         @remarks
            Each NBRE_IRenderOperationProvider should provide a overlay ID, NBRE_RenderOperations got from it
            can only be save to the NBRE_Overlay object which ID is the same.
        @par
            Inherited from NBRE_Visitor.
        */
        virtual void Visit(NBRE_IRenderOperationProvider* updateRenderQueue, NBRE_SurfaceManager* surfaceManager) ;

    private:
        DISABLE_COPY_AND_ASSIGN(RenderQueueVisitor);

    private:
        NBRE_IOverlayManager& mOverlayManager;
        const NBRE_Camera& mCamera;
        const NBRE_Frustum3f& mFrustum;
    };

    typedef NBRE_List<NBRE_IPickedUpObject*> PickObjectList;

    class SpatialPickupVisitor: public NBRE_Visitor
    {
    public:
        SpatialPickupVisitor(PickObjectList& pickObjs): mPickObjectList(pickObjs){};

    public:
        virtual void Visit(NBRE_IPickedUpObject* pickObject);

    private:
        DISABLE_COPY_AND_ASSIGN(SpatialPickupVisitor);

    private:
        PickObjectList& mPickObjectList;
    };

    class SpatialQueryVisitor: public NBRE_Visitor
    {
    public:
        SpatialQueryVisitor(NBRE_MovableObjectList& objs): mMovableObjectList(objs){};

    public:
        virtual void Visit(NBRE_IMovableObject* movableObject);

    private:
        DISABLE_COPY_AND_ASSIGN(SpatialQueryVisitor);

    private:
        NBRE_MovableObjectList& mMovableObjectList;
    };

    /*! A vistor class, visit a NBRE_ILightProvider to get light source.
        @remarks
            Normally, NBRE_ILightProvider is inherited by NBRE_Node 
            as NBRE_LightNode. But any other Nodes can be a NBRE_ILightProvider
            if it wants to provide light sources to NBRE_SceneManager
        @par
            In addition to inheriting, combination is another way to provide
            light source in some case.
    */
    class LightVisitor: public NBRE_Visitor
    {
    public:
        /*! Create a LightVisitor.
        @param lights An NBRE_LightList instance, which stores all NBRE_Lights got from NBRE_ILightProvider.
        */
        LightVisitor(NBRE_LightList& lights):mLights(lights) {}

    public:
        /*! Visit a NBRE_ILightProvider instance to get NBRE_Light.
         @remarks
            Each NBRE_ILightProvider should provide a series of NBRE_Light objects.
        @par
            Inherited from NBRE_Visitor.
        */
        virtual void Visit(NBRE_ILightProvider* lightProvider) ;

    private:
        DISABLE_COPY_AND_ASSIGN(LightVisitor);

    private:
        NBRE_LightList& mLights;
    };

    class FrustumHitTest: public NBRE_ISpaceHitTest
    {
    public:
        FrustumHitTest(const NBRE_Frustum3f& frustum): mFrustum(frustum)
        {
        }

        virtual nb_boolean Intersect(const NBRE_AxisAlignedBox3f& box)
        {
            return NBRE_Intersectionf::HitTest(mFrustum, box);
        }

        virtual nb_boolean Contain(const NBRE_AxisAlignedBox3f& box) 
        {
            return NBRE_Intersectionf::Contain(mFrustum, box);
        }

    private:
        NBRE_Frustum3f mFrustum;
    };

private:
    /** Try to construct the NBRE_SceneManager .
    */
    void Construct();

    void WalkNodeTree(NBRE_Node* node, NBRE_Visitor& v);

private:
    typedef NBRE_List<NBRE_IVisitable*> VisitableList;

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_SceneManager);

private:
    NBRE_Node* mRootNode;
    NBRE_LightList mLights;
    nb_boolean mNeedUpdateLights;
    NBRE_ISpatialDivideStrategy* mSpatialDivideStrategy;

};

typedef shared_ptr<NBRE_SceneManager> NBRE_SceneManagerPtr;

#endif
