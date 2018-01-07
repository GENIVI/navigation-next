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

@file nbvectortransform.h
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

/*!
    @addtogroup nbspatial
    @{
*/

#ifndef NB_VECTOR_TRANSFORM_H
#define NB_VECTOR_TRANSFORM_H

#include "nbcontext.h"
#include "nbexp.h"
#include "nberror.h"
#include "nbspatial.h"

typedef struct NB_VectorTransform NB_VectorTransform;

typedef enum
{
    NB_VTT_Unknown = 0,
    NB_VTT_2D,                                              /*!< 2D transformation */
    NB_VTT_3D                                               /*!< 3D transformation */
} NB_VectorTransformType;

typedef enum
{
    NB_VTP_Unknown = 0,
    NB_VTP_Lower,                                           /*!< Avatar in lower portion of screen */
    NB_VTP_Mid,                                             /*!< Avatar in middle of screen */
    NB_VTP_Upper,                                           /*!< Avatar in upper portion of screen */
    NB_VTP_Custom                                           /*!< Avatar in portion of screen defined by positionMultiplier */

} NB_VectorTransformPosition;

typedef struct
{
    NB_VectorTransformType          type;                   /*!< Transformation type */
    NB_VectorTransformPosition      position;               /*!< User position */
    double                          metersPerPixel;         /*!< Meters per pixel in map */
    double                          height;                 /*!< Height of camera, in meters */
    double                          fieldOfView;            /*!< Camera field of view, in degrees */
    double                          zDistance;              /*!< Distance between camera and avatar?, in meters */
    double                          positionMultiplier;     /*!< Custom position multiplier for NB_VTP_Custom user position */

} NB_VectorTransformSettings;


/*! Create a vector transformation object
@param transform A pointer to a pointer to receive the new transform object; a non-NULL transform must be destroyed with NB_VectorTransformDestroy()
@return NB_Error
*/
NB_DEC NB_Error NB_VectorTransformCreate(NB_VectorTransform** transform);


/*! Destroy a vector transform object
@param transform A pointer to a tranform object previously created with NB_VectorTransformCreate()
@return NB_Error
*/
NB_DEC NB_Error NB_VectorTransformDestroy(NB_VectorTransform* transform);


/*! Set the current transform
@param transform A pointer to a tranform object previously created with NB_VectorTransformCreate()
@param settings The transformation settings to use
@param rect The current view rectangle
@param center The latitude and longitude of the center of the view rectangle
@param heading The view heading
@return NB_Error
*/
NB_DEC NB_Error NB_VectorTransformSet(NB_VectorTransform* transform, NB_VectorTransformSettings* settings, NB_Rectangle* rect, NB_LatitudeLongitude* center, double heading);


/*! Convert a mercator point to a vector
@param transform A pointer to a tranform object previously created with NB_VectorTransformCreate()
@param xMercator Mercator point
@param yMercator Mercator point
@param vector The transformed point value
@return NB_Error
*/
NB_DEC NB_Error NB_VectorTransformMercatorTo3D(NB_VectorTransform* transform, double xMercator, double yMercator, NB_Vector* vector);


/*! Convert a vector to a pixel location
@param transform A pointer to a tranform object previously created with NB_VectorTransformCreate()
@param vector The vector to convert
@param pixel The converted vector
@return NB_Error
*/
NB_DEC NB_Error NB_VectorTransform3DToPixel(NB_VectorTransform* transform, NB_Vector* vector, NB_Point* pixel);


/*! Determine if a tile is visible
@param transform A pointer to a tranform object previously created with NB_VectorTransformCreate()
@param xTileIndex Tile index
@param yTileIndex Tile index
@param zoomLevel Tile zoom level
@param minClipZ Minimum clip value
@param maxClipZ Maximum clip value
@return Non-zero if tile is visible in the current transform; zero otherwise
*/
NB_DEC nb_boolean NB_VectorTransformIsTileVisible(NB_VectorTransform* transform, int xTileIndex, int yTileIndex, int zoomLevel, double minClipZ, double maxClipZ);


/*! Convert a pixel distance to a mercator distance
@param transform A pointer to a tranform object previously created with NB_VectorTransformCreate()
@param pixelDistance The pixel distance
@param mercatorDistance The mercator distance
@return NB_Error
*/
NB_DEC NB_Error NB_VectorTransformPixelDistanceToMercator(NB_VectorTransform* transform, int pixelDistance, double* mercatorDistance);

/*! Given an initialized transform, get the distance from the point from the subject of the view to the bottom of the view (used in turnmaps)
@param Distance out parameter if calculation succeeded
@return NE_NOTINIT if not properly initalized, NE_OK otherwise
*/
NB_DEC NB_Error NB_VectorTransformGetNearPointDistanceForCamera(NB_VectorTransform* pThis, double* distance);

#endif

/* @} */
