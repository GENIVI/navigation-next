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

    @file     nbsearchtypes.h
*/
/*
    (C) Copyright 2011 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef NBSEARCHTYPES_H
#define NBSEARCHTYPES_H

/*!
    @addtogroup nbsearchtypes
    @{
*/

#include "navpublictypes.h"
#include "nbrouteinformation.h"

// Constants ....................................................................................


// Types ........................................................................................

/*! This enumeration specifies the kind of information obtained from a search operation.

    The search result type is returned by NB_SearchInformationGetResultInformation(). It specifies what
    information is returned by the current places search. Multiple types can be returned
    for one search.
*/
typedef enum
{
    NB_SRT_None            = 0,

    // Content per search result
    NB_SRT_SubSearch       = 0x0001,                /*!< A Sub Search can be performed on this result. Call NB_SearchParametersCreateSubSearch() to set it up. */
    NB_SRT_Place           = 0x0002,                /*!< Place information is available. Call NB_SearchInformationGetPlace() to retrieve it. */
    NB_SRT_Events          = 0x0004,                /*!< Events information is available. Call NB_SearchInformationGetEvents() to retrieve it. */
    NB_SRT_EventSummary    = 0x0008,                /*!< Event summary information is available. Call NB_SearchInformationGetEventSummary() to retrieve it. */
    NB_SRT_FuelDetails     = 0x0010,                /*!< Fuel detail information is available. Call NB_SearchInformationGetFuelDetails() to retrieve it. */
    NB_SRT_TrafficIncident = 0x0020,                /*!< Traffic incident information is available. Call NB_SearchInformationGetTrafficIncident() to retrieve it. */
    NB_SRT_SpeedCamera     = 0x0040,                /*!< Speed camera information is available.  Call NB_SearchInformationGetSpeedCamera to retrieve it */

    // Content per downloaded slice
    NB_SRT_FuelSummary     = 0x0100,                /*!< Fuel summary information is available. Call NB_SearchInformationGetFuelSummary() to retrieve it. */

    // Content per search (one per search only)
    NB_SRT_Weather         = 0x1000                 /*!< Weather information is available. Call NB_SearchInformationGetWeather() to retrieve it. */
} NB_SearchResultType;

/*! Combination of flags that can be specified during a call to any of the NB_SearchParametersCreateXXXX functions. */
typedef enum
{
    NB_EXT_None = 0,                                /*!< No POI extended configuration is requested */
    NB_EXT_WantPremiumPlacement = (1 << 0),         /*!< Request premium placement Ad in the response */
    NB_EXT_WantEnhancedPOI = (1 << 1),              /*!< Request enhanced POI information in the response */
    NB_EXT_WantFormattedText = (1 << 2),            /*!< Request POI response as formatted text */
    NB_EXT_WantAccuracy = (1 << 3),                 /*!< Request accuracy of returned match */
    NB_EXT_WantSpellingSuggestions = (1 << 4),      /*!< Request spelling suggestions if available */
    NB_EXT_WantNonProximityPois = (1 << 5),         /*!< Request non-proximity pois if available */
    NB_EXT_WantSuggestDistance = (1 << 6),          /*!< Request suggest distance */
    NB_EXT_WantSuggestThreeLines = (1 << 7),        /*!< Request suggest line3 */
    NB_EXT_WantDistanceToUser = (1 << 8),           /*!< Request distance to user, not to search center */
    NB_EXT_WantCountryName = (1 << 9),              /*!< Request country name */
    NB_EXT_WantExtendedAddress = (1 << 10),         /*!< Request full formatted address (max 5 lines) */
    NB_EXT_WantCompactAddress = (1 << 11),          /*!< Request compact formatted address (max 2 lines) */
    NB_EXT_WantFormattedPhone = (1 << 12),          /*!< Request formatted phone */
    NB_EXT_WantStructuredHoursOfOperation = (1 << 13),      /*!< Request structured hours of operation */
    NB_EXT_WantParentCategory = (1 << 14),          /*!< Request parent category  */
    NB_EXT_WantContentTypes = (1 << 15),            /*!< Request Content Types in the query */
    NB_EXT_WantIconId = (1 << 16),                  /*!< Request Icon ID */
    NB_EXT_WantSearchResultId = (1 << 17),          /*!< Request search result id in search result */
    NB_EXT_WantResultDescription = (1 << 18),       /*!< Request result description if available */
    NB_EXT_WantRelatedSearch = (1 << 19),           /*!< Request related search with related search filter */
    NB_EXT_WantPoiCookies = (1 << 20)               /*!< Request place event cookies and poi attribute cookie. */
} NB_POIExtendedConfiguration;

/*! Types used for search region.

    Specifies which fields of the NB_SearchRegion are valid.

    @see NB_SearchRegion
*/
typedef enum
{
    NB_ST_None = 0,

    NB_ST_Center,                                   /*!< Use the center point for the search */
    NB_ST_RouteAndCenter,                           /*!< Use the route and the center point for the search */
    NB_ST_DirectionAndCenter,                       /*!< Use the direction and the center point for the search */
    NB_ST_BoundingBox                               /*!< Use the bounding box for the search */
} NB_SearchRegionType;

/*! Direction used for search region.

    @see NB_SearchRegion
*/
typedef struct
{
    double  heading;                                /*!< The heading to use (in degrees) */
    double  speed;                                  /*!< The speed of travel (in meters per second) */
} NB_Direction;

/*! Bounding box used for search region.

    @see NB_SearchRegion
*/
typedef struct
{
    NB_LatitudeLongitude topLeft;           /*!< Top-left corner of the bounding box */
    NB_LatitudeLongitude bottomRight;       /*!< Bottom-right corner of the bounding box */
} NB_BoundingBox;

/*! Search region.

    @see NB_SearchRegionType
*/
typedef struct
{
    NB_SearchRegionType     type;                   /*!< Type of search */

    NB_LatitudeLongitude    center;                 /*!< Center point for search. Valid if type is set to 'NB_SRT_Center', 'NB_SRT_RouteAndCenter' or 'NB_SRT_DirectionAndCenter' */
    NB_RouteInformation*    route;                  /*!< Route object for search. Only valid if type is set to 'NB_SRT_RouteAndCenter' */
    NB_Direction            direction;              /*!< Direction for search. Only valid if type is set to 'NB_SRT_DirectionAndCenter' */
    NB_BoundingBox          boundingBox;            /*!< Bounding box for search. Only valid if type is set to 'NB_SRT_BoundingBox' */

    uint32                  width;                  /*! Router search distance.  Only valid if type is set to 'NB_SRT_RouteAndCenter' */
    uint32                  distance;               /*! Route search width.  Only valid if type is set to 'NB_SRT_RouteAndCenter' */
} NB_SearchRegion;

/*! Search source. */
typedef enum
{
    NB_SS_None = 0,                                 /*!< No search source */
    NB_SS_User,                                     /*!< User initiated search source */
    NB_SS_Address,                                  /*!< Address search source */
    NB_SS_Carousel,                                 /*!< Carousel search source */
    NB_SS_Map,                                      /*!< Map search source */
    NB_SS_Navigation,                               /*!< Navigation (Camera, Fast POI) search source */
    NB_SS_Place,                                    /*!< Place search source */
    NB_SS_Airport,                                  /*!< Airport search source */
    NB_SS_DefaultLocation,                          /*!< Default location source */
    NB_SS_Main,                                     /*!< Main search source */
    NB_SS_Movie,                                    /*!< Movie search source */
    NB_SS_Explore,                                  /*!< explore search source */
    NB_SS_POILayer                                  /*!< poi-id search source */
} NB_SearchSource;

/*! Order to be used for search.

    Specifies how search results should be ordered.
*/
typedef enum
{
    NB_SSB_Relevance,                               /*!< Return search results sorted by relevance */
    NB_SSB_Distance                                 /*!< Return search results sorted by distance */
} NB_SearchSortBy;

/*! Method search request was entered.

    Specifies entered method of search request.
*/
typedef enum
{
    NB_SIM_Undefined,                               /*!< Search request input undefined */

    NB_SIM_Screen,                                  /*!< Search request input from screen */
    NB_SIM_Voice,                                   /*!< Search request input from recorded voice */
    NB_SIM_Suggestion,                              /*!< Search request input from previous suggestion */
    NB_SIM_Sort,                                    /*!< Search request input from sort order change */
    NB_SIM_More                                     /*!< Search request input for next or previous page */
} NB_SearchInputMethod;

/*! Search result style type */
typedef enum
{
    NB_SRST_None = 0,                               /*!< No search result style */
    NB_SRST_SingleSearch,                           /*!< Single search result style */
    NB_SRST_Suggestion,                             /*!< Suggestion search result style */
    NB_SRST_ListInterestSuggestion,                 /*!< List Interest Suggestion search result style */
    NB_SRST_AddInterestSuggestion,                  /*!< Add Interest Suggestion search result style */
    NB_SRST_Geocode,                                /*!< Geocode search result style */
    NB_SRST_Airport,                                /*!< Airport search result style */
    NB_SRST_DefaultLocation,                        /*!< Default location result style */
    NB_SRST_RelatedSearch,                          /*!< Related search result style */
    NB_SRST_MovieList,                              /*!< Movie list search result style */
    NB_SRST_MovieTheater,                            /*!< Movie theater search result style */
    NB_SRST_AddressOnly                             /*!< Contact address search result style */
} NB_SearchResultStyleType;

/*! Data availability values for requested country */
typedef struct
{
    nb_boolean   noMapData;             /*!< The country does not have map data available */
    nb_boolean   partialVisualMapData;  /*!< The country has partial coverage for visual map data */
    nb_boolean   fullVisualMapData;     /*!< The country has full coverage for visual map data */
    nb_boolean   partialRoutingData;    /*!< The country has partial coverage for routing data */
    nb_boolean   fullRoutingData;       /*!< The country has full coverage for routing data */
    nb_boolean   partialGeocodingData;  /*!< The country has partial coverage for geocoding data */
    nb_boolean   fullGeocodingData;     /*!< The country has full coverage for geocoding data */
    nb_boolean   partialPoiData;        /*!< The country has partial coverage for POI data */
    nb_boolean   fullPoiData;           /*!< The country has full coverage for POI data */
} NB_DataAvailability;

/*! Search want content type

    @todo: Add the other want content types once the protocol doc is updated
*/
typedef enum
{
    NB_SWCT_None = 0,                               /*!< No search want content type */
    NB_SWCT_TrafficIncident                         /*!< Traffic incident content type */
} NB_SearchWantContentType;


/*! @} */

#endif
