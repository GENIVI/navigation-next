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

    @file     nbnavigationprivate.h
    @defgroup nbnavigation nbnavigation

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

#ifndef NBNAVIGATIONPRIVATE_H_
#define NBNAVIGATIONPRIVATE_H_

#include "nbcontext.h"
#include "nbnavigation.h"
#include "nbrouteinformation.h"
#include "nbgpshistory.h"
#include "navpublictypes.h"
#include "navtypes.h"
#include "nbvectormapprefetchprocessor.h"
#include "nbvectormapprefetchprocessorprivate.h"
#include "nbenhancedcontentprocessorprivate.h"
#include "nbcameraprocessor.h"
#include "nbtrafficprocessor.h"
#include "nbguidanceprocessor.h"
#include "nav_avg_speed.h"
#include "nbroutehandler.h"
#include "nbtraffichandler.h"
#include "nbsearchhandler.h"
#include "nbpronunhandler.h"
#include "nbgpsfilter.h"
#include "nbrecalclimit.h"
#include "nbsinglesearchhandler.h"
#include "nbspeedlimitprocessorprivate.h"
#include "nbspeedlimitinformationprivate.h"
#include "nbnaturallaneguidanceprocessor.h"
#include "nbspecialregionprocessor.h"
#include "nbspecialregionprocessorprivate.h"
#include "nbspecialregioninformationprivate.h"

/*!
    @addtogroup nbnavigation
    @{
*/

struct NB_Navigation
{
    NB_Context* context;

    NB_GuidanceInformation*     guidanceInformation;

    NB_NavigationCallbacks      callbacks;

    NB_NavigateStatus           status;

    NB_NavigationState*         navState;

    NB_NavigationPublicState*   navPublicState;

    NB_CameraInformation*       cameraInformation;
    NB_CameraState*             cameraState;

    NB_EnhancedContentState*    enhancedContentState;
    NB_EnhancedVectorMapState*  enhancedVectorMapState;

    NB_SpeedLimitState*         speedlimitState;
    NB_SpeedLimitInformation*   speedLimit;

    NB_PointsOfInterestPrefetchConfiguration poiConfiguration;
    NB_PointsOfInterestInformation* poiInformation;

    NB_TrafficInformation*      trafficInformation;
    NB_TrafficState*            trafficState;

    NB_GuidanceState*           guidanceState;
    NB_NLGInformation*          naturalLaneGuidanceInformation;
    NB_NLGState*                naturalLaneGuidanceState;

    NB_RouteHandler*            routeHandler;
    NB_RequestHandlerCallback   routeHandlerCallback;

    NB_TrafficHandler*          trafficHandler;
    NB_RequestHandlerCallback   trafficHandlerCallback;

    NB_SearchHandler*           cameraHandler;
    NB_RequestHandlerCallback   cameraHandlerCallback;

    NB_SingleSearchHandler*     poiHandler;
    NB_RequestHandlerCallback   poiHandlerCallback;
    double                      poiLastQueryRouteRemaining;
    nb_boolean                  poiLastQueryFailed;
    uint32                      poiQueryFailedTimes;

    NB_PronunHandler*           pronunHandler;
    NB_RequestHandlerCallback   pronunHandlerCallback;
    uint32                      pronunDownloadRetries;

    NB_NetworkNotifyEventCallback navigationNetworkCallback;

    NB_GpsFilter*               gpsFilter;
    NB_RecalcLimit*             recalcLimit;

    nb_boolean                    initialNotifyComplete;
    uint32                      navSessionId;

    uint32                      trafficPollingIntervalMinutes;

    nb_boolean                  startNotification;

    NB_NavigateAnnouncementType announceType;
    NB_NavigateAnnouncementUnits announceUnits;

    NB_StartupRegion            startupRegion;
    double                      arrivingRegionRadius;

    NB_NavigationConfiguration  configuration;
    nb_boolean                  poiSearchTimerExpired;
    nb_boolean                  fastNavStartupMode;
    NB_NavigationMode           navMode;
    nb_boolean                  theFirstFixProcessed;
    NB_GuidanceMessageCallbackFunction    qaLogGuidanceMessageCallback;
    nb_boolean                  suspended;

    double                      traveledDistance;

    NB_SpecialRegionInformation* specialRegion;
    NB_SpecialRegionState*       specialRegionState;
    NB_ExtrapolatePosition       lastPosition;
};

typedef struct
{
    NB_Navigation* navigation;
    uint32 currentManeuver;
    double actualStartTime;
    double actualStartDistance;
} NB_QaLogGuidanceMessageData;

/*! Finds next congestion region
@param navigation A Navigation object created with NB_NavigationCreate()
@return NB_Error
*/
NB_Error NB_NavigationUpdateNavigationState(NB_Navigation* navigation, NB_GpsLocation* location);

nb_boolean IsHeadingValidForNavigation(NB_NavigationState* navState, NB_GpsLocation* location);

/* @} */

#endif //NBNAVIGATIONPRIVATE_H_
