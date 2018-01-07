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

    @file     nbcamerainformation.h    
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

#ifndef CAMERAINFORMATION_H
#define CAMERAINFORMATION_H

#include "nbrouteinformation.h"
#include "nbsearchinformation.h"

/*!
    @addtogroup nbcamerainformation
    @{
*/

/*! @struct NB_CameraInformation
Information about the speed cameras on a route
*/
typedef struct NB_CameraInformation NB_CameraInformation;

#define CAMERA_DESCRIPTION_SIZE 128

/*! @enum NB_CameraStatus
Speed Camera Status
 */
typedef enum {
    NB_CS_Invalid = 0,
    NB_CS_Active,
    NB_CS_Empty,
    NB_CS_Demolished,
} NB_CameraStatus;

/*! @struct NB_Camera
Information about a speed camera
*/
typedef struct
{
    double      distance;                       /*!< LOS distance to beginning of route */
    NB_Place    place;                          /*!< place associated with the camera location */
    double      speedLimit;                     /*!< Speed limit in m/s */
    double      heading;                        /*!< Camera direction in degrees (0->360) */
    nb_boolean  isBidirectional;                /*!< True if  bi-directional, false if not */
    NB_CameraStatus status;                     /*!< Speed Camera status */
    char        description[CAMERA_DESCRIPTION_SIZE]; /*!< Additional text description of this camera */

    /* Extra Calculated Data */
    double      routeDistance;                  /*!< route distance from beginning of route */
    uint32      nman_closest;                   /*!< the closest manuver to the camera */
    uint32      nseg_closest;                   /*!< the close maneuver segment to the camera */
} NB_Camera;


/*! Create an information object from a previously returned search object

@param context NAVBuilder context instance
@param search The search object containing camera information
@param information On success, a newly created NB_CameraInformation object; NULL otherwise.  A non-NULL object must be destroyed with NB_CameraInformationDestroy
@returns NB_Error
*/
NB_DEC NB_Error NB_CameraInformationCreateFromSearch(NB_Context* context, NB_SearchInformation* search, NB_CameraInformation** information);

/*! Get camera place information

@param information A NB_CameraInformation object
@param index Index of the camera place information
@param camera On success, receives the camera information
@returns NB_Error
*/
NB_DEC NB_Error NB_CameraInformationGetCamera(NB_CameraInformation* information, uint32 index, NB_Camera* camera);

/*! Get number of cameras

@param information A NB_CameraInformation object
@param num the numbers of camera for a particular route.
@returns NB_Error
*/
NB_DEC NB_Error NB_CameraInformationGetCameraCount(NB_CameraInformation* information, uint32* count);

/*! Get cameeras count between two manuevers.

@param information A NB_CameraInformation object
@param maneuver The maneuver containing the camera
@param cameraCount .  On success, the number of speed cameras in the specified maneuvers.
@returns NB_Error
 */
NB_DEC NB_Error NB_CameraInformationGetManeuverCameraCount(NB_CameraInformation* information, uint32 maneuver, uint32* cameraCount);


/*! Get a camera between two maneuvers

@param information A NB_CameraInformation object
@param maneuver The maneuver containing the incident
@param index The index of the camera on the maneuver
@param camera A pointer to NB_Camera to receive camera information.
@returns NB_Error
 */
NB_DEC NB_Error NB_CameraInformationGetManeuverCamera(NB_CameraInformation* information, uint32 maneuver, uint32 index, NB_Camera* camera);


/*! Destroy a previously created TrafficInformation object

@param information A NB_CameraInformation object created with NB_RouteHandlerGetCameraInformation()
@returns NB_Error
*/
NB_DEC NB_Error NB_CameraInformationDestroy(NB_CameraInformation* information);

/*! @} */

#endif
