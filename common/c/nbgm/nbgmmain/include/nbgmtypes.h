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

    @file nbgmtypes.h
*/
/*
    (C) Copyright 2012 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef _NBGM_TYPES_H_
#define _NBGM_TYPES_H_
#include "paltypes.h"
#include "nbgmbinarybuffer.h"
#include <string>

/*! \addtogroup NBGM_Instance
*  @{
*/

/*! A 3D Point. Generally, this is a map position. x means Mercator X, y means
    Mercator Y, z means height
*/
typedef struct NBGM_Point3d
{
    float x;        /*!< x */
    float y;        /*!< y */
    float z;        /*!< z */
}NBGM_Point3d;

/*! A high-precision 3D Point. Generally, this is a map position. x means
    Mercator X, y means Mercator Y, z means height
*/
typedef struct NBGM_Point3d64
{
    double x;        /*!< x */
    double y;        /*!< y */
    double z;        /*!< z */
}NBGM_Point3d64;

/*!  A 2D Point. 
*/
typedef struct NBGM_Point2d
{
    float x;        /*!< x */
    float y;        /*!< y */
}NBGM_Point2d;

/*! A high-precision 2D Point.
*/
typedef struct NBGM_Point2d64
{
    double x;        /*!< x */
    double y;        /*!< y */
}NBGM_Point2d64;

/*! NBGM rect2d structure to define a rectangle
*/
typedef struct NBGM_Rect2d
{
    NBGM_Point2d tl;    /*!< top left */
    NBGM_Point2d br;    /*!< bottom right */
}NBGM_Rect2d;

/*! NBGM high-precision rect2d structure to define a rectangle
*/
typedef struct NBGM_Rect2d64
{
    NBGM_Point2d64 tl;    /*!< top left */
    NBGM_Point2d64 br;    /*!< bottom right */
}NBGM_Rect2d64;

/*! NBGM tm/utm location
*/
typedef struct NBGM_Location
{
    NBGM_Point3d    position;       /*!< position */
    uint32          time;           /*!< time */
    float           speed;          /*!< speed */
    float           accuracy;       /*!< accuracy */
    float           heading;        /*!< heading */
}NBGM_Location;

/*! NBGM high-precision tm/utm location
*/
typedef struct NBGM_Location64
{
    NBGM_Point3d64  position;       /*!< position */
    uint64          time;           /*!< time */
    double          speed;          /*!< speed */
    double          accuracy;       /*!< accuracy */
    double          heading;        /*!< heading */
}NBGM_Location64;


/*! NBGM color structure
    Each color component should be from 0.0(zero intensity) to 1.0(full intensity).
    Color values are clamped to the range [0, 1] before used.
*/
typedef struct NBGM_Color {
    float           red;            /*!< red */
    float           green;          /*!< green */
    float           blue;           /*!< blue */
    float           alpha;          /*!< alpha */
} NBGM_Color;

/*! Texture palette (e.g. daytime or nighttime)
    This struct define the palette of route & turn arrow.
*/
typedef struct NBGM_Palette {
    NBGM_Color      routeFillColor;        /*!< route fill color*/
    NBGM_Color      routeEdgeColor;        /*!< route edge color*/
    NBGM_Color      arrowFillColor;        /*!< maneuver turn arrow fill color*/
    NBGM_Color      arrowEdgeColor;        /*!< maneuver turn arrow edge color*/
    NBGM_Color      backgroundColor;       /*!< view background color */
} NBGM_Palette;

/*! NBGM Circel Parametes
*/
typedef struct NBGM_CircleParameters {
    double centerX;           /*!< circel centerX, in Mercator*/
    double centerY;           /*!< circel centerY, in Mercator*/
    float radius;             /*!< radius, in pixels*/
    NBGM_Color fillColor;     /*!< fill color*/
    NBGM_Color strokeColor;   /*!< stroke color*/
    int zOrder;               /*!< draw order */
    bool visible;             /*!< visiblity */
} NBGM_CircleParameters;

/*! NBGM Rect2D Id Type
*/
typedef int NBGM_Rect2dId;

/*! NBGM Texture Id Type
*/
typedef int NBGM_TextureId;

/*! NBGM Rect2D parametes
*/
typedef struct NBGM_Rect2dParameters {
    double centerX;                /*!< rect centerX, in Mercator*/
    double centerY;                /*!< rect centerY, in Mercator*/
    float width;                   /*!< width, in pixels*/
    float height;                  /*!< height, in pixels*/
    float heading;                 /*!< heading, in degree*/
    bool visible;                  /*!< visiblity */
    NBGM_TextureId textureId;  /*!< texture id */
    NBGM_BinaryBuffer buffer;      /*!< texture buffer */
} NBGM_Rect2dParameters;

typedef struct NBGM_PinParameters {
    NBGM_PinParameters():
        unSelectedCalloutXCenterOffset(0),
        unSelectedCalloutYCenterOffset(0),
        unSelectedBubbleXCenterOffset(0),
        unSelectedBubbleYCenterOffset(0),
        unSelectedCircleInteriorColor(0),
        unSelectedCircleOutlineBitOnColor(0),
        unSelectedCircleOutlineBitOffColor(0),
        displayunSelectedWidth(0),
        dispalyunSelectedHeight(0),
        selectedCalloutXCenterOffset(0),
        selectedCalloutYCenterOffset(0),
        selectedBubbleXCenterOffset(0),
        selectedBubbleYCenterOffset(0),
        selectedCircleInteriorColor(0),
        selectedCircleOutlineBitOnColor(0),
        selectedCircleOutlineBitOffColor(0),
        displaySelectedWidth(0),
        dispalySelectedHeight(0),
        locationX(0),
        locationY(0),
        radius(0),
        circleOutlineWidth(0),
        linePattern(0),
        pinMaterialIndex(0)
    {
        unSelectedDataBinary.addr = NULL;
        unSelectedDataBinary.size = 0;
        selectedDataBinary.addr = NULL;
        selectedDataBinary.size = 0;
    }
public:
    int8      unSelectedCalloutXCenterOffset;
    int8      unSelectedCalloutYCenterOffset;
    int8      unSelectedBubbleXCenterOffset;
    int8      unSelectedBubbleYCenterOffset;

    uint32    unSelectedCircleInteriorColor;
    uint32    unSelectedCircleOutlineBitOnColor;
    uint32    unSelectedCircleOutlineBitOffColor;

    float     displayunSelectedWidth;
    float     dispalyunSelectedHeight;

    NBGM_BinaryBuffer unSelectedDataBinary;

    int8      selectedCalloutXCenterOffset;
    int8      selectedCalloutYCenterOffset;
    int8      selectedBubbleXCenterOffset;
    int8      selectedBubbleYCenterOffset;

    uint32    selectedCircleInteriorColor;
    uint32    selectedCircleOutlineBitOnColor;
    uint32    selectedCircleOutlineBitOffColor;

    float     displaySelectedWidth;
    float     dispalySelectedHeight;

    NBGM_BinaryBuffer selectedDataBinary;

    std::string    pinID;
    std::string    groupID;
    double    locationX;
    double    locationY;
    float     radius;
    uint8     circleOutlineWidth;
    uint32    linePattern;
    uint16    pinMaterialIndex;
}NBGM_PinParameters;

typedef struct NBGM_HBAOParameters
{
    NBGM_HBAOParameters():
        enable(true),
        radius(1.0f),
        bias(0.0f),
        powerExponent(1.2f),
        detailAO(0.0f),
        coarseAO(1.0f),
        enableBlur(TRUE),
        blurRadius(8),
        blurSharpness(1.0f)
    {
    }

public:
    nb_boolean enable;         /*!< Enable or disable AO effect */
    float radius;              /*!< The AO radius in meters */
    float bias;                /*!< To hide low-tessellation artifacts // 0.0~1.0 */
    float powerExponent;       /*!< The final AO output is pow(AO, powerExponent) */
    float detailAO;            /*!< Scale factor for the detail AO, the greater the darker // 0.0~2.0 */
    float coarseAO;            /*!< Scale factor for the coarse AO, the greater the darker // 0.0~2.0 */
    nb_boolean enableBlur;     /*!< To blur the AO with an edge-preserving blur */
    int blurRadius;            /*!< Kernel radius pixels(should be 2, 4, 8) */
    float blurSharpness;       /*!< The higher, the more the blur preserves edges // 0.0~16.0 */
}NBGM_HBAOParameters;
/*! @} */


typedef struct NBGM_GlowParameters
{
    NBGM_GlowParameters():
        enable(true),
        blurRadius(4.0f),
        blurSamples(4)
    {
        clearColor[0] = 1.0f;
        clearColor[1] = 1.0f;
        clearColor[2] = 1.0f;

        scaleColor[0] = 1.0f;
        scaleColor[1] = 1.0f;
        scaleColor[2] = 1.0f;
        scaleColor[3] = 1.0f;
    }

public:
    nb_boolean enable;         /*!< Enable or disable glow effect */
    float      clearColor[3];  /*!< Glow layer clear color in RGB order, range from 0.0 to 1.0. */
    float      scaleColor[4];  /*!< Blurred color in RGBA order, range from 0.0 to 1.0. Blur result is multiplied by it */
    float      blurRadius;     /*!< The blur radius in pixels */
    int        blurSamples;    /*!< The number of nearby pixels used in blur */
}NBGM_GlowParameters;
/*! @} */
#endif
