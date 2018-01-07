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

    @file     nbcameraprocessorprivate.h
       
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

#ifndef NBCAMERAPROCESSORPRIVATE_H
#define NBCAMERAPROCESSORPRIVATE_H

#include "paltypes.h"
#include "nbnavigationstate.h"
#include "navtypes.h"

/*!
    @addtogroup nbcameraprocessor
    @{
*/

struct NB_CameraState
{
    NB_Context*                 context;
    nb_boolean                  messageCameraPending;
    nb_boolean                  messageCameraPlayed;
    nb_boolean                  messageCameraReported;
    uint32                      currentCameraIndex;
    double                      distanceToCurrentCamera;

};

/*! Create a NB_CameraState object

@param context NB_Context
@param cameraState On success, a newly created NB_CameraState object; NULL otherwise.  A valid object must be destroyed using NB_CameraState()
@returns NB_Error
*/
NB_Error NB_CameraProcessorStateCreate(NB_Context* context, struct NB_CameraState** cameraState);

/*! Destroy a previously created NB_CameraState object

@param cameraState A NB_CameraState object created with any of the NB_CameraProcessorStateCreate functions
@returns NB_Error
*/
NB_Error NB_CameraProcessorStateDestroy(struct NB_CameraState* cameraState);

NB_Error NB_CameraProcessorStateReset(struct NB_CameraState* cameraState);


/*! @} */

#endif
