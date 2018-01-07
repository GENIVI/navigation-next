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

    @file     nbcameraprocessor.h
       
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

#ifndef CAMERAPROCESSOR_H
#define CAMERAPROCESSOR_H

#include "paltypes.h"
#include "nbnavigationstate.h"
#include "nbcamerainformation.h"

/*!
    @addtogroup nbcameraprocessor
    @{
*/


/*! @struct NB_CameraState
Camera state
*/
typedef struct NB_CameraState NB_CameraState;


/*! Update the traffic camera state

@param state NB_CameraState
@param location Current GPS position
@returns NB_Error
*/
//NB_DEC NB_Error NB_CameraProcessorUpdate(NB_Navigation* navigation, NB_GpsLocation* location);

/*! Updates the Camera state using the route position

@param cameraState Previously created NB_CameraState object
@param navigation Navigation Session object
@param currentRoutePosition Current route position
@returns NB_Error
*/
NB_DEC NB_Error NB_CameraProcessorUpdate(NB_CameraState* cameraState, NB_NavigationState* navigationState, NB_CameraInformation* cameraInformation, NB_PositionRoutePositionInformation* currentPosition);

/*! Gets next camera on route

@param cameraState Previously created NB_CameraState object
@param camera NB_Camera pointer
@param nextCameraIndex the index of the next camera
@param distanceToNextCamera the distance to the next camera
@return NB_Error NE_NOENT if there is no next camera
*/
NB_DEC NB_Error NB_CameraProcessorGetNextCamera(NB_CameraState* cameraState, uint32* nextCameraIndex, double* distanceToNextCamera);

/*! Checks to see if audio message requirements are met

@param cameraState Previously created NB_CameraState object
@return TRUE if camera audio is pending
*/
NB_DEC nb_boolean NB_CameraProcessorIsAnnouncementPending(NB_CameraState* cameraState);


/*! Retrieve the pending guidance message

*/
NB_DEC NB_Error
NB_CameraGetPendingMessage(NB_CameraState* cameraState, NB_NavigationState* navigationState, NB_GuidanceInformation* guidanceInformation, 
                           NB_CameraInformation* cameraInformation, NB_NavigateAnnouncementType type, NB_NavigateAnnouncementUnits units, NB_GuidanceMessage** message);

/*! @} */

#endif
