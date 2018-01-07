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
#include "nbgmflag.h"
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
#include "nbgmbuildutility.h"

static const char* SQUARE_FLAG_TEX_2D_START = "TEXTURE/SQUARE_FLAG_TEX_2D_START";
static const char* SQUARE_FLAG_TEX_3D_START = "TEXTURE/SQUARE_FLAG_TEX_3D_START";
static const char* SQUARE_FLAG_TEX_2D_END = "TEXTURE/SQUARE_FLAG_TEX_2D_END";
static const char* SQUARE_FLAG_TEX_3D_END = "TEXTURE/SQUARE_FLAG_TEX_3D_END";

static NBRE_ShaderPtr
CreateMapShader(NBGM_Context& nbgmContext, const NBRE_String& path)
{
    NBRE_TexturePtr texture = NBGM_BuildTextureUtility::ConstructImageTexture(*nbgmContext.renderingEngine->Context().mRenderPal, nbgmContext.renderingEngine->Context().mPalInstance, path, 0, TRUE, NBRE_Texture::TT_2D, TRUE);

    NBRE_PassPtr pass(NBRE_NEW NBRE_Pass());
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(TRUE);
    pass->SetEnableCullFace(TRUE);
    pass->SetEnableBlend(FALSE);
    NBRE_TextureUnit unit;
    unit.SetState(NBRE_TextureState(NBRE_TFT_MIPMAP_TRILINEAR, NBRE_TFT_LINEAR, NBRE_TAM_REPEAT, NBRE_TAM_REPEAT, FALSE));
    unit.SetTexture(texture);
    pass->GetTextureUnits().push_back(unit);
    NBRE_ShaderPtr shader(NBRE_NEW NBRE_Shader());
    shader->AddPass(pass);
    return shader;
}

NBGM_Flag::NBGM_Flag(NBGM_Context& nbgmContext, NBRE_SceneManager* sceneManager, int32 overlayId, nb_boolean isEndFlag)
    :NBGM_LayoutElement(nbgmContext)
    ,mSceneManager(sceneManager)
    ,mOverlayID(overlayId)
    ,mIsEndFlag(isEndFlag)
    ,mSize(0.0f)
    ,mMinPixelSize(0.0f)
    ,mMaxPixelSize(0.0f)
    ,mNeedUpdate(TRUE)
    ,mAvailableScreenPt(TRUE)
    ,mExtendType(NBGM_TMBET_NONE)
{
}

NBGM_Flag::~NBGM_Flag()
{
    if(mExtendType != NBGM_TMBET_NONE)
    {
        mParentNode->RemoveChild(mExtendNode.get());
    }
    mParentNode->RemoveChild(mNode.get());
    mSceneManager->RootSceneNode()->RemoveChild(mParentNode.get());
}

void
NBGM_Flag::AddToSence()
{
    NBGM_LayoutElement::AddToSence();
    mParentNode = NBRE_NodePtr(NBRE_NEW NBRE_Node());
    mNode = NBRE_NodePtr(NBRE_NEW NBRE_Node());
    mExtendNode = NBRE_NodePtr(NBRE_NEW NBRE_Node());
    mSceneManager->RootSceneNode()->AddChild(mParentNode);
    mParentNode->AddChild(mNode);

    NBRE_Vector<NBRE_ModelPtr> models;
    NBRE_String path = NBGM_MakeFullPath("SQUARE_FLAG_MODEL_3D");
    NBGM_LoadBinFile(*mNBGMContext.renderingEngine->Context().mRenderPal, mNBGMContext.renderingEngine->Context().mPalInstance,
        mNBGMContext.GetModelScaleFactor(), path.c_str(), models);
    m3DEntity = NBRE_EntityPtr(NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), models.back(), mOverlayID));

    path = NBGM_MakeFullPath("SQUARE_FLAG_MODEL_2D");
    NBGM_LoadBinFile(*mNBGMContext.renderingEngine->Context().mRenderPal, mNBGMContext.renderingEngine->Context().mPalInstance,
        mNBGMContext.GetModelScaleFactor(), path.c_str(), models);
    m2DEntity = NBRE_EntityPtr(NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), models.back(), mOverlayID));

    if (mIsEndFlag)
    {
        m2DEndShader = CreateMapShader(mNBGMContext, NBGM_MakeFullPath(SQUARE_FLAG_TEX_2D_END));
        m3DEndShader = CreateMapShader(mNBGMContext, NBGM_MakeFullPath(SQUARE_FLAG_TEX_3D_END));

        m2DEntity->SetShader(m2DEndShader);
        m3DEntity->SetShader(m3DEndShader);
    }
    else
    {
        m2DStartShader = CreateMapShader(mNBGMContext, NBGM_MakeFullPath(SQUARE_FLAG_TEX_2D_START));
        m3DStartShader = CreateMapShader(mNBGMContext, NBGM_MakeFullPath(SQUARE_FLAG_TEX_3D_START));

        m2DEntity->SetShader(m2DStartShader);
        m3DEntity->SetShader(m3DStartShader);
    }

    mNode->AttachObject(m3DEntity);
    mNode->AttachObject(m2DEntity);

    mExtend2DEntity = NBRE_EntityPtr(m2DEntity->Duplicate());
    mExtend3DEntity = NBRE_EntityPtr(m3DEntity->Duplicate());
    mExtendNode->AttachObject(mExtend3DEntity);
    mExtendNode->AttachObject(mExtend2DEntity);

    Update();
}

void 
NBGM_Flag::SetPosition(const NBRE_Vector3d& position)
{
    mPosition = position;
    mNeedUpdate = TRUE;
}

void 
NBGM_Flag::AdjustSize(float size, float minPixelSize, float maxPixelSize)
{
    mSize = size;
    mMinPixelSize = minPixelSize;
    mMaxPixelSize = maxPixelSize;
    mNeedUpdate = TRUE;
}

void
NBGM_Flag::GetPolygon(Polygon2f polygons[])
{
    NBRE_Vector2f sp[8];
    for (uint32 i = 0; i < 8; ++i)
    {
        sp[i] = NBRE_TypeConvertf::Convert(mScreenPoints[i]);
    }

    uint32 index = 0;

    polygons[index].push_back(sp[0]);
    polygons[index].push_back(sp[1]);
    polygons[index].push_back(sp[5]);
    polygons[index].push_back(sp[4]);
    ++index;

    polygons[index].push_back(sp[1]);
    polygons[index].push_back(sp[2]);
    polygons[index].push_back(sp[6]);
    polygons[index].push_back(sp[5]);
    ++index;

    polygons[index].push_back(sp[3]);
    polygons[index].push_back(sp[2]);
    polygons[index].push_back(sp[6]);
    polygons[index].push_back(sp[7]);
    ++index;

    polygons[index].push_back(sp[0]);
    polygons[index].push_back(sp[3]);
    polygons[index].push_back(sp[7]);
    polygons[index].push_back(sp[4]);
    ++index;

    polygons[index].push_back(sp[4]);
    polygons[index].push_back(sp[5]);
    polygons[index].push_back(sp[6]);
    polygons[index].push_back(sp[7]);
    ++index;
}

void
NBGM_Flag::Update()
{
    NBGM_LayoutManager* layoutManager = mNBGMContext.layoutManager;
    const NBRE_CameraPtr& camera = layoutManager->GetCamera();

    float baseSize = 1.0f;
    const NBRE_Vector3f& camereDir = camera->WorldDirection();
    float heading = NBRE_Math::DirectionToHeading(camereDir.x, camereDir.y);
    if (camera->GetFrustum().GetProjectionType() == NBRE_PT_ORTHO_GRAPHIC)
    {
        m2DEntity->SetVisible(TRUE);
        m3DEntity->SetVisible(FALSE);
        mExtend2DEntity->SetVisible(TRUE);
        mExtend3DEntity->SetVisible(FALSE);
        baseSize = (float)m2DEntity->GetModel()->GetMesh()->AxisAlignedBox().GetSize().x;
    }
    else
    {
        m2DEntity->SetVisible(FALSE);
        m3DEntity->SetVisible(TRUE);
        mExtend2DEntity->SetVisible(FALSE);
        mExtend3DEntity->SetVisible(TRUE);
        baseSize = (float)m3DEntity->GetModel()->GetMesh()->AxisAlignedBox().GetSize().z;
    }

    float scale = 1.0f;
    double pixelPerUnit = 0;
    if (layoutManager->PixelsPerUnit(mPosition, pixelPerUnit))
    {
        float pixelSize = static_cast<float>(pixelPerUnit * mSize);
        if (pixelSize < mMinPixelSize)
        {
            scale *= mMinPixelSize / pixelSize;
        }
        else if (pixelSize > mMaxPixelSize)
        {
            scale *= mMaxPixelSize / pixelSize;
        }
        else
        {
            scale *= 1.0;
        }
    }
    scale = mSize / baseSize * scale;

    mParentNode->SetPosition(mPosition);
    mNode->SetScale(NBRE_Vector3f(scale, scale, scale));
    mExtendNode->SetScale(NBRE_Vector3f(scale, scale, scale));
    mNode->SetOrientation(NBRE_Orientation(NBRE_Vector3f(0, 0, 1), static_cast<float>(heading) - 90.0f));
    mExtendNode->SetOrientation(NBRE_Orientation(NBRE_Vector3f(0, 0, 1), static_cast<float>(heading) - 90.0f));

    const NBRE_NoneAxisAlignedBox3d& aabb = ObjectAlignedBox();
    NBRE_Vector3d aabbCorners[8];
    aabb.GetCorners(aabbCorners);
    mAvailableScreenPt = TRUE;
    for (uint32 i = 0; i < 8; ++i)
    {
        mAvailableScreenPt = layoutManager->WorldToScreen(aabbCorners[i], mScreenPoints[i]);
        if(!mAvailableScreenPt)
        {
            break;
        }
    }

    mNeedUpdate = FALSE;
}

nb_boolean
NBGM_Flag::IsScreenPointAvailable()
{
    return mAvailableScreenPt;
}

nb_boolean
NBGM_Flag::IsElementVisible()
{
    Update();
    return TRUE;
}

void
NBGM_Flag::UpdateRegion()
{   
}

nb_boolean
NBGM_Flag::IsElementRegionAvailable()
{   
    return TRUE;
}

NBRE_Vector3d
NBGM_Flag::GetPosition()
{
    return mParentNode->Position();
}

NBRE_AxisAlignedBox3d 
NBGM_Flag::AxisAlignedBox() const
{
    NBGM_LayoutManager* layoutManager = mNBGMContext.layoutManager;
    const NBRE_CameraPtr& camera = layoutManager->GetCamera();

    if(IsExtendNodeInFrustum())
    {
        mExtendNode->UpdatePosition();
        NBRE_EntityPtr entity;
        if (camera->GetFrustum().GetProjectionType() == NBRE_PT_ORTHO_GRAPHIC)
        {
            entity = mExtend2DEntity;
        }
        else
        {
            entity = mExtend3DEntity;
        }

        return entity->AxisAlignedBox();
    }

    mNode->UpdatePosition();
    NBRE_EntityPtr entity;
    if (camera->GetFrustum().GetProjectionType() == NBRE_PT_ORTHO_GRAPHIC)
    {
        entity = m2DEntity;
    }
    else
    {
        entity = m3DEntity;
    }

    return entity->AxisAlignedBox();
}

NBRE_NoneAxisAlignedBox3d
NBGM_Flag::ObjectAlignedBox() const
{
    NBGM_LayoutManager* layoutManager = mNBGMContext.layoutManager;
    const NBRE_CameraPtr& camera = layoutManager->GetCamera();

    if(IsExtendNodeInFrustum())
    {
        mExtendNode->UpdatePosition();
        NBRE_EntityPtr entity;
        if (camera->GetFrustum().GetProjectionType() == NBRE_PT_ORTHO_GRAPHIC)
        {
            entity = mExtend2DEntity;
        }
        else
        {
            entity = mExtend3DEntity;
        }

        return entity->BoundingBox();
    }

    mNode->UpdatePosition();
    NBRE_EntityPtr entity;
    if (camera->GetFrustum().GetProjectionType() == NBRE_PT_ORTHO_GRAPHIC)
    {
        entity = m2DEntity;
    }
    else
    {
        entity = m3DEntity;
    }

    return entity->BoundingBox();
}

void 
NBGM_Flag::Hide()
{
    m2DEntity->SetVisible(FALSE);
    m3DEntity->SetVisible(FALSE);

    mExtend2DEntity->SetVisible(FALSE);
    mExtend3DEntity->SetVisible(FALSE);
}

void
NBGM_Flag::NotifyExtend(const NBGM_TileMapBoundaryExtendType& extendType)
{
    if(mExtendType != extendType)
    {
        switch(extendType)
        {
        case NBGM_TMBET_NONE:
            mParentNode->RemoveChild(mExtendNode.get());
            break;
        case NBGM_TMBET_LEFT:
            mExtendNode->SetPosition(NBRE_Point3f(mNBGMContext.WorldToModel(-NBRE_Math::PiTwo), 0.0f, 0.0f));
            if(mExtendType == NBGM_TMBET_NONE)
            {
                mParentNode->AddChild(mExtendNode);
            }
            break;
        case NBGM_TMBET_RIGHT:
            mExtendNode->SetPosition(NBRE_Point3f(mNBGMContext.WorldToModel(NBRE_Math::PiTwo), 0.0f, 0.0f));
            if(mExtendType == NBGM_TMBET_NONE)
            {
                mParentNode->AddChild(mExtendNode);
            }
            break;
        default:
            break;
        }
        mExtendType = extendType;
        mNeedUpdate = TRUE;
    }
}

nb_boolean
NBGM_Flag::IsExtendNodeInFrustum() const
{
    return !mNBGMContext.layoutManager->IsInFrustum(mExtendNode->WorldPosition()) 
        || mExtendType == NBGM_TMBET_NONE
        ? FALSE : TRUE;
}
