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

    @file     nbnavigation.h

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

#ifndef NBNAVIGATION_H_
#define NBNAVIGATION_H_

#include "navpublictypes.h"
#include "nbcontext.h"
#include "nbrouteinformation.h"
#include "nbnavigationstate.h"
#include "nbgpshistory.h"
#include "nbcamerainformation.h"
#include "nbtrafficinformation.h"
#include "nbcameraprocessor.h"
#include "nbpointsofinterestinformation.h"
#include "nbtrafficprocessor.h"
#include "nbguidanceprocessor.h"
#include "nbvectormapprefetchprocessor.h"
#include "nbenhancedcontentprocessor.h"
#include "nbspeedlimitprocessor.h"
#include "nbenhancedvectormapprocessor.h"
#include "nbrouteparameters.h"
#include "nbhandler.h"
#include "nbnaturallaneguidanceinterface.h"
#include "nbspecialregionprocessor.h"

/*!
    @addtogroup nbnavigation
    @{
*/

/*! @struct NB_Navigation
Information about the results of a navigation session
*/
typedef struct NB_Navigation NB_Navigation;

/*! Navigation session configuration settings
*/
typedef struct
{
    double          gpsFixMinimumSpeed;                     /*!< Minimum speed, in meters per second, for a GPS fix to be considered valid */
    double          startupRegionSizeFactor;                /*!< A factor applied to the distance to the start of the route to define the startup region */
    double          filteredGpsMinimumSpeed;                /*!< Speed, in meters per second, at which GPS filter is reset */
    double          filteredGpsConstant;                    /*!< Constant used for filtering GPS fixes */
    uint32          trafficNotifyMaximumRetries;            /*!< Maximum number of traffic-notify attempts to make */
    uint32          trafficNotifyDelay;                     /*!< Delay, in milliseconds, between traffic-notify attempts */
    uint32          completeRouteMaximumAttempts;           /*!< Maximum number of tries to recreate complete route */
    uint32          completeRouteRequestDelay;              /*!< Delay, in milliseconds, between route recreate requests */
    uint32          pronunRequestDelay;                     /*!< Delay, in milliseconds, before requesting pronun files for route */
    uint32          cameraSearchDelay;                      /*!< Delay, in milliseconds, before requesting safety camera information */
    uint32          cameraSearchDistance;                   /*!< Search distance along route, in meters, for safety cameras */
    uint32          startupRegionMinimumSize;               /*!< Minimum size of the startup region, in meters */
    uint32          excessiveRecalcMaximumCount;            /*!< Maximum number of recalcs to allow within a certain time period */
    uint32          excessiveRecalcTimePeriod;              /*!< Time period, in seconds, to enforce excessive recalcs */
    nb_boolean      filteredGpsEnabled;                     /*!< Whether or not to enable filtered fixes */
    nb_boolean      cameraSearchEnabled;                    /*!< Whether or not to enable safety camera searches */
    int             offRouteIgnoreCount;                    /*!< The number of rooute events that must happen before the system recalculates the route.
                                                                 Pass 0 is to use default value which is set internally */
    nb_boolean      nameChangedManeuverEnabled;             /*!< Whether or not to enable name changed maneuvers */
    nb_boolean      m1ManeuverEnabled;                      /*!< Whether or not to enable M1 maneuvers */
    nb_boolean      continueOnManeuverEnabled;              /*!< Whether or not to enable continue on (CO.) maneuvers */
} NB_NavigationConfiguration;


/*! Navigation status change callback function signature

This callback function will be called whenever the status of the navigation session changes.

@param navigation The navigation instance that is invoking the callback
@param status The status of the navigation status
@param userData The user data pointer registered with the callback
*/
typedef void (*NB_NavigationStatusChangeCallbackFunction) (NB_Navigation* navigation, NB_NavigateStatus status, void* userData);

/*! Guidance message callback function signature

This callback function will be called a guidance message should be played by the application.

@param navigation The navigation instance that is invoking the callback
@param message The guidance message to be played
@param userData The user data pointer registered with the callback
*/
typedef void (*NB_NavigationGuidanceMessageCallbackFunction) (NB_Navigation* navigation, NB_GuidanceMessage* message, void* userData);

/*! Enhanced Content state notification callback function signature

This callback function will be called to notify the application of an enhanced content state change.

@param navigation The navigation instance that is invoking the callback
@param state The enhanced content state
@param available Bit flags of enhanced content currently available
@param userData The user data pointer registered with the callback
*/
typedef void (*NB_NavigationEnhancedContentCallbackFunction) (NB_Navigation* navigation, NB_EnhancedContentState* state, NB_EnhancedContentStateData available, void* userData);

/*! Maneuver position Change callback function signature

This callback function will be called whenever the navigation maneuver position changes

@param navigation The navigation instance that is invoking the callback
@param maneuver The current maneuver
@param maneuvPosition The current maneuver position
@param userData The user data pointer registered with the callback
 */
typedef void (*NB_NavigationManeuverPositionChangeCallbackFunction) (NB_Navigation* navigation, uint32 maneuver, NB_NavigateManeuverPos maneuverPosition, void* userData);

/*! callback for speed limit.

This callback function will be called when the speed limit state data received

@param speedlimitData          The received speed limit state data
@param userData                The user data pointer registered with the callback
  */
typedef void(*NB_NavigationSpeedLimitCallbackFunction)(NB_SpeedLimitStateData* speedlimitData, void* userData);

/*! Visual Lane Guidance callback function signature

 This callback function will be called whenever the status of Visual Lane Guidance changes.
 */

typedef
void ( *NB_VisualLaneGuidanceCallback) (NB_Navigation* navigation,     /*!< The navigation instance
                                                                        that is invoking the callback */
                                        unsigned long manueverIndex,   /*!< Maneuver index of
                                                                        the visual lane guidance */
                                        unsigned long laneCount,       /*!< lane count */
                                        NB_Lane * laneItems,           /*!< lane items array */
                                        void* userData                 /*!< The user data pointer
                                                                        registered with the callback*/
                                        );

/*! Navigation mode change callback function signature

This callback function will be called whenever the mode of the navigation session changes.

@param navigation The navigation instance that is invoking the callback
@param mode The mode of the navigation session
@param userData The user data pointer registered with the callback
*/
typedef void (*NB_NavigationModeChangeCallbackFunction) (NB_Navigation* navigation, NB_NavigationMode mode, void* userData);

/*! Route requested callback function signature

This callback function will be called whenever the navigation start route requesting

@param navigation The navigation instance that is invoking the callback
@param reason reason why route is requested
@param userData The user data pointer registered with the callback
 */
typedef void (*NB_NavigationRouteRequestedCallbackFunction) (NB_Navigation* navigation, NB_NavigateRouteRequestReason reason, void* userData);


/*! Callback for special region.

This callback function will be called when the special region state data received.

@param specialRegionData       The received special region state data
@param userData                The user data pointer registered with the callback
 */
typedef void (*NB_NavigationSpecialRegionCallbackFunction)(NB_SpecialRegionStateData* specialRegionData, void* userData);

/*! Callback for via point.
 
 This callback function will be called when pass a via point.
 
 @param location       The viapoint location
 @param userData       The user data pointer registered with the callback
 */
typedef void (*NB_NavigationViaPointCallbackFunction)(NB_Location* location, void* userData);

typedef struct NB_NavigationCallbacks
{

    NB_NavigationStatusChangeCallbackFunction       statusChangeCallback;
    NB_RequestHandlerCallbackFunction               routeDownloadCallback;
    NB_RequestHandlerCallbackFunction               routeCompleteDownloadCallback;
    NB_NavigationGuidanceMessageCallbackFunction    guidanceMessageCallback;
    NB_RequestHandlerCallbackFunction               trafficNotificationCallback;
    NB_RequestHandlerCallbackFunction               pointsOfInterestNotificationCallback;   /*!< called when prefetched POIs are available via NB_NavigationGetInformation() */
    NB_RequestHandlerCallbackFunction               cameraNotificationCallback;
    NB_NavigationManeuverPositionChangeCallbackFunction maneuverPositionChangeCallback;
    NB_NavigationEnhancedContentCallbackFunction    enhancedContentNotificationCallback;
    NB_NavigationSpeedLimitCallbackFunction         speedLimitCallback;
    NB_VisualLaneGuidanceCallback                   visualLaneGuidanceCallback;
    NB_GetAnnounceSnippetLength                     getAnnounceSnippetLengthCallback;
    NB_NavigationModeChangeCallbackFunction         modeChangeCallback;
    NB_NavigationRouteRequestedCallbackFunction     routeRequestedCallback;
    NB_NavigationSpecialRegionCallbackFunction      specialRegionCallback;
    NB_NavigationViaPointCallbackFunction           reachViaPointCallback;
    void*                                           userData;

} NB_NavigationCallbacks;

#define NB_POI_PREFETCH_MAX_CATEGORIES 16
#define NB_POI_PREFETCH_SCHEME_LENGTH  32

/*! @struct NB_PointsOfInterestPrefetchConfiguration
Configuration information for POI searches to be queried during navigation
*/
typedef struct NB_PointsOfInterestPrefetchConfiguration
{
    char    searchScheme[NB_POI_PREFETCH_SCHEME_LENGTH];
    int     categoryCount;
    char    categories[NB_POI_PREFETCH_MAX_CATEGORIES][NB_CATEGORY_CODE_LEN+1];
    int     densityFactor;
    int     searchWidthMeters;
    int     searchLengthMeters;
    int     searchPreSearchMeters;

} NB_PointsOfInterestPrefetchConfiguration;

/*! Create a navigation session

@param context NB_Context
@param configuration Navigation session configuration values
@param callbacks Navigation session callbacks
@param vectorPrefetchConfig A NB_VectorMapPrefetchConfiguration object
@param navigation On success, a NB_Navigation object; NULL otherwise.  A valid object must be destroyed with NB_NavigationDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationCreate(NB_Context* context, NB_NavigationConfiguration* configuration, NB_NavigationCallbacks* callbacks, NB_VectorMapPrefetchConfiguration* vectorPrefetchConfig, NB_Navigation** navigation);

/*! Sets the Guidance Information object to be used

Sets the Guidance Information object to be used.  The application must call this method with a NULL information
if the Guidance Information previously passed is being deallocated.  The Navigation object does NOT take ownership
of this memory and will NOT deallocate it.

@param navigation A Navigation object created with NB_NavigationCreate()
@param information The guidance information object to be used.  NULL if none is to be used.
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationSetGuidanceInformation(NB_Navigation* navigation, NB_GuidanceInformation* information);

/*! Sets the Points of Interest search configuration to be used

Sets the Points of Interest search configuration to be used.  The application must call this method with a NULL information
if the Points of Interest search configuration previously passed is being deallocated.  The Navigation object does NOT take ownership
of this memory and will NOT deallocate it.

@param navigation A Navigation object created with NB_NavigationCreate()
@param information The guidance information object to be used.  NULL if none is to be used.
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationSetPointsOfInterestConfiguration(NB_Navigation* navigation, NB_PointsOfInterestPrefetchConfiguration* poiConfiguration);

/*! Start a navigation session using Route Information

Sets navigation route and associated information.  If this functions succeeds, then the navigation session is
responsible for these objects.  The caller is not responsible for freeing them.

@param context NB_Context
@param route A NB_RouteInformation object created with NB_RouteHandlerGetRouteInformation()
@param traffic A NB_TrafficInformation object created with NB_RouteHandlerGetTrafficInformation()
@param camera A NB_CameraInformation object created with NB_RouteHandlerGetCameraInformation()
@param pointsOfInformation A NB_PointsOfInterestInformation object created with NB_RouteHandlerGetPointsOfInterestInformation()
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationStartWithRouteInformation(NB_Navigation* navigation, NB_RouteInformation* route, NB_TrafficInformation* traffic, NB_CameraInformation* camera, NB_PointsOfInterestInformation* pointsOfInterest);

/*! Start a navigation session using Route Parameters

@param context NB_Context
@param parameters A NB_RouteParameters object
@param restart Is navigation session restarted (e.g. after route options change) or it's new one
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationStartWithRouteParameters(NB_Navigation* navigation, NB_RouteParameters* parameters, nb_boolean restart);

/*! Destroy a previously created Route object

@param navigation A Navigation object created with NB_NavigationCreate()
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationDestroy(NB_Navigation* navigation);

/*! Update GPS Location

@param navigation A Navigation object created with NB_NavigationCreate()
@param location current NB_GpsLocation
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationUpdateGPSLocation(NB_Navigation* navigation, NB_GpsLocation* location);

/*! Get a extrapolate path accoding to current location and speed etc.
    Described as a array of NB_ExtrapolatePosition

    @param navigation A Navigation object created with NB_NavigationCreate()
    @param duration How long the path should be extrapolated. count in milliseconds.
    @param timeInterval The time interval between two points on the path.
    @param path Pointer of buffer to store point array. It must be big enouth to save all points calculated.
    @param length Pointer buffer length
    @param count Count of the result
    @param currentPosition Inform the current position displayed in UI. If not NULL, the path before this point will be cut.

    @returns NB_Error
*/
NB_DEC NB_Error NB_NavigationGetExtrapolatePath(NB_Navigation* navigation, uint32 duration, uint32 timeInterval, NB_ExtrapolatePosition* path, uint32 length, uint32* count, double minSpeedExtrapolationThreshold);

/*! Merge two extrapolate path into one

    Positions overlap will be removed.
    This function can be called in other thread.

    @param newPath latest path from NB_NavigationGetExtrapolatePath
    @param newPathLength latest extrapolated path length
    @param oldPath original path
    @param oldPathLength original path length
    @param output merged path buffer
    @param outPutLength out put buffer length
    @param count return count of merged path

    @returns None
*/
NB_DEC void NB_NavigationMergeExtrapolatePath(NB_ExtrapolatePosition* newPath, uint32 newPathLength, NB_ExtrapolatePosition* oldPath, uint32 oldPathLength, NB_ExtrapolatePosition* output, uint32 outPutLength, uint32* count);

/*! Set the last extrapolate position which is used to calculate the extrapolate path. Please set it before invode the NB_NavigationGetExtrapolatePath.

    @param navigation A Navigation object created with NB_NavigationCreate()
    @param lastPosition The last extrapolate position.

    @returns NB_Error
*/
NB_DEC NB_Error NB_NavigationSetLastExtrapolatePosition(NB_Navigation* navigation, NB_ExtrapolatePosition* lastPosition);

/*! Heartbeat

@param navigation A Navigation object created with NB_NavigationCreate()
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationHeartbeat(NB_Navigation* navigation);

/*! Force a Recalculate

@param navigation A Navigation object created with NB_NavigationCreate()
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationRecalculate(NB_Navigation* navigation);

/*! Force a Recalculate with specified options

@param navigation A Navigation object created with NB_NavigationCreate()
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationRecalculateWithOptions(NB_Navigation* navigation, NB_RouteOptions* routeOptions, nb_boolean wantAlternateRoute);

/*! Gets information about the route associated with this session

@param navigation A Navigation object created with NB_NavigationCreate()
@param route The Route Information object associated with this session. NB_Navigation owns this object
@param traffic The Traffic Information object associated with this session. NB_Navigation owns this object
@param camera The Camera Information object associated with this session. NB_Navigation owns this object
@param pointsOfInterest The Points of Interest Information object associated with this session. NB_Navigation owns this object
*/
NB_DEC NB_Error NB_NavigationGetInformation(NB_Navigation* navigation, NB_RouteInformation** route, NB_TrafficInformation** traffic, NB_CameraInformation** camera, NB_PointsOfInterestInformation** pointsOfInterest);

/*! Gets Camera State

@param navigation A Navigation object created with NB_NavigationCreate()
@param cameraState current NB_CameraState. NB_Navigation owns this object
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationGetCameraState(NB_Navigation* navigation, NB_CameraState** cameraState);

/*! Gets Traffic State

@param navigation A Navigation object created with NB_NavigationCreate()
@param trafficState current NB_TrafficState. NB_Navigation owns this object
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationGetTrafficState(NB_Navigation* navigation, NB_TrafficState** trafficState);

/*! Gets Guidance State

@param navigation A Navigation object created with NB_NavigationCreate()
@param guidanceState current NB_GuidanceState. NB_Navigation owns this object
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationGetGuidanceState(NB_Navigation* navigation, NB_GuidanceState** guidanceState);


/*! Create enhanced vector map state

    @param context NB_Context
    @param navigation A Navigation object created with NB_NavigationCreate()
    @param enhancedVectorMapConfig Enhanced Vector Map related parameters. NULL if not use enhanced vector map.
    @param enhancedVectorMapState On success, returns pointer to enhanced vector map state object. NB_Navigation owns this object
    @returns NB_Error
*/
NB_DEC NB_Error NB_NavigationCreateEnhancedVectorMapState(NB_Context* context, NB_Navigation* navigation, NB_EnhancedVectorMapConfiguration* enhancedVectorMapConfig, NB_EnhancedVectorMapState** state);

/*! Sets navigation route and associated information

Sets navigation route and associated information.  If this functions succeeds, then the navigation session is
responsible for these objects.  The caller is not responsible for freeing them.

@param navigation A Navigation object created with NB_NavigationCreate()
@param route The Route Information object for the new route
@param traffic The Traffic Information object for the new route
@param camera The Camera Information object for the new route
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationNewRoute(NB_Navigation* navigation, NB_RouteInformation* route, NB_TrafficInformation* traffic, NB_CameraInformation* camera);

/*! Sets Camera Information object

@param navigation A Navigation object created with NB_NavigationCreate()
@returns cameraInformation camera information object
*/
NB_DEC NB_Error NB_NavigationUpdateCameraInformation(NB_Navigation* navigation, NB_CameraInformation* cameraInformation);

/*! Sets Traffic Information object

@param navigation A Navigation object created with NB_NavigationCreate()
@returns trafficInformation traffic information object
*/
NB_DEC NB_Error NB_NavigationUpdateTrafficInformation(NB_Navigation* navigation, NB_TrafficInformation* trafficInformation);

/*! Gets current route position information

@param navigation A Navigation object created with NB_NavigationCreate()
@returns information NB_PositionRoutePositionInformation object
*/
NB_DEC NB_Error NB_NavigationGetRoutePositionInformation(NB_Navigation* navigation, NB_PositionRoutePositionInformation* information);

/*! Gets current maneuver progress.

@param navigation A Navigation object created with NB_NavigationCreate()
@returns maneuverProgress current maneuver progress
*/
NB_DEC NB_Error NB_NavigationGetManeuverProgress(NB_Navigation* navigation, int* maneuverProgress);

/*! Gets current segment progress.

@param navigation A Navigation object created with NB_NavigationCreate()
@returns segmentProgress current segment progress
*/
NB_DEC NB_Error NB_NavigationGetSegmentProgress(NB_Navigation* navigation, int* segmentProgress);

/*! Gets current ManeuverPosition.

@param navigation A Navigation object created with NB_NavigationCreate()
@returns position Current maneuver position
*/
NB_DEC NB_Error NB_NavigationGetCurrentManeuverPosition(NB_Navigation* navigation, NB_NavigateManeuverPos* position);

/*! Checks to see if navigation state is on route.

@param navigation A Navigation object created with NB_NavigationCreate()
@returns nb_boolean returns TRUE if navigation state is on route, else returns FALSE
*/
NB_DEC nb_boolean NB_NavigationIsOnRoute(NB_Navigation* navigation);

/*! Gets current NBI_NavigateStatus status

@param navigation A Navigation object created with NB_NavigationCreate()
@param status current NBI_NavigateStatus status
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationGetStatus(NB_Navigation* navigation, NB_NavigateStatus* status);

/*! Get the current guidance announcement

*/
NB_DEC NB_Error NB_NavigationGetManualGuidanceMessage(NB_Navigation* navigation, NB_NavigateAnnouncementType type, NB_NavigateAnnouncementUnits units, nb_boolean buttonPressed, NB_GuidanceMessage** message);

/*! Get the current guidance announcement

 */
NB_DEC NB_Error NB_NavigationGetManualGuidanceMessageEx(NB_Navigation* navigation, uint32 index, nb_boolean lookahead, NB_NavigateAnnouncementType type, NB_NavigateAnnouncementUnits units, nb_boolean buttonPressed, NB_GuidanceMessage** message);

/*! Gets current navigation public state

@param navigation A Navigation object created with NB_NavigationCreate()
@param publicState pointer to a place to copy the current NB_NavigationPublicState state
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationGetPublicState(NB_Navigation* navigation, NB_NavigationPublicState* publicState);

/*! Gets current navigation session ID

@param navigation A Navigation object created with NB_NavigationCreate()
@param navSessionId current navigation session ID. A unique navigation
       session ID is assigned for every Navigation object created with
       NB_NavigationCreate(), the session ID is make available through this
       function. When client creates route tracking analytics events, this
       session ID is needed to assoicate those route state tracking events with
       the gps probes events created by the current Navigation object.
@returns NB_Error
@see NB_AnalyticsAddRouteStateEvent
*/
NB_DEC NB_Error NB_NavigationGetSessionId(NB_Navigation* navigation,
        uint32* navSessionId);

/*! Sets navigation announcement type

@param navigation A Navigation object created with NB_NavigationCreate()
@param type Navigation announcement type
@param units Navigation units
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationSetAnnounceType(NB_Navigation* navigation, NB_NavigateAnnouncementType type, NB_NavigateAnnouncementUnits units);

/*! Sets the navigation callbacks

@param navigation A Navigation object created with NB_NavigationCreate()
@param callbacks pointer to callbacks structure
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationSetCallbacks(NB_Navigation* navigation, NB_NavigationCallbacks* callbacks);

/*! Performs traffic request

@param navigation A Navigation object created with NB_NavigationCreate()
@param trafficRecordIdentifier traffic record identifier to get information
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationDoTrafficRequest(NB_Navigation* navigation, const char* trafficRecordIdentifier);

/*! Get errors associated with the navigation session

@param navigation A Navigation object created with NB_NavigationCreate()
@param routeError Route error value
@param error Navigation error value
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationGetErrors(NB_Navigation* navigation,  NB_NavigateRouteError* routeError, NB_Error* error);

/*! Gets trip remaining information

@param navigation A Navigation object created with NB_NavigationCreate()
@param time Gets the trip remaining time in seconds
@param delay Gets the trip remaining delay in seconds
@param meter Gets the trip remaining meter in percentage, meter = 100 - delay percentage. If meter value is not available, a negative vlue will be returned.
@return NB_Error
 */
NB_DEC NB_Error NB_NavigationGetTripRemainingInformation(NB_Navigation* navigation, uint32* time, uint32* delay, double* meter);


/*! Get next traffic event

Gets the next traffic event (incident or congestion) ahead on the navigation session for display.

@param navigation A Navigation object created with NB_NavigationCreate()
@param event Pointer to NB_TrafficEvent object to copy next event info to
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationGetNextTrafficEvent(NB_Navigation* navigation, NB_TrafficEvent* trafficEvent);

/*! Get maneuver traffic information

Get the first traffic event (incident or congestion), delay, and meter for the specified maneuver.

@param navigation A Navigation object created with NB_NavigationCreate()
@param startManeuverIndex The maneuver index to start the summary at (inclusive)
@param distanceOffset The distance into the given maneuver range
@param endManeuverIndex The maneuver index to end the summary at (not inclusive)
@param firstTrafficEvent Pointer to NB_TrafficEvent object to copy first maneuver event to
@param meter Gets meter value for specified maneuver
@param time Gets time for specified maneuver
@param delay Gets delay for specified maneuver
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationGetManeuverTrafficSummary(NB_Navigation* navigation, uint32 startManeuverIndex, double distanceOffset, uint32 endManeuverIndex, NB_TrafficEvent* firstTrafficEvent, double* meter, uint32* time, uint32* delay);

/*! Get array of upcoming traffic events

Allocates and returns an array of upcoming traffic events along the route.  The array must be freed when no longer needed by calling the NB_NavigationFreeTrafficEvents() function.

@param navigation A Navigation object created with NB_NavigationCreate()
@param numberOfTrafficEvents Pointer to where the traffic event count is to be stored
@param trafficEvents Pointer to where the NB_TrafficEvent array pointer is to be stored
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationGetUpcomingTrafficEvents(NB_Navigation* navigation, uint32* numberOfTrafficEvents, NB_TrafficEvent** trafficEvents);

/*! Free allocated array of upcoming traffic events

@param trafficEvents Pointer to the NB_TrafficEvent array to be freed
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationFreeTrafficEvents(NB_TrafficEvent* trafficEvents);

/*! Set enhanced content map configuration

@param navigation A Navigation object created with NB_NavigationCreate()
@param configuration Configuration for enhanced content map
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationSetEnhancedContentMapConfiguration(NB_Navigation* navigation, const NB_EnhancedContentMapConfiguration* configuration);

/*! Force navigation engine to reset ECM state
    Should be called after active route changing

@param navigation A Navigation object created with NB_NavigationCreate()
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationResetEnhancedContentMapState(NB_Navigation* navigation);


/*! Set current map orientation

@param navigation A Navigation object created with NB_NavigationCreate()
@param orientation Current map orientation
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationSetMapOrientation(NB_Navigation* navigation, NB_MapOrientation orientation);

/*! Set traffic polling interval

Sets the interval to manually poll for traffic updates.  A zero value (the default for a nav session) will stop
manual traffic polling if already in progress.  A non-zero value indicates the minutes that should elapse between
traffic update requests.  This enables platforms that can not receive traffic SMS notifications to have traffic
updates requested at regular intervals.

@param navigation A Navigation object created with NB_NavigationCreate()
@param pollingIntervalMinutes Interval traffic updates will be polled for, in minutes
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationSetTrafficPollingInterval(NB_Navigation* navigation, uint32 pollingIntervalMinutes);


/*! Set pronun style in voice cache

 Pronun style is used to validate current cache. If pronun style is different, voice cache is cleared.
 Call this function before adding items to voice cache.

 @param navigation A Navigation object created with NB_NavigationCreate()
 @param content Context object
 @param pronun_style current pronun style in navigation session
 @returns NB_Error
*/
NB_DEC NB_Error NB_NavigationSetVoiceCachePronunStyle (NB_Navigation* navigation, NB_Context* context, const char* pronun_style);

/*! Set speed limit configure

 This function must be call after create navigation handler and  before start navigation query.
 when navigation, if speedlimit configure change, this function can be call no need start a new navigation session.

 @param navigation navigation handler
 @param configuration Parameters configuration
 @returns NB_Error
 */
NB_DEC NB_Error NB_NavigationSetSpeedLimitConfiguration(NB_Navigation* navigation,NB_SpeedLimitConfiguration* configuration);


/*! Set fast start enabled/disabled

 @param navigation navigation handler
 @returns NB_Error
 */
NB_DEC NB_Error NB_NavigationFastStartupStatusChange(NB_Navigation* navigation, nb_boolean isEnter, const NB_GpsLocation* location, nb_boolean* needUpdateRoute);


/*! Sets the natural and lane guidance configuration to be used

 @returns NB_Error
 */
NB_DEC NB_Error
NB_NavigationSetGuidanceConfiguration(NB_Navigation* navigation,               /*!< The navigation instance
                                                                                that is invoking the callback */
                                      NB_RouteConfiguration* routeConfiguation, /*!< The routecConfiguartion
                                                                                 instance that is invoking
                                                                                 the callback */
                                      NB_GuidanceConfiguration* configuration  /*!< The configuration of
                                                                                natural and lane guidance using */
                                      );

/*! Start Traffic notify request (DEPRECATED)

 @param navigation A Navigation object created with NB_NavigationCreate()
 @returns NB_Error
 */
NB_DEC NB_Error NB_NavigationSetActiveRoute(NB_Navigation* navigation);

/*! Get a pronun information

@param navigation A Navigation object created with NB_NavigationCreate()
@param key name of base voice or street voice
@param textBuffer plain text buffer
@param textBufferSize size of plain text buffer (return 0 if empty)
@param phoneticsData Phonetic representation of the text (if available), empty otherwise. The text is encoded using UTF-8 format
@param phoneticsDataSize Size of Phonetics data (return 0 if empty)
@param duration time of playback duration in ms
@param isBaseVoices if the voices is base voice
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationGetPronunInformation(NB_Navigation* navigation, const char* key,
                                                  char* textBuffer, uint32* textBufferSize,
                                                  byte* phoneticsData, uint32* phoneticsDataSize,
                                                  char* language, uint32* languageDataSize,
                                                  char* translatedText, uint32* translatedTextDataSize,
                                                  double* duration, nb_boolean* isBaseVoices );

/*! Suspend session

@param navigation A Navigation object created with NB_NavigationCreate()
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationSuspendSession(NB_Navigation* navigation);

/*! resume session

@param navigation A Navigation object created with NB_NavigationCreate()
@returns NB_Error
*/
NB_DEC NB_Error NB_NavigationResumeSession(NB_Navigation* navigation);

/*! Get the total traveled distance during navigation *

 @param navigation A Navigation object created with NB_NavigationCreate()
 @param distance of traveled
 @return NB_Error
 */
NB_DEC NB_Error NB_NavigationGetTraveledDistance(NB_Navigation* navigation, double* traveledDistance);

/*! Sets traffic setting

@param navigation A Navigation object created with NB_NavigationCreate()
@param setting NB_TrafficSetting object contains traffic setting
@return NB_Error
*/
NB_DEC NB_Error NB_NavigationSetTrafficSetting(NB_Navigation* navigation,NB_TrafficSetting* setting);

/*! Play sound chime in time switching from TBT to Static Mode

  @returns NB_Error
 */
NB_DEC NB_Error NB_PlayTBTToStaticMessage(NB_Navigation* navigation);

/*! Force a Recalculate with specified options

 @returns NB_Error
 */
NB_DEC NB_Error NB_NavigationRecalculateWithOptionsAndConfig(NB_Navigation* navigation,          /*!< A Navigation object created with NB_NavigationCreate() */
                                                             NB_RouteOptions* routeOptions,      /*!< The route option which is used for recalculating */
                                                             NB_RouteConfiguration* routeConfig, /*!< The route config which is used for recalculating */
                                                             nb_boolean wantAlternateRoute);     /*!< Want multiple routes or not */
/*! Force a Recalculate with waypoint
 
 @returns NB_Error
 */
NB_DEC NB_Error NB_NavigationRecalculateWithWayPoint(NB_Navigation* navigation, NB_WayPoint* pWaypoint, uint32 wayPointSize);

/*! Get the summary traffic color from curren position to destination.

@return NB_Error
*/
NB_DEC NB_Error NB_NavigationGetTrafficColor(NB_Navigation* navigation, /*!< A Navigation object created with NB_NavigationCreate() */
                                             char* color);              /*!< Traffic color returned. */

/*! Play the manual message.

 @return NB_Error
 */
NB_DEC NB_Error NB_NavigationPlayManualGuidanceMessage(NB_Navigation* navigation,           /*!< A Navigation object created with NB_NavigationCreate() */
                                                       uint32 maneuverIndex,                /*!< The maneuver index */
                                                       NB_NavigateAnnouncementType type,    /*!< The announcement type */
                                                       NB_NavigateAnnouncementUnits units); /*!< The announcement units */

/*! Play the manual message for maneuver list.

 @return NB_Error
 */
NB_DEC NB_Error NB_NavigationPlayManualGuidanceMessageForManeuverList(NB_Navigation* navigation,           /*!< A Navigation object created with NB_NavigationCreate() */
                                                                      NB_RouteInformation* route,          /*!< The route information object which contains the maneuver. */
                                                                      uint32 maneuverIndex,                /*!< The maneuver index */
                                                                      NB_NavigateAnnouncementType type,    /*!< The announcement type */
                                                                      NB_NavigateAnnouncementUnits units,  /*!< The announcement units */
                                                                      double latitude,                     /*!< The latitude of last position */
                                                                      double longitude);                   /*!< The longtitude of last position */

/*! Get the traffic guidance information.
 
 @return NE_Error
 */
NB_DEC NB_Error NB_NavigationGetTrafficGuidanceInformation(NB_Navigation* navigation,   /*!< A Navigation object created with NB_NavigationCreate() */
                                                           NB_GuidanceMessage* message, /*!< The guidance message instance */
                                                           double* playtimes,           /*!< The output play time of current guidance. */
                                                           nb_boolean* isOverAll,       /*!< The guidance is over all .*/
                                                           nb_boolean* isCongestion,    /*!< The guidance is congestion. */
                                                           nb_boolean* isInCongestion,  /*!< The guidance is in congestion range. */
                                                           char* trafficColor);         /*!< The traffic color. */

/*! Get the next mock point which route polyline

 @return NE_Error
 */
NB_DEC NB_Error NB_NavigationGetNextMockPoint(NB_Navigation* navigation,          /*!< A Navigation object created with NB_NavigationCreate() */
                                              uint32* maneuver,                   /*!< The maneuver index, this value may be updated */
                                              uint32* segment,                    /*!< The segment index, this value may be updated */
                                              double speed,                       /*!< The speed of mock location */
                                              const NB_GpsLocation* lastLocation, /*!< The last location */
                                              NB_GpsLocation* location);          /*!< The next location */

NB_DEF NB_Error NB_NavigationGetDescriptionInformation(NB_Navigation* navigation, const char* key,
                                                       char* textBuffer, uint32* textBufferSize,
                                                       byte* phoneticsData, uint32* phoneticsDataSize,
                                                       char* language, uint32* languageDataSize,
                                                       char* translatedText, uint32* translatedTextDataSize,
                                                       double* duration, nb_boolean* isBaseVoices);
/* @} */
#endif
