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
#include "TestCubeTextureView.h"
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
#include "nbrepngcodec.h"
#include "nbredefaultspatialdividemanager.h"
#include "nbrecommon.h"
#include "nbgmbuildutility.h"

CTestCubeTextureView::CTestCubeTextureView():
    m_ScreenHeight(1),
    m_ScreenWidth(1),
    m_renderEngine(NULL),
    m_node(NULL),
    mMaxOverlayId(0)
{
}

CTestCubeTextureView::~CTestCubeTextureView()
{
    NBRE_DELETE mOverlayManager;
    m_renderEngine->Deinitialize();
    NBRE_DELETE m_renderEngine;
}

static NBRE_ShaderPtr CreateGroundMaterial()
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

class NBRE_CubePngTextureImage: public NBRE_ITextureImage
{
public:
    NBRE_CubePngTextureImage(char* path[6])
    {
        for(int32 i = 0; i < 6; ++i)
        {
            mImage[i] = NBRE_NEW NBRE_PngTextureImage(*GetPal(), path[i], 0, TRUE);
        }
    }
    virtual ~NBRE_CubePngTextureImage()
    {
        for(int32 i = 0; i < 6; ++i)
        {
            NBRE_DELETE mImage[i];
        }
    }

public:
    /// Release resource
    virtual void Release()
    {
        NBRE_DELETE this;
    }
    /// Get mipmaps count
    virtual uint8 GetMipmapCount() const { return 1; }
    /// Get count of faces
    virtual uint8 GetFaceCount() const { return 6; }
    /// Get a image of mipmap level, only level 0 is supported
    virtual const NBRE_Image* GetImage(uint8 face, uint8 mipmapLevel) const
    {
        return mImage[face]->GetImage(0, 0);
    }
    /// Load image from file
    virtual PAL_Error Load()
    {
        for(int32 i = 0; i < 6; ++i)
        {
            mImage[i]->Load();
        }
        return PAL_Ok;
    }

    virtual void ReadInfo(NBRE_ImageInfo &fm,  NBRE_ImageInfo &originalInfo) const
    {
        mImage[0]->ReadInfo(fm, originalInfo);
    }

private:
    DISABLE_COPY_AND_ASSIGN(NBRE_CubePngTextureImage);

protected:
    NBRE_PngTextureImage* mImage[6];
};

static NBRE_TexturePtr CreateCubeTexture(NBRE_Context& context)
{
    char** fullPath = NBRE_NEW char*[6];
    int32 i = 0;
    for(i = 0; i < 6; ++i)
    {
        fullPath[i] = NBRE_NEW char[MAX_PATH + 1];
    }
    char *folder = GetExePath();

    i = 0;
    sprintf_s(fullPath[i++], MAX_PATH + 1, "%s\\%s", folder, "east.PNG");
    sprintf_s(fullPath[i++], MAX_PATH + 1, "%s\\%s", folder, "west.PNG");
    sprintf_s(fullPath[i++], MAX_PATH + 1, "%s\\%s", folder, "up.PNG");
    sprintf_s(fullPath[i++], MAX_PATH + 1, "%s\\%s", folder, "up.PNG");
    sprintf_s(fullPath[i++], MAX_PATH + 1, "%s\\%s", folder, "north.PNG");
    sprintf_s(fullPath[i++], MAX_PATH + 1, "%s\\%s", folder, "south.PNG");

    NBRE_DELETE []folder;

    NBRE_ITextureImage* textureImage = NBRE_NEW NBRE_CubePngTextureImage(fullPath);
    NBRE_TexturePtr texture = NBGM_BuildTextureUtility::ConstructCommonTexture(*context.mRenderPal, "", textureImage, NBRE_Texture::TT_CUBE_MAP, FALSE);
    if(texture.get())
    {
        texture->Load();
    }

    for(i = 0; i < 6; ++i)
    {
        NBRE_DELETE_ARRAY fullPath[i];
    }
    NBRE_DELETE_ARRAY fullPath;

    return texture;
}

static NBRE_ShaderPtr CreateMaterial(NBRE_Context& context)
{
    NBRE_PassPtr pass = NBRE_PassPtr(NBRE_NEW NBRE_Pass);
    NBRE_ShaderPtr shaderPtr(NBRE_NEW NBRE_Shader());
    shaderPtr->AddPass(pass);

    NBRE_TexturePtr texture = CreateTexture(context, "TestTex2.png");

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

static NBRE_ShaderPtr CreateCubeMaterial(NBRE_Context& context)
{
    NBRE_PassPtr pass = NBRE_PassPtr(NBRE_NEW NBRE_Pass);
    NBRE_ShaderPtr shaderPtr(NBRE_NEW NBRE_Shader());
    shaderPtr->AddPass(pass);

    NBRE_TexturePtr texture = CreateCubeTexture(context);

    pass->SetEnableLighting(FALSE);
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(TRUE);
    pass->SetEnableCullFace(TRUE);
    pass->SetEnableBlend(FALSE);
    pass->SetColor(NBRE_Color(1.0f, 1.0f, 1.0f, 1.0f));

    NBRE_TextureUnit unit0;
    
    NBRE_TextureState st;
    st.SetWrapS(NBRE_TAM_CLAMP);
    st.SetWrapT(NBRE_TAM_REPEAT)
     ;

    unit0.SetState(st);
    unit0.SetTransform(NBRE_Transformationf::BuildRotateDegreesMatrix(90, NBRE_Vector3f(1, 0, 0)));
    unit0.SetTexture(texture);
    pass->GetTextureUnits().push_back(unit0);
    
    return shaderPtr;
}

static NBRE_ShaderPtr CreateSphereMaterial(NBRE_Context& context)
{
    NBRE_PassPtr pass = NBRE_PassPtr(NBRE_NEW NBRE_Pass);
    NBRE_ShaderPtr shaderPtr(NBRE_NEW NBRE_Shader());
    shaderPtr->AddPass(pass);

    NBRE_TexturePtr texture = CreateCubeTexture(context);

    pass->SetEnableLighting(FALSE);
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(TRUE);
    pass->SetEnableCullFace(TRUE);
    pass->SetEnableBlend(FALSE);
    pass->SetColor(NBRE_Color(1.0f, 1.0f, 1.0f, 1.0f));

    NBRE_TextureUnit unit0;

    NBRE_TextureState st;
    st.SetWrapS(NBRE_TAM_CLAMP);
    st.SetWrapT(NBRE_TAM_REPEAT)
        ;

    unit0.SetState(st);
    unit0.SetTransform(NBRE_Transformationf::BuildRotateDegreesMatrix(90, NBRE_Vector3f(1, 0, 0)));
    unit0.SetTexture(texture);
    pass->GetTextureUnits().push_back(unit0);
    
    return shaderPtr;
}


void CTestCubeTextureView::CreateScene()
{
    NBRE_SceneManager* sceneMgr = NBRE_NEW NBRE_SceneManager();
    sceneMgr->SetSpatialDivideStrategy(NBRE_NEW NBRE_DefaultSpatialDivideManager(1));

    int32 overlayId = mOverlayManager->AssignOverlayId(0);
    NBRE_Overlay* overlay = mOverlayManager->FindOverlay(overlayId);

    NBRE_Node* node ;
    // Sphere mesh
    NBRE_MeshPtr cubemesh(CreateCube(*m_renderEngine->Context().mRenderPal,1.0f, 1.0f, 1.0f));
    //NBRE_MeshPtr mesh(CreateSphere(1.0f));
    NBRE_Model* model = NBRE_NEW NBRE_Model(cubemesh);
    model->SetShader(CreateCubeMaterial(m_renderEngine->Context()));
    NBRE_Entity* ent = NBRE_NEW NBRE_Entity(m_renderEngine->Context(), NBRE_ModelPtr(model), overlayId);

    m_node = NBRE_NEW NBRE_Node();
    m_node->AttachObject(NBRE_EntityPtr(ent));
    m_node->SetPosition(NBRE_Vector3f(0, 0, 1));
    sceneMgr->RootSceneNode()->AddChild(NBRE_NodePtr(m_node));


    NBRE_MeshPtr spheremesh(Create3DTextureSphere(*m_renderEngine->Context().mRenderPal, 1.0f));
    model = NBRE_NEW NBRE_Model(spheremesh);
    model->SetShader(CreateSphereMaterial(m_renderEngine->Context()));
    ent = NBRE_NEW NBRE_Entity(m_renderEngine->Context(), NBRE_ModelPtr(model), overlayId);

    node = NBRE_NEW NBRE_Node();
    node->AttachObject(NBRE_EntityPtr(ent));
    node->SetPosition(NBRE_Vector3f(4, 0, 0));
    m_node->AddChild(NBRE_NodePtr(node));

    //NBRE_Skywall* sky = NBRE_NEW NBRE_Skywall(0); 
    //sky->SetShader(CreateMaterial());
    //sky->SetSize(80, 10, 50);

    //sceneMgr->AddVisitable(sky);

    model = NBRE_NEW NBRE_Model(NBRE_MeshPtr(CreateGround(*m_renderEngine->Context().mRenderPal, 1, 100)));
    model->SetShader(CreateGroundMaterial());
    ent = NBRE_NEW NBRE_Entity(m_renderEngine->Context(), NBRE_ModelPtr(model), overlayId);

    node = NBRE_NEW NBRE_Node();
    node->AttachObject(NBRE_EntityPtr(ent));
    sceneMgr->RootSceneNode()->AddChild(NBRE_NodePtr(node));


    NBRE_CameraPtr camera(NBRE_NEW NBRE_Camera());
    camera->LookAt(NBRE_Vector3f(0, -10, 10), NBRE_Vector3f(0.0f, 0.0f, 0.0f), NBRE_Vector3f(0, 0, 1));

    //m_node->AddChild(camera);
    sceneMgr->RootSceneNode()->AddChild(camera);


    NBRE_PerspectiveConfig perspective;
    perspective.mAspect = 1;
    perspective.mFov = 45;
    camera->GetFrustum().SetAsPerspective(perspective, 1, 100);

    //m_camera = NBRE_CameraPtr(NBRE_NEW NBRE_Camera());
    //m_camera->LookAt(NBRE_Vector3f(0, 0, 5), NBRE_Vector3f(0, 0, 0), NBRE_Vector3f(0, 1, 0));
    //m_node->AddChild(m_camera);

    //m_camera->GetFrustum().SetAsPerspective(perspective, 1, 10);

    //debugEngine->AddCameraDebug(m_camera.Get(), 0); 

    NBRE_RenderSurface* surface = m_renderEngine->Context().mSurfaceManager->CreateSurface();
    m_renderEngine->SetActiveRenderSurface(surface);

    NBRE_SurfaceSubView& subview = surface->CreateSubView(0, 0, 0, 0);
    subview.Viewport().SetCamera(camera);
    subview.SetSceneManager(NBRE_SceneManagerPtr(sceneMgr));
    subview.SetOverlayManager(mOverlayManager);



    overlay->SetClearColor(NBRE_Color(0, 0, 0, 1));
    overlay->SetClearFlag(NBRE_CLF_CLEAR_COLOR|NBRE_CLF_CLEAR_DEPTH);


    m_CameraHandler.SetRotateAnglePerAction(1.0f);
    m_CameraHandler.SetSlideDistancePerAction(0.5f);
    m_CameraHandler.BindCamera(camera.get());
}

int CTestCubeTextureView::OnCreate()
{
    m_renderEngine = CreateRenderEngine(m_RenderPal);
    mOverlayManager = NBRE_NEW NBRE_DefaultOverlayManager<int32>(m_renderEngine->Context(), &mMaxOverlayId);
    CreateScene();
    return 0;
}

void CTestCubeTextureView::OnSize(UINT nType, int cx, int cy)
{
    NBRE_RenderSurface* surface = m_renderEngine->ActiveRenderSurface();
    NBRE_SurfaceSubView& subView = surface->GetSubView(0);

    subView.Viewport().Update(0, 0, cx, cy);

    NBRE_PerspectiveConfig perspective;
    perspective.mAspect = float(cx)/cy;
    perspective.mFov = 45;
    subView.Viewport().Camera()->GetFrustum().SetAsPerspective(perspective, 1, 100);
}

void CTestCubeTextureView::OnRender()
{
    GetWindow()->OnRenderBegine();
    m_renderEngine->Render();
    GetWindow()->OnRenderEnd();
}

void CTestCubeTextureView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    m_CameraHandler.OnKeyEvent(nChar);
    switch(nChar)
    {
    case('I'):
        {
            m_node->Rotate(NBRE_Vector3f(1.0f, 0.0f, 0.0f), 1);
        }
        break;
    case('K'):
        {
            m_node->Rotate(NBRE_Vector3f(1.0f, 0.0f, 0.0f), -1);
        }
        break;

    case('J'):
        {
            m_node->Rotate(NBRE_Vector3f(0.0f, 0.0f, 1.0f), 1);
        }
        break;
    case('L'):
        {
            m_node->Rotate(NBRE_Vector3f(0.0f, 0.0f, 1.0f), -1);
        }
        break;


    default:
        break;
    }
}
