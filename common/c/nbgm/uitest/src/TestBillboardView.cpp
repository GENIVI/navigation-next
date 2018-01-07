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
#include "TestBillboardView.h"
#include "nbrehardwareindexbuffer.h"
#include "nbrehardwarevertexbuffer.h"
#include "nbrecommon.h"
#include "nbgmTestUtilities.h"
#include "nbrelightnode.h"
#include "nbreipassoperation.h"
#include "nbretextureunit.h"
#include "nbrepngtextureimage.h"
#include "nbrefontmanager.h"
#include "nbrefreetypefont.h"
#include "nbreentity.h"
#include "nbredefaultspatialdividemanager.h"
#include "nbretypeconvert.h"
#include "nbgmbuildutility.h"

static const uint32 NUM_POI_TEXTURES = 3;
static char* poiTextures[] = {"ACC.png", "AE.png", "AIE.png"};
static const uint32 NUM_POIS = 100;
static PAL_Instance* pal = NULL;
static const uint32 NUM_PARTICLES = 500;
static const uint32 NUM_SHIELDS = 50;


static float RandFloat()
{
    return (rand() / static_cast<float>(RAND_MAX)) - 0.5f;
}

static NBRE_Vector3f RandVec3()
{
    NBRE_Vector3f vec(RandFloat(), RandFloat(), RandFloat());
    vec.Normalise();
    return vec;
}

CTestBillboardView::CTestBillboardView():
    m_ScreenHeight(1),
    m_ScreenWidth(1),
    m_renderEngine(NULL),
    m_node1(NULL),
    m_node2(NULL),
    mBillboardSet(NULL),
    m_particleBillboardSet(NULL),
    mMaxOverlayId(0)
{
}

CTestBillboardView::~CTestBillboardView()
{   
    for (ParticleList::iterator i = mParticles.begin(); i != mParticles.end(); ++i)
    {
        NBRE_DELETE *i;
    }
    NBRE_DELETE mBillboardSet;
    NBRE_DELETE m_particleBillboardSet;
    NBRE_DELETE mOverlayManager;
    m_renderEngine->Deinitialize();
    NBRE_DELETE m_renderEngine;
}


int CTestBillboardView::OnCreate()
{
    m_renderEngine = CreateRenderEngine(m_RenderPal);
    mOverlayManager = NBRE_NEW NBRE_DefaultOverlayManager<int32>(m_renderEngine->Context(), &mMaxOverlayId);
    CreateMaterial();

    NBRE_Context& context = m_renderEngine->Context();
    NBRE_ShaderManager& sm = *context.mShaderManager;
    NBRE_IRenderPal& renderer = *context.mRenderPal;

    NBRE_DefaultSpatialDivideManager* spatialDivideManager = NBRE_NEW NBRE_DefaultSpatialDivideManager(1);
    NBRE_SceneManager* sceneMgr = NBRE_NEW NBRE_SceneManager;
    sceneMgr->SetSpatialDivideStrategy(spatialDivideManager);

    int32 overlayId = mOverlayManager->AssignOverlayId(0);
    NBRE_Overlay* overlay = mOverlayManager->FindOverlay(overlayId);

    overlay->SetClearColor(NBRE_Color(0, 0, 0, 1));
    overlay->SetClearFlag(NBRE_CLF_CLEAR_COLOR|NBRE_CLF_CLEAR_DEPTH);


    NBRE_Model* model = NBRE_NEW NBRE_Model(NBRE_MeshPtr(CreateGround(renderer, 1.0f, 100)));
    model->SetShaderName("sphere");

    NBRE_EntityPtr groundEntity(NBRE_NEW NBRE_Entity(context, NBRE_ModelPtr(model), overlayId));
    NBRE_Node* groundNode = NBRE_NEW NBRE_Node();
    groundNode->AttachObject(groundEntity);
    sceneMgr->RootSceneNode()->AddChild(NBRE_NodePtr(groundNode));

    // Sphere mesh
    NBRE_MeshPtr mesh(CreateSphere(renderer, 1.0f));
    // Shpere model
    model = NBRE_NEW NBRE_Model(mesh);
    // Shader name must be predefined in shader manager
    model->SetShaderName("sphere");
    NBRE_ModelPtr modelPtr(model);

    NBRE_Entity* ent = NBRE_NEW NBRE_Entity(context, modelPtr, overlayId);

    // Center sphere
    m_node1 = NBRE_NEW NBRE_Node();
    m_node1->AttachObject(NBRE_EntityPtr(ent));
    sceneMgr->RootSceneNode()->AddChild(NBRE_NodePtr(m_node1));

    // Rotate sphere
    ent = NBRE_NEW NBRE_Entity(context, modelPtr, overlayId);
    m_node2 = NBRE_NEW NBRE_Node();
    m_node2->AttachObject(NBRE_EntityPtr(ent));
    m_node1->AddChild(NBRE_NodePtr(m_node2));
    m_node2->SetPosition(NBRE_Vector3f(5, 0, 0));
    m_node2->SetScale(NBRE_Vector3f(0.5f, 0.5f, 0.5f));

    // Create billboard set node
    // Note: 
    // 1. To render billboards properly, billboard set MUST be placed behind all nodes in overlay.
    //    Or just set its overlay id to a higher value.
    // 2. It's better to merge billboard images into a single texture(or several) when billboard number is large.
    //    If all billboards in a set use same shader, the performance is best(only one draw call).
    mBillboardSet = NBRE_NEW NBRE_BillboardSet(m_renderEngine->Context(), 0);

    // Create billboard nodes
    for (uint32 i = 0; i < NUM_POIS; ++i)
    {
        int32 poiIndex = rand() % NUM_POI_TEXTURES;
        NBRE_Vector3f pos(RandFloat() * 100.0f, RandFloat() * 100.0f, 0.0f);

        // Every billboard has its own size, shader, color and texture coordinates.
        // The shader must have a texture unit.
        NBRE_BillboardNodePtr poi = mBillboardSet->AddBillboard();
        poi->AddEntity(NBRE_BillboardEntityPtr(NBRE_NEW NBRE_ImageEntity(m_renderEngine->Context()
            , NBRE_Vector2f(0.5f, 0.5f)
            , sm.GetShader(poiTextures[poiIndex])
            , NBRE_Color(1.0f, 1.0f, 1.0f, 1.0f))));

        poi->SetPosition(pos);
        sceneMgr->RootSceneNode()->AddChild(poi);
    }
    
    NBRE_FontMaterial fontMaterial;
    fontMaterial.foreColor.r = 1.0f;
    fontMaterial.foreColor.g = 1.0f;
    fontMaterial.foreColor.b = 1.0f;
    fontMaterial.backColor.a = 0.0f;
    // Create shield nodes
    for (uint32 i = 0; i < NUM_SHIELDS; ++i)
    {
        char num[5];
        sprintf(num, "%d", rand() % 200);
        NBRE_Vector3f pos(RandFloat() * 100.0f, RandFloat() * 100.0f, 0.0f);

        NBRE_BillboardNodePtr shield = mBillboardSet->AddBillboard();
        shield->AddEntity(NBRE_BillboardEntityPtr(NBRE_NEW NBRE_ImageEntity(m_renderEngine->Context(), NBRE_Vector2f(1.0f, 1.0f)
            , sm.GetShader("shield")
            , NBRE_Color(1.0f, 1.0f, 1.0f, 1.0f))));
        
        shield->AddEntity(NBRE_BillboardEntityPtr(NBRE_NEW NBRE_TextAreaEntity(m_renderEngine->Context(), 
            num, fontMaterial, 0.5f, PAL_TA_CENTER)));

        shield->SetPosition(pos);
        sceneMgr->RootSceneNode()->AddChild(shield);
    }


    // Simulate a simple particle system.
    // Better to implement in some other node type
    m_particleBillboardSet = NBRE_NEW NBRE_BillboardSet(m_renderEngine->Context(), 1);
    for (uint32 i = 0; i < NUM_PARTICLES; ++i)
    {
        NBRE_Vector3f pos(RandFloat() * 1.0f, RandFloat() * 1.0f, 0.0f);

        NBRE_BillboardNodePtr particle = mBillboardSet->AddBillboard();
        particle->AddEntity(NBRE_BillboardEntityPtr(NBRE_NEW NBRE_ImageEntity(m_renderEngine->Context(), NBRE_Vector2f(0.1f, 0.1f)
            , sm.GetShader("particle")
            , NBRE_Color(0.5f, 0.3f, 1.0f, 0.3f))));

        sceneMgr->RootSceneNode()->AddChild(particle);
        mParticles.push_back(new Particle(particle, (RandFloat() + 0.5f)));
    }

    // Set camera
    mCamera = NBRE_CameraPtr(NBRE_NEW NBRE_Camera());
    mCamera->LookAt(NBRE_Vector3f(0, -10, 10), NBRE_Vector3f(0.0f, 0.0f, 0.0f), NBRE_Vector3f(0, 0, 1));
    sceneMgr->RootSceneNode()->AddChild(mCamera);
    NBRE_PerspectiveConfig perspective;
    perspective.mAspect = 1;
    perspective.mFov = 45;
    mCamera->GetFrustum().SetAsPerspective(perspective, 1, 100);

    NBRE_RenderSurface* surface = context.mSurfaceManager->CreateSurface();
    m_renderEngine->SetActiveRenderSurface(surface);

    NBRE_SurfaceSubView& subview = surface->CreateSubView(0, 0, 0, 0);
    subview.Viewport().SetCamera(mCamera);
    subview.SetSceneManager(NBRE_SceneManagerPtr(sceneMgr));
    subview.SetOverlayManager(mOverlayManager);

    //subview.DebugEngine().AddDebugElement(spatialDivideDebug); 

    subview.AddVisitable(mBillboardSet);
    subview.AddVisitable(m_particleBillboardSet);

    return 0;
}

void CTestBillboardView::OnSize(UINT nType, int cx, int cy)
{
    NBRE_RenderSurface* surface = m_renderEngine->ActiveRenderSurface();
    NBRE_SurfaceSubView& subView = surface->GetSubView(0);

    subView.Viewport().Update(0, 0, cx, cy);

    NBRE_PerspectiveConfig perspective;
    perspective.mAspect = float(cx)/cy;
    perspective.mFov = 45;
    subView.Viewport().Camera()->GetFrustum().SetAsPerspective(perspective, 1, 100);
}

void CTestBillboardView::Update()
{
    static clock_t lastC = clock();
    clock_t c = clock();
    float secondsPassed = (c - lastC) / static_cast<float>(CLOCKS_PER_SEC);

    m_node1->SetOrientation(NBRE_Orientation(NBRE_Vector3f(0,0,1), static_cast<float>(c) / CLOCKS_PER_SEC * 60.0f));
    m_node2->SetOrientation(NBRE_Orientation(NBRE_Vector3f(0,0,1), static_cast<float>(c) / CLOCKS_PER_SEC * 60.0f));

    // Update particles
    NBRE_Vector3f pos = NBRE_Transformationf::CoordinatesGetTranslate(NBRE_TypeConvertf::Convert(m_node2->WorldTransform()));
    
    for (ParticleList::iterator i = mParticles.begin(); i != mParticles.end(); ++i)
    {
        Particle* particle = *i;
        particle->Update(secondsPassed);
        if (particle->IsDead())
        {
            // Regenerate dead particles in node2 center
            particle->SetPosition(pos);
            particle->SetVelocity(RandVec3() * (RandFloat() + 0.5f) * 2.0f);
            particle->Activate(RandFloat() + 0.5f);
        }
    }
    lastC = c;
}

void CTestBillboardView::OnRender()
{
    GetWindow()->OnRenderBegine();
    Update();
    m_renderEngine->Render();
    GetWindow()->OnRenderEnd();
}

void CTestBillboardView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    m_CameraHandler.SetRotateAnglePerAction(1.0f);
    m_CameraHandler.SetSlideDistancePerAction(0.5f);
    m_CameraHandler.BindCamera(mCamera.get());
    m_CameraHandler.OnKeyEvent(nChar);
}


static NBRE_PassPtr CreateShaderAndGetDefaultPass(NBRE_ShaderManager* sm, const char* shaderName)
{
    NBRE_PassPtr passPtr = NBRE_PassPtr(NBRE_NEW NBRE_Pass);
    NBRE_ShaderPtr shaderPtr = NBRE_ShaderPtr(NBRE_NEW NBRE_Shader());
    shaderPtr->AddPass(passPtr);
    sm->SetShader(shaderName, shaderPtr);
    return passPtr;
}

NBRE_TexturePtr CTestBillboardView::CreateTexture(NBRE_Context& context, const char* name)
{
    char fullPath[MAX_PATH + 1] = {0};
    char *folder = GetExePath();
    sprintf_s(fullPath, MAX_PATH + 1, "%s\\%s", folder, name);
    NBRE_DELETE []folder;

    NBRE_TexturePtr texture = NBGM_BuildTextureUtility::ConstructImageTexture(*context.mRenderPal, context.mPalInstance, fullPath, 0, FALSE, NBRE_Texture::TT_2D, TRUE);
    if(texture.get())
    {
        texture->Load();
    }
    mTextures.push_back(texture);
    return texture;
}

static NBRE_TexturePtr
CreateCreateColorTexture(NBRE_Context& context, const NBRE_Color& c)
{
    NBRE_TexturePtr texture = NBGM_BuildTextureUtility::ConstructColorTexture(*context.mRenderPal, c, "");
    return texture;
}

void CTestBillboardView::CreateMaterial()
{
    CreateTexture(m_renderEngine->Context(), "TestTex2.png");
    CreateTexture(m_renderEngine->Context(), "light.png");
    CreateTexture(m_renderEngine->Context(), "interstate.png");
    NBRE_ShaderManager* sm = m_renderEngine->Context().mShaderManager;

    {
        NBRE_PassPtr pass1 = CreateShaderAndGetDefaultPass(sm, "sphere");
        pass1->SetEnableLighting(FALSE);
        pass1->SetEnableDepthTest(TRUE);
        pass1->SetEnableDepthWrite(TRUE);
        pass1->SetEnableCullFace(TRUE);
        pass1->SetEnableBlend(FALSE);

        NBRE_TextureUnit unit0;
        unit0.SetTexture(mTextures[0]);
        pass1->GetTextureUnits().push_back(unit0);
    }

    {
        NBRE_PassPtr pass1 = CreateShaderAndGetDefaultPass(sm, "particle");
        pass1->SetSrcBlend(NBRE_BF_SRC_ALPHA);
        pass1->SetDstBlend(NBRE_BF_ONE);
        NBRE_TextureUnit unit0;
        unit0.SetTexture(mTextures[1]);
        pass1->GetTextureUnits().push_back(unit0);
    }

    {
        NBRE_PassPtr pass1 = CreateShaderAndGetDefaultPass(sm, "shield");
        pass1->SetEnableLighting(FALSE);
        pass1->SetEnableDepthTest(TRUE);
        pass1->SetEnableDepthWrite(TRUE);
        pass1->SetEnableCullFace(FALSE);
        pass1->SetEnableBlend(FALSE);

        NBRE_TextureUnit unit0;
        unit0.SetTexture(mTextures[2]);
        unit0.SetState(NBRE_TextureState(NBRE_TFT_MIPMAP_TRILINEAR, NBRE_TFT_LINEAR, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, FALSE));
        pass1->GetTextureUnits().push_back(unit0);
    }

    for (uint32 i = 0 ;i  < NUM_POI_TEXTURES; ++i)
    {
        CreateTexture(m_renderEngine->Context(), poiTextures[i]);
        NBRE_PassPtr pass1 = CreateShaderAndGetDefaultPass(sm, poiTextures[i]);
        pass1->SetSrcBlend(NBRE_BF_SRC_ALPHA);
        pass1->SetDstBlend(NBRE_BF_ONE_MINUS_SRC_ALPHA);
        NBRE_TextureUnit unit0;
        unit0.SetTexture(mTextures[3 + i]);
        pass1->GetTextureUnits().push_back(unit0);

        NBRE_TextureUnit unit1;
        unit1.SetTexture(CreateCreateColorTexture(m_renderEngine->Context(), NBRE_Color(1, 0, 0, 1)));
        pass1->GetTextureUnits().push_back(unit1);
    }
}
