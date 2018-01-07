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

    @file     nbguidanceinfo.c
    @defgroup nbguidanceinfo

    Information about guidance
*/
/*
    See file description in header file.

    (C) Copyright 2004 - 2009 by Networks In Motion, Inc.                

    The information contained herein is confidential, proprietary 
    to Networks In Motion, Inc., and considered a trade secret as 
    defined in section 499C of the penal code of the State of     
    California. Use of this information by anyone other than      
    authorized employees of Networks In Motion is granted only    
    under a written non-disclosure agreement, expressly           
    prescribing the scope and manner of such use.                 

---------------------------------------------------------------------------*/

/*! @{ */

#include "palmath.h"

#include "nbcontext.h"
#include "nbcontextprotected.h"
#include "nbcamerainformation.h"
#include "nbcamerainformationprivate.h"
#include "nbrouteinformationprivate.h"
#include "nbnavigationstate.h"
#include "instructset.h"
#include "navutil.h"
#include "nbutility.h"
#include "data_blob.h"
#include "nbsearchinformation.h"
#include "nbsearchinformationprivate.h"
#include "nbcameraprocessorprivate.h"
#include "nbguidanceinformationprivate.h"
#include "nbguidancemessageprivate.h"
#include "nbnavigationstateprivate.h"

struct NB_CameraInformation
{
    NB_Context*   context;
    struct CSL_Vector*    vec_cameras;          // NB_Camera container
};

static NB_CameraInformation* AllocateCameraInformation(NB_Context* context);
static NB_Camera* GetCameraByIndex(NB_CameraInformation* information, uint32 index);

NB_DEF NB_Error 
NB_CameraInformationDestroy(NB_CameraInformation* information )
{
    if (!information)
    {
        return NE_INVAL;
    }

    if (information->vec_cameras) 
    {
        CSL_VectorDealloc(information->vec_cameras);
    }

    nsl_free(information);

    return NE_OK;
}

NB_DEF NB_Error
NB_CameraInformationCreateFromSearch(NB_Context* context, NB_SearchInformation* search, NB_CameraInformation** information)
{
    int i = 0;
    int count = 0;
    data_util_state* dataState = NULL;
    NB_CameraInformation* pThis = 0;
    NB_SearchResultType type = NB_SRT_None;
    NB_Error err = NE_OK;

    if (!context || !search || !information)
    {
        return NE_INVAL;
    }

    pThis = AllocateCameraInformation(context);
    if (!pThis)
    {
        return NE_NOMEM;
    }
    *information = pThis;

    err = NB_SearchInformationGetResultType(search, &type);
    err = err ? err : NB_SearchInformationGetResultCount(search, &count);
    if (err || ((type & NB_SRT_SpeedCamera) == 0) || count == 0)
    {
        // Return empty camera information if could not get result type or no cameras
        return NE_OK;
    }

    dataState = NB_ContextGetDataState(context);

    for (i = 0; i < count; i++)
    {
        NB_Camera camera = { 0 };

        data_place* place = NB_SearchInformationGetCameraPlace(search, i, &camera.distance);
        if (place && (place->elements & PlaceElement_SpeedCamera))
        {
            SetNIMPlaceFromPlace(&camera.place, dataState, place);

            /* copy speed camera information */
            camera.speedLimit = place->camera.limit;
            camera.heading = place->camera.heading;
            camera.isBidirectional = place->camera.bi_directional;
            nsl_strlcpy(camera.description, data_string_get(dataState, &place->camera.description), sizeof(camera.description));

            /* convert speed camera status from string to enum */
            if (nsl_strcmp(place->camera.status, "A") == 0)
            {
                camera.status = NB_CS_Active;
            }
            else if (nsl_strcmp(place->camera.status, "L") == 0)
            {
                camera.status = NB_CS_Empty;
            }
            else if (nsl_strcmp(place->camera.status, "Z") == 0)
            {
                camera.status = NB_CS_Demolished;
            }
            else
            {
                camera.status = NB_CS_Invalid;
            }

            if (!CSL_VectorAppend(pThis->vec_cameras, &camera))
            {
                NB_CameraInformationDestroy(pThis);
                return NE_NOMEM;
            }
        }
    }

    return NE_OK;
}

NB_DEF 
NB_Error NB_CameraInformationGetCameraCount(NB_CameraInformation* information, uint32* count)
{    
    if (!information || !count)
    {
        return NE_INVAL;
    }

    *count = (uint32) CSL_VectorGetLength(information->vec_cameras);
    
    return NE_OK;
}

NB_DEF NB_Error 
NB_CameraInformationGetCamera(NB_CameraInformation* information, uint32 index, NB_Camera* cameraOut)
{
    NB_Camera* camera = 0;

    if (!information || !cameraOut)
    {
        return NE_INVAL;
    }
    if (CSL_VectorGetLength(information->vec_cameras) <= 0)
    {
        return NE_INVAL;
    }
    
    camera = GetCameraByIndex(information, index);
    
    if (camera)
    {
        nsl_memcpy(cameraOut, camera, sizeof(*camera));
    }
    else
    {
        return NE_INVAL;
    }
    return NE_OK;
}

NB_Error
NB_CameraInformationGetCameraDistance(NB_CameraInformation* information, uint32 index, double* distance)
{
    NB_Camera* camera = 0;

    if (!information || !distance)
    {
        return NE_INVAL;
    }
    
    camera = GetCameraByIndex(information, index);
    if (camera)
    {
        *distance = camera->routeDistance;
    }
    else
    {
        return NE_INVAL;
    }

    return NE_OK;
}


static NB_CameraInformation* AllocateCameraInformation(NB_Context* context)
{
    NB_CameraInformation* pThis = NULL;

    if ((pThis = nsl_malloc(sizeof(NB_CameraInformation))) == NULL)
    {
        return NULL;
    }

    nsl_memset(pThis, 0, sizeof(NB_CameraInformation));

    pThis->vec_cameras = CSL_VectorAlloc(sizeof(NB_Camera));

    return pThis;
}

static NB_Camera* 
GetCameraByIndex(NB_CameraInformation* information, uint32 index)
{
    if (!information)
    {
        return NULL;
    }

    if (index > (uint32) CSL_VectorGetLength(information->vec_cameras))
    {
        return NULL;
    }

    return (NB_Camera*) CSL_VectorGetPointer(information->vec_cameras, index);
}

NB_Error 
NB_CameraInformationGetAnnouncement(NB_CameraState* cameraState, NB_GuidanceInformation* information, 
                                    NB_GuidanceDataSourceFunction dataSource, void* dataSourceUserData, uint32 index, 
                                    NB_NavigateAnnouncementSource source, NB_NavigateAnnouncementType type, NB_NavigateAnnouncementUnits units, NB_GuidanceMessage** message)
{
    NB_Error err = NE_OK;
    NB_GuidanceMessage* cameraMessage = NULL;
    InstructionCodes* codes = NULL;
    instructset* voices = NULL;

    if ( !information || !dataSource )
    {
        return NE_INVAL;
    }

    err = NB_GuidanceMessageCreate(cameraState->context, source, NB_GMT_Camera, &cameraMessage);

    if (err != NE_OK)
    {
        return err;
    }

    *message = cameraMessage;

    codes = NB_GuidanceMessageGetInstructionCodes(cameraMessage);

    err = NB_GuidanceInformationSetDataSourceFunction(information, dataSource, dataSourceUserData);

    NB_GuidanceInformationGetVoices(information, &voices);

    if (err == NE_OK)
    {
        err = nb_instructset_getdata(voices, index, codes, NULL, NULL, NULL,
            it_audio, type, FALSE, units, NB_NMP_Turn, NULL);
    }

    if (err != NE_OK)
    {
        if (cameraMessage)
        {
            NB_GuidanceMessageDestroy(cameraMessage);
            *message = NULL;
        }
    }

    return err;
}

static int
CompareCameraRouteDistance(const void* left, const void* right)
{
    const NB_Camera* camera1 = (const NB_Camera*)left;
    const NB_Camera* camera2 = (const NB_Camera*)right;

    if (camera1->routeDistance < camera2->routeDistance)
    {
        return -1;
    }
    else if (camera1->routeDistance > camera2->routeDistance)
    {
        return 1;
    }

    return 0;
}

NB_Error
NB_CameraInformationUpdateCameraDistances(NB_CameraInformation* information, NB_NavigationState* navigationState)
{
    uint32 count = 0;
    uint32 i = 0;
    NB_Error err = NE_OK;
    NB_Camera tempSortCamera = {0};

    if (information == NULL)
    {
        return NE_INVAL;
    }

    if (NB_CameraInformationGetCameraCount(information, &count) || count <= 0)
    {
        /* no camera */
        return NE_OK;
    }

    for (i = 0; i < count; i++)
    {
        NB_LatitudeLongitude location = { 0 };
        NB_Camera* camera = GetCameraByIndex(information, i);

        location.latitude = camera->place.location.latitude;
        location.longitude = camera->place.location.longitude;
        err = err? err : NB_RouteDistanceToClosestPoint(navigationState, &location, &camera->nman_closest,
                                                        &camera->nseg_closest, &camera->routeDistance);
    }

    /* sort camera by route distance */
    CSL_VectorSort2(information->vec_cameras, CompareCameraRouteDistance, &tempSortCamera);

    return err;
}

NB_DEF NB_Error
NB_CameraInformationGetManeuverCameraCount(NB_CameraInformation* information,
                                           uint32 maneuver,
                                           uint32* count)
{
    uint32 cameraCount = 0;
    uint32 i = 0;
    uint32 result = 0;
    NB_Error err = NE_OK;

    if ((information == NULL) || (count == NULL))
    {
        return NE_INVAL;
    }

    err = NB_CameraInformationGetCameraCount(information, &cameraCount);
    if (err != NE_OK)
    {
        return err;
    }

    for(i=0; i < cameraCount; i++)
    {
        NB_Camera* camera = GetCameraByIndex(information, i);
        if (camera->nman_closest == maneuver)
        {
            result++;
        }
    }

    *count = result;
    return NE_OK;
}


NB_DEF NB_Error
NB_CameraInformationGetManeuverCamera(NB_CameraInformation* information,
                                      uint32 maneuver,
                                      uint32 index,
                                      NB_Camera* result)
{
    uint32 cameraCount = 0;
    uint32 maneuverCameraIndex = 0;
    uint32 maneuverCameraCount = 0;
    uint32 i = 0;
    NB_Error err = NE_OK;

    if ((information == NULL) || (result == NULL))
    {
        return NE_INVAL;
    }

    err = NB_CameraInformationGetManeuverCameraCount(information, maneuver, &maneuverCameraCount);
    if (err != NE_OK)
    {
        return err;
    }

    if (index >= maneuverCameraCount)
    {
        return NE_NOENT;
    }

    err = NB_CameraInformationGetCameraCount(information, &cameraCount);
    if (err != NE_OK)
    {
        return err;
    }

    for(i=0; i < cameraCount; i++)
    {
        NB_Camera* camera = GetCameraByIndex(information, i);
        if (camera->nman_closest == maneuver)
        {
            if (maneuverCameraIndex == index)
            {
                *result = *camera;
                return NE_OK;
            }

            maneuverCameraIndex++;
        }
    }

    return NE_NOENT;
}
/*! @} */
