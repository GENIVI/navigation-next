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
#include "nbgmrect2dimpl.h"
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
#include "nbgmcustomobjectscontext.h"


static NBRE_EntityPtr
CreateRect2DEntity(NBGM_Context& context, NBGM_CustomObjectsContext& customContext, int32 overlayId)
{
    NBRE_EntityPtr entity;
    NBRE_MeshPtr mesh = customContext.GetMesh(NBGM_MT_UNIT_SQUAR2D);
    if(mesh)
    {
        NBRE_ModelPtr model(NBRE_NEW NBRE_Model(mesh));
        entity.reset(NBRE_NEW NBRE_Entity(context.renderingEngine->Context(), model, overlayId));
    }
    return entity;
}

static NBRE_ShaderPtr
CreateRect2DShader(NBGM_Context& nbgmContext, NBRE_IOStream* stream)
{
    NBRE_ITextureImage* textureImage = NBRE_NEW NBRE_PngTextureImage(*(nbgmContext.renderingEngine->Context().mPalInstance), stream, 0, FALSE);
    NBRE_TexturePtr texture(nbgmContext.renderingEngine->Context().mRenderPal->CreateTexture(textureImage, NBRE_Texture::TT_2D, TRUE, "") );
    NBRE_PassPtr pass(NBRE_NEW NBRE_Pass());
    pass->SetEnableDepthTest(FALSE);
    pass->SetEnableDepthWrite(FALSE);
    pass->SetEnableCullFace(TRUE);
    pass->SetEnableBlend(TRUE);
    NBRE_TextureUnit unit;
    unit.SetState(NBRE_TextureState(NBRE_TFT_MIPMAP_TRILINEAR, NBRE_TFT_LINEAR, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, FALSE));
    unit.SetTexture(texture);
    pass->GetTextureUnits().push_back(unit);
    NBRE_ShaderPtr shader(NBRE_NEW NBRE_Shader());
    shader->AddPass(pass);
    return shader;
}

static NBRE_ShaderPtr
CreateRect2DShader(NBGM_CustomObjectsContext& customContext, NBGM_TextureId textureId)
{
    NBRE_ShaderPtr shader;
    NBRE_TexturePtr texture = customContext.GetTexture(textureId);
    if(!texture)
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBGM_Rect2dImpl::CreateRect2DShader from textureId failed!");
        return shader;
    }
    NBRE_PassPtr pass(NBRE_NEW NBRE_Pass());
    pass->SetEnableDepthTest(FALSE);
    pass->SetEnableDepthWrite(FALSE);
    pass->SetEnableCullFace(TRUE);
    pass->SetEnableBlend(TRUE);
    NBRE_TextureUnit unit;
    unit.SetState(NBRE_TextureState(NBRE_TFT_MIPMAP_TRILINEAR, NBRE_TFT_LINEAR, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, FALSE));
    unit.SetTexture(texture);
    pass->GetTextureUnits().push_back(unit);
    shader.reset(NBRE_NEW NBRE_Shader());
    shader->AddPass(pass);
    return shader;
}

NBGM_Rect2dImpl::NBGM_Rect2dImpl(NBGM_Context& nbgmContext,
                                 NBRE_SceneManager* sceneManager,
                                 NBRE_SurfaceSubView* subView,
                                 NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager,
                                 DrawOrderStruct drawOrder,
                                 NBGM_CustomObjectsContext& customContext,
                                 NBGM_Rect2dId rectId,
                                 const NBGM_Rect2dParameters &para)
    :mNBGMContext(nbgmContext)
    ,mSceneManager(sceneManager)
    ,mSubView(subView)
    ,mOverlayManager(overlayManager)
    ,mExtendType(NBGM_TMBET_NONE)
    ,mCustomContext(customContext)
    ,mNeedShaderUpdate(TRUE)
    ,mPosChanged(TRUE)
    ,mHeadingChanged(TRUE)
    ,mCenter(mNBGMContext.WorldToModel(para.centerX), mNBGMContext.WorldToModel(para.centerY))
    ,mWidth(para.width)
    ,mHeight(para.height)
    ,mHeading(para.heading)
    ,mVisible(para.visible)
    ,mRectId(rectId)
{
    mOverlayID = mOverlayManager.AssignOverlayId(drawOrder);
    InitializeShader(para);
}

NBGM_Rect2dImpl::~NBGM_Rect2dImpl()
{
    if(mExtendType != NBGM_TMBET_NONE)
    {
        mParentNode->RemoveChild(mExtendRectNode.get());
    }
    mParentNode->RemoveChild(mRectNode.get());
    mSceneManager->RootSceneNode()->RemoveChild(mParentNode.get());
}

void
NBGM_Rect2dImpl::InitializeShader(const NBGM_Rect2dParameters &para)
{
    // 1. Try to use the shared texture First.
    if(para.textureId != 0)
    {
        mShader = CreateRect2DShader(mCustomContext, para.textureId);
    }
    // 2. If failed, then try to use the exclusive texture
    if(!mShader && !para.buffer.empty())
    {
        NBRE_MemoryStream *memoryIO = NBRE_NEW NBRE_MemoryStream(para.buffer.addr, para.buffer.size, TRUE);
        if(memoryIO  != NULL)
        {
            mShader = CreateRect2DShader(mNBGMContext, memoryIO);
        }
    }
}

void
NBGM_Rect2dImpl::AddToSence()
{
    mParentNode = NBRE_NodePtr(NBRE_NEW NBRE_Node());
    mSceneManager->RootSceneNode()->AddChild(mParentNode);

    mRectNode = NBRE_NodePtr(NBRE_NEW NBRE_Node());
    mExtendRectNode = NBRE_NodePtr(NBRE_NEW NBRE_Node());
    mParentNode->AddChild(mRectNode);

    mRectEntity = CreateRect2DEntity(mNBGMContext, mCustomContext, mOverlayID);
    if(mRectEntity)
    {
        mExtendRectEntity = NBRE_EntityPtr(mRectEntity->Duplicate());
        mRectNode->AttachObject(mRectEntity);
        mExtendRectNode->AttachObject(mExtendRectEntity);
        mRectEntity->SetVisible(mVisible);
        mExtendRectEntity->SetVisible(mVisible);
    }
}

void
NBGM_Rect2dImpl::SetSize(float width, float height)
{
    mWidth = width;
    mHeight = height;
}

NBGM_Rect2dId
NBGM_Rect2dImpl::ID() const
{
    return mRectId;
}


void NBGM_Rect2dImpl::Update(double mercatorX, double mercatorY, float heading)
{
    double x = mNBGMContext.WorldToModel(mercatorX);
    double y = mNBGMContext.WorldToModel(mercatorY);
    if(mCenter.x != x || mCenter.y != y)
    {
        mCenter.x = x;
        mCenter.y = y;
        mPosChanged = TRUE;
    }

    if(mHeading != heading)
    {
        mHeading = heading;
        mHeadingChanged = TRUE;
    }
}

void
NBGM_Rect2dImpl::SetVisible(bool visible)
{
    if(mVisible != visible)
    {
        mVisible = visible;
        mRectEntity->SetVisible(visible);
        mExtendRectEntity->SetVisible(visible);
    }
}

void
NBGM_Rect2dImpl::SetTexture(const NBGM_BinaryBuffer& buffer)
{
    if(buffer.empty())
    {
        return;
    }
    NBRE_ShaderPtr shader;
    NBRE_MemoryStream *memoryIO = NBRE_NEW NBRE_MemoryStream(buffer.addr, buffer.size, TRUE);
    if(memoryIO != NULL)
    {
        shader = CreateRect2DShader(mNBGMContext, memoryIO);
    }
    if(!shader)
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBGM_Rect2dImpl::SetTexture from buffer failed!");
        return;
    }

    mShader = shader;
    mNeedShaderUpdate = TRUE;
}

void
NBGM_Rect2dImpl::SetTexture(NBGM_TextureId textureId)
{
    NBRE_ShaderPtr shader = CreateRect2DShader(mCustomContext, textureId);
    if(!shader)
    {
        NBRE_DebugLog(PAL_LogSeverityCritical, "NBGM_Rect2dImpl::SetTexture from id failed!");
        return;
    }

    mShader = shader;
    mNeedShaderUpdate = TRUE;
}

void
NBGM_Rect2dImpl::Update(double pixelPerUnit)
{
    if(!mVisible)
    {
        return;
    }

    NBRE_Point3d rectPos(mCenter.x, mCenter.y, 0.0);
    float scaleX = 0.f;
    float scaleY = 0.f;
    if(pixelPerUnit > 0)
    {
        scaleX = static_cast<float>(mHeight/pixelPerUnit);
        scaleY = static_cast<float>(mWidth/pixelPerUnit);
    }
    mRectNode->SetScale(NBRE_Vector3f(scaleX, scaleY, 1.f));
    mExtendRectNode->SetScale(NBRE_Vector3f(scaleX, scaleY, 1.f));

    if(mPosChanged)
    {
        mParentNode->SetPosition(rectPos);
        mPosChanged = FALSE;
    }

    if(mHeadingChanged)
    {
        mRectNode->SetOrientation(NBRE_Orientation(NBRE_Vector3f(0, 0, 1), static_cast<float>(90- mHeading)));
        mExtendRectNode->SetOrientation(NBRE_Orientation(NBRE_Vector3f(0, 0, 1), static_cast<float>(90-mHeading)));
        mHeadingChanged = FALSE;
    }

    if (mNeedShaderUpdate && mRectEntity && mShader)
    {
        mRectEntity->SetVisible(TRUE);
        mExtendRectEntity->SetVisible(TRUE);
        mRectEntity->GetSubEntity(0).SetShader(mShader);
        mExtendRectEntity->GetSubEntity(0).SetShader(mShader);
        mNeedShaderUpdate = FALSE;
    }
}

void
NBGM_Rect2dImpl::NotifyExtend(const NBGM_TileMapBoundaryExtendType& extendType)
{
    if(mExtendType != extendType)
    {
        switch(extendType)
        {
        case NBGM_TMBET_NONE:
            mParentNode->RemoveChild(mExtendRectNode.get());
            break;
        case NBGM_TMBET_LEFT:
            mExtendRectNode->SetPosition(NBRE_Point3f(mNBGMContext.WorldToModel(-NBRE_Math::PiTwo), 0.0f, 0.0f));
            if(mExtendType == NBGM_TMBET_NONE)
            {
                mParentNode->AddChild(mExtendRectNode);
            }
            break;
        case NBGM_TMBET_RIGHT:
            mExtendRectNode->SetPosition(NBRE_Point3f(mNBGMContext.WorldToModel(NBRE_Math::PiTwo), 0.0f, 0.0f));
            if(mExtendType == NBGM_TMBET_NONE)
            {
                mParentNode->AddChild(mExtendRectNode);
            }
            break;
        default:
            break;
        }
        mExtendType = extendType;
    }
}
