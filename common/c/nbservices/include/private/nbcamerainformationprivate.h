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

    @file     nbcamerainformationprivate.h
    
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

#ifndef CAMERAINFORMATIONPRIVATE_H
#define CAMERAINFORMATIONPRIVATE_H

#include "nbcontext.h"
#include "nbcamerainformation.h"
#include "data_place.h"
#include "nbcameraprocessor.h"
#include "nbguidanceinformation.h"

/*!
    @addtogroup nbcamerainformation
    @{
*/

/*! Gets camera information by index

@param information Previously created NB_CameraInformation objects
@param index Index of the camera
@param camera Pointer to data_place object that the desired camera will be copied to
@returns NB_Error
*/
NB_Error NB_CameraInformationGetCamera(NB_CameraInformation* information, uint32 index, NB_Camera* camera);

/*! Set the camera distances to the distance from the start of the route

@param information The camera information object to update
@param navigationState The navigation state object
@return NB_Error
 */
NB_Error NB_CameraInformationUpdateCameraDistances(NB_CameraInformation* information, NB_NavigationState* navigationState);


/*! Gets camera distance by index

@param information Previously created NB_CameraInformation objects
@param index Index of the camera
@param distance Pointer to a double to receive the distance, in meters, from the start of the route
@returns NB_Error
*/
NB_Error NB_CameraInformationGetCameraDistance(NB_CameraInformation* information, uint32 index, double* distance);


NB_Error 
NB_CameraInformationGetAnnouncement(NB_CameraState* cameraState, NB_GuidanceInformation* information, 
                                    NB_GuidanceDataSourceFunction dataSource, void* dataSourceUserData, uint32 index, 
                                    NB_NavigateAnnouncementSource source, NB_NavigateAnnouncementType type, NB_NavigateAnnouncementUnits units, NB_GuidanceMessage** message);

/*! @} */

#endif
