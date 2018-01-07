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

    @file nbresurfacesubview.h
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
#ifndef _NBRE_SURFACE_SUBVIEW_H_
#define _NBRE_SURFACE_SUBVIEW_H_
#include "nbretypes.h"
#include "nbreoverlay.h"
#include "nbreveiwport.h"
#include "nbrescenemanager.h"
#include "nbrecontext.h"
#include "nbreoverlaymanager.h"

class NBRE_BillboardSet;

/** \addtogroup NBRE_Scene
*  @{
*/
/** 
NBRE_SurfaceSubView contain a scene manager and a camera
*/

class NBRE_SurfaceSubView
{
    friend class NBRE_RenderSurface;

public:
    NBRE_SurfaceSubView(NBRE_Context& context, int32 left, int32 top, uint32 width, uint32 height);
    ~NBRE_SurfaceSubView();

    NBRE_Viewport& Viewport();
    void SetSceneManager(const NBRE_SceneManagerPtr& sceneMgr);

    void AddVisitable(NBRE_IVisitable* visitable);
    void AddBillboardSet(int32 overlayId);
    NBRE_BillboardSet* GetBillboardSet(int32 overlayId);

    void SetOverlayManager(NBRE_IOverlayManager* overlayManager);

private:
    class RenderQueueVisitor: public NBRE_Visitor
    {
    public:
        /*! Create a RenderQueueVisitor.
        @param overlayMap An NBRE_OverlayMap instance, which stores all Overlay objects as well as their IDs.
            The NBRE_Overlay objects in this overlayMap is the place where save the visit results.
        @param camera Current camera object, tell the NBRE_IRenderOperationProvider current camera setting.
        */
        RenderQueueVisitor(NBRE_IOverlayManager& overlayMap, const NBRE_Camera& camera):
              mOverlayManager(overlayMap),
              mCamera(camera)
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
    };

private:
    ///update SceneManager's renderable to Overlay List
    void Update();

    ///render Overlay List
    void Render();

private:
    typedef NBRE_List<NBRE_IVisitable*> VisitableList;
    typedef NBRE_Map<int32, NBRE_BillboardSet*> BillboardSetMap;

private:
    NBRE_Context& mContext;
    NBRE_Viewport mViewport;
    NBRE_SceneManagerPtr mSceneMgr;
    NBRE_IOverlayManager* mOverlayManager;


    VisitableList mVisitables;
    BillboardSetMap mBillboardSets;

};

/** @} */
#endif
