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
#include "TestSkyView.h"
#include "nbrecommon.h"
#include "nbrescenemanager.h"
#include "nbrerendersurface.h"
#include "nbreentity.h"
#include "nbrehardwareindexbuffer.h"
#include "nbrehardwarevertexbuffer.h"
#include "nbresubmesh.h"
#include "nbretransformation.h"
#include <time.h>
#include "nbreintersection.h"
#include "nbgmTestUtilities.h"
#include "nbgmskywall.h"
#include "nbretextureunit.h"
#include "nbrepngtextureimage.h"
#include "nbredefaultspatialdividemanager.h"
#include "nbrespatialdividemanagerdebug.h"
#include "nbrecameradebug.h"
#include "nbrecommon.h"
#include "nbgmsky.h"
#include "nbgmbuildutility.h"


CTestSkyView::CTestSkyView():
    mContext(NULL),
    m_ScreenHeight(1),
    m_ScreenWidth(1),
    m_renderEngine(NULL),
    m_node(NULL),
    mSpatialDivideDebug(NULL),
    mCameraDebug(NULL),
    m_sky(NULL),
    mMaxOverlayId(0)
{
}

CTestSkyView::~CTestSkyView()
{
	NBRE_DELETE mOverlayManager;
    m_renderEngine->Deinitialize();
    NBRE_DELETE m_renderEngine;
}


static NBRE_TexturePtr CreateTexture(NBRE_Context& context, const char* name)
{
    char fullPath[MAX_PATH + 1] = {0};
    char *folder = GetExePath();
    sprintf_s(fullPath, MAX_PATH + 1, "%s\\%s", folder, name);
    NBRE_DELETE []folder;
    NBRE_TexturePtr texture = NBGM_BuildTextureUtility::ConstructImageTexture(*context.mRenderPal, context.mPalInstance, fullPath, 0, FALSE, NBRE_Texture::TT_2D, FALSE); 
    if(texture.get())
    {
        texture->Load();
    }
    return texture;
}

static NBRE_ShaderPtr CreateMaterial(NBRE_Context& context)
{
    NBRE_PassPtr pass = NBRE_PassPtr(NBRE_NEW NBRE_Pass);
    NBRE_ShaderPtr shaderPtr(NBRE_NEW NBRE_Shader());
    shaderPtr->AddPass(pass);

    NBRE_TexturePtr texture = CreateTexture(context, "SKY.png");

    pass->SetEnableLighting(FALSE);
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(TRUE);
    pass->SetEnableCullFace(TRUE);
    pass->SetEnableBlend(FALSE);
    pass->SetColor(NBRE_Color(1.0f, 1.0f, 1.0f, 1.0f));

    NBRE_TextureUnit unit0;
    unit0.SetTexture(texture);
    pass->GetTextureUnits().push_back(unit0);
    
    return shaderPtr;
}

static NBRE_ShaderPtr CreateMaterial1()
{
    NBRE_PassPtr pass = NBRE_PassPtr(NBRE_NEW NBRE_Pass);
    NBRE_ShaderPtr shaderPtr(NBRE_NEW NBRE_Shader());
    shaderPtr->AddPass(pass);

    pass->SetEnableLighting(FALSE);
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(TRUE);
    pass->SetEnableCullFace(FALSE);
    pass->SetEnableBlend(FALSE);
    pass->SetColor(NBRE_Color(0.0f, 1.0f, 0.0f, 1.0f));

    
    return shaderPtr;
}

void CTestSkyView::CreateScene(NBRE_Context& context)
{


    NBRE_RenderSurface* surface = m_renderEngine->Context().mSurfaceManager->CreateSurface();
    m_renderEngine->SetActiveRenderSurface(surface);

    NBRE_SurfaceSubView& subview = surface->CreateSubView(0, 0, 0, 0);
    subview.SetOverlayManager(mOverlayManager);


    int32 overlayId = mOverlayManager->AssignOverlayId(0);

    NBRE_Overlay* overlay = mOverlayManager->FindOverlay(overlayId);

    overlay->SetClearColor(NBRE_Color(1, 1, 1, 1));
    overlay->SetClearFlag(NBRE_CLF_CLEAR_COLOR|NBRE_CLF_CLEAR_DEPTH);

    NBRE_DefaultSpatialDivideManager* spatialDivideManager = NBRE_NEW NBRE_DefaultSpatialDivideManager(1);
    NBRE_SceneManager* sceneMgr = NBRE_NEW NBRE_SceneManager();
    sceneMgr->SetSpatialDivideStrategy(spatialDivideManager);

    subview.SetSceneManager(NBRE_SceneManagerPtr(sceneMgr));

    float vertex[9] = {-3.0f, 0.0f, 0.0f,
                        3.0f, 0.0f, 0.0f,
                        0.0f, 4.0f, 0.0f};
    NBRE_MeshPtr mesh(CreatePolygon3D(*context.mRenderPal, vertex, 3));

    NBRE_Entity* ent = NBRE_NEW NBRE_Entity(context, NBRE_ModelPtr(NBRE_NEW NBRE_Model(mesh)), overlayId);

    m_node = NBRE_NEW NBRE_Node;
    m_node->AttachObject(NBRE_EntityPtr(ent));
    sceneMgr->RootSceneNode()->AddChild(NBRE_NodePtr(m_node));

    m_sky = NBRE_NEW NBGM_Skywall(context, FALSE, 0); 
    m_sky->SetShader(CreateMaterial(context));
    m_sky->SetSize(80, 10, 50);


    NBRE_ModelPtr model(NBRE_NEW NBRE_Model(NBRE_MeshPtr(CreateGround(*context.mRenderPal, 1, 100))));
    model->SetShader(CreateMaterial1());

    ent = NBRE_NEW NBRE_Entity(context, model, overlayId);

    NBRE_Node* node = NBRE_NEW NBRE_Node;
    node->AttachObject(NBRE_EntityPtr(ent));
    sceneMgr->RootSceneNode()->AddChild(NBRE_NodePtr(node));


    NBRE_CameraPtr camera(NBRE_NEW NBRE_Camera());
    camera->LookAt(NBRE_Vector3f(0, -10, 10), NBRE_Vector3f(0.0f, 0.0f, 0.0f), NBRE_Vector3f(0, 0, 1));

    m_node->AddChild(camera);
   
    subview.Viewport().SetCamera(camera);


    NBRE_PerspectiveConfig perspective;
    perspective.mAspect = 1;
    perspective.mFov = 45;
    camera->GetFrustum().SetAsPerspective(perspective, 1, 100);

    m_camera = NBRE_CameraPtr(NBRE_NEW NBRE_Camera());
    m_camera->LookAt(NBRE_Vector3f(0, 0, 5), NBRE_Vector3f(0, 0, 0), NBRE_Vector3f(0, 1, 0));
    m_node->AddChild(m_camera);

    m_camera->GetFrustum().SetAsPerspective(perspective, 1, 10);
  
    mCameraDebug = NBRE_NEW NBRE_CameraDebug(context, *m_camera, 0);

    subview.AddVisitable(mCameraDebug);
    subview.AddVisitable(mSpatialDivideDebug);
    subview.AddVisitable(m_sky);

    mSpatialDivideDebug = NBRE_NEW NBRE_SpatialDivideManagerDebug(context, *spatialDivideManager, 0);

    m_CameraHandler.SetRotateAnglePerAction(1.0f);
    m_CameraHandler.SetSlideDistancePerAction(0.5f);
    m_CameraHandler.BindCamera(camera.get());
}

int CTestSkyView::OnCreate()
{
    m_renderEngine = CreateRenderEngine(m_RenderPal);

	mContext = &m_renderEngine->Context();

	mOverlayManager = NBRE_NEW NBRE_DefaultOverlayManager<int32>(*mContext, &mMaxOverlayId);
	
	CreateScene(m_renderEngine->Context());
    return 0;
}

void CTestSkyView::OnSize(UINT nType, int cx, int cy)
{
    NBRE_RenderSurface* surface = m_renderEngine->ActiveRenderSurface();
    NBRE_SurfaceSubView& subView = surface->GetSubView(0);

    subView.Viewport().Update(0, 0, cx, cy);

    NBRE_PerspectiveConfig perspective;
    perspective.mAspect = float(cx)/cy;
    perspective.mFov = 45;
    subView.Viewport().Camera()->GetFrustum().SetAsPerspective(perspective, 1, 100);
}

void CTestSkyView::OnRender()
{
    GetWindow()->OnRenderBegine();
    m_renderEngine->Render();
    GetWindow()->OnRenderEnd();
}

void CTestSkyView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    m_CameraHandler.OnKeyEvent(nChar);
}
