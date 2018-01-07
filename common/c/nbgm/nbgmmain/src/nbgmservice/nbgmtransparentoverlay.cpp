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

/*--------------------------------------------------------------------------

(C) Copyright 2012 by Networks In Motion, Inc.

The information contained herein is confidential, proprietary
to Networks In Motion, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of Networks In Motion is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#include "nbgmtransparentoverlay.h"
#include "nbrerendersurface.h"
#include "nbgmcontext.h"
#include "nbrerenderengine.h"
#include "nbremesh.h"
#include "nbreentity.h"
#include "nbretransformation.h"
#include "nbgmmapviewprofiler.h"
#include "nbgmsky.h"
#include "nbgmbuildutility.h"
#include "nbgmconst.h"
#include "nbretransformutil.h"

static const uint32 FrameBufferSize = 1024;
static const float GroundSize = 100000.f;

static const NBRE_Color CLEAR_COLOR = NBRE_Color(0.61f, 0.61f, 0.61f, 0.0f);

NBGM_TransparentLayer::NBGM_TransparentLayer(NBGM_Context& nbgmContext, const DrawOrderStruct& buildingDrawOrder, const DrawOrderStruct& roadBgDrawOrder, const DrawOrderStruct& transparentOverlayDrawOrder, const DrawOrderStruct& transparentWallDrawOrder, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager):
        mNBGMContext(nbgmContext),
        mBuildingDrawOrder(buildingDrawOrder),
        mRoadBgDrawOrder(roadBgDrawOrder),
        mTransparentOverlayDrawOrder(transparentOverlayDrawOrder),
        mTransparentWallOrder(transparentWallDrawOrder),
        mB3dDrawOrder(0),
        mRoadDrawOrder(0),
        mMapState(MT_CommonMap),
        mTransparentSurface(NULL),
        mTransparentSubView(NULL),
        mOverlayManager(overlayManager),
        mTransparentOverlay(NULL),
        mShardVertex(NULL),
        mIndexData(NULL),
        mAlpha(B3D_TRANSPARENT_ALPHA),
        mCommonPass(NULL),
        mSatelliteRoadPass(NULL),
        mAoPass(NULL),
        mSky(NULL),
        mSubView(NULL),
        mIsDrawOrderInitialized(FALSE),
        mRestoreDepthSky(NULL),
        mAOTarget(NULL),
        mCommonTarget(NULL),
        mSatelliteRoadTarget(NULL)
{
}

NBGM_TransparentLayer::~NBGM_TransparentLayer()
{
    NBRE_DELETE mRestoreDepthSky;
    NBRE_DELETE mSky;
    NBRE_DELETE mCommonPass;
    NBRE_DELETE mSatelliteRoadPass;
    NBRE_DELETE mShardVertex;
    NBRE_DELETE mIndexData;
    NBRE_DELETE mTransparentOverlayManager;
}

void NBGM_TransparentLayer::InitGround(const NBRE_SceneManagerPtr& sceneManager)
{
    NBRE_IRenderPal& renderPal = *(mNBGMContext.renderingEngine->Context().mRenderPal);
    if (!renderPal.GetCapabilities().GetSupportHBAO())
    {
        return;
    }

    static const float vertices[] =
    {
        -1, -1,
         1, -1,
        -1,  1,
         1,  1
    };

    static const float color[] =
    {
       1, 1, 1, 0,
       1, 1, 1, 0,
       1, 1, 1, 0,
       1, 1, 1, 0,
    };

    NBRE_HardwareVertexBuffer* vertextBuff = renderPal.CreateVertexBuffer(sizeof(float) * 2, 4, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    vertextBuff->WriteData(0, sizeof(float) * 2 * 4, vertices, FALSE);
    NBRE_HardwareVertexBuffer* colorBuff = renderPal.CreateVertexBuffer(sizeof(float) * 4, 4, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    colorBuff->WriteData(0, sizeof(float) * 4 * 4, color, FALSE);

    NBRE_VertexDeclaration* decalration = renderPal.CreateVertexDeclaration();
    NBRE_VertexElement* posElem1= NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_POSITION, 0);
    NBRE_VertexElement* posElem2= NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT4, NBRE_VertexElement::VES_DIFFUSE, 0);
    decalration->GetVertexElementList().push_back(posElem1);
    decalration->GetVertexElementList().push_back(posElem2);

    NBRE_VertexData *vertexData = NBRE_NEW NBRE_VertexData(0);
    vertexData->AddBuffer(vertextBuff);
    vertexData->AddBuffer(colorBuff);
    vertexData->SetVertexDeclaration(decalration);

    NBRE_IndexData* indexData = NBRE_NEW NBRE_IndexData(NULL, 0, 4);

    NBRE_MeshPtr mesh(NBRE_NEW NBRE_Mesh(vertexData));
    mesh->CreateSubMesh(indexData, NBRE_PMT_TRIANGLE_STRIP);
    mesh->CalculateBoundingBox();
    NBRE_ModelPtr model(NBRE_NEW NBRE_Model(mesh));

    NBRE_PassPtr pass = NBRE_PassPtr(NBRE_NEW NBRE_Pass());
    pass->SetEnableLighting(FALSE);
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(TRUE);
    pass->SetEnableCullFace(FALSE);
    pass->SetEnableBlend(TRUE);
    pass->SetSrcBlend(NBRE_BF_SRC_ALPHA);
    pass->SetDstBlend(NBRE_BF_ONE_MINUS_SRC_ALPHA);
    pass->SetColor(NBRE_Color(1.0f, 1.0f, 1.0f, 1.0f));
    NBRE_ShaderPtr shader(NBRE_NEW NBRE_Shader());
    shader->AddPass(pass);

    NBRE_EntityPtr entity(NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), model, mBuildingOverlayId));
    entity->SetShader(shader);
    entity->SetVisible(TRUE);

    mGroundNode = NBRE_NodePtr(NBRE_NEW NBRE_Node());
    mGroundNode->AttachObject(entity);
    float scale = METER_TO_MERCATOR(mNBGMContext.WorldToModel(GroundSize));
    mGroundNode->SetScale(NBRE_Vector3f(scale, scale, scale));
    sceneManager->RootSceneNode()->AddChild(mGroundNode);
}


void NBGM_TransparentLayer::CreateRectVertex()
{
    NBRE_IRenderPal& renderPal = *(mNBGMContext.renderingEngine->Context().mRenderPal);
    if (renderPal.GetCapabilities().GetSupportHBAO())
    {
        mAlpha = 0.9f;
    }

    static const float vertices[] =
    {
        -1, -1,
         1, -1,
        -1,  1,
         1,  1,
    };

    static const float texcord[] =
    {
         0,  0,
         1,  0,
         0,  1,
         1,  1,
    };

    static const float color[] =
    {
         1.0f, 1.0f, 1.0f, mAlpha,
         1.0f, 1.0f, 1.0f, mAlpha,
         1.0f, 1.0f, 1.0f, mAlpha,
         1.0f, 1.0f, 1.0f, mAlpha,
    };

    

    NBRE_HardwareVertexBuffer* vertextBuff = renderPal.CreateVertexBuffer(sizeof(float) * 2, 4, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    vertextBuff->WriteData(0, sizeof(float) * 2 * 4, vertices, FALSE);

    NBRE_HardwareVertexBuffer* texBuff = renderPal.CreateVertexBuffer(sizeof(float) * 2, 4, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    texBuff->WriteData(0, sizeof(float) * 2 * 4, texcord, FALSE);

    NBRE_HardwareVertexBuffer* colorBuff = renderPal.CreateVertexBuffer(sizeof(float) * 4, 4, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    colorBuff->WriteData(0, sizeof(float) * 4 * 4, color, FALSE);


    NBRE_VertexDeclaration* decalration = renderPal.CreateVertexDeclaration();
    NBRE_VertexElement* posElem1= NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_POSITION, 0);
    NBRE_VertexElement* posElem2= NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
    NBRE_VertexElement* posElem3= NBRE_NEW NBRE_VertexElement(2, 0, NBRE_VertexElement::VET_FLOAT4, NBRE_VertexElement::VES_DIFFUSE, 0);

    decalration->GetVertexElementList().push_back(posElem1);
    decalration->GetVertexElementList().push_back(posElem2);
    decalration->GetVertexElementList().push_back(posElem3);

    mShardVertex = NBRE_NEW NBRE_VertexData(0);
    mShardVertex->AddBuffer(vertextBuff);
    mShardVertex->AddBuffer(texBuff);
    mShardVertex->AddBuffer(colorBuff);

    mShardVertex->SetVertexDeclaration(decalration);

    mIndexData = NBRE_NEW NBRE_IndexData(NULL, 0, 4);
}

void NBGM_TransparentLayer::SetWallSize(float width, float height, float dsitance)
{
    if(mSky != NULL)
    {
        mSky->SetWallSize(width, height, dsitance);
    }

    if(mRestoreDepthSky)
    {
        mRestoreDepthSky->SetWallSize(width, height, dsitance);
    }
}

static NBRE_TexturePtr CreateEmptyTexture(NBRE_IRenderPal &renderPal)
{
    NBRE_Image **images = NBRE_NEW NBRE_Image*[1];

    NBRE_Image *image = NBRE_NEW NBRE_Image(1, 1, NBRE_PF_R8G8B8A8);
    uint8 *buffer = image->GetImageData();
    buffer[0] = 255;
    buffer[1] = 255;
    buffer[2] = 255;
    buffer[3] = 255;

    images[0] = image;
    NBRE_TexturePtr texture(renderPal.CreateTexture(images, 1, 1, FALSE, NBRE_Texture::TT_2D, "NBGM_TransparentTextureAOEmpty"));
    return texture;
}

void NBGM_TransparentLayer::Initialize(const NBRE_SceneManagerPtr& sceneManager, const NBRE_CameraPtr& camera, NBRE_SurfaceSubView* subView, NBGM_MapViewProfiler& profiler, nb_boolean drawSky)
{
    mSubView = subView;

    CreateRectVertex();

    NBRE_IRenderPal* renderPal = mNBGMContext.renderingEngine->Context().mRenderPal;

    // Create pass using common road
    NBRE_ImageInfo imageInfo(FrameBufferSize, FrameBufferSize, NBRE_PF_R8G8B8A8);
    NBRE_TexturePtr texture = NBGM_BuildTextureUtility::ConstructCommonTexture(*renderPal, "NBGM_TransparentTexture", imageInfo, NBRE_Texture::TT_2D_RENDERTARGET, FALSE);

    mCommonPass = NBRE_NEW NBRE_Pass();
    mCommonPass->SetEnableLighting(FALSE);
    mCommonPass->SetEnableDepthTest(FALSE);
    mCommonPass->SetEnableDepthWrite(FALSE);
    mCommonPass->SetEnableCullFace(FALSE);
    mCommonPass->SetEnableBlend(TRUE);
    mCommonPass->SetSrcBlend(NBRE_BF_SRC_ALPHA);
    mCommonPass->SetDstBlend(NBRE_BF_ONE_MINUS_SRC_ALPHA);
    mCommonPass->SetColor(NBRE_Color(1.0f, 1.0f, 1.0f, 1.0f));

    NBRE_TextureUnit unit0;
    unit0.SetTexture(texture);
    mCommonPass->GetTextureUnits().push_back(unit0);

    subView->AddVisitable(this);

    NBRE_SurfaceManager* surfaceManager = mNBGMContext.renderingEngine->Context().mSurfaceManager;
    mTransparentSurface = surfaceManager->CreateSurface();

    if (renderPal->GetCapabilities().GetSupportHBAO())
    {
        mAoPass = NBRE_NEW NBRE_Pass();
        mAoPass->SetEnableLighting(FALSE);
        mAoPass->SetEnableDepthTest(FALSE);
        mAoPass->SetEnableDepthWrite(FALSE);
        mAoPass->SetEnableCullFace(FALSE);
        mAoPass->SetEnableBlend(TRUE);
        mAoPass->SetSrcBlend(NBRE_BF_SRC_ALPHA);
        mAoPass->SetDstBlend(NBRE_BF_ONE_MINUS_SRC_ALPHA);
        mAoPass->SetColor(NBRE_Color(1.0f, 1.0f, 1.0f, 1.0f));
        NBRE_TextureUnit unitAo0;
        unitAo0.SetTexture(texture);
        mAoPass->GetTextureUnits().push_back(unitAo0);

        // Create pass using for satellite road.
        NBRE_TexturePtr texture1 = NBGM_BuildTextureUtility::ConstructCommonTexture(*renderPal, "NBGM_TransparentTexture", imageInfo, NBRE_Texture::TT_2D_RENDERTARGET, FALSE);

        mSatelliteRoadPass = NBRE_NEW NBRE_Pass();
        mSatelliteRoadPass->SetEnableLighting(FALSE);
        mSatelliteRoadPass->SetEnableDepthTest(FALSE);
        mSatelliteRoadPass->SetEnableDepthWrite(FALSE);
        mSatelliteRoadPass->SetEnableCullFace(FALSE);
        mSatelliteRoadPass->SetEnableBlend(TRUE);
        mSatelliteRoadPass->SetSrcBlend(NBRE_BF_SRC_ALPHA);
        mSatelliteRoadPass->SetDstBlend(NBRE_BF_ONE_MINUS_SRC_ALPHA);
        mSatelliteRoadPass->SetColor(NBRE_Color(1.0f, 1.0f, 1.0f, 1.0f));

        NBRE_TextureUnit unit0;
        unit0.SetTexture(texture1);
        mSatelliteRoadPass->GetTextureUnits().push_back(unit0);

        mCommonTarget = surfaceManager->GetRenderTargetFromTexture(texture.get(), 0, 0, TRUE);
        mSatelliteRoadTarget = surfaceManager->GetRenderTargetFromTexture(texture1.get(), 0, 0, FALSE);

        imageInfo.mFormat = NBRE_PF_R8G8B8;
        NBRE_IRenderPal* renderPal = mNBGMContext.renderingEngine->Context().mRenderPal;
        mAOTexture = NBGM_BuildTextureUtility::ConstructCommonTexture(*renderPal, "NBGM_TransparentTextureAO", imageInfo, NBRE_Texture::TT_2D_RENDERTARGET, FALSE);
        mAOTarget = surfaceManager->GetRenderTargetFromTexture(mAOTexture.get(), 0, 0, FALSE);

        mBlurTexture = NBGM_BuildTextureUtility::ConstructCommonTexture(*renderPal, "NBGM_TransparentTextureAOBlur", imageInfo, NBRE_Texture::TT_2D_RENDERTARGET, FALSE);
        mBlurTarget = surfaceManager->GetRenderTargetFromTexture(mBlurTexture.get(), 0, 0, FALSE);

        NBRE_ImageInfo imageInfoDownsample = imageInfo;
        imageInfoDownsample.mWidth /= 2;
        imageInfoDownsample.mHeight /= 2;
        mAoDownsampleTexture = NBGM_BuildTextureUtility::ConstructCommonTexture(*renderPal, "NBGM_TransparentTextureAODownsample", imageInfoDownsample, NBRE_Texture::TT_2D_RENDERTARGET, FALSE);
        mAoDownsampleTarget = surfaceManager->GetRenderTargetFromTexture(mAoDownsampleTexture.get(), 0, 0, FALSE);

        mEmptyTexture = CreateEmptyTexture(*renderPal);

        NBRE_HBAOParams* hp = NBRE_NEW NBRE_HBAOParams();
        hp->mEnable = TRUE;
        hp->mInputTarget = mCommonTarget;
        hp->mOutputTarget = mAOTarget;
        hp->mBlurTarget = mBlurTarget;
        hp->mAoTarget = mAoDownsampleTarget;
        hp->mMetersToViewSpaceUnits = mNBGMContext.WorldToModel(METER_TO_MERCATOR(1.0f));
        hp->mRadius = 40.0f;
        hp->mBias = 0.0f;
        hp->mPowerExponent = 4.0f;
        hp->mEnableBlur = TRUE;
        hp->mBlurSharpness = 1.0f;
        hp->mBlurRadius = 4;
        hp->mCamera = camera.get();
        mAoPass->SetHBAOParams(NBRE_HBAOParamsPtr(hp));
        NBRE_TextureUnit unit1;
        unit1.SetTexture(mAOTexture);
        mAoPass->GetTextureUnits().push_back(unit1);
        mAoPass->SetGpuProgram("ao");
        mAoPass->SetGpuProgramParam("u_texSampler_1", 0);
        mAoPass->SetGpuProgramParam("u_texSampler_2", 1);

        NBRE_Matrix4x4f mat = NBRE_Transformationf::BuildScaleMatrix(1.0f, 1.0f, 1.0f);
        float buf[16] = {0};
        mat.CopyTo(buf, FALSE);
        mAoPass->SetGpuProgramParam("u_texMatrix0", NBRE_GpuProgramParam(buf));
    }
    else
    {
        mCommonTarget = surfaceManager->GetRenderTargetFromTexture(texture.get(), 0, 0, FALSE);
    }
    mTransparentSurface->AttachTarget(mCommonTarget);

    mTransparentOverlayManager = NBRE_NEW NBRE_DefaultOverlayManager<DrawOrderStruct>(mNBGMContext.renderingEngine->Context(), &mNBGMContext.maxOverlayId);

    mTransparentSubView = &mTransparentSurface->CreateSubView(0, 0, 0, 0);
    mTransparentSubView->Viewport().SetCamera(camera);
    mTransparentSubView->SetSceneManager(sceneManager);
    mTransparentSubView->SetOverlayManager(mTransparentOverlayManager);

    mRoadBgOverlayId = mTransparentOverlayManager->AssignOverlayId(mRoadBgDrawOrder);
    NBRE_Overlay* overlay = mTransparentOverlayManager->FindOverlay(mRoadBgOverlayId);
    overlay->SetClearColor(CLEAR_COLOR);
    overlay->SetClearFlag(drawSky?(NBRE_CLF_CLEAR_COLOR):(NBRE_CLF_CLEAR_COLOR|NBRE_CLF_CLEAR_DEPTH));

    mBuildingOverlayId = mTransparentOverlayManager->AssignOverlayId(mBuildingDrawOrder);
    overlay = mTransparentOverlayManager->FindOverlay(mBuildingOverlayId);
    overlay->SetClearColor(CLEAR_COLOR);
    overlay->SetClearFlag(0);

    int32 overlayId = 0;
    if(drawSky)
    {
        overlayId = mTransparentOverlayManager->AssignOverlayId(mTransparentWallOrder);
        overlay = mTransparentOverlayManager->FindOverlay(overlayId);
        overlay->SetClearColor(CLEAR_COLOR);
        overlay->SetClearFlag(NBRE_CLF_CLEAR_COLOR|NBRE_CLF_CLEAR_DEPTH);

        mSky = NBRE_NEW NBGM_Sky(mNBGMContext, overlayId, TRUE, FALSE, mTransparentSubView);
    }

    overlayId = mOverlayManager.AssignOverlayId(mTransparentOverlayDrawOrder);
    mTransparentOverlay = mOverlayManager.FindOverlay(overlayId);
    mTransparentOverlay->SetClearFlag(NBRE_CLF_CLEAR_DEPTH);

    // Update screen overlay
    mScreenCamera = NBRE_CameraPtr(NBRE_NEW NBRE_Camera());
    NBRE_OrthoConfig config;
    config.mLeft = -1;
    config.mRight = 1;
    config.mBottom = -1;
    config.mTop = 1;
    mScreenCamera->GetFrustum().SetAsOrtho(config, -1, 1);
    mTransparentOverlay->SetCamera(mScreenCamera);

    NBRE_RenderQueue* queue = mTransparentOverlayManager->FindOverlay(mTransparentOverlayManager->FindId(mBuildingDrawOrder));
    profiler.AddRenderQueue(TID_B3D, *queue);
    InitGround(sceneManager);
}

void NBGM_TransparentLayer::OnSizeChanged(int32 x, int32 y, uint32 w, uint32 h)
{
    if(w > FrameBufferSize)
    {
        w = FrameBufferSize;
    }

    if(h > FrameBufferSize)
    {
        h = FrameBufferSize;
    }

    float wScale = static_cast<float>(w)/FrameBufferSize;
    float hScale = static_cast<float>(h)/FrameBufferSize;

    mCommonPass->GetTextureUnits()[0].SetTransform(NBRE_Transformationf::BuildScaleMatrix(wScale, hScale, 1.0f));
    if (mCommonPass->GetTextureUnits().size() > 1)
    {
        NBRE_Matrix4x4f mat = NBRE_Transformationf::BuildScaleMatrix(wScale, hScale, 1.0f);
        float buf[16] = {0};
        mat.CopyTo(buf, FALSE);
        mCommonPass->SetGpuProgramParam("u_texMatrix0", NBRE_GpuProgramParam(buf));
    }
    if(mSatelliteRoadPass)
    {
        mSatelliteRoadPass->GetTextureUnits()[0].SetTransform(NBRE_Transformationf::BuildScaleMatrix(wScale, hScale, 1.0f));
    }
    mAoPass->GetTextureUnits()[0].SetTransform(NBRE_Transformationf::BuildScaleMatrix(wScale, hScale, 1.0f));
    if (mAoPass->GetTextureUnits().size() > 1)
    {
        NBRE_Matrix4x4f mat = NBRE_Transformationf::BuildScaleMatrix(wScale, hScale, 1.0f);
        float buf[16] = {0};
        mat.CopyTo(buf, FALSE);
        mAoPass->SetGpuProgramParam("u_texMatrix0", NBRE_GpuProgramParam(buf));
    }
    mTransparentSubView->Viewport().Update(x, y, w, h);
}

void NBGM_TransparentLayer::Accept(NBRE_Visitor& visitor)
{
    visitor.Visit(static_cast<NBRE_IRenderOperationProvider*>(this), mNBGMContext.renderingEngine->Context().mSurfaceManager);
}

void NBGM_TransparentLayer::NotifyCamera(const NBRE_Camera& camera)
{
    if(mGroundNode)
    {
        NBRE_Vector3d eye = mNBGMContext.transUtil->GetEyePosition();
        mGroundNode->SetPosition(NBRE_Vector3d(eye.x, eye.y, mZoomLevel < 16 ? NBRE_Math::Infinity : 0));
    }
}

void NBGM_TransparentLayer::UpdateRenderQueue(NBRE_Overlay& overlay)
{
    if(mShardVertex == NULL || mIndexData == NULL)
        return;

    NBRE_RenderOperation& renderOp = overlay.CreateRenderOperation();

    renderOp.SetVertexData(mShardVertex);
    renderOp.SetIndexData(mIndexData);
    renderOp.SetOperationType(NBRE_PMT_TRIANGLE_STRIP);

    NBRE_IRenderPal* renderPal = mNBGMContext.renderingEngine->Context().mRenderPal;
    if (renderPal->GetCapabilities().GetSupportHBAO())
    {
        if(mMapState == MT_CommonMap)
        {
            renderOp.SetPass(mCommonPass);
        }
        else
        {
            renderOp.SetPass(mSatelliteRoadPass);
        }
    }
    else
    {
        renderOp.SetPass(mCommonPass);
    }

    renderOp.SetTransform(&mVertexTransform);

    {
        NBRE_IRenderPal* renderPal = mNBGMContext.renderingEngine->Context().mRenderPal;
        if (renderPal->GetCapabilities().GetSupportHBAO() && mMapState == MT_CommonMap)
        {
            NBRE_RenderOperation& renderOp = overlay.CreateRenderOperation();

            renderOp.SetVertexData(mShardVertex);
            renderOp.SetIndexData(mIndexData);
            renderOp.SetOperationType(NBRE_PMT_TRIANGLE_STRIP);
            renderOp.SetPass(mAoPass);
            renderOp.SetTransform(&mVertexTransform);
        }

    }
}

int32 NBGM_TransparentLayer::OverlayId() const
{
    return mOverlayManager.FindId(mTransparentOverlayDrawOrder);
}

void NBGM_TransparentLayer::SetDrawOrder(uint8 value)
{
    if (value == mTransparentOverlayDrawOrder.mainDrawOrder)
    {
        return;
    }

    if (mSubView)
    {
        int32 overlayId = mOverlayManager.FindId(mTransparentOverlayDrawOrder);
        mOverlayManager.RemoveOverlay(overlayId);
        mTransparentOverlayDrawOrder.mainDrawOrder = value;
        overlayId = mOverlayManager.AssignOverlayId(mTransparentOverlayDrawOrder);
        mTransparentOverlay = mOverlayManager.FindOverlay(overlayId);
        mTransparentOverlay->SetClearFlag(NBRE_CLF_CLEAR_DEPTH);
        mTransparentOverlay->SetCamera(mScreenCamera);
    }

    // @TODO: This is needed by sky but will clear layer depth
    //        which affect HBAO rendering.
    //        Disabled because sky is no longer used.
    //
    //if (!mRestoreDepthSky)
    //{
    //    DrawOrderStruct skyDrawOrder = mTransparentOverlayDrawOrder;
    //    skyDrawOrder.layer += 1;
    //    int32 overlayId = mOverlayManager.AssignOverlayId(skyDrawOrder);
    //    mOverlayManager.FindOverlay(overlayId)->SetClearFlag(NBRE_CLF_CLEAR_DEPTH);
    //    mRestoreDepthSky = NBRE_NEW NBGM_Sky(mNBGMContext, overlayId, TRUE, FALSE, mSubView);
    //}
    mIsDrawOrderInitialized = TRUE;
}

void NBGM_TransparentLayer::SetSatelliteMode(nb_boolean isSatellite)
{
    if (isSatellite)
    {
        mMapState = MT_SatelliteMap;
        SetAlpha(SATELLITE_TRANSPARENT_ALPHA);
        if (mRoadDrawOrder != 0)
        {
            SetDrawOrder(mRoadDrawOrder);
        }
    }
    else
    {
        mMapState = MT_CommonMap;
        SetAlpha(B3D_TRANSPARENT_ALPHA);
        if (mB3dDrawOrder != 0)
        {
            SetDrawOrder(mB3dDrawOrder);
        }
    }
}

void
NBGM_TransparentLayer::SwitchRenderTarget()
{
    NBRE_IRenderPal* renderPal = mNBGMContext.renderingEngine->Context().mRenderPal;
    if (renderPal->GetCapabilities().GetSupportHBAO())
    {
        if(mMapState == MT_SatelliteMap)
        {
            mTransparentSurface->SwitchTarget(mSatelliteRoadTarget);
        }
        else
        {
            mTransparentSurface->SwitchTarget(mCommonTarget);
        }
    }
}

void NBGM_TransparentLayer::SetBuildingDrawOrder(uint8 value)
{
    mB3dDrawOrder = value;
    if (mMapState == MT_CommonMap)
    {
        SetDrawOrder(value);
    }
}


void NBGM_TransparentLayer::SetRoadDrawOrder(uint8 value)
{
    mRoadDrawOrder = value;
    if (mMapState == MT_SatelliteMap)
    {
        SetDrawOrder(value);
    }
}

uint8 NBGM_TransparentLayer::GetBuildingDrawOrder()
{
    return mB3dDrawOrder;
}

uint8 NBGM_TransparentLayer::GetRoadDrawOrder()
{
    return mRoadDrawOrder;
}

nb_boolean NBGM_TransparentLayer::Visible() const
{
    return TRUE;
}

void NBGM_TransparentLayer::UpdateSurfaceList()
{
    mNBGMContext.renderingEngine->Context().mSurfaceManager->UpdateSurfaceList(mTransparentSurface);
}

int32 NBGM_TransparentLayer::BuildingOverlayId()
{
    return mBuildingOverlayId;
}

int32 NBGM_TransparentLayer::RoadBackgroundOverlayId()
{
    return mRoadBgOverlayId;
}

void NBGM_TransparentLayer::SetAlpha(float alpha)
{
    if (mAlpha == alpha)
    {
        return;
    }

    mAlpha = alpha;
    NBRE_VertexDeclaration::VertexElementList& declList = mShardVertex->GetVertexDeclaration()->GetVertexElementList();
    for (NBRE_VertexDeclaration::VertexElementList::iterator declIt = declList.begin(); declIt != declList.end(); ++declIt)
    {
        NBRE_VertexElement* declElem = *declIt;
        if (declElem->Semantic() == NBRE_VertexElement::VES_DIFFUSE)
        {
            NBRE_HardwareVertexBuffer* vertexBuffer = mShardVertex->GetBuffer(declElem->Source());
            float* pVertex = (float*)vertexBuffer->Lock(0, NBRE_HardwareBuffer::HBL_WRITE_ONLY);
            uint32 vertexCount = vertexBuffer->GetNumVertices();
            for (uint32 vIdx = 0; vIdx < vertexCount; ++vIdx)
            {
                float* pV = pVertex + vIdx * 4;
                pV[3] = mAlpha;
            }
            vertexBuffer->UnLock();
        }
    }
}

void NBGM_TransparentLayer::SetHBAOParameters(const NBGM_HBAOParameters& parameters)
{
    if (mNBGMContext.renderingEngine->Context().mRenderPal->GetCapabilities().GetSupportHBAO())
    {
        NBRE_HBAOParamsPtr hp = mAoPass->GetHBAOParams();
        hp->mEnable = parameters.enable;
        hp->mRadius = parameters.radius;
        hp->mBias = parameters.bias;
        hp->mPowerExponent = parameters.powerExponent;
        hp->mDetailAO = parameters.detailAO;
        hp->mCoarseAO = parameters.coarseAO;
        hp->mEnableBlur = parameters.enableBlur;
        hp->mBlurSharpness = parameters.blurSharpness;
        hp->mBlurRadius = parameters.blurRadius;

        mAoPass->GetTextureUnits()[1].SetTexture(parameters.enable ? mAOTexture : mEmptyTexture);
    }
}

void NBGM_TransparentLayer::SetZoomLevel(int8 zoomLevel)
{
    mZoomLevel = zoomLevel;
}
