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
#include "nbgmanimation.h"
#include "nbrerendersurface.h"
#include "nbgmcontext.h"
#include "nbrerenderengine.h"
#include "nbremesh.h"
#include "nbreentity.h"
#include "nbretransformation.h"
#include "nbgmmapviewprofiler.h"
#include "nbrelinearspatialdividemanager.h"
#include "nbgmprotected.h"
#include "nbrepngtextureimage.h"
#include "nbgmbuildutility.h"

static const uint32 FrameBufferSize = 1024;

NBGM_Animation::NBGM_Animation(NBRE_RenderEngine* renderEngine):
        mRenderEngine(renderEngine),
        mSurface(NULL),
        mSubView(NULL),
        mSceneManager(NULL),
        mMapSurface(NULL),
        mShardVertex(NULL),
        mIndexData(NULL),
        mPass1(NULL),
        mPass2(NULL)
{
}

NBGM_Animation::~NBGM_Animation()
{
    NBRE_DELETE mPass1;
    NBRE_DELETE mPass2;
    NBRE_DELETE mShardVertex;
    NBRE_DELETE mIndexData;
    NBRE_DELETE mOverlayManager;
}

void NBGM_Animation::CreateRectVertex()
{
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

    NBRE_IRenderPal& renderPal = *(mRenderEngine->Context().mRenderPal);

    NBRE_HardwareVertexBuffer* vertextBuff = renderPal.CreateVertexBuffer(sizeof(float) * 2, 4, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    vertextBuff->WriteData(0, sizeof(float) * 2 * 4, vertices, FALSE);

    NBRE_HardwareVertexBuffer* texBuff = renderPal.CreateVertexBuffer(sizeof(float) * 2, 4, FALSE, NBRE_HardwareBuffer::HBU_STATIC);
    texBuff->WriteData(0, sizeof(float) * 2 * 4, texcord, FALSE);

    NBRE_VertexDeclaration* decalration = renderPal.CreateVertexDeclaration();
    NBRE_VertexElement* posElem1= NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_POSITION, 0);
    NBRE_VertexElement* posElem2= NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);

    decalration->GetVertexElementList().push_back(posElem1);
    decalration->GetVertexElementList().push_back(posElem2);

    mShardVertex = NBRE_NEW NBRE_VertexData(0);
    mShardVertex->AddBuffer(vertextBuff);
    mShardVertex->AddBuffer(texBuff);

    mShardVertex->SetVertexDeclaration(decalration);

    mIndexData = NBRE_NEW NBRE_IndexData(NULL, 0, 4);
}

#define SKY_TEXTURE_NORTH    "TEXTURE/SKY"

static NBRE_Pass* CreatePass(NBRE_Context& context)
{
    NBRE_ImageInfo imageInfo(FrameBufferSize, FrameBufferSize, NBRE_PF_R8G8B8A8);
    NBRE_TexturePtr texture = NBGM_BuildTextureUtility::ConstructCommonTexture(*context.mRenderPal, "NBGM_AnimationTexture", imageInfo, NBRE_Texture::TT_2D_RENDERTARGET, FALSE);

    NBRE_Pass* pass = NBRE_NEW NBRE_Pass();
    pass->SetEnableLighting(FALSE);
    pass->SetEnableDepthTest(FALSE);
    pass->SetEnableDepthWrite(FALSE);
    pass->SetEnableCullFace(FALSE);
    pass->SetEnableBlend(FALSE);
    pass->SetColor(NBRE_Color(1.0f, 1.0f, 1.0f, 1.0f));

    NBRE_TextureUnit unit0;
    unit0.SetTexture(texture);
    pass->GetTextureUnits().push_back(unit0);

    return pass;
}

void NBGM_Animation::Initialize()
{
    CreateRectVertex();

    mPass1 = CreatePass(mRenderEngine->Context());
    mPass2 = CreatePass(mRenderEngine->Context());

    mSurface = mRenderEngine->Context().mSurfaceManager->CreateSurface();

    // Update screen overlay
    NBRE_CameraPtr screenCamera(NBRE_NEW NBRE_Camera());
    NBRE_OrthoConfig config;
    config.mLeft = -1;
    config.mRight = 1;
    config.mBottom = -1;
    config.mTop = 1;
    screenCamera->GetFrustum().SetAsOrtho(config, -1, 1);

    mSubView = &mSurface->CreateSubView(0, 0, 0, 0);
    mSubView->Viewport().SetCamera(screenCamera);

    mOverlayManager = NBRE_NEW NBRE_DefaultOverlayManager<int32>(mRenderEngine->Context(), &mMaxOverlayId);
    int32 overlayId = mOverlayManager->AssignOverlayId(0);
    NBRE_Overlay* overlay = mOverlayManager->FindOverlay(overlayId);
    overlay->SetClearColor(NBRE_Color(0.0f, 0.0f, 0.0f, 0.0f));
    overlay->SetClearFlag(NBRE_CLF_CLEAR_COLOR|NBRE_CLF_CLEAR_DEPTH);

    mSubView->AddVisitable(this);
}

void NBGM_Animation::OnSizeChanged(int32 x, int32 y, uint32 w, uint32 h)
{
    if(w > FrameBufferSize)
    {
        w = FrameBufferSize;
    }

    if(h > FrameBufferSize)
    {
        h = FrameBufferSize;
    }

    mPass1->GetTextureUnits()[0].SetTransform(NBRE_Transformationf::BuildScaleMatrix(static_cast<float>(w)/FrameBufferSize, static_cast<float>(h)/FrameBufferSize, 1.0f));
    mPass2->GetTextureUnits()[0].SetTransform(NBRE_Transformationf::BuildScaleMatrix(static_cast<float>(w)/FrameBufferSize, static_cast<float>(h)/FrameBufferSize, 1.0f));
    mSubView->Viewport().Update(x, y, w, h);
}

void NBGM_Animation::Accept(NBRE_Visitor& visitor)
{
    visitor.Visit(static_cast<NBRE_IRenderOperationProvider*>(this), mRenderEngine->Context().mSurfaceManager);
}

void NBGM_Animation::NotifyCamera(const NBRE_Camera& /*camera*/)
{
}

void NBGM_Animation::UpdateRenderQueue(NBRE_Overlay& overlay)
{
    if(mShardVertex == NULL || mIndexData == NULL)
        return;

    {
        NBRE_RenderOperation& renderOp = overlay.CreateRenderOperation();

        renderOp.SetVertexData(mShardVertex);
        renderOp.SetIndexData(mIndexData);
        renderOp.SetOperationType(NBRE_PMT_TRIANGLE_STRIP);

        renderOp.SetPass(mPass1);
        renderOp.SetTransform(&mVertexTransform1);
    }

    {
        NBRE_RenderOperation& renderOp = overlay.CreateRenderOperation();

        renderOp.SetVertexData(mShardVertex);
        renderOp.SetIndexData(mIndexData);
        renderOp.SetOperationType(NBRE_PMT_TRIANGLE_STRIP);

        renderOp.SetPass(mPass2);
        renderOp.SetTransform(&mVertexTransform2);
    }
}

int32 NBGM_Animation::OverlayId() const
{
    return 0;
}

nb_boolean NBGM_Animation::Visible() const
{
    return TRUE;
}

void NBGM_Animation::UpdateSurfaceList()
{
    //mRenderEngine->Context().mSurfaceManager->UpdateSurfaceList(mMapSurface);
}

void NBGM_Animation::ShootPicture1(NBRE_RenderSurface* surface)
{
    surface->AttachTarget(mRenderEngine->Context().mSurfaceManager->GetRenderTargetFromTexture(mPass1->GetTextureUnits()[0].GetTexture(mRenderEngine->Context()).get(), 0, 0, FALSE));
}

void NBGM_Animation::ShootPicture2(NBRE_RenderSurface* surface)
{
    surface->AttachTarget(mRenderEngine->Context().mSurfaceManager->GetRenderTargetFromTexture(mPass2->GetTextureUnits()[0].GetTexture(mRenderEngine->Context()).get(), 0, 0, FALSE));
}

void NBGM_Animation::Begin()
{
    mRenderEngine->SetActiveRenderSurface(mSurface);
}

void NBGM_Animation::End(NBRE_RenderSurface* surface)
{
    mRenderEngine->SetActiveRenderSurface(surface);
}

void NBGM_Animation::Move(float xPercent)
{
    xPercent *= 2;
    mVertexTransform1 = NBRE_Transformationd::BuildTranslateMatrix(-xPercent, 0, 0);
    mVertexTransform2 = NBRE_Transformationd::BuildTranslateMatrix(2-xPercent, 0, 0);
}
