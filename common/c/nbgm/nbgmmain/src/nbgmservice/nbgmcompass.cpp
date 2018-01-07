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
#include "nbgmcompass.h"
#include "nbgmcontext.h"
#include "nbreentity.h"
#include "nbrerenderengine.h"
#include "nbrepass.h"
#include "nbreshader.h"
#include "nbreutility.h"
#include "nbgmbuildutility.h"
#include "nbgmconst.h"
#include "nbreentity.h"
#include "nbrefrustum.h"
#include "nbreintersection.h"
#include "nbgmplatformconfig.h"
#include "nbredefaultshadergroup.h"
#include "paldisplay.h"
#include "nbgmprotected.h"

static const uint32 HD_2K_HEIGHT = 2000;
static const uint32 FULL_HD_WIDTH = 1080;
static const uint32 WVGA_WIDTH = 320;
static const float DEFAULT_COMPASS_WIDTH_MM = 12.25f;
static const float DEFAULT_COMPASS_HEIGHT_MM = 12.25f;

NBGM_Compass::NBGM_Compass(NBGM_Context& nbgmContext, NBRE_SurfaceSubView* subView, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, const DrawOrderStruct& drawOrder)
    :mNBGMContext(nbgmContext)
    ,mSubView(subView)
    ,mOverlayManager(overlayManager)
    ,mCamera(NBRE_NEW NBRE_Camera())
    ,mOverlay(NULL)
    ,mNode(NBRE_NEW NBRE_Node())
    ,mShadowNode(NBRE_NEW NBRE_Node())
    ,mIsDefault(TRUE)
    ,mEnable(FALSE)
    ,mViewPortUpdated(FALSE)
    ,mIsDayMode(TRUE)
    ,mUpdateIcons(TRUE)
    ,mVisible(FALSE)
    ,mRotateAngle(0.0f)
{
    mCamera->LookAt(NBRE_Vector3f(0.0f, 0.0f, 1.0f),
                    NBRE_Vector3f(0.0f, 0.0f, 0.0f),
                    NBRE_Vector3f(0.0f, 1.0f, 0.0f));

    mConfig.mBottom = 0.0f;
    mConfig.mLeft = 0.0f;
    mConfig.mRight = 0.0f;
    mConfig.mTop = 0.0f;

    mOverlayID = overlayManager.AssignOverlayId(drawOrder);
    mOverlay = overlayManager.FindOverlay(mOverlayID);
    mOverlay->SetAutoClearRenderProvider(FALSE);
    mOverlay->SetCamera(mCamera);
}

NBGM_Compass::~NBGM_Compass()
{
}

void
NBGM_Compass::OnSizeChanged(int32 x, int32 y, uint32 w, uint32 h)
{
    mConfig.mLeft = static_cast<float>(x);
    mConfig.mRight = static_cast<float>(x + w);
    mConfig.mBottom = static_cast<float>(y);
    mConfig.mTop = static_cast<float>(y + h);

    mViewPortUpdated = TRUE;
}

void
NBGM_Compass::SetPosition(float screenX, float screenY)
{
    mPosition.x = screenX;
    mPosition.y = screenY;
    UpdateBoundingBox2D(mDayIconSize, mDayBoundingBox);
    UpdateBoundingBox2D(mNightIconSize, mNightBoundingBox);
}

void
NBGM_Compass::Rotate(float angle)
{
    mRotateAngle += angle;
}

void
NBGM_Compass::SetRotateAngle(float angle)
{
    mRotateAngle = angle;
}

void
NBGM_Compass::SetEnable(nb_boolean value)
{
    mEnable = value;
}

nb_boolean
NBGM_Compass::IsTapped(float screenX, float screenY)
{ 
    if(mEnable)
    {
        if(mIsDayMode)
        {
            return NBRE_Intersectionf::HitTest(mDayBoundingBox, NBRE_Vector2f(screenX, screenY));
        }
        else
        {
            return NBRE_Intersectionf::HitTest(mNightBoundingBox, NBRE_Vector2f(screenX, screenY));
        }
    }
    return FALSE;
}

void
NBGM_Compass::Update()
{
    if (mViewPortUpdated)
    {
        mCamera->GetFrustum().SetAsOrtho(mConfig, -1.0f, 1.0f);
        mViewPortUpdated = FALSE;
    }

    NBRE_Point3d pos;
    NBRE_Vector2i vec = mSubView->Viewport().GetRect().GetSize();
    pos.x = mPosition.x;
    pos.y = vec.y - mPosition.y;

    mNode->SetPosition(pos);

    UpdateIcons();

    if(mIsDefault)
    {
        mNode->SetOrientation(NBRE_Orientation(NBRE_Vector3f(0, 0, 1), -mRotateAngle + 90.0f));
        mShadowNode->SetOrientation(NBRE_Orientation(NBRE_Vector3f(0, 0, 1), 90.0f));
        mShadowNode->SetPosition(pos);
    }
    else
    {
        mNode->SetOrientation(NBRE_Orientation(NBRE_Vector3f(0, 0, 1), -mRotateAngle + 90.0f));
    }

    mDayEntity->SetVisible(FALSE);
    mNightEntity->SetVisible(FALSE);
    if(mShadowEntity)
    {
        mShadowEntity->SetVisible(FALSE);
    }

    if(mEnable)
    {
        mVisible = TRUE;

        if(mIsDayMode)
        {
            mDayEntity->SetVisible(mVisible);
            mNightEntity->SetVisible(FALSE);
            if(mIsDefault)
            {
                mShadowEntity->SetVisible(mVisible);
            }
        }
        else
        {
            mDayEntity->SetVisible(FALSE);
            mNightEntity->SetVisible(mVisible);
            if(mIsDefault)
            {
                mShadowEntity->SetVisible(mVisible);
            }
        }
    }
}

void
NBGM_Compass::SetDayNightMode(nb_boolean isDayMode)
{
    mIsDayMode = isDayMode;
}

void
NBGM_Compass::GetDefaultIconPath(NBRE_String& dayIconPath, NBRE_String& nightIconPath, NBRE_String& shadowPath)
{
    const NBRE_String DAY_ICON_256x256 = "TEXTURE/COMPASS_DAY_TEX_256x256";
    const NBRE_String DAY_ICON_128x128 = "TEXTURE/COMPASS_DAY_TEX_128x128";
    const NBRE_String DAY_ICON_64x64 = "TEXTURE/COMPASS_DAY_TEX_64x64";
    const NBRE_String DAY_ICON_32x32 = "TEXTURE/COMPASS_DAY_TEX_32x32";

    const NBRE_String NIGHT_ICON_256x256 = "TEXTURE/COMPASS_NIGHT_TEX_256x256";
    const NBRE_String NIGHT_ICON_128x128 = "TEXTURE/COMPASS_NIGHT_TEX_128x128";
    const NBRE_String NIGHT_ICON_64x64 = "TEXTURE/COMPASS_NIGHT_TEX_64x64";
    const NBRE_String NIGHT_ICON_32x32 = "TEXTURE/COMPASS_NIGHT_TEX_32x32";

    const NBRE_String SHADOW_ICON_256x256 = "TEXTURE/COMPASS_SHADOW_TEX_256x256";
    const NBRE_String SHADOW_ICON_128x128 = "TEXTURE/COMPASS_SHADOW_TEX_128x128";
    const NBRE_String SHADOW_ICON_64x64 = "TEXTURE/COMPASS_SHADOW_TEX_64x64";
    const NBRE_String SHADOW_ICON_32x32 = "TEXTURE/COMPASS_SHADOW_TEX_32x32";

    dayIconPath = DAY_ICON_64x64;
    nightIconPath = NIGHT_ICON_64x64;
    shadowPath = SHADOW_ICON_64x64;

    uint32 width = 0;
    uint32 height = 0;
    uint32 dpi = 0;

    PAL_Error err = PAL_DisplayGetInfo(PD_Main, &width, &height, &dpi);
    if(err == PAL_Ok)
    {
        if(width > height)
        {
            uint32 temp = width;
            width = height;
            height = temp;
        }

        //Use 256*256 image for 2K screen
        if(height >= HD_2K_HEIGHT)
        {
            dayIconPath = DAY_ICON_256x256;
            nightIconPath = NIGHT_ICON_256x256;
            shadowPath = SHADOW_ICON_256x256;
        }
        else if(width >= FULL_HD_WIDTH)
        {
            dayIconPath = DAY_ICON_128x128;
            nightIconPath = NIGHT_ICON_128x128;
            shadowPath = SHADOW_ICON_128x128;
        }
        else if (width <= WVGA_WIDTH)
        {
            dayIconPath = DAY_ICON_32x32;
            nightIconPath = NIGHT_ICON_32x32;
            shadowPath = SHADOW_ICON_32x32;
        }
    }

    dayIconPath = NBGM_MakeFullPath(dayIconPath.c_str());
    nightIconPath = NBGM_MakeFullPath(nightIconPath.c_str());
    shadowPath = NBGM_MakeFullPath(shadowPath.c_str());
}

NBRE_ShaderPtr
NBGM_Compass::CreateShader(const NBRE_String& icon, NBRE_Vector2f& iconSize)
{
    NBRE_IRenderPal* rp = mNBGMContext.renderingEngine->Context().mRenderPal;
    PAL_Instance* pal = mNBGMContext.renderingEngine->Context().mPalInstance;

    NBRE_TexturePtr texture = NBGM_BuildTextureUtility::ConstructImageTexture(*rp, pal, icon , 0, FALSE,
                                                                              NBRE_Texture::TT_2D, FALSE);
    iconSize.x = MM_TO_PIXEL(DEFAULT_COMPASS_WIDTH_MM, NBGM_GetConfig()->dpi);
    iconSize.y = MM_TO_PIXEL(DEFAULT_COMPASS_HEIGHT_MM, NBGM_GetConfig()->dpi);

    NBRE_TextureUnit unit;
    unit.SetState(NBRE_TextureState(NBRE_TFT_LINEAR, NBRE_TFT_LINEAR, NBRE_TAM_CLAMP, NBRE_TAM_CLAMP, FALSE));
    unit.SetTexture(texture);

    NBRE_PassPtr pass(NBRE_NEW NBRE_Pass());
    pass->SetEnableDepthTest(FALSE);
    pass->SetEnableDepthWrite(FALSE);
    pass->SetEnableCullFace(FALSE);
    pass->SetEnableBlend(TRUE);
    pass->SetSrcBlend(NBRE_BF_SRC_ALPHA);
    pass->SetDstBlend(NBRE_BF_ONE_MINUS_SRC_ALPHA);
    pass->GetTextureUnits().push_back(unit);

    NBRE_ShaderPtr shader = NBRE_ShaderPtr(NBRE_NEW NBRE_Shader());
    shader->AddPass(pass);

    return shader;
}

NBRE_EntityPtr
NBGM_Compass::CreateEntity(NBRE_ShaderPtr shader, const NBRE_Vector2f& size)
{
    NBRE_MeshPtr mesh(NBRE_Utility::CreateRect3D(mNBGMContext, size.x, 0.0f, size.y));
    NBRE_ModelPtr model(NBRE_NEW NBRE_Model(mesh));
    model->SetShader(shader);
    NBRE_EntityPtr entity(NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), model, mOverlayID));
    entity->SetVisible(FALSE);
    mOverlay->AppendRenderProvider(entity.get());

    return entity;
}

void
NBGM_Compass::UpdateBoundingBox2D(const NBRE_Vector2f& size, NBRE_AxisAlignedBox2f& box)
{
    float halfWidth = size.x * 0.5f;
    float halfHeight = size.y * 0.5f;

    box.minExtend.x = static_cast<float>(mPosition.x) - halfWidth;
    box.minExtend.y = static_cast<float>(mPosition.y) - halfHeight;
    box.maxExtend.x = static_cast<float>(mPosition.x) + halfWidth;
    box.maxExtend.y = static_cast<float>(mPosition.y) + halfHeight;
}

nb_boolean
NBGM_Compass::IsNorth()
{
    float angle = mRotateAngle;

    while(angle >= 360.0f)
    {
        angle -= 360.0f;
    }

    while(angle < 0.0f)
    {
        angle += 360.0f;
    }

    return NBRE_Math::AreEqual(angle, 0.f, 0.01f) || NBRE_Math::AreEqual(angle, 360.f, 0.01f);
}

void
NBGM_Compass::UpdateIcons(const NBRE_String& dayIcon, const NBRE_String& nightIcon)
{
    mUpdateIcons = TRUE;
    mIsDefault = FALSE;
    mDayIconName = dayIcon;
    mNightIconName = nightIcon;
}

void
NBGM_Compass::UpdateIcons()
{
    if(!mUpdateIcons)
    {
        return;
    }
    mUpdateIcons = FALSE;

    if (mDayEntity)
    {
        mOverlay->RemoveRenderProvider(mShadowEntity.get());
        mOverlay->RemoveRenderProvider(mDayEntity.get());
        mOverlay->RemoveRenderProvider(mNightEntity.get());
        mNode->DetachAllObjects();
        mShadowNode->DetachAllObjects();
    }

    NBRE_ShaderPtr dayShader;
    NBRE_ShaderPtr nightShader;
    NBRE_ShaderPtr shadowShader;

    if(mIsDefault)
    {
        NBRE_String shadowIcon;

        GetDefaultIconPath(mDayIconName, mNightIconName, shadowIcon);

        dayShader = CreateShader(mDayIconName, mDayIconSize);
        nightShader = CreateShader(mNightIconName, mNightIconSize);

        shadowShader = CreateShader(shadowIcon, mDayIconSize);
        mShadowEntity = CreateEntity(shadowShader, mDayIconSize);
        mShadowNode->AttachObject(mShadowEntity);
        mShadowNode->SetPosition(mPosition);
    }
    else
    {
        // remove default compass entities
        mOverlay->RemoveRenderProvider(mDayEntity.get());
        mOverlay->RemoveRenderProvider(mNightEntity.get());

        mNode->DetachAllObjects();

        if(mDayIconName == mNightIconName)
        {
            dayShader = CreateShader(mDayIconName, mDayIconSize);
            nightShader = dayShader;
            mNightIconSize = mDayIconSize;
        }
        else
        {
            dayShader = CreateShader(mDayIconName, mDayIconSize);
            nightShader = CreateShader(mNightIconName, mNightIconSize);
        }
    }

    mDayEntity = CreateEntity(dayShader, mDayIconSize);
    mNode->AttachObject(mDayEntity);
    UpdateBoundingBox2D(mDayIconSize, mDayBoundingBox);

    mNightEntity = CreateEntity(nightShader, mNightIconSize);
    mNode->AttachObject(mNightEntity);
    UpdateBoundingBox2D(mNightIconSize, mNightBoundingBox);
}


NBRE_AxisAlignedBox2f
NBGM_Compass::GetBoundingBox() const
{
    if(mEnable && mVisible)
    {
        if(mIsDayMode)
        {
            return mDayBoundingBox;
        }
        return mNightBoundingBox;
    }

    NBRE_AxisAlignedBox2f boundingBox;
    return boundingBox;
}

void
NBGM_Compass::UpdateDPI()
{
    mUpdateIcons = TRUE;
    Update();
}
