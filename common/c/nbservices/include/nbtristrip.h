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

@file     nbtristrip.h
*/
/*
(C) Copyright 2005 - 2009 by Networks In Motion, Inc.

The information contained herein is confidential, proprietary
to Networks In Motion, Inc., and considered a trade secret as
defined in section 499C of the penal code of the State of
California. Use of this information by anyone other than
authorized employees of Networks In Motion is granted only
under a written non-disclosure agreement, expressly
prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

/*!
@addtogroup nbtristrip
@{
*/

#ifndef NBTRISTRIP_H
#define NBTRISTRIP_H

#include "paltypes.h"
#include "nbexp.h"
#include "nbvectortile.h"
#include "nbspatial.h"

/*! TriStrip object that is returned from NB_TriStripPolylineToMercatorCreate. */
typedef struct 
{
    NB_MercatorTriStrip*    triStripForeground;     /*!< Foreground array of NB_MercatorPoint that makes up the TriStrip  */
    NB_MercatorTriStrip*    triStripBackground;     /*!< Background array of NB_MercatorPoint that makes up the TriStrip  */
} NB_TriStrip; 


/*! Returns a NB_TriStrip object from the given Polyline, length (optional), and direction

You must call NB_TriStripDestroy when you are done with the NB_TriStrip object.

@param roadType Road type to use as width
@param polyline TriStrip will be created from this Polyline
@param tileIndex Tile index where this polyline is located
@param zoomLevel Refer to NB_VectorTileManager for more information on zoom levels
@param forward Direction to iterate along the polyline.  TRUE will iterate forward
from index 0, FALSE will iterate in the reverse direction from (polyline.count - 1)
@param length Maximum length to iterate along the polyline.  Pass -1 for no limit.
@param triStrip On success, a new TriStrip object of the Polyline; otherwise, NULL
@param lastPoint On success, return the final point at the end of the segment if
parameter is not NULL.
@param lastHeading On success, return the final heading at the end of the segment
if parameter is not NULL.
@returns NB_Error
*/
NB_DEC NB_Error
NB_TriStripPolylineToMercatorCreate(
                                    NB_VectorTileRoadType roadType,
                                    NB_MercatorPolyline* polyline,
                                    NB_VectorTileIndex tileIndex,
                                    int zoomLevel,
                                    nb_boolean forward,
                                    double maxLength,
                                    NB_TriStrip** triStrip,
                                    NB_MercatorPoint* lastPoint,
                                    NB_Vector* lastHeading);

/*! Returns a beveled NB_TriStrip object from the given Polyline, length (optional), and direction

You must call NB_TriStripDestroy when you are done with the NB_TriStrip object.

@param roadType Road type to use as width
@param polyline TriStrip will be created from this Polyline
@param tileIndex Tile index where this polyline is located
@param zoomLevel Refer to NB_VectorTileManager for more information on zoom levels
@param forward Direction to iterate along the polyline.  TRUE will iterate forward
from index 0, FALSE will iterate in the reverse direction from (polyline.count - 1)
@param length Maximum length to iterate along the polyline.  Pass -1 for no limit.
@param triStrip On success, a new TriStrip object of the Polyline; otherwise, NULL
@param lastPoint On success, return the final point at the end of the segment if
parameter is not NULL.
@param lastHeading On success, return the final heading at the end of the segment
if parameter is not NULL.
@returns NB_Error
*/
NB_DEC NB_Error
NB_TriStripPolylineToMercatorCreateBeveled(
    NB_VectorTileRoadType roadType,
    NB_MercatorPolyline* polyline,
    NB_VectorTileIndex tileIndex,
    int zoomLevel,
    nb_boolean forward,
    double maxLength,
    NB_TriStrip** triStrip,
    NB_MercatorPoint* lastPoint,
    NB_Vector* lastHeading);


/*! Destroys a previously created NB_TriStrip object

@param triStrip A NB_TriStrip object previously created from a call to NB_TriStripPolylineToMercatorCreate
or NB_TriStripPolylineToMercatorLengthCreate
@returns NB_Error
*/
NB_DEC NB_Error
NB_TriStripDestroy(
    NB_TriStrip* triStrip);

#endif

/*! @} */
