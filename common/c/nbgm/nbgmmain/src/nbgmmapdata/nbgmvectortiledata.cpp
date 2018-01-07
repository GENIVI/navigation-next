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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#include "nbgmvectortiledata.h"
#include "palstdlib.h"

static void
DestroyMercatorArray(NBRE_Array* array)
{
    if(array != NULL)
    {
        uint32 count = NBRE_ArrayGetSize(array);
        uint32 i = 0;
        for(i=0; i<count; ++i)
        {
            NBRE_Vector2d* pt = (NBRE_Vector2d*)NBRE_ArrayGetAt(array, i);

            // Don't call free(), the vector was allocated using new!
            NBRE_DELETE pt;
        }
        NBRE_ArrayDestroy(array);
    }
}

void NBRE_MapdataDestroy(NBRE_Mapdata* mapdata)
{
    if(mapdata != NULL)
    {
        uint32 count = NBRE_ArrayGetSize(mapdata->layerArray);
        uint32 i = 0;
        for(i=0; i<count; ++i)
        {
            NBRE_Layer* layer = (NBRE_Layer*)NBRE_ArrayGetAt(mapdata->layerArray, i);
            NBRE_LayerDestroy(layer);
        }
        NBRE_ArrayDestroy(mapdata->layerArray);
        nsl_free(mapdata);
    }
}

PAL_Error
NBRE_MapdataGetAssociatedLayer(NBRE_Mapdata* mapdata, NBRE_LayerInfo* layerInfo, NBRE_Layer** layer)
{
    if(mapdata == NULL || layerInfo == NULL || layer == NULL)
    {
        return PAL_ErrBadParam;
    }
    if(layerInfo->associateLayerIndex>=NBRE_ArrayGetSize(mapdata->layerArray))
    {
        return PAL_ErrOverflow;
    }
    *layer = (NBRE_Layer*)NBRE_ArrayGetAt(mapdata->layerArray, layerInfo->associateLayerIndex);
    return PAL_Ok;
}

typedef void (*LayerDataDestroyFunc)(void* data);
static LayerDataDestroyFunc
GetLayerDataDestroyFunc(NBRE_LayerInfo layerInfo)
{
    LayerDataDestroyFunc dataDestroyFunc = NULL;
    switch(layerInfo.layerDataType)
    {
    case NBRE_LDT_LPTH:
        {
            dataDestroyFunc = (LayerDataDestroyFunc)NBRE_LayerLPTHDataDestroy;
        }
        break;
    case NBRE_LDT_AREA:
        {
            dataDestroyFunc = (LayerDataDestroyFunc)NBRE_LayerAREADataDestroy;
        }
        break;
    case NBRE_LDT_TPTH:
        {
            dataDestroyFunc = (LayerDataDestroyFunc)NBRE_LayerTPTHDataDestroy;
        }
        break;
    case NBRE_LDT_PNTS:
        {
            dataDestroyFunc = (LayerDataDestroyFunc)NBRE_LayerPNTSDataDestroy;
        }
        break;
    case NBRE_LDT_MESH:
        {
            dataDestroyFunc = (LayerDataDestroyFunc)NBRE_LayerMESHDataDestroy;
        }
        break;
    case NBRE_LDT_TPSH:
        {
            dataDestroyFunc = (LayerDataDestroyFunc)NBRE_LayerTPSHDataDestroy;
        }
        break;
    case NBRE_LDT_TPAR:
        {
            dataDestroyFunc = (LayerDataDestroyFunc)NBRE_LayerTPARDataDestroy;
        }
        break;
    case NBRE_LDT_GEOM:
        {
            dataDestroyFunc = (LayerDataDestroyFunc)NBRE_LayerGEOMDataDestroy;
        }
        break;
    case NBRE_LDT_POIS:
        {
            dataDestroyFunc = (LayerDataDestroyFunc)NBRE_LayerPOISDataDestroy;
        }
        break;
    case NBRE_LDT_GPIN:
        {
            dataDestroyFunc = (LayerDataDestroyFunc)NBRE_LayerGPINDataDestroy;
        }
        break;
    case NBRE_LDT_EIDS:
        {
            dataDestroyFunc = (LayerDataDestroyFunc)NBRE_LayerEIDSDataDestroy;
        }
        break;
    case NBRE_LDT_LPDR:
        {
            dataDestroyFunc = (LayerDataDestroyFunc)NBRE_LayerLPDRDataDestroy;
        }
        break;
    default:
        dataDestroyFunc = NULL;
        break;
    }
    return dataDestroyFunc;
}

void NBRE_LayerDestroy(NBRE_Layer* layer)
{
    if(layer != NULL)
    {
        uint32 i=0;
        LayerDataDestroyFunc dataDestroyFunc = GetLayerDataDestroyFunc(layer->info);
        if(layer->data != NULL && dataDestroyFunc != NULL)
        {
            for(i=0; i<layer->dataCount; ++i)
            {
                dataDestroyFunc(layer->data[i]);
            }
            nsl_free(layer->data);
        }
        nsl_free(layer);
    }
}

void NBRE_LayerAREADataDestroy(NBRE_LayerAREAData* area)
{
    if(area != NULL)
    {
        NBRE_LayerPNTSDataDestroy(area->label);
        DestroyMercatorArray(area->polygon);
        if(area->material != NULL)
        {
            NBRE_DELETE area->material;
            area->material = NULL;
        }
        nsl_free(area);
    }
}

void NBRE_LayerLPTHDataDestroy(NBRE_LayerLPTHData* lpth)
{
    if(lpth != NULL)
    {
        DestroyMercatorArray(lpth->polyline);
        if (lpth->material != NULL)
        {
            NBRE_DELETE lpth->material;
            lpth->material = NULL;
        }
        if (lpth->elementId != NULL)
        {
            NBRE_DELETE lpth->elementId;
            lpth->elementId = NULL;
        }
        nsl_free(lpth);
    }
}

void NBRE_LayerLPDRDataDestroy(NBRE_LayerLPDRData* lpdr)
{
    if(lpdr != NULL)
    {
        DestroyMercatorArray(lpdr->polyline);
        if (lpdr->material != NULL)
        {
            NBRE_DELETE lpdr->material;
            lpdr->material = NULL;
        }
        if (lpdr->elementId != NULL)
        {
            NBRE_DELETE lpdr->elementId;
            lpdr->elementId = NULL;
        }
        nsl_free(lpdr);
    }
}

void NBRE_LayerTPTHDataDestroy(NBRE_LayerTPTHData* tpth)
{
    if(tpth != NULL)
    {
        NBRE_LayerTEXTDataDestroy(tpth->label);
        DestroyMercatorArray(tpth->polyline);
        nsl_free(tpth);
    }
}

void NBRE_LayerPNTSDataDestroy(NBRE_LayerPNTSData* pnts)
{
    if(pnts != NULL)
    {
        NBRE_LayerTEXTDataDestroy(pnts->label);
        NBRE_DELETE pnts->id;
        nsl_free(pnts);
    }
}

void NBRE_LayerMESHDataDestroy(NBRE_LayerMESHData* mesh)
{
    if(mesh != NULL)
    {
        //destroy points
        uint32 i=0;
        uint32 count = mesh->labelCount;

        //label content array
        for(i=0; i<count; ++i)
        {
            NBRE_LayerPNTSData* pnts = mesh->labelArray[i];
            NBRE_LayerPNTSDataDestroy(pnts);
        }
        if(mesh->labelArray != NULL)
        {
            nsl_free(mesh->labelArray);
        }

        NBRE_DELETE mesh->models;
        nsl_free(mesh);
    }
}

void NBRE_LayerTPSHDataDestroy(NBRE_LayerTPSHData* tpsh)
{
    if(tpsh != NULL)
    {
        NBRE_LayerTEXTDataDestroy(tpsh->label);
        DestroyMercatorArray(tpsh->polyline);
        nsl_free(tpsh);
    }
}

void NBRE_LayerTPARDataDestroy(NBRE_LayerTPARData* tpar)
{
    if(tpar != NULL)
    {
        DestroyMercatorArray(tpar->polyline);
        nsl_free(tpar);
    }
}

void NBRE_LayerGEOMDataDestroy(NBRE_LayerGEOMData* geom)
{
    if(geom != NULL)
    {
        nsl_free(geom);
    }
}

void NBRE_LayerPOISDataDestroy(NBRE_LayerPOISData* pois)
{
    if(pois != NULL)
    {
        NBRE_LayerPNTSDataDestroy(pois->point);
        if(pois->identifier != NULL)
        {
            nsl_free(pois->identifier);
        }
        NBRE_DELETE pois;
    }
}

void NBRE_LayerGPINDataDestroy(NBRE_LayerGPINData* gpin)
{
    if(gpin != NULL)
    {
        if(gpin->identifier != NULL)
        {
            nsl_free(gpin->identifier);
        }
        NBRE_DELETE gpin;
    }
}

void NBRE_LayerTEXTDataDestroy(NBRE_LayerTEXTData* text)
{
    if(text != NULL)
    {
        for(uint32 i=0; i<NBRE_ArrayGetSize(text->labelArray); ++i)
        {
            NBRE_LayerLabelData* label = (NBRE_LayerLabelData*)NBRE_ArrayGetAt(text->labelArray, i);
            if(label != NULL)
            {
                if(label->label != NULL)
                {
                    nbre_free(label->label);
                }
                nbre_free(label);
            }
        }
        NBRE_ArrayDestroy(text->labelArray);
        nbre_free(text);
    }
}

void NBRE_LayerEIDSDataDestroy(NBRE_LayerEIDSData* eids)
{
    if(eids)
    {
        NBRE_DELETE eids->mID;
        NBRE_DELETE eids;
    }
}

typedef void (*LayerInternalMaterialDestroyFunc)(void* data);
static LayerInternalMaterialDestroyFunc
GetLayerInternalMaterialDestroyFunc(NBRE_LayerInfo layerInfo)
{
    LayerInternalMaterialDestroyFunc func = NULL;
    switch(layerInfo.layerDataType)
    {
    case NBRE_LDT_LPTH:
        func = (LayerInternalMaterialDestroyFunc)NBRE_LayerLPTHDataInternalMaterialDestroy;
        break;
    case NBRE_LDT_AREA:
        func = (LayerInternalMaterialDestroyFunc)NBRE_LayerAREADataInternalMaterialDestroy;
        break;
    case NBRE_LDT_TPTH:
        func = (LayerInternalMaterialDestroyFunc)NBRE_LayerTPTHDataInternalMaterialDestroy;
        break;
    case NBRE_LDT_PNTS:
        func = (LayerInternalMaterialDestroyFunc)NBRE_LayerPNTSDataInternalMaterialDestroy;
        break;
    case NBRE_LDT_MESH:
        func = (LayerInternalMaterialDestroyFunc)NBRE_LayerMESHDataInternalMaterialDestroy;
        break;
    case NBRE_LDT_TPSH:
        func = (LayerInternalMaterialDestroyFunc)NBRE_LayerTPSHDataInternalMaterialDestroy;
        break;
    case NBRE_LDT_TPAR:
        func = (LayerInternalMaterialDestroyFunc)NBRE_LayerTPARDataInternalMaterialDestroy;
        break;
    case NBRE_LDT_GEOM:
        func = (LayerInternalMaterialDestroyFunc)NBRE_LayerGEOMDataInternalMaterialDestroy;
        break;
    case NBRE_LDT_POIS:
        func = (LayerInternalMaterialDestroyFunc)NBRE_LayerPOISDataInternalMaterialDestroy;
        break;
    case NBRE_LDT_GPIN:
        func = (LayerInternalMaterialDestroyFunc)NBRE_LayerGPINDataInternalMaterialDestroy;
        break;
    case NBRE_LDT_LPDR:
        func = (LayerInternalMaterialDestroyFunc)NBRE_LayerLPDRDataInternalMaterialDestroy;
        break;

    default:
        break;
    }
    return func;
}

void NBRE_MapdataDestoryInternalMaterial(NBRE_Mapdata* mapdata)
{
    if(mapdata != NULL)
    {
        uint32 count = NBRE_ArrayGetSize(mapdata->layerArray);
        uint32 i = 0;
        for(i=0; i<count; ++i)
        {
            NBRE_Layer* layer = (NBRE_Layer*)NBRE_ArrayGetAt(mapdata->layerArray, i);
            LayerInternalMaterialDestroyFunc func = GetLayerInternalMaterialDestroyFunc(layer->info);
            if(layer->data != NULL && func != NULL)
            {
                for(i=0; i<layer->dataCount; ++i)
                {
                    func(layer->data[i]);
                }
            }
        }
    }
}

void NBRE_LayerTEXTDataInternalMaterialDestroy(NBRE_LayerTEXTData* text)
{
    if(text != NULL && text->material != NULL)
    {
        NBRE_DELETE text->material;
    }
}

void NBRE_LayerLPTHDataInternalMaterialDestroy(NBRE_LayerLPTHData* lpth)
{
    if(lpth != NULL && lpth->material != NULL)
    {
        NBRE_DELETE lpth->material;
    }
}

void NBRE_LayerLPDRDataInternalMaterialDestroy(NBRE_LayerLPDRData* lpdr)
{
    if(lpdr != NULL && lpdr->material != NULL)
    {
        NBRE_DELETE lpdr->material;
    }
}

void NBRE_LayerAREADataInternalMaterialDestroy(NBRE_LayerAREAData* area)
{
    if(area != NULL)
    {
        NBRE_LayerPNTSDataInternalMaterialDestroy(area->label);
        NBRE_DELETE area->material;
    }
}

void NBRE_LayerTPTHDataInternalMaterialDestroy(NBRE_LayerTPTHData* tpth)
{
    if(tpth != NULL)
    {
        NBRE_LayerTEXTDataInternalMaterialDestroy(tpth->label);
    }
}

void NBRE_LayerPNTSDataInternalMaterialDestroy(NBRE_LayerPNTSData* pnts)
{
    if(pnts != NULL)
    {
        NBRE_LayerTEXTDataInternalMaterialDestroy(pnts->label);
        NBRE_DELETE pnts->material;
    }
}

void NBRE_LayerMESHDataInternalMaterialDestroy(NBRE_LayerMESHData* mesh)
{
    if(mesh != NULL)
    {
        for(uint32 i = 0; i<mesh->labelCount; ++i)
        {
            NBRE_LayerPNTSData* pnts = mesh->labelArray[i];
            NBRE_LayerPNTSDataInternalMaterialDestroy(pnts);
        }
    }
}

void NBRE_LayerTPSHDataInternalMaterialDestroy(NBRE_LayerTPSHData* tpsh)
{
    if(tpsh != NULL)
    {
        NBRE_LayerTEXTDataInternalMaterialDestroy(tpsh->label);
    }
}

void NBRE_LayerTPARDataInternalMaterialDestroy(NBRE_LayerTPARData* /*tpar*/)
{
}

void NBRE_LayerGEOMDataInternalMaterialDestroy(NBRE_LayerGEOMData* /*geom*/)
{
}

void NBRE_LayerPOISDataInternalMaterialDestroy(NBRE_LayerPOISData* pois)
{
    if(pois != NULL)
    {
        NBRE_LayerPNTSDataInternalMaterialDestroy(pois->point);
        NBRE_DELETE pois->selectedMaterial;
    }
}

void NBRE_LayerGPINDataInternalMaterialDestroy(NBRE_LayerGPINData* gpin)
{
    if(gpin != NULL)
    {
        NBRE_DELETE gpin->material;
    }
}
