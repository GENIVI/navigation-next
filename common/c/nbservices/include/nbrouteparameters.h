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

    @file     nbrouteparameters.h

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

#ifndef NBROUTEPARAMS_H
#define NBROUTEPARAMS_H

#include "nbdetourparameters.h"
#include "nbrouteinformation.h"
#include "nbroutesettings.h"
#include "nbgpshistory.h"
#include "nbplace.h"
#include "nbtrafficinformation.h"

/*!
    @addtogroup nbrouteparameters
    @{
*/

#define NB_PHONETICS_FORMAT_MAX 250

typedef enum NB_RouteDetectionLevel
{
    NBRDL_Relaxed = 0, /* default */
    NBRDL_Tight        /*!< If tight is selected, then the map matching is tighter, resulting in faster detection
                            of turns and off-route conditions, but also more unwanted recalculations due to poor GPS fixes. */
} NB_RouteDetectionLevel;

typedef enum NB_ManueverType
{
    NBMT_Normal = 0,
    NBMT_Highway,
    NBMT_Destination,
    NBMT_Origin,
    NBMT_Merge,
    NBMT_Ferry,
    NBMT_Camera,
    NBMT_Max
} NB_ManueverType;

typedef enum NB_InstructDistanceType
{
    NBIDT_BaseDistance = 0,
    NBIDT_AnnounceDistance,
    NBIDT_ContinueDistance,
    NBIDT_PrepareDistance,
    NBIDT_PrepareMinDistance,
    NBIDT_InstructionDistance,
    NBIDT_ShowStackDistance,
    NBIDT_ShowVectorDistance,
    NBIDT_LaneGuidanceDistance,
    NBIDT_Max
} NB_InstructDistanceType;

typedef struct
{
    NB_Location location;
    nb_boolean isStopPoint;
} NB_WayPoint;

/*! @struct NB_NavigationInstruction
Navigation Instruction
*/
typedef struct NB_NavigationInstruction
{
    double      baseDistMul;
    double      baseDistAdd;
    double      mult[NBIDT_Max];

    double      ShowTapeDistanceInInches;
    double      ShowTapeDistanceInMeters;
    double      TapeDistanceInMeters;
    double      TapeDistanceInInches;
} NB_NavigationInstruction;

/*! @struct NB_RouteOptions
User routing options
*/
typedef struct NB_RouteOptions
{
    NB_RouteType               type;           /*!< Route type (fastest, easiest, etc) */
    NB_RouteAvoid              avoid;          /*!< Things to avoid (toll roads, uturns, etc) */
    NB_TransportationMode      transport;      /*!< Transportation mode (car, pedestrian, etc) */
    NB_TrafficType             traffic;        /*!< Traffic type */
    char*                      pronunStyle;    /*!< Name of voice style */
} NB_RouteOptions;


/*! @struct NB_RouteConfiguration
Application-specific routing configuration
*/
typedef struct NB_RouteConfiguration
{
    nb_boolean wantOriginLocation;             /*!< Whether or not to include the origin lat/long in the reply */
    nb_boolean wantDestinationLocation;        /*!< Whether or not to include the destination lat/long in the reply */
    nb_boolean wantRouteMap;                   /*!< Specify true for v4 navigation compatibility */
    nb_boolean wantCrossStreets;
    nb_boolean wantPolyline;                   /*!< Whether or not to include entire polyline in reply (yes for web) */
    nb_boolean wantManeuvers;                  /*!< DEPRECATED Whether or not to include maneuver information in reply (no for web) */
    nb_boolean wantFerryManeuvers;
    nb_boolean wantTowardsRoadInformation;
    nb_boolean wantIntersectionRoadInformation;
    nb_boolean wantCountryInformation;
    nb_boolean wantEnterRoundaboutManeuvers;
    nb_boolean wantEnterCountryManeuvers;
    nb_boolean wantBridgeManeuvers;
    nb_boolean wantTunnelManeuvers;
    nb_boolean wantRouteExtents;
    nb_boolean wantUnpavedRoadInformation;
    nb_boolean wantSpeedCameras;
    nb_boolean wantSpeedLimit;
    nb_boolean wantMaxTurnDistanceForStreetCount;

    uint32  routeTimeOverride;                 /*!< If specified, GPS time to use instead of 'now' for speed and traffic information */
    uint32  routeMapAngleOfView;               /*!< Route map angle of view */
    uint32  routeMapDistanceOfView;            /*!< Route map distance of view */
    uint32  routeMapMinTime;                   /*!< Route map min time */
    uint32  routeMapPasses;                    /*!< Route map passes */
    uint32  routeMapTileZ;                     /*!< Route map tile z */
    uint32  routeMapMaxTiles;                  /*!< Route map max tiles */
    uint32  commandSetVersion;                 /*!< Command Set Version Passed in from the Guidance Processor */
    nb_boolean disableNavigation;              /*!< Return static directions only, no navigation or guidance information*/
    uint32  maxPronunFiles;                    /*!< Max number of voices returned on initial route query */

    nb_boolean  wantRealisticSigns;            /*!< Whether or not to include realistic signs in the reply */
    nb_boolean  wantJunctionModels;            /*!< Whether or not to include junction models in the reply */
    nb_boolean  wantCityModels;                /*!< Whether or not to include city models in the reply */
    nb_boolean  wantNonGuidanceManeuvers;      /*!< Whether or not client wants non-guidance maneuvers through areas */

    nb_boolean wantGuidanceConfig;             /*!< Enable guidance config */
    nb_boolean wantExitNumbers;                /*!< Enable exit numbers */
    nb_boolean wantNaturalGuidance;            /*!< Enable natural guidance */
    nb_boolean wantExtendedNaturalGuidance;    /*!< Enable extended natural guidance */
    nb_boolean wantLaneGuidance;               /*!< Enable lane guidance */
    nb_boolean wantAlternateRoutes;            /*!< Whether or not to include alternate routes */
    nb_boolean wantLabelPoints;                /*!< Whether or not to include label points */
    nb_boolean wantPlayTimes;                  /*!< Whether or not to include messages play time */
    nb_boolean wantCompleteRoute;              /*!< Whether or not to get the entire route without any route window limitations */
    char       supportedPhoneticsFormats[NB_PHONETICS_FORMAT_MAX];  /*!< Phonetics formats supported by
                                                                 the client, comma separated Example: X-SAMPA,IPA */
    NB_RouteDetectionLevel routeDetectionLevel; /*!< If tight is selected, then the map matching is tighter, resulting in faster detection
                                                of turns and off-route conditions, but also more unwanted recalculations due to poor GPS fixes. */
    nb_boolean wantRouteSummary;                /*!< Whether or not to include route summary. */
    nb_boolean wantRouteSummaryDelayTime;       /*!< Weather or not to include rotue summary delay time. */
    nb_boolean wantSpecialRegions;              /*!< Whether or not to include want special regions. */
    nb_boolean wantFirstManeuverCrossStreets;   /*!< Whether or not to include want first maneuver cross streets. */
    nb_boolean wantSecondaryNameForExits;       /*!< Whether or not to inclued want secondary name for exits. */
    nb_boolean wantCheckUnsupportedCountries;   /*!< Whether or not to include want check unsupported countries. */
    nb_boolean wantFirstMajorRoad;              /*!< Whether or not to include want first major road. */
    nb_boolean startToNavigate;                 /*!< Whether or not to include start to navigate. */

    double orManeuverTurnAngleTolerance;        /*!< The angle tolerance from current location to origin. */
    nb_boolean wantPedestrianManeuvers;         /*!< Whether or not to include want pedestrian maneuvers. */
    double viaPointThreshold;                   /*!< Via point threshold */
    double announcementHeadsUpInterval;         /*!< announcement heads up interval. */
} NB_RouteConfiguration;

/*! @struct NB_RouteParameters
Opaque data structure that defines the parameters to generate a route
*/
typedef struct NB_RouteParameters NB_RouteParameters;

/*! Create route parameters using GPS history as the starting point

@param context NB_Context
@param history Where the route starts
@param destination Where the route ends
@param options User options that influence the route
@param configuration Application configuration that specifies data options
@param parameters On success, a newly created NB_RouteParameters object; NULL otherwise.  A valid object must be destroyed using NB_RouteParametersDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteParametersCreateFromGpsHistory(NB_Context* context, NB_GpsHistory* history, NB_Place* destination, NB_RouteOptions* options, NB_RouteConfiguration* configuration, NB_RouteParameters** parameters);

/*! Create route parameters using a place as the starting point

@param context NB_Context
@param origin Where the route starts
@param destination Where the route ends
@param options User options that influence the route
@param configuration Application configuration that specifies data options
@param parameters On success, a newly created NB_RouteParameters object; NULL otherwise.  A valid object must be destroyed using NB_RouteParametersDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteParametersCreateFromPlace(NB_Context* context, NB_Place* origin, NB_Place* destination, NB_RouteOptions* options, NB_RouteConfiguration* configuration, NB_RouteParameters** parameters);


/*! Create route parameters for an off-route route

@param context NB_Context
@param route Off-route NB_Route object
@param parameters On success, a newly created NB_RouteParameters object; NULL otherwise.  A valid object must be destroyed using NB_RouteParametersDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteParametersCreateOffRoute(NB_Context* context,  NB_GpsHistory* gpsHistory, NB_RouteInformation* route, NB_RouteParameters** parameters);

/*! Create route parameters for an off-route route

@param context NB_Context
@param route Off-route NB_Route object
@param parameters On success, a newly created NB_RouteParameters object; NULL otherwise.  A valid object must be destroyed using NB_RouteParametersDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteParametersCreateOffRouteEx(NB_Context* context,  NB_GpsHistory* gpsHistory, NB_RouteInformation* route, NB_RouteParameters** parameters, nb_boolean wantAlternateRoute);


/*! Create route parameters for a detour

@param context NB_Context
@param route NB_Route requiring the detour
@param traffic the NB_TrafficInformation associated with the route
@param avoid NB_DetourParameters object with route sections to avoid, or NULL to clear all detours
@param parameters On success, a newly created NB_RouteParameters object; NULL otherwise.  A valid object must be destroyed using NB_RouteParametersDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteParametersCreateDetour(NB_Context* context, NB_RouteInformation* route, NB_TrafficInformation* traffic, NB_DetourParameters* detour, NB_GpsHistory* gpsHistory, NB_RouteParameters** parameters);

/*! Create route parameters for a detour with changed options

@param context NB_Context
@param route NB_Route requiring the detour
@param traffic the NB_TrafficInformation associated with the route
@param avoid NB_DetourParameters object with route sections to avoid, or NULL to clear all detours
@param options User options that influence the route
@param parameters On success, a newly created NB_RouteParameters object; NULL otherwise.  A valid object must be destroyed using NB_RouteParametersDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteParametersCreateDetourWithChangedOptions(NB_Context* context, NB_RouteInformation* route, NB_TrafficInformation* traffic, NB_DetourParameters* detour, NB_GpsHistory* gpsHistory, NB_RouteOptions* options, NB_RouteParameters** parameters);

/*! Create route parameters for request, based on exist route, but with updated options

@param context NB_Context
@param route NB_Route requiring the detour
@param traffic the NB_TrafficInformation associated with the route
@param avoid NB_DetourParameters object with route sections to avoid, or NULL to clear all detours
@param options User options that influence the route
@param parameters On success, a newly created NB_RouteParameters object; NULL otherwise.  A valid object must be destroyed using NB_RouteParametersDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteParametersCreateChangeRouteOptions(NB_Context* context, NB_RouteInformation* route, NB_TrafficInformation* traffic, NB_DetourParameters* detour, NB_GpsHistory* gpsHistory, NB_RouteOptions* options, NB_RouteParameters** parameters, nb_boolean wantAlternateRoute);

/*! Create route parameters for user-initiated recalculation

@param context NB_Context
@param route NB_Route requiring the detour
@param traffic the NB_TrafficInformation associated with the route
@param avoid NB_DetourParameters object with route sections to avoid, or NULL to clear all detours
@param options User options that influence the route
@param parameters On success, a newly created NB_RouteParameters object; NULL otherwise.  A valid object must be destroyed using NB_RouteParametersDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteParametersCreateRecalculateRoute(NB_Context* context, NB_RouteInformation* route, NB_TrafficInformation* traffic, NB_DetourParameters* detour, NB_GpsHistory* gpsHistory, NB_RouteOptions* options, NB_RouteParameters** parameters);


/*! Create route parameters for a route recreation (complete route)

@param context NB_Context
@param route NB_Route requiring the detour
@param parameters On success, a newly created NB_RouteParameters object; NULL otherwise.  A valid object must be destroyed using NB_RouteParametersDestroy()
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteParametersRecreateRoute(NB_Context* context, NB_RouteInformation* route, NB_RouteParameters** parameters);

/*! Destroy a previously created RouteParameters object

@param parameters A NB_RouteParameters object created with any of the NB_RouteParametersCreate functions
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteParametersDestroy(NB_RouteParameters* parameters);


/*! Set the language setting

When a NB_RouteParameters is created, the language defaults to US English. Client can call this function
to change the language of the route request.

@param parameters A NB_RouteParameters object created with any of the NB_RouteParametersCreate functions
@param language 2 to 5 charaeter string specifying the language of the request. If NULL, default US English is assumed.
@returns NB_Error
 */
NB_DEC NB_Error NB_RouteParametersSetLanguage(NB_RouteParameters* parameters, const char* language);

/*! Update route parameter with route options
@param route parameters, which have to be updated
@param options User options that influence the route
@returns NB_Error
*/
NB_DEC NB_Error NB_SetDataQueryOptionsForRouteParameters(NB_RouteParameters* pRouteParameters, NB_RouteOptions* options);

/*! Update route parameter with navigation instruction
@param route parameters, which have to be updated
@param instructions navigation instructions
@returns NB_Error
*/
NB_DEC NB_Error NB_RouteParametersSetNavigationInstruction(NB_RouteParameters* pRouteParameters, NB_NavigationInstruction* instruction, NB_ManueverType maneuverType);

/*! Update route parameter From TPS Buffer
 @param context NB_Context
 @param route parameters, which have to be updated
 @param datalib TPL buffer
 @param datalibSize TPL buffer size
 @param configBuffer TPS buffer
 @param configBufferSize TPS buffer size
 @returns NB_Error
 */
NB_DEC NB_Error
NB_RouteParametersSetConfigurationFromBuffer(NB_Context* context,
                                             NB_RouteParameters* pRouteParameters,
                                             char *datalib,
                                             nb_size datalibSize,
                                             char * configBuffer,
                                             nb_size configBufferSize
                                             );

/*! Update font map From TPS Buffer
 @param context NB_Context
 @param route parameters, which have to be updated
 @param datalib TPL buffer
 @param datalibSize TPL buffer size
 @param fontMapBuffer  font map TPS buffer
 @param fontMapBufferSize font map TPS buffer size
 @returns NB_Error
 */
NB_DEC NB_Error
NB_RouteParametersSetFontMapFromBuffer(NB_Context* context,
                                       NB_RouteParameters* pRouteParameters,
                                       char *datalib,
                                       nb_size datalibSize,
                                       char * fontMapBuffer,
                                       nb_size fontMapBufferSize
                                       );
/*! Update HOV map From TPS Buffer
 @param context NB_Context
 @param route parameters, which have to be updated
 @param datalib TPL buffer
 @param datalibSize TPL buffer size
 @param HOVFontMapBuffer  font map TPS buffer
 @param HOVFontMapBufferSize font map TPS buffer size
 @returns NB_Error
 */
NB_DEC NB_Error
NB_RouteParametersSetHOVFontMapFromBuffer(NB_Context * context,
                                          NB_RouteParameters* pRouteParameters,
                                          char *datalib,
                                          nb_size datalibSize,
                                          char * HOVFontMapBuffer,
                                          nb_size HOVFontMapBufferSize
                                          );

/*! Create route parameters for request, based on exist route, but with updated options

 @returns NB_Error
 */
NB_DEC NB_Error
NB_RouteParametersCreateChangeRouteOptionsAndConfig(NB_Context* context,             /*!< NB_Context */
                                                    NB_RouteInformation* route,      /*!< NB_Route requiring the */
                                                    NB_TrafficInformation* traffic,  /*!< the NB_TrafficInformation associated with the route */
                                                    NB_DetourParameters* detour,     /*!< NB_DetourParameters object with route sections to avoid, or NULL to clear all detours */
                                                    NB_GpsHistory* gpsHistory,       /*!< gps history*/
                                                    NB_RouteOptions* options,        /*!< User options that influence the route */
                                                    NB_RouteConfiguration* config,   /*!< User config that influenec the route */
                                                    NB_RouteParameters** parameters, /*!< On success, a newly created NB_RouteParameters object; NULL otherwise.  A valid object must be destroyed using NB_RouteParametersDestroy()*/
                                                    nb_boolean wantAlternateRoute    /*!< want multiple routes or not*/);


/*! Add via point to route parameters.
 
 @return NB_Error
 */
NB_Error
NB_RouteParametersSetViaPoint(NB_RouteParameters* pRouteParameters, NB_WayPoint* pWayPoint);

/*! Add route summary extra destination, route summary support muti destination.
 @return NB_Error
 */
NB_Error
NB_RouteParametersSetRouteSummaryDestination(NB_RouteParameters* pRouteParameters, NB_Place* pPlace);
/*! @} */

#endif
