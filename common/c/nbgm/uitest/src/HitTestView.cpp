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
#include "HitTestView.h"
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
#include "nbrecameradebug.h"
#include "nbredefaultspatialdividemanager.h"
#include "nbrebillboardnode.h"
#include "nbrepngtextureimage.h"
#include "nbgmskywall.h"
#include "nbrespatialdividemanagerdebug.h"
#include "nbredebugray.h"
#include "nbrecommon.h"
#include "nbretypeconvert.h"
#include "nbgmbuildutility.h"


static const uint32 NUM_POI_TEXTURES = 3;
static char* poiTextures[] = {"ACC.png", "AE.png", "AIE.png"};
static const uint32 NUM_POIS = 100;
static const uint32 NUM_PARTICLES = 500;
static const uint32 NUM_SHIELDS = 50;


CHitTestView::CHitTestView():
    m_ScreenHeight(1),
    m_ScreenWidth(1),
    m_renderEngine(NULL),
    m_node1(NULL),
    m_node2(NULL),
    m_subview(NULL),
    m_curCamera(NULL),
    mPal(NULL),
    m_sceneMgr(NULL),
    m_debugRay(NULL),
    m_debugFrustum(NULL),
    mBillboardSet(NULL),
    m_animation(true),
    mMaxOverlayId(0)
{
}

CHitTestView::~CHitTestView()
{
    NBRE_DELETE mBillboardSet;
    NBRE_DELETE mOverlayManager;
    m_renderEngine->Deinitialize();
    NBRE_DELETE m_renderEngine;
}

NBRE_VertexData* CHitTestView::CreateTriangleVertexData()
{
    static float pos[9] = {
        -0.5, -0.5, 0,
        0.5, -0.5, 0,
        0, 1, 0
    };
    static float col[9] = {
        1, 0, 0,
        0, 1, 0,
        0, 0, 1
    };


    //create vertex data
    NBRE_VertexElement* positionElement = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    NBRE_HardwareVertexBuffer* vertexBuffer = m_renderEngine->Context().mRenderPal->CreateVertexBuffer(
        sizeof(float)*NBRE_VertexElement::GetTypeCount(NBRE_VertexElement::VET_FLOAT3), 3, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    static_cast<NBRE_HardwareBuffer*>(vertexBuffer)->WriteData(0, sizeof(pos), pos, FALSE);

    NBRE_VertexElement* colorElement = NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_DIFFUSE, 0);
    NBRE_HardwareVertexBuffer* clolorBuffer = m_renderEngine->Context().mRenderPal->CreateVertexBuffer(
        sizeof(float)*NBRE_VertexElement::GetTypeCount(NBRE_VertexElement::VET_FLOAT3), 3, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    static_cast<NBRE_HardwareBuffer*>(clolorBuffer)->WriteData(0, sizeof(col), col, FALSE);


    NBRE_VertexDeclaration* vertexDeclaration = m_renderEngine->Context().mRenderPal->CreateVertexDeclaration();
    vertexDeclaration->GetVertexElementList().push_back(positionElement);
    vertexDeclaration->GetVertexElementList().push_back(colorElement);
    
    NBRE_VertexData* vertexData = NBRE_NEW NBRE_VertexData(0);
    vertexData->AddBuffer(vertexBuffer);
    vertexData->AddBuffer(clolorBuffer);

    vertexData->SetVertexDeclaration(vertexDeclaration);

    return vertexData;
}

NBRE_IndexData* CHitTestView::CreateTriangleIndexData()
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


NBRE_Mesh* CHitTestView::CreateTriangle()
{
    NBRE_Mesh* mesh = NBRE_NEW NBRE_Mesh(CreateTriangleVertexData());

    mesh->CreateSubMesh(CreateTriangleIndexData(), NBRE_PMT_TRIANGLE_LIST);
    mesh->CalculateBoundingBox();

    return mesh;
}

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

static NBRE_PassPtr CreateShaderAndGetDefaultPass(NBRE_ShaderManager* shaderManager, const char* shaderName)
{
    NBRE_PassPtr passPtr = NBRE_PassPtr(NBRE_NEW NBRE_Pass);
    NBRE_ShaderPtr shaderPtr = NBRE_ShaderPtr(NBRE_NEW NBRE_Shader());
    shaderPtr->AddPass(passPtr);
    shaderManager->SetShader(shaderName, shaderPtr);
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
    pass->SetColor(NBRE_Color(1.0f, 1.0f, 1.0f, 1.0f));

    
    return shaderPtr;
}


void CHitTestView::CreateScene(NBRE_Context& context)
{
    int32 overlayId = mOverlayManager->AssignOverlayId(0);
    NBRE_Overlay* overlay = mOverlayManager->FindOverlay(overlayId);
    overlay->SetClearColor(NBRE_Color(0, 0, 0, 1));
    overlay->SetClearFlag(NBRE_CLF_CLEAR_COLOR|NBRE_CLF_CLEAR_DEPTH);

    for (uint32 i = 0 ;i  < NUM_POI_TEXTURES; ++i)
    {
        NBRE_PassPtr pass1 = CreateShaderAndGetDefaultPass(context.mShaderManager, poiTextures[i]);

        pass1->SetSrcBlend(NBRE_BF_SRC_ALPHA);
        pass1->SetDstBlend(NBRE_BF_ONE_MINUS_SRC_ALPHA);
        NBRE_TextureUnit unit0;
        unit0.SetTexture(CreateTexture(context, poiTextures[i]));
        pass1->GetTextureUnits().push_back(unit0);
    }

    NBRE_DefaultSpatialDivideManager* spatialDivideManager = NBRE_NEW NBRE_DefaultSpatialDivideManager(1);
    NBRE_SceneManager* sceneMgr = NBRE_NEW NBRE_SceneManager();
    m_sceneMgr = sceneMgr;
    sceneMgr->SetSpatialDivideStrategy(spatialDivideManager);

    NBRE_MeshPtr mesh(CreateTriangle());

    NBRE_Model* model = NBRE_NEW NBRE_Model(mesh);

    NBRE_ModelPtr modelptr(model);
    NBRE_Entity* ent = NBRE_NEW NBRE_Entity(context, modelptr, overlayId);
    ent->SetPickListener(this);

    m_node1 = NBRE_NEW NBRE_Node();
    m_node1->AttachObject(NBRE_EntityPtr(ent));

    ent = NBRE_NEW NBRE_Entity(context, modelptr, overlayId);
    ent->SetPickListener(this);

    m_node2 = NBRE_NEW NBRE_Node();
    m_node2->AttachObject(NBRE_EntityPtr(ent));

    sceneMgr->RootSceneNode()->AddChild(NBRE_NodePtr(m_node1));
    m_node1->AddChild(NBRE_NodePtr(m_node2));

    m_node2->SetPosition(NBRE_Vector3f(2, 0, 0));
    m_node2->SetScale(NBRE_Vector3f(0.5f, 0.5f, 0.5f));

    model = NBRE_NEW NBRE_Model(NBRE_MeshPtr(CreateGround(*context.mRenderPal, 1, 100, 0.5f, 0.5f, 0.5f)));
    model->SetShader(CreateGroundMaterial());
    ent = NBRE_NEW NBRE_Entity(context, NBRE_ModelPtr(model), overlayId);

    NBRE_Node* node = NBRE_NEW NBRE_Node;
    node->AttachObject(NBRE_EntityPtr(ent));
    sceneMgr->RootSceneNode()->AddChild(NBRE_NodePtr(node));

    NBRE_PerspectiveConfig perspective;

    NBRE_CameraPtr camera(NBRE_NEW NBRE_Camera());
    //camera->LookAt(NBRE_Vector3f(0, 0, 10), NBRE_Vector3f(0, 0, 0), NBRE_Vector3f(0, 1, 0));
    camera->LookAt(NBRE_Vector3f(0, -10, 10), NBRE_Vector3f(0.0f, 0.0f, 0.0f), NBRE_Vector3f(0, 0, 1));
    //camera->SetScale(NBRE_Vector3f(5, 5, 5));

    perspective.mAspect = 1;
    perspective.mFov = 45;
    camera->GetFrustum().SetAsPerspective(perspective, 1, 100);

    m_curCamera = camera.get();

    sceneMgr->RootSceneNode()->AddChild(camera);

    // Create billboard set node
    // Note: 
    // 1. To render billboards properly, billboard set MUST be placed behind all nodes in overlay.
    //    Or just set its overlay id to a higher value.
    // 2. It's better to merge billboard images into a single texture(or several) when billboard number is large.
    //    If all billboards in a set use same shader, the performance is best(only one draw call).
    mBillboardSet = NBRE_NEW NBRE_BillboardSet(m_renderEngine->Context(), 0);

    // Create billboard nodes
    //for (uint32 i = 0; i < 50; ++i)
    //{
    //    int32 poiIndex = rand() % 3;
    //    NBRE_Vector3f pos(RandFloat() * 100.0f, RandFloat() * 100.0f, 0.0f);

    //    // Every billboard has its own size, shader, color and texture coordinates.
    //    // The shader must have a texture unit.
    //    NBRE_BillboardNodePtr poi = mBillboardSet->AddBillboard();
    //    poi->AddEntity(NBRE_BillboardEntityPtr(NBRE_NEW NBRE_ImageEntity(NBRE_Vector2f(0.5f, 0.5f)
    //        , NBRE_ShaderManager::GetSingleton().GetShader(poiTextures[poiIndex])
    //        , NBRE_Color(1.0f, 1.0f, 1.0f, 1.0f))));

    //    poi->SetPosition(pos);
    //    sceneMgr->RootSceneNode()->AddChild(poi);
    //}



    NBRE_RenderSurface* surface = context.mSurfaceManager->CreateSurface();
    m_renderEngine->SetActiveRenderSurface(surface);

    NBRE_SurfaceSubView& subview = surface->CreateSubView(0, 0, 0, 0);
    
    m_subview = &subview;

    subview.Viewport().SetCamera(camera);
    subview.SetSceneManager(NBRE_SceneManagerPtr(sceneMgr));
    subview.SetOverlayManager(mOverlayManager);


    NBRE_SpatialDivideManagerDebug* spatialDivideDebug = NBRE_NEW NBRE_SpatialDivideManagerDebug(context, *spatialDivideManager, 0);
    spatialDivideDebug->SetOnlyShowHasObj(TRUE);
    subview.AddVisitable(spatialDivideDebug); 

    m_debugRay = NBRE_NEW NBRE_DebugRay(context, 0);
    m_debugRay->SetColor(NBRE_Color(1.0f, 0.0f, 0.0f, 1.0f));
    subview.AddVisitable(m_debugRay); 

    m_debugFrustum = NBRE_NEW NBRE_DebugFrustum(context, 0);
    subview.AddVisitable(m_debugFrustum); 



    subview.AddVisitable(mBillboardSet);



    m_CameraHandler.SetRotateAnglePerAction(1.0f);
    m_CameraHandler.SetSlideDistancePerAction(0.5f);
    m_CameraHandler.BindCamera(camera.get());
}

int CHitTestView::OnCreate()
{
    m_renderEngine = CreateRenderEngine(m_RenderPal);
    mOverlayManager = NBRE_NEW NBRE_DefaultOverlayManager<int32>(m_renderEngine->Context(), &mMaxOverlayId);
    CreateScene(m_renderEngine->Context());

    return 0;
}

void CHitTestView::OnSize(UINT nType, int cx, int cy)
{
    m_ScreenHeight = cx;
    m_ScreenWidth = cy;

    NBRE_RenderSurface* surface = m_renderEngine->ActiveRenderSurface();
    NBRE_SurfaceSubView& subView = surface->GetSubView(0);

    subView.Viewport().Update(0, 0, cx, cy);

    NBRE_PerspectiveConfig perspective;
    perspective.mAspect = float(cx)/cy;
    perspective.mFov = 45;
    subView.Viewport().Camera()->GetFrustum().SetAsPerspective(perspective, 1, 100);
}

void CHitTestView::OnRender()
{
    GetWindow()->OnRenderBegine();
    m_renderEngine->Render();
    GetWindow()->OnRenderEnd();
}



void CHitTestView::OnTimer(UINT_PTR nIDEvent)
{
    clock_t c = clock();

    if(m_animation)
    {
        m_node1->SetOrientation(NBRE_Orientation(NBRE_Vector3f(0,0,1), static_cast<float>((c/10)%360)));

        m_node2->SetOrientation(NBRE_Orientation(NBRE_Vector3f(0,0,1), static_cast<float>((c/10)%360)));
    }
}

void CHitTestView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    //NBRE_Camera* camera = m_curCamera;

    m_CameraHandler.OnKeyEvent(nChar);

    switch(nChar)
    {
    case(' '):   
        {
            m_animation = !m_animation;
        }
        break;

    //case('1'):   
    //    {
    //        m_curCamera = m_camera.Get();
    //    }
    //    break;

    //case('2'):   
    //    {
    //        NBRE_RenderSurface* surface = m_renderEngine->DefaultRenderSurface();
    //        NBRE_SurfaceSubView& subView = surface->GetSubView(0);
    //        m_curCamera = subView.Viewport().Camera().Get();
    //    }
    //    break;

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
    //case(VK_LEFT):
    //    {
    //        camera->Translate(NBRE_Vector3f(-1, 0, 0));
    //    }
    //    break;
    //case(VK_RIGHT):    
    //    {
    //        camera->Translate(NBRE_Vector3f(1, 0, 0));
    //    }
    //    break;
    //case(VK_UP):    
    //    {
    //        camera->Translate(NBRE_Vector3f(0, 1, 0));
    //    }
    //    break;
    //case(VK_DOWN):    
    //    {
    //        camera->Translate(NBRE_Vector3f(0, -1, 0));
    //    }
    //    break;

    default:
        break;
    }


}

void TransformPoint(NBRE_Frustum& frustum, NBRE_AxisAlignedBox2i& rect, NBRE_Point3f& pt) 
{
    const NBRE_PerspectiveConfig& pers = frustum.GetPerspectiveConfig();
    double top = nsl_tan(pers.mFov * 0.5 * NBRE_Math::Pi64 / 180.0) * frustum.GetZNear();
    double right = top * pers.mAspect;
    pt.x = static_cast<float>(2*pt.x*right/rect.GetSize().x);
    pt.y = static_cast<float>(2*pt.y*top/rect.GetSize().y);
    pt.z = -frustum.GetZNear();
}

void CHitTestView::OnLButtonDown(UINT nFlags, CPoint point)
{

    NBRE_Ray3f ray = NBRE_TypeConvertf::Convert(m_subview->Viewport().GetRay(static_cast<float>(point.x), static_cast<float>(point.y)));
    m_debugRay->SetLine(ray.origin, ray.origin + ray.direction*20);

    //NBRE_Frustum3f frustum(camera->GetFrustum().ExtractMatrix()*camera->ExtractMatrix());
    //m_debugFrustum->SetFrustum(frustum);

    NBRE_CameraPtr camera = m_subview->Viewport().Camera();

    m_sceneMgr->PickupObject(*camera, ray, FALSE);


    //{
    //    NBRE_Planef plane(NBRE_Vector3f(0, 0, 1), 0);

    //    NBRE_Vector3f pos;
    //    NBRE_Intersectionf::HitTest(plane, ray, &pos);

    //    int32 poiIndex = rand() % 3;

    //    // Every billboard has its own size, shader, color and texture coordinates.
    //    // The shader must have a texture unit.
    //    NBRE_BillboardNodePtr poi = mBillboardSet->AddBillboard();
    //    poi->AddEntity(NBRE_BillboardEntityPtr(NBRE_NEW NBRE_ImageEntity(NBRE_Vector2f(0.5f, 0.5f)
    //        , NBRE_ShaderManager::GetSingleton().GetShader(poiTextures[poiIndex])
    //        , NBRE_Color(1.0f, 1.0f, 1.0f, 1.0f))));

    //    poi->SetPosition(pos);
    //    m_sceneMgr->RootSceneNode()->AddChild(poi);
    //}
}

void CHitTestView::OnPickedUp(NBRE_Entity* ent) 
{
    ent->SetVisible(!ent->Visible());
}
