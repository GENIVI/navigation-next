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
#include "nbgmpoilayertile.h"
#include "nbgmmapmaterialmanager.h"
#include "nbgmicon.h"
#include "nbgmconst.h"
#include "nbgm.h"

NBGM_PoiLayerTile::NBGM_PoiLayerTile()
    :mNBGMContext(NULL)
    ,mBillboardSet(NULL)
    ,mNode(NBRE_NEW NBRE_Node)
    ,mSubNode(NBRE_NEW NBRE_Node())
    ,mPriority(0)
{
    mNode->AddChild(mSubNode);
}

NBGM_PoiLayerTile::NBGM_PoiLayerTile(NBGM_Context* nbgmContext, const NBRE_String& id, NBRE_BillboardSet* billboardSet, int32 priority)
    :mNBGMContext(nbgmContext)
    ,mBillboardSet(billboardSet)
    ,mId(id)
    ,mNode(NBRE_NEW NBRE_Node())
    ,mSubNode(NBRE_NEW NBRE_Node())
    ,mPriority(priority)
    ,mIsActive(TRUE)
{
    mNode->AddChild(mSubNode);
}

NBGM_PoiLayerTile::~NBGM_PoiLayerTile()
{
    if (mSubNode)
    {
        mNode->RemoveChild(mSubNode.get());
    }

    for (NBGM_LayoutElementList::iterator i = mElements.begin(); i != mElements.end(); ++i)
    {
        NBRE_DELETE *i;
    }
}

void
NBGM_PoiLayerTile::CreateTilePoi(NBGM_NBMLayerData* layer, const NBRE_Point3d& refCenter)
{
    NBGM_NBMPOISLayerData* poisLayer = static_cast<NBGM_NBMPOISLayerData*> (layer);
    for (NBRE_Vector<NBGM_PoiData*>::const_iterator iter = poisLayer->GetPois().begin(); iter != poisLayer->GetPois().end(); ++iter)
    {
        NBGM_PoiData* poiData = *iter;
        if(poiData == NULL || poiData->pointLabel == NULL)
        {
            continue;
        }

        NBRE_Vector3d position = NBRE_Vector3d(poiData->pointLabel->position.x + refCenter.x, poiData->pointLabel->position.y + refCenter.y, refCenter.z);
        if(poiData->pointLabel->material->Type() != NBRE_MMT_PinMaterial)
        {
            NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM_PoiLayerTile::CreateTilePoi, wrong material poiData->pointLabel->material->type = %d!", poiData->pointLabel->material->Type());
            continue;
        }
        if(poiData->selectedMaterial->Type() != NBRE_MMT_PinMaterial)
        {
            NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM_PoiLayerTile::CreateTilePoi, wrong material poiData->selectedMaterial->type = %d!", poiData->selectedMaterial->Type());
            continue;
        }
        const NBRE_PinMaterial* normalMaterial = static_cast<const NBRE_PinMaterial*> (poiData->pointLabel->material);
        const NBRE_PinMaterial* selectedMaterial = static_cast<const NBRE_PinMaterial*> (poiData->selectedMaterial);

        nbre_assert(normalMaterial!=NULL);
        nbre_assert(selectedMaterial!=NULL);

        NBRE_ShaderPtr normalShader(normalMaterial->GetShader(0));
        NBRE_ShaderPtr selectedShader(selectedMaterial->GetShader(0));
        NBRE_Point2f imageOffset(normalMaterial->XImageOffset(), normalMaterial->YImageOffset());
        NBRE_Point2f bubbleOffset(normalMaterial->XBubbleOffset(), normalMaterial->YBubbleOffset());
        bubbleOffset -= imageOffset;
        imageOffset.x = imageOffset.x-0.5f;
        imageOffset.y = 0.5f-imageOffset.y;

        imageOffset.x *= -1.f;
        imageOffset.y *= -1.f;
        bubbleOffset.x *= -1.f;
        bubbleOffset.y *= -1.f;

        // Offset from the center point, unit is percentage
        // Y axis is from bottom to up( OpenGL coordinates )
        NBRE_TexturePtr tex = normalShader->GetPass(0)->GetTextureUnits()[0].GetTexture(mNBGMContext->renderingEngine->Context());
        nbre_assert(tex);
        float sizeFactor = DP_TO_PIXEL(RATIO_FOR_PX_TO_DP);
        NBRE_Vector2f size((float)tex->GetOriginalWidth() * sizeFactor,
            (float)tex->GetOriginalHeight() * sizeFactor);

        NBRE_TexturePtr texSelected = selectedShader->GetPass(0)->GetTextureUnits()[0].GetTexture(mNBGMContext->renderingEngine->Context());
        nbre_assert(texSelected);
        NBRE_Vector2f selectedTexSize((float)texSelected->GetOriginalWidth() * sizeFactor,
            (float)texSelected->GetOriginalHeight() * sizeFactor);

        NBGM_Icon* icon = NBRE_NEW NBGM_Icon(*mNBGMContext,
                                             mSubNode.get(),
                                             mBillboardSet,
                                             size,
                                             selectedTexSize,
                                             normalShader,
                                             selectedShader,
                                             position,
                                             poiData->id,
                                             TRUE,
                                             imageOffset,
                                             imageOffset,
                                             bubbleOffset);
        icon->SetPriority(mPriority);
        mElements.push_back(icon);
    }
}

void
NBGM_PoiLayerTile::CreateTileGPin(NBGM_NBMLayerData* layer, const NBRE_Point3d& refCenter)
{
    NBGM_NBMGPINLayerData* gpinLayer = static_cast<NBGM_NBMGPINLayerData*> (layer);
    if(gpinLayer == NULL)
    {
        return;
    }

    for (NBRE_Vector<NBGM_GpinData*>::const_iterator iter = gpinLayer->GetGpins().begin(); iter != gpinLayer->GetGpins().end(); ++iter)
    {
        NBGM_GpinData* gpinData = *iter;
        if (gpinData == NULL || gpinData->material == NULL)
        {
            continue;
        }

        if(gpinData->material->Type() != NBRE_MMT_GPINMaterial)
        {
            continue;
        }

        const NBRE_RadialPINMaterial* material = static_cast<const NBRE_RadialPINMaterial*> (gpinData->material);
        NBRE_Vector3d position(gpinData->position.x + refCenter.x,
                               gpinData->position.y + refCenter.y,
                               refCenter.z);
        NBRE_ShaderPtr normalShader = material->NormalShader();
        NBRE_ShaderPtr selectedShader = material->SelectedShader();
        NBRE_Point2f imageOffset(material->NormalXImageOffset(), material->NormalYImageOffset());
        NBRE_Point2f selectedImageOffset(material->SelectedXImageOffset(), material->SelectedYImageOffset());
        NBRE_Point2f bubbleOffset(material->SelectedXBubbleOffset(), material->SelectedYBubbleOffset());

        bubbleOffset -= selectedImageOffset;

        imageOffset.x = imageOffset.x - 0.5f;
        imageOffset.y = 0.5f - imageOffset.y;
        imageOffset.x *= -1.f;
        imageOffset.y *= -1.f;

        selectedImageOffset.x = selectedImageOffset.x - 0.5f;
        selectedImageOffset.y = 0.5f - selectedImageOffset.y;
        selectedImageOffset.x *= -1.f;
        selectedImageOffset.y *= -1.f;

        bubbleOffset.x *= -1.f;
        bubbleOffset.y *= -1.f;

        NBRE_Vector2f size(material->GetNormalWidth(), material->GetNormalHeight());
        NBRE_Vector2f selectedTexSize(material->GetSelectedWidth(), material->GetSelectedHeight());

        NBGM_Icon* icon = NBRE_NEW NBGM_Icon(*mNBGMContext,
                                             mSubNode.get(),
                                             mBillboardSet,
                                             size,
                                             selectedTexSize,
                                             normalShader,
                                             selectedShader,
                                             position,
                                             gpinData->id,
                                             TRUE,
                                             imageOffset,
                                             selectedImageOffset,
                                             bubbleOffset);
        icon->InitializeHalo(material->NormalHaloInteriorShader(), material->NormalHaloOutlineShader(), gpinData->radius);
        icon->SetPriority(mPriority);
        mElements.push_back(icon);
    }
}

void
NBGM_PoiLayerTile::CreateTileItems(NBGM_NBMLayerData* layer, const NBRE_Point3d& refCenter)
{
    if(layer->GetLayerInfo().layerType == NBRE_DLT_Associated && layer->GetLayerInfo().layerDataType != NBRE_LDT_TPSH)
    {
        return;
    }

    switch(layer->GetLayerInfo().layerDataType)
    {
    case NBRE_LDT_POIS:
        CreateTilePoi(layer, refCenter);
        break;
    case NBRE_LDT_GPIN:
        CreateTileGPin(layer, refCenter);
        break;
    default:
        break;
    }
}

void
NBGM_PoiLayerTile::OnActive()
{
    for (NBGM_LayoutElementList::iterator i = mElements.begin(); i != mElements.end(); ++i)
    {
        NBGM_LayoutElement* elem = *i;
        elem->SetEnable(TRUE);
        elem->SetVisible(TRUE);
    }
    mIsActive = TRUE;
}

void
NBGM_PoiLayerTile::OnDeactive()
{
    for (NBGM_LayoutElementList::iterator i = mElements.begin(); i != mElements.end(); ++i)
    {
        NBGM_LayoutElement* elem = *i;
        elem->SetEnable(FALSE);
        elem->SetVisible(FALSE);
    }
    mIsActive = FALSE;
}

nb_boolean
NBGM_PoiLayerTile::SelectIcon(const NBRE_String& id, nb_boolean isSelected)
{
    if (!mIsActive)
    {
        return FALSE;
    }

    nb_boolean result = FALSE;
    for (NBGM_LayoutElementList::iterator i = mElements.begin(); i != mElements.end(); ++i)
    {
        NBGM_Icon* elem = (NBGM_Icon*)*i;
        if (elem->GetId() == id)
        {
            elem->SetSelected(isSelected);
            result = TRUE;
        }
    }
    return result;
}

void
NBGM_PoiLayerTile::UnselectAll()
{
    if (!mIsActive)
    {
        return;
    }

    for (NBGM_LayoutElementList::iterator i = mElements.begin(); i != mElements.end(); ++i)
    {
        NBGM_Icon* elem = (NBGM_Icon*)*i;
        elem->SetSelected(FALSE);
    }
}

nb_boolean
NBGM_PoiLayerTile::GetIconPosition(const NBRE_String& id, NBRE_Point2f& pos)
{
    if (!mIsActive)
    {
        return FALSE;
    }

    nb_boolean result = FALSE;
    for (NBGM_LayoutElementList::iterator i = mElements.begin(); i != mElements.end(); ++i)
    {
        NBGM_Icon* elem = (NBGM_Icon*)*i;
        if (elem->GetId() == id)
        {
            if(elem->GetScreenPosition(pos))
            {
                result = TRUE;
                break;
            }
        }
    }
    return result;
}

nb_boolean
NBGM_PoiLayerTile::GetIconBubblePosition(const NBRE_String& id, NBRE_Point2f& pos)
{
    if (!mIsActive)
    {
        return FALSE;
    }

    nb_boolean result = FALSE;
    for (NBGM_LayoutElementList::iterator i = mElements.begin(); i != mElements.end(); ++i)
    {
        NBGM_Icon* elem = (NBGM_Icon*)*i;
        if (elem->GetId() == id)
        {
            if(elem->GetScreenBubblePosition(pos))
            {
                result = TRUE;
                break;
            }
        }
    }
    return result;
}

void
NBGM_PoiLayerTile::Refresh(float skyHeight)
{
    if(mIsActive)
    {
        NBRE_Point2f pos;
        for (NBGM_LayoutElementList::iterator i = mElements.begin(); i != mElements.end(); ++i)
        {
            NBGM_Icon* elem = static_cast<NBGM_Icon*>(*i);
            if(elem->GetScreenBottomPosition(pos))
            {
                if(pos.y > skyHeight)
                {
                    elem->SetVisible(FALSE);

                }
                else
                {
                    elem->SetVisible(TRUE);

                }
            }
        }

    }
}

void
NBGM_PoiLayerTile::GetIconsAt(const NBRE_Point2f& screenPosition, NBRE_Vector<NBRE_String>& icons)
{
    if (!mIsActive)
    {
        return;
    }

    for (NBGM_LayoutElementList::iterator i = mElements.begin(); i != mElements.end(); ++i)
    {
        NBGM_Icon* elem = (NBGM_Icon*)*i;
        if (elem->HitTest(screenPosition))
        {
            icons.push_back(elem->GetId());
        }
    }
}

void
NBGM_PoiLayerTile::AddToScene()
{
    for (NBGM_LayoutElementList::iterator i = mElements.begin(); i != mElements.end(); ++i)
    {
        NBGM_LayoutElement* elem = *i;
        elem->AddToSence();
    }
}

NBRE_NodePtr
NBGM_PoiLayerTile::Node()
{
    return mNode;
}

void
NBGM_PoiLayerTile::Update()
{
    if (!mIsActive)
    {
        return;
    }

    for (NBGM_LayoutElementList::iterator i = mElements.begin(); i != mElements.end(); ++i)
    {
        NBGM_Icon* elem = (NBGM_Icon*)*i;
        elem->Update();
    }
}

void
NBGM_PoiLayerTile::NotifyExtend(const NBGM_TileMapBoundaryExtendType& extendType)
{
    if (!mIsActive)
    {
        return;
    }

    for (NBGM_LayoutElementList::iterator i = mElements.begin(); i != mElements.end(); ++i)
    {
        NBGM_Icon* elem = (NBGM_Icon*)*i;
        elem->NotifyExtend(extendType);
    }
}

NBGM_PoiLayerTileCreator::NBGM_PoiLayerTileCreator(NBGM_Context& nbgmContext, NBRE_BillboardSet* billboardSet, int32* priority)
    :mNBGMContext(nbgmContext)
    ,mBillboardSet(billboardSet)
    ,mPriority(priority)
{
}

NBGM_PoiLayerTileCreator::~NBGM_PoiLayerTileCreator()
{
}

NBGM_IMapLayerTile*
NBGM_PoiLayerTileCreator::CreateMapLayerTile(const NBGM_VectorTileID& tileId, NBGM_NBMData* nbmData, NBRE_DefaultOverlayManager<DrawOrderStruct>& /*overlayManager*/, uint8 /*drawOrder*/, int32 /*subDrawOrder*/, uint8 /*labelDrawOrder*/, nb_boolean /*enablePicking*/, uint32 /*selectMask*/, const NBRE_String& /*materialCategoryName*/)
{
    // check poi data
    nb_boolean hasPOI = FALSE;

    for(NBRE_Vector<NBGM_NBMLayerData*>::const_iterator iter = nbmData->GetLayers().begin(); iter != nbmData->GetLayers().end(); ++iter)
    {
        NBGM_NBMLayerData* layer = *iter;
        if(layer->GetLayerInfo().layerDataType == NBRE_LDT_POIS || layer->GetLayerInfo().layerDataType == NBRE_LDT_GPIN)
        {
            hasPOI = TRUE;
            break;
        }
    }

    if(hasPOI)
    {
        NBGM_PoiLayerTile* tileData = NBRE_NEW NBGM_PoiLayerTile(&mNBGMContext, tileId, mBillboardSet, *mPriority);
        for(NBRE_Vector<NBGM_NBMLayerData*>::const_iterator iter = nbmData->GetLayers().begin(); iter != nbmData->GetLayers().end(); ++iter)
        {
            NBGM_NBMLayerData* layer = *iter;
            tileData->CreateTileItems(layer, nbmData->GetCenter());
        }

        return tileData;
    }

    return NULL;
}
