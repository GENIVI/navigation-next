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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#include "stdafx.h"
#include "TestCameraControllerView.h"
#include "nbrecommon.h"
#include "nbrescenemanager.h"
#include "nbrerendersurface.h"
#include "nbreentity.h"
#include "nbremesh.h"
#include "nbrevector3.h"
#include "nbgmTestUtilities.h"
#include "palgl.h"
#include "nbredefaultspatialdividemanager.h"
#include "nbrecommon.h"



CTestCameraControllerView::CTestCameraControllerView():
    m_ScreenHeight(1),
    m_ScreenWidth(1),
    m_renderEngine(NULL),
    m_node(NULL),
    mMaxOverlayId(0)
{
}

CTestCameraControllerView::~CTestCameraControllerView()
{
    NBRE_DELETE mOverlayManager;
    m_renderEngine->Deinitialize();
    NBRE_DELETE m_renderEngine;
}


void CTestCameraControllerView::CreateScene(NBRE_Context& context)
{
    NBRE_SceneManager* sceneMgr = NBRE_NEW NBRE_SceneManager();
    sceneMgr->SetSpatialDivideStrategy(NBRE_NEW NBRE_DefaultSpatialDivideManager(1));
  
    int32 overlayId = mOverlayManager->AssignOverlayId(0);
    NBRE_Overlay* overlay = mOverlayManager->FindOverlay(overlayId);

    overlay->SetClearColor(NBRE_Color(0, 0, 0, 1));
    overlay->SetClearFlag(NBRE_CLF_CLEAR_COLOR);
    
    float vertex[9] = {-3.0f, 0.0f, 0.0f,
                        3.0f, 0.0f, 0.0f,
                        0.0f, 4.0f, 0.0f};
    NBRE_MeshPtr mesh(CreatePolygon3D(*context.mRenderPal, vertex, 3));
    NBRE_ModelPtr model(NBRE_NEW NBRE_Model(mesh));
    NBRE_EntityPtr ent(NBRE_NEW NBRE_Entity(context, model, overlayId));

    m_node = NBRE_NEW NBRE_Node;
    m_node->AttachObject(ent);

    sceneMgr->RootSceneNode()->AddChild(NBRE_NodePtr(m_node));


    NBRE_Entity* entity = NBRE_NEW NBRE_Entity(context, NBRE_ModelPtr(NBRE_NEW NBRE_Model(NBRE_MeshPtr(CreateGround(*context.mRenderPal, 1, 100)))), overlayId);
    NBRE_Node* node = NBRE_NEW NBRE_Node;
    node->AttachObject(NBRE_EntityPtr(entity));
    sceneMgr->RootSceneNode()->AddChild(NBRE_NodePtr(node));

    NBRE_CameraPtr camera(NBRE_NEW NBRE_Camera());
    camera->LookAt(NBRE_Vector3f(0, -10, 10), NBRE_Vector3f(0.0f, 0.0f, 0.0f), NBRE_Vector3f(0, 0, 1));

    m_node->AddChild(camera);

    NBRE_PerspectiveConfig perspective;
    perspective.mAspect = 1;
    perspective.mFov = 45;
    camera->GetFrustum().SetAsPerspective(perspective, 1, 100);

    NBRE_RenderSurface* surface = context.mSurfaceManager->CreateSurface();
    m_renderEngine->SetActiveRenderSurface(surface);

    NBRE_SurfaceSubView& subview = surface->CreateSubView(0, 0, 0, 0);
    subview.Viewport().SetCamera(camera);
    subview.SetSceneManager(NBRE_SceneManagerPtr(sceneMgr));
    subview.SetOverlayManager(mOverlayManager);


    m_CameraHandler.SetRotateAnglePerAction(1.0f);
    m_CameraHandler.SetSlideDistancePerAction(0.5f);
    m_CameraHandler.BindCamera(camera.get());

}

int CTestCameraControllerView::OnCreate()
{
    m_renderEngine = CreateRenderEngine(m_RenderPal);
    mOverlayManager = NBRE_NEW NBRE_DefaultOverlayManager<int32>(m_renderEngine->Context(), &mMaxOverlayId);
    CreateScene(m_renderEngine->Context());
    return 0;
}

void CTestCameraControllerView::OnSize(UINT nType, int cx, int cy)
{
    NBRE_RenderSurface* surface = m_renderEngine->ActiveRenderSurface();
    NBRE_SurfaceSubView& subView = surface->GetSubView(0);

    subView.Viewport().Update(0, 0, cx, cy);

    NBRE_PerspectiveConfig perspective;
    perspective.mAspect = float(cx)/cy;
    perspective.mFov = 45;
    subView.Viewport().Camera()->GetFrustum().SetAsPerspective(perspective, 1, 100);
}

#define LINE_LEN 30
void CTestCameraControllerView::OnRender()
{
    GetWindow()->OnRenderBegine();
    m_renderEngine->Render();
    GetWindow()->OnRenderEnd();
}

void CTestCameraControllerView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    m_CameraHandler.OnKeyEvent(nChar);

}
