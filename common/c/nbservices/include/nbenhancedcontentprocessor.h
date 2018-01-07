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

    @file     nbenhancedcontentprocessor.h
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.             

    The information contained herein is confidential, proprietary 
    to TeleCommunication Systems, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

#ifndef ENHANCEDCONTENTPROCESSOR_H
#define ENHANCEDCONTENTPROCESSOR_H

#include "nbrouteinformation.h"
#include "nbenhancedcontenttypes.h"

/*!
    @addtogroup nbenhancedcontentprocessor
    @{
*/


// Public Types ..................................................................................

/*! @struct NB_CameraProjectionParameters */
typedef struct NB_CameraProjectionParameters
{
    int32 cameraHeightMeters;               /*!< */
    int32 cameraToAvatarHorizontalMeters;   /*!< */
    int32 avatarToHorizonHorizontalMeters;  /*!< */
    int32 horizontalFieldOfViewDegrees;     /*!< */
    int32 avatarToMapBottomPixel;           /*!< */

} NB_CameraProjectionParameters;


/*! @enum NB_MapOrientation */
typedef enum
{
    NB_MO_Portrait = 0,                     /*!< */
    NB_MO_Landscape = 1,                    /*!< */

} NB_MapOrientation;


/*! @struct NB_EnhancedContentMapConfiguration */
typedef struct NB_EnhancedContentMapConfiguration
{
    NB_CameraProjectionParameters cameraProjection[2];  /*!< Camera projection for NB_MO_Portrait & NB_MO_Landscape */

    uint32      mapFieldWidthPixel;                     /*!< @todo: */
    uint32      mapFieldHeightPixel;                    /*!< @todo: */
    
    uint32      waitBetweenCityModelDisplaySeconds;     /*!< Minimum time between showing enhanced city model */
    uint32      maximumJunctionViewDistanceMeters;      /*!< Maximum view distance for motorway junction. Used to exit MJO regions. */
    uint32      maximumCameraToAvatarDistanceMeters;    /*!< Maximum camera to avatar distance for all zoom levels. Used to enter MJO regions. */
    
} NB_EnhancedContentMapConfiguration;


/*! Enhanced Content Data available 

    Bit flags indicating enhanced content that is currently available.

    Note: NB_ECSD_JunctionMapFrame and NB_ECSD_CityModelMapFrame are mutually exclusive.
*/
typedef enum
{
    NB_ECSD_None                    = 0,        /*!< No enhanced content available */
    NB_ECSD_RealisticSign           = 1 << 0,   /*!< Realistic sign available for display */
    NB_ECSD_JunctionMapFrame        = 1 << 1,   /*!< Junction map frame data available for display */
    NB_ECSD_CityModelMapFrame       = 1 << 2,   /*!< City Model map frame data available for display */
    
    NB_ECSD_SynchronizationNeeded   = 1 << 3,   /*!< Data synchronization needed due to manifest version change */

} NB_EnhancedContentStateData;


/*! @struct NB_RealisticSign */
typedef struct NB_RealisticSign
{
    const char* signId;                 /*!< Realistic sign ID */
    uint32 dataSize;                    /*!< Realistic sign image data length */
    uint8* data;                        /*!< Realistic sign image data */

    const char* portraitSignId;         /*!< Portrait realistic sign ID */
    uint32 portraitDataSize;            /*!< Portrait realistic sign image data length */
    uint8* portraitData;                /*!< Portrait realistic sign image data */

    uint32      maneuver;               /*!< Maneuver index of the sign */

} NB_RealisticSign;


/*! @struct NB_MapFrameSpline */
typedef struct NB_MapFrameSpline
{
    const char*     pathId;             /*!< Path ID */
    uint32          splineSize;         /*!< Size of spline */
    const uint8*    spline;             /*!< Route packed spline */

} NB_MapFrameSpline;


/*! @struct NB_MapFrameProjectionParameters */
typedef struct NB_MapFrameProjectionParameters
{
    NB_EnhancedContentProjectionType    projection; /*!< Projection type */
    NB_EnhancedContentProjectionDatum   datum;      /*!< Projection datum */

    double      originLatitude;         /*!< Latitude of origin for the projection. */
    double      originLongitude;        /*!< Longitude of the origin of the projection. */
    double      scaleFactor;            /*!< Always 0.9996 for UTM. */
    double      falseEasting;           /*!< 500000.0 for UTM. */
    double      falseNorthing;          /*!< 10000000.0 for UTM in the southern hemisphere. */
    double      zOffset;                /*!< A constant value to add to all z-coordinates in the model to
                                             obtain the actual height off the referenced ellipsoid. */
} NB_MapFrameProjectionParameters;

/*! @struct NB_EnhancedContentState

*/
typedef struct NB_EnhancedContentState NB_EnhancedContentState;


// Public Functions ..............................................................................

/*! Get current realistic sign information and data

Returns the current realistic sign and image(s).  Note that caller takes ownership of the sign
data, and should free it when no longer needed by calling NB_EnhancedContentFreeRealisticSign().

@return NE_OK for success
@see NB_EnhancedContentStateFreeRealisticSign
*/
NB_DEC NB_Error NB_EnhancedContentStateGetRealisticSign(
    NB_EnhancedContentState* enhancedContentState,  /*!< Enhanced Content processor state */
    NB_RealisticSign* realisticSign                 /*!< On return current realistic sign data */
    );


/*! Free realistic sign data

Frees the data for the indicated realistic sign.

@return NE_OK for success
@see NB_EnhancedContentStateGetRealisticSign
*/
NB_DEC NB_Error NB_EnhancedContentStateFreeRealisticSign(
    NB_EnhancedContentState* enhancedContentState,  /*!< Enhanced Content processor state */
    NB_RealisticSign* realisticSign                 /*!< Realistic sign data to free */
    );


/*! Get array of tile IDs for current map frame

Caller should copy the IDs returned for use beyond the current state of the enhanced content
processor (i.e. before next fix).

@return NE_OK for success
*/
NB_DEC NB_Error NB_EnhancedContentStateGetMapFrameTileIds(
    NB_EnhancedContentState* enhancedContentState,  /*!< Enhanced Content processor state */
    uint32* tileCount,                              /*!< On return count of tile IDs in array*/
    const char*** tileIds                           /*!< On return array of tile IDs */
    );


/*! Get array of route splines for current map frame

Caller should copy the splines returned for use beyond the current state of the enhanced content
processor (i.e. before next fix).

@return NE_OK for success
*/
NB_DEC NB_Error NB_EnhancedContentStateGetMapFrameSplines(
    NB_EnhancedContentState* enhancedContentState,  /*!< Enhanced Content processor state */
    uint32* splineCount,                            /*!< On return count of splines in array */
    const NB_MapFrameSpline** splines               /*!< On return array of splines */
    );


/*! Get start and end maneuver indexes for current map frame

@return NE_OK for success
*/
NB_DEC NB_Error NB_EnhancedContentStateGetMapFrameManeuvers(
    NB_EnhancedContentState* enhancedContentState,          /*!< Enhanced Content processor state */
    uint32* startManeuver,                                  /*!< On return start maneuver index for current map frame */
    uint32* endManeuver                                     /*!< On return end maneuver index for current map frame */
    );


/*! Get projection parameters for current map frame

For motorway junctions, the projection parameters will be from the content region in the route reply.
For city models, the projection parameters will be from the metadata for the current city.

@return NE_OK for success
*/
NB_DEC NB_Error NB_EnhancedContentStateGetMapFrameProjectionParameters(
    NB_EnhancedContentState* enhancedContentState,          /*!< Enhanced Content processor state */
    NB_MapFrameProjectionParameters* projectionParameters   /*!< On return projection parameters for current map frame */
    );


/*! @} */

#endif
