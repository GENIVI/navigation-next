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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */


#include "paltypes.h"
#include "nbcontext.h"
#include "nbcontextprotected.h"
#include "nbcameraprocessor.h"
#include "nbcameraprocessorprivate.h"
#include "nbcamerainformationprivate.h"
#include "nbnavigation.h"
#include "nbnavigationprivate.h"
#include "nbrouteinformationprivate.h"
#include "navtypes.h"
#include "nbspatial.h"
#include "nbutility.h"
#include "palmath.h"
#include "nbnavigationstateprivate.h"
#include "nbguidancemessageprivate.h"
#include "nbqalog.h"
#include "nbnavqalog.h"

static void NB_CameraProcessorUpdateCameraPosition(NB_NavigationState* pNav,  NB_CameraState* cameraState, NB_CameraInformation* cameraInformation, NB_PositionRoutePositionInformation* ppos);

NB_Error
NB_CameraProcessorStateCreate(NB_Context* context, NB_CameraState** cameraState)
{
    NB_CameraState* pThis = NULL;
    
    if (!context)
    {
        return NE_INVAL;
    }
    
    if ((pThis = nsl_malloc(sizeof(*pThis))) == NULL)
    {
        return NE_NOMEM;
    }

    nsl_memset(pThis, 0, sizeof(*pThis));
   
    pThis->context = context;
    pThis->currentCameraIndex        = 0;
    pThis->distanceToCurrentCamera = -1.0;
    pThis->messageCameraPending = FALSE;
    pThis->messageCameraReported = FALSE;

    *cameraState = pThis;

    return NE_OK;
}

NB_Error
NB_CameraProcessorStateDestroy(NB_CameraState* cameraState)
{
    if (!cameraState)
    {
        return NE_INVAL;
    }

    nsl_free(cameraState);

    return NE_OK;
}

NB_DEF
NB_Error NB_CameraProcessorUpdate(NB_CameraState* cameraState, NB_NavigationState* navigationState, NB_CameraInformation* cameraInformation, NB_PositionRoutePositionInformation* currentPosition)
{
    NB_CameraProcessorUpdateCameraPosition(navigationState, cameraState, cameraInformation, currentPosition);
    return NE_OK;
}


NB_DEF nb_boolean
NB_CameraProcessorIsAnnouncementPending(NB_CameraState* cameraState)
{
    if (!cameraState)
    {
        return FALSE;
    }

    return cameraState->messageCameraPending;
}

static void
NB_CameraProcessorUpdateCameraPosition(NB_NavigationState* navigationState, NB_CameraState* cameraState, NB_CameraInformation* cameraInformation, NB_PositionRoutePositionInformation* ppos)
{
    uint32 i = 0;
    uint32 count = 0;
    double announceDistance = 0.0;
    double speed = 0.0;
    double routeLength = 0.0;
    NB_RoutePrivateConfiguration* privateConfig = 0;
    NB_Error err = NE_OK;

    if (NB_CameraInformationGetCameraCount(cameraInformation, &count) || count <= 0)
    {
        return;
    }

    cameraState->messageCameraPending = FALSE;

    privateConfig = NB_RouteInformationGetPrivateConfiguration(navigationState->navRoute);

    speed = (navigationState->currentSpeed >= navigationState->currentRoutePosition.maneuver_base_speed) ? navigationState->currentSpeed : navigationState->currentRoutePosition.maneuver_base_speed;
    err = NB_NavigationGetRouteLength(navigationState, &routeLength);

    for (i = cameraState->currentCameraIndex; i < count; i++)
    {
        double distance = 0;
        if (NB_CameraInformationGetCameraDistance(cameraInformation, i, &distance) == NE_OK)
        {
            cameraState->distanceToCurrentCamera = distance - (routeLength - ppos->remainingRouteDist);
            if (cameraState->distanceToCurrentCamera > 0.0) //found the next camera
            {
                if (cameraState->currentCameraIndex != i)
                {
                    /* next camera is changed */
                    cameraState->currentCameraIndex = i;
                    cameraState->messageCameraReported = FALSE; /* has not warned about this camera */
                    cameraState->messageCameraPlayed = FALSE;
                }

                announceDistance = NB_RouteInformationGetInstructionDistance(&privateConfig->prefs, speed, maneuver_type_camera, instruct_dist);
                announceDistance = MAX(announceDistance, privateConfig->prefs.minCameraInstructDistance);

                if (cameraState->distanceToCurrentCamera > 0.0 && cameraState->distanceToCurrentCamera < announceDistance)
                {
                    if (!cameraState->messageCameraReported)
                    {
                        NB_Camera camera = {0};

                        /* has not warn about this camera */
                        cameraState->messageCameraPending = TRUE;
                        cameraState->messageCameraReported = TRUE;

                        err = NB_CameraInformationGetCamera(cameraInformation, i, &camera);
                        if (err == NE_OK)
                        {
                            NB_QaLogCameraWarning(navigationState->context, cameraState->distanceToCurrentCamera, &camera);
                        }

                        NB_QaLogGuidanceMessageTrigger(navigationState->context, NB_NAS_Automatic, NB_GMT_Camera,
                                announceDistance, cameraState->distanceToCurrentCamera, speed, -1);
                    }
                }

                break;
            }
        }
    }
}

NB_Error
NB_CameraProcessorStateReset(NB_CameraState* cameraState)
{
    if (!cameraState)
        return NE_INVAL;

    cameraState->distanceToCurrentCamera = -1.0;
    cameraState->currentCameraIndex        = 0;
    cameraState->messageCameraPending = FALSE;
    cameraState->messageCameraReported = FALSE;
    cameraState->messageCameraPlayed = FALSE;

    return NE_OK;
}

NB_DEF NB_Error
NB_CameraProcessorGetNextCamera(NB_CameraState* cameraState, uint32* nextCameraIndex, double* distanceToNextCamera)
{
    if (!cameraState)
    {
        return NE_INVAL;
    }

    if (cameraState->distanceToCurrentCamera < 0.0) /* no next camera */
    {
        return NE_NOENT;
    }

    if (nextCameraIndex)
    {
        *nextCameraIndex = cameraState->currentCameraIndex;
    }

    if (distanceToNextCamera)
    {
        *distanceToNextCamera = cameraState->distanceToCurrentCamera;
    }

    return NE_OK;
}


static void
NB_CameraMessagePlayedCallback(NB_GuidanceMessage* message, void* userData)
{
    MessagePlayedCallbackData* messagePlayedCallbackData = userData;
    NB_CameraState* cameraState = messagePlayedCallbackData ? messagePlayedCallbackData->messageState : NULL;
    
    if (!cameraState)
    {
        return;
    }

    if (cameraState->messageCameraPending)
    {
        cameraState->messageCameraPending = FALSE;
        cameraState->messageCameraPlayed = TRUE;
    }

    NB_NavigationMessagePlayed(messagePlayedCallbackData->navigationState, message);
}

NB_DEF NB_Error
NB_CameraGetPendingMessage(NB_CameraState* cameraState, NB_NavigationState* navigationState, NB_GuidanceInformation* guidanceInformation, 
                                            NB_CameraInformation* cameraInformation, NB_NavigateAnnouncementType type, NB_NavigateAnnouncementUnits units, NB_GuidanceMessage** message)
{
    NB_Error error = NE_OK;
    NB_RouteDataSourceOptions dataSourceOptions = {0};
    NB_PositionRoutePositionInformation routePositionInformation = {0};

    if (message == NULL)
    {
        return NE_INVAL;
    }

    *message = NULL;

    if (!NB_CameraProcessorIsAnnouncementPending(cameraState))
    {
        return NE_NOENT;
    }

    error = NB_NavigationStateGetRoutePositionInformation(navigationState, &routePositionInformation);

    dataSourceOptions.route = navigationState->navRoute;
    dataSourceOptions.maneuverPos = NB_NMP_Turn;
    dataSourceOptions.collapse = TRUE;
    dataSourceOptions.currentRoutePosition = &routePositionInformation;
    dataSourceOptions.cameraState = cameraState;

    error = NB_CameraInformationGetAnnouncement(cameraState, guidanceInformation, NB_RouteInformationDataSource, &dataSourceOptions, 
        NAV_MANEUVER_CAMERA, NB_NAS_Automatic, type, units, message);

    if (error == NE_OK && message != NULL)
    {
        navigationState->messageCallbackData.messageState = cameraState;
        navigationState->messageCallbackData.navigationState = navigationState;
        error = NB_GuidanceMessageSetPlayedCallback(*message, NB_CameraMessagePlayedCallback, &navigationState->messageCallbackData);
    }

    return error;
}
