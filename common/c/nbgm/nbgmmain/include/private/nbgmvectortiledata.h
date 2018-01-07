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

/*!--------------------------------------------------------------------------

    @file nbgmvectortiledata.h

*/
/*
(C) Copyright 2011 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _NBRE_VECTORTILEDATA_H_
#define _NBRE_VECTORTILEDATA_H_

#include "nbretypes.h"
#include "nbgmarray.h"
#include "nbgmdom.h"
#include "nbrevector2.h"
#include "nbrevector3.h"
#include "nbreentity.h"
#include "palerror.h"
#include "nbgmmapmaterial.h"
#include "nbgmcontext.h"

typedef NBRE_String NBGM_ElementId;

typedef enum NBRE_LayerDataType
{
    NBRE_LDT_LPTH = 0,
    NBRE_LDT_AREA,
    NBRE_LDT_TPTH,
    NBRE_LDT_PNTS,
    NBRE_LDT_MESH,
    NBRE_LDT_TPSH,
    NBRE_LDT_TPAR,
    NBRE_LDT_GEOM,
    NBRE_LDT_POIS,
    NBRE_LDT_GPIN,
    NBRE_LDT_EIDS,
    NBRE_LDT_LPDR
}NBRE_LayerDataType;

struct NBRE_LayerEIDSData
{
    NBGM_ElementId* mID;
};

typedef struct NBRE_LayerLabelData
{
    char*                       label;
    uint32                      languageCode;
}NBRE_LayerLabelData;
typedef struct NBRE_LayerTEXTData
{
    NBRE_Array*                 labelArray;             /*!< NBRE_Array<NBRE_LayerLabelData*> */
    uint16                      materialId;
    NBRE_MapMaterial*           material;               /*!< Internal Material*/
}NBRE_LayerTEXTData;

typedef struct NBRE_LayerPNTSData
{
    uint16                      materialId;             /*!< Material id of the point. */
    NBRE_MapMaterial*           material;               /*!< Internal Material*/
    NBRE_Point2f                position;               /*!< position the point. */
    NBRE_LayerTEXTData*         label;                  /*!< Name of the point. */
    NBGM_ElementId*             id;                     /*!< Used for Static POI. */
}NBRE_LayerPNTSData;

typedef struct NBRE_LayerGEOMData
{
    NBRE_DOM_Chunk*             geomChunk;
}NBRE_LayerGEOMData;

typedef struct NBRE_LayerAREAData
{
    uint16                      materialId;
    NBRE_LayerPNTSData*         label;                  /*!< Label point. */
    uint16                      labelIndex;             /*!< Index to point. internal usage*/
    NBRE_MapMaterial*           material;               /*!< Internal material*/
    NBRE_Array*                 polygon;                /*!< Mercator polygon array, each element in this array is a NBRE_Point2d. */
} NBRE_LayerAREAData;

typedef struct NBRE_LayerLPTHData
{
    uint16                      materialId;
    NBRE_MapMaterial*           material;               /*!< Internal color for DVRT*/
    float                       width;                  /*!< Width of this road */
    NBGM_ElementId*             elementId;              /*!< element id for picking*/
    NBRE_Array*                 polyline;               /*!< Array of mercator polylines.each element in this array is a NBRE_Point2d. */
} NBRE_LayerLPTHData;

typedef struct NBRE_LayerLPDRData
{
    uint16                      materialId;
    NBRE_MapMaterial*           material;               /*!< Internal color for DVRT*/
    float                       width;                  /*!< Width of this road */
    uint16                      drawOrder;              /*!< Draw order of road */
    nb_boolean                  roadFlag;               /*!< Road flag. To fix connection issue between draw orders of the same road. */
    NBGM_ElementId*             elementId;              /*!< element id for picking*/
    NBRE_Array*                 polyline;               /*!< Array of mercator polylines.each element in this array is a NBRE_Point2d. */
} NBRE_LayerLPDRData;

typedef struct NBRE_LayerTPTHData
{
    NBRE_LayerTEXTData*         label;                  /*!< Name of the track. */
    float                       width;
    NBRE_Array*                 polyline;               /*!< Array of mercator polylines.each element in this array is a NBRE_Point2d. */
}NBRE_LayerTPTHData;

typedef struct NBRE_LayerMESHData
{
    NBRE_LayerPNTSData**        labelArray;             /*!< Index to point. NBRE_Array<NBRE_LayerPNTSData*> */
    uint32                      labelCount;

    NBRE_Vector<NBRE_ModelPtr>* models;
    //Create Info, do not use when build 3d model
    NBRE_DOM_Chunk*             meshChunk;
}NBRE_LayerMESHData;

typedef struct NBRE_LayerTPSHData
{
    uint16                      materialId;
    NBRE_LayerTEXTData*         label;                  /*!< Name of the shield. */
    NBRE_Array*                 polyline;               /*!< Array of mercator polylines.each element in this array is a NBRE_Point2d. */
}NBRE_LayerTPSHData;

typedef struct NBRE_LayerTPARData
{
    uint16                      materialId;
    NBRE_Array*                 polyline;               /*!< Array of mercator polylines.each element in this array is a NBRE_Point2d. */
}NBRE_LayerTPARData;

typedef struct NBRE_LayerPOISData
{
    uint16                      selectedMaterialId;     /*!< Material ID, index into common material. */
    NBRE_MapMaterial*           selectedMaterial;       /*!< internal material for route */
    NBRE_LayerPNTSData*         point;                  /*!< Label point. */
    char*                       identifier;             /*!< An identifier unique to the POI, specified as a utf8 encoded text string. */
    uint16                      pointId;                /*!< Point ID, index into point layer, internal usage */ 
    NBRE_Vector2d               tileCenterPos;
}NBRE_LayerPOISData;

typedef struct NBRE_LayerGPINData
{
    uint16                      materialId;             /*!< Material ID, index into common material. */
    NBRE_MapMaterial*           material;               /*!< internal material. */
    NBRE_Vector2f               location;               /*!< location of this pin. */
    char*                       identifier;             /*!< An identifier unique to the PIN. */
    float                       radius;                 /*!< Radius of halo. */
}NBRE_LayerGPINData;

typedef struct NBRE_LayerInfo
{
    NBRE_DOM_LayerType          layerType;
    NBRE_LayerDataType          layerDataType;
    uint8                       associateLayerIndex;
    float                       nearVisibility;
    float                       farVisibility;
    uint16                      materialFlag;
}NBRE_LayerInfo;

typedef struct NBRE_Layer
{
    NBRE_LayerInfo              info;
    void**                      data;
    uint32                      dataCount;
}NBRE_Layer;

typedef struct NBRE_Mapdata
{
    NBRE_Point3d   refCenter;
    uint16         flag;
    NBRE_Array*    layerArray;                         //NBRE_Array<NBRE_Layer*>
}NBRE_Mapdata;

typedef struct LayerDataCrateStruct
{
    NBRE_DOM*               dom;
    NBRE_DOM_Chunk*         domChunk;
    NBRE_DOM_LayerType      layerType;
    NBRE_Point2d            orgOffset;
    NBRE_Point3d            offset;
    NBGM_ResourceContext*   resourceContext;
    NBRE_String             materialCategoryName;
}LayerDataCrateStruct;

PAL_Error NBRE_MapdataCreate(NBGM_ResourceContext& resourceContext, NBRE_DOM* dom, const NBRE_String& materialCategoryName, NBRE_Mapdata** mapdata);
void NBRE_MapdataDestroy(NBRE_Mapdata* mapdata);
void NBRE_MapdataDestoryInternalMaterial(NBRE_Mapdata* mapdata);
PAL_Error NBRE_MapdataGetAssociatedLayer(NBRE_Mapdata* mapdata, NBRE_LayerInfo* layerInfo, NBRE_Layer** layer);

void NBRE_LayerDestroy(NBRE_Layer* layer);
void NBRE_LayerAREADataDestroy(NBRE_LayerAREAData* area);
void NBRE_LayerLPTHDataDestroy(NBRE_LayerLPTHData* lpth);
void NBRE_LayerTPTHDataDestroy(NBRE_LayerTPTHData* tpth);
void NBRE_LayerPNTSDataDestroy(NBRE_LayerPNTSData* pnts);
void NBRE_LayerMESHDataDestroy(NBRE_LayerMESHData* mesh);
void NBRE_LayerTPSHDataDestroy(NBRE_LayerTPSHData* tpsh);
void NBRE_LayerTPARDataDestroy(NBRE_LayerTPARData* tpar);
void NBRE_LayerGEOMDataDestroy(NBRE_LayerGEOMData* geom);
void NBRE_LayerPOISDataDestroy(NBRE_LayerPOISData* pois);
void NBRE_LayerTEXTDataDestroy(NBRE_LayerTEXTData* text);
void NBRE_LayerGPINDataDestroy(NBRE_LayerGPINData* gpin);
void NBRE_LayerEIDSDataDestroy(NBRE_LayerEIDSData* eids);
void NBRE_LayerLPDRDataDestroy(NBRE_LayerLPDRData* lpdr);

void NBRE_LayerTEXTDataInternalMaterialDestroy(NBRE_LayerTEXTData* text);
void NBRE_LayerLPTHDataInternalMaterialDestroy(NBRE_LayerLPTHData* lpth);
void NBRE_LayerAREADataInternalMaterialDestroy(NBRE_LayerAREAData* area);
void NBRE_LayerTPTHDataInternalMaterialDestroy(NBRE_LayerTPTHData* tpth);
void NBRE_LayerPNTSDataInternalMaterialDestroy(NBRE_LayerPNTSData* pnts);
void NBRE_LayerMESHDataInternalMaterialDestroy(NBRE_LayerMESHData* mesh);
void NBRE_LayerTPSHDataInternalMaterialDestroy(NBRE_LayerTPSHData* tpsh);
void NBRE_LayerTPARDataInternalMaterialDestroy(NBRE_LayerTPARData* tpar);
void NBRE_LayerGEOMDataInternalMaterialDestroy(NBRE_LayerGEOMData* geom);
void NBRE_LayerPOISDataInternalMaterialDestroy(NBRE_LayerPOISData* pois);
void NBRE_LayerGPINDataInternalMaterialDestroy(NBRE_LayerGPINData* gpin);
void NBRE_LayerLPDRDataInternalMaterialDestroy(NBRE_LayerLPDRData* lpdr);

#endif
