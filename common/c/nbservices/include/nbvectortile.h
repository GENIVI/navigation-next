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

@file     nbvectortile.h

*/
/*
(C) Copyright 2014 by TeleCommunication Systems, Inc.                

The information contained herein is confidential, proprietary 
to TeleCommunication Systems, Inc., and considered a trade secret as 
defined in section 499C of the penal code of the State of     
California. Use of this information by anyone other than      
authorized employees of TeleCommunication Systems is granted only    
under a written non-disclosure agreement, expressly           
prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#ifndef VECTORTILE_H
#define VECTORTILE_H

#include "nbexp.h"

/*! 
	@addtogroup nbvectortile
	@{ 
*/


/*! Point in Mercator projection. 

    The coordinates go from -PI to +PI. Use the spatial interface to convert to/from latitude/longitude
    coordinates.
*/
typedef struct 
{
    double mx;                      /*!< x-coordinate in Mercator projection */
    double my;                      /*!< y-coordinate in Mercator projection */

} NB_MercatorPoint;

/*! Array of Mercator points. */
typedef struct 
{
    NB_MercatorPoint*  points;     /*!< Mercator point array. */
    int                 count;      /*!< Number of elements in mercator point array. */

} NB_MercatorPointBuffer;

typedef NB_MercatorPointBuffer NB_MercatorTriStrip;
typedef NB_MercatorPointBuffer NB_MercatorPolygon;
typedef NB_MercatorPointBuffer NB_MercatorPolyline;

/*! Vector Map Priority. */
typedef enum 
{
    NB_VMP_Lower,
    NB_VMP_Low,
    NB_VMP_Normal,
    NB_VMP_Medium,
    NB_VMP_High,
    NB_VMP_Higher

} NB_VectorTilePriority;

/*! Type for map area. */
typedef enum 
{
    NB_VMAT_None,
    NB_VMAT_Building,
    NB_VMAT_Interior,
    NB_VMAT_Water,
    NB_VMAT_Campus,
	NB_VMAT_Parks,
	NB_VMAT_Geopolitical

} NB_VectorTileAreaType;

/*! Vector map area used for tile. */
typedef struct 
{
    NB_VectorTilePriority   priority;
    NB_VectorTileAreaType   areaType;
    char*                   label;          /*!< Name of the area. */
    NB_MercatorPolygon*		polygons;       /*!< Mercator polygon array. */
    int                     polygonCount;   /*!< Number of elements in mercator polygon array. */

} NB_VectorTileArea;

/*! Vector map line used for tile. */
typedef struct 
{
    NB_VectorTilePriority   priority;
    char*                   label;
    NB_MercatorPolyline*   polylines;      /*!< Mercator polyline array. */
    int                     polylineCount;  /*!< Number of elements in mercator polyline array. */

} NB_VectorTileLine;

/*! Vector point used for tile. */
typedef struct 
{
    NB_VectorTilePriority   priority;
    char*                   label;          /*!< Name of point */
    NB_MercatorPoint       point;

} NB_VectorTilePoint;

/*! Vector map road type. */
typedef enum 
{
	NB_VMRT_None = 0,
	NB_VMRT_Limited_access,
	NB_VMRT_Arterial,
	NB_VMRT_Local,
	NB_VMRT_Terminal,
	NB_VMRT_Rotary,
	NB_VMRT_Ramp,
	NB_VMRT_Bridge,
	NB_VMRT_Tunnel,
	NB_VMRT_Skyway,
    NB_VMRT_Ferry

} NB_VectorTileRoadType;

/*! Vector map road used for vector tile. */
typedef struct 
{
    NB_VectorTilePriority   priority;
    NB_VectorTileRoadType   roadType;
    char*                   label;                  /*!< Name of the street/road */
    uint32                  lanes;                  /*!< Number or lanes for this road */
    NB_MercatorPolyline*   polylines;              /*!< Array of mercator polylines. */
    int                     polylineCount;          /*!< Number of elements in the mercator polyline array. */
    NB_MercatorTriStrip*   foregroundTriStrip;     /*!< Array of mercator tri-strips for the foreground. */
    int                     foregroundCount;        /*!< Number of elements in the foreground tri-strip array. */
    NB_MercatorTriStrip*   backgroundTriStrip;     /*!< Array of mercator tri-strips for the background. */
    int                     backgroundCount;        /*!< Number of elements in the background tri-strip array. */

} NB_VectorTileRoad;

/*! Vector map tile. */
typedef struct 
{
    int                     x;              /*!< mercator x index of tile*/
    int                     y;              /*!< mercator y index of tile*/
    int                     zoomLevel;      /*!< tile zoom level. Between 0 and 17 */
    
    NB_VectorTileArea*      areas;          /*!< Array of areas. */
    int                     areaCount;      /*!< Number of elements in area array. */

    // Lines are currently not supported. Always empty!
    NB_VectorTileLine*      lines;          /*!< Array of lines */
    int                     lineCount;      /*!< Number of elements in line array */
    
    // Points are currently not supported. Always empty!
    NB_VectorTilePoint*     points;         /*!< Array of points */
    int                     pointCount;     /*!< Number of elements in point array */
    
    NB_VectorTileRoad*      roads;          /*!< Array of roads */
    int                     roadCount;      /*!< Number of elements in road array */
    
} NB_VectorTile;


/*! Vector Tile Index. */
typedef struct 
{
    int x;			/*!< x-index of tile to get */
    int y;			/*!< y-index of tile to get */
    
} NB_VectorTileIndex;


/*! Destroy a NB_MercatorPolyline object
@param polyline The polyline to destroy
@return NB_Error
*/
/// @todo Move this to nbmercator.h
NB_DEC NB_Error NB_MercatorPolylineDestroy(NB_MercatorPolyline* polyline);


/*! @} */

#endif // VECTORTILE_H
