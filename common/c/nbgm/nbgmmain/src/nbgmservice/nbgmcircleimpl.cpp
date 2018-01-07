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

(C) Copyright 2014 by Networks In Motion, Inc.

The information contained herein is confidential, proprietary
to Networks In Motion, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of Networks In Motion is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#include "nbgmcircleimpl.h"
#include "nbretransformation.h"
#include "nbreoverlay.h"
#include "nbrerenderpal.h"
#include "nbrehardwarebuffer.h"
#include "nbreintersection.h"
#include "nbremath.h"
#include "nbretypeconvert.h"
#include "nbrescenemanager.h"
#include "nbgmbinloader.h"
#include "nbrepngtextureimage.h"
#include "nbrerenderengine.h"
#include "nbgmprotected.h"
#include "nbgmconst.h"
#include "nbrememorystream.h"
#include "nbretransformutil.h"
#include "nbgmbuildutility.h"


static NBRE_ShaderPtr
CreateCircleShader(NBRE_Context& context, const NBRE_Color& clr)
{
    NBRE_PassPtr pass(NBRE_NEW NBRE_Pass());
    pass->SetEnableDepthTest(FALSE);
    pass->SetEnableDepthWrite(FALSE);
    pass->SetEnableCullFace(TRUE);
    pass->SetEnableBlend(TRUE);
    NBRE_ShaderPtr shader(NBRE_NEW NBRE_Shader());

    NBRE_Image *image = NBRE_NEW NBRE_Image(1, 1, NBRE_PF_R8G8B8A8);
    uint8 *buffer = image->GetImageData();
    buffer[0] = static_cast<uint8>(clr.r*255);
    buffer[1] = static_cast<uint8>(clr.g*255);
    buffer[2] = static_cast<uint8>(clr.b*255);
    buffer[3] = static_cast<uint8>(clr.a*255);

    NBRE_Image **images = NBRE_NEW NBRE_Image*[1];
    images[0] = image;
    NBRE_TexturePtr texture(context.mRenderPal->CreateTexture(images, 1, 1, FALSE, NBRE_Texture::TT_2D, ""));

    NBRE_TextureUnit unit;
    unit.SetState(NBRE_TextureState(NBRE_TFT_POINT, NBRE_TFT_POINT, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, FALSE));
    unit.SetTexture(texture);
    pass->GetTextureUnits().push_back(unit);

    shader->AddPass(pass);
    return shader;
}

//@TODO:
// a. Move this function to a utility class.
// b. Decide if all the circles could share one mesh.
static NBRE_EntityPtr
CreateCircleEntity(NBGM_Context& context, int32 overlayId)
{
    const uint32 N = 90;
    const float Det = NBRE_Math::PiTwo/N;
    const uint32 VertexCount = N+2;
    const uint32 indexCount0 = N*3;
    const uint32 indexCount1 = N+1;

    float* vertices = NBRE_NEW float[VertexCount*3];
    float* texCoords = NBRE_NEW float[VertexCount*2];
    uint16* indices0 = NBRE_NEW uint16[indexCount0];
    uint16* indices1 = NBRE_NEW uint16[indexCount1];
    nsl_memset(vertices, 0, VertexCount*3*sizeof(float));
    nsl_memset(texCoords, 0, VertexCount*2*sizeof(float));
    nsl_memset(indices0, 0, indexCount0*sizeof(uint16));
    nsl_memset(indices1, 0, indexCount1*sizeof(uint16));

    for(uint32 i = 1; i < VertexCount; i++)
    {
        const uint32 index  = i*3;
        const float angle = Det*(i-1);
        vertices[index]   = nsl_cos(angle);
        vertices[index+1] = nsl_sin(angle);
        vertices[index+2] = 0;
    }
    for(uint16 i = 0, j = 1; i < N; i++, j++)
    {
       const uint16 index = i*3;
       indices0[index]   = 0;
       indices0[index+1] = j;
       indices0[index+2] = j+1;
    }
    for(uint16 i = 0; i < indexCount1; i++)
    {
        indices1[i] = i+1;
    }

    NBRE_IRenderPal *renderPal = context.renderingEngine->Context().mRenderPal;
    NBRE_HardwareVertexBuffer* vertextBuff = renderPal->CreateVertexBuffer(sizeof(float)*3, VertexCount, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    vertextBuff->WriteData(0, sizeof(float)*3*VertexCount, vertices, TRUE);
    NBRE_HardwareVertexBuffer* texCoordBuff = renderPal->CreateVertexBuffer(sizeof(float)*2, VertexCount, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    texCoordBuff->WriteData(0, sizeof(float)*2*VertexCount, texCoords, TRUE);

    NBRE_VertexDeclaration* decalration = renderPal->CreateVertexDeclaration();
    NBRE_VertexElement* posElem = NBRE_NEW NBRE_VertexElement(0, 0, NBRE_VertexElement::VET_FLOAT3, NBRE_VertexElement::VES_POSITION, 0);
    decalration->GetVertexElementList().push_back(posElem);
    NBRE_VertexElement* texCoordElem = NBRE_NEW NBRE_VertexElement(1, 0, NBRE_VertexElement::VET_FLOAT2, NBRE_VertexElement::VES_TEXTURE_COORDINATES, 0);
    decalration->GetVertexElementList().push_back(texCoordElem);

    NBRE_VertexData* vertexData = NBRE_NEW NBRE_VertexData(0);
    vertexData->AddBuffer(vertextBuff);
    vertexData->AddBuffer(texCoordBuff);
    vertexData->SetVertexDeclaration(decalration);

    NBRE_HardwareIndexBuffer* indexBuff0 = renderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, indexCount0, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    indexBuff0->WriteData(0, sizeof(uint16)*indexCount0, indices0, TRUE);
    NBRE_IndexData* indexData0 = NBRE_NEW NBRE_IndexData(indexBuff0, 0, indexBuff0->GetNumIndexes());

    NBRE_HardwareIndexBuffer* indexBuff1 = renderPal->CreateIndexBuffer(NBRE_HardwareIndexBuffer::IT_16BIT, indexCount1, TRUE, NBRE_HardwareBuffer::HBU_STATIC);
    indexBuff1->WriteData(0, sizeof(uint16)*indexCount1, indices1, TRUE);
    NBRE_IndexData* indexData1 = NBRE_NEW NBRE_IndexData(indexBuff1, 0, indexBuff1->GetNumIndexes());

    NBRE_MeshPtr mesh(NBRE_NEW NBRE_Mesh(vertexData));
    mesh->CreateSubMesh(indexData0, NBRE_PMT_TRIANGLE_LIST);
    mesh->CreateSubMesh(indexData1, NBRE_PMT_LINE_STRIP);
    mesh->CalculateBoundingBox();
    NBRE_ModelPtr model(NBRE_NEW NBRE_Model(mesh));
    NBRE_EntityPtr entity(NBRE_NEW NBRE_Entity(context.renderingEngine->Context(), model, overlayId));

    NBRE_DELETE_ARRAY vertices;
    NBRE_DELETE_ARRAY texCoords;
    NBRE_DELETE_ARRAY indices0;
    NBRE_DELETE_ARRAY indices1;
    return entity;
}

NBGM_CircleImpl::NBGM_CircleImpl(NBGM_Context& nbgmContext,
                                 NBRE_SceneManager* sceneManager,
                                 NBRE_SurfaceSubView* subView,
                                 NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager,
                                 DrawOrderStruct drawOrder,
                                 int circleId,
                                 const NBGM_CircleParameters &circlePara)
    :mNBGMContext(nbgmContext)
    ,mSceneManager(sceneManager)
    ,mSubView(subView)
    ,mOverlayManager(overlayManager)
    ,mExtendType(NBGM_TMBET_NONE)
    ,mModelScale(0.f)
    ,mNeedShaderUpdate(TRUE)
    ,mCenter(mNBGMContext.WorldToModel(circlePara.centerX), mNBGMContext.WorldToModel(circlePara.centerY))
    ,mRadius(circlePara.radius)
    ,mFillClr(circlePara.fillColor.red, circlePara.fillColor.green, circlePara.fillColor.blue, circlePara.fillColor.alpha)
    ,mOutlineClr(circlePara.strokeColor.red, circlePara.strokeColor.green, circlePara.strokeColor.blue, circlePara.strokeColor.alpha)
    ,mZOrder(circlePara.zOrder)
    ,mVisible(circlePara.visible)
    ,mCircleId(circleId)
{
    mOverlayID = mOverlayManager.AssignOverlayId(drawOrder);
}

NBGM_CircleImpl::~NBGM_CircleImpl()
{
    if(mExtendType != NBGM_TMBET_NONE)
    {
        mParentNode->RemoveChild(mExtendCircleNode.get());
    }
    mParentNode->RemoveChild(mCircleNode.get());
    mSceneManager->RootSceneNode()->RemoveChild(mParentNode.get());
}

void
NBGM_CircleImpl::AddToSence()
{
    mParentNode = NBRE_NodePtr(NBRE_NEW NBRE_Node());
    mSceneManager->RootSceneNode()->AddChild(mParentNode);

    mCircleNode = NBRE_NodePtr(NBRE_NEW NBRE_Node());
    mExtendCircleNode = NBRE_NodePtr(NBRE_NEW NBRE_Node());
    mParentNode->AddChild(mCircleNode);

    mFillShader = CreateCircleShader(mNBGMContext.renderingEngine->Context(), mFillClr);
    mOutlineShader = CreateCircleShader(mNBGMContext.renderingEngine->Context(), mOutlineClr);

    mCircleEntity = CreateCircleEntity(mNBGMContext, mOverlayID);
    mExtendCircleEntity = NBRE_EntityPtr(mCircleEntity->Duplicate());
    mCircleEntity->SetVisible(mVisible);
    mExtendCircleEntity->SetVisible(mVisible);
    mCircleNode->AttachObject(mCircleEntity);
    mExtendCircleNode->AttachObject(mExtendCircleEntity);
}

int
NBGM_CircleImpl::ID() const
{
    return mCircleId;
}

void
NBGM_CircleImpl::SetCenter(double x, double y)
{
    mCenter.x = mNBGMContext.WorldToModel(x);
    mCenter.y = mNBGMContext.WorldToModel(y);
}

void
NBGM_CircleImpl::SetVisible(bool visible)
{
    if(mVisible != visible)
    {
        mVisible = visible;
        mCircleEntity->SetVisible(visible);
        mExtendCircleEntity->SetVisible(visible);
        mNeedShaderUpdate = TRUE;
    }
}

void
NBGM_CircleImpl::SetStyle(float radius, const NBGM_Color& fillColor, const NBGM_Color& outlineColor)
{
    mRadius = radius;
    NBRE_Color innerClr(fillColor.red, fillColor.green, fillColor.blue, fillColor.alpha);
    NBRE_Color outlineClr(outlineColor.red, outlineColor.green, outlineColor.blue, outlineColor.alpha);

    if(mFillClr != innerClr)
    {
        mFillShader = CreateCircleShader(mNBGMContext.renderingEngine->Context(), innerClr);
        mFillClr = innerClr;
        mNeedShaderUpdate = TRUE;
    }

    if(mOutlineClr != outlineClr)
    {
        mOutlineShader = CreateCircleShader(mNBGMContext.renderingEngine->Context(), outlineClr);
        mOutlineClr = outlineClr;
        mNeedShaderUpdate = TRUE;
    }
}

void
NBGM_CircleImpl::Update(double pixelPerUnit)
{
    if(!mVisible)
    {
        return;
    }

    NBRE_Point3d circlePos(mCenter.x, mCenter.y, 0.0);
    float scale = 0.f;
    if(pixelPerUnit > 0)
    {
        scale = static_cast<float>(mRadius/pixelPerUnit);
    }

    mCircleNode->SetScale(NBRE_Vector3f(scale, scale, 1.f));
    mExtendCircleNode->SetScale(NBRE_Vector3f(scale, scale, 1.f));

    mParentNode->SetPosition(circlePos);
    if (!mNeedShaderUpdate)
    {
        return;
    }

    if(mVisible && mRadius > 0.f)
    {
        mCircleEntity->SetVisible(TRUE);
        mExtendCircleEntity->SetVisible(TRUE);
        mCircleEntity->GetSubEntity(0).SetShader(mFillShader);
        mCircleEntity->GetSubEntity(1).SetShader(mOutlineShader);
        mExtendCircleEntity->GetSubEntity(0).SetShader(mFillShader);
        mExtendCircleEntity->GetSubEntity(1).SetShader(mOutlineShader);
    }

    mNeedShaderUpdate = FALSE;
}

void
NBGM_CircleImpl::NotifyExtend(const NBGM_TileMapBoundaryExtendType& extendType)
{
    if(mExtendType != extendType)
    {
        switch(extendType)
        {
        case NBGM_TMBET_NONE:
            mParentNode->RemoveChild(mExtendCircleNode.get());
            break;
        case NBGM_TMBET_LEFT:
            mExtendCircleNode->SetPosition(NBRE_Point3f(mNBGMContext.WorldToModel(-NBRE_Math::PiTwo), 0.0f, 0.0f));
            if(mExtendType == NBGM_TMBET_NONE)
            {
                mParentNode->AddChild(mExtendCircleNode);
            }
            break;
        case NBGM_TMBET_RIGHT:
            mExtendCircleNode->SetPosition(NBRE_Point3f(mNBGMContext.WorldToModel(NBRE_Math::PiTwo), 0.0f, 0.0f));
            if(mExtendType == NBGM_TMBET_NONE)
            {
                mParentNode->AddChild(mExtendCircleNode);
            }
            break;
        default:
            break;
        }
        mExtendType = extendType;
    }
}