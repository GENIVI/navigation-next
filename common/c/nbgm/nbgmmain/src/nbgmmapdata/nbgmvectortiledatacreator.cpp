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

(C) Copyright 2010 by Networks In Motion, Inc.

The information contained herein is confidential, proprietary
to Networks In Motion, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of Networks In Motion is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#include "nbgmvectortiledata.h"
#include "nbrelog.h"
#include "nbgmmapmaterialfactory.h"
#include "nbrerenderengine.h"
#include "nbgmcommon.h"
#include "nbgmmapmaterialmanager.h"
#include "nbrelinearinterpolate.h"

extern void CreateModelsFromMapdata(NBGM_ResourceContext& resourceContext, NBRE_DOM* dom, const NBRE_DOM_MESH& meshCunk, const NBRE_DOM_GEOM& geomChunk, NBRE_Vector<NBRE_ModelPtr>& models, NBRE_Layer* layer, LayerDataCrateStruct* cs);

static NBRE_Point2f*
CreatePoint2f(NBGM_ResourceContext& resourceContext, float x, float y)
{
    return NBRE_NEW NBRE_Point2f(resourceContext.WorldToModel(x), resourceContext.WorldToModel(y));
}

static float
LowPrecision(NBGM_ResourceContext& resourceContext, double /*offset*/, double /*orgOffset*/, double x)
{
    return resourceContext.WorldToModel((float)x);
}

static NBRE_Point2f*
CreateLowPrecisionPoint2d(NBGM_ResourceContext& resourceContext, NBRE_Point2d* orgOffset, NBRE_Point3d* offset, double x, double y)
{
    return NBRE_NEW NBRE_Point2f(LowPrecision(resourceContext, offset->x, orgOffset->x, x), LowPrecision(resourceContext, offset->y, orgOffset->y, y));
}

static NBRE_LayerTEXTData* CreateLabelData(LayerDataCrateStruct* cs, NBRE_DOM_TEXT* textChunk, uint16 labelIndex);
typedef PAL_Error (*LayerDataCreateFunc)(LayerDataCrateStruct* cs, NBRE_Layer** data);
static PAL_Error CreateLPTHData(LayerDataCrateStruct* cs, NBRE_Layer** data);
static PAL_Error CreateAREAData(LayerDataCrateStruct* cs, NBRE_Layer** data);
static PAL_Error CreateTPTHData(LayerDataCrateStruct* cs, NBRE_Layer** data);
static PAL_Error CreatePNTSData(LayerDataCrateStruct* cs, NBRE_Layer** data);
static PAL_Error CreateMESHData(LayerDataCrateStruct* cs, NBRE_Layer** data);
static PAL_Error CreateTPSHData(LayerDataCrateStruct* cs, NBRE_Layer** data);
static PAL_Error CreateTPARData(LayerDataCrateStruct* cs, NBRE_Layer** data);
static PAL_Error CreateGEOMData(LayerDataCrateStruct* cs, NBRE_Layer** data);
static PAL_Error CreatePOISData(LayerDataCrateStruct* cs, NBRE_Layer** data);
static PAL_Error CreateGPINData(LayerDataCrateStruct* cs, NBRE_Layer** data);
static PAL_Error CreateEIDSData(LayerDataCrateStruct* cs, NBRE_Layer** data);
static PAL_Error CreateLPDRData(LayerDataCrateStruct* cs, NBRE_Layer** data);

static LayerDataCreateFunc GetLayerDataCreateFunc(NBRE_DOM_ChunkInfo* chunkInfo);
static PAL_Error UnreferenceAssociatedLayer(NBGM_ResourceContext& resourceContext, NBRE_Mapdata* mapdata, NBRE_DOM* dom, LayerDataCrateStruct* cs);

PAL_Error
NBRE_MapdataCreate(NBGM_ResourceContext& resourceContext, NBRE_DOM* dom, const NBRE_String& materialCategoryName, NBRE_Mapdata** mapdata)
{
    //error handling
    PAL_Error err = PAL_Ok;
    uint32 errLine = INVALID_INDEX;

    //result data
    NBRE_Mapdata* result = NULL;

    //dom data
    NBRE_DOM_LAYS* layerChunk = NULL;
    NBRE_DOM_NBMF* headerChunk = NULL;

    uint32 layerIndex = 0;
    LayerDataCrateStruct cs = {0};

    if(dom == NULL || mapdata == NULL)
    {
        err = PAL_ErrBadParam;
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

    //get layer chunk array from dom
    //NOTE: for now, LAYS chunk  is singleton
    err = NBRE_DOMGetChunkById(dom, DOM_ID_LAYS, (NBRE_DOM_Chunk**)&layerChunk);
    err = (err == PAL_Ok)?NBRE_DOMGetChunkById(dom, DOM_ID_NBMF, (NBRE_DOM_Chunk**)&headerChunk):err;
    if(err != PAL_Ok)
    {
        errLine = __LINE__;
        goto HandleError;
    }

    result->layerArray = NBRE_ArrayCreate(layerChunk->count);
    if(result->layerArray == NULL)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }

    //map reference center
    result->refCenter.x = resourceContext.WorldToModel(headerChunk->refCenterX);
    result->refCenter.y = resourceContext.WorldToModel(headerChunk->refCenterY);
    result->flag = headerChunk->flag;

    cs.resourceContext = &resourceContext;
    cs.materialCategoryName = materialCategoryName;

    cs.orgOffset.x = headerChunk->refCenterX;
    cs.orgOffset.y = headerChunk->refCenterY;

    cs.offset.x = result->refCenter.x;
    cs.offset.y = result->refCenter.y;

    //for each layer
    for(layerIndex=0; layerIndex<layerChunk->count; ++layerIndex)
    {
        NBRE_DOM_Layer* domLayer = &layerChunk->layers[layerIndex];
        NBRE_DOM_Chunk* layerRefChunk = NULL;
        NBRE_Layer* layer = NULL;

        err = NBRE_DOMGetChunk(dom, domLayer->chunkOffset, &layerRefChunk);
        if(err != PAL_Ok)
        {
            NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_MapdataCreate, get chunk failed, refChunkOffset=%u, layerIndex=%u", domLayer->chunkOffset, layerIndex);
            continue;
        }

        //init create struct
        NBRE_DOM_ChunkInfo* chunkInfo = (NBRE_DOM_ChunkInfo*)layerRefChunk;
        LayerDataCreateFunc createFunc = GetLayerDataCreateFunc(chunkInfo);
        cs.dom = dom;
        cs.domChunk = layerRefChunk;
        cs.layerType = domLayer->type;

        if(createFunc != NULL && createFunc(&cs, &layer) == PAL_Ok)
        {
            //far/near original value is meter
            float nearVisibility = domLayer->nearVisibility;
            float farVisibility = domLayer->farVisibility;

            if (NBRE_Math::IsZero(nearVisibility, 1e-7f))
            {
                nearVisibility = 0.0f;
            }

            if (NBRE_Math::IsZero(farVisibility, 1e-7f))
            {
                farVisibility = 0.0f;
            }

            //No need to convert into Mercator Unit.
            layer->info.farVisibility  = farVisibility;
            layer->info.nearVisibility = nearVisibility;
            layer->info.layerType = domLayer->type;
            layer->info.associateLayerIndex = domLayer->associateLayerIndex;
            err = NBRE_ArrayAppend(result->layerArray, layer);
            if(err != PAL_Ok)
            {
                NBRE_LayerDestroy(layer);
                NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_MapdataCreate, no memory for appending created layer to array, layerIndex=%u", layerIndex);
            }
        }
    }

    err = UnreferenceAssociatedLayer(resourceContext, result, dom, &cs);
    if(err != PAL_Ok)
    {
        errLine = __LINE__;
        goto HandleError;
    }

    *mapdata = result;

    return PAL_Ok;

HandleError:
    NBRE_MapdataDestoryInternalMaterial(result);
    NBRE_MapdataDestroy(result);
    NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_MapdataCreate: err=0x%08x, errLine=%u", err, errLine);
    return err;
}

static LayerDataCreateFunc
GetLayerDataCreateFunc(NBRE_DOM_ChunkInfo* chunkInfo)
{
    switch(chunkInfo->ID)
    {
    case DOM_ID_PNTS:
        return CreatePNTSData;
    case DOM_ID_LPTH:
        return CreateLPTHData;
    case DOM_ID_AREA:
        return CreateAREAData;
    case DOM_ID_TPTH:
        return CreateTPTHData;
    case DOM_ID_MESH:
        return CreateMESHData;
    case DOM_ID_TPSH:
        return CreateTPSHData;
    case DOM_ID_GEOM:
        return CreateGEOMData;
    case DOM_ID_TPAR:
        return CreateTPARData;
    case DOM_ID_POIS:
        return CreatePOISData;
    case DOM_ID_GPIN:
        return CreateGPINData;
    case DOM_ID_EIDS:
        return CreateEIDSData;
    case DOM_ID_LPDR:
        return CreateLPDRData;
    default:
        NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_MapdataCreate: not supported layer, layer reference chunk to %d", chunkInfo->ID);
        break;
    }
    return NULL;
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

static PAL_Error
CreateMaterial(LayerDataCrateStruct* cs,
               uint16 domMaterialIndex,
               NBRE_MapMaterial** resultMaterial,
               uint16& resultMaterialID)
{
    PAL_Error err = PAL_Ok;

    if (VERIFY_NBM_INTERNAL_INDEX(domMaterialIndex))
    {
        err = NBRE_MapMaterialFactory::CreateMapMaterialFromDom(cs->resourceContext->palInstance,
                                                                *cs->resourceContext->renderPal,
                                                                *cs->resourceContext->textureManager,
                                                                *cs->dom,
                                                                domMaterialIndex,
                                                                FALSE,
                                                                resultMaterial);
    }
    else
    {
        resultMaterialID = MAKE_NBM_INDEX(domMaterialIndex);
    }

    return err;
}

static NBRE_Array*
CreateLowPrecisionPolyline(LayerDataCrateStruct* cs, double* srcPoints, uint32 count)
{
    NBRE_Array* polyline = NBRE_ArrayCreate(count);
    if(polyline == NULL)
    {
        return NULL;
    }

    for(uint32 i=0; i<count; ++i)
    {
        uint32 index = i << 1;
        NBRE_Point2f* pos = CreateLowPrecisionPoint2d(*cs->resourceContext,
                                                      &cs->orgOffset,
                                                      &cs->offset,
                                                      srcPoints[index],
                                                      srcPoints[index+1]);
        if(pos == NULL)
        {
            continue;
        }
        NBRE_ArrayAppend(polyline, pos);
    }

    return polyline;
}

static PAL_Error
CreateLPTHData(LayerDataCrateStruct* cs, NBRE_Layer** data)
{
    PAL_Error err = PAL_Ok;
    uint32 errLine = INVALID_INDEX;

    //Note: no need to check input
    NBRE_DOM* dom = cs->dom;
    NBRE_DOM_LPTH* lpthChunk = (NBRE_DOM_LPTH*)cs->domChunk;

    NBRE_Layer* result = NULL;
    NBRE_LayerLPTHData* lpth = NULL;

    NBRE_DOM_PLIN* plinChunk = NULL;

    //get pLine chunk
    err = NBRE_DOMGetChunkById(dom, DOM_ID_PLIN, (NBRE_DOM_Chunk **)&plinChunk);
    if(err != PAL_Ok)
    {
        errLine = __LINE__;
        goto HandleError;
    }

    result = CreateEmptyLayerData(NBRE_LDT_LPTH, lpthChunk->count);
    if(result == NULL)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }

    for(uint32 roadIndex = 0; roadIndex<result->dataCount; ++roadIndex)
    {
        NBRE_DOM_Polyline* domPolyline = NULL;
        NBRE_DOM_Road* domRoad = NULL;

        lpth = (NBRE_LayerLPTHData*) nsl_malloc(sizeof(NBRE_LayerLPTHData));
        if(lpth == NULL)
        {
            err = PAL_ErrNoMem;
            errLine = __LINE__;
            goto HandleError;
        }
        nsl_memset(lpth, 0, sizeof(NBRE_LayerLPTHData));

        domRoad = &lpthChunk->roads[roadIndex];

        err = CreateMaterial(cs, domRoad->material, &lpth->material, lpth->materialId);
        if (err != PAL_Ok)
        {
            errLine = __LINE__;
            goto HandleError;
        }

        //width
        lpth->width = cs->resourceContext->WorldToModel(domRoad->lineWidth);

        //polyline
        domPolyline = &plinChunk->polylines[domRoad->lineIndex];
        lpth->polyline = CreateLowPrecisionPolyline(cs,
                                                    domPolyline->locations,
                                                    domPolyline->count);
        if (lpth->polyline == NULL)
        {
            err = PAL_ErrNoMem;
            errLine = __LINE__;
            goto HandleError;
        }

        result->data[roadIndex] = lpth;

        lpth = NULL;
    }

    *data = result;
    return PAL_Ok;

HandleError:
    NBRE_LayerLPTHDataDestroy(lpth);
    NBRE_LayerDestroy(result);
    NBRE_DebugLog(PAL_LogSeverityMajor, "CreateLPTHData, err=0x%08x, errLine=%u", err, errLine);
    return err;
}

static PAL_Error
CreateLPDRData(LayerDataCrateStruct* cs, NBRE_Layer** data)
{
    uint32 errLine = INVALID_INDEX;

    NBRE_DOM_LPDR* lpdrChunk = (NBRE_DOM_LPDR*)cs->domChunk;
    NBRE_Layer* result = NULL;
    NBRE_LayerLPDRData* lpdr = NULL;
    NBRE_DOM_PLIN* plinChunk = NULL;

    PAL_Error err = NBRE_DOMGetChunkById(cs->dom, DOM_ID_PLIN, (NBRE_DOM_Chunk **)&plinChunk);
    if(err != PAL_Ok)
    {
        errLine = __LINE__;
        goto HandleError;
    }

    result = CreateEmptyLayerData(NBRE_LDT_LPDR, lpdrChunk->count);
    if(result == NULL)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }

    for(uint32 roadIndex = 0; roadIndex<result->dataCount; ++roadIndex)
    {
        NBRE_DOM_Polyline* domPolyline = NULL;
        NBRE_DOM_RoadWithDrawOrder* domRoad = NULL;

        lpdr = (NBRE_LayerLPDRData*) nsl_malloc(sizeof(NBRE_LayerLPDRData));
        if(lpdr == NULL)
        {
            err = PAL_ErrNoMem;
            errLine = __LINE__;
            goto HandleError;
        }
        nsl_memset(lpdr, 0, sizeof(NBRE_LayerLPDRData));

        domRoad = &lpdrChunk->roads[roadIndex];

        err = CreateMaterial(cs, domRoad->material, &lpdr->material, lpdr->materialId);
        if (err != PAL_Ok)
        {
            errLine = __LINE__;
            goto HandleError;
        }

        lpdr->width = cs->resourceContext->WorldToModel(domRoad->lineWidth);
        lpdr->drawOrder = domRoad->drawOrder;
        lpdr->roadFlag = domRoad->roadFlag;

        //polyline
        domPolyline = &plinChunk->polylines[domRoad->lineIndex];
        lpdr->polyline = CreateLowPrecisionPolyline(cs,
                                                    domPolyline->locations,
                                                    domPolyline->count);
        if(lpdr->polyline == NULL)
        {
            err = PAL_ErrNoMem;
            errLine = __LINE__;
            goto HandleError;
        }

        result->data[roadIndex] = lpdr;

        if ((lpdr->roadFlag & 0x1) == 0)
        {
            // Line segment to erase round joints between draw orders.
            // length = road width / 2
            // Because NCDB has some precision issues, they can't ensure the length is short enough.
            // NBGM need to adjust the end point if it's longer.
            // NCDB need ensure the order(joint point -> end point) and vertex count (=2) of this line segment.
            uint32 vs = NBRE_ArrayGetSize(lpdr->polyline);
            if (vs == 2)
            {
                NBRE_Point2f* p0 = (NBRE_Point2f*)NBRE_ArrayGetAt(lpdr->polyline, 0);
                NBRE_Point2f* p1 = (NBRE_Point2f*)NBRE_ArrayGetAt(lpdr->polyline, 1);
                float length = p0->DistanceTo(*p1);
                float r = 0.0f;
                if((lpdr->roadFlag & 0x02) == 0)
                {
                    r = lpdr->width * 0.5f;
                }
                else
                {
                    r = cs->resourceContext->WorldToModel(domRoad->distance);
                }

                if (length > r)
                {
                    *p1 = NBRE_LinearInterpolatef::Lerp(*p0, *p1, r / length);
                }
            }
            else
            {
                NBRE_DebugLog(PAL_LogSeverityCritical, "CreateLPDRData line=%d, wrong NBM data, erase polyline count = %d", __LINE__, vs);
            }
        }
        lpdr = NULL;
    }

    *data = result;
    return PAL_Ok;

HandleError:
    NBRE_LayerLPDRDataDestroy(lpdr);
    NBRE_LayerDestroy(result);
    NBRE_DebugLog(PAL_LogSeverityMajor, "CreateLPDRData, err=0x%08x, errLine=%u", err, errLine);
    return err;
}

static PAL_Error
CreateAREAData(LayerDataCrateStruct* cs, NBRE_Layer** data)
{
    PAL_Error err = PAL_Ok;
    uint32 errLine = INVALID_INDEX;

    NBRE_DOM* dom = cs->dom;
    NBRE_DOM_AREA* areaChunk = (NBRE_DOM_AREA*)cs->domChunk;

    NBRE_Layer* result = NULL;
    NBRE_LayerAREAData* area = NULL;

    NBRE_DOM_PLIN* plinChunk = NULL;

    //get pLine chunk
    err = NBRE_DOMGetChunkById(dom, DOM_ID_PLIN, (NBRE_DOM_Chunk **)&plinChunk);
    if(err != PAL_Ok)
    {
        errLine = __LINE__;
        goto HandleError;
    }

    result = CreateEmptyLayerData(NBRE_LDT_AREA, areaChunk->count);
    if(result == NULL)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }

    for(uint32 areaIndex = 0; areaIndex < areaChunk->count; ++areaIndex)
    {
        NBRE_DOM_Polyline* domPolyline = NULL;
        NBRE_DOM_Area* domArea = NULL;
        nb_boolean is2DBuilding = FALSE;

        area = (NBRE_LayerAREAData*)nsl_malloc(sizeof(NBRE_LayerAREAData));
        if (area == NULL)
        {
            err = PAL_ErrNoMem;
            errLine = __LINE__;
            goto HandleError;
        }
        nsl_memset(area, 0, sizeof(NBRE_LayerAREAData));

        domArea = &areaChunk->areas[areaIndex];

        if (VERIFY_NBM_INTERNAL_INDEX(domArea->material))
        {
            err = NBRE_MapMaterialFactory::CreateMapMaterialFromDom(cs->resourceContext->palInstance, *cs->resourceContext->renderPal, *cs->resourceContext->textureManager, *cs->dom, domArea->material, FALSE, &area->material);
            if (err != PAL_Ok)
            {
                errLine = __LINE__;
                goto HandleError;
            }

            if (area->material->Type() == NBRE_MMT_OutlinedComplexColorFillMaterial
                || area->material->Type() == NBRE_MMT_OutlinedSimpleColorFillMaterial
                || area->material->Type() == NBRE_MMT_OutlinedHoleyMaterial)
            {
                is2DBuilding = TRUE;
            }
        }
        else
        {
            area->materialId = MAKE_NBM_INDEX(domArea->material);
            const NBRE_MapMateriaGroup& materiaGroup = cs->resourceContext->mapMaterialManager->GetCurrentGroup(cs->materialCategoryName);
            const NBRE_MapMaterial* mat = &materiaGroup.GetMapMaterial(area->materialId);
            if (mat->Type() == NBRE_MMT_OutlinedComplexColorFillMaterial
                || mat->Type() == NBRE_MMT_OutlinedSimpleColorFillMaterial
                || mat->Type() == NBRE_MMT_OutlinedHoleyMaterial)
            {
                is2DBuilding = TRUE;
            }
        }

        //label
        area->labelIndex = domArea->label;

        //polygons
        domPolyline = &plinChunk->polylines[domArea->shape];

        area->polygon = NBRE_ArrayCreate(domPolyline->count);
        if(area->polygon == NULL)
        {
            err = PAL_ErrNoMem;
            errLine = __LINE__;
            goto HandleError;
        }

        for(uint32 i = 0; i < domPolyline->count; ++i)
        {
            uint32 index = i << 1;
            NBRE_Point2f* pos = NULL;
            if (is2DBuilding)
            {
                pos = CreatePoint2f(*cs->resourceContext, (float)domPolyline->locations[index], (float)domPolyline->locations[index+1]);
            }
            else
            {
                pos = CreateLowPrecisionPoint2d(*cs->resourceContext, &cs->orgOffset, &cs->offset, domPolyline->locations[index], domPolyline->locations[index+1]);
            }

            if(pos == NULL)
            {
                err = PAL_ErrNoMem;
                errLine = __LINE__;
                goto HandleError;
            }
            NBRE_ArrayAppend(area->polygon, pos);
        }
        result->data[areaIndex] = area;
        area = NULL;
    }
    *data = result;
    return PAL_Ok;

HandleError:
    NBRE_LayerAREADataDestroy(area);
    NBRE_LayerDestroy(result);
    NBRE_DebugLog(PAL_LogSeverityMajor, "CreateAREAData, err=0x%08x, errLine=%u", err, errLine);
    return err;
}

static PAL_Error
CreateTPTHData(LayerDataCrateStruct* cs, NBRE_Layer** data)
{
    PAL_Error err = PAL_Ok;
    uint32 errLine = INVALID_INDEX;

    NBRE_DOM* dom = cs->dom;
    NBRE_DOM_TPTH* tpthChunk = (NBRE_DOM_TPTH*)cs->domChunk;

    NBRE_Layer* result = NULL;
    NBRE_LayerTPTHData* tpth = NULL;

    uint32 trackIndex = 0;
    NBRE_DOM_PLIN* plinChunk = NULL;
    NBRE_DOM_TEXT* textChunk = NULL;

    if (tpthChunk->count == 0)
    {
        err = PAL_ErrBadParam;
        errLine = __LINE__;
        goto HandleError;
    }

    //get pLine chunk
    err = NBRE_DOMGetChunkById(dom, DOM_ID_PLIN, (NBRE_DOM_Chunk **)&plinChunk);
    if(err != PAL_Ok)
    {
        errLine = __LINE__;
        goto HandleError;
    }
    //get text chunk
    err = NBRE_DOMGetChunkById(dom, DOM_ID_TEXT, (NBRE_DOM_Chunk **)&textChunk);
    if(err != PAL_Ok)
    {
        errLine = __LINE__;
        goto HandleError;
    }

    result = CreateEmptyLayerData(NBRE_LDT_TPTH, tpthChunk->count);
    if(result == NULL)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }

    for(trackIndex = 0; trackIndex<tpthChunk->count; ++trackIndex)
    {
        NBRE_DOM_Polyline* domPolyline = NULL;
        NBRE_DOM_TextPath* domTpth = &tpthChunk->textPaths[trackIndex];

        uint32 i=0;

        domPolyline = &plinChunk->polylines[domTpth->line];
        if (domPolyline->count == 0)
        {
            continue;
        }

        tpth = (NBRE_LayerTPTHData*)nsl_malloc(sizeof(NBRE_LayerTPTHData));
        if(tpth == NULL)
        {
            err = PAL_ErrNoMem;
            errLine = __LINE__;
            goto HandleError;
        }
        nsl_memset(tpth, 0, sizeof(NBRE_LayerTPTHData));

        //label
        tpth->label = CreateLabelData(cs, textChunk, domTpth->label);

        //width
        tpth->width = cs->resourceContext->WorldToModel(domTpth->lineWidth);

        //pLine
        tpth->polyline = NBRE_ArrayCreate(domPolyline->count);
        if(tpth->polyline == NULL)
        {
            err = PAL_ErrNoMem;
            errLine = __LINE__;
            goto HandleError;
        }
        for(i=0; i<domPolyline->count; ++i)
        {
            NBRE_Point2f* pos = CreateLowPrecisionPoint2d(*cs->resourceContext, &cs->orgOffset, &cs->offset, domPolyline->locations[i*2], domPolyline->locations[i*2+1]);
            if(pos == NULL)
            {
                err = PAL_ErrNoMem;
                errLine = __LINE__;
                goto HandleError;
            }
            NBRE_ArrayAppend(tpth->polyline, pos);
        }
        result->data[trackIndex] = tpth;
        tpth = NULL;
    }
    *data = result;
    return PAL_Ok;
HandleError:
    NBRE_LayerTPTHDataDestroy(tpth);
    NBRE_LayerDestroy(result);
    NBRE_DebugLog(PAL_LogSeverityMajor, "CreateTPTHData, err=0x%08x, errLine=%u", err, errLine);
    return err;
}

static void
CloneLayerTEXTData(NBGM_ResourceContext& resourceContext, NBRE_DOM* dom, const NBRE_LayerTEXTData* srcData, NBRE_LayerTEXTData** desData)
{
    NBRE_LayerTEXTData* result = NULL;

    if(dom == NULL || srcData == NULL || desData == NULL || srcData->labelArray == NULL)
    {
        goto HandleError;
    }

    result = (NBRE_LayerTEXTData*)nbre_malloc(sizeof(NBRE_LayerTEXTData));
    if(result == NULL)
    {
        goto HandleError;
    }
    nsl_memset(result, 0, sizeof(NBRE_LayerTEXTData));

    result->materialId = srcData->materialId;
    result->labelArray = NBRE_ArrayCreate(NBRE_ArrayGetSize(srcData->labelArray));
    if(result->labelArray == NULL)
    {
        goto HandleError;
    }

    for(uint32 i=0; i<NBRE_ArrayGetSize(srcData->labelArray); ++i)
    {
        NBRE_LayerLabelData* oldData = (NBRE_LayerLabelData*)NBRE_ArrayGetAt(srcData->labelArray, i);
        if(oldData != NULL)
        {
            NBRE_LayerLabelData* newData = (NBRE_LayerLabelData*)nbre_malloc(sizeof(NBRE_LayerLabelData));
            if(newData != NULL)
            {
                nsl_memset(newData, 0, sizeof(NBRE_LayerLabelData));
                newData->label = nsl_strdup(oldData->label);
                newData->languageCode = oldData->languageCode;
                NBRE_ArrayAppend(result->labelArray, newData);
            }
        }
    }

    if (VERIFY_NBM_INTERNAL_INDEX(srcData->materialId))        // internal material
    {
        NBRE_MapMaterialFactory::CreateMapMaterialFromDom(resourceContext.palInstance, *resourceContext.renderPal, *resourceContext.textureManager, *dom, srcData->materialId, FALSE, &result->material);
    }

    *desData = result;

HandleError:
    *desData = NULL;
    if (result)
    {
        NBRE_LayerTEXTDataDestroy(result);
    }
}

static NBRE_LayerPNTSData*
CloneLayerPNTSData(NBGM_ResourceContext& resourceContext, NBRE_DOM* dom, const NBRE_LayerPNTSData* srcData)
{
    NBRE_LayerPNTSData* result = (NBRE_LayerPNTSData*)nsl_malloc(sizeof(NBRE_LayerPNTSData));
    if (result == NULL)
    {
        return NULL;
    }
    nsl_memset(result, 0, sizeof(NBRE_LayerPNTSData));

    result->materialId = srcData->materialId;
    result->position = srcData->position;

    if (VERIFY_NBM_INTERNAL_INDEX(srcData->materialId))
    {
        NBRE_MapMaterialFactory::CreateMapMaterialFromDom(resourceContext.palInstance, *resourceContext.renderPal, *resourceContext.textureManager, *dom, result->materialId, FALSE, &result->material);
        if (result->material == NULL)
        {
            nsl_free(result);
            return NULL;
        }
    }

    CloneLayerTEXTData(resourceContext, dom, srcData->label, &result->label);

    return result;
}

static PAL_Error
CreatePNTSData(LayerDataCrateStruct* cs, NBRE_Layer** data)
{
    PAL_Error err = PAL_Ok;
    uint32 errLine = INVALID_INDEX;
    NBRE_LayerPNTSData* pnts = NULL;
    NBRE_DOM* dom = cs->dom;
    NBRE_DOM_PNTS* pntsChunk = (NBRE_DOM_PNTS*)cs->domChunk;

    NBRE_Layer* result = CreateEmptyLayerData(NBRE_LDT_PNTS, pntsChunk->count);
    if(result == NULL)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }

    for(uint32 pntsIndex = 0; pntsIndex < pntsChunk->count; ++pntsIndex)
    {
        NBRE_DOM_Point* domPoint = &pntsChunk->points[pntsIndex];

        pnts = (NBRE_LayerPNTSData*)nsl_malloc(sizeof(NBRE_LayerPNTSData));
        if (pnts == NULL)
        {
            err = PAL_ErrNoMem;
            errLine = __LINE__;
            goto HandleError;
        }
        nsl_memset(pnts, 0, sizeof(NBRE_LayerPNTSData));

        //label string
        if (domPoint->labelIndex != INVALID_SHORT_INDEX)
        {
            NBRE_DOM_TEXT* textChunk = NULL;

            //get pLine chunk
            err = NBRE_DOMGetChunkById(dom, DOM_ID_TEXT, (NBRE_DOM_Chunk **)&textChunk);
            if(err != PAL_Ok)
            {
                errLine = __LINE__;
                goto HandleError;
            }
            pnts->label = CreateLabelData(cs, textChunk, domPoint->labelIndex);
        }
        //label position
        pnts->position.x = cs->resourceContext->WorldToModel(domPoint->location[0]);
        pnts->position.y = cs->resourceContext->WorldToModel(domPoint->location[1]);
        //label material
        pnts->materialId = domPoint->material;   // external material

        if (VERIFY_NBM_INTERNAL_INDEX(pnts->materialId))
        {
           err = NBRE_MapMaterialFactory::CreateMapMaterialFromDom(cs->resourceContext->palInstance, *cs->resourceContext->renderPal, *cs->resourceContext->textureManager, *cs->dom, pnts->materialId, FALSE, &pnts->material);
           if (err != PAL_Ok)
           {
                errLine = __LINE__;
                goto HandleError;
           }
        }

        result->data[pntsIndex] = pnts;
        pnts = NULL;
    }

    *data = result;
    return PAL_Ok;

HandleError:
    NBRE_LayerPNTSDataDestroy(pnts);
    NBRE_LayerDestroy(result);
    NBRE_DebugLog(PAL_LogSeverityMajor, "CreatePNTSData, err=0x%08x, errLine=%u", err, errLine);
    return err;
}

static PAL_Error
CreateMESHData(LayerDataCrateStruct* cs, NBRE_Layer** data)
{
    PAL_Error err = PAL_Ok;
    uint32 errLine = INVALID_INDEX;

    NBRE_Layer* result = CreateEmptyLayerData(NBRE_LDT_MESH, 1);
    NBRE_LayerMESHData* mesh = NULL;

    if(result == NULL)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }

    mesh = (NBRE_LayerMESHData*)nsl_malloc(sizeof(NBRE_LayerMESHData));
    if (mesh == NULL)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }
    nsl_memset(mesh, 0, sizeof(NBRE_LayerMESHData));
    mesh->meshChunk = cs->domChunk;
    mesh->models = NBRE_NEW NBRE_Vector<NBRE_ModelPtr>;

    result->data[0] = mesh;

    *data = result;
    return err;
HandleError:
    NBRE_LayerMESHDataDestroy(mesh);
    NBRE_LayerDestroy(result);
    NBRE_DebugLog(PAL_LogSeverityMajor, "CreateMESHData, err=0x%08x, errLine=%u", err, errLine);
    return err;
}

static void
ParseDomLabelExonym(NBRE_DOM_Label* domLabe, NBRE_Array** labelArray, uint16 flag)
{
    // Exonym format is:
    // <utf8><null><lang(uint8)><utf8><null><lang(uint8)>...
    // max 7 language now
    NBRE_Array* result = NBRE_ArrayCreate(7);
    uint32 totalLength = (flag & 0x2 ? domLabe->stringLenHigh << 8 : 0) + domLabe->stringLenLow;
    uint32 i = 0;
    char* pStr = (char*)domLabe->string;

    NBRE_String stringBuffer;
    // iterate utf8 string characters
    while ( i < totalLength)
    {
        uint8 ch = domLabe->string[i];
        if ((ch & 0xFE) == 0xFC)//1111110x
        {
            stringBuffer.append(pStr + i, 6);
            i += 6;
        }
        else if ((ch & 0xFC) == 0xF8)//111110xx
        {
            stringBuffer.append(pStr + i, 5);
            i += 5;
        }
        else if ((ch & 0xF8) == 0xF0)//11110xxx
        {
            stringBuffer.append(pStr + i, 4);
            i += 4;
        }
        else if ((ch & 0xF0) == 0xE0)//1110xxxx
        {
            stringBuffer.append(pStr + i, 3);
            i += 3;
        }
        else if ((ch & 0xE0) == 0xC0)//110xxxxx
        {
            stringBuffer.append(pStr + i, 2);
            i += 2;
        }
        else//0xxxxxxx
        {
            stringBuffer.append(pStr + i, 1);
            ++i;
            if (ch == '\0')
            {
                // end of string, read language code behind it
                NBRE_LayerLabelData* labelData = (NBRE_LayerLabelData*)nsl_malloc(sizeof(NBRE_LayerLabelData));
                if (labelData != NULL)
                {
                    nsl_memset(labelData, 0, sizeof(NBRE_LayerLabelData));
                    labelData->label = nsl_strdup(stringBuffer.c_str());
                    labelData->languageCode = domLabe->string[i];
                    NBRE_ArrayAppend(result, labelData);
                }
                ++i;
                stringBuffer.clear();
            }
        }
    }

    // Because the language code and 0 terminator are both optional.
    // There are some cases which only contain the label string.
    if (stringBuffer.length() > 0)
    {
        NBRE_LayerLabelData* labelData = (NBRE_LayerLabelData*)nsl_malloc(sizeof(NBRE_LayerLabelData));
        if (labelData != NULL)
        {
            nsl_memset(labelData, 0, sizeof(NBRE_LayerLabelData));
            labelData->label = nsl_strdup(stringBuffer.c_str());
            labelData->languageCode = 0;
            NBRE_ArrayAppend(result, labelData);
        }
    }
    *labelArray = result;
    return;
}

static NBRE_LayerTEXTData*
CreateLabelData(LayerDataCrateStruct* cs, NBRE_DOM_TEXT* textChunk, uint16 labelIndex)
{
    NBRE_LayerTEXTData* data = (NBRE_LayerTEXTData*)nbre_malloc(sizeof(NBRE_LayerTEXTData));
    nsl_memset(data, 0, sizeof(NBRE_LayerTEXTData));

    if(data != NULL && labelIndex != INVALID_SHORT_INDEX)
    {
        nsl_memset(data, 0, sizeof(NBRE_LayerTEXTData));
        NBRE_DOM_Label* domLabel = &textChunk->labels[labelIndex];
        ParseDomLabelExonym(domLabel, &data->labelArray, textChunk->flag);

        if (VERIFY_NBM_INTERNAL_INDEX(domLabel->material))
        {
            NBRE_MapMaterialFactory::CreateMapMaterialFromDom(cs->resourceContext->palInstance, *cs->resourceContext->renderPal, *cs->resourceContext->textureManager, *cs->dom, domLabel->material, FALSE, &data->material);
            if ( data->material == NULL)
            {
                NBRE_DebugLog(PAL_LogSeverityMajor, "CreateLabelData, Create internal material failed!");
                return data;
            }
        }
        data->materialId = domLabel->material;
    }

    return data;
}

static PAL_Error
CreateGEOMData(LayerDataCrateStruct* cs, NBRE_Layer** data)
{
    PAL_Error err = PAL_Ok;
    uint32 errLine = INVALID_INDEX;
    NBRE_Layer* result = NULL;

    NBRE_LayerGEOMData* geom = (NBRE_LayerGEOMData*)nsl_malloc(sizeof(NBRE_LayerGEOMData));
    if(geom == NULL)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }
    geom->geomChunk = cs->domChunk;

    result = CreateEmptyLayerData(NBRE_LDT_GEOM, 1);
    if(result == NULL)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }
    result->data[0] = geom;

    *data = result;
    return err;
HandleError:
    NBRE_LayerGEOMDataDestroy(geom);
    NBRE_LayerDestroy(result);
    NBRE_DebugLog(PAL_LogSeverityMajor, "CreateMESHData, err=0x%08x, errLine=%u", err, errLine);
    return err;
}

static PAL_Error
CreatePOISData(LayerDataCrateStruct* cs, NBRE_Layer** data)
{
    uint32 errLine = INVALID_INDEX;
    NBRE_DOM_POIS* poisChunk = (NBRE_DOM_POIS*)cs->domChunk;

    NBRE_DOM_NBMF* headerChunk = NULL;

    PAL_Error err = PAL_Ok;

    NBRE_Layer* result = CreateEmptyLayerData(NBRE_LDT_POIS, poisChunk->count);
    if(result == NULL)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }

    for(uint32 i = 0; i<poisChunk->count; ++i)
    {
        const NBRE_DOM_Poi& domPoi = poisChunk->pois[i];

        NBRE_LayerPOISData* pois = NBRE_NEW NBRE_LayerPOISData;
        if(pois == NULL)
        {
            err = PAL_ErrNoMem;
            errLine = __LINE__;
            goto HandleError;
        }
        nsl_memset(pois, 0, sizeof(NBRE_LayerPOISData));

        pois->selectedMaterialId = domPoi.selectedMaterialId;
        pois->selectedMaterial = NULL;

        if (VERIFY_NBM_INTERNAL_INDEX(pois->selectedMaterialId))
        {
            err = NBRE_MapMaterialFactory::CreateMapMaterialFromDom(cs->resourceContext->palInstance, *cs->resourceContext->renderPal, *cs->resourceContext->textureManager, *cs->dom, pois->selectedMaterialId, FALSE, &pois->selectedMaterial);
            if (err != PAL_Ok)
            {
                NBRE_LayerPOISDataDestroy(pois);
                errLine = __LINE__;
                goto HandleError;
            }
        }

        pois->pointId = domPoi.pointId;            // external material
        pois->identifier = nsl_strdup((const char *)domPoi.identifier);

        err = NBRE_DOMGetChunkById(cs->dom, DOM_ID_NBMF, (NBRE_DOM_Chunk **)&headerChunk);
        if (err != PAL_Ok)
        {
            NBRE_LayerPOISDataDestroy(pois);
            errLine = __LINE__;
            goto HandleError;
        }

        pois->tileCenterPos.x = headerChunk->refCenterX;
        pois->tileCenterPos.y = headerChunk->refCenterY;

        result->data[i] = pois;
        pois = NULL;
    }

    *data = result;
    return err;

HandleError:
    NBRE_LayerDestroy(result);
    NBRE_DebugLog(PAL_LogSeverityMajor, "CreateMESHData, err=0x%08x, errLine=%u", err, errLine);
    return err;
}

static PAL_Error
CreateGPINData(LayerDataCrateStruct* cs, NBRE_Layer** data)
{
    uint32 errLine = INVALID_INDEX;
    NBRE_DOM_GPIN* gpinChunk = (NBRE_DOM_GPIN*)cs->domChunk;
    PAL_Error err = PAL_Ok;

    NBRE_Layer* result = CreateEmptyLayerData(NBRE_LDT_GPIN, gpinChunk->count);
    if(result == NULL)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }

    for(uint32 i = 0; i < gpinChunk->count; ++i)
    {
        const NBRE_DOM_Gpin& domGpin = gpinChunk->gpins[i];

        NBRE_LayerGPINData* gpin = NBRE_NEW NBRE_LayerGPINData;
        if(gpin == NULL)
        {
            err = PAL_ErrNoMem;
            errLine = __LINE__;
            goto HandleError;
        }
        nsl_memset(gpin, 0, sizeof(NBRE_LayerGPINData));

        gpin->materialId = domGpin.materialIndex;
        gpin->material = NULL;

        if (VERIFY_NBM_INTERNAL_INDEX(gpin->materialId))
        {
            err = NBRE_MapMaterialFactory::CreateMapMaterialFromDom(cs->resourceContext->palInstance, *cs->resourceContext->renderPal, *cs->resourceContext->textureManager, *cs->dom, gpin->materialId, FALSE, &gpin->material);
            if (err != PAL_Ok)
            {
                NBRE_LayerGPINDataDestroy(gpin);
                errLine = __LINE__;
                goto HandleError;
            }
        }

        gpin->location.x = cs->resourceContext->WorldToModel(domGpin.location[0]);
        gpin->location.y = cs->resourceContext->WorldToModel(domGpin.location[1]);
        gpin->radius = cs->resourceContext->WorldToModel(METER_TO_MERCATOR((float) domGpin.radius));

        //label string
        if (domGpin.pinIdDndex != INVALID_SHORT_INDEX)
        {
            NBRE_DOM_TEXT* textChunk = NULL;
            //get pLine chunk
            err = NBRE_DOMGetChunkById(cs->dom, DOM_ID_TEXT, (NBRE_DOM_Chunk **)&textChunk);
            if(err != PAL_Ok)
            {
                NBRE_LayerGPINDataDestroy(gpin);
                errLine = __LINE__;
                goto HandleError;
            }
            NBRE_DOM_Label* domLabel = &textChunk->labels[domGpin.pinIdDndex];
            char* id = (char*) domLabel->string;
            gpin->identifier = nsl_strdup(id);
        }

        result->data[i] = gpin;
        gpin = NULL;
    }

    *data = result;
    return err;

HandleError:
    NBRE_LayerDestroy(result);
    NBRE_DebugLog(PAL_LogSeverityMajor, "CreateGPINData, err=0x%08x, errLine=%u", err, errLine);
    return err;
}

static PAL_Error
CreateTPSHData(LayerDataCrateStruct* cs, NBRE_Layer** data)
{
    PAL_Error err = PAL_Ok;
    uint32 errLine = INVALID_INDEX;
    uint32 shieldIndex = 0;

    NBRE_DOM* dom = cs->dom;
    NBRE_DOM_TPSH* tpshChunk = (NBRE_DOM_TPSH*)cs->domChunk;

    NBRE_Layer* result = NULL;
    NBRE_LayerTPSHData* tpsh = NULL;

    NBRE_DOM_PLIN* plinChunk = NULL;
    NBRE_DOM_TEXT* textChunk = NULL;

    //get pLine chunk
    err = NBRE_DOMGetChunkById(dom, DOM_ID_PLIN, (NBRE_DOM_Chunk **)&plinChunk);
    if(err != PAL_Ok)
    {
        errLine = __LINE__;
        goto HandleError;
    }
    //get text chunk
    err = NBRE_DOMGetChunkById(dom, DOM_ID_TEXT, (NBRE_DOM_Chunk **)&textChunk);
    if(err != PAL_Ok)
    {
        errLine = __LINE__;
        goto HandleError;
    }

    result = CreateEmptyLayerData(NBRE_LDT_TPSH, tpshChunk->count);
    if(result == NULL)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }

    for(shieldIndex = 0; shieldIndex<tpshChunk->count; ++shieldIndex)
    {
        NBRE_DOM_Polyline* domPolyline = NULL;
        NBRE_DOM_PathShield* domTpsh = &tpshChunk->pathShields[shieldIndex];

        uint32 i=0;

        tpsh = (NBRE_LayerTPSHData*)nsl_malloc(sizeof(NBRE_LayerTPSHData));
        if(tpsh == NULL)
        {
            err = PAL_ErrNoMem;
            errLine = __LINE__;
            goto HandleError;
        }
        nsl_memset(tpsh, 0, sizeof(NBRE_LayerTPSHData));

        //label
        tpsh->label = CreateLabelData(cs, textChunk, domTpsh->labelIndex);
        //material
        tpsh->materialId = domTpsh->shieldMaterialIndex;
        //pLine
        domPolyline = &plinChunk->polylines[domTpsh->lineIndex];
        tpsh->polyline = NBRE_ArrayCreate(domPolyline->count);
        if(tpsh->polyline == NULL)
        {
            err = PAL_ErrNoMem;
            errLine = __LINE__;
            goto HandleError;
        }
        for(i=0; i<domPolyline->count; ++i)
        {
            uint32 index = i << 1;
            NBRE_Point2f* pos = CreateLowPrecisionPoint2d(*cs->resourceContext, &cs->orgOffset, &cs->offset, domPolyline->locations[index], domPolyline->locations[index+1]);
            if(pos == NULL)
            {
                err = PAL_ErrNoMem;
                errLine = __LINE__;
                goto HandleError;
            }
            NBRE_ArrayAppend(tpsh->polyline, pos);
        }
        result->data[shieldIndex] = tpsh;
        tpsh = NULL;
    }
    *data = result;
    return PAL_Ok;
HandleError:
    NBRE_LayerTPSHDataDestroy(tpsh);
    NBRE_LayerDestroy(result);
    NBRE_DebugLog(PAL_LogSeverityMajor, "CreateLPSHData, err=0x%08x, errLine=%u", err, errLine);
    return err;
}

static PAL_Error
CreateTPARData(LayerDataCrateStruct* cs, NBRE_Layer** data)
{
    uint32 errLine = INVALID_INDEX;
    NBRE_LayerTPARData* tpar = NULL;
    NBRE_Layer* result = NULL;
    NBRE_DOM_TPAR* tparChunk = (NBRE_DOM_TPAR*)cs->domChunk;

    //get pLine chunk
    NBRE_DOM* dom = cs->dom;
    NBRE_DOM_PLIN* plinChunk = NULL;
    PAL_Error err = NBRE_DOMGetChunkById(dom, DOM_ID_PLIN, (NBRE_DOM_Chunk **)&plinChunk);
    if(err != PAL_Ok)
    {
        errLine = __LINE__;
        goto HandleError;
    }

    result = CreateEmptyLayerData(NBRE_LDT_TPAR, tparChunk->count);
    if(result == NULL)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }

    for(uint32 pathArrowIndex = 0; pathArrowIndex < tparChunk->count; ++pathArrowIndex)
    {
        NBRE_DOM_PathArrow* domTpar = &tparChunk->pathArrows[pathArrowIndex];

        tpar = (NBRE_LayerTPARData*)nsl_malloc(sizeof(NBRE_LayerTPARData));
        if(tpar == NULL)
        {
            err = PAL_ErrNoMem;
            errLine = __LINE__;
            goto HandleError;
        }
        nsl_memset(tpar, 0, sizeof(NBRE_LayerTPARData));

        //TODO: current nbm data is incorrect, the index refers to internal materials

        //if ((domTpar->materialIndex & 0x8000) == 0)
        //{
        //    NBRE_DebugLog(PAL_LogSeverityMajor, "CreateTPARData Error !! Reference to internal material.");
        //    nbre_assert(0);
        //}
        //else
        {
            tpar->materialId = domTpar->materialIndex;
        }

        //pLine
        NBRE_DOM_Polyline* domPolyline = &plinChunk->polylines[domTpar->lineIndex];
        tpar->polyline = NBRE_ArrayCreate(domPolyline->count);
        if(tpar->polyline == NULL)
        {
            err = PAL_ErrNoMem;
            errLine = __LINE__;
            goto HandleError;
        }

        for(uint32 i = 0; i < domPolyline->count; ++i)
        {
            uint32 index = i << 1;
            NBRE_Point2f* pos = CreateLowPrecisionPoint2d(*cs->resourceContext, &cs->orgOffset, &cs->offset, domPolyline->locations[index], domPolyline->locations[index + 1]);
            if(pos == NULL)
            {
                err = PAL_ErrNoMem;
                errLine = __LINE__;
                goto HandleError;
            }
            NBRE_ArrayAppend(tpar->polyline, pos);
        }
        result->data[pathArrowIndex] = tpar;
    }
    *data = result;

    return PAL_Ok;

HandleError:
    NBRE_LayerTPARDataDestroy(tpar);
    NBRE_LayerDestroy(result);
    NBRE_DebugLog(PAL_LogSeverityMajor, "CreateTPARData, err=0x%08x, errLine=%u", err, errLine);
    return err;
}

static PAL_Error
CreateEIDSData(LayerDataCrateStruct* cs, NBRE_Layer** data)
{
    uint32 errLine = INVALID_INDEX;
    NBRE_LayerEIDSData* eids = NULL;
    NBRE_DOM_EIDS* eidsChunk = (NBRE_DOM_EIDS*)cs->domChunk;
    PAL_Error err = PAL_Ok;

    NBRE_Layer* result = CreateEmptyLayerData(NBRE_LDT_EIDS, eidsChunk->count);
    if(result == NULL)
    {
        err = PAL_ErrNoMem;
        errLine = __LINE__;
        goto HandleError;
    }

    for(uint16 i = 0; i < eidsChunk->count; ++i)
    {
        NBRE_DOM_Eids* domEids = &eidsChunk->eids[i];
        eids = NBRE_NEW NBRE_LayerEIDSData;
        if(eids == NULL)
        {
            err = PAL_ErrNoMem;
            errLine = __LINE__;
            goto HandleError;
        }

        eids->mID = NBRE_NEW NBGM_ElementId((char*)(domEids->asciiString));
        if(eids->mID == NULL)
        {
            err = PAL_ErrNoMem;
            errLine = __LINE__;
            goto HandleError;
        }
        result->data[i] = eids;
    }
    *data = result;

    return PAL_Ok;

HandleError:
    NBRE_LayerEIDSDataDestroy(eids);
    NBRE_LayerDestroy(result);
    NBRE_DebugLog(PAL_LogSeverityMajor, "CreateEIDSData, err=0x%08x, errLine=%u", err, errLine);
    return err;
}

static PAL_Error
UnreferenceMeshLayer(NBGM_ResourceContext& resourceContext, NBRE_Layer* layer, NBRE_Mapdata* mapdata, NBRE_DOM* dom, LayerDataCrateStruct* cs)
{
    PAL_Error err = PAL_Ok;
    uint8 associateLayerIndex= layer->info.associateLayerIndex;
    NBRE_LayerMESHData* meshData = (NBRE_LayerMESHData*)layer->data[0];

    while(associateLayerIndex != INVALID_CHAR_INDEX && err == PAL_Ok)
    {
        NBRE_Layer* associatedLayer = (NBRE_Layer*)NBRE_ArrayGetAt(mapdata->layerArray, associateLayerIndex);
        NBRE_DOM_MESH* meshChunk = (NBRE_DOM_MESH*)meshData->meshChunk;
        if(associatedLayer->info.layerDataType == NBRE_LDT_PNTS)
        {
            uint32 i=0;
            uint32 labelCount = meshChunk->count;

            if(meshData->labelArray != NULL)
            {
                NBRE_DebugLog(PAL_LogSeverityMajor, "UnreferenceMeshLayer, mesh has more than one point associatedLayer, ignore later one");
                continue;
            }
            meshData->labelArray = (NBRE_LayerPNTSData**)nsl_malloc(sizeof(NBRE_LayerPNTSData*) * labelCount);
            if (meshData->labelArray == NULL)
            {
                NBRE_DebugLog(PAL_LogSeverityMajor, "UnreferenceMeshLayer: fail to alloc memory for mesh->labelArray");
                err = PAL_ErrNoMem;
                break;
            }
            nsl_memset(meshData->labelArray, 0, sizeof(NBRE_LayerPNTSData*) * labelCount);

            for(i=0; i<labelCount; ++i)
            {
                uint16 labelIndex = meshChunk->meshes[i].labelIndex;
                if (labelIndex != INVALID_SHORT_INDEX)
                {
                    if(labelIndex < associatedLayer->dataCount)
                    {
                        meshData->labelArray[i] = (NBRE_LayerPNTSData*)associatedLayer->data[labelIndex];
                        associatedLayer->data[labelIndex] = NULL;
                    }
                    else
                    {
                        NBRE_DebugLog(PAL_LogSeverityMajor, "UnreferenceMeshLayer, array index over flow, index=%d, areaData->labelIndex=%d, associatedLayer->dataCount=%d", i, labelIndex, associatedLayer->dataCount);
                    }
                }
            }
            meshData->labelCount = labelCount;
        }
        else if(associatedLayer->info.layerDataType == NBRE_LDT_GEOM)
        {
            NBRE_LayerGEOMData* geom = (NBRE_LayerGEOMData*)(associatedLayer->data[0]);
            NBRE_DOM_GEOM* geomChunk = (NBRE_DOM_GEOM*)(geom->geomChunk);
            NBRE_DOM_MESH* meshChunk = (NBRE_DOM_MESH*)(meshData->meshChunk);
            CreateModelsFromMapdata(resourceContext, dom, *meshChunk, *geomChunk, *meshData->models, layer, cs);
        }
        else
        {
            NBRE_DebugLog(PAL_LogSeverityMajor, "NBRE_MapdataCreate: UnreferenceMeshLayer failed, unsupported associated layer data type!");
        }

        associateLayerIndex = associatedLayer->info.associateLayerIndex;
    }
    if(err == PAL_Ok && meshData->models->size() == 0)
    {
        NBRE_DOM_MESH* meshChunk = (NBRE_DOM_MESH*)meshData->meshChunk;
        NBRE_DOM_GEOM* geomChunk = NULL;
        err = NBRE_DOMGetChunkById(dom, DOM_ID_GEOM, (NBRE_DOM_Chunk**)&geomChunk);
        if(err != PAL_Ok)
        {
            return err;
        }
        CreateModelsFromMapdata(resourceContext, dom, *meshChunk, *geomChunk, *meshData->models, layer, cs);
    }

    return err;
}

static PAL_Error
UnreferenceAreaLayer(NBRE_Layer* layer, NBRE_Mapdata* mapdata)
{
    NBRE_Layer* associatedLayer = (NBRE_Layer*)NBRE_ArrayGetAt(mapdata->layerArray, layer->info.associateLayerIndex);
    if(associatedLayer->info.layerDataType != NBRE_LDT_PNTS
        || associatedLayer->info.layerType != NBRE_DLT_Associated)
    {
        return PAL_ErrWrongFormat;
    }

    for(uint32 index=0; index<layer->dataCount; ++index)
    {
        NBRE_LayerAREAData* areaData = (NBRE_LayerAREAData*)layer->data[index];
        if(areaData->labelIndex != INVALID_SHORT_INDEX)
        {
            if (areaData->labelIndex < associatedLayer->dataCount)
            {
                areaData->label = (NBRE_LayerPNTSData*)associatedLayer->data[areaData->labelIndex];
                associatedLayer->data[areaData->labelIndex] = NULL;
            }
            else
            {
                NBRE_DebugLog(PAL_LogSeverityMajor, "UnreferenceAreaLayer, array index over flow, index=%d, areaData->labelIndex=%d, associatedLayer->dataCount=%d", index, areaData->labelIndex, associatedLayer->dataCount);
            }
        }
    }
    return PAL_Ok;
}

static PAL_Error
UnreferencePoisLayer(NBGM_ResourceContext& resourceContext, NBRE_Layer* layer, NBRE_Mapdata* mapdata, NBRE_DOM* dom)
{
    NBRE_Layer* associatedLayer = (NBRE_Layer*)NBRE_ArrayGetAt(mapdata->layerArray, layer->info.associateLayerIndex);
    if(associatedLayer->info.layerDataType != NBRE_LDT_PNTS || associatedLayer->info.layerType != NBRE_DLT_Associated)
    {
        return PAL_ErrWrongFormat;
    }

    for(uint32 index = 0; index < layer->dataCount; ++index)
    {
        NBRE_LayerPOISData* poisData = (NBRE_LayerPOISData*)layer->data[index];
        if(poisData->pointId != INVALID_SHORT_INDEX)
        {
            if (poisData->pointId < associatedLayer->dataCount)
            {
                NBRE_LayerPNTSData* data = static_cast<NBRE_LayerPNTSData*>(associatedLayer->data[poisData->pointId]);
                poisData->point = CloneLayerPNTSData(resourceContext, dom, data);
            }
            else
            {
                NBRE_DebugLog(PAL_LogSeverityMajor, "UnreferencePoisLayer, array index over flow, index=%d, poisData->pointId=%d, associatedLayer->dataCount=%d", index, poisData->pointId, associatedLayer->dataCount);
            }
        }
    }
    return PAL_Ok;
}

static PAL_Error
UnreferenceLPTHLayer(NBGM_ResourceContext& /*resourceContext*/, NBRE_Layer* layer, NBRE_Mapdata* mapdata, NBRE_DOM* /*dom*/)
{
    NBRE_Layer* associatedLayer = static_cast<NBRE_Layer*>(NBRE_ArrayGetAt(mapdata->layerArray, layer->info.associateLayerIndex));
    if(associatedLayer->info.layerDataType != NBRE_LDT_EIDS || associatedLayer->info.layerType != NBRE_DLT_Associated)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "UnreferenceEidsLayer, wrong format, associated layer type is not NBRE_LDT_EIDS.");
        return PAL_ErrWrongFormat;
    }

    if(layer->dataCount != associatedLayer->dataCount)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "UnreferenceEidsLayer, wrong format! The count of LTPH is not equal EIDS.");
        return PAL_ErrWrongFormat;
    }

    for(uint32 index = 0; index < layer->dataCount; ++index)
    {
        NBRE_LayerLPTHData* ipthData = static_cast<NBRE_LayerLPTHData*>(layer->data[index]);
        NBRE_LayerEIDSData* eidsData = static_cast<NBRE_LayerEIDSData*>(associatedLayer->data[index]);
        if(ipthData == NULL || eidsData == NULL)
        {
            continue;
        }

        ipthData->elementId = eidsData->mID;
        eidsData->mID = NULL;
    }
    return PAL_Ok;
}

static PAL_Error
UnreferenceLPDRLayer(NBGM_ResourceContext& /*resourceContext*/, NBRE_Layer* layer, NBRE_Mapdata* mapdata, NBRE_DOM* /*dom*/)
{
    NBRE_Layer* associatedLayer = static_cast<NBRE_Layer*>(NBRE_ArrayGetAt(mapdata->layerArray, layer->info.associateLayerIndex));
    if(associatedLayer->info.layerDataType != NBRE_LDT_EIDS || associatedLayer->info.layerType != NBRE_DLT_Associated)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "UnreferenceEidsLayer, wrong format, associated layer type is not NBRE_LDT_EIDS.");
        return PAL_ErrWrongFormat;
    }

    if(layer->dataCount != associatedLayer->dataCount)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "UnreferenceEidsLayer, wrong format! The count of LTPH is not equal EIDS.");
        return PAL_ErrWrongFormat;
    }

    for(uint32 index = 0; index < layer->dataCount; ++index)
    {
        NBRE_LayerLPDRData* ipdrData = static_cast<NBRE_LayerLPDRData*>(layer->data[index]);
        NBRE_LayerEIDSData* eidsData = static_cast<NBRE_LayerEIDSData*>(associatedLayer->data[index]);
        if(ipdrData == NULL || eidsData == NULL)
        {
            continue;
        }

        ipdrData->elementId = eidsData->mID;
        eidsData->mID = NULL;
    }
    return PAL_Ok;
}

static PAL_Error
UnreferenceStaticPOILayer(NBGM_ResourceContext& /*resourceContext*/, NBRE_Layer* layer, NBRE_Mapdata* mapdata, NBRE_DOM* /*dom*/)
{
    if(layer->info.layerType != NBRE_DLT_StaticPOI)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "UnreferenceStaticPOILayer, wrong format, layer type is not NBRE_DLT_StaticPOI.");
        return PAL_ErrWrongFormat;
    }

    NBRE_Layer* associatedLayer = static_cast<NBRE_Layer*>(NBRE_ArrayGetAt(mapdata->layerArray, layer->info.associateLayerIndex));
    if(associatedLayer->info.layerDataType != NBRE_LDT_EIDS || associatedLayer->info.layerType != NBRE_DLT_Associated)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "UnreferenceStaticPOILayer, wrong format, associated layer type is not NBRE_LDT_EIDS.");
        return PAL_ErrWrongFormat;
    }

    if(layer->dataCount != associatedLayer->dataCount)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "UnreferenceStaticPOILayer, wrong format! The count of PNTS is not equal EIDS.");
        return PAL_ErrWrongFormat;
    }

    for(uint32 index = 0; index < layer->dataCount; ++index)
    {
        NBRE_LayerPNTSData* pntsData = static_cast<NBRE_LayerPNTSData*>(layer->data[index]);
        NBRE_LayerEIDSData* eidsData = static_cast<NBRE_LayerEIDSData*>(associatedLayer->data[index]);
        if(pntsData == NULL || eidsData == NULL)
        {
            NBRE_DebugLog(PAL_LogSeverityMajor, "UnreferenceStaticPOILayer, pntsData or eidsData is NULL!");
            return PAL_ErrWrongFormat;
        }

        pntsData->id = eidsData->mID;
        eidsData->mID = NULL;
    }
    return PAL_Ok;
}

static PAL_Error
UnreferenceAssociatedLayer(NBGM_ResourceContext& resourceContext, NBRE_Mapdata* mapdata, NBRE_DOM* dom, LayerDataCrateStruct* cs)
{
    PAL_Error err = PAL_Ok;
    uint32 errLine = INVALID_INDEX;

    if(mapdata == NULL || mapdata->layerArray == NULL)
    {
        NBRE_DebugLog(PAL_LogSeverityMajor, "UnreferenceAssociatedLayer, Bad Param!, errLine=%u", __LINE__);
        return PAL_ErrBadParam;
    }

    uint32 count = NBRE_ArrayGetSize(mapdata->layerArray);
    for(uint32 layerIndex = 0; layerIndex < count; ++layerIndex)
    {
        NBRE_Layer* layer = (NBRE_Layer*)NBRE_ArrayGetAt(mapdata->layerArray, layerIndex);
        if(layer->info.associateLayerIndex != INVALID_CHAR_INDEX)
        {
            switch(layer->info.layerDataType)
            {
            case NBRE_LDT_MESH:
                {
                    err = UnreferenceMeshLayer(resourceContext, layer, mapdata, dom, cs);
                    errLine = __LINE__;
                }
                break;

            case NBRE_LDT_AREA:
                {
                    err = UnreferenceAreaLayer(layer, mapdata);
                    errLine = __LINE__;
                }
                break;

            case NBRE_LDT_POIS:
                {
                    err = UnreferencePoisLayer(resourceContext, layer, mapdata, dom);
                    errLine = __LINE__;
                }
                break;

             case NBRE_LDT_LPTH:
                {
                    err = UnreferenceLPTHLayer(resourceContext, layer, mapdata, dom);
                    errLine = __LINE__;
                }
                break;

                case NBRE_LDT_LPDR:
                {
                    err = UnreferenceLPDRLayer(resourceContext, layer, mapdata, dom);
                    errLine = __LINE__;
                }
                break;

             case NBRE_LDT_PNTS:
                {
                    err = UnreferenceStaticPOILayer(resourceContext, layer, mapdata, dom);
                    errLine = __LINE__;
                }
                break;

            default:
                break;
            }

            if(err != PAL_Ok)
            {
                NBRE_DebugLog(PAL_LogSeverityMajor, "UnreferenceAssociatedLayer, layerIndex=%u, err=0x%08x, errLine=%u", layerIndex, err, errLine);
            }
        }
    }

    return err;
}
