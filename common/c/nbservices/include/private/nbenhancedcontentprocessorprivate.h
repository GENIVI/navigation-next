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

    @file     nbenhancedcontentprocessorprivate.h
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

#ifndef ENHANCEDCONTENTPROCESSORPRIVATE_H
#define ENHANCEDCONTENTPROCESSORPRIVATE_H

#include "nbenhancedcontentprocessor.h"
#include "nbenhancedcontentmanager.h"
#include "nbnavigationstate.h"
#include "nbrouteinformationprivate.h"
#include "nbspatial.h"
#include "vec.h"

#define VIEWING_PLANE_POINT_COUNT 4

/*!
    @addtogroup nbenhancedcontentprocessor
    @{
*/


/*! ContentRegion 

    Wrapper around NB_RouteContentRegion containing some extra computated data.
    This also avoids having to get the NB_RouteContentRegion over and over again from the route information.
*/
typedef struct 
{
    uint32                  lastShowTimestampSec;   /*!< Time region last shown */

    NB_RouteContentRegion   region;                 /*!< Content region from route */

} ContentRegion;


/*! CityParameters */
typedef struct 
{
    char*                   datasetId;              /*!< Dataset ID (city name) */
    char*                   version;                /*!< Current version */
    NB_CityBoundingBox      cityBoundingBox;        /*!< Bounding box for entire city */
    
    uint32                  tileBoxesCount;         /*!< Count of city tiles */
    NB_CityBoundingBox*     tileBoxes;              /*!< Array of city tiles */

    uint32                  tileColumnCount;        /*!< Count of tile columns */
    double*                 columnLongitudes;       /*!< Array of tile column longitude values */

    uint32                  tileRowCount;           /*!< Count of tile rows */
    double*                 rowLatitudes;           /*!< Array of tile row latitude values */

} CityParameters;


/*! Data object to hold the parameters that define the camera projection.

    These parameters only need to get recalculated if we switch between portrait and landscape view.
    They are used to calculate the CameraProjectionContext.

    @see CameraProjectionContext
*/
typedef struct 
{
    int cameraHeightMeters;
    int cameraToAvatarHorizontalMeters;
    int avatarToHorizonHorizontalMeters;
    
    int cameraToFrustumStartMeters;
    int cameraToHorizonMeters;
    
    int cameraToFrustumStartHorizontalMeters;
    int cameraToFrustumEndHorizontalMeters;
    
    double tangentHalfHorizontalFieldOfView;        // Tangent of half the field-of-view angle
    double aspectRatio;                             // Aspect ratio of screen (screen height / screen width)

} CameraProjectionParameters;    


/*! Camera projection context.

    I think this needs to be calculated for every frame.
*/
typedef struct 
{
    /* Calculated parameters using the fixed parameters */
    double horizontalAvatarOffset;  // horizontal camera distance from user position

    double rotateYSine;             // sin(yCamRotate)
    double rotateYCosine;           // cos(yCamRotate)

    double cosLatitude;             // cos(user latitude)
    double cosLatRadius;            // cosLatitude * Earth radius

    double dxAvatar;                // avatar projection on x axis
    double dzAvatar;                // avatar projection on z axis
    
    double dxMinViewDist;           // hMinViewDist projection on x axis
    double dzMinViewDist;           // hMinViewDist projection on z axis

    double dxMaxViewDist;           // hMaxViewDist projection on x axis
    double dzMaxViewDist;           // hMaxViewDist projection on z axis
    
    double widthMinViewPlane;       // width of the nearest view plane
    double widthMaxViewPlane;       // width of the nearest view plane

} CameraProjectionContext; 


/*!
    @todo: Not sure how to organize this
*/
typedef struct 
{
    /*! The viewing plane is the projection of the viewing-frustum to the x/y-plane.
        The z-values are always zero. The values are in mercator coordinates.
    
        @todo: I think we could pass this as a parameter instead of making it a member here.
    */
    NB_Vector viewingPlane[VIEWING_PLANE_POINT_COUNT];

} NB_MapFrameData;


/*! Enhanced Content State for Enhanced Content Processor */
struct NB_EnhancedContentState
{
    NB_Context*                 context;                        /*!< Reference to current context */
    NB_EnhancedContentManager*  enhancedContentManager;         /*!< Reference to Enhanced Content Manager */

    NB_EnhancedContentMapConfiguration  configuration;          /*!< Enhanced content configuration */
    nb_boolean                          configured;             /*!< Configuration parameters set? */
    NB_MapOrientation                   orientation;            /*!< Current enhanced content map orientation */

    /* Enhanced Content state info from last update call */
    NB_EnhancedContentStateData availableContent;               /*!< Bit field of available content for next callback/update */
    nb_boolean                  initialUpdate;                  /*!< Is this the first update since reset? */
    nb_boolean                  stateChanged;                   /*!< Has state changed from last update? */
    nb_boolean                  mapFrameTilesAvailable;         /*!< Set to TRUE to indicate that we have all tiles for the current/last frame. 
                                                                     Used to determine if we need to call the callback/content update or not */
    uint32                      currentManeuver;                /*!< Current maneuver; set at each update */
    uint32                      lastPrefetchManeuver;           /*!< Maneuver of last sign/junction prefetch */

    /* Current realistic sign info */
    NB_RealisticSign            currentSign;                    /*!< Current sign information and data */
    nb_boolean                  needToFreeSign;                 /*!< Does sign need to be freed on clear? */

    /* Content Regions data */
    CSL_Vector*                 contentRegions;                 /*!< All content regions for this route. Gets updated if the route changes. 
                                                                     Contains 'ContentRegion' */
    int                         currentRegionIndex;             /*!< Index of current region. Set to '-1' if we're not in a region */
    NB_RouteId                  routeId;                        /*!< Route ID of current route. If it changes then we need to refresh our regions */
    uint32                      lastRegionShowTimestampSec;     /*!< Time last content region was shown */

    /* Current map frame data */
    CSL_Vector*                 currentTiles;                   /*!< Tiles needed for current map frame */
    CSL_Vector*                 currentSplines;                 /*!< Path splines for current map frame */
    
    NB_MapFrameData             mapFrameData;                   /*!< Map frame data for client */

    /* Current city info */
    CityParameters*             currentCityParameters;          /*!< Parameters for current city */
    
    CameraProjectionContext     cameraContext;                  /*!< I think this needs to be calculated for every frame. Move somewhere else? */
    CameraProjectionParameters  cameraParameters;               /*!< I think this only needs to be calculated once */
};


/*! Create a NB_EnhancedContentState object

@returns NB_Error
*/
NB_Error NB_EnhancedContentProcessorStateCreate(NB_Context* context, NB_EnhancedContentState** enhancedContentState);


/*! Destroy a previously created NB_EnhancedContentState object

@returns NB_Error
*/
NB_Error NB_EnhancedContentProcessorStateDestroy(NB_EnhancedContentState* enhancedContentState);


/*! Reset Enhanced Content state object

@returns NB_Error
*/
NB_Error NB_EnhancedContentProcessorStateReset(NB_EnhancedContentState* enhancedContentState);


/*! Set Enhanced Content configuration

@returns NB_Error
*/
NB_Error NB_EnhancedContentProcessorSetConfiguration(NB_EnhancedContentState* enhancedContentState, const NB_EnhancedContentMapConfiguration* configuration);


/*! Set Enhanced Content map orientation

@returns NB_Error
*/
NB_Error NB_EnhancedContentProcessorSetMapOrientation(NB_EnhancedContentState* enhancedContentState, NB_MapOrientation orientation);


/*! Update Enhanced Content state at position change

@returns NB_Error
*/
NB_Error NB_EnhancedContentProcessorUpdate(NB_EnhancedContentState* enhancedContentState, NB_NavigationState* navigationState);


/*! @} */

#endif
