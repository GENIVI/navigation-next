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
#include "nbgmpatharrowlayertile.h"
#include "nbgmbuildutility.h"
#include "nbretypeconvert.h"
#include "nbgmcommon.h"
#include "nbgmmapmaterialmanager.h"
#include "nbgmconst.h"
#include "nbgmcommon.h"

NBGM_PathArrowLayerTile::NBGM_PathArrowLayerTile(NBGM_Context& nbgmContext, const NBRE_String& id, const NBRE_Point3d& tileCenter, int32 overlayId)
    :mNBGMContext(nbgmContext), mId(id), mNode(NBRE_NEW NBRE_Node()), mOverlayId(overlayId), mIsActive(TRUE)
{
    mNode->SetPosition(NBRE_Vector3d(tileCenter.x, tileCenter.y, 0.0));
}

NBGM_PathArrowLayerTile::~NBGM_PathArrowLayerTile()
{
}


void
NBGM_PathArrowLayerTile::CreatePathArrow(NBGM_NBMLayerData* layer)
{
    if (layer == NULL)
    {
        return;
    }

    NBGM_NBMTPARLayerData* tparLayer = static_cast<NBGM_NBMTPARLayerData*> (layer);

    if(tparLayer->GetModel().get() == NULL)
    {
        return;
    }

    NBRE_Entity* entity = NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), tparLayer->GetModel(), 0);

    entity->SetOverlayId(mOverlayId);
    mNode->AttachObject(NBRE_EntityPtr(entity));

    EntityAssistInfo info(layer->GetLayerInfo().nearVisibility, layer->GetLayerInfo().farVisibility, entity);
    mEntityAssistInfoList.push_back(info);
}

#if DEBUG_PATH_ARROW_TRACK
void
NBGM_PathArrowLayerTile::CreateDebugPathArrowTrack(NBRE_Layer* layer, const NBRE_ShaderPtr& shader)
{
    if (layer == NULL)
    {
        return;
    }

    NBGM_FloatBuffer& vertexBuffer = mNBGMContext.buildModelContext->mVetexBuffer;
    NBGM_FloatBuffer& texcoordBuffer = mNBGMContext.buildModelContext->mTexcoordBuffer1;
    NBGM_ShortBuffer& indexBuffer = mNBGMContext.buildModelContext->mIndexBuffer;

    vertexBuffer.clear();
    texcoordBuffer.clear();
    indexBuffer.clear();

    for (uint32 i = 0; i < layer->dataCount; ++i)
    {
        NBRE_LayerTPARData* tparData = static_cast<NBRE_LayerTPARData*>(layer->data[i]);
        NBRE_Vector<NBRE_Point2f> polyline;
        for(uint32 j = 0; j < NBRE_ArrayGetSize(tparData->polyline); ++j)
        {
            NBRE_Point2f pos = *static_cast<NBRE_Point2f*>(NBRE_ArrayGetAt(tparData->polyline, j));
            polyline.push_back(pos);
        }
        NBGM_BuildModelUtility::Track2Tristripe(polyline, mNBGMContext.WorldToModel(ROAD_BASIC_WIDTH) * 0.05f, vertexBuffer, indexBuffer);
    }

    if (vertexBuffer.size() == 0)
    {
        return;
    }

    NBRE_MeshPtr mesh = BuildPathArrowMesh(vertexBuffer, texcoordBuffer, indexBuffer, TRUE);
    NBRE_Entity* entity = NBRE_NEW NBRE_Entity(mNBGMContext.renderingEngine->Context(), mesh, 0);

    entity->SetShader(shader);
    entity->SetOverlayId(mDrawOrder);

    mNode->AttachObject(NBRE_EntityPtr(entity));

    EntityAssistInfo info(layer->info.nearVisibility, layer->info.farVisibility, entity);
    mEntityAssistInfoList.push_back(info);
}
#endif

void
NBGM_PathArrowLayerTile::OnActive()
{
    EntityAssistInfoList::iterator i = mEntityAssistInfoList.begin();
    for(; i != mEntityAssistInfoList.end(); ++i)
    {
        i->mEntity->SetVisible(TRUE);
    }
    mIsActive = TRUE;
}

void
NBGM_PathArrowLayerTile::OnDeactive()
{
    EntityAssistInfoList::iterator i = mEntityAssistInfoList.begin();
    for(; i != mEntityAssistInfoList.end(); ++i)
    {
        i->mEntity->SetVisible(FALSE);
    }
    mIsActive = FALSE;
}

void
NBGM_PathArrowLayerTile::UpdateEntitiesVisibility( const NBRE_Planef& cameraPlane)
{
    if(!mIsActive)
    {
        return;
    }

    float distance = NBRE_Math::Abs(cameraPlane.DistanceTo(NBRE_TypeConvertf::Convert(mNode->Position())));
    distance = mNBGMContext.ModelToWorld(MERCATER_TO_METER(distance));

    EntityAssistInfoList::iterator pInfo = mEntityAssistInfoList.begin();
    EntityAssistInfoList::iterator pInfoEnd = mEntityAssistInfoList.end();

    for (; pInfo != pInfoEnd; ++pInfo)
    {
        nb_boolean isVisible = CheckNearFarVisibility(distance, pInfo->mNearVisibility, pInfo->mFarVisibility);
        pInfo->mEntity->SetVisible(isVisible);
    }
}

NBGM_PathArrowLayerTileCreator::NBGM_PathArrowLayerTileCreator(NBGM_Context& nbgmContext)
:mNBGMContext(nbgmContext)
{
}

NBGM_PathArrowLayerTileCreator::~NBGM_PathArrowLayerTileCreator()
{
}

#if DEBUG_PATH_ARROW_TRACK

NBRE_ShaderPtr
CreateArrowTrackShader(const NBRE_Color& color)
{
    NBRE_Pass* pass = NBRE_NEW NBRE_Pass();
    pass->SetEnableLighting(FALSE);
    pass->SetEnableDepthTest(TRUE);
    pass->SetEnableDepthWrite(FALSE);
    pass->SetEnableCullFace(FALSE);
    pass->SetEnableBlend(TRUE);
    pass->SetColor(color);

    NBRE_Shader* shader = NBRE_NEW NBRE_Shader();
    shader->AddPass(NBRE_PassPtr(pass));

    return NBRE_ShaderPtr(shader);
}

#endif

NBGM_IMapLayerTile*
NBGM_PathArrowLayerTileCreator::CreateMapLayerTile(const NBGM_VectorTileID& tileId, NBGM_NBMData* nbmData, NBRE_DefaultOverlayManager<DrawOrderStruct>& overlayManager, uint8 drawOrder, int32 subDrawOrder, uint8 /*labelDrawOrder*/, nb_boolean /*enablePicking*/, uint32 /*selectMask*/, const NBRE_String& /*materialCategoryName*/)
{
    // check path arrow data
    nb_boolean hasPathArrow = FALSE;

    for(NBRE_Vector<NBGM_NBMLayerData*>::const_iterator iter = nbmData->GetLayers().begin(); iter != nbmData->GetLayers().end(); ++iter)
    {
        NBGM_NBMLayerData* layer = *iter;
        if(layer->GetLayerInfo().layerType == NBRE_DLT_RoadNetwork && layer->GetLayerInfo().layerDataType == NBRE_LDT_TPAR)
        {
            hasPathArrow = TRUE;
            break;
        }
    }

    if(!hasPathArrow)
    {
        return NULL;
    }

    NBGM_PathArrowLayerTile* tileData = NBRE_NEW NBGM_PathArrowLayerTile(mNBGMContext, tileId, nbmData->GetCenter(), overlayManager.AssignOverlayId(DrawOrderStruct(drawOrder, subDrawOrder, 2)));
    for(NBRE_Vector<NBGM_NBMLayerData*>::const_iterator iter = nbmData->GetLayers().begin(); iter != nbmData->GetLayers().end(); ++iter)
    {
        NBGM_NBMLayerData* layer = *iter;
        if(layer->GetLayerInfo().layerType == NBRE_DLT_RoadNetwork && layer->GetLayerInfo().layerDataType == NBRE_LDT_TPAR)
        {
        #if DEBUG_PATH_ARROW_TRACK
            if(!mArrowTrackShader)
            {
                mArrowTrackShader = CreateArrowTrackShader(NBRE_Color(1.f, 0.f, 0.f, 0.5f));
            }
            tileData->CreateDebugPathArrowTrack(layer, mArrowTrackShader);
        #else
            tileData->CreatePathArrow(layer);
        #endif
        }
    }
    return tileData;
}
