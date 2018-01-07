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

    @file     nbplace.h
*/
/*
    (C) Copyright 2005 - 2009 by Networks In Motion, Inc.

    The information contained herein is confidential, proprietary
    to Networks In Motion, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of Networks In Motion is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/

#ifndef NBPLACE_H
#define NBPLACE_H

#include "nberror.h"
#include "paltypes.h"
#include "csltypes.h"
#include "nblocation.h"

/*! @{ */
#define NB_TRANSIT_TIME_LEN             20

#define NB_PHONE_COUNTRY_LEN           6
#define NB_PHONE_AREA_LEN              6
#define NB_PHONE_NUMBER_LEN            12
#define NB_PHONE_FORMATTED_PHONE_LEN   64

#define NB_CATEGORY_CODE_LEN           10
#define NB_CATEGORY_NAME_LEN           50

#define NB_PLACE_NAME_LEN              150

#define NB_PLACE_NUM_CAT               4

#define NB_PLACE_NUM_PHONE             4

#define NB_CURRENCY_LEN                3
#define NB_UNITS_LEN                   8

#define NB_FUEL_PRODUCTS_NUM           8
#define NB_FUEL_CODE_LEN               1
#define NB_FUEL_NAME_LEN               24

#define NB_WEATHER_FORECASTS_NUM       8
#define NB_WEATHER_CONDITION_LEN       50
#define NB_WEATHER_SKY_LEN             50
#define NB_WEATHER_UVDESCRIPTION_LEN   20
#define NB_WEATHER_SUNRISE_LEN         10
#define NB_WEATHER_SUNSET_LEN          10
#define NB_WEATHER_MOONRISE_LEN        10
#define NB_WEATHER_MOONSET_LEN         10

#define NB_EVENT_NAME_LEN              100
#define NB_EVENT_MPAA_LEN              10

#define NB_TRAFFIC_INCIDENT_MAX_LEN    400


/*! Opaque forward declaration. */
typedef struct NB_SearchFormattedTextContentHandle NB_SearchFormattedTextContentHandle;


/* brief Category (?) */
typedef struct
{
    char    code[NB_CATEGORY_CODE_LEN+1];
    char    name[NB_CATEGORY_NAME_LEN+1];
} NB_Category;


/*! Type of phone contact information
 @see NB_Phone */
typedef enum
{
    NB_Phone_Primary = 0,
    NB_Phone_Secondary,
    NB_Phone_National,
    NB_Phone_Mobile,
    NB_Phone_Fax
} NB_PhoneType;


/*! This type describes a phone contact */
typedef struct
{
    NB_PhoneType    type;                               /*!< Kind of phone contact */
    char            country[NB_PHONE_COUNTRY_LEN+1];    /*!< Country code */
    char            area[NB_PHONE_AREA_LEN+1];          /*!< Area code */
    char            number[NB_PHONE_NUMBER_LEN+1];      /*!< Actual phone number */
    char            formattedText[NB_PHONE_FORMATTED_PHONE_LEN + 1];    /*!< Formatted text of phone to display */
} NB_Phone;


/*! A type describing a mapped place or or point of interest */
typedef struct
{
    char            name[NB_PLACE_NAME_LEN+1];      /*!< Name of the place */
    NB_Location     location;                       /*!< Location of the place */
    NB_Phone        phone[NB_PLACE_NUM_PHONE];      /*!< All the phones of this place */
    int             numphone;                       /*!< Actual number of phone contacts for this place. */
    NB_Category     category[NB_PLACE_NUM_CAT];     /*!< Category of this point of interest */
    int             numcategory;                    /*!< Actual number of categories for this place. */
    uint64          placeID;                        /*!<Place ID. */

    // ! Important: Don't add any additional data members here. This structure is fixed
    // and should not be changed. Add any additional data to NB_ExtendedPlace.
} NB_Place;


/*! Type indicating content of NB_ExtendedPlace. */
typedef enum
{
    NB_EPI_None = 0,
    NB_EPI_PremiumPlacement = (1 << 0),             /*!< Indicates result is Premium Placement Ad */
    NB_EPI_EnhancedPOI = (1 << 1),                  /*!< Indicates result is Enhanced POI */
    NB_EPI_POIContent = (1 << 2),                   /*!< Indicates result contains additional POI information */
    NB_EPI_Accuracy = (1 << 3)                      /*!< Indicates result contains match accuracy */
} NB_ExtendedPlaceInformation;

/*! Overall rating of the POI */
typedef struct
{
    double  averageRating;
    uint32  ratingCount;
} NB_OverallRating;

/*! Contains information to be returned to the server for POI that can be monetized */
typedef struct
{
    char*   providerId;
    byte*   state;
    int     stateSize;
} NB_GoldenCookie;

/*! Contains information for search event tracking */
typedef struct
{
    char*   providerId;
    byte*   state;
    int     stateSize;
} NB_SearchEventCookie;

/*! Contains information for place event tracking */
typedef struct
{
    char*   providerId;
    byte*   state;
    int     stateSize;
} NB_PlaceEventCookie;

/*! Contains POI information if NB_EXT_WantFormattedText IS NOT specified in
    the NB_POIExtendedConfiguration parameter of NB_SearchParametersCreateXXXX
    function calls */
typedef struct
{
    char*   key;
    char*   value;
} NB_POIContent;

/*! Vendor content.
 */
typedef struct
{
    NB_POIContent*                          content;
    int                                     contentCount;
    NB_OverallRating                        overallRating;
    char*                                   name;
} NB_VendorContent;

/*! Extended place information.

    This structure is dynamically allocated. The strings are variable lenth strings
    to allow flexibility. This structure is created by CreateNIMExtendedPlace()
    and freed by FreeNIMExtendedPlace(). If new members are added make sure that
    they are created/filled and freed correctly in those functions.

    @see CreateNIMExtendedPlace
    @see FreeNIMExtendedPlace
*/
typedef struct
{
    NB_ExtendedPlaceInformation             extendedInformation;    /*!< Flags that indicates what extended information is available */
    char*                                   poiContentId;           /*!< Unique identifier associated with this POI */
    char*                                   tagline;                /*!< A merchant provided memorable slogan or teaser to gain the consumer’s attention. */
    NB_SearchFormattedTextContentHandle*    formattedContentHandle; /*!< Pass this handle to NB_ExtendedPlaceGetFormattedTextContent to get the POI details */
    NB_POIContent*                          poiContent;             /*!< Array of structure. Keyed values intended only for display are included only if the client did not request want-formatted */
    int                                     poiContentCount;        /*!< Number of items in poiContent array of structure pointer */
    NB_OverallRating                        overallRating;
    NB_GoldenCookie                         goldenCookie;
    byte*                                   imageData;              /*!< Optional image associated with content */
    nb_size                                 imageDataSize;          /*!< Size of the optional image data */
    uint32                                  accuracy;               /*!< Accuracy of the returned match (range 0-100) */
    NB_PlaceEventCookie                     placeEventCookie;       /*!< Information for place event tracking */
    NB_VendorContent*                       vendorContent;          /*!< Array of structure for Vendor Content */
    int                                     vendorContentCount;     /*!< Number of items in vendorContent array of structure pointer */
} NB_ExtendedPlace;


/*! A type describing price information. */
typedef struct
{
    double  value;
    char    currency[NB_CURRENCY_LEN+1];
	uint32  modtime;
} NB_Price;

/*! A type describing information for different kinds of fuels */
typedef struct
{
    char    product_name[NB_FUEL_NAME_LEN+1];  /*!< Name of the actual fuel product as sold by providers */
    char    code[NB_FUEL_CODE_LEN+1];          /*!< Internal code for this fuel type */
    char    type_name[NB_FUEL_NAME_LEN+1];     /*!< The type of the fuel regardless of the provider */
} NB_FuelType;


/*! A type describing a fuel product as found in various locations */
typedef struct
{
    NB_Price        price;
    char            units[NB_UNITS_LEN+1];
    NB_FuelType     fuel_type;
} NB_FuelProduct;


/*! Summary information for a given fuel product */
typedef struct
{
    NB_FuelProduct    average;
    NB_FuelProduct    low;
} NB_FuelSummary;


/*! A type describing fuel products */
typedef struct
{
    NB_FuelProduct      fuel_product[NB_FUEL_PRODUCTS_NUM];
    int                 num_fuelproducts;                        /*!< Actual number of elements in the fuel_product array */
} NB_FuelDetails;


/*! Different kinds of severity for a traffic incident */
typedef enum
{
    NB_TrafficIncident_Severe = 0,
    NB_TrafficIncident_Major,
    NB_TrafficIncident_Minor,
    NB_TrafficIncident_LowImpact
} NB_TrafficIncidentSeverity;


/*! Traffic incident information */
typedef struct
{
    uint32                          type;                                       /*!< Internal code categorizing the type of the incident */
    NB_TrafficIncidentSeverity      severity;
    uint32                          entry_time;                                 /* (?) */
    uint32                          start_time;                                 /*!< When the incident occurred */
    uint32                          end_time;                                   /*!< When the incident was cleared off the street */
    int32                           utc_offset;                                 /*!< Offset the time data with this amount. */
    char                            road[NB_LOCATION_MAX_STREET_LEN+1];             /*!< A string describing the road where this incident happened. */
    char                            description[NB_TRAFFIC_INCIDENT_MAX_LEN+1];/*!< A string description of this accident */
    uint32                          original_item_id;                           /* (?) */
} NB_TrafficIncident;


/*! Current weather information */
typedef struct
{
    double  temp;                                       /*!< Temperature */
    double  dewpt;
    uint32  rel_humidity;                               /*!< Humidity value in (?) */
    double  wind_speed;                                 /*!< Wind speed in (?) */
    uint32  wind_dir;                                   /*!< Wind direction */
    double  wind_gust;
    double  pressure;                                   /*!< Pressure value in (?) */
    uint32  condition_code;
    char    condition[NB_WEATHER_CONDITION_LEN+1];
    char    sky[NB_WEATHER_SKY_LEN+1];                 /*!< Description of the sky conditions */
    double  ceil;
    uint32  visibility;                                 /*!< Road visibility information */
    double  heat_index;
    double  wind_chill;
    double  snow_depth;
    double  max_temp_6hr;                               /*!< Maximum temperature in the next 6 hours */
    double  max_temp_24hr;                              /*!< Maximum temperature in the next 24 hours */
    double  min_temp_6hr;                               /*!< Minimum temperature in the next 6 hours */
    double  min_temp_24hr;                              /*!< Minimum temperature in the next 24 hours */
    double  precipitation_3hr;                          /*!< Precipitation amount in a 3 hour period */
    double  precipitation_6hr;                          /*!< Precipitation amount in a 6 hour period */
    double  precipitation_24hr;                         /*!< Precipitation amount in a 24 hour period */
    uint32  update_time;                                /*!< Time this information was gathered */
    int32   utc_offset;                                 /*!< Offset time values by this amount  */
} NB_WeatherConditions;


/*! Weather forecast information */
typedef struct
{
    uint32  date;                                       /*!< (?) */
    double  high_temp;                                  /*!< Forecasted high temperature */
    double  low_temp;                                   /*!< Forecasted low temperature */
    uint32  wind_dir;                                   /*!< Forecasted wind direction */
    double  wind_speed;                                 /*!< Forecasted wind speed */
    uint32  precipitation_probability;                  /*!< Forecasted precipitation probability */
    uint32  rel_humidity;                               /*!< Forecasted humidity */
    uint32  condition_code;
    char    condition[NB_WEATHER_CONDITION_LEN+1];
    uint32  uv_index;
    char    uv_desc[NB_WEATHER_UVDESCRIPTION_LEN+1];
    char    sunrise[NB_WEATHER_SUNRISE_LEN+1];         /*!< Forecasted sunrise */
    char    sunset[NB_WEATHER_SUNSET_LEN+1];
    char    moonrise[NB_WEATHER_MOONRISE_LEN+1];
    char    moonset[NB_WEATHER_MOONSET_LEN+1];
    uint32  moon_phase;
    uint32  day_number;                                 /*!< (?) */
    uint32  update_time;                                /*!< Time this forecast was produced. */
    int32   utc_offset;                                 /*!< Offset time values by this amount  */
} NB_WeatherForecast;


/*! A collection of weather forecasts */
typedef struct
{
    NB_WeatherForecast      weather_forecasts[NB_WEATHER_FORECASTS_NUM];
    int                     num_weather_forecasts; /*!< Actual number of forecasts elements in the `weather_forecasts' array */
} NB_WeatherForecasts;


/*! Summary results for a Point of Interest search */
typedef struct
{
    NB_Category     category; /*!< Category of results */
    int             count;
    uint32          time;
} NB_SummaryResult;


/*! Ambiguous results for a point of interest search */
typedef struct
{
    const char*     caption;    /*!< ??? */
} NB_AmbiguousResult;


/*! Different kind of font styles for displaying formatted text. */
typedef enum
{
    NB_Font_Normal = 0,
    NB_Font_Bold,
    NB_Font_Large,
    NB_Font_Large_Bold,
    NB_Font_Custom1,
    NB_Font_Custom2,
    NB_Font_Custom3,
    NB_Font_Custom4
} NB_Font;


/*! A type describing a performance happening at a given time for a given price. */
typedef struct
{
    uint32     start_time;
    uint32     end_time;
    nb_boolean bargain_price; /*!< Use this nb_boolean to signal the performance as a bargain */
} NB_EventPerformance;


/*! Event information */
typedef struct
{
    char                            name[NB_EVENT_NAME_LEN + 1];   /*!< Name of the event */
    char                            mpaa[NB_EVENT_MPAA_LEN + 1];   /*!< MPAA rating of the event */
    uint32                          star;                           /*!< Number of stars for this event */

    int                             num_performance;                /*!< How many performances are in this event (length of performances array) */
    NB_EventPerformance*            performances;                   /*!< Dynamic array of performances included in this event */

    NB_SearchFormattedTextContentHandle*    contentHandle;          /*!< Pass this handle to NB_SearchInformationGetFormattedContentText() to retrieve formatted
                                                                     event details. */
} NB_Event;

/*! Public transit information */
typedef struct
{
    char            operatingDays[NB_TRANSIT_TIME_LEN+1];               /*!< The name of operating days,such as weekdays or weekends etc */
    char            startTime[NB_TRANSIT_TIME_LEN+1];                   /*!< The start time of the service */
    char            endTime[NB_TRANSIT_TIME_LEN+1];                     /*!< The close time of the service*/
    char            headwaySeconds[NB_TRANSIT_TIME_LEN+1];              /*!< Headway in seconds */
} NB_Hours;

typedef struct
{
    uint32                              id;                             /*!< Unique route agency id */
    char                                shortName[NB_PLACE_NAME_LEN+1]; /*!< Transit route short name */
    NB_Category                         type;                           /*!< Transit route type */
    nb_color                            color;                          /*!< Transit route color */
    nb_color                            textColor;                      /*!< Transit route text color */
} NB_TransitRoute;

typedef enum
{
    NBPTST_None =      0,
    NBPTST_Tram =      0x01,
    NBPTST_Subway =    0x02,
    NBPTST_Rail =      0x04,
    NBPTST_Bus =       0x08,
    NBPTST_Ferry =     0x10,
    NBPTST_CableCar =  0x20
} NB_PublicTransitStopType;

typedef struct
{
    uint32                              stopId;                              /*!< Unique stop agency id */
    uint32                              sequence;                            /*!< Stop sequence*/
    char                                agencyName[NB_PLACE_NAME_LEN+1];     /*!< Name of the transit agency */
    uint32                              routeId;                             /*!< Unique route agency id */
    char                                routeShortName[NB_PLACE_NAME_LEN+1]; /*!< Transit route short name */
    char                                routeLongName[NB_PLACE_NAME_LEN+1];  /*!< Transit route long name */
    NB_Category                         routeType;                           /*!< Transit route type */
    nb_color                            routeColor;                          /*!< Transit route color */
    nb_color                            routeTextColor;                      /*!< Transit route text color */
    NB_Hours*                           hoursOfOperation;                    /*!< Array of hours */
    int                                 hoursOfOperationCount;               /*!< Length of the NB_Hours array */
    uint32                              stopType;                            /*!< Combined routeType's for a stop */
} NB_PublicTransitStopInfo;

typedef struct
{
    NB_PublicTransitStopInfo*           transitStops;                    /*!< Array of NB_PublicTransitStop*/
    int                                 transitStopCount;                /*!< Length of the NB_PublicTransitStop array.*/
} NB_PublicTransitPlaceInfo;

/*! Day of week */
typedef enum
{
    NB_DOW_Sunday = 0,
    NB_DOW_Monday,
    NB_DOW_Tuesday,
    NB_DOW_Wednesday,
    NB_DOW_Thursday,
    NB_DOW_Friday,
    NB_DOW_Saturday
} NB_DayOfWeek;

/*! Time range */
typedef struct
{
    NB_DayOfWeek dayOfWeek;         /*!< Day of week */
    uint32 startTime;               /*!< Start time, number of seconds since midnight (local time of POI) */
    uint32 endTime;                 /*!< End time */
} NB_TimeRange;

/*! Hours of operation */
typedef struct
{
    nb_boolean hasUtcOffset;        /*!< Is the UTC offset present? */
    int32 utcOffset;                /*!< Offset in seconds from UTC of POI local time, can be negative. */
    uint32 timeRangeCount;          /*!< Count of time range */
    NB_TimeRange* timeRangeArray;   /*!* Time range array */
} NB_HoursOfOperation;

/*! @} */

#endif
