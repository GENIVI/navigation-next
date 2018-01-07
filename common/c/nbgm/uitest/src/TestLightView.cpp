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
#include "TestLightView.h"
#include "nbrehardwareindexbuffer.h"
#include "nbrehardwarevertexbuffer.h"
#include "nbrecommon.h"
#include "nbretransformation.h"
#include "nbreentity.h"
#include "nbgmTestUtilities.h"
#include "nbrelightnode.h"
#include "nbreipassoperation.h"
#include "nbretextureunit.h"
#include "nbrepngtextureimage.h"
#include "nbredefaultspatialdividemanager.h"
#include "nbgmbuildutility.h"

NBRE_LightSource* CTestLightView::CreateLight(NBRE_Node* parent, const NBRE_Color& diffuse, const char* shaderName)
{
    // Create light
    NBRE_LightSource* lightNode = NBRE_NEW NBRE_LightSource();
    lightNode->SetType(NBRE_LT_POINT);
    lightNode->SetAmbient(NBRE_Color(1.0f, 1.0f, 1.0f, 1.0f));
    lightNode->SetDiffuse(diffuse);
    lightNode->SetSpecular(NBRE_Color(0.0f, 0.0f, 0.0f, 1.0f));   
    parent->AddChild(NBRE_NodePtr(lightNode));
    
    // Show light with entity
    NBRE_MeshPtr mesh(CreateSphere(*m_renderEngine->Context().mRenderPal, 1.0f));
    NBRE_ModelPtr model(NBRE_NEW NBRE_Model(mesh));
    model->SetShaderName(shaderName);
    NBRE_EntityPtr ent(NBRE_NEW NBRE_Entity(m_renderEngine->Context(), model, mOverlayId));
    ent->AddPassOperation(&mShiningController);

    lightNode->AttachObject(ent);
    lightNode->SetScale(NBRE_Vector3f(0.2f, 0.2f, 0.2f));

    return lightNode;
}

static PAL_Instance* pal = NULL;
//int m_ScreenHeight;
//int m_ScreenWidth;
//NBRE_RenderEngine* m_renderEngine;
//NBRE_Node* m_node1;
//NBRE_Node* m_node2;
//NBRE_LightSource* m_lightNode1;
//NBRE_LightSource* m_lightNode2;
//NBRE_CameraPtr mCamera;
//NBRE_ShiningController mShiningController;
//NBRE_TexturePtr mTexture0;
//CCameraController m_CameraHandler;

CTestLightView::CTestLightView():
    m_ScreenHeight(1),
    m_ScreenWidth(1),
    m_renderEngine(NULL),
    m_node1(NULL),
    m_node2(NULL),
    m_lightNode1(NULL),
    m_lightNode2(NULL),
    mMaxOverlayId(0)
{
}

CTestLightView::~CTestLightView()
{
    NBRE_DELETE mOverlayManager;
    m_renderEngine->Deinitialize();
    NBRE_DELETE m_renderEngine;
}

int CTestLightView::OnCreate()
{
    m_renderEngine = CreateRenderEngine(m_RenderPal);
    mOverlayManager = NBRE_NEW NBRE_DefaultOverlayManager<int32>(m_renderEngine->Context(), &mMaxOverlayId);
    CreateMaterial();

    NBRE_SceneManager* sceneMgr = NBRE_NEW NBRE_SceneManager;
    sceneMgr->SetSpatialDivideStrategy(NBRE_NEW NBRE_DefaultSpatialDivideManager(1));

    mOverlayId = mOverlayManager->AssignOverlayId(0);
    NBRE_Overlay* overlay = mOverlayManager->FindOverlay(mOverlayId);

    overlay->SetClearColor(NBRE_Color(0, 0, 0, 1));
    overlay->SetClearFlag(NBRE_CLF_CLEAR_COLOR | NBRE_CLF_CLEAR_DEPTH);


    // Sphere mesh
    NBRE_MeshPtr mesh(CreateSphere(*m_renderEngine->Context().mRenderPal, 1.0f));
    NBRE_ModelPtr model(NBRE_NEW NBRE_Model(mesh));
    model->SetShaderName("sphere");
    NBRE_Entity* ent = NBRE_NEW NBRE_Entity(m_renderEngine->Context(), model, mOverlayId);
    // Shader name must be predefined in shader manager

    // Center sphere
    m_node1 = NBRE_NEW NBRE_Node;
    m_node1->AttachObject(NBRE_EntityPtr(ent));
    sceneMgr->RootSceneNode()->AddChild(NBRE_NodePtr(m_node1));

    // Rotate sphere
    ent = NBRE_NEW NBRE_Entity(m_renderEngine->Context(), model, mOverlayId);
    m_node2 = NBRE_NEW NBRE_Node;
    m_node2->AttachObject(NBRE_EntityPtr(ent));
    m_node1->AddChild(NBRE_NodePtr(m_node2));
    m_node2->SetPosition(NBRE_Vector3f(5, 0, 0));
    m_node2->SetScale(NBRE_Vector3f(0.5f, 0.5f, 0.5f));

    // Create lights
    m_lightNode1 = CreateLight(sceneMgr->RootSceneNode(), NBRE_Color(2.0f, 0.4f, 0.3f, 1.0f), "light1");
    m_lightNode1->SetSpecular(NBRE_Color(1.0f, 10.0f, 10.0f, 1.0f));
    m_lightNode2 = CreateLight(m_node2, NBRE_Color(0.2f, 0.6f, 0.1f, 1.0f), "light2");
    // Call this after light add/delete, so scene manager will know the change
    sceneMgr->RefreshLights();
    // Set position of lights
    m_lightNode1->SetPosition(NBRE_Vector3f(10, 5, 3));
    m_lightNode2->SetPosition(NBRE_Vector3f(0, 0, 0));


    // Set camera
    mCamera = NBRE_CameraPtr(NBRE_NEW NBRE_Camera());
    mCamera->LookAt(NBRE_Vector3f(0, 0, 20), NBRE_Vector3f(0, 0, 0), NBRE_Vector3f(0, 1, 0));
    sceneMgr->RootSceneNode()->AddChild(mCamera);
    NBRE_PerspectiveConfig perspective;
    perspective.mAspect = 1;
    perspective.mFov = 45;
    mCamera->GetFrustum().SetAsPerspective(perspective, 1, 100);

    NBRE_RenderSurface* surface = m_renderEngine->Context().mSurfaceManager->CreateSurface();
    m_renderEngine->SetActiveRenderSurface(surface);

    NBRE_SurfaceSubView& subview = surface->CreateSubView(0, 0, 0, 0);
    subview.Viewport().SetCamera(mCamera);
    subview.SetSceneManager(NBRE_SceneManagerPtr(sceneMgr));
    subview.SetOverlayManager(mOverlayManager);


    return 0;
}

void CTestLightView::OnSize(UINT nType, int cx, int cy)
{
    NBRE_RenderSurface* surface = m_renderEngine->ActiveRenderSurface();
    NBRE_SurfaceSubView& subView = surface->GetSubView(0);

    subView.Viewport().Update(0, 0, cx, cy);

    NBRE_PerspectiveConfig perspective;
    perspective.mAspect = float(cx)/cy;
    perspective.mFov = 60;
    subView.Viewport().Camera()->GetFrustum().SetAsPerspective(perspective, 1, 100);
}

void CTestLightView::Update()
{
    clock_t c = clock();
    m_node1->SetOrientation(NBRE_Orientation(NBRE_Vector3f(0,0,1), static_cast<float>((c/10)%360) ));
    m_node2->SetOrientation(NBRE_Orientation(NBRE_Vector3f(0,0,1), static_cast<float>((c/10)%360) ));
    // Update animation
    mShiningController.Update();
}

void CTestLightView::OnRender()
{
    GetWindow()->OnRenderBegine();
    Update();
    m_renderEngine->Render();
    GetWindow()->OnRenderEnd();
}


void CTestLightView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    float d = 1.0f;

    switch (nChar)
    {
    case 'W':
        mCamera->Slide(NBRE_Vector3f(0, 0, -d));
        break;
    case 'S':
        mCamera->Slide(NBRE_Vector3f(0, 0, d));
        break;
    case 'A':
        mCamera->Slide(NBRE_Vector3f(-d, 0, 0));
        break;
    case 'D':
        mCamera->Slide(NBRE_Vector3f(d, 0, 0));
        break;
    case 'Q':
        mCamera->Slide(NBRE_Vector3f(0, -d, 0));
        break;
    case 'E':
        mCamera->Slide(NBRE_Vector3f(0, d, 0));
        break;
    }
}

static NBRE_PassPtr
CreateShaderAndGetDefaultPass(NBRE_ShaderManager* sm, const char* shaderName)
{
    NBRE_PassPtr passPtr = NBRE_PassPtr(NBRE_NEW NBRE_Pass);
    NBRE_ShaderPtr shaderPtr = NBRE_ShaderPtr(NBRE_NEW NBRE_Shader());
    shaderPtr->AddPass(passPtr);
    sm->SetShader(shaderName, shaderPtr);
    return passPtr;
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

void CTestLightView::CreateMaterial()
{
    mTexture0 = CreateTexture(m_renderEngine->Context(), "TestTex2.png");

    {
        NBRE_PassPtr pass1 = CreateShaderAndGetDefaultPass(m_renderEngine->Context().mShaderManager, "sphere");
        pass1->SetEnableLighting(FALSE);
        pass1->SetEnableDepthTest(TRUE);
        pass1->SetEnableDepthWrite(TRUE);
        pass1->SetEnableCullFace(TRUE);
        pass1->SetEnableBlend(FALSE);
        NBRE_Material& mtl = pass1->GetMaterial();
        mtl.SetDiffuse(NBRE_Color(1.0f, 1.0f, 1.0f, 1.0f));
        mtl.SetSpecular(NBRE_Color(0.5f, 0.5f, 0.5f, 1.0f));
        mtl.SetShiness(2);

     
        NBRE_TextureUnit unit0;
        unit0.SetTexture(mTexture0);
        pass1->GetTextureUnits().push_back(unit0);
    }
    
    {
        NBRE_PassPtr pass1 = CreateShaderAndGetDefaultPass(m_renderEngine->Context().mShaderManager, "light1");
        pass1->SetEnableLighting(TRUE);
        pass1->SetEnableDepthTest(TRUE);
        pass1->SetEnableDepthWrite(TRUE);
        pass1->SetEnableCullFace(TRUE);
        NBRE_Material& mtl = pass1->GetMaterial();
        mtl.SetDiffuse(NBRE_Color(0, 0, 0, 1));
        mtl.SetEmissive(NBRE_Color(1.0f, 0.3f, 0.3f, 1.0f));
    }

    {
        NBRE_PassPtr pass1 = CreateShaderAndGetDefaultPass(m_renderEngine->Context().mShaderManager, "light2");
        pass1->SetEnableLighting(TRUE);
        pass1->SetEnableDepthTest(TRUE);
        pass1->SetEnableDepthWrite(TRUE);
        pass1->SetEnableCullFace(TRUE);
        NBRE_Material& mtl = pass1->GetMaterial();
        mtl.SetDiffuse(NBRE_Color(0, 0, 0, 1));
        mtl.SetEmissive(NBRE_Color(0.3f, 0.7f, 0.3f, 1.0f));
    }
}
