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
#include "resource.h"
#include "TestCameraView.h"
#include "nbrecommon.h"
#include "nbrescenemanager.h"
#include "nbrerendersurface.h"
#include "nbreentity.h"
#include "nbrehardwareindexbuffer.h"
#include "nbrehardwarevertexbuffer.h"
#include "nbresubmesh.h"
#include "nbretransformation.h"
#include <time.h>
#include "nbgmTestUtilities.h"
#include "nbredefaultspatialdividemanager.h"
#include "nbrecommon.h"

CTestCameraView::CTestCameraView():
    m_ScreenHeight(1),
    m_ScreenWidth(1),
    m_renderEngine(NULL),
    m_node1(NULL),
    m_node2(NULL),
    mMaxOverlayId(0)
{
}

CTestCameraView::~CTestCameraView()
{
    NBRE_DELETE mOverlayManager;
    m_renderEngine->Deinitialize();
    NBRE_DELETE m_renderEngine;
}

NBRE_VertexData* CTestCameraView::CreateTriangleVertexData()
{
    static float pos[9] = {
        -0.5, -0.5, 0,
        0.5, -0.5, 0,
        0, 1, 0
    };

    static float col[12] = {
        1, 0, 0,1,
        0, 1, 0,1,
        0, 0, 1,1
    };


    //create vertex data
    NBRE_VertexElement* positionElement = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    NBRE_HardwareVertexBuffer* vertexBuffer = m_renderEngine->Context().mRenderPal->CreateVertexBuffer(
        sizeof(float)*NBRE_VertexElement::GetTypeCount(NBRE_VertexElement::VET_FLOAT3), 3, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    static_cast<NBRE_HardwareBuffer*>(vertexBuffer)->WriteData(0, sizeof(pos), pos, FALSE);

    NBRE_VertexElement* colorElement = NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT4, NBRE_VertexElement::VES_DIFFUSE, 0);
    NBRE_HardwareVertexBuffer* clolorBuffer = m_renderEngine->Context().mRenderPal->CreateVertexBuffer(
        sizeof(float)*NBRE_VertexElement::GetTypeCount(NBRE_VertexElement::VET_FLOAT4), 3, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    static_cast<NBRE_HardwareBuffer*>(clolorBuffer)->WriteData(0, sizeof(col), col, FALSE);


    NBRE_VertexDeclaration* vertexDeclaration = m_renderEngine->Context().mRenderPal->CreateVertexDeclaration();
    vertexDeclaration->GetVertexElementList().push_back(positionElement);
    vertexDeclaration->GetVertexElementList().push_back(colorElement);
    
    NBRE_VertexData* vertexData = NBRE_NEW NBRE_VertexData(0);
    vertexData->AddBuffer(vertexBuffer);
    vertexData->AddBuffer(clolorBuffer);
    vertexData->SetVertexCount(3);
    vertexData->SetVertexStart(0);
    vertexData->SetVertexDeclaration(vertexDeclaration);

    return vertexData;
}

NBRE_IndexData* CTestCameraView::CreateTriangleIndexData()
{
    //create index data
    static uint16 indics[3] = 
    {
        0, 1, 2
    };

    NBRE_HardwareIndexBuffer* indexBuffer = m_renderEngine->Context().mRenderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, 3, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    static_cast<NBRE_HardwareBuffer*>(indexBuffer)->WriteData(0, sizeof(indics), indics, FALSE);

    return NBRE_NEW NBRE_IndexData(indexBuffer, 0, 3);
}


NBRE_Mesh* CTestCameraView::CreateTriangle()
{
    NBRE_Mesh* mesh = NBRE_NEW NBRE_Mesh(CreateTriangleVertexData());

    mesh->CreateSubMesh(CreateTriangleIndexData(), NBRE_PMT_TRIANGLE_LIST);
    mesh->CalculateBoundingBox();

    return mesh;
}

void CTestCameraView::CreateScene()
{
    NBRE_SceneManager* sceneMgr = NBRE_NEW NBRE_SceneManager;
    sceneMgr->SetSpatialDivideStrategy(NBRE_NEW NBRE_DefaultSpatialDivideManager(1));

    int32 overlayId = mOverlayManager->AssignOverlayId(0);
    NBRE_Overlay* overlay = mOverlayManager->FindOverlay(overlayId);

    overlay->SetClearColor(NBRE_Color(0, 0, 0, 1));
    overlay->SetClearFlag(NBRE_CLF_CLEAR_COLOR);

    NBRE_MeshPtr mesh(CreateTriangle());
    NBRE_ModelPtr model(NBRE_NEW NBRE_Model(mesh));

    NBRE_Entity* ent = NBRE_NEW NBRE_Entity(m_renderEngine->Context(), model, overlayId);
    m_node1 = NBRE_NEW NBRE_Node();
    m_node1->AttachObject(NBRE_EntityPtr(ent));

    ent = NBRE_NEW NBRE_Entity(m_renderEngine->Context(), model, overlayId);
    m_node2 = NBRE_NEW NBRE_Node();
    m_node2->AttachObject(NBRE_EntityPtr(ent));

    sceneMgr->RootSceneNode()->AddChild(NBRE_NodePtr(m_node1));
    m_node1->AddChild(NBRE_NodePtr(m_node2));

    m_node2->SetPosition(NBRE_Vector3f(2, 0, 0));
    m_node2->SetScale(NBRE_Vector3f(0.5f, 0.5f, 0.5f));


    NBRE_CameraPtr camera(NBRE_NEW NBRE_Camera());
    camera->LookAt(NBRE_Vector3f(0, 0, 10), NBRE_Vector3f(0, 0, 0), NBRE_Vector3f(0, 1, 0));
    camera->SetScale(NBRE_Vector3f(5, 5, 5));

    //m_node2->AddChild(camera);
    sceneMgr->RootSceneNode()->AddChild(camera);

    NBRE_PerspectiveConfig perspective;
    perspective.mAspect = 1;
    perspective.mFov = 45;
    camera->GetFrustum().SetAsPerspective(perspective, 1, 100);

    
    NBRE_RenderSurface* surface = m_renderEngine->Context().mSurfaceManager->CreateSurface();
    m_renderEngine->SetActiveRenderSurface(surface);

    NBRE_SurfaceSubView& subview = surface->CreateSubView(0, 0, 0, 0);
    subview.Viewport().SetCamera(camera);
    subview.SetSceneManager(NBRE_SceneManagerPtr(sceneMgr));
    subview.SetOverlayManager(mOverlayManager);


}

int CTestCameraView::OnCreate()
{
    m_renderEngine = CreateRenderEngine(m_RenderPal);
    mOverlayManager = NBRE_NEW NBRE_DefaultOverlayManager<int32>(m_renderEngine->Context(), &mMaxOverlayId);
    CreateScene();
    return 0;
}

void CTestCameraView::OnSize(UINT /*nType*/, int cx, int cy)
{
    NBRE_RenderSurface* surface = m_renderEngine->ActiveRenderSurface();
    NBRE_SurfaceSubView& subView = surface->GetSubView(0);

    subView.Viewport().Update(0, 0, cx, cy);

    NBRE_PerspectiveConfig perspective;
    perspective.mAspect = float(cx)/cy;
    perspective.mFov = 45;
    subView.Viewport().Camera()->GetFrustum().SetAsPerspective(perspective, 1, 100);
}

void CTestCameraView::OnRender()
{
    GetWindow()->OnRenderBegine();
    m_renderEngine->Render();
    GetWindow()->OnRenderEnd();
}

void CTestCameraView::OnTimer(UINT_PTR nIDEvent)
{
    clock_t c = clock();
    m_node1->SetOrientation(NBRE_Orientation(NBRE_Vector3f(0,0,1), static_cast<float>((c/10)%360)));
    m_node2->SetOrientation(NBRE_Orientation(NBRE_Vector3f(0,0,1), static_cast<float>((c/10)%360)));
}

void CTestCameraView::OnKeyDown(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
    //NBRE_RenderSurface* surface = m_renderEngine->DefaultRenderSurface();
    //NBRE_RenderSurface::SurfaceParameter& surfacePara = surface->GetSubView(0);
    //const NBRE_CameraPtr& camera = surfacePara.mViewport->Camera();

    //switch(nChar)
    //{
    //case('A'):   
    //    {
    //        camera->Rotate(NBRE_Vector3f(0, 1, 0), NBRE_Vector3f(0, 0, 0), -2);
    //    }
    //    break;

    //case('D'):
    //    {
    //        camera->Rotate(NBRE_Vector3f(0, 1, 0), NBRE_Vector3f(0, 0, 0), 2);
    //    }
    //    break;    

    //case('S'):    
    //    {
    //        camera->Rotate(camera->Right(), NBRE_Vector3f(0, 0, 0), 2);
    //    }
    //    break;

    //case('W'):    
    //    {
    //        camera->Rotate(camera->Right(), NBRE_Vector3f(0, 0, 0), -2);
    //    }
    //    break;

    //default:
    //    break;
    //}
}
