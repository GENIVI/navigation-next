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

    @file     nbrouteinformation.h

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

#ifndef NBROUTEINFORMATION_H_
#define NBROUTEINFORMATION_H_

#include "nbcontext.h"
#include "nbguidanceinformation.h"
#include "nbnaturallaneguidanceinterface.h"
#include "nbguidancemessage.h"
#include "nbvectortile.h"
#include "nbgpstypes.h"
#include "csltypes.h"

/*!
    @addtogroup nbrouteinformation
    @{
*/

/*! @struct NB_RouteInformation
Information about the results of a route request
*/
typedef struct NB_RouteInformation NB_RouteInformation;

typedef struct NB_RouteDataSourceOptions NB_RouteDataSourceOptions;

/*! @struct NB_TrafficInformation
Information about the results of a traffic request
*/
typedef struct NB_TrafficInformation NB_TrafficInformation;

/*! @struct NB_TrafficEvent
Generic traffic event object containing an incident or congestion
*/
typedef struct NB_TrafficEvent NB_TrafficEvent;

typedef enum NB_GuidancePointType
{
    NBGPT_Invalid = 0,
    NBGPT_StopSign,
    NBGPT_TrafficLight,
    NBGPT_EndOfTheRoad,
    NBGPT_Bridge,
    NBGPT_StreetCount,
    NBGPT_TrainTracks,
    NBGPT_NavteqNaturalGuidance
} NB_GuidancePointType;

typedef struct
{
    byte*    data;
    nb_size  size;
} NB_RouteId;

/*! Route information of route layer. */
typedef struct _RouteInfo
{
    NB_RouteId* routeId;                /*!< Identifier of route */
    nb_color    routeColor;             /*!< Color of route */
} RouteInfo;

typedef struct LaneFont NB_CommandFont;


typedef struct
{
    nb_boolean  isTollsOnRoute;
    nb_boolean  isHovLanesOnRoute;
    nb_boolean  isHighWayOnRoute;
    nb_boolean  isUnpavedOnRoute;
    nb_boolean  isFerryOnRoute;
    nb_boolean  isCongestionOnRoute;
    nb_boolean  isAccidentOnRoute;
    nb_boolean  isConstructionOnRoute;
    nb_boolean  isEnterCoutryOnRoute;
    nb_boolean  isPrivateRoadOnRoute;
    nb_boolean  isGatedAccessOnRoute;
} NB_OnRouteInformation;

#define NB_ROUTEINFORMATION_STREET_NAME_MAX_LEN 256

#define ROUTE_SELECTOR_ALTERNATE_ROUTES_MAX_COUNT  3
#define ROUTE_SELECTOR_DETOURS_MAX_COUNT           2


/*! Destroy a previously created Route object

@param route A NB_Route object created with NB_RouteHandlerGetRouteInformation()
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationDestroy(NB_RouteInformation* route);


/*! Get the detour name associated with the route

This call will only succeed if the route was created as a detour from another route

@param route NB_Route previously created route object
@param nameBuffer The address of a text buffer to receive the name of the detour
@param nameBufferSize The size of the text buffer
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetDetourName(NB_RouteInformation* route, char* nameBuffer, uint32 nameBufferSize);


/*! Get the route polyline

@param route NB_Route previously created route object
@param polyline On success, a NB_MercatorPolyline object; NULL otherwise.  A valid object must be destroyed with NB_MercatorPolylineDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetRoutePolyline(NB_RouteInformation* route, uint32 startManeuver, uint32 endManeuver, NB_MercatorPolyline** polyline);

/*! Get the route bounds

@param route            NB_Route previously created route object
@param startManeuver    Index of start maneuver for which to get the route bounds for
@param endManeuver      Index of end maneuver for which to get the route bounds for
@param min              On return the top/left corner of the route bounds
@param max              On return the bottom/right corner of the route bounds
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetRouteExtent(NB_RouteInformation* route, uint32 startManeuver, uint32 endManeuver, NB_LatitudeLongitude* min, NB_LatitudeLongitude* max);

/*! Get the all route bounds

@param route            NB_Route previously created route object
@param startManeuver    Index of start maneuver for which to get the route bounds for
@param endManeuver      Index of end maneuver for which to get the route bounds for
@param min              On return the top/left corner of the route bounds
@param max              On return the bottom/right corner of the route bounds
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetRouteExtentAll(NB_RouteInformation* route, uint32 startManeuver, uint32 endManeuver, NB_LatitudeLongitude* min, NB_LatitudeLongitude* max);

/*! Get the number of maneuvers on the route

@param route NB_RouteInformation previously created route object
@returns Number of maneuvers on route
*/
NB_DEC uint32 NB_RouteInformationGetManeuverCount(NB_RouteInformation* route);


/*! Get a turn announcement

@param route NB_RouteInformation previously created route object
@param index The index of the turn image to retrieve
@param message On success an NB_GuidanceMessage object containing the turn announcement, NULL otherwise
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetTurnAnnouncement(NB_RouteInformation* route, NB_GuidanceInformation* guidanceInformation, uint32 index, NB_NavigateManeuverPos maneuverPosition, NB_NavigateAnnouncementSource source, NB_NavigateAnnouncementType type, NB_NavigateAnnouncementUnits units, NB_GuidanceMessage** message);

/*! Get a turn announcement extensive function

@param route NB_RouteInformation previously created route object
@param index The index of the turn image to retrieve
@param message On success an NB_GuidanceMessage object containing the turn announcement, NULL otherwise
@param projLatitude project latitude of current location
@param projLongtitude project longtitude of current location
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetTurnAnnouncementExt(NB_RouteInformation* route,
                                                          NB_GuidanceInformation* guidanceInformation,
                                                          uint32 index,
                                                          NB_NavigateManeuverPos maneuverPosition,
                                                          NB_NavigateAnnouncementSource source,
                                                          NB_NavigateAnnouncementType type,
                                                          NB_NavigateAnnouncementUnits units,
                                                          NB_GuidanceMessage** message,
                                                          double projLatitude,
                                                          double projLongtitude);

/*! Get a turn image

@param route NB_RouteInformation previously created route object
@param index The index of the turn image to retrieve
@param imageCodeBuffer Buffer to receive the name of the turn image
@param imageCodeBufferSize Size of the buffer to receive the name of the turn image
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetTurnImage(NB_RouteInformation* route, NB_GuidanceInformation* guidanceInformation, uint32 index, NB_NavigateManeuverPos maneuverPosition, NB_NavigateAnnouncementUnits units, char* imageCodeBuffer, uint32 imageCodeBufferSize);

/*! Get turn characters

 @param route NB_RouteInformation previously created route object
 @param index The index of the turn arrow to retrieve
 @param turnArrows the Arrow characters of the turn
 @returns NB_Error
 */
NB_DEC NB_Error NB_RouteInformationGetTurnCommandCharacter(NB_RouteInformation* route,
                                                           NB_GuidanceInformation* guidanceInformation,
                                                           uint32 index,
                                                           NB_NavigateManeuverPos maneuverPosition,
                                                           NB_NavigateAnnouncementUnits units,
                                                           NB_CommandFont* commandFont);

/*! Get the next significant maneuver after the specified maneuver

@param route NB_RouteInformation previously created route object
@param afterIndex The index of the maneuver after which to find the next significant maneuver
@param significantIndex On success, the index of the requested maneuver
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetNextSignificantManeuverIndex(NB_RouteInformation* route, uint32 afterIndex, uint32* significantIndex);


/*! Enumerate the text directions associated with a maneuver

@param route NB_RouteInformation previously created route object
@param textType The type (primary or secondary display) of the maneuver text
@param index Zero-based index of the maneuver for which text information will be returned.  This must be less than the return value of NB_RouteGetManeuverCount() or one of the following special values: NB_RouteManeuverOrigin (start of route text) or NB_RouteManeuverSummary (route summary text)
@param callback Callback function to receive text information
@param userData User data pointer to pass to callback function
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationEnumerateManeuverText(NB_RouteInformation* route, NB_GuidanceInformation* guidanceInformation, NB_RouteTextType textType, NB_NavigateAnnouncementUnits units, uint32 index, NB_GuidanceInformationTextCallback callback, void* userData);


/*! Enumerate the stacked turn text directions associated with a maneuver

@param route NB_RouteInformation previously created route object
@param textType The type (primary or secondary display) of the stacked turn maneuver text
@param index Zero-based index of the maneuver for which stacked turn text information will be returned.  This must be less than the return value of NB_RouteGetManeuverCount() or one of the following special values: NB_RouteManeuverOrigin (start of route text) or NB_RouteManeuverSummary (route summary text)
@param callback Callback function to receive stacked turn text information
@param userData User data pointer to pass to callback function
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationEnumerateStackedTurnText(NB_RouteInformation* route, NB_GuidanceInformation* guidanceInformation, NB_RouteTextType textType, NB_NavigateAnnouncementUnits units, uint32 index, NB_GuidanceInformationTextCallback callback, void* userData);


/*! Enumerate the arrival directions associated with a maneuver

=====> Can this be replaced with a special NB_RouteManeuverArrival index to NB_RouteEnumerateManeuverText() ? <======

@param route NB_RouteInformation previously created route object
@param textType The type (primary or secondary display) of the arrival text
@param callback Callback function to receive text information
@param userData User data pointer to pass to callback function
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationEnumerateArrivalText(NB_RouteInformation* route, NB_GuidanceInformation* guidanceInformation, NB_RouteTextType textType, NB_NavigateAnnouncementUnits units,  NB_GuidanceInformationTextCallback callback, void* userData);


/*! Get the index of the next significant (non-informational) maneuver after the specified maneuver

@param route NB_RouteInformation previously created route object
@param index Index of the maneuver after which to search for a significant (non-informational) maneuver
@returns Index of the next significant maneuver, if any; NB_RouteNavManeuverNone if no significant maneuver found
*/
NB_DEC uint32 NB_RouteInformationGetNextSignificantManeuver(NB_RouteInformation* route, uint32 index);


/*! Get the index of the previous significant (non-informational) maneuver before the specified maneuver

@param route NB_RouteInformation previously created route object
@param index Index of the maneuver before which to search for a significant (non-informational) maneuver
@returns Index of the previous significant maneuver, if any; NB_RouteNavManeuverNone if no significant maneuver found
*/
NB_DEC uint32 NB_RouteInformationGetPreviousSignificantManeuver(NB_RouteInformation* route, uint32 index);


/*! Determines if a maneuver is significant or informational.

@param route NB_RouteInformation previously created route object
@param index Index of the maneuver to check
@returns Non-zero if the maneuver is significant; zero if the maneuver is informational
*/
NB_DEC nb_boolean NB_RouteInformationIsManeuverSignificant(NB_RouteInformation* route, uint32 index);

/*! Determines if a maneuver is significant, it use for turn list.

@param route NB_RouteInformation previously created route object
@param index Index of the maneuver to check
@returns Non-zero if the maneuver is significant; zero if the maneuver is not significant
*/
NB_DEC nb_boolean
NB_RouteInformationIsManeuverSignificantForTurnLists(NB_RouteInformation* route, uint32 index, nb_boolean isNextManeuver);

/*! Get the index of the next maneuver after the specified maneuver

@param route NB_RouteInformation previously created route object
@param index Index of the maneuver after which to search
@returns Index of the next maneuver, if any; NB_RouteNavManeuverNone if no maneuver found
*/
NB_DEC uint32 NB_RouteInformationGetNextManeuver(NB_RouteInformation* route, uint32 index);


/*! Get the index of the previous maneuver before the specified maneuver

@param route NB_RouteInformation previously created route object
@param index Index of the maneuver before which to search
@returns Index of the previous maneuver, if any; NB_RouteNavManeuverNone if no maneuver found
*/
NB_DEC uint32 NB_RouteInformationGetPreviousManeuver(NB_RouteInformation* route, uint32 index);


/*! Get summary information about the current route.

@param route NB_RouteInformation previously created route object
@param state Current navigation state
@param time On success, the estimated time to complete the route
@param delay On success, the estimated traffic delay along the route
@param distance On success, the distance left along the route
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetSummaryInformation(NB_RouteInformation* route, uint32* time, uint32* delay, double* distance);


/*! Get turn information on the current route

@param route NB_RouteInformation previously created route object
@param index Index of the maneuver to get turn information about
@param turnPoint the location of the maneuver
@param primaryNameBuffer On success, the primary road name or place name
@param primaryNameBufferSize The size of the primary name buffer
@param secondaryNameBuffer On success, the secondary road name or location name
@param secondaryNameBufferSize The size of the secondary name buffer
@param currentNameBuffer On success, the current (?) name
@param currentNameBufferSize The size of the current name buffer
@param distance On success, the (possibly collapsed) distance remaining in the maneuver
@param collapsed Whether or not to return the collapsed or base distance remaining in the maneuver
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetTurnInformation(NB_RouteInformation* route, uint32 index, NB_LatitudeLongitude* turnPoint, char* primaryNameBuffer, uint32 primaryNameBufferSize,
    char* secondaryNameBuffer, uint32 secondaryNameBufferSize, char* currentNameBuffer, uint32 currentNameBufferSize, double* distance, nb_boolean collapsed);

/*! Get turn information on the current route

@param route NB_RouteInformation previously created route object
@param index Index of the maneuver to get turn information about
@param turnPoint the location of the maneuver
@param primaryNameBuffer On success, the primary road name or place name
@param primaryNameBufferSize The size of the primary name buffer
@param secondaryNameBuffer On success, the secondary road name or location name
@param secondaryNameBufferSize The size of the secondary name buffer
@param currentNameBuffer On success, the current road name
@param currentNameBufferSize The size of the current road name buffer
@param secondaryCurrentNameBuffer On success, the secondary current road name
@param secondaryCurrentNameBufferSize The size of the secondary current road name buffer
@param distance On success, the (possibly collapsed) distance remaining in the maneuver
@param speed On success, base speed maneuver
@param stackAdvise On success, stacked instruction is advised
@param commandBuffer On success, The command buffer
@param commandBufferSize On success, The size of the command buffer
@param collapsed Whether or not to return the collapsed or base distance remaining in the maneuver
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetTurnInformation2(NB_RouteInformation* route, uint32 index, NB_LatitudeLongitude* turnPoint, char* primaryNameBuffer, uint32 primaryNameBufferSize,
    char* secondaryNameBuffer, uint32 secondaryNameBufferSize, char* currentNameBuffer, uint32 currentNameBufferSize, char* secondaryCurrentNameBuffer, uint32 secondaryCurrentNameBufferSize,
    double* distance, double* speed, nb_boolean* stackAdvise, char* commandBuffer, uint32 commandBufferSize, nb_boolean collapsed);

/*! Get the name of the current road before an maneuver

@param route NB_RouteInformation previously created route object
@param index Index of the maneuver to get turn information about
@param primaryNameBuffer On success, the primary road name or place name
@param primaryNameBufferSize The size of the primary name buffer
@param secondaryNameBuffer On success, the secondary road name or location name
@param secondaryNameBufferSize The size of the secondary name buffer
@returns NB_Error
*/

NB_DEC NB_Error NB_RouteInformationGetCurrentRoadNames(NB_RouteInformation* route, uint32 index,
    char* primaryNameBuffer, uint32 primaryNameBufferSize, char* secondaryNameBuffer, uint32 secondaryNameBufferSize);

/*! Gets the cross streets of a maneuver

@param route NB_RouteInformation previously created route object
@param index Index of the maneuver
@param crossStreets array of cross streets for maneuver
@param crossStreetCount number of cross street for particular maneuver
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetManeuverCrossStreetsCount(NB_RouteInformation* route, uint32 index, uint32* crossStreetCount);

/*! Gets the cross streets of a maneuver

@param route NB_RouteInformation previously created route object
@param index Index of the maneuver
@param crossStreets array of cross streets for maneuver
@param crossStreetCount number of cross street for particular maneuver
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetManeuverCrossStreets(NB_RouteInformation* route, uint32 index, uint32 crossStreetIndex, char* crossStreets);

/*! Get traffic flags for the route

@param route NB_RouteInformation previously created route object
@param flags On success, the traffic flags associated with the route
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetTrafficFlags(NB_RouteInformation* route, NB_TrafficType* flags);


/*! Gets origin of the route

@param route NB_RouteInformation previously created route object
@param origin On success, return route origin
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetOrigin(NB_RouteInformation* route, NB_Place* origin);


/*! Gets origin GPS location that initiated the route

@param route NB_RouteInformation previously created route object
@param origin On success, return route origin GPS location
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetOriginGpsLocation(NB_RouteInformation* route, NB_GpsLocation* location);


/*! Gets destination of the route

@param route NB_RouteInformation previously created route object
@param origin On success, return route destination
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetDestination(NB_RouteInformation* route, NB_Place* destination);

/*! Get the transport mode for this route

@param route NB_RouteInformation previously created route object
@param origin On success, returns the transport mode
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetTransportMode(NB_RouteInformation* route, NB_NavigateTransportMode* mode);

/*! Gets if route is complete or not

@param route NB_RouteInformation previously created route object
@returns nb_boolean
*/
NB_DEC nb_boolean NB_RouteInformationIsRouteComplete(NB_RouteInformation* route);

/*! Gets if maneuver is destination unknown or not

@param route NB_RouteInformation previously created route object
@param index Index of the maneuver
@returns nb_boolean
*/
NB_DEC nb_boolean NB_RouteInformationIsManeuverComplete(NB_RouteInformation* route, uint32 index);

/*! Gets the Route ID of a route

    The route-ID object is owned by NB_RouteInformation and is valid until NB_RouteInformation is destroyed.

@param route NB_RouteInformation previously created route object
@param routeId pointer to route id object of route
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetRouteId(NB_RouteInformation* route, NB_RouteId** routeId);


/*! Gets if maneuver is within the route polyline window

@param route NB_RouteInformation previously created route object
@param index Index of the maneuver
@returns nb_boolean
*/
NB_DEC nb_boolean NB_RouteInformationIsManeuverWithinRouteWindow(NB_RouteInformation* route, uint32 index);


/*! Gets the appropriate lat/long center point for a vector turn map of a specified maneuver

@param route NB_RouteInformation previously created route object
@param maneuverIndex Index of the maneuver
@param centerPoint On success, the computed center point of the maneuver turn map
@param heading On success, the computed heading of the maneuver turn map
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetManeuverTurnMapCenterPoint(NB_RouteInformation* route, uint32 manueverIndex, NB_LatitudeLongitude* centerPoint, double* heading);


/*! Gets the turn country code of a specified maneuver

@param route NB_RouteInformation previously created route object
@param maneuverIndex Index of the maneuver
@param countryCode Pointer for pointer to the country code
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetTurnCountryCode(NB_RouteInformation* route, uint32 manueverIndex, const char** countryCode);

/* Gets the first major road name
 @param route NB_RouteInformation previously created route object
 @param nameBuffer On success, the primary road name
 @param nameBufferSize The size of the name buffer
 @returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetFirstMajorRoadName(NB_RouteInformation* route, char* nameBuffer, uint32 nameBufferSize);


/*! Get exit number on the current route

 @param route NB_RouteInformation previously created route object
 @param index Index of the maneuver to get turn information about
 @param turnPoint the location of the maneuver
 @param exitNumber On success, exit number
 @param exitNumberBufferSize The size of the exit number buffer
 @returns NB_Error
 */
NB_DEC NB_Error NB_RouteInformationGetExitNumber(NB_RouteInformation* route,
                                                 uint32 index,
                                                 NB_LatitudeLongitude* turnPoint,
                                                 char* exitNumber,
                                                 uint32 exitNumberBufferSize
                                                 );
/*! Get lane characters on the current route

 @param route NB_RouteInformation previously created route object
 @param index Index of the maneuver to get turn information about
 @param turnPoint the location of the maneuver
 @param LaneCharacters On success, lane characters
 @param LaneCountSize The unit count of the lane characters
 @returns NB_Error
 */
NB_DEC NB_Error NB_RouteInformationGetLaneCharacters(NB_RouteInformation* route,
                                                     uint32 index,
                                                     NB_LatitudeLongitude* turnPoint,
                                                     NB_Lane* LaneCharacters,
                                                     uint32 LaneCountSize
                                                     );

/*! Get route count

@param route NB_RouteInformation previously created route object
@param routesCount number of routes
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetRoutesCount(NB_RouteInformation* route, uint32* routesCount);

/*! Set active route

@param route NB_RouteInformation previously created route object
@param activeRoute index for active route - starting from 1
NOTE: to updated traffic information you should call the separate function: NB_RouteInformationGetTrafficInformation
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationSetActiveRoute(NB_RouteInformation* route, uint32 activeRoute);

/*! Get on route information

@param route NB_RouteInformation previously created route object
@param onRouteInfo on route information
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationOnRouteInfo(NB_RouteInformation* route, NB_OnRouteInformation* onRouteInfo);

/*! Get route style

@param route NB_RouteInformation previously created route object
@param routeStyle current route style
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetRouteStyle(NB_RouteInformation* route, NB_RouteSettings* routeStyle);

/*! Get route label point

@param route NB_RouteInformation previously created route object
@param NB_LatitudeLongitude label point latitude and longtitude
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetLabelPoint(NB_RouteInformation* route, NB_LatitudeLongitude* latLong);

/*! Get detour current route label point - only valid with detour request

@param route NB_RouteInformation previously created route object
@param NB_LatitudeLongitude label point latitude and longtitude
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetDetourCurrentLabelPoint(NB_RouteInformation* route, NB_LatitudeLongitude* latLong);

/*! Get guidance point for turn instruction of the maneuver

 @param route NB_RouteInformation previously created route object
 @param index Index of the maneuver to get turn information about
 @param index guidancePointIndex index of the guidance point to get turn information about. could be 0 or 1
 @param pType guidance point type
 @param pGuidancePoint guidance point latitude and longtitude
 @param pManeuverPointOffset Distance to the maneuver point
 @returns NB_Error
 */
NB_DEC NB_Error
NB_RouteInformationGetTurnGuidancePointInformation(NB_RouteInformation* route,
                                               uint32 index,
                                               NB_GuidancePointType* pType,
                                               NB_LatitudeLongitude* pGuidancePoint,
                                               double* pManeuverPointOffset
                                               );

/*! Get guidance point for prepare instruction of the maneuver

 @param route NB_RouteInformation previously created route object
 @param index Index of the maneuver to get turn information about
 @param index guidancePointIndex index of the guidance point to get turn information about. could be 0 or 1
 @param pType guidance point type
 @param pGuidancePoint guidance point latitude and longtitude
 @param pManeuverPointOffset Distance to the maneuver point
 @param pUseForPrepare guidance point can be referenced in the prepare instruction
 @param pUseForTurn guidance point can be referenced in the turn instruction
 @returns NB_Error
 */
NB_DEC NB_Error
NB_RouteInformationGetPrepareGuidancePointInformation(NB_RouteInformation* route,
                                                   uint32 index,
                                                   NB_GuidancePointType* pType,
                                                   NB_LatitudeLongitude* pGuidancePoint,
                                                   double* pManeuverPointOffset
                                                   );

/*! Retrieves a NB_TrafficInformantion object containing traffic information corresponded to the passed Route

@param handler A NB_RouteInformation object
@param route On success, a NB_TrafficInformation object with the result of the last download; NULL otherwise. An object returned from this function must be destroyed using NB_RouteDestroy().
NOTE: also see NB_RouteInformationSetActiveRoute
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetTrafficInformation(NB_RouteInformation* route, NB_TrafficInformation** information);

/*! Clone a routeID.

    The returned route-ID needs to be destroyed using NB_RouteIdDestroy() once not needed anymore.

    @param originalRouteId route-ID to clone
    @param newRouteId cloned route-ID

    @see NB_RouteIdDestroy
    @returns NB_Error
*/
NB_DEC NB_Error NB_RouteIdClone(const NB_RouteId* originalRouteId, NB_RouteId** newRouteId);

/*! Destroy a cloned route-ID.

    !IMPORTANT: Don't call this function on a route-ID returned by NB_RouteInformationGetRouteId(). That route-ID is owned by the route information object.
    Only use this function to destroy route-IDs created by NB_RouteIdClone().

    Function has no effect on empty route-ID

    @param routeId Route-ID to destroy. Has to be created by NB_RouteIdClone(). Pointer gets reset after destruction.

    @see NB_RouteIdClone
    @returns NB_Error

 */
NB_DEC NB_Error NB_RouteIdDestroy(NB_RouteId** routeId);

/*! Gets disable guidance info

 @param route NB_RouteInformation previously created route object
 @param index Index of maneuver
 @param pguidance On return disable guidance info
 @returns NB_Error
 */
NB_DEC NB_Error NB_RouteInformationGetManeuverGuidanceInformation(NB_RouteInformation* route, uint32 index, nb_boolean* pGuidanceDisabled);

/*! Get a pronun information

@param route NB_RouteInformation previously created route object
@param key name of base voice or street voice
@param textBuffer plain text buffer
@param textBufferSize size of plain text buffer (return 0 if it is empty)
@param phoneticsData Phonetic representation of the text (if available), empty otherwise. The text is encoded using UTF-8 format
@param phoneticsDataSize Size of Phonetics data (return 0 if it is empty)
@param duration time of playback duration in ms
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetPronunInformation(NB_RouteInformation* routeInformation, const char* key,
                                                        char* textBuffer, uint32* textBufferSize,
                                                        byte* phoneticsData, uint32* phoneticsDataSize,
                                                        char* language, uint32* languageDataSize,
                                                        char* translatedText, uint32* translatedTextDataSize,
                                                        double* duration);

/*! Get the maximum turn distance for street count.
@param route NB_RouteInformation previously created route object
@param index Index of the maneuver to get turn information about
@param maxTurnDistance on Success, the maximum turn distance for street count
@returns NB_Error
 */

NB_DEC NB_Error NB_RouteInformationGetMaxTurnDistanceForStreetCount(NB_RouteInformation* routeInformation,
                                                                    uint32 index, double* maxTurnDistance);

/*! Get the route information which contains single route.
@param route NB_RouteInformation previously created route object
@param activeRoute index for active route - starting from 1
@param singleRouteInformation routeInformation which contains single route.
       singleRouteInformation should be destroyed by NB_RouteInformationDestroy()
@returns NB_Error
 */

NB_DEC NB_Error NB_RouteInformationGetSingleRouteInformation(NB_RouteInformation* routeInformation,
                                                             uint32 index,
                                                             NB_RouteInformation** singleRouteInformation);

/*! Get number of lanes on the current route and position of the lanes for the maneuver

 @param route NB_RouteInformation previously created route object
 @param index Index of the maneuver to get lane information about
 @param numberOfLanes On success, The number of lanes that are used in the maneuver
 @param lanePosition On success, The position of the lanes starting from the leftmost lanes, 1-based.
        Example, if the road has 7 lanes and two lanes on the right (leaving the rightmost lane out)
        are used, then this will contain 5 and number-of-lanes will be 2 meaning lanes 5
        and 6 are used for the maneuver.
 @returns NB_Error
 */
NB_DEC NB_Error NB_RouteInformationGetLaneInfo(NB_RouteInformation* route,
                                               uint32 index,
                                               uint32 *numberOfLanes,
                                               uint32 *lanePosition);

/*! Get current route description.

 @param route NB_RouteInformation previously created route object
 @param description Const pointer of description data
 @return NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetRouteDescription(NB_RouteInformation* route,
                                                       const char** description);


/*! Get route summary information.

 @param route NB_RouteInformation previously created route object
 @param distance Provides the driving distance in meters
 @param travelTime Provides the driving time for the route, including traffic delays
 @param via Provides the name of the longest segment in the route, to be used when displaying the route as in "via XX"
 @param delayTime Provides the driving delay time for the route
 @param index the index of route summary, for FastETA, the route summary count is get from NB_RouteInformationGetRouteSummaryInformationCount
 @return NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetRouteSummaryInformation(NB_RouteInformation* route,
                                                              uint32* distance,
                                                              uint32* travelTime,
                                                              const char** via,
                                                              uint32* delayTime,
                                                              uint32  index);

/*! Get route summary information.
 
 @param route NB_RouteInformation previously created route object
 @param count the route summary count
 @return NB_Error
 */
NB_DEC NB_Error NB_RouteInformationGetRouteSummaryInformationCount(NB_RouteInformation* route,
                                                              uint32* count);

/*! Get traffic events count of route information.

 @param route NB_RouteInformation previously created route object
 @param count Count of traffic events
 @return NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetTrafficEventCount(NB_RouteInformation* route,
                                                        uint32* count);

/*! Get traffic event point of route information.

 @param route NB_RouteInformation previously created route object
 @param count Count of traffic events
 @return NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetTrafficEvent(NB_RouteInformation* route,
                                                   uint32 index,
                                                   NB_TrafficEvent* trafficEvent);

/*! Get traffic color of route information.

 @param route NB_RouteInformation previously created route object
 @param color The Char pointer of traffic color
 @return NB_Error
*/
NB_DEC NB_Error NB_RouteInformationGetTrafficColor(NB_RouteInformation* route,
                                                   char* color);


/*! Get traffic delay time of route information.

 @param route NB_RouteInformation previously created route object
 @param maneuverIndex The maneuver index.
 @param delay The traffic delay time.
 @return NB_Error
 */
NB_DEC NB_Error NB_RouteInformationGetTrafficDelay(NB_RouteInformation* route,
                                                   uint32 maneuverIndex,
                                                   uint32* delay);

/*! Get direction name from loction to route start.

 @param route NB_RouteInformation previously created route object
 @param projLatitude The project latitude of location
 @param projLongitude The project longitude of location
 @param directionName The const char pointer of direction name.
 @param headingToOrigin The output value of heading to origin place.
 @return NB_Error
 */
NB_DEC NB_Error NB_RouteInformationGetDirectionToRoute(NB_RouteInformation* route,
                                                       double projLatitude,
                                                       double projLongitude,
                                                       const char** directionName,
                                                       double* headingToOrigin);

/*! Update description with current route and detour.

 @param route NB_RouteInformation previously created route object
 @param detourRoute NB_RouteInformation detour route object
 @return NB_Error
 */
NB_DEC NB_Error NB_RouteInformationUpdateDescriptionWithDetour(NB_RouteInformation* route,
                                                               NB_RouteInformation* detourRoute);


/*! Get maneuver propertys. */
NB_DEC NB_Error NB_RouteInformationGetManeuverPropertys(NB_RouteInformation* route, uint32 index, NB_OnRouteInformation* propertys);

/*! get/set trafficinformation. */
NB_TrafficInformation* NB_RouteInformationGetTrafficInformationPointer(NB_RouteInformation* route);
void NB_RouteInformationSetTrafficInformationPointer(NB_RouteInformation* route, NB_TrafficInformation* info);

/* @} */

#endif
