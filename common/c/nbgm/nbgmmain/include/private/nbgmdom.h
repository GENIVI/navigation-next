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

    @file nbgmdom.h
*/
/*
    (C) Copyright 2010 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef _NBRE_DOM_H_
#define _NBRE_DOM_H_

#include "palerror.h"
#include "nbgmarray.h"
#include "nbretypes.h"
#include "nbreiostream.h"

static const uint32 INVALID_INDEX = 0xFFFFFFFF;
static const uint16 INVALID_SHORT_INDEX = 0xFFFF;
static const uint8 INVALID_CHAR_INDEX = 0xFF;

#define CURRENT_SUPPORTED_NBM_REVISOION 24

#define MAKE_DOM_NAKE(A, B, C, D) ( ((D)<<24) + ((C)<<16) + ((B)<<8) + (A) )

#define DOM_ID_NBMF MAKE_DOM_NAKE ('N', 'B', 'M', 'F')
#define DOM_ID_LAYS MAKE_DOM_NAKE ('L', 'A', 'Y', 'S')
#define DOM_ID_SEEN MAKE_DOM_NAKE ('S', 'E', 'E', 'N')
#define DOM_ID_OBJS MAKE_DOM_NAKE ('O', 'B', 'J', 'S')
#define DOM_ID_OBJX MAKE_DOM_NAKE ('O', 'B', 'J', 'X')
#define DOM_ID_MESH MAKE_DOM_NAKE ('M', 'E', 'S', 'H')
#define DOM_ID_GEOM MAKE_DOM_NAKE ('G', 'E', 'O', 'M')
#define DOM_ID_MTRL MAKE_DOM_NAKE ('M', 'T', 'R', 'L')
#define DOM_ID_BMAP MAKE_DOM_NAKE ('B', 'M', 'A', 'P')
#define DOM_ID_IMGS MAKE_DOM_NAKE ('I', 'M', 'G', 'S')
#define DOM_ID_AREA MAKE_DOM_NAKE ('A', 'R', 'E', 'A')
#define DOM_ID_PNTS MAKE_DOM_NAKE ('P', 'N', 'T', 'S')
#define DOM_ID_TEXT MAKE_DOM_NAKE ('T', 'E', 'X', 'T')
#define DOM_ID_PLIN MAKE_DOM_NAKE ('P', 'L', 'I', 'N')
#define DOM_ID_SPLN MAKE_DOM_NAKE ('S', 'P', 'L', 'N')
#define DOM_ID_TPSH MAKE_DOM_NAKE ('T', 'P', 'S', 'H')
#define DOM_ID_TPAR MAKE_DOM_NAKE ('T', 'P', 'A', 'R')
#define DOM_ID_TPTH MAKE_DOM_NAKE ('T', 'P', 'T', 'H')
#define DOM_ID_LPTH MAKE_DOM_NAKE ('L', 'P', 'T', 'H')
#define DOM_ID_POIS MAKE_DOM_NAKE ('P', 'O', 'I', 'S')
#define DOM_ID_GPIN MAKE_DOM_NAKE ('G', 'P', 'I', 'N')
#define DOM_ID_EIDS MAKE_DOM_NAKE ('E', 'I', 'D', 'S')
#define DOM_ID_DROD MAKE_DOM_NAKE ('D', 'R', 'O', 'D')
#define DOM_ID_LPDR MAKE_DOM_NAKE ('L', 'P', 'D', 'R')

typedef enum NBRE_DOM_MaterialType
{
    NBRE_DMT_StandardTexturedMaterial = 0,
    NBRE_DMT_StandardColorMaterial,
    NBRE_DMT_OutlinedColorMaterial,
    NBRE_DMT_LightedMaterial,
    NBRE_DMT_FontMaterial,
    NBRE_DMT_LinePatternMaterial,
    NBRE_DMT_BackgroundMaterial,
    NBRE_DMT_ShieldMaterial,
    NBRE_DMT_DashOutlineMaterial,
    NBRE_DMT_PathArrowMaterial,
    NBRE_DMT_HorizontalLightSourceMaterial,
    NBRE_DMT_PinMaterial,
    NBRE_DMT_OutlinedComplexColorFillMaterial,
    NBRE_DMT_OutlinedSimpleColorFillMaterial,
    NBRE_DMT_ShieldWithYGapMaterial,
    NBRE_DMT_RadialPinMaterial,
    NBRE_DMT_StaticPOIMaterial,
    NBRE_DMT_PolylineCapMaterial,
    NBRE_DMT_BitMapPatternMaterial,
    NBRE_DMT_OutlinedHoleyMaterial = 20
}NBRE_DOM_MaterialType;

typedef enum NBRE_DOM_LayerType
{
    NBRE_DLT_Associated = 0,
    NBRE_DLT_RoadNetwork,
    NBRE_DLT_AreaPolygon,
    NBRE_DLT_UnTextureBuilding,
    NBRE_DLT_UnTextureLandmarkBuilding,
    NBRE_DLT_LandmarkBuilding,
    NBRE_DLT_Raster,
    NBRE_DLT_Pois,
    NBRE_DLT_Route,
    NBRE_DLT_PointLabel,
    NBRE_DLT_Satellite,
    NBRE_DLT_StaticPOI,
    NBRE_DLT_Material,
    NBRE_DLT_ECM = 0x8000,
}NBRE_DOM_LayerType;

typedef void (*NBRE_DOM_ChunkDestroyFunc)(void* chunk);
typedef struct NBRE_DOM NBRE_DOM;

typedef struct NBRE_DOM_ChunkInfo
{
    uint32                          ID;
    uint32                          baseSize;
    uint32                          chunkSize;
    uint32                          fileOffset;
    NBRE_DOM_ChunkDestroyFunc       destroyFunc;
}NBRE_DOM_ChunkInfo;

typedef struct NBRE_DOM_NBMF
{
    NBRE_DOM_ChunkInfo          chunkInfo;
    uint16                      flag;
    uint8                       majorVersion;
    uint8                       minorVersion;
    double                      refCenterX;
    double                      refCenterY;
    double                      tileSizeX;
    double                      tileSizeY;
    float                       aabbLeft;
    float                       aabbBottom;
    float                       aabbRight;
    float                       aabbTop;
}NBRE_DOM_NBMF;

typedef struct NBRE_DOM_Image
{
    uint8                       nameLen;
    uint8*                      textureName;
}NBRE_DOM_Image;
typedef struct NBRE_DOM_IMGS
{
    NBRE_DOM_ChunkInfo          chunkInfo;
    uint16                      flag;
    uint16                      count;
    NBRE_DOM_Image*             images;
}NBRE_DOM_IMGS;

typedef struct NBRE_DOM_Bitmap
{
    uint8                       nameLen;
    uint8*                      name;
    uint32                      fileSize;
    uint32                      offset;
    uint8*                      imageData;
}NBRE_DOM_Bitmap;
typedef struct NBRE_DOM_BMAP
{
    NBRE_DOM_ChunkInfo          chunkInfo;
    uint16                      flag;
    uint16                      count;
    NBRE_DOM_Bitmap*            bitmaps;
}NBRE_DOM_BMAP;

typedef struct NBDM_DOM_Lighting
{
    float                       emission[3];
    float                       ambient[3];
    float                       diffuse[3];
    float                       specular[3];
    float                       shininess;
}NBDM_DOM_Lighting;

typedef struct NBRE_DOM_StandardTexturedMaterial
{
    uint32                      color;
    uint16                      texId;
}NBRE_DOM_StandardTexturedMaterial;
typedef struct NBRE_DOM_StandardColorMaterial
{
    uint32                      color;
}NBRE_DOM_StandardColorMaterial;
typedef struct NBRE_DOM_OutlinedColorsMaterial
{
    uint32                      interiorColor;
    uint32                      outlineColor;
    uint8                       outlineWidth;
}NBRE_DOM_OutlinedColorsMaterial;
typedef struct NBRE_DOM_LightedMaterial
{
    uint32                      color;
    uint16                      texId;
    NBDM_DOM_Lighting*          linghting;
}NBRE_DOM_LightedMaterial;
typedef struct NBRE_DOM_FontMaterial
{
    uint32                      textColor;
    uint32                      outlineColor;
    uint8                       family;
    uint8                       style;
    uint8                       optimal;
    uint8                       maximum;
    uint8                       minimum;
    uint8                       latterSpacing;
    uint8                       outlineWidth;

}NBRE_DOM_FontMaterial;
typedef struct NBRE_DOM_LinePatternMaterial
{
    uint32                      bitOnColor;
    uint32                      bitOffColor;
    uint32                      outlineColor;
    uint32                      linePattern;
    uint8                       outlineWidth;
}NBRE_DOM_LinePatternMaterial;
typedef struct NBRE_DOM_BackgroundMaterial
{
    uint32                      color;
    uint16                      textureIndex;
}NBRE_DOM_BackgroundMaterial;
typedef struct NBRE_DOM_ShieldMaterial
{
    uint16                      materialIndex;
    uint16                      iconIndex;
    int8                        xOffset;
    int8                        yOffset;
    uint8                       xPadding;
}NBRE_DOM_ShieldMaterial;
typedef struct NBRE_DOM_ShieldWithYGapMaterial
{
    uint16                      materialIndex;
    uint16                      iconIndex;
    int8                        xOffset;
    int8                        yOffset;
    uint8                       xPadding;
    uint8                       yPadding;
}NBRE_DOM_ShieldWithYGapMaterial;
typedef struct NBRE_DOM_DashOutlineMaterial
{
    uint32                      bitOnColor;
    uint32                      bitOffColor;
    uint32                      pattern;
    uint32                      interiorColor;
    uint8                       outlineWidth;
}NBRE_DOM_DashOutlineMaterial;
typedef struct NBRE_DOM_PathArrowMaterial
{
    uint32                      arrowColor;
    uint8                       tailWidth;
    uint8                       tailLength;
    uint8                       headWidth;
    uint8                       headLength;
    uint8                       repeat;
}NBRE_DOM_PathArrowMaterial;
typedef struct NBRE_DOM_HorizontalLightSourceMaterial
{
    float                       lightAngle;
    uint32                      degree0Color;
    uint32                      degree180Color;
    uint32                      degree270Color;
    uint32                      degree90Color;
    uint32                      topColor;
    uint32                      bottomColor;
    uint8                       transparency;
}NBRE_DOM_HorizontalLightSourceMaterial;
typedef struct NBRE_DOM_PinMaterial
{
    uint16                      iconIndex;
    int8                        xImageOffset;
    int8                        yImageOffset;
    int8                        xBubbleOffset;
    int8                        yBubbleOffset;
}NBRE_DOM_PinMaterial;
typedef struct NBRE_DOM_OutlinedComplexFillMaterial
{
    uint32                      interiorFillColor;
    uint32                      outlineColor;
    uint8                       width;
}NBRE_DOM_OutlinedComplexFillMaterial;
typedef struct NBRE_DOM_OutlinedSimpleFillMaterial
{
    uint32                      interiorFillColor;
    uint32                      outlineColor;
    uint8                       width;
}NBRE_DOM_OutlinedSimpleFillMaterial;
typedef struct NBRE_DOM_RadialPinpMaterial
{
    uint16    unSelectedBMP;
    uint16    selectedBMP;

    int8      unSelectedCalloutXCenterOffset;
    int8      unSelectedCalloutYCenterOffset;
    int8      unSelectedBubbleXCenterOffset;
    int8      unSelectedBubbleYCenterOffset;

    int8      selectedCalloutXCenterOffset;
    int8      selectedCalloutYCenterOffset;
    int8      selectedBubbleXCenterOffset;
    int8      selectedBubbleYCenterOffset;

    uint32    selectedCircleInteriorColor;
    uint32    unSelectedCircleInteriorColor;

    float     displayNormalWidth;
    float     dispalyNormalHeight;
    float     displaySelectedWidth;
    float     dispalySelectedHeight;

    uint8     circleOutlineWidth;

    uint32    selectedCircleOutlineBitOnColor;
    uint32    selectedCircleOutlineBitOffColor;
    uint32    unSelectedCircleOutlineBitOnColor;
    uint32    unSelectedCircleOutlineBitOffColor;

    uint32    linePattern;
}NBRE_DOM_RadialPinpMaterial;
typedef struct NBRE_DOM_StaticPOIMaterial
{
    uint16    iconBPMIndex;
    uint16    selectedBMPIndex;
    uint16    unSelectedBMPIndex;
    int8      calloutXCenterOffset;
    int8      calloutYCenterOffset;
    int8      bubbleXCenterOffset;
    int8      bubbleYCenterOffset;
    int8      distanceToAnother;
    int8      distanceToLabel;
    int8      distanceToPoi;
    uint8     iconWidth;
    uint8     iconHeight;
}NBRE_DOM_StaticPOIMaterial;
struct NBRE_DOM_PolylineCapMaterial
{
    uint32    interiorColor;
    uint32    outlineColor;
    uint8     outlineWidth;
    uint8     startType;
    uint8     endType;
    uint16    height;
    uint16    width;
    uint16    radius;
};

struct NBRE_DOM_BitMapPatternMaterial
{
    uint32    color;
    uint16    textureID;
    uint16    distance;
    uint16    height;
    uint16    width;
};

typedef struct NBRE_DOM_OutlinedHoleyMaterial
{
    uint32                      interiorFillColor;
    uint32                      outlineColor;
    uint8                       width;
}NBRE_DOM_OutlinedHoleyMaterial;


typedef struct NBRE_DOM_Material
{
    uint8                                   type;
    uint8                                   materialSize;
    uint16                                  backupIndex;
    NBRE_DOM_StandardTexturedMaterial*      stm;
    NBRE_DOM_StandardColorMaterial*         scm;
    NBRE_DOM_OutlinedColorsMaterial*        ocm;
    NBRE_DOM_LightedMaterial*               lm;
    NBRE_DOM_FontMaterial*                  fm;
    NBRE_DOM_LinePatternMaterial*           lpm;
    NBRE_DOM_BackgroundMaterial*            bgm;
    NBRE_DOM_ShieldMaterial*                sm;
    NBRE_DOM_DashOutlineMaterial*           dom;
    NBRE_DOM_PathArrowMaterial*             pam;
    NBRE_DOM_HorizontalLightSourceMaterial* hlsm;
    NBRE_DOM_PinMaterial*                   pinm;
    NBRE_DOM_OutlinedComplexFillMaterial*   ocfm;
    NBRE_DOM_OutlinedSimpleFillMaterial*    osfm;
    NBRE_DOM_ShieldWithYGapMaterial*        sym;
    NBRE_DOM_RadialPinpMaterial*            rpm;
    NBRE_DOM_StaticPOIMaterial*             spm;
    NBRE_DOM_PolylineCapMaterial*           pcm;
    NBRE_DOM_BitMapPatternMaterial*         bmpm;
    NBRE_DOM_OutlinedHoleyMaterial*         ohm;
}NBRE_DOM_Material;
typedef struct NBRE_DOM_MTRL
{
    NBRE_DOM_ChunkInfo          chunkInfo;
    uint16                      flag;
    uint16                      count;
    NBRE_DOM_Material*          materials;
}NBRE_DOM_MTRL;

typedef struct NBRE_DOM_ShapeList
{
    float                      position[3];
    float                      outTan[3];
    float                      inTan[3];
}NBRE_DOM_ShapeList;
typedef struct NBRE_DOM_Curves
{
    uint16                      material;
    float                       width;
    uint16                      label;
    uint16                      count;
    NBRE_DOM_ShapeList*         shapeList;
}NBRE_DOM_Curves;
typedef struct NBRE_DOM_SPLN
{
    NBRE_DOM_ChunkInfo          chunkInfo;
    uint16                      flag;
    uint16                      count;
    NBRE_DOM_Curves*            curves;
}NBRE_DOM_SPLN;

typedef struct NBRE_DOM_Polyline
{
    uint16                      count;
    double*                      locations;
}NBRE_DOM_Polyline;
typedef struct NBRE_DOM_PLIN
{
    NBRE_DOM_ChunkInfo          chunkInfo;
    uint16                      flag;
    uint16                      count;
    NBRE_DOM_Polyline*          polylines;
}NBRE_DOM_PLIN;

typedef struct NBRE_DOM_Road
{
    uint16                      material;
    float                       lineWidth;
    uint16                      lineIndex;
}NBRE_DOM_Road;
typedef struct NBRE_DOM_LPTH
{
    NBRE_DOM_ChunkInfo          chunkInfo;
    uint16                      flag;
    uint16                      count;
    NBRE_DOM_Road*              roads;
}NBRE_DOM_LPTH;

typedef struct NBRE_DOM_RoadWithDrawOrder
{
    uint16                      material;
    float                       lineWidth;
    uint16                      lineIndex;
    uint8                       roadFlag;
    uint8                       drawOrder;
    float                       distance;
}NBRE_DOM_RoadWithDrawOrder;
typedef struct NBRE_DOM_LPDR
{
    NBRE_DOM_ChunkInfo          chunkInfo;
    uint16                      flag;
    uint16                      count;
    NBRE_DOM_RoadWithDrawOrder* roads;
}NBRE_DOM_LPDR;

typedef struct NBRE_DOM_PathShield
{
    uint16                      shieldMaterialIndex;
    uint16                      labelIndex;
    uint16                      lineIndex;
}NBRE_DOM_PathShield;
typedef struct NBRE_DOM_TPSH
{
    NBRE_DOM_ChunkInfo          chunkInfo;
    uint16                      flag;
    uint16                      count;
    NBRE_DOM_PathShield*        pathShields;
}NBRE_DOM_TPSH;

typedef struct NBRE_DOM_PathArrow
{
    uint16                      materialIndex;
    uint16                      lineIndex;
}NBRE_DOM_PathArrow;
typedef struct NBRE_DOM_TPAR
{
    NBRE_DOM_ChunkInfo          chunkInfo;
    uint16                      flag;
    uint16                      count;
    NBRE_DOM_PathArrow*         pathArrows;
}NBRE_DOM_TPAR;

typedef struct NBRE_DOM_TextPath
{
    float                       lineWidth;
    uint16                      label;
    uint16                      line;
}NBRE_DOM_TextPath;
typedef struct NBRE_DOM_TPTH
{
    NBRE_DOM_ChunkInfo          chunkInfo;
    uint16                      flag;
    uint16                      count;
    NBRE_DOM_TextPath*          textPaths;
}NBRE_DOM_TPTH;

typedef struct NBRE_DOM_Label
{
    uint16                      material;
    uint8                       stringLenHigh;
    uint8                       stringLenLow;
    uint8*                      string;
}NBRE_DOM_Label;
typedef struct NBRE_DOM_TEXT
{
    NBRE_DOM_ChunkInfo          chunkInfo;
    uint16                      flag;
    uint16                      count;
    NBRE_DOM_Label*             labels;
}NBRE_DOM_TEXT;

typedef struct NBRE_DOM_Point
{
    uint16                      material;
    float                       location[2];
    uint16                      labelIndex;
}NBRE_DOM_Point;
typedef struct NBRE_DOM_PNTS
{
    NBRE_DOM_ChunkInfo          chunkInfo;
    uint16                      flag;
    uint16                      count;
    NBRE_DOM_Point*             points;
}NBRE_DOM_PNTS;

typedef struct NBRE_DOM_Area
{
    uint16                      material;
    uint16                      label;
    uint16                      shape;
}NBRE_DOM_Area;
typedef struct NBRE_DOM_AREA
{
    NBRE_DOM_ChunkInfo          chunkInfo;
    uint16                      flag;
    uint16                      count;
    NBRE_DOM_Area*              areas;
}NBRE_DOM_AREA;

typedef struct NBRE_DOM_Geometry
{
    uint8                       flag;
    uint16                      size;
    float*                      position;
    float*                      normal;
    float*                      texCoord;
}NBRE_DOM_Geometry;
typedef struct NBRE_DOM_GEOM
{
    NBRE_DOM_ChunkInfo          chunkInfo;
    uint16                      flag;
    uint16                      count;
    NBRE_DOM_Geometry*          geometries;
}NBRE_DOM_GEOM;

typedef struct NBRE_DOM_Mesh
{
    uint8                       flag;
    uint16                      materialIndex;
    uint16                      labelIndex;
    uint16                      geometryIndex;

    uint16                      indicesCount;
    uint16*                     indices;

}NBRE_DOM_Mesh;
typedef struct NBRE_DOM_MESH
{
    NBRE_DOM_ChunkInfo          chunkInfo;
    uint16                      flag;
    uint16                      count;
    NBRE_DOM_Mesh*              meshes;
}NBRE_DOM_MESH;

typedef struct NBRE_DOM_Object
{
    uint32                      chunkID;
    uint16                      objIndex;
}NBRE_DOM_Object;
typedef struct NBRE_DOM_OBJS
{
    NBRE_DOM_ChunkInfo          chunkInfo;
    uint16                      flag;
    uint16                      count;
    NBRE_DOM_Object*            objects;
}NBRE_DOM_OBJS;

typedef struct NBRE_DOM_ExternalObject
{
    uint16                      fileNameLen;
    char*                       fileName;
    uint32                      checkSum;
    uint16                      count;
    uint16*                     index;
}NBRE_DOM_ExternalObject;
typedef struct NBRE_DOM_OBJX
{
    NBRE_DOM_ChunkInfo          chunkInfo;
    uint16                      flag;
    uint16                      count;
    NBRE_DOM_ExternalObject*    externalObjs;
}NBRE_DOM_OBJX;

typedef struct NBRE_DOM_Scene
{
    uint32                      chunkID;
    uint8                       placementFlag;
    float                       placementMatrix[16];
    uint16                      objCount;
    uint16*                     objIndex;
}NBRE_DOM_Scene;
typedef struct NBRE_DOM_SEEN
{
    NBRE_DOM_ChunkInfo          chunkInfo;
    uint16                      flag;
    uint16                      count;
    NBRE_DOM_Scene*             scenes;
}NBRE_DOM_SEEN;

typedef struct NBRE_DOM_Poi
{
    uint16                      selectedMaterialId;
    uint16                      pointId;
    uint8                       identifierLen;
    uint8*                      identifier;
}NBRE_DOM_Poi;
typedef struct NBRE_DOM_POIS
{
    NBRE_DOM_ChunkInfo          chunkInfo;
    uint16                      flag;
    uint16                      count;
    NBRE_DOM_Poi*               pois;
}NBRE_DOM_POIS;

typedef struct NBRE_DOM_Layer
{
    NBRE_DOM_LayerType          type;
    float                       nearVisibility;
    float                       farVisibility;
    uint32                      chunkOffset;
    uint8                       associateLayerIndex;
}NBRE_DOM_Layer;
typedef struct NBRE_DOM_LAYS
{
    NBRE_DOM_ChunkInfo          chunkInfo;
    uint16                      flag;
    uint16                      count;
    NBRE_DOM_Layer*             layers;
}NBRE_DOM_LAYS;

typedef struct NBRE_DOM_Gpin
{
    uint16                      materialIndex;
    float                       location[2];
    uint16                      pinIdDndex;
    uint16                      radius;
}NBRE_DOM_Gpin;
typedef struct NBRE_DOM_GPIN
{
    NBRE_DOM_ChunkInfo          chunkInfo;
    uint16                      flag;
    uint16                      count;
    NBRE_DOM_Gpin*              gpins;
}NBRE_DOM_GPIN;

typedef struct NBRE_DOM_Eids
{
    uint8   byteCount;
    uint8*  asciiString;
}NBRE_DOM_Eids;
typedef struct NBRE_DOM_EIDS
{
    NBRE_DOM_ChunkInfo          chunkInfo;
    uint16                      flag;
    uint16                      count;
    NBRE_DOM_Eids*              eids;
}NBRE_DOM_EIDS;

typedef struct NBRE_DOM_Drod
{
    uint16   materialIndex;
    uint16   drawOrder;
}NBRE_DOM_Drod;
typedef struct NBRE_DOM_DROD
{
    NBRE_DOM_ChunkInfo          chunkInfo;
    uint16                      flag;
    uint16                      count;
    NBRE_DOM_Drod*              drod;
}NBRE_DOM_DROD;

typedef void* NBRE_DOM_Chunk;

PAL_Error NBRE_DOMCreateFromFile(NBRE_IOStream* stream, const NBRE_String& domName, uint32 nbmOffset, NBRE_DOM** dom);
void NBRE_DOMDestroy(NBRE_DOM* dom);

const char* NBRE_DOMGetName(NBRE_DOM* dom);

PAL_Error NBRE_DOMGetChunkArray(NBRE_DOM* dom, uint32 chunkID, NBRE_Array** chunkArray);
PAL_Error NBRE_DOMGetChunk(NBRE_DOM* dom, uint32 chunkOffset, NBRE_DOM_Chunk** chunk);
PAL_Error NBRE_DOMGetChunkById(NBRE_DOM* dom, uint32 chunkID, NBRE_DOM_Chunk** chunk);
#endif
