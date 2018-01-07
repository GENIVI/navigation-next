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
    @file     SingleSearchInformation.h
*/
/*
    (C) Copyright 2014 by TeleCommunication Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems, is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
/*! @{ */

#ifndef __SINGLESEARCHINFORMATION_H__
#define __SINGLESEARCHINFORMATION_H__

#include <string>
#include <list>
#include <vector>
#include "smartpointer.h"
#include "paltypes.h"

namespace nbsearch
{

extern const char* POI_CONTENT_KEY_CUISINES;
extern const char* POI_CONTENT_KEY_DESCRIPTION;
extern const char* POI_CONTENT_KEY_FEATURES;
extern const char* POI_CONTENT_KEY_HOURSOFOPERATION;
extern const char* POI_CONTENT_KEY_PARKING;
extern const char* POI_CONTENT_KEY_PRICE;
extern const char* POI_CONTENT_KEY_PAYEMENTMETHOD;
extern const char* POI_CONTENT_KEY_RESERVATIONS;
extern const char* POI_CONTENT_KEY_SITEURL;
extern const char* POI_CONTENT_KEY_SPECIALFEATURES;
extern const char* POI_CONTENT_KEY_PHOTOURL;
extern const char* POI_CONTENT_KEY_THUMBNAILPHOTOURL;
extern const char* POI_CONTENT_KEY_WIFI;
extern const char* POI_CONTENT_KEY_DRESSCODE;
extern const char* POI_CONTENT_KEY_SMOKING;
extern const char* POI_CONTENT_KEY_SHUTTLESERVICE;
extern const char* POI_CONTENT_KEY_TIPS;

typedef enum
{
    POIK_Description          = 0,    /*!< POI Key for Description  */
    POIK_Cuisines,                    /*!< POI Key for Cuisines  */
    POIK_Features,                    /*!< POI Key for Features  */
    POIK_HoursOfOperation,            /*!< POI Key for Hours of operation  */
    POIK_Parking,                     /*!< POI Key for Parking  */
    POIK_Price,                       /*!< POI Key for Price  */
    POIK_PaymentMethod,               /*!< POI Key for Payment method  */
    POIK_Reservations,                /*!< POI Key for Reservations  */
    POIK_SiteUrl,                     /*!< POI Key for Site URL  */
    POIK_SpecialFeatures,             /*!< POI Key for Special features  */
    POIK_ThumbnailPhotoUrl,           /*!< POI Key for Thumbnail photo url  */
    POIK_PhotoUrl,                    /*!< POI Key for Photo Url*/
    POIK_Wifi,                        /*!< POI Key for Wifi  */
    POIK_Dresscode,                   /*!< POI Key for Dress code  */
    POIK_Smoking,                     /*!< POI Key for Smoking  */
    POIK_ShuttleService,              /*!< POI Key for Shuttle service  */
    POIK_Tips                         /*!< POI Key for Tips */
} POIKey;

/*!  Latitude and Longitude indicates a lcoation
 */
class LatLonPoint
{
public:
    LatLonPoint() : m_latitude(0.0),m_longitude(0.0) {}
    LatLonPoint(double lat, double lon) : m_latitude(lat),m_longitude(lon) {}
    virtual ~LatLonPoint() {}

    virtual double GetLatitude() const {return m_latitude;}
    virtual double GetLongitude() const {return m_longitude;}

public:
    double m_latitude;
    double m_longitude;
};

/*! Bounding box indicates a rectangle region.
 */
class BoundingBox
{
public:
    virtual ~BoundingBox() {}

    virtual const LatLonPoint& GetTopLeft() const {return m_TopLeft;}
    virtual const LatLonPoint& GetBottomRight() const {return m_BottomRight;}

public:
    LatLonPoint m_TopLeft;
    LatLonPoint m_BottomRight;
};

class PoiAttributeCookie
{
public:
    virtual ~PoiAttributeCookie() {}

    virtual const std::string& GetProviderId() const { return m_provideId; }
    virtual const std::string& GetState() const { return m_state; }
    virtual const std::string& GetSerialized() const { return m_serialized; }

public:
    std::string m_provideId;
    std::string m_state;
    std::string m_serialized;
};

/*! Key-Value pair, may have more elements in the future
 */
class StringPair
{
public:
    virtual ~StringPair() {}

    virtual const std::string& GetKey() const {return m_key;}
    virtual const std::string& GetValue() const {return m_value;}
    virtual const PoiAttributeCookie& GetPoiAttributeCookie() const { return m_cookie; }

public:
    std::string m_key;
    std::string m_value;
    PoiAttributeCookie m_cookie;
};

/*! It's used for Ads, andress, or movies.
 */
class PlaceEventCookie
{
public:
    virtual ~PlaceEventCookie() {}
    virtual const std::string& GetProviderId() const = 0;
    virtual const std::string& GetState() const = 0;
    virtual const std::string& GetSerialized() const = 0;
};

/*!
 *  Vendor content information (i.e. trip adviser), could have serverial pairs with the same key (like url).
 */
class VendorContent
{
public:
    virtual ~VendorContent() {}

    virtual const std::string& GetName() const = 0; //Vendor name
    virtual double GetAverageRating() const = 0;
    virtual uint32 GetRatingCount() const = 0;
    virtual const std::vector<StringPair*>& GetPairs() const = 0;
};

/*! This type provides interface to get the inforamtion of a category.
 */
class Category
{
public:
    virtual ~Category() {}

    virtual const std::string& GetCode() const = 0;
    virtual const std::string& GetName() const = 0;
    virtual const Category* GetParentCategory() const = 0;
};

/*! This type provides interface to access search filter provided by server. For now only use serialized
 */
class SearchFilter
{
public:
    virtual ~SearchFilter() {}

    virtual const std::string& GetSerialized() const {return m_serialized;}
    virtual const std::string& GetResultStyle() const {return m_resultStyle;}
    virtual const std::vector<StringPair>& GetPairs() const {return m_pairs;}

public:
    std::string m_serialized;
    std::string m_resultStyle;
    std::vector<StringPair> m_pairs;
};

/*! Information provided by server to perform related search
 */
class RelatedSearch
{
public:
    virtual ~RelatedSearch() {}

    virtual const SearchFilter* GetSearchFilter() const = 0;
    virtual const std::string& GetLine1() const = 0;
};

/*! This type provides interface to get result description information of a singlesearch
 */
class ResultDescription
{
public:
    virtual ~ResultDescription() {}

    virtual const std::string& GetLine1() const = 0;
    virtual const std::string& GetLine2() const = 0;
};

/*! This type provides interface to get a range of time in a day and which day it is in a week.
 */
class TimeRange
{
public:
    virtual ~TimeRange() {}

    virtual uint32 GetDayOfWeek() const = 0;    //Day of the week, where Sunday = 0.
    virtual uint32 GetStartTime() const = 0;    //Time of day in which a business opens, specified as number of seconds since midnight (local time of business).
    virtual uint32 GetEndTime() const = 0;      //Time of day in which a business opens, specified as number of seconds since midnight (local time of business).
};

/*! Represents a fixed date irrespective of time zone.
 */
class FixedDate
{
public:
    virtual ~FixedDate() {}

    virtual const std::string& GetType() const = 0;
    virtual const uint32 GetYear() const = 0;
    virtual const uint32 GetMonth() const = 0;
    virtual const uint32 GetDay() const = 0;
};

/*! This type provides interface to get working time information (e.g. when a store will open)
 */
class HoursOfOperation
{
public:
    virtual ~HoursOfOperation() {}

    virtual const std::vector<TimeRange*>& GetTimeRanges() const = 0;
    virtual bool GetUTCOffset(uint32& utcoffset) const = 0;
};

typedef enum
{
    ECT_Movie,      /*!< Movie type of the content */
    ECT_Event       /*!< Event type of the content */
} EventContentType;

/*! This type provides interface to get content of event.
 */
class EventContent
{
public:
    virtual ~EventContent() {}

    virtual const std::string& GetName() const = 0;
    virtual const EventContentType GetType() const = 0;
    virtual const std::string& GetURL() const = 0;
    virtual const std::string& GetMPAARating() const = 0;
    virtual const std::string& GetFormatedText() const = 0;
    virtual uint32 GetStartRating() const = 0;
    virtual const FixedDate* GetFixedDate() const = 0;
    virtual const std::vector<StringPair*>& GetPairs() const = 0;
    virtual const std::vector<Category*>& GetCategories() const = 0;
    virtual const PlaceEventCookie* GetPlaceEventCookie() const = 0;
};

/*! This is an performance/showtime for an event.
 */
class EventPerformance
{
public:
    virtual ~EventPerformance() {}

    virtual uint32 GetStartTime() const = 0;
    virtual uint32 GetEndTime() const = 0;
    virtual int32 GetUtcOffset() const = 0;
};

/*! This type is event. It represents a movie on show in a theater.
 */
class Event
{
public:
    virtual ~Event() {}

    virtual const EventContent* GetEventContent() const = 0;
    virtual const std::vector<EventPerformance*>& GetEventPerformances() const = 0;
};

typedef enum
{
    PT_Primary = 0,          /*!< phone type primary */
    PT_Secondary,            /*!< phone type secondary */
    PT_National,             /*!< phone type national */
    PT_Mobile,               /*!< phone type mobile */
    PT_Fax                   /*!< phone type fax */
} PhoneType;

/*! This type provides interface to access a phone contact
 */
class Phone
{
public:
    virtual ~Phone() {}

    virtual const PhoneType GetPhoneType() const = 0;
    virtual const std::string& GetCountry() const = 0;
    virtual const std::string& GetArea() const = 0;
    virtual const std::string& GetNumber() const = 0;
    virtual const std::string& GetFormattedNumber() const = 0;
};

/*! This type provides interface which describes all possible information necessary to geocode an address
 */
class Address
{
public:
    virtual ~Address() {}

    virtual const std::string& GetNumber() const = 0;
    virtual const std::string& GetStreet() const = 0;
    virtual const std::string& GetCity() const = 0;
    virtual const std::string& GetCounty() const = 0;
    virtual const std::string& GetState() const = 0;
    virtual const std::string& GetZipCode() const = 0;
    virtual const std::string& GetCountry() const = 0;
};


/*! Formatted address line, max 5 lines return by server.
 */
class ExtendedAddress
{
public:
    virtual ~ExtendedAddress() {}

    virtual const std::vector<std::string> & GetLines() const {return m_lines;}

public:
    std::vector<std::string> m_lines;
};

/*! Formatted address line, max 2 lines return by server.
 */
class CompactAddress
{
public:
    virtual ~CompactAddress() {}

    virtual const std::string& GetLineSeparator() const {return m_lineSeparator;}
    virtual const std::string& GetLine1() const {return m_line1;}
    virtual const std::string& GetLine2() const {return m_line2;}
    virtual const std::string& GetAddressLine() const {return m_addressLine;}

public:
    std::string m_lineSeparator;
    std::string m_line1;
    std::string m_line2;
    std::string m_addressLine;
};

typedef enum {
    LT_None = 0,             /*!< No location information available */
    LT_Address,              /*!< Address as in Street number, Street, City, State, ZIP code */
    LT_Intersection,         /*!< Intersection between Street 1, Street 2 at City, State, ZIP code */
    LT_Airport,              /*!< Airport name */
    LT_LatLon,               /*!< Latitude, Longitude */
    LT_MyGPS,                /*!< GPS location */
    LT_Route,                /*!< Location information along a route */
    LT_RouteGPS,             /*!< GPS location, along a route */
    LT_InMyDirection,        /*!< To indicate a location in my direction */
    LT_AddressFreeForm       /*!< Address as in Street number, Street, City, State, ZIP code in free form*/
} LocationType;


/*! Main type which provide interface to describe location.

 The value of the `type' field describes where to find the location
 information in the other fields.
 */
class Location
{
public:
    virtual ~Location() {}

    virtual LocationType GetType(void) const {return m_type;}
    virtual const LatLonPoint& GetCoordinates(void) const {return m_point;}
    virtual const std::string& GetName(void) const {return m_name;}
    virtual const std::string& GetNumber(void) const {return m_number;}
    virtual const std::string& GetStreet(void) const {return m_street;}
    virtual const std::string& GetCrossStreet(void) const {return m_crossStreet;}
    virtual const std::string& GetCity(void) const {return m_city;}
    virtual const std::string& GetCounty(void) const {return m_county;}
    virtual const std::string& GetState(void) const {return m_state;}
    virtual const std::string& GetZipCode(void) const {return m_zipcode;}
    virtual const std::string& GetCountry(void) const {return m_country;}
    virtual const std::string& GetAirport(void) const {return m_airport;}
    virtual const std::string& GetCountryName(void) const {return m_countryName;}
    virtual const std::string& GetFreeForm(void) const { return m_freeForm; }
    virtual const ExtendedAddress& GetExtendedAddress(void) const {return m_extAddress;}
    virtual const CompactAddress& GetCompactAddress(void) const {return m_compAddress;}
    virtual float GetUTCOffset(void) const {return m_UTCOffset;}

public:
    LocationType m_type;
    LatLonPoint m_point;
    std::string m_name;
    std::string m_number;
    std::string m_street;
    std::string m_crossStreet;
    std::string m_city;
    std::string m_county;
    std::string m_state;
    std::string m_zipcode;
    std::string m_country;
    std::string m_airport;
    std::string m_countryName;
    std::string m_freeForm;
    ExtendedAddress m_extAddress;
    CompactAddress m_compAddress;
    float       m_UTCOffset;
};

/*! Weather conditions wrapper.
 */
class WeatherConditions
{
public:
    virtual ~WeatherConditions() {}

    virtual float GetTemperature() const = 0;
    virtual float GetDewPoint() const = 0;
    virtual uint32 GetRelHumidity() const = 0;
    virtual float GetWindSpeed() const = 0;
    virtual uint32 GetWindDirection() const = 0;
    virtual float GetWindGust() const = 0;
    virtual float GetPressure() const = 0;
    virtual const std::string& GetWeatherCondition() const = 0;
    virtual uint32 GetConditionCode() const = 0;
    virtual const std::string& GetSkyCover() const = 0;
    virtual float GetCloudBaseHeight() const = 0;
    virtual uint32 GetVisibility() const = 0;
    virtual float GetHeatIndex() const = 0;
    virtual float GetWindChill() const = 0;
    virtual float GetSnowDepth() const = 0;
    virtual float GetMaxTempInLast6Hours() const = 0;
    virtual float GetMaxTempInLast24Hours() const = 0;
    virtual float GetMinTempInLast6Hours() const = 0;
    virtual float GetMinTempInLast24Hours() const = 0;
    virtual float GetPrecipitatinInLast3Hours() const = 0;
    virtual float GetPrecipitatinInLast6Hours() const = 0;
    virtual float GetPrecipitationInLast24Hours() const = 0;
    virtual uint32 GetUpdateTime() const = 0;
    virtual int GetUTCOffset() const = 0;
};

/*! Weather forecast wrapper.
 */
class WeatherForecast
{
public:
    virtual ~WeatherForecast() {}

    virtual uint32 GetDate() const = 0;
    virtual float GetHighTemperature() const = 0;
    virtual float GetLowTemperature() const = 0;
    virtual float GetWindSpeed() const = 0;
    virtual uint32 GetWindDirection() const = 0;
    virtual uint32 GetPrecipitationProbability() const = 0;
    virtual uint32 GetRelativeHumidity() const = 0;
    virtual const std::string& GetWeatherCondition() const = 0;
    virtual uint32 GetConditionCode() const = 0;
    virtual uint32 GetUVIndex() const = 0;
    virtual const std::string& GetUVDescription() const = 0;
    virtual const std::string& GetSunriseTime() const = 0;
    virtual const std::string& GetSunsetTime() const = 0;
    virtual const std::string& GetMoonriseTime() const = 0;
    virtual const std::string& GetMoonsetTime() const = 0;
    virtual uint32 GetMoonPhase() const = 0;
    virtual uint32 GetUpdateTime() const = 0;
    virtual int GetUTCOffset() const = 0;
};

/*! Weather conditions summary wrapper.
 */
class WeatherConditionsSummary
{
public:
    virtual ~WeatherConditionsSummary() {}

    virtual float GetTemperature() const = 0;             /*! Current temperature in degrees C. */
    virtual const std::string& GetCondition() const = 0;  /*! Weather conditions expanded to a string. */
    virtual uint32 GetConditionCode() const = 0;          /*! Weather conditions as a numeric code. */
    virtual float GetMaxTemperatureIn24Hours() const = 0; /*! 24 hours maximum temperature in degrees C. */
    virtual float GetMinTemperatureIn24Hours() const = 0; /*! 24 hours minmum temperature in degrees C. */
    virtual int GetUTCOffset() const = 0;              /*! UTC offset in seconds. */
};

/*! This type provides interface for a mapped place or or point of interest
 */
class Place
{
public:
    virtual ~Place() {}

    virtual const std::string& GetName() const = 0;
    virtual const Location* GetLocation() const = 0;
    virtual const std::vector<Phone*>& GetPhoneNumbers() const = 0;
    virtual const std::vector<Category*>& GetCategories() const = 0;
    virtual const std::vector<Event*>& GetEvents() const = 0; /*! Please see the Event definition */
    virtual bool GetTheaterId(std::string&) const = 0;
    virtual const std::vector<WeatherConditionsSummary*> GetWeatherConditionsSummaries() const = 0; /*! Summary information for weather conditons at a place. */
    virtual const WeatherConditions* GetWeatherConditions() const = 0;  /*! Weather conditions for a place. */
    virtual const std::vector<WeatherForecast*> GetWeatherForecast() const = 0; /*! Weather forecast for a place. */
    virtual const PlaceEventCookie* GetPlaceEventCookie() const = 0;  /*! Place event cookie for a place. */
};

/*! Information to be returned to the server when reporting events that can be monetized.
 */
class GoldenCookie
{
public:
    virtual ~GoldenCookie() {}
    virtual const std::string& GetProviderId() const = 0;
    virtual const std::string& GetState() const = 0;
    virtual const std::string& GetSerialized() const = 0;
};

/*! This type provides interface for the content information of a POI
 */
class POIContent
{
public:
    virtual ~POIContent() {}

    virtual const std::vector<POIKey> GetKeys() const = 0;
    virtual bool GetKeyValue(POIKey poiKey, std::vector<std::string>& outValue) const = 0;

    virtual bool GetTagLine(std::string& tagLine) const = 0;
    virtual const HoursOfOperation* GetHoursOfOperation() const = 0;
    virtual const std::vector<VendorContent*>& GetVendorContents() const = 0;
    virtual const std::vector<std::string> GetKeysOfString() const = 0;
    virtual const std::vector<std::string> GetValuesByKey(const std::string& key) const = 0;
    virtual const std::vector<StringPair*> GetStringPairs() const = 0;
    virtual const GoldenCookie* GetGoldenCookie() const = 0;
    virtual const std::string& GetId() const = 0;

    static POIKey convertStringToKey(const std::string& string);
};

/*! This type provides interface for a type which describing information for different kinds of fuels.
 */
class FuelType
{
public:
    virtual ~FuelType() {}

    virtual const std::string& GetProductName() const = 0; /*!< Name of the actual fuel product as sold by providers */
    virtual const std::string& GetCode() const = 0;        /*!< Internal code for this fuel type */
    virtual const std::string& GetTypeName() const = 0;    /*!< The type of the fuel regardless of the provider */
};

/*! This type provides interface for a type which describing price information.
 */
class Price
{
public:
    virtual ~Price() {}

    virtual double GetValue() const = 0;
    virtual const std::string& GetCurrency() const = 0;
    virtual uint32 GetModTime() const = 0;
};

/*! This type provides interface for a type which describing a fuel product as found in various locations.
 */
class FuelProduct
{
public:
    virtual ~FuelProduct() {}

    virtual const Price* GetPrice() const = 0;
    virtual const std::string& GetUnits() const = 0;
    virtual const FuelType* GetFuelType() const = 0;
};

/*! This type provides interface for a type which describing summary information for a given fuel product
 */
class FuelSummary
{
public:
    virtual ~FuelSummary() {}

    virtual const FuelProduct* GetAverage() const = 0;
    virtual const FuelProduct* GetLow() const = 0;
};

/*! This type provides interface for a type which describing fuel products
 */
class FuelDetails
{
public:
    virtual ~FuelDetails() {}

    virtual const std::vector<FuelProduct*>& GetFuelProducts() const = 0;
};

/*! This type provides interface for a type which describing result state.
 */
class ResultState
{
public:
    virtual ~ResultState() {}

    virtual const std::string& GetSerialized() const = 0;
};

typedef enum
{
    SRT_None = 0,           /*!< Search result type none */
    SRT_POI,                /*!< Search result type poi */
    SRT_SuggestMatch,       /*!< Search result type suggest match */
    SRT_SuggestList,        /*!< Search result type suggest list */
    SRT_ProxMatchContent    /*!< Search result type movie */
} SearchResultType;
/*!
 *  Base class of POI/Suggestion/Movie
 */
class SearchResultBase
{
public:
    SearchResultBase() {}
    virtual ~SearchResultBase() {}

    virtual SearchResultType GetSearchResultType() const = 0;
};

typedef enum
{
    SMT_None = 0,       /*!< Suggestion Match type none */
    SMT_POI,            /*!< Suggestion Match type POI */
    SMT_Address,        /*!< Suggestion Match type Address */
    SMT_Airport,        /*!< Suggestion Match type Airport */
    SMT_Gas,            /*!< Suggestion Match type Gas */
    SMT_Category,       /*!< Suggestion Match type Category */
    SMT_Local,          /*!< Suggestion Match type Local */
    SMT_Movie           /*!< Suggestion Match type Movie */
} SuggestionMatchType;

class SuggestionMatch : public SearchResultBase
{
public:

    virtual ~SuggestionMatch(){};

    virtual const SearchFilter* GetSearchFilter() const = 0;
    virtual double GetDistance() const = 0;
    virtual const std::string& GetLine1() const = 0;
    virtual const std::string& GetLine2() const = 0;
    virtual const std::string& GetLine3() const = 0;
    virtual const SuggestionMatchType GetMatchType() const = 0;
    virtual const std::vector<std::string>& GetIconIDs() const = 0;
};

class SuggestionList : public SearchResultBase
{
public:
    virtual ~SuggestionList() {}

    virtual const std::string& GetName() const = 0;
    virtual const std::vector<SuggestionMatch*>& GetSuggestionMatches() const = 0;
};

/*! Results which represent event content available in the local area.
 *  Once the content is selected, showings/performances of the content can be searched for.
 */
class ProxMatchContent : public SearchResultBase
{
public:
    virtual ~ProxMatchContent() {}

    virtual const SearchFilter* GetSearchFilter() const = 0;
    virtual const EventContent* GetEventContent() const = 0;
    virtual const RelatedSearch* GetRelatedSearch() const = 0;
};

typedef enum
{
    POIT_Location    = 0,    /*!< poi type location */
    POIT_POI                 /*!< poi type POI */
} POIType;

class POI : public SearchResultBase
{
public:
    virtual ~POI(){}

    virtual POIType GetPOIType() const = 0;
    virtual const Place* GetPlace() const = 0;
    virtual double GetDistance() const = 0;
    virtual const SearchFilter* GetSearchFilter() const = 0;
    virtual const POIContent* GetPOIContent() const = 0;
    virtual const FuelDetails* GetFuelDetails() const = 0;
    virtual const RelatedSearch* GetRelatedSearch() const = 0;
    virtual bool IsPremiumPlacement() const = 0;
    virtual bool IsUnmappable() const = 0;
    virtual bool IsEnhancedPoi() const = 0;
    virtual uint32 GetSearchQueryEventId() const = 0;
};

typedef enum
{
    SSIRT_None = 0,            /*!< Result type none */
    SSIRT_SuggestionMatch,     /*!< Result type Suggestion match */
    SSIRT_POI,                 /*!< Result type POI */
    SSIRT_SuggestionList,      /*!< Result type Suggestion list */
    SSIRT_MovieShowing,        /*!< Result type movie and it's theaters */
    SSIRT_Movie,               /*!< Result type movies */
    SSIRT_MovieTheater,        /*!< Result type theater */
} SingleSearchInformationResultType;

class SingleSearchInformation
{
public:
    virtual ~SingleSearchInformation(){}

    virtual bool HasMore() const = 0;

    virtual int GetResultCount() const = 0;
    virtual SingleSearchInformationResultType GetResultType() const = 0;

    virtual const SearchResultBase* GetResultAtIndex(uint32 index) const = 0;
    virtual const POI* GetPOIAtIndex(uint32 index) const = 0;
    virtual const SuggestionMatch* GetSuggestionMatchAtIndex(uint32 index) const = 0;
    virtual const FuelSummary* GetFuelSummary() const = 0;
    virtual const std::vector<ResultDescription*>& GetResultDescriptions() const = 0;
    virtual const SuggestionList* GetSuggestionListAtIndex(uint32 index) const = 0;
    virtual const ResultState* GetResultState() const = 0;
    virtual const ProxMatchContent* GetProxMatchContentAtIndex(uint32 index) const = 0;
};

}

#endif //__SINGLESEARCHINFORMATION_H__

/*! @} */
