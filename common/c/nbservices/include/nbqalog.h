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

    @file     nbqalog.h
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

#ifndef NBQALOG_H
#define NBQALOG_H

#include "nbcontext.h"
#include "nbgpstypes.h"
#include "nbnavigationstate.h"
#include "nbcamerainformation.h"
#include "nbtrafficinformation.h"
#include "nbqarecord.h"

struct seg_match;

/*!
    @addtogroup nbqalog
    @{
*/

/*! Type used for NB_QaLogRasterTileEvent() */
typedef enum 
{
    NB_QLRTET_Start = 1,
    NB_QLRTET_End   = 2

} NB_QaLogRasterTileEventType;

/*! Type used for DTS and raster tile logging */
typedef enum 
{
    NB_QLMT_Map         = 0,
    NB_QLMT_Traffic     = 1,
    NB_QLMT_Route       = 2,

    NB_QLMT_BaseUrl     = 3,

    NB_QLMT_Satellite   = 4,
    NB_QLMT_Hybrid      = 5,

    NB_QLMT_PTRoute     = 6,

    NB_QLMT_PTRouteSelector1 = 7,
    NB_QLMT_PTRouteSelector2 = 8,
    NB_QLMT_PTRouteSelector3 = 9,

    NB_QLMT_None        = 100

} NB_QaLogMapType;

/* Route query type */
typedef enum 
{
    NB_QLRQT_Unknown        = 0,
    NB_QLRQT_FromPlace      = 1,
    NB_QLRQT_FromHistory    = 2,
    NB_QLRQT_Detour         = 3,
    NB_QLRQT_OffRoute       = 4,
    NB_QLRQT_Recreate       = 5
} NB_QaLogRouteQueryType;

/* CellID Network Type */
typedef enum
{
    NB_QLLNT_Unknown        = 0,
    NB_QLLNT_GSM            = 1,
    NB_QLLNT_CDMA           = 2,
    NB_QLLNT_WiFi           = 3
} NB_QaLogLocationNetworkType;

/* Traffic Event List Operation */
typedef enum
{
    NB_QLTELO_Add           = 0,
    NB_QLTELO_Remove        = 1

} NB_QaLogTrafficEventListOperation;

/* Realistic Sign State Type */
typedef enum
{
    NB_QLRSST_Undefined     = 0,
    NB_QLRSST_AutoOn        = 1,
    NB_QLRSST_AutoOff       = 2,
    NB_QLRSST_ManualOff     = 3

} NB_QaLogRealisticSignStateType;

/* Enhanced Content Type */
typedef enum
{
    NB_QLECT_Undefined          = 0,
    NB_QLECT_Junctions          = 1,
    NB_QLECT_RealisticSigns     = 2,
    NB_QLECT_CityModels         = 3,
    NB_QLECT_Textures           = 4,
    NB_QLECT_MapTiles           = 5,
    NB_QLECT_CommonTextures     = 6,
    NB_QLECT_SpecificTextures   = 7,
    NB_QLECT_SpeedLimit         = 8,
    NB_QLECT_SpecialRegion      = 9

} NB_QaLogEnhancedContentType;

typedef enum
{
    NB_QLECRS_Undefined         = 0,
    NB_QLECRS_Enter             = 1,
    NB_QLECRS_Exit              = 2

} NB_QaLogEnhancedContentRegionState;

/* Wanted Content Type */
typedef enum
{
    NB_QLWCT_Undefined          = 0,
    NB_QLWCT_MetadataSource     = 1,
    NB_QLWCT_Manifest           = 2

} NB_QaLogWanteedContentType;

/* Projection Type */
typedef enum
{
    NB_QLPT_Undefined                   = 0,
    NB_QLPT_Mercator                    = 1,
    NB_QLPT_TransverseMercator          = 2,
    NB_QLPT_UniversalTransverseMercator = 3

} NB_QaLogProjectionType;

/* Projection Datum */
typedef enum
{
    NB_QLPD_Undefined   = 0,
    NB_QLPD_WGS84       = 1,
    NB_QLPD_NAD83       = 2

} NB_QaLogProjectionDatum;

/* API Call Record Type */
typedef enum
{
    NB_QLAC_StartFunction  = 0, /*!< Start of function call */
    NB_QLAC_EndFunction    = 1, /*!< End of function call */
    NB_QLAC_StartCallback  = 2, /*!< Start of callback call */
    NB_QLAC_CallbackReturn = 3  /*!< Return from callback call */
} NB_QaLogApiCallRecordType;

/* Location Event */
typedef enum
{
    NB_QLLE_Request  = 0,    /*!< Start of location-based server request */
    NB_QLLE_Reply,           /*!< Start of location-based server request */
    NB_QLLE_StartTracking,   /*!< Start of a tracking mode */
    NB_QLLE_EndTracking      /*!< End of a tracking mode */
} NB_QaLogLocationEventType;

/* Download result */
typedef enum
{
    NB_QLDR_Undefined   = 0,
    NB_QLDR_Success     = 1,
    NB_QLDR_Failure     = 2
} NB_QaLogDownloadResult;

/* Enhanced Content Type */
typedef enum
{
    NB_QLECDT_Undefined     = 0,
    NB_QLECDT_Preload       = 1,
    NB_QLECDT_OnDemand      = 2
} NB_QaLogEnhancedContentDownloadType;

/* QA Log Location Source type type */
typedef enum
{
    NB_QLLST_TCSCellID = 0,     /*!< Start of location-based server request */
    NB_QLLST_TCSWiFi,           /*!< Start of location-based server request */
    NB_QLLST_OtherCellID,       /*!< Start of callback call */
    NB_QLLST_OtherWiFi,         /*!< Return from callback call */
    NB_QLLST_TCSCellIDLocal,
    NB_QLLST_TCSWiFiLocal
} NB_QaLogLocationSourceType;


/*! Write to the QA log file

@param context A pointer to the shared NB_Context instance
@param record The QA Log Record to write
@return NB_Error
*/
NB_DEC NB_Error NB_QaLogWrite(NB_Context* context, NB_QaRecord* record);

/*! Log an application state message to the QA Log

@param context A pointer to the shared NB_Context instance
@param state A NULL terminated string containing the state message
*/
NB_DEC void NB_QaLogAppState(NB_Context* context, const char* state);

/*! Log usage of a base audio file to the QA Log

@param context A pointer to the shared NB_Context instance
@param name A NULL terminated string containing the name of the file
*/
NB_DEC void NB_QaLogAudioBase(NB_Context* context, const char* name);

/*! Log usage of a downloaded audio file to the QA Log

@param context A pointer to the shared NB_Context instance
@param name A NULL terminated string containing the pronun code
*/
NB_DEC void NB_QaLogAudioPronun(NB_Context* context, const char* name);

/*! Log a audio cache lookup to the QA Log

@param context A pointer to the shared NB_Context instance
@param id A NULL terminated string containing the pronun code
*/
void NB_QaLogAudioCacheLookup(NB_Context* context, const char* id, nb_size idlen);

/*! Log a NB_GpsLocation to the QA Log

@param context A pointer to the shared NB_Context instance
@param location A pointer to the location to log
*/
NB_DEC void NB_QaLogGPSFix(NB_Context* context, NB_GpsLocation* location);

/*! Log a filtered NB_GpsLocation to the QA Log

@param context A pointer to the shared NB_Context instance
@param location A pointer to the location to log
*/
NB_DEC void NB_QaLogGPSFixFiltered(NB_Context* context, NB_GpsLocation* location);

/*! Log the fix filtered state to the QA Log

@param context A pointer to the shared NB_Context instance
@param sum The filter sum
@param constant The filter constant
@param latitude Accumulated latitude
@param longitude Accumulated longitude
*/
NB_DEC void NB_QaLogGPSFilterState(NB_Context* context, double sum, double constant, double latitude, double longitude);

/*! Log a route origin (NB_GpsLocation) to the QA Log

@param context A pointer to the shared NB_Context instance
@param location A pointer to the location to log
*/
NB_DEC void NB_QaLogRouteOrigin(NB_Context* context, NB_GpsLocation* location);

/*! Log a segment match to the QA Log

@param context A pointer to the shared NB_Context instance
@param segment A pointer to a route segment
@param bestMatch Non-zero if best segment match; zero otherwise
*/
NB_DEC void NB_QaLogSegmentMatch(NB_Context* context, struct seg_match* segment, nb_boolean bestMatch);


/*! Log a route id to the QA Log

@param context A pointer to the shared NB_Context instance
@param data The route id
@param dataSize The number of bytes in the route id
*/
NB_DEC void NB_QaLogRouteId(NB_Context* context, const byte* data, uint32 dataSize);


/*! Log a place to the QA Log

@param context A pointer to the shared NB_Context instance
@param data The place to log
@param label A label to associate with the place
*/
NB_DEC void NB_QaLogPlace(NB_Context* context, NB_Place* place, const char* label);


/*! Log a navigation error to the QA Log

@param context A pointer to the shared NB_Context instance
@param routeError A route error code
@param nbError A NB error code
@param networkStatus A network status code
@param function Function where the error is reported
@param file File where the error is reported
@param line Line number in the file where the error is reported
*/
NB_DEC void NB_QaLogNavigationError(NB_Context* context, NB_NavigateRouteError routeError, NB_Error nbError, NB_NetworkRequestStatus networkStatus, const char* function, const char* file, int line);


/*! Log voice cache information to the QA Log

@param context A pointer to the shared NB_Context instance
@param maxEntres The maximum number of voice cache entries
@param dirtyEntries The number of dirty, or memory-based, cache entries
@param cacheSize The size of the cache
@param protectedEntries The number of protected entries in the cache
@param downloadCount The number of downloaded voice files
@param deletedEntries The number of entries that have been deleted from the cache
*/
NB_DEC void NB_QaLogVoiceCache(NB_Context* context, int32 maxEntries, int32 dirtyEntries, int32 cacheSize, int32 protectedEntries, int32 downloadCount, int32 deletedEntries);

/*! Log traffic flow to the QA Log

@param context A pointer to the shared NB_Context instance
@param start start offset of the traffic region relative to the maneuver
@param length length of the traffic region
@param location location attribute of the traffic region
*/
NB_DEC void NB_QaLogRouteTrafficRegion(NB_Context* context, uint32 maneuver, double start, double length, const char* location);

/*! Log a traffic incident to the QA Log

@param context A pointer to the shared NB_Context instance
@param currentManeuver The index of the current maneuver
@param maneuverDistanceOffset the distance that has been travelled in the current maneuver
@param routeRemainTime The route remain travel time in seconds
@param routeRemainDelay The route remain delay in seconds
@param routeRemainMeter the route remain traffic meter
 */
NB_DEC void NB_QaLogTrafficState(NB_Context* context, uint32 currentManeuver, double maneuverDistanceOffset, uint32 routeRemainTime, uint32 routeRemainDelay, double routeRemainMeter);

/*! Log a traffic incident to the QA Log

@param context A pointer to the shared NB_Context instance
@param incident The traffic incident to log
@param place The place to log
@param label A label to associate with the traffic incident
*/
NB_DEC void NB_QaLogTrafficIncident(NB_Context* context, NB_TrafficIncident* incident, NB_Place* place, const char* label);


/*! Log traffic flow to the QA Log

@param context A pointer to the shared NB_Context instance
@param label A label to associate with the traffic flow
@param tmcLocation The TMC location code
@param color The traffic flow color
@param speed The speed
@param freeFlowSpeed The free flow speed
 */
NB_DEC void NB_QaLogTrafficFlow(NB_Context* context, const char* label, const char* tmcLocation, const char* color, double speed,  double freeFlowSpeed);


/*! Log traffic session id to the QA Log

@param context A pointer to the shared NB_Context instance
@param sessionId The traffic session id to log
*/
NB_DEC void NB_QaLogTrafficId(NB_Context* context, const char* sessionId);


/*! Log traffic congestion to the QA Log

@param context A pointer to the shared NB_Context instance
@param tmcLocation The TMC location code
@param distance Distance to the congestion
@param type The type of the congestion
@param speed The speed through the congestion
*/
NB_DEC void NB_QaLogTrafficCongestionWarning(NB_Context* context, const char* tmcLocation, double distance, NB_NavigateTrafficCongestionType type, double speed);


/*! Log traffic incident warning to the QA Log

@param context A pointer to the shared NB_Context instance
@param distance Distance to the incident
@param latitude The latitude of the incident
@param longitude The longitude of the incident
*/
NB_DEC void NB_QaLogTrafficIncidentWarning(NB_Context* context, double distance, double latitude, double longitude);


/*! Log traffic update to the QA Log

@param context A pointer to the shared NB_Context instance
@param newIncidents Number of new incidents
@param clearedIncidents Number of cleared incidents
@param travelTimeDelta Change in travel time due to incidents
*/
NB_DEC void NB_QaLogTrafficUpdate(NB_Context* context, uint32 newIncidents, uint32 clearedIncidents, uint32 travelTimeDelta);


/*! Log a prefetch area to the QA log

@param context A pointer to the shared NB_Context instance
@param segmentLatitude1 The latitude of the first point in which defines the prefetch segment
@param segmentLongitude1 The longitude of the first point in which defines the prefetch segment
@param segmentLatitude2 The latitude of the second point in which defines the prefetch segment
@param segmentLongitude2 The longitude of the second point in which defines the prefetch segment
@param boxLatitude1 The latitude of the first point in which defines the prefetch box
@param boxLongitude1 The longitude of the first point in which defines the prefetch box
@param boxLatitude2 The latitude of the second point in which defines the prefetch box
@param boxLongitude2 The longitude of the second point in which defines the prefetch box
@param boxLatitude3 The latitude of the third point in which defines the prefetch box
@param boxLongitude3 The longitude of the third point in which defines the prefetch box
@param boxLatitude4 The latitude of the fourth point in which defines the prefetch box
@param boxLongitude4 The longitude of the fourth point in which defines the prefetch box
*/

NB_DEC void NB_QaLogPrefetchArea(NB_Context* context, double segmentLatitude1, double segmentLongitude1, 
     double segmentLatitude2, double segmentLongitude2, double boxLatitude1, double boxLongitude1, 
     double boxLatitude2, double boxLongitude2, double boxLatitude3, double boxLongitude3, 
     double boxLatitude4, double boxLongitude4);

/*! Log a prefetch area to the QA log

@param context A pointer to the shared NB_Context instance
@param latitude The latitude of the prefetch point
@param longitude The longitude of the prefetch point
*/

NB_DEC void NB_QaLogPrefetchPoint(NB_Context* context, double latitude, double longitude);

/*! Log a request for a vector tile

@param context A pointer to the shared NB_Context instance
@param tileIndex describes the tile being requested
*/
NB_DEC void NB_QaLogRequestVectorTile(NB_Context* context, uint32 tx, uint32 ty, uint32 tz);

#define NB_RECEIVE_TILE_INITIAL 0
#define NB_RECEIVE_TILE_REFRESH 1

/*! Log receipt of a vector tile

@param context A pointer to the shared NB_Context instance
@param tileIndex describes the tile being received
*/
NB_DEC void NB_QaLogReceiveVectorTile(NB_Context* context, uint32 type, uint32 tx, uint32 ty, uint32 tz);

/*! Log discarding of a vector tile from the cache

@param context A pointer to the shared NB_Context instance
@param tileIndex describes the tile being discarded
*/
NB_DEC void NB_QaLogDiscardVectorTile(NB_Context* context, uint32 tx, uint32 ty, uint32 tz);

/*! Log the information about the current state of the vector tile manager

@param context A pointer to the shared NB_Context instance
@param totaltiles The total number of tiles either in memory, pending or downloaded
@param validTiles The total number of valid tiles in memory
@param pendingTiles The total number of tiles that have been requested, but are not yet downloading
@param downloadingTiles The total number of tiles currently being downloaded
@param discardedTiles The total number of tiles that have been discarded from the cache
@param lastData The data size for the last download request
@param totalData The total data size for tile downloads
*/
NB_DEC void NB_QaLogVectorTileState(NB_Context* context, uint32 totalTiles, uint32 validTiles, 
                uint32 pendingtiles, uint32 downloadingtiles, uint32 discardedtiles, 
                uint32 lastData, uint32 totalData);

/*! Log the status of an device analytics event

@param context A pointer to the shared NB_Context instance
@param eventId The unique Id of the Device Analytics event within the
               analytics session
@param timeStamp The timestamp of the device analytics event
@param sessionId The device analytics session Id
@param eventType The type of the device analytics event
@param eventStatus The status of the device analytics event
*/
NB_DEC void NB_QaLogAnalyticsEvent(NB_Context* context,
        uint32 eventId, uint32 timeStamp, uint32 sessionId,
        uint8 eventType, uint8 eventStatus);

/*! Log the current device analytics configuration in use.

@param context A pointer to the shared NB_Context instance
@param maxHoldTime device analytics config attr. max-hold-time
@param maxDataAge device analytics config attr. max-data-age
@param maxRequestSize device analytics config attr. max-request-size
@param goldEventsPriority device analytics config element gold events priority
@param poiEventsPriority device analytics config element POI events priority
@param routeTrackingPriority device analytics config element route tracking
       events priority
@param gpsProbesPriority device analytics config element gps probes priority
@param gpsProbesSampleRate device analytics config element gps probes sample
       rate
*/

NB_DEC void NB_QaLogAnalyticsConfig(NB_Context* context,
        uint32 maxHoldTime, uint32 maxDataAge, uint32 maxRequestSize,
        uint8 goldEventsPriority, uint8 poiEventsPriority,
        uint8 routeTrackingPriority, uint8 gpsProbesPriority,
        uint32 gpsProbesSampleRate);

/*! Log the request to the dynamic tile server.

    @return None
*/
NB_DEC void NB_QaLogDTSRequest(NB_Context* context);

/*! Log the reply from the dynamic tile server.

    @param context      A pointer to the shared NB_Context instance
    @param type         Type of reply. Currently either map, traffic or route
    @param url          Base URL + template
                 
    @return None
*/
NB_DEC void NB_QaLogDTSReply(NB_Context* context,
                 NB_QaLogMapType type,
                 const char* url);

/*! Log raster tile request to the server.

    @param context      A pointer to the shared NB_Context instance
    @param mapType      Map type
    @param xPosition    x-position of requested tile
    @param yPosition    y-position of requested tile
    @param zoomLevel    zoom level of requested tile
    @param resolution   Resolution of raster tile
    @param tileSize     Tile size (width/height)
    @param url          Url of raster tile request

    @return None
*/
NB_DEC void NB_QaLogRasterTileRequest(NB_Context* context,                         
                          NB_QaLogMapType mapType,
                          uint32 xPosition,
                          uint32 yPosition,
                          uint32 zoomLevel,
                          uint32 resolution,
                          uint32 tileSize,
                          const char* url);

/*! Log server reply for raster tile request.

    @param context          A pointer to the shared NB_Context instance
    @param mapType          Map type of reply
    @param xPosition        x-position of returned tile
    @param yPosition        y-position of returned tile
    @param zoomLevel        zoom level of returned tile
    @param resolution       Resolution of raster tile
    @param tileSize         Tile size (width/height)
    @param tileBufferSize   Buffer size of returned raster tile
    @param result           Result of raster tile reply

    @return None
*/
NB_DEC void NB_QaLogRasterTileReply(NB_Context* context,
                        NB_QaLogMapType mapType,
                        uint32 xPosition,
                        uint32 yPosition,
                        uint32 zoomLevel,
                        uint32 resolution,
                        uint32 tileSize,
                        uint32 tileBufferSize,
                        NB_Error result);

/*! Log status change in raster tile cache.

    @param context          A pointer to the shared NB_Context instance
    @param totalCount       Number of tiles in cache
    @param memoryCount      Number of tiles in memory cache
    @param persistentCount  Number of tiles in persistent/file cache
    @param pendingCount     Number of tiles in pending list
    @param discardedCount   Number of discarded tiles

    @return None
*/
NB_DEC void NB_QaLogRasterCacheState(NB_Context* context,
                         uint32 totalCount, 
                         uint32 memoryCount, 
                         uint32 persistentCount, 
                         uint32 pendingCount, 
                         uint32 discardedCount);

/*! Log change in center position for raster tile map.

    @param context              A pointer to the shared NB_Context instance
    @param oldCenterLatitude    Old latitude of center of map
    @param oldCenterLongitude   Old longitude of center of map
    @param newCenterLatitude    New latitude of center of map
    @param newCenterLongitude   New longitude of center of map

    @return None
*/
NB_DEC void NB_QaLogRasterTileCenterChange(NB_Context* context,
                               double oldCenterLatitude, 
                               double oldCenterLongitude, 
                               double newCenterLatitude, 
                               double newCenterLongitude);

/*! Log change in zoom level for raster tile map.

    @param context      A pointer to the shared NB_Context instance
    @param 
    @param 

    @return None
*/
NB_DEC void NB_QaLogRasterTileZoomChange(NB_Context* context,
                             uint32 oldZoomLevel, 
                             uint32 newZoomLevel);

/*! Log creation or destruction of raster tile map control.

    @param context          A pointer to the shared NB_Context instance
    @param centerLatitude   Center position of map
    @param centerLongitude  Center position of map
    @param zoomLevel        Zoom level for map
    @param functionName     Function or module that created the map or similar
    @param eventType        Event type to log

    @return None
*/
NB_DEC void NB_QaLogRasterTileEvent(NB_Context* context,
                        double centerLatitude, 
                        double centerLongitude, 
                        int zoomLevel, 
                        const char* functionName, 
                        NB_QaLogRasterTileEventType eventType);


/*! Log route query

@param context A pointer to the shared NB_Context instance
@param type The type of route being requested
@param originLatLong A pointer to the origin lat/long
@param destinationLatLong A pointer to the destination lat/long
@param settings A pointer to the route settings
*/
NB_DEC void NB_QaLogRouteQuery(NB_Context* context, NB_QaLogRouteQueryType type, NB_LatitudeLongitude* originLatLong, NB_LatitudeLongitude* destinationLatLong, NB_RouteSettings* settings);


/*! Log route reply

@param context A pointer to the shared NB_Context instance
@param originLatLong A pointer to the origin lat/long
@param destinationLatLong A pointer to the destination lat/long
@param data The route id
@param dataSize The number of bytes in the route id
@param full Non-zero if the route is complete; zero if the route is partial
*/
NB_DEC void NB_QaLogRouteReply(NB_Context* context, NB_LatitudeLongitude* originLatLong, NB_LatitudeLongitude* destinationLatLong, const byte* data, uint32 dataSize, nb_boolean full);


/*! Log nav session start

@param context A pointer to the shared NB_Context instance
@param sessiodId The navigation session id
*/
NB_DEC void NB_QaLogNavigationSessionStart(NB_Context* context, uint32 sessionId);


/*! Log that a route is being navigated within the session

@param context A pointer to the shared NB_Context instance
@param sessiodId The navigation session id
@param data The route id
@param dataSize The number of bytes in the route id
*/
NB_DEC void NB_QaLogNavigationSessionRoute(NB_Context* context, uint32 sessionId, const byte* data, uint32 dataSize);


/*! Log a recalc rwequest within the session

@param context A pointer to the shared NB_Context instance
@param sessiodId The navigation session id
@param reason The reason for the recalc
*/
NB_DEC void NB_QaLogNavigationSessionRecalc(NB_Context* context, uint32 sessionId, uint8 reason);


/*! Log nav session end

@param context A pointer to the shared NB_Context instance
@param sessiodId The navigation session id
*/
NB_DEC void NB_QaLogNavigationSessionEnd(NB_Context* context, uint32 sessionId);

/*! Log POI Along Route Search

@param context A pointer to the shared NB_Context instance
@param name The name of the POI being searched for
@param scheme The POI search scheme
@param categories The categories being searched for
@param numberOfCategories The number of categories
@param sliceSize The number of POIs to download
*/
NB_DEC void NB_QaLogPOIAlongRouteSearch(NB_Context* context, double startLatitude, double startLongitude, double distance, double width, const char* name, const char* scheme, const char** categories, uint32 numberOfCategories, uint32 sliceSize);

/*! Log navigation startup region

@param context A pointer to the shared NB_Context instance
@param centerLatitude The latitude of the circle center
@param centerLongitude The longitude of the circle center
@param radius The radius of the circle
@param distance The actual distance from the start of the route
*/
NB_DEC void NB_QaLogNavigationStartupRegion(NB_Context* context, double centerLatitude, double centerLongitude, double radius, double distance);

/*! Log Pronun File Download

@param context A pointer to the shared NB_Context instance
@param const char* codeData
@param nb_size codeSize
*/
NB_DEC void NB_QaLogPronunFileDownload(NB_Context* context, const char* codeData, nb_size codeSize);

/*! CellId Request

@param context A pointer to the shared NB_Context instance
@param signalStrengthDbm Signal strength (dbm)
@param mobileCountryCodeOrSystemId Mobile Country Code (GSM) / System ID (CDMA)
@param mobileNetworkCodeOrNetworkId Mobile Network Code (GSM) / Network ID (CDMA)
@param locationAreaCode Location Area Code (GSM)
@param cellId Cell ID
*/
NB_DEC void NB_QaLogCellIdRequest(NB_Context* context, NB_QaLogLocationNetworkType type, int16 signalStrengthDbm,
                                  uint32 mobileCountryCodeOrSystemId, uint32 mobileNetworkCodeOrNetworkId,
                                  uint32 locationAreaCode, uint32 cellId);

/*! CellId Wifi Request

@param context A pointer to the shared NB_Context instance
@param signalStrengthDbm Signal strength (dbm)
@param macAddress MAC Address

*/
NB_DEC void NB_QaLogCellIdWifiRequest(NB_Context* context, int16 signalStrengthDbm, const char* macAddress);

/*! CellId Reply

@param context A pointer to the shared NB_Context instance
@param latitude Latitude of network position returned
@param longitude Longitude of network position returned
@param accuracyMeters Accuracy of position (meters)
@param statusCode Status error code returned for location request
*/
NB_DEC void NB_QaLogCellIdReply(NB_Context* context, double latitude, double longitude, uint32 accuracyMeters, uint32 statusCode);

/* Log key press

@param context A pointer to the shared NB_Context instance
@param statusCode Code of key pressed
*/
NB_DEC void NB_QaLogKey(NB_Context* context, uint16 keyCode);

/*! Safety Camera

@param context A pointer to the shared NB_Context instance
@param camera  the camera to log
 */
NB_DEC void NB_QaLogCamera(NB_Context* context, NB_Camera* camera);

/*! Log camera warning to the QA Log

@param context A pointer to the shared NB_Context instance
@param distanceToCamera Distance to the camera
@param camera  the camera to log
 */
NB_DEC void NB_QaLogCameraWarning(NB_Context* context, double distanceToCamera, NB_Camera* camera);

/*! Log traffic event

@param context A pointer to the shared NB_Context instance
@param operation Traffic Event list operation
@param trafficEvent Traffic event to log
 */
NB_DEC void NB_QaLogTrafficEvent(NB_Context* context, NB_QaLogTrafficEventListOperation operation, NB_TrafficEvent* trafficEvent);

/*! Log App2App command

@param context A pointer to the shared NB_Context instance
@param command An App2App command 
 */
NB_DEC void NB_QaLogApp2AppCommand(NB_Context* context, const char* command);

/*! Log client session ID

@param context A pointer to the shared NB_Context instance
@param clientSessionId A unique session identifier for this client representing an application session with the server.  This value should be set in the client when the application is started.  A gpstime timestamp is a good choice here.
 */
NB_DEC void NB_QaLogClientSessionId(NB_Context* context, uint32 clientSessionId);

/*! NBI wifi probes event

@param context A pointer to the shared NB_Context instance
@param latitude Latitude of wifi probe
@param longitude Longitude of wifi probe
@param locationTimestamp Timestamp from the GPS fix
@param numNetworks Number of wifi network data records passed
@param macAddress The MAC address of a visible wifi network
@param ssid SSID of the Wi-Fi network
@param rssi Signal strength for the network
@param timedelta Timestamp at which the MAC address was found minus the locationTimestamp
*/
NB_DEC void NB_QaLogNbiWifiProbes(NB_Context* context, double latitude, double longitude,
                                  unsigned int locationTimestamp,
                                  unsigned char numNetworks, 
                                  const unsigned char* macAddress,
                                  const unsigned char* ssid,
                                  int rssi, unsigned int timedelta);

/*! Log metada source request.

Used for Enhanced Content for navigation.

@param context A pointer to the shared NB_Context instance
@param language A current language setting of the device
@param wantedContentCount A count of requested contents
@param wantExtendedMaps Non-zero if the extended maps is requested; zero otherwise
@param screenWidth A screen width of the device, specified in pixels
@param screenHeight A screen height of the device, specified in pixels
@param screenResolution A screen width of the device, specified in pixels
*/
NB_DEC void NB_QaLogMetadataSourceRequest(NB_Context* context, const char* language, const int wantedContentCount,
                                                const nb_boolean wantExtendedMaps, const uint32 screenWidth, const uint32 screenHeight,
                                                const uint32 screenResolution);

/*! Log wanted content

Used for Enhanced Content for navigation.

@param context A pointer to the shared NB_Context instance
@param requestType A type of request
@param contentType A type of content
@param country A country
*/
NB_DEC void NB_QaLogWantedContent(NB_Context* context, const unsigned char requestType, const char* contentType, const char* country);

/*! Log metadata source response

Used for Enhanced Content for navigation.

@param context A pointer to the shared NB_Context instance
@param citySummary A text of available city model content
@param baseUrl A url
@param projectionType A type of projection
@param templateCount A number of templates
*/
NB_DEC void NB_QaLogMetadataSourceResponse(NB_Context* context, const char* citySummary, const char* baseUrl,
                                           const char* projectionType, const int templateCount);

/*! Log metadata source template

Used for Enhanced Content for navigation.

@param context A pointer to the shared NB_Context instance
@param templateType A type of template
@param templateString A template
@param cacheId Cache Id
@param boxLatitude1 A first latitude of the box
@param boxLongitude1 A first longitude of the box
@param boxLatitude2 A second latitude of the box
@param boxLongitude2 A second longitude of the box
@param projectionDatum A datum of projection
@param projectionFalseEasting A false easting of projection
@param projectionFalseNorthing A false northing of projection
@param originLatitude A latitude of origin for the projection.
@param originLongitude A longitude of the origin of the projection
@param scaleFactor A scale factor
@param zOffset A constant value to add to all z-coordinates in the model to obtain the actual height off the referenced ellipsoid.
*/
NB_DEC void NB_QaLogMetadataSourceTemplate(NB_Context* context, const char* templateType, const char* templateString, const uint32 cacheId,
            const char* metadataId, const char* manifestVersion, const uint32 manifestSize, const char* displayName,
            const double boxLatitude1, const double boxLongitude1, const double boxLatitude2, const double boxLongitude2,
            const char* projectionDatum, const double projectionFalseEasting, const double projectionFalseNorthing,
            const double originLatitude, const double originLongitude, const double scaleFactor, const double zOffset);

/*! Log manifest request

Used for Enhanced Content for navigation.

@param context A pointer to the shared NB_Context instance
@param wantedContentCount a nbumer of request specific type of data (junction models, realistic road signs, city models, map tiles, etc.)
*/
NB_DEC void NB_QaLogManifestRequest(NB_Context* context, const int wantedContentCount);

/*! Log manifest response

Used for Enhanced Content for navigation.

@param context A pointer to the shared NB_Context instance
@param datasetId Dataset Id of item to request
@param tileContentCount A nubmer of tile contents
@param firstTileId First Tile Id
@param firstTileBoxLatitude1 A first latitude of the first tile box
@param firstTileBoxLongitude1 A first longitude of the first tile box
@param firstTileBoxLatitude2 A second latitude of the first tile box
@param firstTileBoxLongitude2 A second longitude of the first tile box
@param lastTileId Last Tile Id
@param lastTileBoxLatitude1 A first latitude of the last tile box
@param lastTileBoxLongitude1 A first longitude of the last tile box
@param lastTileBoxLatitude2 A second latitude of the last tile box
@param lastTileBoxLongitude2 A second longitude of the last tile box
*/
NB_DEC void NB_QaLogManifestResponse(NB_Context* context, const char* datasetId, const int tileContentCount,
            const char* firstTileId, const double firstTileBoxLatitude1, const double firstTileBoxLongitude1,
            const double firstTileBoxLatitude2, const double firstTileBoxLongitude2,
            const char* lastTileId, const double lastTileBoxLatitude1, const double lastTileBoxLongitude1,
            const double lastTileBoxLatitude2, const double lastTileBoxLongitude2);

/*! Log manifest response content

Used for Enhanced Content for navigation.

@param context A pointer to the shared NB_Context instance
@param contentFieldId Content Field Id
@param contentFileVersion File version of content
@param boxLatitude1 A first latitude of the box
@param boxLongitude1 A first longitude of the box
@param boxLatitude2 A second latitude of the box
@param boxLongitude2 A second longitude of the box
*/
NB_DEC void NB_QaLogManifestResponseContent(NB_Context* context, const char* contentFieldId, const char* contentFileVersion,
            const double boxLatitude1, const double boxLongitude1, const double boxLatitude2, const double boxLongitude2);

/*! Log realistic sign info

@param context A pointer to the shared NB_Context instance
@param sarId A identifier used to retrieve the sign graphics from the content server
@param sarType A file type of extended content (e.g. PNG)
@param dataset Dataset Id of item to request
@param version a current version of the sign
@param maneuver A maneuver
*/
NB_DEC void NB_QaLogRealisticSignInfo(NB_Context* context, const char* sarId, const char* sarType, const char* datasetId,
            const char* version,const int maneuver);

/*! Log realistic sign state

@param context A pointer to the shared NB_Context instance
@param sarId A identifier used to retrieve the sign graphics from the content server
@param sarState A state
@param latitude A latitude
@param longitude A longitude
*/
NB_DEC void NB_QaLogRealisticSignState(NB_Context* context, const char* sarId, const unsigned char sarState, const double latitude, const double longitude);

/*! Log enhanced content state.

    Used for Enhanced Content for navigation.
    Logs as navigation session enters and exits a junction or city model content region.

    @param context              A pointer to the shared NB_Context instance
    @param enhancedContentId    Content ID as string
    @param contentType          Content type, motor junction or city models
    @param state                Enter or exit region
    @param latitude             user position
    @param longitude            user position
*/
NB_DEC void 
NB_QaLogEnhancedContentState(NB_Context* context, 
                             const char* enhancedContentId,
                             NB_QaLogEnhancedContentType contentType,
                             NB_QaLogEnhancedContentRegionState state,
                             double latitude,
                             double longitude);

/*! Log enhanced content info

Used for Enhanced Content for navigation.

@param context A pointer to the shared NB_Context instance
@param ecId Enhanced info Id
@param ecType A Type of enhanced info
@param version A version
@param datasetId Dataset Id of item to request
@param startManeuver A start maneuver
@param maneuverOffset A offset of start maneuver
@param endManeuver A end maneuver
@param endManeuverOffset A offset of end maneuver
@param projectionType A type of projection; one of the NB_EnhancedContentProjectionType
@param projectionDatum A datum of projection
@param projectionFalseEasting A false easting of projection
@param projectionFalseNorthing A false northing of projection
@param originLatitude A latitude of origin for the projection.
@param originLongitude A longitude of the origin of the projection
@param scaleFactor A value of scale factor
@param zOffset A constant value to add to all z-coordinates in the model to obtain the actual height off the referenced ellipsoid.
*/
NB_DEC void NB_QaLogEnhancedContentInfo(NB_Context* context, const char* ecId, const unsigned char ecType, const char* version, const char* datasetId,
            const int startManeuver, const double maneuverOffset, const int endManeuver, const double endManeuverOffset,
            const unsigned char projectionType, const unsigned char projectionDatum, const double projectionFalseEasting, const double projectionFalseNorthing,
            const double originLatitude, const double originLongitude, const double scaleFactor, const double zOffset);

/*! Log Enhanced Content item downloading start record

Used for Enhanced Content for navigation.

@param context A pointer to the shared NB_Context instance
@param ecId Enhanced info Id
@param ecType A type of enhanced info
@param queueType A type of queue
@param version A version of content
@param size Item size
@param url Donwload URL
*/
NB_DEC void NB_QaLogEnhancedContentDownload(NB_Context* context, const char* ecId,
            const unsigned char ecType,
            const unsigned char queueType,
            const char* version,
            const int size,
            const char* url);

/*! Logging Enhanced Content downloading result record

Used for Enhanced Content for navigation.

@param context A pointer to the shared NB_Context instance
@param ecId item Id
@param ecType Enhanced content type
@param queueType A queue type (preload, on-demand)
@param version Content version
@param size Item size
@param status Donwload status
*/
NB_DEC void NB_QaLogEnhancedContentDownloadResult(NB_Context* context, const char* ecId,
            const unsigned char ecType,
            const unsigned char queueType,
            const char* version,
            const int size,
            const unsigned char status);

/*! Logging API calls

@param context A pointer to the shared NB_Context instance
@param type Record type, e.g. entering a function
@param function Function name
@param message Additional text line logged together with API call
*/
NB_DEC void NB_QaLogApiCall(NB_Context* context,
            NB_QaLogApiCallRecordType type,
            const char* function,
            const char* message);

/*! Logging location events in the location service

@param context A pointer to the shared NB_Context instance
@param eventID Location event Id, e.g. location request or reply
*/
NB_DEC void NB_QaLogLocationEvent (NB_Context* context,
            NB_QaLogLocationEventType eventID);

/*! Log location request by cell ID

@return 'NE_OK' for success. NB_Error code for failure.
*/
NB_DEC NB_Error NB_QaLogLocationRequestByCellID (NB_Context* context,
         int sid,
         int nid,
         int bid,
         int rssi);

/*! Log location request by WiFi

@return 'NE_OK' for success. NB_Error code for failure.
*/
NB_DEC NB_Error NB_QaLogLocationRequestByWiFi (NB_Context* context,
         unsigned char numNetworks,
         const unsigned char* macAddress,
         int rssi,
         int timestamp);

/*! Log replies on the location request, from the location server

@return 'NE_OK' for success. NB_Error code for failure.
*/
NB_DEC NB_Error
NB_QaLogLocationReply (NB_Context* context,
         double latitude,
         double longitude,
         unsigned short errorRadius,
         NB_QaLogLocationSourceType source);


/*! Log tile request to the server.

    @return None
*/
NB_DEC void
NB_QaLogTileRequest(NB_Context* context,      /*!< A pointer to the shared
                                                   NB_Context instance         */
                    uint32 xPosition,         /*!< x-position of returned tile */
                    uint32 yPosition,         /*!< y-position of returned tile */
                    uint32 zoomLevel          /*!< zoom level of returned tile */
                    );

/*! Log reply for tile request.

    @return None
*/
NB_DEC void
NB_QaLogTileReceived(NB_Context* context,      /*!< A pointer to the shared
                                                    NB_Context instance         */
                     uint32 type,              /*!< Map type of reply           */
                     uint32 xPosition,         /*!< x-position of returned tile */
                     uint32 yPosition,         /*!< y-position of returned tile */
                     uint32 zoomLevel          /*!< zoom level of returned tile */
                     );


/*! Log event: succeeded in downloading a tile.

    @return 'NE_OK' for success. NB_Error code for failure.
*/
NB_DEC NB_Error
NB_QaLogTileDownloadSuccess(NB_Context* context,    /*!< Pointer to NB_Context instance. */
                            uint32 x,               /*!< x coordinate of tile. */
                            uint32 y,               /*!< y coordinate of tile. */
                            uint32 zoomLevel,       /*!< zoom Level of tile */
                            const char* layerId,    /*!< Identifier of layer */
                            uint32 tileSize,        /*!< Number of tile Size  */
                            double downloadTime     /*!< Time spent to download this tile. */
                            );

/*! Log event: failed to download a tile.

    @return 'NE_OK' for success. NB_Error code for failure.
*/
NB_DEC NB_Error
NB_QaLogTileDownloadFailure(NB_Context* context,    /*!< Pointer to NB_Context instance. */
                            uint32 x,               /*!< x coordinate of tile. */
                            uint32 y,               /*!< y coordinate of tile. */
                            uint32 zoomLevel,       /*!< zoom Level of tile */
                            const char* layerId,    /*!< Identifier of layer */
                            uint32 errorCode        /*!< Error code representing reason of failure*/
                            );

/*! Log event: failed to initiate a UnifiedMapConfiguration object.

    @return 'NE_OK' for success. NB_Error code for failure.
*/
NB_DEC NB_Error
NB_QaLogUnifiedMapConfigInitializationFailure(NB_Context* context,       /*!< Pointer to NB_Context instance. */
                                              unsigned char failureType, /*!< Type of object that failed to initialize. */
                                              uint32 retriedTime,        /*!< Retry times before failure. */
                                              uint32 exceptionNumber     /*!< Identifier of exception occurred.*/
                                              );

/*! Log event: begin to prefetch window.

  @return 'NE_OK' for success. NB_Error code for failure.
*/
NB_DEC NB_Error
NB_QaLogPrefetchWindow(NB_Context* context,     /*!< Pointer to NB_Context instance. */
                       double startLatitude,    /*!< latitude of the position start prefetching. */
                       double startLongitude,   /*!< longitude of the position start prefetching. */
                       double endLatitude,      /*!< latitude of the end position of this prefetch. */
                       double endLongitude,     /*!< longitude of the end position of this prefetch. */
                       double startDistance,    /*!< remain distance at the start point. */
                       double endDistance       /*!< remain distance at the end point. */
                       );

struct NB_QaLogSpeedLimitInformation;
struct NB_QaLogSpeedLimitStateData;
typedef struct NB_QaLogSpeedLimitInformation NB_QaLogSpeedLimitInformation;
typedef struct NB_QaLogSpeedLimitStateData NB_QaLogSpeedLimitStateData;
/*! Logging speed setting*/
NB_DEC  void
NB_QaLogSpeedLimitSetting(NB_Context* context,
                          NB_QaLogSpeedLimitInformation* info
                          );

/*! Logging performance measures

    @return None.
*/
NB_DEC void NB_QaLogPerformance(const NB_Context* context,  /*!< Pointer to context instance */
                                const char* msg,      /*!< Operation description */
                                uint32 timeMs         /*!< Operation time in ms */
                                );

NB_DEC void
NB_QaLogLayerAvailabilityMatrixCheckResult(NB_Context* context,/*!< NB_Context instance pointer */
                                           int32 x,            /*!< x coordinate of tile */
                                           int32 y,            /*!< y coordinate of tile */
                                           const char* layerId,/*!< Identifier of layer */
                                           nb_boolean hasData  /*!< If this layer has data or not */
                                           );

/*! Logging tile coordinate of DAM checked

*/
NB_DEC void
NB_QaLogDAMCheckResult(NB_Context* context,/*!< NB_Context instance pointer */
                       int32 x,            /*!< x coordinate of tile */
                       int32 y,            /*!< y coordinate of tile */
                       uint8 bit,          /*!< bitset that DAM checked */
                       nb_boolean hasData  /*!< If this layer has data or not */
                       );

/*! Opening the KPI session

@param context A pointer to the shared NB_Context instance
@param bundleId ID of the bundle that owns the session
@param applicationId ID of the application/library that opens the session
@param deviceId ID of the device on which the application/library is run
@return NB_Error code
*/
NB_DEC NB_Error
NB_QaLogKpiSessionOpen(NB_Context* context,
                        const char* bundleId,
                        const char* applicationId,
                        const char* deviceId);


/*! Closing the KPI session

@param bundleId ID of the bundle that owns the session
@param applicationId ID of the application/library that opens the session
@param deviceId ID of the device on which the application/library is run
@return NB_Error code
*/
NB_DEC NB_Error
NB_QaLogKpiSessionClose(NB_Context* context,
                        const char* bundleId,
                        const char* applicationId,
                        const char* deviceId);


/*! Logging KPI freeform event

@param context A pointer to the shared NB_Context instance
@param eventLabel String that identifies particular kpi event
@param data Data string associated with eventLabel, in form of "arg1=value1&arg2=value2&..."
@return NB_Error code
*/
NB_DEC NB_Error NB_QaLogKpiFreeformEvent(NB_Context* context,
        const char* eventLabel,
        const char* data);

/*! Setup qalog filter

@param context A pointer to the shared NB_Context instance
@param id ID of QaRecord
@param enable Enable or disable the label
@return NB_Error
*/
NB_DEC NB_Error NB_QaLogFilterSet(NB_Context* context,
        uint16 id,
        nb_boolean enable);


/*! Setup qalog filter (for all logs)

@param context A pointer to the shared NB_Context instance
@param enable Enable or disable all logs
@return NB_Error
*/
NB_DEC NB_Error NB_QaLogFilterSetAll(NB_Context* context,
        nb_boolean enable);


/*! Setup KPI freeform event filter

@param context A pointer to the shared NB_Context instance
@param eventLabel String that identifies particular kpi event
@param enable Enable or disable the label
@return NB_Error
*/
NB_DEC NB_Error NB_QaLogKpiFreeformEventFilterSet(NB_Context* context,
        const char* eventLabel,
        nb_boolean enable);


/*! Setup KPI freeform event filter (for all labels)

@param context A pointer to the shared NB_Context instance
@param enable TRUE if enabled or FALSE if disabled
*/
NB_DEC void NB_QaLogKpiFreeformEventFilterSetAll(NB_Context* context,
        nb_boolean enable);


/*! Puts a KPI freeform event to QaLog and returns an unique interval ID

@param context A pointer to the shared NB_Context instance
@param eventLabel String that identifies particular kpi event
@return An unique interval ID
*/
NB_DEC uint32 NB_QaLogKpiIntervalBegin(NB_Context* context,
        const char* eventLabel);


/*! Setup KPI freeform event filter (for all labels)

@param context A pointer to the shared NB_Context instance
@param eventLabel String that identifies particular kpi event
@param intervalID Interval ID returned by NB_QaLogKpiIntervalBegin()
*/
NB_DEC void NB_QaLogKpiIntervalEnd(NB_Context* context,
        uint32 intervalID,
        const char* eventLabel);


/*! Logging speed state*/
NB_DEC void
NB_QaLogSpeedLimitState(NB_Context* context,
                        NB_QaLogSpeedLimitStateData* state
                        );



struct NB_QaLogRoadInfoData;
struct NB_QaLogLaneInfoData;
struct NB_QaLogLaneItmeInfoData;
struct NB_QaLogGuidanceInfoData;

typedef struct NB_QaLogRoadInfoData NB_QaLogRoadInfoData;
typedef struct NB_QaLogLaneInfoData NB_QaLogLaneInfoData;
typedef struct NB_QaLogLaneItmeInfoData NB_QaLogLaneItmeInfoData;
typedef struct NB_QaLogGuidanceInfoData NB_QaLogGuidanceInfoData;

/*! Logging road info*/
NB_DEC void
NB_QaLogRoadInfo(NB_Context* context,
                 int32 maneuverIndex,
                 uint8 roadType,
                 NB_QaLogRoadInfoData* roadInfo
                 );
/*! Logging lane info*/
NB_DEC void
NB_QaLogLaneInfo(NB_Context* context,
                 int32 maneuverIndex,
                 NB_QaLogLaneInfoData * laneInfo
                 );
/*! Logging lane item info*/
NB_DEC void
NB_QaLogLaneItemInfo(NB_Context* context,
                     int32 maneuverIndex,
                     NB_QaLogLaneItmeInfoData * laneGuidance
                     );
/*! Logging natural guidance point*/
NB_DEC void
NB_QaLogGuidancePoint(NB_Context* context,
                      int32 maneuverIndex,
                      NB_QaLogGuidanceInfoData * guidance
                      );
/*! Opening the KPI session

@param context A pointer to the shared NB_Context instance
@param bundleId ID of the bundle that owns the session
@param applicationId ID of the application/library that opens the session
@param deviceId ID of the device on which the application/library is run
@return NB_Error code
*/
NB_DEC NB_Error
NB_QaLogKpiSessionOpen(NB_Context* context,
                        const char* bundleId,
                        const char* applicationId,
                        const char* deviceId);


/*! Closing the KPI session

@param bundleId ID of the bundle that owns the session
@param applicationId ID of the application/library that opens the session
@param deviceId ID of the device on which the application/library is run
@return NB_Error code
*/
NB_DEC NB_Error
NB_QaLogKpiSessionClose(NB_Context* context,
                        const char* bundleId,
                        const char* applicationId,
                        const char* deviceId);


/*! Logging KPI freeform event

@param context A pointer to the shared NB_Context instance
@param eventLabel String that identifies particular kpi event
@param data Data string associated with eventLabel, in form of "arg1=value1&arg2=value2&..."
@return NB_Error code
*/
NB_DEC NB_Error NB_QaLogKpiFreeformEvent(NB_Context* context,
        const char* eventLabel,
        const char* data);

/*! Setup qalog filter

@param context A pointer to the shared NB_Context instance
@param id ID of QaRecord
@param enable Enable or disable the label
@return NB_Error
*/
NB_DEC NB_Error NB_QaLogFilterSet(NB_Context* context,
        uint16 id,
        nb_boolean enable);


/*! Setup qalog filter (for all logs)

@param context A pointer to the shared NB_Context instance
@param enable Enable or disable all logs
@return NB_Error
*/
NB_DEC NB_Error NB_QaLogFilterSetAll(NB_Context* context,
        nb_boolean enable);


/*! Setup KPI freeform event filter

@param context A pointer to the shared NB_Context instance
@param eventLabel String that identifies particular kpi event
@param enable Enable or disable the label
@return NB_Error
*/
NB_DEC NB_Error NB_QaLogKpiFreeformEventFilterSet(NB_Context* context,
        const char* eventLabel,
        nb_boolean enable);


/*! Setup KPI freeform event filter (for all labels)

@param context A pointer to the shared NB_Context instance
@param enable TRUE if enabled or FALSE if disabled
*/
NB_DEC void NB_QaLogKpiFreeformEventFilterSetAll(NB_Context* context,
        nb_boolean enable);


/*! Puts a KPI freeform event to QaLog and returns an unique interval ID

@param context A pointer to the shared NB_Context instance
@param eventLabel String that identifies particular kpi event
@return An unique interval ID
*/
NB_DEC uint32 NB_QaLogKpiIntervalBegin(NB_Context* context,
        const char* eventLabel);


/*! Setup KPI freeform event filter (for all labels)

@param context A pointer to the shared NB_Context instance
@param eventLabel String that identifies particular kpi event
@param intervalID Interval ID returned by NB_QaLogKpiIntervalBegin()
*/
NB_DEC void NB_QaLogKpiIntervalEnd(NB_Context* context,
        uint32 intervalID,
        const char* eventLabel);

/*! Write to the QA log file

@param context A pointer to the shared NB_Context instance
@param data The QA Log data to write
@param dataSize The data size
@return NB_Error
*/
NB_DEC NB_Error NB_QaLogWriteBuffer(NB_Context* context,
                                    const uint8* data,
                                    uint32 dataSize);

/*! @} */

#endif
