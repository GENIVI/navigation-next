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
#include "nbgmglowoverlay.h"
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

static const uint32 FrameBufferSize = 512;

NBGM_GlowLayer::NBGM_GlowLayer(NBGM_Context& nbgmContext, const DrawOrderStruct& buildingDrawOrder, const DrawOrderStruct& transparentOverlayDrawOrder, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager):
        mNBGMContext(nbgmContext),
        mBuildingDrawOrder(buildingDrawOrder),
        mTransparentOverlayDrawOrder(transparentOverlayDrawOrder),
        mB3dDrawOrder(0),
        mTransparentSurface(NULL),
        mTransparentSubView(NULL),
        mOverlayManager(overlayManager),
        mTransparentOverlay(NULL),
        mShardVertex(NULL),
        mIndexData(NULL),
        mAlpha(0.9f),
        mCommonPass(NULL),
        mSubView(NULL),
        mIsDrawOrderInitialized(FALSE),
        mCommonTarget(NULL),
        mEnabled(TRUE),
        mClearColor(1.0f, 1.0f, 1.0f, 0.0f),
        mScaleColor(1.0f, 1.0f, 1.0f, 1.0f),
        mRadius(4.0f)
{
}

NBGM_GlowLayer::~NBGM_GlowLayer()
{
    NBRE_DELETE mCommonPass;
    NBRE_DELETE mShardVertex;
    NBRE_DELETE mIndexData;
    NBRE_DELETE mTransparentOverlayManager;
}

void NBGM_GlowLayer::CreateRectVertex()
{
    NBRE_IRenderPal& renderPal = *(mNBGMContext.renderingEngine->Context().mRenderPal);

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
    static NBRE_Color glowBaseColor(1.0f, 1.0f, 1.0f, 1.0f);
    static const float color[] =
    {
         glowBaseColor.r, glowBaseColor.g, glowBaseColor.b, glowBaseColor.a,
         glowBaseColor.r, glowBaseColor.g, glowBaseColor.b, glowBaseColor.a,
         glowBaseColor.r, glowBaseColor.g, glowBaseColor.b, glowBaseColor.a,
         glowBaseColor.r, glowBaseColor.g, glowBaseColor.b, glowBaseColor.a,
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

void NBGM_GlowLayer::Initialize(const NBRE_SceneManagerPtr& sceneManager, const NBRE_CameraPtr& camera, NBRE_SurfaceSubView* subView, NBGM_MapViewProfiler& profiler, nb_boolean drawSky)
{
    mSubView = subView;

    CreateRectVertex();

    NBRE_IRenderPal* renderPal = mNBGMContext.renderingEngine->Context().mRenderPal;

    // Create pass using common road
    NBRE_ImageInfo imageInfo(FrameBufferSize, FrameBufferSize, NBRE_PF_R8G8B8A8);
    NBRE_TexturePtr texture = NBGM_BuildTextureUtility::ConstructCommonTexture(*renderPal, "NBGM_GlowTexture", imageInfo, NBRE_Texture::TT_2D_RENDERTARGET, FALSE);

    mCommonPass = NBRE_NEW NBRE_Pass();
    mCommonPass->SetEnableLighting(FALSE);
    mCommonPass->SetEnableDepthTest(FALSE);
    mCommonPass->SetEnableDepthWrite(FALSE);
    mCommonPass->SetEnableCullFace(FALSE);
    mCommonPass->SetEnableBlend(TRUE);
    mCommonPass->SetSrcBlend(NBRE_BF_SRC_ALPHA);
    mCommonPass->SetDstBlend(NBRE_BF_ONE);
    mCommonPass->SetColor(NBRE_Color(1.0f, 1.0f, 1.0f, 1.0f));

    NBRE_TextureUnit unit0;
    unit0.SetTexture(texture);
    mCommonPass->GetTextureUnits().push_back(unit0);

    subView->AddVisitable(this);

    NBRE_SurfaceManager* surfaceManager = mNBGMContext.renderingEngine->Context().mSurfaceManager;
    mTransparentSurface = surfaceManager->CreateSurface();

    mCommonTarget = surfaceManager->GetRenderTargetFromTexture(texture.get(), 0, 0, TRUE);

    mEmptyTexture = CreateEmptyTexture(*renderPal);
    NBRE_TextureUnit unit1;
    unit1.SetTexture(mEmptyTexture);
    mCommonPass->GetTextureUnits().push_back(unit1);
    mCommonPass->SetGpuProgram("glow");
    mCommonPass->SetGpuProgramParam("u_texSampler_1", 0);
    mCommonPass->SetGpuProgramParam("u_texSampler_2", 1);
    mCommonPass->SetGpuProgramParam("pixelSize", NBRE_GpuProgramParam(1.0f/FrameBufferSize, 1.0f/FrameBufferSize));
    mCommonPass->SetGpuProgramParam("scaleColor", NBRE_GpuProgramParam(mScaleColor.r, mScaleColor.g, mScaleColor.b, mScaleColor.a));
    mCommonPass->SetGpuProgramParam("radius", NBRE_GpuProgramParam(mRadius));
    SetBlurSamples(4);

    NBRE_Matrix4x4f mat = NBRE_Transformationf::BuildScaleMatrix(1.0f, 1.0f, 1.0f);
    float buf[16] = {0};
    mat.CopyTo(buf, FALSE);
    mCommonPass->SetGpuProgramParam("u_texMatrix0", NBRE_GpuProgramParam(buf));

    mTransparentSurface->AttachTarget(mCommonTarget);

    mTransparentOverlayManager = NBRE_NEW NBRE_DefaultOverlayManager<DrawOrderStruct>(mNBGMContext.renderingEngine->Context(), &mNBGMContext.maxOverlayId);

    mTransparentSubView = &mTransparentSurface->CreateSubView(0, 0, 0, 0);
    mTransparentSubView->Viewport().SetCamera(camera);
    mTransparentSubView->SetSceneManager(sceneManager);
    mTransparentSubView->SetOverlayManager(mTransparentOverlayManager);

    mBuildingOverlayId = mTransparentOverlayManager->AssignOverlayId(mBuildingDrawOrder);
    NBRE_Overlay* overlay = mTransparentOverlayManager->FindOverlay(mBuildingOverlayId);
    overlay->SetClearColor(mClearColor);
    overlay->SetClearFlag(NBRE_CLF_CLEAR_DEPTH | NBRE_CLF_CLEAR_COLOR);

    int32 overlayId = 0;
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

    //NBRE_RenderQueue* queue = mTransparentOverlayManager->FindOverlay(mTransparentOverlayManager->FindId(mBuildingDrawOrder));
    //profiler.AddRenderQueue(TID_B3D, *queue);
}

void NBGM_GlowLayer::OnSizeChanged(int32 x, int32 y, uint32 w, uint32 h)
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
    mTransparentSubView->Viewport().Update(x, y, w, h);
}

void NBGM_GlowLayer::Accept(NBRE_Visitor& visitor)
{
    visitor.Visit(static_cast<NBRE_IRenderOperationProvider*>(this), mNBGMContext.renderingEngine->Context().mSurfaceManager);
}

void NBGM_GlowLayer::NotifyCamera(const NBRE_Camera& /*camera*/)
{
}

void NBGM_GlowLayer::UpdateRenderQueue(NBRE_Overlay& overlay)
{
    if(mShardVertex == NULL || mIndexData == NULL || !mEnabled)
        return;

    NBRE_RenderOperation& renderOp = overlay.CreateRenderOperation();
    renderOp.SetVertexData(mShardVertex);
    renderOp.SetIndexData(mIndexData);
    renderOp.SetOperationType(NBRE_PMT_TRIANGLE_STRIP);
    renderOp.SetPass(mCommonPass);
    renderOp.SetTransform(&mVertexTransform);
}

int32 NBGM_GlowLayer::OverlayId() const
{
    return mOverlayManager.FindId(mTransparentOverlayDrawOrder);
}

void NBGM_GlowLayer::SetDrawOrder(uint8 value)
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
    mIsDrawOrderInitialized = TRUE;
}

void NBGM_GlowLayer::SetBuildingDrawOrder(uint8 value)
{
    mB3dDrawOrder = value;
    SetDrawOrder(value);
}

nb_boolean NBGM_GlowLayer::Visible() const
{
    return TRUE;
}

void NBGM_GlowLayer::UpdateSurfaceList()
{
    mNBGMContext.renderingEngine->Context().mSurfaceManager->UpdateSurfaceList(mTransparentSurface);
}

int32 NBGM_GlowLayer::BuildingOverlayId()
{
    return mBuildingOverlayId;
}

void NBGM_GlowLayer::SetAlpha(float alpha)
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

inline float CalculateBlurWeight(float t)
{
    if (t < 0.0f)
    {
        t = -t;
    }
    
    return t > 1.0f ? 0.0f : (float)nsl_cos(t * NBRE_Math::PiHalf64);
}

static void GenerateBlurSamples(NBRE_Vector<float>& samples, int n)
{
    float total = 0.0f;
    float r = n * 0.5f;
    for (int j = 0; j < n; ++j)
    {
        float y = (j + 0.5f - r) / r;//-1 to 1
        for (int i = 0; i < n; ++i)
        {
            float x = (i + 0.5f - r) / r;//-1 to 1
            int index = (j * n + i) * 3;
            samples[index] = x;
            samples[index + 1] = y;
            float w = CalculateBlurWeight(NBRE_Vector2f(x, y).Length());
            samples[index + 2] = w;
            total += w;
        }
    }
    
    // normalize weights
    float invTotal = 1.0f / total;
    int nn = n * n;
    for (int i = 0; i < nn; ++i)
    {
        samples[i * 3 + 2] *= invTotal;
    }
}

void NBGM_GlowLayer::SetBlurSamples(int s)
{
    const int MAX_SAMPLES = 4;
    NBRE_Vector<float>* samples = new NBRE_Vector<float>();
    samples->resize(MAX_SAMPLES * MAX_SAMPLES * 3);
    if (s > MAX_SAMPLES)
    {
        s = MAX_SAMPLES;
    }
    
    GenerateBlurSamples(*samples, s);

    shared_ptr<NBRE_Vector<float> > arr(samples);
    NBRE_GpuProgramParam param(arr, MAX_SAMPLES * MAX_SAMPLES);
    mCommonPass->SetGpuProgramParam("samples", param);
    mCommonPass->SetGpuProgramParam("numSamples", NBRE_GpuProgramParam(s * s));
}

void NBGM_GlowLayer::SetGlowParameters(const NBGM_GlowParameters& parameters)
{
    mEnabled = parameters.enable;

    mClearColor.r = parameters.clearColor[0];
    mClearColor.g = parameters.clearColor[1];
    mClearColor.b = parameters.clearColor[2];
    mClearColor.a = 0.0f;

    mScaleColor.r = parameters.scaleColor[0];
    mScaleColor.g = parameters.scaleColor[1];
    mScaleColor.b = parameters.scaleColor[2];
    mScaleColor.a = parameters.scaleColor[3];

    mCommonPass->SetGpuProgramParam("scaleColor", NBRE_GpuProgramParam(mScaleColor.r, mScaleColor.g, mScaleColor.b, mScaleColor.a));
    mCommonPass->SetGpuProgramParam("radius", NBRE_GpuProgramParam(parameters.blurRadius));
    SetBlurSamples(parameters.blurSamples);

    NBRE_Overlay* overlay = mTransparentOverlayManager->FindOverlay(mBuildingOverlayId);
    overlay->SetClearColor(mClearColor);
}
