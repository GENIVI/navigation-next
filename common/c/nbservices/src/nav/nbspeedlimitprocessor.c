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
 @file     nbspeedlimitprocessor.c
 @defgroup speedlimit

 */
/*
 (C) Copyright 2011 by TeleCommunication Systems, Inc.

 The information contained herein is confidential, proprietary
 to TeleCommunication Systems, Inc., and considered a trade secret as
 defined in section 499C of the penal code of the State of
 California. Use of this information by anyone other than
 authorized employees of TeleCommunication Systems, is granted only
 under a written non-disclosure agreement, expressly
 prescribing the scope and manner of such use.

 ---------------------------------------------------------------------------*/
/*! @{ */

#include <stdio.h>
#include <nbcontextaccess.h>
#include "nbnavigation.h"
#include "nbspeedlimitprocessorprivate.h"
#include "nbenhancedcontentprocessorprivate.h"
#include "nbenhancedcontentmanagerprotected.h"
#include "vec.h"
#include "nbspeedlimittypes.h"
#include "nbspeedlimitinformationprivate.h"
#include "palclock.h"
#include "nbnavigationstateprivate.h"
#include "nbspeedlimitinformationprivate.h"
#include "nbqalog.h"
#include "nbnavqalog.h"

// Local function
static void
NB_SpeedLimitMessagePlayedCallback(NB_GuidanceMessage* message, void* userData);

static void
NB_SpeedLimitSchoolZoneMessagePlayedCallback(NB_GuidanceMessage* message, void* userData);

static void
QaLogSpeedlimitSetting(NB_Context* context,
                       NB_SpeedLimitConfiguration * configuration);
static void
QaLogSpeedlimitState(const NB_SpeedLimitState* state,
                     NB_PositionRoutePositionInformation* currentPostion,
                     NB_SpeedLimitInformation* speedLimitInformation,
                     NB_RouteInformation* route);
static void
QaLogSpeedLimitEnterZone(NB_Context* context,
                         const double speedLimit,
                         const double latitude,
                         const double longitude
                         );
static void
QaLogSpeedLimitExitZone(NB_Context* context,
                        const double speedLimit,
                        const double latitude,
                        const double longitude
                        );

static void
QaLogSpeedLimitEnterRegion(NB_Context* context,
                         NB_SpeedRegion* region,
                         const double latitude,
                         const double longitude
                         );
static void
QaLogSpeedLimitOutRegion(NB_Context* context,
                        NB_SpeedRegion* region,
                        const double latitude,
                        const double longitude
                        );

/* See header file for description. */
NB_Error
NB_SpeedLimitProcessorUpdate(NB_SpeedLimitState* speedLimitState,
                             NB_RouteInformation* route,
                             NB_SpeedLimitInformation* information,
                             NB_PositionRoutePositionInformation* currentPosition,
                             double speed,
                             NB_NavigationSpeedLimitCallbackFunctionProtected callback,
                             void* userData
                             )
{
    data_nav_maneuver* maneuver = NULL;
    uint32 closetManeuver = 0;
    double offsetInManeuver = 0;
    double distance = 0;
    uint32 length = 0;
    CSL_Vector* regions;
    NB_Context* context = NULL;
    NB_SpeedRegion* speedRegion = NULL;
    NB_SpeedRegion* tmpRegion = NULL;
    NB_SpeedRegion* schoolZone = NULL;
    uint32 currentSpeedLimitRegionIndex = NAV_MANEUVER_NONE;
    uint32 currentSchoolZoneIndex = NAV_MANEUVER_NONE;
    NB_SpeedLimitStateData speedlimitData;
    nb_boolean isSpeedExceed = FALSE;
    nb_boolean needCallback = FALSE;
    uint32 i = 0;

    //check input valid
    if (speedLimitState == NULL || speedLimitState->context == NULL || route == NULL || information == NULL || currentPosition == NULL)
    {
        return NE_INVAL;
    }
    closetManeuver = currentPosition->closestManeuver;
    // Check if context is valid.
    context = speedLimitState->context;
    if (context == NULL)
    {
        return NE_INVAL;
    }

    context = speedLimitState->context;

    //check configure first, if displayspeedlimit == false, direct return.
    if (!speedLimitState->configure.displaySpeedLimit)
    {
        return NE_OK;
    }

    //get all speedlimit regions
    regions = NULL;
    if (NB_SpeedLimitInformationGetSpeedRegions(information,&regions) == NE_OK)
    {
        length = CSL_VectorGetLength(regions);
    }

    if(length <= 0)
    {
        return NE_OK;  //no speedlimit region
    }

    //get current maneuver
    if (closetManeuver == NAV_MANEUVER_NONE || closetManeuver == NAV_MANEUVER_START)
    {
        return NE_OK;  //if offroute, not show speedlimit sign
    }
    else
    {
        maneuver = NB_RouteInformationGetManeuver(route, closetManeuver);
    }
    //cacluate offset from the maneuver start point
    offsetInManeuver = maneuver->distance - currentPosition->maneuverDistanceRemaining;

    //find if current maneuver have a speedlimit region
    for (i=0; i<length; i++)
    {
        tmpRegion = (NB_SpeedRegion*)CSL_VectorGetPointer(regions,i);

        //find school zone, caculate the distance from current position to school zone
        distance = -1;

        if (tmpRegion->m_speedZoneType != NB_SpeedZoneType_None)
        {
            if (tmpRegion->m_startManeuver > closetManeuver)
            {
                if (tmpRegion->m_startManeuver -1 == closetManeuver)
                {
                    data_nav_maneuver* pManeuver = NB_RouteInformationGetManeuver(route,closetManeuver);
                    if (pManeuver != NULL)
                    {
                        distance = pManeuver->distance;
                    }
                }
                else if (NB_RouteGetDistanceBetweenManeuvers(route, closetManeuver, tmpRegion->m_startManeuver - 1, &distance) != NE_OK)
                {
                    distance = -1;
                }
                if (distance >= 0)
                {
                    distance = distance + tmpRegion->m_startManeuverOffset - offsetInManeuver;
                }
            }
            else if (tmpRegion->m_startManeuver == closetManeuver)
            {
                distance = tmpRegion->m_startManeuverOffset - offsetInManeuver;
                if (distance < 0)
                {
                    distance = 0;
                }
            }
            else if (tmpRegion->m_endManeuver >= closetManeuver)
            {
                distance = 0;
            }

            if (distance >=0 && distance <= tmpRegion->m_warnAhead)
            {
                currentSchoolZoneIndex = i;
                schoolZone = tmpRegion;
            }
        }

        if (tmpRegion->m_startManeuver <= closetManeuver && tmpRegion->m_endManeuver >= closetManeuver)
        {
            //check if current position is in a speedlimit region
            if ((tmpRegion->m_startManeuver == closetManeuver && offsetInManeuver >= tmpRegion->m_startManeuverOffset) ||
                (tmpRegion->m_startManeuver < closetManeuver && tmpRegion->m_endManeuver > closetManeuver) ||
                (tmpRegion->m_endManeuver == closetManeuver && offsetInManeuver < tmpRegion->m_endManeuverOffset))
            {
                if (tmpRegion->m_startManeuver == tmpRegion->m_endManeuver &&
                    (tmpRegion->m_startManeuverOffset > offsetInManeuver ||
                     tmpRegion->m_endManeuverOffset < offsetInManeuver))
                {
                    // out of the tmpRegion
                    QaLogSpeedLimitOutRegion(speedLimitState->context,
                                            tmpRegion,
                                            currentPosition->projLat,
                                            currentPosition->projLon
                                            );
                    continue;
                }

                // into the tmpRegion.
                QaLogSpeedLimitEnterRegion(speedLimitState->context,
                                         tmpRegion,
                                         currentPosition->projLat,
                                         currentPosition->projLon
                                         );

                if (speedRegion == NULL)
                {
                    speedRegion = tmpRegion;
                    currentSpeedLimitRegionIndex = i;
                }
                else
                {
                    if (speedRegion->m_speedLimit > tmpRegion->m_speedLimit)
                    {
                        speedRegion = tmpRegion;
                        currentSpeedLimitRegionIndex = i;
                    }
                    else if (speedRegion->m_speedLimit == tmpRegion->m_speedLimit)
                    {
                        if (speedRegion->m_speedZoneType != NB_SpeedZoneType_SchoolZone && tmpRegion->m_speedZoneType == NB_SpeedZoneType_SchoolZone)
                        {
                            speedRegion = tmpRegion;  //school zone have a highest priority
                            currentSpeedLimitRegionIndex = i;
                        }
                    }
                }//end if(speedRegion....
            }
        }
        else
        {
            // out of the tmpRegion
            QaLogSpeedLimitOutRegion(speedLimitState->context,
                                    tmpRegion,
                                    currentPosition->projLat,
                                    currentPosition->projLon
                                    );
        }
    }
    //update school zone state
    if (currentSchoolZoneIndex == NAV_MANEUVER_NONE)
    {
        speedLimitState->currentSpecialZoneIndex = NAV_MANEUVER_NONE;
        speedLimitState->specialZoneAlertPlayed = FALSE;
        speedLimitState->specialZoneAlertPending = FALSE;
    }
    else if (speedLimitState->currentSpecialZoneIndex != currentSchoolZoneIndex && schoolZone != NULL && !schoolZone->m_haveAlert)
    {
        schoolZone->m_haveAlert = TRUE;
        speedLimitState->currentSpecialZoneIndex = currentSchoolZoneIndex;
        speedLimitState->specialZoneAlertPlayed = FALSE;
    }

    nsl_memset(&speedlimitData, 0, sizeof(NB_SpeedLimitStateData));

    //update speedlimit state and do a callback
    if (currentSpeedLimitRegionIndex == NAV_MANEUVER_NONE)
    {
        //not find speedlimit region
        if (speedLimitState->currentSpeedRegionIndex != NAV_MANEUVER_NONE)
        {
            speedLimitState->currentSpeedRegionIndex = NAV_MANEUVER_NONE;
            speedLimitState->isSpeedExceed = FALSE;
            speedLimitState->speedingAlertTime = 0;
            speedLimitState->speedLimitPictureMissing = FALSE;
            speedLimitState->speedLimitAlertPending = FALSE;
            //callback
            if (callback)
            {
                speedlimitData.limitSpeed = 0;
                speedlimitData.pictureSize = 0;
                speedlimitData.showSchoolZone = FALSE;
                speedlimitData.showSpeedLimit = FALSE;
                speedlimitData.speedlimitPictureData = NULL;
                callback(&speedlimitData, NULL, userData);
            }
        }
    }
    else
    {
        const char* signId  = NULL;
        const char* fileVersion = NULL;
        NB_Error error = NE_OK;
        speedlimitData.speedlimitPictureData = NULL;
        if(speedRegion)
        {
        isSpeedExceed = (speed > speedRegion->m_speedLimit)? TRUE : FALSE;
        }
        if ((speedLimitState->currentSpeedRegionIndex != currentSpeedLimitRegionIndex) ||
            (isSpeedExceed != speedLimitState->isSpeedExceed) ||
            (speedLimitState->speedLimitPictureMissing))
        {
            NB_EnhancedContentManager* manager = NULL;

            if (isSpeedExceed && speedLimitState->configure.speedingDisplayAlert)
            {
                signId = speedRegion->m_signidHighlighted;
                fileVersion = speedRegion->m_versionHighlighted;
            }
            else
            {
                signId = speedRegion->m_signId;
                fileVersion = speedRegion->m_version;
            }

            manager = NB_ContextGetEnhancedContentManager(context);
            if (manager)
            {
                error = NB_EnhancedContentManagerGetDataItem(manager,
                                                                  NB_ECDT_SpeedLimitsSign,
                                                                  speedRegion->m_datasetid,
                                                                  signId,
                                                                  fileVersion,
                                                                  FALSE,
                                                                  &(speedlimitData.pictureSize),
                                                                  &(speedlimitData.speedlimitPictureData));
            if(error != NE_OK)
            {
                speedlimitData.pictureSize = 0;
                speedlimitData.speedlimitPictureData = NULL;
                speedlimitData.showSpeedLimit = FALSE;
            }
            }


            speedLimitState->speedingAlertTime = 0;
            speedLimitState->currentSpeedRegionIndex = currentSpeedLimitRegionIndex;
            speedLimitState->isSpeedExceed = speed > speedRegion->m_speedLimit ? TRUE : FALSE;
            speedlimitData.limitSpeed = speedRegion->m_speedLimit;
            speedlimitData.showSchoolZone = (speedRegion->m_speedZoneType == NB_SpeedZoneType_SchoolZone) ? TRUE : FALSE;
            speedlimitData.showSpeedLimit = TRUE;
            if ((!speedLimitState->speedLimitPictureMissing && !speedlimitData.speedlimitPictureData) ||
                speedlimitData.speedlimitPictureData)
            {
                needCallback = TRUE;
            }
            if (!speedlimitData.speedlimitPictureData)
            {
                speedLimitState->speedLimitPictureMissing = TRUE;
            }
        }
    }

    NB_SpeedLimitAlertUpdate(speedLimitState, route, information, currentPosition, speed);

    if (needCallback && callback)
    {
        callback(&speedlimitData, tmpRegion->m_signId, userData);
        if (speedlimitData.speedlimitPictureData)
        {
            nsl_free(speedlimitData.speedlimitPictureData);
            speedlimitData.speedlimitPictureData = NULL;
            speedLimitState->speedLimitPictureMissing = FALSE;
        }
    }

    return NE_OK;
}

/* See header file for description. */
NB_Error
NB_SpeedLimitAlertUpdate(NB_SpeedLimitState* speedLimitState,
                         NB_RouteInformation* route,
                         NB_SpeedLimitInformation* information,
                         NB_PositionRoutePositionInformation* currentPosition,
                         double speed
                         )
{
    data_nav_maneuver* maneuver = NULL;
    uint32 closetManeuver = currentPosition->closestManeuver;
    double offsetInManeuver = 0;
    uint32 length = 0;
    CSL_Vector* regions = NULL;
    nb_gpsTime currentTime = 0;
    NB_Context* context = NULL;
    NB_SpeedRegion* tmpRegion = NULL;

    //check input valid
    if (speedLimitState == NULL || route == NULL || information == NULL)
    {
        return NE_INVAL;
    }

    // Check if context is valid.
    context = speedLimitState->context;
    if (context == NULL)
    {
        return NE_INVAL;
    }

    //check configure first, if displayspeedlimit == false, direct return.
    if (!speedLimitState->configure.displaySpeedLimit)
    {
        return NE_OK;
    }

    //get all speedlimit regions
    regions = NULL;
    if (NB_SpeedLimitInformationGetSpeedRegions(information,&regions) == NE_OK)
    {
        length = CSL_VectorGetLength(regions);
    }

    if (length <= 0)
    {
        return NE_OK;  //no speedlimit region
    }

    //get current maneuver
    if (closetManeuver == NAV_MANEUVER_NONE || closetManeuver == NAV_MANEUVER_START)
    {
        return NE_OK;  //if offroute, not show speedlimit sign
    }
    else
    {
        maneuver = NB_RouteInformationGetManeuver(route, closetManeuver);
    }

    //cacluate offset from the maneuver start point
    offsetInManeuver = maneuver->distance - currentPosition->maneuverDistanceRemaining;

    //check school zone
    if (speedLimitState->currentSpecialZoneIndex != NAV_MANEUVER_NONE &&
        speedLimitState->currentSpecialZoneIndex < length &&
        speedLimitState->configure.schoolAlert &&
        !speedLimitState->specialZoneAlertPlayed)
    {
        NB_EnhancedContentManager* manager = NB_ContextGetEnhancedContentManager(context);

        /* @todo: This logic follows original code. Should play guidance message of special zone alert if
                  there is no images of speed limit? */
        if (manager)
        {
        tmpRegion = (NB_SpeedRegion*)CSL_VectorGetPointer(regions,speedLimitState->currentSpecialZoneIndex);
            if (NB_EnhancedContentManagerIsDataAvailable(manager, NB_ECDT_SpeedLimitsSign, tmpRegion->m_datasetid,tmpRegion->m_signId) &&
                NB_EnhancedContentManagerIsDataAvailable(manager, NB_ECDT_SpeedLimitsSign, tmpRegion->m_datasetid,tmpRegion->m_signidHighlighted))
        {
            speedLimitState->specialZoneAlertPending = TRUE;
        }
    }
    }

    //check speed limit zone
    if (speedLimitState->currentSpeedRegionIndex != NAV_MANEUVER_NONE && speedLimitState->currentSpeedRegionIndex < length &&
        speedLimitState->configure.speedAlert)
    {
        tmpRegion = (NB_SpeedRegion*)CSL_VectorGetPointer(regions,speedLimitState->currentSpeedRegionIndex);
        currentTime = PAL_ClockGetGPSTime();
        if (currentTime - speedLimitState->speedingAlertTime >= 60 &&
            !speedLimitState->speedLimitAlertPending && !speedLimitState->speedLimitPictureMissing)
        {
            if ((tmpRegion->m_speedLimit + speedLimitState->configure.speedWarningLevel) < speed)
            {
                speedLimitState->speedLimitAlertPending = TRUE;
                speedLimitState->speedLimitAlertPlayed = FALSE;
            }
        }

        if (speedLimitState->speedLimitAlertPending &&
            (tmpRegion->m_speedLimit + speedLimitState->configure.speedWarningLevel) > speed)
        {
            speedLimitState->speedLimitAlertPending = FALSE;
            speedLimitState->speedLimitAlertPlayed = FALSE;
            speedLimitState->speedingAlertTime = 0;
        }
    }

    return NE_OK;
}

/* See header file for description. */
NB_Error
NB_SpeedLimitProcessorDownloadPicture(NB_RouteInformation* route,
                                      NB_SpeedLimitState* speedLimitState,
                                      NB_SpeedLimitInformation* information)
{
    NB_Context* context = NULL;
    NB_EnhancedContentManager* manager = NULL;
    CSL_Vector* regions = NULL;

    if (route == NULL || speedLimitState == NULL || speedLimitState->context == NULL || information == NULL)
    {
        return NE_INVAL;
    }

    // Check if context is valid.
    context = speedLimitState->context;
    if (context == NULL)
    {
        return NE_INVAL;
    }

    // Download nothing if enhanced content manager is NULL.
    manager = NB_ContextGetEnhancedContentManager(context);
    if (manager == NULL)
    {
        return NE_OK;
    }

    if (NB_SpeedLimitInformationGetSpeedRegions(information,&regions) == NE_OK)
    {
        uint32 i=0;
        uint32 length = CSL_VectorGetLength(regions);
        for (i=0; i<length;i++)
        {
            NB_SpeedRegion* speedRegion = (NB_SpeedRegion*)CSL_VectorGetPointer(regions,i);

            //check if it have been in cache or downloaded, if not downloaded it
            if (!NB_EnhancedContentManagerIsDataAvailable(manager, NB_ECDT_SpeedLimitsSign, speedRegion->m_datasetid,
                                                          speedRegion->m_signId) ||
                NB_EnhancedContentManagerCheckFileVersion(manager, NB_ECDT_SpeedLimitsSign, speedRegion->m_datasetid,
                                                         speedRegion->m_signId, speedRegion->m_version) != NE_OK)
            {
                NB_EnhancedContentManagerRequestDataDownload(manager, NB_ECDT_SpeedLimitsSign, speedRegion->m_datasetid,
                                                             speedRegion->m_signId, speedRegion->m_version);
            }

            if (!NB_EnhancedContentManagerIsDataAvailable(manager, NB_ECDT_SpeedLimitsSign, speedRegion->m_datasetid,
                                                          speedRegion->m_signidHighlighted) ||
                NB_EnhancedContentManagerCheckFileVersion(manager, NB_ECDT_SpeedLimitsSign, speedRegion->m_datasetid,
                                                         speedRegion->m_signidHighlighted, speedRegion->m_versionHighlighted) != NE_OK)
            {
                NB_EnhancedContentManagerRequestDataDownload(manager, NB_ECDT_SpeedLimitsSign, speedRegion->m_datasetid,
                                                             speedRegion->m_signidHighlighted, speedRegion->m_versionHighlighted);
            }
        }
    }
    return NE_OK;
}

/* See header file for description. */
NB_Error
NB_SpeedLimitStateReset(NB_SpeedLimitState* speedLimitState)
{
    if(speedLimitState == NULL)
    {
        return NE_INVAL;
    }
    speedLimitState->currentSpeedRegionIndex = NAV_MANEUVER_NONE;
    speedLimitState->isSpeedExceed = FALSE;
    speedLimitState->specialZoneAlertPlayed = FALSE;
    speedLimitState->speedingAlertTime = 0;
    speedLimitState->currentSpecialZoneIndex = NAV_MANEUVER_NONE;
    speedLimitState->speedLimitAlertPending = FALSE;
    speedLimitState->speedLimitAlertPlayed = FALSE;
    speedLimitState->specialZoneAlertPending = FALSE;
    speedLimitState->speedLimitPictureMissing = FALSE;
    return NE_OK;
}

/* See header file for description. */
NB_Error
NB_SpeedLimitStateCreate(NB_Context* context,
                         NB_SpeedLimitState** speedLimitState)
{
    NB_SpeedLimitState* pThis = NULL;

    if (speedLimitState == NULL || context == NULL)
    {
        return NE_INVAL;
    }

	if ((pThis = nsl_malloc(sizeof(*pThis))) == NULL)
    {
        return NE_NOMEM;
    }

	nsl_memset(pThis, 0, sizeof(*pThis));
    NB_SpeedLimitStateReset(pThis);
    pThis->context = context;

    //TODO
    pThis->configure.displaySpeedLimit = TRUE;
    pThis->configure.schoolAlert = TRUE;
    //configuration->schoolWarningType = 5;
    pThis->configure.speedAlert = TRUE;
    pThis->configure.speedWarningLevel = 5;
    pThis->configure.speedingDisplayAlert = TRUE;

    *speedLimitState = pThis;
    return NE_OK;
}

/* See header file for description. */
NB_Error
NB_SpeedLimitStateDestroy(NB_SpeedLimitState* speedLimitState)
{
    nsl_free(speedLimitState);
    return NE_OK;
}

/* See header file for description. */
NB_Error
NB_SpeedLimitStateSetConfigure(NB_SpeedLimitState* speedLimitState, NB_SpeedLimitConfiguration* configuration)
{
    if(configuration == NULL || speedLimitState == NULL)
    {
        return NE_INVAL;
    }

    speedLimitState->configure = *configuration;

    QaLogSpeedlimitSetting(speedLimitState->context, configuration);

    return NE_OK;
}

NB_DEF nb_boolean
NB_SpeedLimitIsAnnouncementPending(NB_SpeedLimitState* speedLimitState)
{
    if (!speedLimitState)
    {
        return FALSE;
    }

    return speedLimitState->speedLimitAlertPending || speedLimitState->specialZoneAlertPending;
}


NB_DEF NB_Error
NB_SpeedLimitGetPendingMessage(NB_SpeedLimitState* speedLimitState,
                               NB_NavigationState* navigationState,
                               NB_GuidanceInformation* guidanceInformation,
                               NB_SpeedLimitInformation* speedLimitInformation,
                               NB_NavigateAnnouncementUnits units,
                               NB_GuidanceMessage** message)
{
    NB_Error error = NE_OK;
    NB_RouteDataSourceOptions dataSourceOptions = {0};
    NB_PositionRoutePositionInformation routePositionInformation = {0};
    NB_NavigateAnnouncementType type = NB_NAT_None;

    if (message == NULL)
    {
        return NE_INVAL;
    }

    *message = NULL;

    if (!NB_SpeedLimitIsAnnouncementPending(speedLimitState))
    {
        return NE_NOENT;
    }

    error = NB_NavigationStateGetRoutePositionInformation(navigationState, &routePositionInformation);

    dataSourceOptions.route = navigationState->navRoute;
    dataSourceOptions.maneuverPos = NB_NMP_Current;
    dataSourceOptions.collapse = TRUE;
    dataSourceOptions.currentRoutePosition = &routePositionInformation;
    dataSourceOptions.speedLimitState = speedLimitState;

    if (speedLimitState->specialZoneAlertPending)
    {
        switch (speedLimitState->configure.schoolWarningType)
        {
            case SP_Tone:
                type = NB_NAT_Tone;
                break;
            case SP_Caution:
                type = NB_NAT_Voice;

            default:
                break;
        }
    }else if (speedLimitState->speedLimitAlertPending)
    {
        switch (speedLimitState->configure.speedWarningType)
        {
            case SP_Tone:
                type = NB_NAT_Tone;
                break;
            case SP_Caution:
                type = NB_NAT_Voice;

            default:
                break;
        }
    }

    error = NB_SpeedLimitInformationGetAnnouncement(speedLimitState,
                                                    guidanceInformation,
                                                    NB_RouteInformationDataSource,
                                                    &dataSourceOptions,
                                                    NAV_MANEUVER_NONE,
                                                    NB_NAS_Automatic,
                                                    type,
                                                    units,
                                                    message);

    if (error == NE_OK && message != NULL)
    {

        QaLogSpeedlimitState(speedLimitState, &routePositionInformation, speedLimitInformation, navigationState->navRoute);

        navigationState->messageCallbackData.messageState = speedLimitState;
        navigationState->messageCallbackData.navigationState = navigationState;
        if (speedLimitState->specialZoneAlertPending)
        {
            error = NB_GuidanceMessageSetPlayedCallback(*message, NB_SpeedLimitSchoolZoneMessagePlayedCallback,
                                                        &navigationState->messageCallbackData);
        }
        else if (speedLimitState->speedLimitAlertPending)
        {
            error = NB_GuidanceMessageSetPlayedCallback(*message, NB_SpeedLimitMessagePlayedCallback,
                                                        &navigationState->messageCallbackData);
        }
    }

    return error;
}


static void
NB_SpeedLimitMessagePlayedCallback(NB_GuidanceMessage* message, void* userData)
{
    MessagePlayedCallbackData* messagePlayedCallbackData = userData;
    NB_SpeedLimitState* speedLimitState = messagePlayedCallbackData ? messagePlayedCallbackData->messageState : NULL;

    if (!speedLimitState)
    {
        return;
    }

    speedLimitState->speedingAlertTime = PAL_ClockGetGPSTime();
    speedLimitState->speedLimitAlertPending = FALSE;
    speedLimitState->speedLimitAlertPlayed = TRUE;

    NB_NavigationMessagePlayed(messagePlayedCallbackData->navigationState, message);
}

static void
NB_SpeedLimitSchoolZoneMessagePlayedCallback(NB_GuidanceMessage* message, void* userData)
{
    MessagePlayedCallbackData* messagePlayedCallbackData = userData;
    NB_SpeedLimitState* speedLimitState = messagePlayedCallbackData ? messagePlayedCallbackData->messageState : NULL;

    if (!speedLimitState)
    {
        return;
    }

    speedLimitState->specialZoneAlertPending = FALSE;
    speedLimitState->specialZoneAlertPlayed = TRUE;

    NB_NavigationMessagePlayed(messagePlayedCallbackData->navigationState, message);

}

void
QaLogSpeedlimitState(const NB_SpeedLimitState* speedLimitState,
                     NB_PositionRoutePositionInformation* currentPostion,
                     NB_SpeedLimitInformation* speedLimitInformation,
                     NB_RouteInformation* route)
{
    CSL_Vector * regions = NULL;
    NB_SpeedRegion * region = NULL;
    data_nav_maneuver* maneuver = NULL;
    uint32 closetManeuver = 0;
    double offset = 0;
    NB_QaLogSpeedLimitStateData log_state={0};
    if (speedLimitState == NULL || currentPostion == NULL)
    {
        return;
    }
    closetManeuver = currentPostion->closestManeuver;
    if (closetManeuver == NAV_MANEUVER_START)
    {
        return;
    }

    //get current maneuver
    maneuver = NB_RouteInformationGetManeuver(route, closetManeuver);

    //cacluate offset from the maneuver start point
    offset = maneuver->distance - currentPostion->maneuverDistanceRemaining;

    NB_SpeedLimitInformationGetSpeedRegions(speedLimitInformation, &regions);

    if (speedLimitState->specialZoneAlertPending && speedLimitState->currentSpecialZoneIndex != NAV_MANEUVER_NONE)
    {
        region = (NB_SpeedRegion*)CSL_VectorGetPointer(regions,speedLimitState->currentSpecialZoneIndex);
        log_state.m_warnType = (uint8)QALOG_WARN_TYPE_WarnAhead;
        log_state.m_maneuverId = speedLimitState->currentSpecialZoneIndex;
    }
    else if (speedLimitState->speedLimitAlertPending && speedLimitState->currentSpeedRegionIndex != NAV_MANEUVER_NONE)
    {
        region = (NB_SpeedRegion*)CSL_VectorGetPointer(regions,speedLimitState->currentSpeedRegionIndex);
        log_state.m_warnType = (uint8)QALOG_WARN_TYPE_Speeding;
        log_state.m_maneuverId = speedLimitState->currentSpeedRegionIndex;
    }
    else
    {
        return;
    }

    nsl_strncpy(log_state.m_signId,region->m_signId,15);
    log_state.m_latitude = currentPostion->projLat;
    log_state.m_longitude = currentPostion->projLon;
    log_state.m_offset = offset;
    log_state.m_speedLimit = region->m_speedLimit;
    log_state.m_zoneType = (uint8)region->m_speedZoneType;

    NB_QaLogSpeedLimitState(speedLimitState->context, &log_state);
}

static void
QaLogSpeedlimitSetting(NB_Context* context,
                       NB_SpeedLimitConfiguration * configuration)
{
    NB_QaLogSpeedLimitInformation info = {0};
    if (context == NULL || configuration == NULL) {
        return;
    }
    info.m_speedLimitEnable = TRUE;
    info.m_displayEnable = configuration->displaySpeedLimit;
    info.m_speedAlertenable = configuration->speedAlert;
    switch (configuration->speedWarningType)
    {
        case SP_Tone:
            info.m_speedWarningSoundType = 0;
            break;
        case SP_Caution:
            info.m_speedWarningSoundType = 1;
            break;
        default:
            break;
    }
    info.m_speedWarningRange = configuration->speedWarningLevel;

    info.m_zoneAlertEnable = configuration->schoolAlert;
    switch (configuration->schoolWarningType)
    {
        case SP_Tone:
            info.m_zoneAlertEnable= 0;
            break;
        case SP_Caution:
            info.m_zoneAlertEnable = 1;
            break;
        default:
            break;
    }

    info.m_debugEnable = FALSE;
    info.m_debugOffset = 0;

    NB_QaLogSpeedLimitSetting(context, &info);
}

void
QaLogSpeedLimitEnterRegion(NB_Context* context,
                    NB_SpeedRegion* region,
                    const double latitude,
                    const double longitude
                    )
{
    if (region->m_qalogEnter) {
        return;
    }
    region->m_qalogEnter = TRUE;

    QaLogSpeedLimitEnterZone(context, region->m_speedLimit, latitude, longitude);
}

void
QaLogSpeedLimitOutRegion(NB_Context* context,
                         NB_SpeedRegion* region,
                         const double latitude,
                         const double longitude
                         )
{
    if (! region->m_qalogEnter) {
        return;
    }
    region->m_qalogEnter = FALSE;

    QaLogSpeedLimitExitZone(context, region->m_speedLimit, latitude, longitude);

}

static void
QaLogSpeedLimitEnterZone(NB_Context* context,
                         const double speedLimit,
                         const double latitude,
                         const double longitude
                         )
{
    char strSpeed[20] = "";
    nsl_sprintf(strSpeed,"%19f",speedLimit);
    NB_QaLogEnhancedContentState(context,
                                 strSpeed,
                                 NB_QLECT_SpeedLimit,
                                 NB_QLECRS_Enter,
                                 latitude,
                                 longitude
                                 );
}
static void
QaLogSpeedLimitExitZone(NB_Context* context,
                        const double speedLimit,
                        const double latitude,
                        const double longitude
                        )
{
    char strSpeed[20] = "";
    nsl_sprintf(strSpeed,"%19f",speedLimit);
    NB_QaLogEnhancedContentState(context,
                                 strSpeed,
                                 NB_QLECT_SpeedLimit,
                                 NB_QLECRS_Exit,
                                 latitude,
                                 longitude
                                 );
}





/*! @} */
