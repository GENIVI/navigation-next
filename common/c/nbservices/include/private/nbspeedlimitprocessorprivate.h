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
 @file     nbspeedlimitprocessorprivate.h
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

#ifndef NBSERVICES_NBSPEEDLIMITPROCESSORPRIVATE_H
#define NBSERVICES_NBSPEEDLIMITPROCESSORPRIVATE_H

#include "paltypes.h"
#include "navtypes.h"
#include "nbspeedlimitprocessor.h"
#include "nbenhancedcontentmanager.h"
#include "nbspeedlimittypes.h"
#include "nbnavigation.h"

typedef void(*NB_NavigationSpeedLimitCallbackFunctionProtected)(NB_SpeedLimitStateData* speedlimitData,
                                                                const char* sign_id,
                                                                void* userData);
/*! speed limit state,used by speedlimit processor to save state.
*/
typedef struct NB_SpeedLimitState
{
    NB_Context*                 context;
    uint32                      currentSpeedRegionIndex;
    uint32                      currentSpecialZoneIndex;
    nb_boolean                  isSpeedExceed;
    nb_boolean                  specialZoneAlertPending;
    nb_boolean                  specialZoneAlertPlayed;
    nb_boolean                  speedLimitAlertPending;
    nb_boolean                  speedLimitAlertPlayed;
    nb_boolean                  speedLimitPictureMissing;
    nb_gpsTime                  speedingAlertTime;
    NB_SpeedLimitConfiguration  configure;
} NB_SpeedLimitState;

/*! Create update speedlimit state and call callback if state change

    This function can be called after gps location update and after a new route reply.

@return NE_OK on success
*/
NB_DEF NB_Error
NB_SpeedLimitProcessorUpdate(NB_SpeedLimitState* speedLimitState,                      /*!< speed limit state. */
                             NB_RouteInformation* route,                               /*!< navgation state */
                             NB_SpeedLimitInformation* information,                    /*!< contain speed limit region data */
                             NB_PositionRoutePositionInformation* currentPosition,     /*!< current position at menuver. */
                             double speed,                                             /*!< current speed. */
                             NB_NavigationSpeedLimitCallbackFunctionProtected callback,/*!< speedlimit callback. */
                             void* userData                                            /*!< user private data for callback. */
                             );

/*! Speed limit alert update.

    Check current speed state and play alert if needed. the function canbe called after NB_SpeedLimitProcessorUpdate

@return NE_OK if success
*/
NB_DEF NB_Error
NB_SpeedLimitAlertUpdate(NB_SpeedLimitState* speedLimitState,                          /*!< speed limit state. */
                         NB_RouteInformation* route,                                   /*!< navgation state */
                         NB_SpeedLimitInformation* information,                        /*!< contain speed limit region data */
                         NB_PositionRoutePositionInformation* currentPosition,         /*!< current position at menuver. */
                         double speed                                                  /*!< current speed. */
                         );

/*! Call enhancedContentManager to download speedlimit picture.

@return NE_OK
*/
NB_DEF NB_Error
NB_SpeedLimitProcessorDownloadPicture(NB_RouteInformation* route,                      /*!< route information */
                                      NB_SpeedLimitState* speedLimitState,             /*!< speed limit state. */
                                      NB_SpeedLimitInformation* information            /*!< contain speed limit region data */
                                      );

/*! Reset speedlimitstate

@param speedLimitState used by SpeedLimitProcessor and SpeedlimitAlert
@return NB_Error
*/
NB_DEF NB_Error
NB_SpeedLimitStateReset(NB_SpeedLimitState* speedLimitState);

/*! Create speedlimit state.

@return NB_Error
*/
NB_DEF NB_Error
NB_SpeedLimitStateCreate(NB_Context* context,                                  /*!< context  NB_Context */
                         NB_SpeedLimitState** speedLimitState                  /*!< speedLimitState the speedLimit state be created.*/
                         );

/*! Destory the speedlimit state.

@return NB_Error
*/
NB_DEF NB_Error
NB_SpeedLimitStateDestroy(NB_SpeedLimitState* speedLimitState                   /*!< speedlimit state object. */
                          );

/*! Set speedlimit configure.

@return NB_Error
*/
NB_DEF NB_Error
NB_SpeedLimitStateSetConfigure(NB_SpeedLimitState* speedLimitState,            /*!< speedlimit state object. */
                               NB_SpeedLimitConfiguration* configuration       /*!< speedlimit configure. */
                               );

/*! Check Speed limit message pending*/
NB_DEF nb_boolean
NB_SpeedLimitIsAnnouncementPending(NB_SpeedLimitState* speedLimitState         /*!< speedlimit state object. */
                                   );

/*! Get Speed limit message*/
NB_DEF NB_Error
NB_SpeedLimitGetPendingMessage(NB_SpeedLimitState* speedLimitState,            /*!< Speedlimit state object. */
                               NB_NavigationState* navigationState,            /*!< Navigation state object. */
                               NB_GuidanceInformation* guidanceInformation,    /*!< Guidance information*/
                               NB_SpeedLimitInformation* speedLimitInformation,/*!< Speed limit information*/
                               NB_NavigateAnnouncementUnits units,             /*!< Units, mile/meter*/
                               NB_GuidanceMessage** message                    /*!< return message*/
                               );

/*! @} */

#endif
