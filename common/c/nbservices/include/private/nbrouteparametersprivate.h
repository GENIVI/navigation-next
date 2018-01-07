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

    @file     nbrouteparametersprivate.h
    
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

#ifndef NBROUTEPARAMETERSPRIVATE_H
#define NBROUTEPARAMETERSPRIVATE_H

#include "tpselt.h"
#include "nbrouteparameters.h"
#include "navprefs.h"
#include "data_nav_query.h"
#include "nbqalog.h"
#include "cslhashtable.h"

/*!
    @addtogroup nbrouteparameters
    @{
*/

/*! @struct NB_RoutePrivateConfiguration
    Application-specific private routing preferences
*/

#define MAX_NUM_TURN_TIMES 11
#define MAX_NUM_TURN_COMMAND    256
#define MAX_NUM_FONT_MAP        2048
#define MAX_NUM_HOV_MAP         64
typedef struct NB_RoutePreferencesPrivate
{    
    uint32                   minNoDataDisableTrafficBar;
    NB_VectorMapPreferences  preferences3D;
    NB_VectorMapPreferences  preferences2D;
    uint32                   navRouteMapPasses;
    uint32                   navRouteMapTZ;
    uint32                   numInitialTurnmapTiles;
    double                   navMinTmcSpeed;
    navturntime              navTurnTimes[MAX_NUM_TURN_TIMES];
    int                      numNavTurnTimes;
    navturnindex             navTurnIndex[MAX_NUM_TURN_COMMAND];
    uint32                   laneGuidanceFontMapVersion;
    CSL_HashTable*           laneGuidanceFontMap;
    uint32                   hovFontMapVersion;
    int                      numHovFontMap;
    HovFontMap               hovFontMap[MAX_NUM_HOV_MAP];
    int                      numNavTurnIndex;
    double                   bboxbuffer;
    int                      navWWIgnore;
    int                      navOrIgnore;
    int                      navStartupMinMatches;
    int                      navEllErrorEnable;
    int                      navL1HeadingMargin;
    double                   navL1DistThreshold;
    int                      navL2HeadingMargin;
    double                   navL2DistThreshold;
    int                      navL3HeadingMargin;
    double                   navL3DistThreshold;
    int                      navL4HeadingMargin;
    double                   navL4DistThreshold;
    int                      navL5HeadingMargin;
    double                   navL5DistThreshold;
    int                      navL6HeadingMargin;
    double                   navL6DistThreshold;
    int                      navL1HeadingMarginTightLevel;
    double                   navL1DistThresholdTightLevel;
    double                   navMinSpeedValidHeadingCar;
    double                   navMinSpeedValidHeadingPed;
    int                      navHeadingMargin;
    double                   navMaxError;
    double                   navMinError;
    double                   navMaxErrorTightLevel;
    double                   navMinErrorTightLevel;
    double                   navSifterError;
    double                   navStartThresh;
    double                   navEndThresh;
    double                   navTurnThreshold;
    double                   navHideDistThreshold;
    double                   navTurnBuf;
    double                   navSoftRecalcDist;
    double                   minCameraInstructDistance;
    double                   navCameraDistThreshold;
    navinstruct              instruct[maneuver_type_max];
    double                   navAvgSpeedA;
    int                      navNumPtsForValid;
    double                   navDefaultManeuverSpeed;
    double                   navAvgSpeedThreshold;
    int                      navSvgSpeedMinUpdateBelow;
    int                      navAvgSpeedMinUpdateAbove;
    double                   navMaxTrafficIncidentAnnounce;
    double                   navMinTrafficIncidentAnnounce;
    double                   navMaxTrafficCongestionAnnounce;
    double                   navMinTrafficCongestionAnnounce;
    double                   navMinTrafficCongestionLengthAnnounce;
    uint32                   minTrafficMeterLightTraffic;
    uint32                   maxTrafficMeterLightTraffic;
    uint32                   minTrafficMeterModerateTraffic;
    uint32                   maxTrafficMeterModerateTraffic;
    uint32                   minTrafficMeterSevereTraffic;
    uint32                   maxTrafficMeterSevereTraffic;
    uint32                   minTrafficMeterDelayAnnounce;
    uint32                   maxTrafficMeterDelayAnnounce;
    double                   maxTowardDistAnnounce;
    double                   minTrafficMeterChangeToAnnounce;
    uint32                   minRemainTrafficUpdateAnnounce;
    double                   snapMapThreshold;
    double                   navPedRecalcThreshold;
    double                   turnMapHeadingDistanceFromManeuver;
    uint32                   maxEnhancedContentPrefetchManeuvers;
    uint32                   minGapBetweenContinueAndPrepareAnnounce;
} NB_RoutePreferencesPrivate;

/*! @struct NB_RoutePrivateConfiguration
Application-specific private routing configuration
*/
typedef struct NB_RoutePrivateConfiguration
{
    NB_RoutePreferencesPrivate prefs;
} NB_RoutePrivateConfiguration;

struct NB_RouteParameters
{
    NB_Context*                     context;
    data_nav_query                  dataQuery;
    char*                           pronunStyle;
    char*                           language;
    NB_RoutePrivateConfiguration    privateConfig;
    NB_RouteConfiguration           config;
    NB_GpsLocation                  originFix;
    NB_QaLogRouteQueryType          type;
};

/*! Convert NB_RouteParameters data_nav_query to tps struct

@param parameters parameters previously created NB_RouteParameters object
@returns tpselt
*/
tpselt NB_RouteParametersToTPSQuery(NB_RouteParameters* parameters);

/*! Gets the route settings of a NB_RouteParameters object

@param parameters parameters previously created NB_RouteParameters object
@param settings destination NB_RouteSettings where the settings will be copied
@returns NB_RoutePrivateConfiguration
*/
NB_Error NB_RouteParametersGetRouteSettingsFromQuery(NB_RouteParameters* parameters, NB_RouteSettings* settings);

/*! Copy configuration settings of a NB_RouteParameters object

@param parameters previously created NB_RouteParameters object
@param dst destination NB_RoutePrivateConfiguration where the private configuration will be copied
@returns NB_RoutePrivateConfiguration
*/
NB_Error NB_RouteParametersCopyPrivateConfigFromParameters(NB_RouteParameters* params, NB_RoutePrivateConfiguration* dst);

/*! Copy the private configuration settings of a NB_RouteParameters object

@param parameters previously created NB_RouteParameters object
@param dst destination NB_RouteConfiguration where the configuration will be copied
@returns NB_RoutePrivateConfiguration
*/
NB_Error NB_RouteParametersCopyConfigFromParameters(NB_RouteParameters* params, NB_RouteConfiguration* dst);


/*! Gets origin GPS location that initiated the route

@param params NB_RouteParameters previously created route parameters object
@param origin On success, return route origin GPS location
@returns NB_Error
*/
NB_Error NB_RouteParametersGetOriginGpsLocation(NB_RouteParameters* params, NB_GpsLocation* location);

/*! Gets whether the Route Parameters specify to use traffic for route calculation

*/
nb_boolean NB_RouteParametersIsTrafficRoute(NB_RouteParameters* params);

NB_Error NB_RouteParametersPreferencesPrivateInit(NB_RoutePreferencesPrivate* pprefs);

NB_Error NB_RouteParametersClone(NB_RouteParameters* params, NB_RouteParameters** clone);


/*! Gets the route destination information

@param params NB_RouteParameters previously created route parameters object
@param destinationLatitudeLongitude On success, destination lat/long
@param destinationLatitudeAddress On success, destination street
@returns NB_Error
*/
NB_Error NB_RouteParametersGetDestination(NB_RouteParameters* params, NB_LatitudeLongitude* destinationLatitudeLongitude, const char** destinationStreet);

NB_Error NB_RouteParametersGetTrafficFlags(NB_RouteParameters* parameters, NB_TrafficType* flags);

/*! Get the maximum number of pronun files to download in the route request
@param params NB_RouteParameters previously created route parameters object
@param maxPronunFiles on Success, the the maximum number of pronun files to download in the route request
@returns NB_Error
 */
NB_Error NB_RouteParametersGetMaxPronunFiles(NB_RouteParameters* params, uint32* maxPronunFiles);

/*! Set the maximum number of pronun files to download in the route request
@param params NB_RouteParameters previously created route parameters object
@param maxPronunFiles the maximum number of pronun files to download in the route request
@returns NB_Error
 */
NB_Error NB_RouteParametersSetMaxPronunFiles(NB_RouteParameters* params, uint32 maxPronunFiles);

/*! Update route parameter with navigation command type
 @param route parameters, which have to be updated
 @param command navigation command string
 @param command type
 @returns NB_Error
 */
NB_Error
NB_RouteParametersSetNavigationCommandType(NB_RouteParameters* pRouteParameters, const char * command, NB_ManueverType type);

/*! Get command type from route parameters
 @param params NB_RouteParameters previously created route parameters object
 @param command string of navigation command
 @returns NB_Error
 */
enum manuever_type
NB_RouteParametersGetCommandType(NB_RouteParameters* pRouteParameters,
                                 const char * command
                                 );
/*! Get command characters from route parameters
 @param params NB_RouteParameters previously created route parameters object
 @param command string of navigation command
 @param commandCharsBuffer string buffer of command characters
 @param commandCharBuffers string buffer size
 @returns NB_Error
 */
NB_Error
NB_RouteParametersGetCommandCharactersFromMapFont(NB_RouteParameters* pRouteParameters,
                                                  char * command,
                                                  char * commandCharsBuffer,
                                                  int    commandCharsBufferSize
                                                  );
/*! Get command characters from route parameters
 @param params NB_RouteParameters previously created route parameters object
 @param command string of navigation command
 @param commandCharsBuffer string buffer of command characters
 @param commandCharBuffers string buffer size
 @returns NB_Error
 */
NB_Error
NB_RouteParametersGetLaneCharactersFromMapFont(NB_RouteParameters* pRouteParameters,
                                               uint32 laneType,
                                               uint32 unselectedArrows,
                                               uint32 selectedArrows,
                                               char * unselectedCharsBuffer,
                                               int    unselectedCharsBufferSize,
                                               char * selectedCharsBuffer,
                                               int    selectedCharsBufferSize
                                               );

void DeleteEachEntryFromNewHashTable(void* value,
                                     void* userData
                                     );
void CopyEachEntryToNewHashTable(
                                 void * value,
                                 void * userData
                                 );
NB_Error
GetHOVFontMapFromBuffer(NB_Context * context,
                        char *datalib,
                        nb_size datalibSize,
                        char * fontMapBuffer,
                        nb_size fontMapBufferSize,
                        NB_RouteParameters *pRouteParameters
                        );

NB_Error
GetConfigurationDataFromBuffer(NB_Context * context,
                               char *datalib,
                               nb_size datalibSize,
                               char * configBuffer,
                               nb_size configBufferSize,
                               NB_RouteParameters *pRouteParameters
                               );
NB_Error
GetFontMapFromBuffer(NB_Context * context,
                     char *datalib,
                     nb_size datalibSize,
                     char * fontMapBuffer,
                     nb_size fontMapBufferSize,
                     NB_RouteParameters *pRouteParameters
                     );

#endif // NBROUTEPARAMETERSPRIVATE_H
