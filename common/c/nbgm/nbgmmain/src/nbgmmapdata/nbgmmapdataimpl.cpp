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
#include "nbgmmapdataimpl.h"
#include "nbgmnbmdata.h"
#include "nbgmvectortilebuilder.h"
#include "nbgmmapmaterial.h"
#include "nbgmcommon.h"
#include "nbgmconst.h"
#include "nbgmvectorvertexscale.h"
#include "nbgmtypes.h"

NBGM_MapDataImpl::NBGM_MapDataImpl(const NBGM_Point2d& refCenter)
    :mRefCenter(refCenter)
{
}

NBGM_MapDataImpl::~NBGM_MapDataImpl()
{
    for (uint32 i = 0; i < mLayers.size(); ++i)
    {
        NBRE_DELETE mLayers[i];
    }
}

PAL_Error NBGM_MapDataImpl::CreateNBREMapData(NBGM_ResourceContext& resourceContext, const NBRE_String& nbmDataId, NBRE_Mapdata** mapdata)
{
    //error handling
    PAL_Error err = PAL_Ok;
    uint32 errLine = INVALID_INDEX;

    //result data
    NBRE_Mapdata* result = NULL;

    uint32 layerCount = mLayers.size();
    if (layerCount == 0)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }

    //create result
    result = (NBRE_Mapdata*)nsl_malloc(sizeof(NBRE_Mapdata));
    if(result == NULL)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }
    nsl_memset(result, 0, sizeof(NBRE_Mapdata));

    result->layerArray = NBRE_ArrayCreate(layerCount);
    if(result->layerArray == NULL)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }

    //map reference center
    result->refCenter.x = resourceContext.WorldToModel(mRefCenter.x);
    result->refCenter.y = resourceContext.WorldToModel(mRefCenter.y);

    //for each layer
    for(uint32 layerIndex = 0; layerIndex < layerCount; ++layerIndex)
    {
        NBGM_IMapDataLayer* layerData = mLayers[layerIndex];
        NBRE_Layer* layer = NULL;
        if(layerData->CreateLayerData(resourceContext, nbmDataId, &layer) == PAL_Ok)
        {
            err = NBRE_ArrayAppend(result->layerArray, layer);
            if(err != PAL_Ok)
            {
                NBRE_LayerDestroy(layer);
                NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM_MapDataImpl::CreateNBREMapData, no memory for appending created layer to array, layerIndex=%u", layerIndex);
            }
        }
    }
    *mapdata = result;
    return PAL_Ok;

HandleError:
    NBRE_MapdataDestoryInternalMaterial(result);
    NBRE_MapdataDestroy(result);
    NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM_MapDataImpl::CreateNBREMapData: err=0x%08x, errLine=%u", err, errLine);
    return err;
}

NBGM_IPolylineLayer* 
NBGM_MapDataImpl::AddPolylineLayer(float nearVisibility, float farVisibility)
{
    NBGM_PolylineLayerImpl* layer = NBRE_NEW NBGM_PolylineLayerImpl(nearVisibility, farVisibility);
    mLayers.push_back(layer);
    return layer;
}

NBGM_PolylineLayerImpl::PolylineData::PolylineData(const NBGM_ElementId& id, vector<NBGM_Point2d>& pts, float width, uint16 exteralMateral, NBGM_IMapMaterial* material)
    :id(id)
    ,points(pts)
    ,width(width)
    ,exteralMateral(exteralMateral)
    ,material((NBGM_MapMaterialImpl*)material)
{
}

NBGM_PolylineLayerImpl::PolylineData::~PolylineData()
{
    if (material)
    {
        NBRE_DELETE material;
    }
}

NBGM_PolylineLayerImpl::NBGM_PolylineLayerImpl(float nearVisibility, float farVisibility)
:nearVisibility(nearVisibility)
,farVisibility(farVisibility)
{
}

NBGM_PolylineLayerImpl::~NBGM_PolylineLayerImpl()
{
    for (uint32 i = 0; i < mPolylines.size(); ++i)
    {
        NBRE_DELETE mPolylines[i];
    }
}

static NBRE_Layer*
CreateEmptyLayerData(NBRE_LayerDataType layerDataType, uint32 dataCount)
{
    NBRE_Layer* result = (NBRE_Layer*)nsl_malloc(sizeof(NBRE_Layer));
    if(result == NULL)
    {
        return NULL;
    }
    nsl_memset(result, 0, sizeof(NBRE_Layer));

    result->data = (void **)nsl_malloc(sizeof(void*) * dataCount);
    if(result->data == NULL)
    {
        nsl_free(result);
        return NULL;
    }
    nsl_memset(result->data, 0, sizeof(void*)*dataCount);
    result->dataCount = dataCount;
    result->info.layerDataType = layerDataType;

    return result;
}

static NBRE_Point2f*
CreatePoint2f(NBGM_ResourceContext& resourceContext, float x, float y)
{
    return NBRE_NEW NBRE_Point2f(resourceContext.WorldToModel(x), resourceContext.WorldToModel(y));
}

PAL_Error
NBGM_PolylineLayerImpl::CreateLayerData(NBGM_ResourceContext& resourceContext, const NBRE_String& nbmDataId, NBRE_Layer** data)
{
    PAL_Error err = PAL_Ok;
    uint32 errLine = INVALID_INDEX;

    NBRE_Layer* result = NULL;
    NBRE_LayerLPTHData* lpth = NULL;
    uint32 plCount = mPolylines.size();

    result = CreateEmptyLayerData(NBRE_LDT_LPTH, plCount);
    if(result == NULL)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }

    for (uint32 plIdx = 0; plIdx < plCount; ++plIdx)
    {
        PolylineData* plData = mPolylines[plIdx];

        lpth = (NBRE_LayerLPTHData*) nsl_malloc(sizeof(NBRE_LayerLPTHData));
        if(lpth == NULL)
        {
            err = PAL_ErrNoMem;
            errLine = __LINE__;
            goto HandleError;
        }
        nsl_memset(lpth, 0, sizeof(NBRE_LayerLPTHData));

        if (plData->material != NULL)
        {
            lpth->material = plData->material->CreateNBREMaterial(resourceContext.palInstance, *resourceContext.renderPal, *resourceContext.textureManager, nbmDataId, FALSE);
        }
        else
        {
            lpth->materialId = MAKE_NBM_INDEX(plData->exteralMateral);
        }

        //width
        lpth->width = resourceContext.WorldToModel(plData->width);
        lpth->elementId = NBRE_NEW NBGM_ElementId(plData->id);
        //polyline
        lpth->polyline = NBRE_ArrayCreate(plData->points.size());
        if(lpth->polyline == NULL)
        {
            err = PAL_ErrNoMem;
            errLine = __LINE__;
            goto HandleError;
        }
        for(uint32 i = 0; i < plData->points.size(); ++i)
        {
            NBRE_Point2f* pos = CreatePoint2f(resourceContext, plData->points[i].x, plData->points[i].y);
            if(pos == NULL)
            {
                err = PAL_ErrNoMem;
                errLine = __LINE__;
                goto HandleError;
            }
            NBRE_ArrayAppend(lpth->polyline, pos);
        }
        result->data[plIdx] = lpth;
        lpth = NULL;
    }

    //far/near original value is meter
    if (NBRE_Math::IsZero(nearVisibility, 1e-7f))
    {
        nearVisibility = 0.0f;
    }

    if (NBRE_Math::IsZero(farVisibility, 1e-7f))
    {
        farVisibility = 0.0f;
    }

    //No need to convert into Mercator Unit.
    result->info.farVisibility  = farVisibility;
    result->info.nearVisibility = nearVisibility;
    result->info.layerDataType = NBRE_LDT_LPTH;
    result->info.layerType = NBRE_DLT_Route;
    result->info.associateLayerIndex = 0;

    *data = result;
    return PAL_Ok;

HandleError:
    NBRE_LayerLPTHDataDestroy(lpth);
    NBRE_LayerDestroy(result);
    NBRE_DebugLog(PAL_LogSeverityMajor, "NBGM_PolylineLayerImpl::CreateLayerData, err=0x%08x, errLine=%u", err, errLine);
    return err;
}

void 
NBGM_PolylineLayerImpl::AddPolyline(const std::string& id, vector<NBGM_Point2d>& pts, float width, uint16 exteralMateral )
{
    mPolylines.push_back(NBRE_NEW PolylineData(id, pts, width, exteralMateral, NULL));
}

void 
NBGM_PolylineLayerImpl::AddPolyline(const std::string& id, vector<NBGM_Point2d>& pts, float width, NBGM_IMapMaterial* material)
{
    mPolylines.push_back(NBRE_NEW PolylineData(id, pts, width, 0, material));
}

NBGM_StandardColorMapMaterialImpl::NBGM_StandardColorMapMaterialImpl(uint32 color)
    :mColor(color)
{
}

NBGM_StandardColorMapMaterialImpl::~NBGM_StandardColorMapMaterialImpl() 
{
}

NBRE_MapMaterial* NBGM_StandardColorMapMaterialImpl::CreateNBREMaterial(PAL_Instance* palInstance, NBRE_IRenderPal& renderPal, NBRE_TextureManager& textureManager, const NBRE_String& nbmDataId, nb_boolean isExternalMaterial)
{
    return NBRE_MapStandardColorMaterial::CreateMemoryInstance(palInstance, renderPal, textureManager, nbmDataId.c_str(), mColor, GetId(), isExternalMaterial);
}
