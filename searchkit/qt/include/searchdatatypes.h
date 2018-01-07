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

/*
 * (C) Copyright 2015 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
---------------------------------------------------------------------------*/

/*! @{ */

#ifndef SEARCHDATATYPES_H
#define SEARCHDATATYPES_H

#include <QString>
#include <QList>
#include "coordinate.h"
#include <QSharedPointer>
#include <QDate>

namespace locationtoolkit
{

/**
 * The file contains variety of data for SearchKit.
 */

/**
 * Type of search result.
 */
typedef enum SearchDirection
{
    SD_Start                    = 0,
    SD_Prev,
    SD_Next
}SearchDirection;

/**
 * Type of search result.
 */
typedef enum SearchResultType
{
    SRT_None                     = 0,
    SRT_POI,
    SRT_Suggest,
    SRT_SuggestList,
    SRT_ProxMatchContent
}SearchResultType;

/**
 * The base class for search result.
 */
class SearchResult
{
public:
    virtual ~SearchResult() {}
public:
    SearchResultType         searchResultType;
};

/**
 * Price of fuel.
 */
typedef struct SearchFuelPrice
{
    qint64                   modTime;
    float                    value;
    QString                  currency;
}SearchFuelPrice;

/**
 * Type of fuel.
 */
typedef struct SearchFuelType
{
    QString                  productName;
    QString                  code;
    QString                  typeName;
}SearchFuelType;

/**
 * Product of fuel.
 */
typedef struct SearchFuelProduct
{
    SearchFuelPrice          price;
    SearchFuelType           type;
    QString                  units;
}SearchFuelProduct;

/**
 * The summary of fuel contains the lowest and average of FuelProduct.
 */
typedef struct SearchFuelSummary
{
    SearchFuelProduct        average;
    SearchFuelProduct        low;
}SearchFuelSummary;

/**
 * Details of fuel.
 */
typedef struct SearchFuelDetails
{
    QList<SearchFuelProduct> fuelProducts;
}SearchFuelDetails;

/**
 * The desciption for search results.
 */
typedef struct SearchResultDescription
{
    QList<QString>           lines;
}SearchResultDescription;

/**
 * Type of POI.
 */
typedef enum SearchPOIType
{
    SPOIT_Locaton                = 0,
    SPOIT_POI
}SearchPOIType;

/**
 * Type of location.
 */
typedef enum SearchLocationType
{
    SLT_None                      = 0,
    SLT_Address,
    SLT_Intersection,
    SLT_Airport,
    SLT_LatLon,
    SLT_MyGPS,
    SLT_Route,
    SLT_RouteGPS,
    SLT_MyDirection,
    SLT_AddressFreeForm
}SearchLocationType;

/**
 * Extended address.
 */
typedef struct SearchExtendedAddress
{
    QList<QString>                addressLines;
}SearchExtendedAddress;

/**
 * Compact address.
 */
typedef struct SearchCompactAddress
{
    QString                       lineSeparator;
    QList<QString>                addressLines;
    QString                       formattedLine;
public:
    SearchCompactAddress() : lineSeparator(""), formattedLine("") { }
}SearchCompactAddress;

/**
 * The location information for one place.
 */
typedef struct SearchLocation
{
    QString                        name;
    QString                        streetNumer;
    QString                        street;
    QString                        crossStreet;
    QString                        city;
    QString                        county;
    QString                        state;
    QString                        postal;
    QString                        country;
    QString                        airport;
    QString                        countryName;
    QString                        freeForm;
    float                          UTCOffset;
    Coordinates                    coordinate;
    SearchLocationType             type;
    SearchExtendedAddress          extendedAddress;
    SearchCompactAddress           compactAddress;
}SearchLocation;

/**
 * Type of phone.
 */
typedef enum SearchPhoneType
{
    SPT_Primary                    = 0,
    SPT_Secondary,
    SPT_National,
    SPT_Mobile,
    SPT_Tax
}SearchPhoneType;

/**
 * Information of phone.
 */
typedef struct SearchPhone
{
    SearchPhoneType                type;
    QString                        country;
    QString                        area;
    QString                        number;
    QString                        formattedText;
}SearchPhone;

/**
 * Catergory of place.
 */
typedef struct SearchCategory
{
    QString                        code;
    QString                        name;
    QSharedPointer<SearchCategory> parent;
}SearchCategory;

/**
 * Information of weather conditions.
 */
typedef struct SearchWeatherConditions
{
    float                          temperature;
    float                          dewPoint;
    quint32                        relativeHumidity;
    float                          windSpeed;
    quint32                        windDirection;
    float                          windGust;
    float                          pressure;
    QString                        condition;
    quint32                        conditionCode;
    QString                        sky;
    float                          ceiling;
    quint32                        visibility;
    float                          heatIndex;
    float                          windChill;
    float                          snowDepth;
    float                          maxTemp6Hour;
    float                          maxTemp24Hour;
    float                          minTemp6Hour;
    float                          minTemp24hour;
    float                          precipitation3Hour;
    float                          precipitation6Hour;
    float                          precipitation24Hour;
    quint32                        updateTime;
    qint32                         utcOffset;
}SearchWeatherConditions;

/**
 * Summary of weather conditions.
 */
typedef struct SearchWeatherConditionsSummary
{
    float                          temperature;
    QString                        condition;
    quint32                        conditionCode;
    float                          maxTemperatureInLast24hours;
    float                          minTemperatureInLast24hours;
    qint32                         utcOffset;
}SearchWeatherConditionsSummary;

/**
 * Information of weather forcast.
 */
typedef struct SearchWeatherForcast
{
    quint32                        date;
    float                          highTemperature;
    float                          lowTemperature;
    float                          windSpeed;
    quint32                        windDirection;
    quint32                        precipitationProbability;
    quint32                        relativeHumidity;
    QString                        condition;
    quint32                        conditionCode;
    quint32                        uvIndex;
    QString                        uvDescription;
    QString                        sunriseTime;
    QString                        sunsetTime;
    QString                        moonriseTime;
    QString                        moonsetTime;
    quint32                        moonPhase;
    quint32                        updateTime;
    qint32                         utcOffset;
}SearchWeatherForcast;

/**
 * The place information.
 */
typedef struct SearchPlace
{
    QString                        name;
    SearchLocation                 location;
    QList<SearchPhone>             phones;
    QList<QSharedPointer<SearchCategory> > categories;
    SearchWeatherConditions        weatherConditions;
    QList<SearchWeatherForcast>    weatherForestcast;
    QList<SearchWeatherConditionsSummary> weatherSummary;
    QString                        cookie;
}SearchPlace;

/**
 * Pair of string.
 */
typedef struct SearchStringPair
{
    QString                        key;
    QString                        value;
    QString                        cookie;
}SearchStringPair;

/**
 * Filter of search result.
 */
typedef struct SearchFilter
{
    QString                        serialized;
    QString                        resultType;
    QList<SearchStringPair>        pairs;
}SearchFilter;

/**
 * Content key of POI.
 */
typedef enum SearchPOIConentKey
{
    SPOICK_Description             = 0,
    SPOICK_Cuisines,
    SPOICK_Features,
    SPOICK_HoursOfOperation,
    SPOICK_Parking,
    SPOICK_Price,
    SPOICK_PaymentMethod,
    SPOICK_Reservations,
    SPOICK_SiteUrl,
    SPOICK_SpecialFeatures,
    SPOICK_ThumbnailPhotoUrl,
    SPOICK_PhotoUrl,
    SPOICK_Wifi,
    SPOICK_Dresscode,
    SPOICK_Smoking,
    SPOICK_Shuttleservice,
    SPOICK_Tips
}SearchPOIConentKey;

/**
 * Pair of key value of POI content.
 */
typedef struct SearchPOIContentKeyValuePair
{
    SearchPOIConentKey              key;
    QString                         value;
    QString                         attributeCookie;
}SearchPOIContentKeyValuePair;

/**
 * Content of vender.
 */
typedef struct SearchVendorContent
{
    QString                         name;
    QList<SearchStringPair>         pairs;
    quint32                         ratingCount;
    float                           averageRating;
}SearchVendorContent;

/**
 * Seven days of one week.
 */
typedef enum SearchDayOfWeek
{
    SDOW_Sunday                     = 0,
    SDOW_Monday,
    SDOW_Tuesday,
    SDOW_Wendesday,
    SDOW_THursdaty,
    SDOW_Friday,
    SDOW_Saturday
}SearchDayOfWeek;

/**
 * Range of time.
 */
typedef struct SearchTimeRange
{
    SearchDayOfWeek                 dayOfWeek;
    qint32                          startTime;
    qint32                          endTime;
}SearchTimeRange;

/**
 * Operation of hours.
 */
typedef struct SearchHoursOfOperation
{
    bool                            hasUtcOffset;
    qint64                          utcOffset;
    QList<SearchTimeRange>          timeRange;
public:
    SearchHoursOfOperation() : hasUtcOffset(false), utcOffset(0) { }
}SearchHoursOfOperation;

/**
 * Content of POI.
 */
typedef struct SearchPOIContent
{
    QList<SearchPOIContentKeyValuePair> pairs;
    QString                         tagLine;
    QList<SearchVendorContent>      vendorContents;
    SearchHoursOfOperation          hoursOfOperation;
    QString                         goldenCookie;
    QString                         poiContentId;
}SearchPOIContent;

/**
 * Related Search Description.
 */
typedef struct SearchRelatedSearch
{
    SearchFilter                    searchFilter;
    QString                         line;
}SearchRelatedSearch;

/**
 * SearchKit POI.
 */
class SearchPOI : public SearchResult
{
public:
    ~SearchPOI() {}
public:
    SearchPOIType                   type;
    SearchPlace                     place;
    float                           distance;
    SearchFilter                    searchFilter;
    SearchPOIContent                poiContent;
    SearchRelatedSearch             relatedSearch;
    bool                            isPremiumPlacement;
    bool                            isUnmappable;
    bool                            isEnhancedPoi;
    quint32                         searchQueryEventId;
    SearchFuelDetails               fuelDetails;
};

/**
 * Type of suggestion.
 */
typedef enum SearchSuggestionType
{
    SST_None                        = 0,
    SST_POI,
    SST_Address,
    SST_Airport,
    SST_Gas,
    SST_Category,
    SST_Local,
    SST_Movie
}SearchSuggestionType;

/**
 * SearchKit Suggestion Match.
 */
class SearchSuggestion : public SearchResult
{
public:
    ~SearchSuggestion() {}
public:
    QList<QString>                  lines;
    SearchFilter                    filter;
    QList<QString>                  iconIds;
    float                           distance;
    SearchSuggestionType            type;
};

/**
 * SearchKit Interest Suggestion List.
 */
class SearchSuggestionList : public SearchResult
{
public:
    ~SearchSuggestionList() {}
public:
    QString                         name;
    QList<SearchSuggestion>         suggestions;
};

/**
 * The content for search event.
 */
typedef struct SearchEventContent
{
    QString                   name;
    QString                   url;
    QString                   formattedText;
    QString                   mpaaRating;
    QDate                     fixedDate;
    QList<QSharedPointer<SearchCategory> > categories;
    QList<SearchStringPair>   pairs;
    QString                   cookie;
    quint32                   ratingStar;
}SearchEventContent;

/**
 * Content of movie information.
 */
typedef struct SearchMovieContent
{
    QString                         genre;
    QString                         duration;
    QString                         movieDesciption;
    QString                         language;
    QString                         actors;
    QString                         director;
    QString                         filmFormat;
    QList<QString>                  posterURLs;
    QList<QString>                  photoURLs;
    SearchEventContent              event;
}SearchMovieContent;


/**
 * SearchKit Movie Information.
 */
class SearchMovie : public SearchResult
{
public:
    ~SearchMovie() {}
public:
    SearchFilter                    searchFilter;
    SearchRelatedSearch             relatedSearch;
    SearchMovieContent              movieContent;
};

/**
 * Type of result information.
 */
typedef enum SearchResultInformationType
{
    SRIT_None                    = 0,
    SRIT_Suggestion              = 1,
    SRIT_POI                     = 2,
    SRIT_SuggestionList          = 3,
    SRIT_MovieShowing            = 4,
    SRIT_Movie                   = 5,
    SRIT_Theater                 = 6,
    SRIT_Fuel                    = 100
}SearchResultInformationType;

/**
 * Content of search result state.
 */
typedef struct SearchResultState
{
    QString                                 serialized;
}SearchResultState;


/**
 * Search Information, mainly contains SearchResult and SearchResultDesciption.
 */
class SearchInformation
{
public:
    ~SearchInformation() {}

public:
    SearchResultInformationType              type;
    bool                                     hasMore;
    QList<QSharedPointer<SearchResult> >     results;
    QList<SearchResultDescription>           descriptions;
    QSharedPointer<SearchFuelSummary>        fuelSummary;
    SearchResultState                        state;
    QList<SearchHoursOfOperation>            hoursOfOperation;
    QList<SearchExtendedAddress>             extendedAddress;
    QList<SearchCompactAddress>              compactAddress;
};

}

#endif // SEARCHDATATYPES_H

/*! @} */
