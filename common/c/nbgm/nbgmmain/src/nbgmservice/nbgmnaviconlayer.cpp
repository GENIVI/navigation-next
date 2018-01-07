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
#include "nbgmnaviconlayer.h"
#include "nbgmmapmaterialmanager.h"
#include "nbgmpointlabel.h"
#include "nbgmicon.h"
#include "nbrepngtextureimage.h"
#include "nbgmconst.h"
#include "nbgm.h"
#include "nbgmbuildutility.h"

NBGM_NavIconLayer::NBGM_NavIconLayer(NBGM_Context& nbgmContext, NBRE_Node& parentNode, NBRE_BillboardSet* billboardSet)
    :mNBGMContext(nbgmContext)
    ,mParentNode(parentNode)
    ,mBillboardSet(billboardSet)
    ,mPriority(0)
    ,mExtendType(NBGM_TMBET_NONE)
{
    for (uint32 i = 0; i < NODECOUNT; ++i)
    {
        mNode[i] = NBRE_NodePtr(NBRE_NEW NBRE_Node());
    }
}

void
NBGM_NavIconLayer::AddToSence()
{
    mNode[0]->SetPosition(NBRE_Point3f(mNBGMContext.WorldToModel(-NBRE_Math::PiTwo), 0.0f, 0.0f));
    mNode[1]->SetPosition(NBRE_Point3f(mNBGMContext.WorldToModel(0.0f), 0.0f, 0.0f));
    mNode[2]->SetPosition(NBRE_Point3f(mNBGMContext.WorldToModel(NBRE_Math::PiTwo), 0.0f, 0.0f));
    for (uint32 i = 0; i < NODECOUNT; ++i)
    {
        mParentNode.AddChild(mNode[i]);
    }
}

NBGM_NavIconLayer::~NBGM_NavIconLayer()
{
    for (uint32 i = 0; i < NODECOUNT; ++i)
    {
        mParentNode.RemoveChild(mNode[i].get());
    }
    for (uint32 i = 0; i < mIcons.size(); ++i)
    {
        NBRE_DELETE mIcons[i];
    }
}

void 
NBGM_NavIconLayer::Refresh(const PoiList& pois)
{

    NBRE_Vector2f size(DP_TO_PIXEL(POI_DEFAULT_DP_SIZE), DP_TO_PIXEL(POI_DEFAULT_DP_SIZE));

    IconList oldIcons = mIcons;
    mIcons.clear();

    for (uint32 i = 0; i < pois.size(); ++i)
    {
        NBGM_Poi* poi = pois[i];
        nb_boolean existing = FALSE;
        for (uint32 j = 0; j < oldIcons.size(); ++j)
        {
            NBGM_Icon* icon = oldIcons[j];
            //@TODO: if poi id is not unique in navigation, compare their position instead
            const NBRE_Vector3d& pos = icon->GetPosition();
            if (pos.x == poi->position.x && pos.y == poi->position.y && pos.z == poi->position.z)
            {
                mIcons.push_back(icon);
                existing = TRUE;
            }
        }

        if (!existing)
        {
            NBRE_Vector3d position(mNBGMContext.WorldToModel(poi->position.x), mNBGMContext.WorldToModel(poi->position.y), mNBGMContext.WorldToModel(poi->position.z));
            NBRE_ShaderPtr shader = CreatePoiShader(poi->image);
            // Poi is align to its center
            NBRE_Vector2f imageOffset(0, 0);
            NBRE_Vector2f bubbleOffset(0, 0);

            for (uint32 index = 0; index < NODECOUNT; ++index)
            {
                NBGM_Icon* icon = NBRE_NEW NBGM_Icon(mNBGMContext,
                                        mNode[index].get(),
                                        mBillboardSet,
                                        size,
                                        size,
                                        shader,
                                        shader,
                                        position,
                                        poi->poiId,
                                        FALSE,
                                        imageOffset,
                                        imageOffset,
                                        bubbleOffset);
                icon->SetPriority(mPriority);
                icon->AddToSence();
                mIcons.push_back(icon);
            }
        }
    }

    for (uint32 i = 0; i < oldIcons.size(); ++i)
    {
        nb_boolean existing = FALSE;
        NBGM_Icon* oldIcon = oldIcons[i];
        for (uint32 j = 0; j < mIcons.size(); ++j)
        {
            NBGM_Icon* newIcon = mIcons[j];
            if (newIcon == oldIcon)
            {
                existing = TRUE;
                break;
            }
        }
        if (!existing)
        {
            mNBGMContext.layoutManager->RemoveElement(oldIcon);
            NBRE_DELETE oldIcon;
        }
    }
}

NBRE_ShaderPtr 
NBGM_NavIconLayer::CreatePoiShader(const NBRE_String& path)
{
    NBRE_TexturePtr texture = NBGM_BuildTextureUtility::ConstructImageTexture(*mNBGMContext.renderingEngine->Context().mRenderPal, mNBGMContext.renderingEngine->Context().mPalInstance, path, 0, TRUE, NBRE_Texture::TT_2D, TRUE);

    NBRE_PassPtr pass(NBRE_NEW NBRE_Pass());
    pass->SetEnableDepthTest(FALSE);
    pass->SetEnableDepthWrite(FALSE);
    pass->SetEnableCullFace(FALSE);
    pass->SetEnableBlend(TRUE);
    NBRE_TextureUnit unit;
    unit.SetState(NBRE_TextureState(NBRE_TFT_MIPMAP_TRILINEAR, NBRE_TFT_LINEAR, NBRE_TAM_REPEAT, NBRE_TAM_REPEAT, FALSE));
    unit.SetTexture(texture);
    pass->GetTextureUnits().push_back(unit);
    NBRE_ShaderPtr shader(NBRE_NEW NBRE_Shader());
    shader->AddPass(pass);
    return shader;
}

void NBGM_NavIconLayer::Update()
{
    for (uint32 i = 0; i < mIcons.size(); ++i)
    {
        mIcons[i]->Update();
    }
}

void
NBGM_NavIconLayer::NotifyExtend(const NBGM_TileMapBoundaryExtendType& extendType)
{
    if (mExtendType != extendType)
    {
        if (mExtendType == NBGM_TMBET_RIGHT && extendType == NBGM_TMBET_LEFT)
        {
            // shift left
            NBRE_NodePtr node = mNode[0];
            mNode[0] = mNode[1];
            mNode[1] = mNode[2];
            mNode[2] = node;
        }
        if (mExtendType == NBGM_TMBET_LEFT && extendType == NBGM_TMBET_RIGHT)
        {
            // shift right
            NBRE_NodePtr node = mNode[2];
            mNode[2] = mNode[1];
            mNode[1] = mNode[0];
            mNode[0] = node;
        }
        mExtendType = extendType;
        mNode[0]->SetPosition(NBRE_Point3f(mNBGMContext.WorldToModel(-NBRE_Math::PiTwo), 0.0f, 0.0f));
        mNode[1]->SetPosition(NBRE_Point3f(mNBGMContext.WorldToModel(0.0f), 0.0f, 0.0f));
        mNode[2]->SetPosition(NBRE_Point3f(mNBGMContext.WorldToModel(NBRE_Math::PiTwo), 0.0f, 0.0f));
    }
}

void
NBGM_NavIconLayer::UpdateLayoutList(NBGM_LayoutElementList& layoutElements)
{
    NBGM_LayoutManager* layoutManager = mNBGMContext.layoutManager;
    for (uint32 i = 0; i < mIcons.size(); ++i)
    {
        NBGM_Icon* icon = mIcons[i];
        if (layoutManager->IsInFrustum(icon->GetAABB()) 
            && layoutManager->IsInFOV(icon->GetAABB()))
        {
            layoutElements.push_back(mIcons[i]);
        }
    }
}
