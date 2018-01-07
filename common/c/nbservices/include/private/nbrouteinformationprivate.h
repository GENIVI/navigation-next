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

    @file     nbrouteinformationprivate.h

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

#ifndef NBROUTE_H
#define NBROUTE_H

#include "nbrouteinformation.h"
#include "nbcontext.h"
#include "tpselt.h"
#include "instructset.h"
#include "data_nav_reply.h"
#include "nbrouteparametersprivate.h"
#include "navtypes.h"
#include "data_blob.h"
#include "cslcache.h"
#include "data_nav_maneuver.h"

#include "nbnavigationstate.h"
#include "navpublictypes.h"
#include "nbtrafficinformation.h"
#include "nbtrafficprocessorprivate.h"
#include "nbcameraprocessor.h"
#include "nbenhancedcontenttypes.h"
#include "nbspeedlimitprocessorprivate.h"
#include "nbguidanceprocessorprivate.h"

/*!
    @addtogroup nbroute
    @{
*/

/*! @struct NB_Route
Application-specific route object
*/
struct NB_RouteInformation
{
    NB_Context*                 context;
    nb_boolean                  camerasOnRoute;
    nb_boolean                  hasUnpavedRoads;
    NB_NavigateTransportMode    transportMode;
    nb_boolean                  collapseInformational;
    nb_boolean                  useFetchCorridor;

    NB_Place                    start;
    NB_Place                    destination;

    NB_GpsLocation              originFix;

    double                      routeLength;
    uint32                      routeTime;
    uint32                      routeDelay;

    NB_RoutePrivateConfiguration config;
    NB_RouteConfiguration       parameterConfiguration;
    NB_RouteParameters*         routeParameters;

    NB_RouteId                  routeID;

    double                      tempDouble;
    uint32                      tempUint32;
    int32                       tempInt32;

    data_nav_reply              dataReply;

    uint32                      numberOfSelector;
    uint32                      currentActiveSelector;
    NB_OnRouteInformation       onRouteInfo;
    char**                      routeDescription;
    const char*                 currentDescription;

    NB_TrafficInformation*      trafficInformation;
    nb_boolean                  trafficDataValid;
};

struct NB_RouteDataSourceOptions
{
    NB_RouteInformation* route;
    NB_NavigateManeuverPos maneuverPos;
    nb_boolean collapse;
    NB_PositionRoutePositionInformation* currentRoutePosition;
    NB_TrafficInformation* trafficInfo;
    NB_TrafficState* trafficState;
    NB_CameraState* cameraState;
    NB_NavigationState* navState;
    NB_SpeedLimitState* speedLimitState;
    NB_GuidanceState* guidanceState;
    nb_boolean buttonPressed;         // TODO: provide this value from client
    nb_boolean isLookahead;
    nb_boolean isPositionOnlyForLocation;  /* TURE for can only be used for getting latitude and longtitude from currentRoutePosition. */
    nb_boolean isManeuverList;             /* TURE for maneuver turn list. */
};


/*! @struct NB_RouteRealisticSign
Content information for a route realistic sign.  Data valid only for life of NB_RouteInformation object.
*/
typedef struct NB_RouteRealisticSign
{
    const char* signId;         /*!< Identifier used to retrieve the sign graphics from the content server */
    const char* signPortraitId; /*!< Identifier used to retrieve sign graphics optimized for display on a
                                     portrait-oriented device. If blank, no optimized graphics available. */
    const char* signFileType;   /*!< File type of extended content (e.g. PNG) */
    const char* datasetId;      /*!< The dataset id to distinguish country */
    const char* version;        /*!< Current version of the sign.  Note this is the version associated
                                     with the content, not manifest version associated with the dataset. */
    uint32      maneuver;       /*!< Maneuver index of the sign. */
    double      positionMeters; /*!< The position of the realistic sign relative to the start of this
                                     maneuver (i.e. the distance from the previous maneuver) in meters.*/
} NB_RouteRealisticSign;


/*! @enum NB_RouteContentRegionType
Content Region Type
 */
typedef enum {
    NB_RCRT_Invalid = 0,
    NB_RCRT_JunctionModel,                  /*!< Junction model content region*/
    NB_RCRT_CityModel,                      /*!< City model content region */

} NB_RouteContentRegionType;


/*! @struct NB_RouteContentRegion
Content region information.  Data valid only for life of NB_RouteInformation object.
*/
typedef struct NB_RouteContentRegion
{
    NB_RouteContentRegionType type;

    const char* regionId;               /*!< Identifier of content region */
    const char* datasetId;              /*!< Identifier of content region dataset */
    const char* version;                /*!< Content region version */

    uint32      startManeuver;          /*!< Start maneuver index of content region */
    double      startManeuverOffset;    /*!< Start maneuver offset */
    uint32      endManeuver;            /*!< End maneuver index of content region */
    double      endManeuverOffset;      /*!< End maneuver offset */

    uint32      splinePathCount;        /*!< Count of spline paths for this content region */

    /* Projection parameters */
    NB_EnhancedContentProjectionType    projection; /*!< Projection type */
    NB_EnhancedContentProjectionDatum   datum;      /*!< Projection datum */
    double      originLatitude;         /*!< Latitude of origin for the projection. */
    double      originLongitude;        /*!< Longitude of the origin of the projection. */
    double      scaleFactor;            /*!< Always 0.9996 for UTM. */
    double      falseEasting;           /*!< 500000.0 for UTM. */
    double      falseNorthing;          /*!< 10000000.0 for UTM in the southern hemisphere. */
    double      zOffset;                /*!< A constant value to add to all z-coordinates in the model to
                                             obtain the actual height off the referenced ellipsoid. */
    /* Computed measurements*/
    double                  distanceFromRouteStart; /*!< Computed region distance from route start */
    double                  length;                 /*!< Computed region length */
    NB_LatitudeLongitude    endPoint;               /*!< Computed region end point */

} NB_RouteContentRegion;


/*! Gets information from tpselt reply and fills in data_nav_reply structure with appropriate information

@param context NB_Context
@param tpselt tpselt that we will get information of the data_nav_reply from
@param params params route parameters used to make a route request
@param information A non-NULL route object must be destroyed with nsl_free
@returns NB_Error
*/
NB_Error NB_RouteInformationCreateFromTPSReply(NB_Context* context, tpselt reply, NB_RouteParameters* params, NB_RouteInformation** information);

/*! Data source function for the route object

@param elem data source element type
@param nman index of maneuver of desired information
@param pUser callback data
@param pdata name
@param psize size
@param pfreedata
@returns NB_Error
*/
boolean NB_RouteInformationDataSource(uint32 elem, uint32 nman, void* pUser, const char** pdata, nb_size* psize, boolean* pfreedata);

/*! Gets information from tpselt reply and fills in data_nav_reply structure with appropriate information

@param route previously created route object
@param id
@returns NB_Error
*/
NB_Error NB_RouteInformationGetID(NB_RouteInformation* route, data_blob** id);

/*! Copies route settings

@param dst destination NB_RouteSettings
@param src source NB_RouteSettings
@returns NB_Error
*/
NB_Error NB_RouteInformationCopyRouteSettings(NB_RouteSettings* destination, NB_RouteSettings* source);

int NB_RouteInformationFindSegmentMatches(NB_RouteInformation* route, NB_GpsLocation* pfix, double heading, uint32 manprogress, uint32 segprogress, double last_heading, struct seg_match* pmatches, int nmaxmatches, struct route_position* prevpos);

double NB_RouteInformationManeuverRemainingDist(data_util_state* dataState, data_nav_reply* preply, uint32 maneuver, uint32 segment, double segment_remain);

double NB_RouteInformationTraveledDistance(data_util_state* dataState, data_nav_reply* preply, uint32 maneuver, uint32 segment, double segment_remain);

void NB_RouteInformationTripRemainAfterManeuver(NB_RoutePreferencesPrivate* prefs, data_util_state* pdus, data_nav_reply* preply, uint32 nmaneuver, double* pdist, uint32* ptime);

void NB_RouteInformationSpeedCameraRemain(NB_RoutePreferencesPrivate* prefs, data_util_state* pdus, data_nav_reply* preply, uint32 nmaneuver, double* pdist, uint32* ptime);

uint32    NB_RouteInformationGetTurnTime(NB_RoutePreferencesPrivate* prefs, const char* szturn);

uint32    NB_RouteInformationGetManueverTime(NB_RoutePreferencesPrivate* prefs, data_util_state* pds, data_nav_reply* preply, uint32 nmaneuver);

struct seg_match* NB_RouteInformationBestMatch(struct seg_match* new_match, struct seg_match* cur_match, boolean use_speed, boolean* pchange);

int NB_RouteInformationFindBestMatch(NB_RouteInformation* route, struct seg_match* pmatches, int nmatches, boolean use_speed);

data_nav_reply* NB_RouteInformationGetDataNavReply(NB_RouteInformation *route);

NB_RoutePrivateConfiguration* NB_RouteInformationGetPrivateConfiguration(NB_RouteInformation* route);

double NB_RouteInformationGetInstructionDistance(NB_RoutePreferencesPrivate* prefs, double speed, enum manuever_type manueuver_type, enum instruct_dist_type dist_type);

data_nav_maneuver* NB_RouteInformationGetManeuver(NB_RouteInformation* route, uint32 maneuverIndex);

NB_Error NB_RouteGetDistanceBetweenManeuvers(NB_RouteInformation* route, uint32 startManeuver, uint32 endManeuver, double* pdist);

NB_Error NB_RouteProtectVoiceCache(NB_RouteInformation* route);

NB_Error NB_RouteInformationGetTMCSpeed(NB_RouteInformation* route, const char* tmcloc, TrafficSpeedType speed_type, double* pspeed, NB_NavigateTrafficCongestionType* ptc);

NB_Error NB_RouteCollapseManeuvers(NB_RouteInformation* route, uint32 nman, double* padddist, uint32* paddtime,
                                   nb_boolean* pstack, uint32* ppmaneuver_base, uint32* ppmaneuver, uint32* pnman,
                                   double* padddist_next, uint32* paddtime_next, nb_boolean* pstacknext,
                                   uint32* ppmaneuvernext, uint32* pnman_next, boolean isManeuverList);

nb_boolean NB_RouteInformationCompareRouteIDs(NB_RouteInformation* route1, NB_RouteInformation* route2);

void NB_RouteInformationGetRouteStart(NB_RouteInformation* route, double* latitude, double* longitude);

NB_Error NB_RouteInformationGetPlayTimeForMessageCode(NB_RouteInformation* route, const char* code, uint32* time);


/*! Gets the count of realistic signs for a specified maneuver

@param route NB_RouteInformation previously created route object
@param maneuverIndex Index of the maneuver
@returns Count of realistic signs for maneuver
*/
uint32 NB_RouteInformationGetManeuverRealisticSignCount(NB_RouteInformation* route, uint32 maneuverIndex);


/*! Gets the realistic sign information for the specified realistic sign of a specified maneuver

@param route NB_RouteInformation previously created route object
@param maneuverIndex Index of the maneuver
@param signIndex Index of the realistic sign for the maneuver
@param realisticSign Pointer to NB_RouteRealisticSign structure.  Data only valid during for life of NB_RouteInformation object.
@returns NB_Error
*/
NB_Error NB_RouteInformationGetManeuverRealisticSign(NB_RouteInformation* route, uint32 maneuverIndex, uint32 signIndex, NB_RouteRealisticSign* realisticSign);


/*! Gets the count of content regions in route

@param route NB_RouteInformation previously created route object
@returns Count of content regions for route
*/
uint32 NB_RouteInformationGetContentRegionCount(NB_RouteInformation* route);


/*! Gets the content region information for the specified content region

@param route NB_RouteInformation previously created route object
@param regionIndex Index of the content region
@returns NB_Error
*/
NB_Error NB_RouteInformationGetContentRegion(NB_RouteInformation* route, uint32 regionIndex, NB_RouteContentRegion* contentRegion);


/*! Gets a spline path for the specified content region

@param route NB_RouteInformation previously created route object
@param regionIndex Index of the content region
@param splineIndex Index of requested spline; should not exceed splinePathCount in NB_RouteContentRegion
@param pathId On return the ID of the spline path
@param splineSize On return the size of the retrieved spline
@param splineData On return the request packed spline data
@returns NB_Error
*/
NB_Error NB_RouteInformationGetContentRegionSplinePath(NB_RouteInformation* route, uint32 regionIndex, uint32 splineIndex, const char** pathId, uint32* splineSize, const uint8** splineData);

/*! Gets a positions for street count guidance point

 @param route NB_RouteInformation previously created route object
 @param index Index of maneuver
 @param startDistance On return the distance of the crossing street when announcement shall start
 @param endDistance On return the distance of the crossing street when announcement shall end
 @returns NB_Error
 */
NB_Error
NB_RouteInformationGetDistancesForStreetCountGuidancePoint(NB_RouteInformation* route, uint32 index, double* startDistance, double* endDistance);

/*! Destroy route description.

 @param route NB_RouteInformation previously created route object
 @returns NB_Error
*/
NB_Error
NB_RouteInformationDestroyDescription(NB_RouteInformation* route);

/*! Allocates and initializes NB_RouteInformation

  @param context pointer of NB_Context instance
  @returns Allocated route information.
*/
NB_RouteInformation* NB_RouteInformationAlloc(NB_Context* context);

double GetRouteLength(NB_RouteInformation* route);
void GetRouteTimeAndDelay(NB_RouteInformation* route,  uint32* pTime, uint32* pDelay);

#endif // NBROUTEPRIVATE_H
