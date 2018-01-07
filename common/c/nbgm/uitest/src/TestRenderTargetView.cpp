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
#include "TestRenderTargetView.h"
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
#include "nbretexturemanager.h"
#include "nbredefaultspatialdividemanager.h"
#include "nbrecommon.h"
#include "nbgmbuildutility.h"



CTestRenderTargetView::CTestRenderTargetView():
    m_ScreenHeight(1),
    m_ScreenWidth(1),
    m_renderEngine(NULL),
    m_textureSurface(NULL),
    mMaxOverlayId(0)
{
}

CTestRenderTargetView::~CTestRenderTargetView()
{
    NBRE_DELETE mOverlayManager;
    m_renderEngine->Deinitialize();
    NBRE_DELETE m_renderEngine;
}

static NBRE_TexturePtr CreateTexture(NBRE_Context& context, int width, int height)
{
    NBRE_ImageInfo imageInfo(width, height, NBRE_PF_R8G8B8A8);
    NBRE_TexturePtr texture = NBGM_BuildTextureUtility::ConstructCommonTexture(*context.mRenderPal, "", imageInfo, NBRE_Texture::TT_2D_RENDERTARGET, FALSE);
    return texture;
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

static NBRE_ShaderPtr CreateMaterial(const NBRE_TexturePtr& texture)
{
    NBRE_PassPtr pass = NBRE_PassPtr(NBRE_NEW NBRE_Pass);
    NBRE_ShaderPtr shaderPtr(NBRE_NEW NBRE_Shader());
    shaderPtr->AddPass(pass);

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

static NBRE_Mesh* CreateRect(NBRE_IRenderPal& renderer, float width, float height)
{
    NBRE_HardwareVertexBuffer* vertextBuf;
    NBRE_HardwareVertexBuffer* texcordBuf;

    NBRE_VertexDeclaration* decalration = renderer.CreateVertexDeclaration();

    NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem);

    NBRE_VertexElement* texElem = NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
    decalration->GetVertexElementList().push_back(texElem);

    vertextBuf = renderer.CreateVertexBuffer(sizeof(float) * 2, 4, FALSE, NBRE_HardwareBuffer::HBU_STATIC);

    texcordBuf = renderer.CreateVertexBuffer(sizeof(float) * 2, 4, FALSE, NBRE_HardwareBuffer::HBU_STATIC);

    NBRE_VertexData* vertexData = NBRE_NEW NBRE_VertexData(0);
    vertexData->SetVertexDeclaration(decalration);
    vertexData->AddBuffer(vertextBuf);
    vertexData->AddBuffer(texcordBuf);

    float vertex[] = 
    {
        -width/2, -height/2,
         width/2, -height/2,
        -width/2,  height/2,
         width/2,  height/2,
    };
    vertextBuf->WriteData(0, sizeof(vertex), vertex, FALSE);

    float texcord[] = 
    {
        0, 0, 
        1, 0, 
        0, 1, 
        1, 1, 
    };
    texcordBuf->WriteData(0, sizeof(texcord), texcord, FALSE);

    static uint16 indics[] = 
    {
        0, 1, 2,
        1, 3, 2, 
    };
    NBRE_HardwareIndexBuffer* indexBuf = renderer.CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, sizeof(indics)/sizeof(uint16), FALSE, NBRE_HardwareBuffer::HBU_STATIC);

    indexBuf->WriteData(0, sizeof(indics), indics, FALSE);

    NBRE_IndexData* indexData = NBRE_NEW NBRE_IndexData(indexBuf, 0, indexBuf->GetNumIndexes());

    NBRE_Mesh* mesh = NBRE_NEW NBRE_Mesh();

    mesh->CreateSubMesh(vertexData, indexData, NBRE_PMT_TRIANGLE_LIST);
    mesh->CalculateBoundingBox();

    return mesh;
}

void CTestRenderTargetView::CreateScene()
{
    NBRE_SceneManagerPtr sceneMgr(NBRE_NEW NBRE_SceneManager());
    sceneMgr->SetSpatialDivideStrategy(NBRE_NEW NBRE_DefaultSpatialDivideManager(1));

    NBRE_Model* model = NBRE_NEW NBRE_Model(NBRE_MeshPtr(CreateGround(*m_renderEngine->Context().mRenderPal, 1, 100)));
    model->SetShader(CreateMaterial1());

    NBRE_Entity* ent = NBRE_NEW NBRE_Entity(m_renderEngine->Context(), NBRE_ModelPtr(model), 0);

    NBRE_Node* node = NBRE_NEW NBRE_Node;
    node->AttachObject(NBRE_EntityPtr(ent));
    sceneMgr->RootSceneNode()->AddChild(NBRE_NodePtr(node));

    NBRE_TexturePtr texture = CreateTexture(m_renderEngine->Context(), 256, 256);
    m_textureSurface = m_renderEngine->Context().mSurfaceManager->CreateSurface();
    m_textureSurface->AttachTarget(m_renderEngine->Context().mSurfaceManager->GetRenderTargetFromTexture(texture.get(), 0, 0, FALSE));

    node = NBRE_NEW NBRE_Node;
    model = NBRE_NEW NBRE_Model(NBRE_MeshPtr(CreateRect(*m_renderEngine->Context().mRenderPal, 5, 5)));
    model->SetShader(CreateMaterial(texture));
    ent = NBRE_NEW NBRE_Entity(m_renderEngine->Context(), NBRE_ModelPtr(model), 0);
    ent->AddRefSurface(m_textureSurface);

    node->AttachObject(NBRE_EntityPtr(ent));
    node->SetOrientation(NBRE_Orientation(NBRE_Vector3f(1, 0, 0), 90));
    node->SetPosition(NBRE_Vector3f(0, 10, 5));
    sceneMgr->RootSceneNode()->AddChild(NBRE_NodePtr(node));

    model = NBRE_NEW NBRE_Model(NBRE_MeshPtr(CreateSphere(*m_renderEngine->Context().mRenderPal, 1.0f)));
    model->SetShader(CreateMaterial(CreateTexture(m_renderEngine->Context(), "TestTex2.png")));
    ent = NBRE_NEW NBRE_Entity(m_renderEngine->Context(), NBRE_ModelPtr(model), 0);

    node = NBRE_NEW NBRE_Node;
    node->AttachObject(NBRE_EntityPtr(ent));
    node->SetPosition(NBRE_Vector3f(0, 0, 5));

    sceneMgr->RootSceneNode()->AddChild(NBRE_NodePtr(node));

    float vertex[9] = {-3.0f, 0.0f, 0.0f,
                        3.0f, 0.0f, 0.0f,
                        0.0f, 4.0f, 0.0f};

    model = NBRE_NEW NBRE_Model(NBRE_MeshPtr(CreatePolygon3D(*m_renderEngine->Context().mRenderPal, vertex, 3)));
    ent = NBRE_NEW NBRE_Entity(m_renderEngine->Context(), NBRE_ModelPtr(model), 0);

    node = NBRE_NEW NBRE_Node;
    node->AttachObject(NBRE_EntityPtr(ent));

    sceneMgr->RootSceneNode()->AddChild(NBRE_NodePtr(node));


    NBRE_CameraPtr camera(NBRE_NEW NBRE_Camera());
    camera->LookAt(NBRE_Vector3f(0, -10, 10), NBRE_Vector3f(0.0f, 0.0f, 0.0f), NBRE_Vector3f(0, 0, 1));
    sceneMgr->RootSceneNode()->AddChild(camera);
    NBRE_PerspectiveConfig perspective;
    perspective.mAspect = 1;
    perspective.mFov = 45;
    camera->GetFrustum().SetAsPerspective(perspective, 1, 100);

    m_CameraHandler1.SetRotateAnglePerAction(1.0f);
    m_CameraHandler1.SetSlideDistancePerAction(0.5f);
    m_CameraHandler1.BindCamera(camera.get());

    NBRE_RenderSurface* surface = m_renderEngine->Context().mSurfaceManager->CreateSurface();
    m_renderEngine->SetActiveRenderSurface(surface);
    CreateSurface(surface, camera, sceneMgr);

    camera = NBRE_CameraPtr(NBRE_NEW NBRE_Camera());
    camera->LookAt(NBRE_Vector3f(0, -10, 10), NBRE_Vector3f(0.0f, 0.0f, 0.0f), NBRE_Vector3f(0, 0, 1));

    sceneMgr->RootSceneNode()->AddChild(camera);

    perspective.mAspect = 1;
    perspective.mFov = 45;
    camera->GetFrustum().SetAsPerspective(perspective, 1, 100);

    m_CameraHandler2.SetRotateAnglePerAction(1.0f);
    m_CameraHandler2.SetSlideDistancePerAction(0.5f);
    m_CameraHandler2.BindCamera(camera.get());

    CreateSurface(m_textureSurface, camera, sceneMgr);

}

void CTestRenderTargetView::CreateSurface(NBRE_RenderSurface* surface, const NBRE_CameraPtr& camera, const NBRE_SceneManagerPtr& sceneMgr)
{
    int32 overlayId = mOverlayManager->AssignOverlayId(0);

    NBRE_Overlay* overlay = mOverlayManager->FindOverlay(overlayId);

    overlay->SetClearColor(NBRE_Color(0, 0, 0, 1));
    overlay->SetClearFlag(NBRE_CLF_CLEAR_COLOR|NBRE_CLF_CLEAR_DEPTH);

    NBRE_SurfaceSubView& subview = surface->CreateSubView(0, 0, 0, 0);
    subview.Viewport().SetCamera(camera);
    subview.SetSceneManager(sceneMgr);
    subview.SetOverlayManager(mOverlayManager);

    //subview.DebugEngine().AddDebugElement(spatialDivideDebug); 

}

int CTestRenderTargetView::OnCreate()
{
    m_renderEngine = CreateRenderEngine(m_RenderPal);
    mOverlayManager = NBRE_NEW NBRE_DefaultOverlayManager<int32>(m_renderEngine->Context(), &mMaxOverlayId);
    CreateScene();
    return 0;
}

void CTestRenderTargetView::OnSize(UINT nType, int cx, int cy)
{
    NBRE_RenderSurface* surface = m_renderEngine->ActiveRenderSurface();
    NBRE_SurfaceSubView& subView = surface->GetSubView(0);
    subView.Viewport().Update(0, 0, cx, cy);
    NBRE_PerspectiveConfig perspective;
    perspective.mAspect = float(cx)/cy;
    perspective.mFov = 45;
    subView.Viewport().Camera()->GetFrustum().SetAsPerspective(perspective, 1, 100);
    {
        NBRE_SurfaceSubView& subView = m_textureSurface->GetSubView(0);
        subView.Viewport().Update(0, 0, 256, 256);
        NBRE_PerspectiveConfig perspective;
        perspective.mAspect = 1.f;
        perspective.mFov = 45;
        subView.Viewport().Camera()->GetFrustum().SetAsPerspective(perspective, 1, 100);
    }
}

void CTestRenderTargetView::OnRender()
{
    GetWindow()->OnRenderBegine();
    m_renderEngine->Render();
    GetWindow()->OnRenderEnd();
}

void CTestRenderTargetView::OnTimer(UINT_PTR nIDEvent)
{
}

void CTestRenderTargetView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    m_CameraHandler1.OnKeyEvent(nChar);
    m_CameraHandler2.OnKeyEvent(nChar);
}
