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
 @file     SingleSearchInformationImpl.h
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

#ifndef __SEARCH_SINGLE_SEARCH_ONBOARD_INFORMATION_H__
#define __SEARCH_SINGLE_SEARCH_ONBOARD_INFORMATION_H__

#include "SingleSearchInformation.h"
#include "NBProtocolSuggestList.h"
#include "NBProtocolSuggestListSerializer.h"
#include "nbsinglesearchhandler.h"
#include <map>
#include <vector>
#include <sstream>
#include "smartpointer.h"
#include "NBProtocolSingleSearchSourceInformation.h"
#include "TpsElement.h"
#include <ncdb/include/SingleSearch.h>

namespace nbsearch
{

class OnboardPoiAttributeCookieUtil
{
public:
    static PoiAttributeCookie toPoiAttributeCookie(const protocol::PoiAttributeCookieSharedPtr& cookie);
    static std::string Serialize(const protocol::PoiAttributeCookieSharedPtr& poiAttributeCookieProtocol);
    static protocol::PoiAttributeCookieSharedPtr Deserialize(const std::string& serializedData);
};

class OnboardStringPairUtil
{
public:
    static StringPair toStringPair(const protocol::PairSharedPtr& pair);
};

class OnboardPlaceEventCookieImpl : public PlaceEventCookie
{
public:
    OnboardPlaceEventCookieImpl(const char* provideId, const char* state) : m_provideId(provideId), m_state(state) {}
    OnboardPlaceEventCookieImpl(const PlaceEventCookie& copy);
    OnboardPlaceEventCookieImpl(const OnboardPlaceEventCookieImpl& copy);
    OnboardPlaceEventCookieImpl(protocol::PlaceEventCookieSharedPtr cookie);
    virtual ~OnboardPlaceEventCookieImpl() {}
    virtual const std::string& GetProviderId() const { return m_provideId; }
    virtual const std::string& GetState() const { return m_state; }
    virtual const std::string& GetSerialized() const { return m_serialized; }

    static std::string Serialize(const protocol::PlaceEventCookieSharedPtr& placeEventCookieProtocol);
    static protocol::PlaceEventCookieSharedPtr Deserialize(const std::string& serializedData);

private:
    std::string m_provideId;
    std::string m_state;
    std::string m_serialized;
};

class OnboardVendorContentImpl : public VendorContent
{
public:
    OnboardVendorContentImpl(protocol::VendorContentSharedPtr vendorContent);
    OnboardVendorContentImpl(const VendorContent& copy);
    OnboardVendorContentImpl(const char* name) : m_name(name) {}
    virtual ~OnboardVendorContentImpl();

    virtual const std::string& GetName() const {return m_name;} //Vendor name
    virtual double GetAverageRating() const {return m_averageRating;}
    virtual uint32 GetRatingCount() const {return m_ratingCount;}
    virtual const std::vector<StringPair*>& GetPairs() const {return m_pairs;}

private:
    std::string m_name;
    std::vector<StringPair*> m_pairs;
    uint32 m_ratingCount;
    double m_averageRating;
};

class OnboardCategoryImpl : public Category
{
public:
    OnboardCategoryImpl() : m_parent(NULL) {}
    OnboardCategoryImpl(const Category& copy);
    OnboardCategoryImpl(const OnboardCategoryImpl& copy);
    OnboardCategoryImpl(const char* code, const char* name) : m_code(code), m_name(name), m_parent(NULL) {}
    OnboardCategoryImpl(protocol::CategorySharedPtr category);
    virtual ~OnboardCategoryImpl() {delete m_parent;}

    virtual const std::string& GetCode() const {return m_code;}
    virtual const std::string& GetName() const {return m_name;}
    virtual const Category* GetParentCategory() const {return m_parent;}

    std::string m_code;
    std::string m_name;
    OnboardCategoryImpl* m_parent;
};

class OnboardSearchFilterUtil
{
public:
    static SearchFilter toSearchFilter(const protocol::SearchFilterSharedPtr& searchFilterProtocol);
    static std::string Serialize(const protocol::SearchFilterSharedPtr& searchFilterProtocol);
    static protocol::SearchFilterSharedPtr Deserialize(const std::string& serializedData);
};

class OnboardRelatedSearchImpl : public RelatedSearch
{
public:
    OnboardRelatedSearchImpl(protocol::RelatedSearchSharedPtr relatedSearch);
    OnboardRelatedSearchImpl(const OnboardRelatedSearchImpl& copy);
    OnboardRelatedSearchImpl(const RelatedSearch& copy);
    virtual ~OnboardRelatedSearchImpl();

    virtual const SearchFilter* GetSearchFilter() const {return m_searchFilter;}
    virtual const std::string& GetLine1() const {return m_line1;}

private:
    SearchFilter* m_searchFilter;
    std::string m_line1;
};

class OnboardResultDescriptionImpl : public ResultDescription
{
public:
    OnboardResultDescriptionImpl(protocol::ResultDescriptionSharedPtr resultDescription);
    virtual ~OnboardResultDescriptionImpl() {}

    virtual const std::string& GetLine1() const {return m_line1;}
    virtual const std::string& GetLine2() const {return m_line2;}

    std::string m_line1;
    std::string m_line2;
};

class OnboardTimeRangeImpl : public TimeRange
{
public:
    OnboardTimeRangeImpl() : m_dayOfWeek(0), m_startTime(0), m_endTime(0) {}
    OnboardTimeRangeImpl(const OnboardTimeRangeImpl& copy);
    OnboardTimeRangeImpl(protocol::TimeRangeSharedPtr timeRange);
    virtual ~OnboardTimeRangeImpl() {}

    virtual uint32 GetDayOfWeek() const {return m_dayOfWeek;}    //Day of the week, where Sunday = 0.
    virtual uint32 GetStartTime() const {return m_startTime;}    //Time of day in which a business opens, specified as number of seconds since midnight (local time of business).
    virtual uint32 GetEndTime() const {return m_endTime;}      //Time of day in which a business opens, specified as number of seconds since midnight (local time of business).

private:
    uint32 m_dayOfWeek;
    uint32 m_startTime;
    uint32 m_endTime;
};

class OnboardFixedDateImpl : public FixedDate
{
public:
    OnboardFixedDateImpl(const protocol::FixedDateSharedPtr);
    virtual ~OnboardFixedDateImpl() {}

    virtual const std::string& GetType() const {return m_type;}
    virtual const uint32 GetYear() const {return m_year;}
    virtual const uint32 GetMonth() const {return m_month;}
    virtual const uint32 GetDay() const {return m_day;}

private:
    std::string m_type;
    uint32 m_year;
    uint32 m_month;
    uint32 m_day;
};

class OnboardHoursOfOperationImpl : public HoursOfOperation
{
public:
    OnboardHoursOfOperationImpl() : m_hasUtcOffset(false), m_utcOffset(0) {}
    OnboardHoursOfOperationImpl(const HoursOfOperation& copy);
    OnboardHoursOfOperationImpl(const OnboardHoursOfOperationImpl& copy);
    OnboardHoursOfOperationImpl(protocol::HoursOfOperationSharedPtr hourOfOperation);
    virtual ~OnboardHoursOfOperationImpl();

    virtual const std::vector<TimeRange*>& GetTimeRanges() const {return m_timeRanges;}
    virtual bool GetUTCOffset(uint32& utcoffset) const;

private:
    bool m_hasUtcOffset;
    uint32 m_utcOffset;
    std::vector<TimeRange*> m_timeRanges;
};

class OnboardEventContentImpl : public EventContent
{
public:
    explicit OnboardEventContentImpl(protocol::EventContentSharedPtr eventContent);
    virtual ~OnboardEventContentImpl();

    virtual const std::string& GetName() const {return m_name;}
    virtual const EventContentType GetType() const {return m_type;}
    virtual const std::string& GetURL() const {return m_url;}
    virtual const std::string& GetMPAARating() const {return m_mpaa;}
    virtual const std::string& GetFormatedText() const {return m_text;}
    virtual uint32 GetStartRating() const {return m_startRating;}
    virtual const FixedDate* GetFixedDate() const {return m_fixDate;}
    virtual const std::vector<StringPair*>& GetPairs() const {return m_pairs;}
    virtual const std::vector<Category*>& GetCategories() const {return m_categories;}
    virtual const PlaceEventCookie* GetPlaceEventCookie() const {return m_cookie;}

private:
    std::string m_name;
    EventContentType m_type;
    std::string m_url;
    std::string m_mpaa;
    std::string m_text;
    uint32 m_startRating;
    OnboardFixedDateImpl* m_fixDate;
    std::vector<StringPair*> m_pairs;
    std::vector<Category*> m_categories;
    OnboardPlaceEventCookieImpl* m_cookie;
};

class OnboardEventPerformanceImpl : public EventPerformance
{
public:
    explicit OnboardEventPerformanceImpl(const protocol::EventPerformanceSharedPtr eventPerformance);
    virtual ~OnboardEventPerformanceImpl() {}

    virtual uint32 GetStartTime() const {return m_startTime;}
    virtual uint32 GetEndTime() const {return m_endTime;}
    virtual int32 GetUtcOffset() const {return m_utcOffset;}

private:
    uint32 m_startTime;
    uint32 m_endTime;
    int32 m_utcOffset;
};

class OnboardEventImpl : public Event
{
public:
    explicit OnboardEventImpl(const protocol::EventSharedPtr event);
    virtual ~OnboardEventImpl();

    virtual const EventContent* GetEventContent() const {return m_eventContent;}
    virtual const std::vector<EventPerformance*>& GetEventPerformances() const {return m_eventPerformances;}

private:
    OnboardEventContentImpl* m_eventContent;
    std::vector<EventPerformance*> m_eventPerformances;
};

/*! Results which represent event content available in the local area.
    Once the content is selected, showings/performances of the content can be searched for.
 */
class OnboardProxMatchContentImpl : public ProxMatchContent
{
public:
    explicit OnboardProxMatchContentImpl(protocol::ProxMatchContentSharedPtr proxMatchContent);
    virtual ~OnboardProxMatchContentImpl();

    virtual SearchResultType GetSearchResultType() const {return SRT_ProxMatchContent;}
    virtual const SearchFilter* GetSearchFilter() const {return m_searchFilter;}
    virtual const EventContent* GetEventContent() const {return m_eventContent;}
    virtual const RelatedSearch* GetRelatedSearch() const {return m_relatedSearch;}

private:
    SearchFilter* m_searchFilter;
    OnboardEventContentImpl* m_eventContent;
    OnboardRelatedSearchImpl* m_relatedSearch;
};

/*! This type provides interface to access a phone contact
 */
class OnboardPhoneImpl : public Phone
{
public:
    OnboardPhoneImpl() {}
    OnboardPhoneImpl(const Phone& copy);
    OnboardPhoneImpl(const OnboardPhoneImpl& copy);
    OnboardPhoneImpl(const Ncdb::UtfString& phone);
    virtual ~OnboardPhoneImpl() {}

    virtual const PhoneType GetPhoneType() const {return m_type;}
    virtual const std::string& GetCountry() const {return m_country;}
    virtual const std::string& GetArea() const {return m_area;}
    virtual const std::string& GetNumber() const {return m_number;}
    virtual const std::string& GetFormattedNumber() const {return m_formattedNumber;}

    PhoneType   m_type;
    std::string m_country;
    std::string m_area;
    std::string m_number;
    std::string m_formattedNumber;
};

/*! This type provides interface which describes all possible information necessary to geocode an address
 */
class OnboardAddressImpl : public Address
{
public:
    OnboardAddressImpl() {}
    OnboardAddressImpl(const OnboardAddressImpl& copy);
    virtual ~OnboardAddressImpl() {}

    virtual const std::string& GetNumber() const {return m_number;}
    virtual const std::string& GetStreet() const {return m_street;}
    virtual const std::string& GetCity() const {return m_city;}
    virtual const std::string& GetCounty() const {return m_county;}
    virtual const std::string& GetState() const {return m_state;}
    virtual const std::string& GetZipCode() const {return m_zipcode;}
    virtual const std::string& GetCountry() const {return m_country;}

private:
    std::string m_number;
    std::string m_street;
    std::string m_city;
    std::string m_county;
    std::string m_state;
    std::string m_zipcode;
    std::string m_country;
};

class OnboardExtendedAddressUtil
{
public:
    static ExtendedAddress toExtendedAddress(const Ncdb::FormattedAddressAutoArray& extendedAddress);
};

class OnboardCompactAddressUtil
{
public:
    static CompactAddress toCompactAddress(const Ncdb::FormattedAddressAutoArray& compactAddress);
};

class OnboardLocationUtil
{
public:
    static bool isEqual(const Location& location1, const Location& location2);
    static Location* toLocation(Ncdb::SearchMatch* sm);
};

class OnboardWeatherConditionsImpl : public WeatherConditions
{
public:
    OnboardWeatherConditionsImpl();
    virtual ~OnboardWeatherConditionsImpl() {}
    OnboardWeatherConditionsImpl(const WeatherConditions& copy);
    OnboardWeatherConditionsImpl(const OnboardWeatherConditionsImpl& copy);
    OnboardWeatherConditionsImpl(protocol::WeatherConditionsSharedPtr conditons);

    virtual float GetTemperature() const { return m_temperature; }
    virtual float GetDewPoint() const { return m_dewPoint; }
    virtual uint32 GetRelHumidity() const { return m_relHumidity; }
    virtual float GetWindSpeed() const { return m_windSpeed; }
    virtual uint32 GetWindDirection() const { return m_windDirection; }
    virtual float GetWindGust() const { return m_windGust; }
    virtual float GetPressure() const { return m_pressure; }
    virtual const std::string& GetWeatherCondition() const { return m_condition; }
    virtual uint32 GetConditionCode() const { return m_conditonCode; }
    virtual const std::string& GetSkyCover() const { return m_sky; }
    virtual float GetCloudBaseHeight() const { return m_ceiling; }
    virtual uint32 GetVisibility() const { return m_visibility; }
    virtual float GetHeatIndex() const { return m_heatIndex; }
    virtual float GetWindChill() const { return m_windChill; }
    virtual float GetSnowDepth() const { return m_snowDepth; }
    virtual float GetMaxTempInLast6Hours() const { return m_maxTemp6Hour; }
    virtual float GetMaxTempInLast24Hours() const { return m_maxTemp24Hour; }
    virtual float GetMinTempInLast6Hours() const { return m_minTemp6Hour; }
    virtual float GetMinTempInLast24Hours() const { return m_minTemp24Hour; }
    virtual float GetPrecipitatinInLast3Hours() const { return m_precipitation3Hour; }
    virtual float GetPrecipitatinInLast6Hours() const { return m_precipitation6Hour; }
    virtual float GetPrecipitationInLast24Hours() const { return m_precipitation24Hour; }
    virtual uint32 GetUpdateTime() const { return m_updateTime; }
    virtual int GetUTCOffset() const { return m_utcOffset; }

    float m_temperature;
    float m_dewPoint;
    uint32 m_relHumidity;
    float m_windSpeed;
    uint32 m_windDirection;
    float m_windGust;
    float m_pressure;
    std::string m_condition;
    uint32 m_conditonCode;
    std::string m_sky;
    float m_ceiling;
    uint32 m_visibility;
    float m_heatIndex;
    float m_windChill;
    float m_snowDepth;
    float m_maxTemp6Hour;
    float m_maxTemp24Hour;
    float m_minTemp6Hour;
    float m_minTemp24Hour;
    float m_precipitation3Hour;
    float m_precipitation6Hour;
    float m_precipitation24Hour;
    uint32 m_updateTime;
    int m_utcOffset;
};

class OnboardWeatherForecastImpl : public WeatherForecast
{
public:
    OnboardWeatherForecastImpl();
    virtual ~OnboardWeatherForecastImpl() {}
    OnboardWeatherForecastImpl(const OnboardWeatherForecastImpl& copy);
    OnboardWeatherForecastImpl(const WeatherForecast& copy);
    OnboardWeatherForecastImpl(protocol::WeatherForecastSharedPtr forecast);

    virtual uint32 GetDate() const { return m_date; }
    virtual float GetHighTemperature() const { return m_highTemperature; }
    virtual float GetLowTemperature() const { return m_lowTemperature; }
    virtual float GetWindSpeed() const { return m_windSpeed; }
    virtual uint32 GetWindDirection() const { return m_windDirection; }
    virtual uint32 GetPrecipitationProbability() const { return m_precipitationProbability; }
    virtual uint32 GetRelativeHumidity() const { return m_relativeHumidity; }
    virtual const std::string& GetWeatherCondition() const { return m_condition; }
    virtual uint32 GetConditionCode() const { return m_conditonCode; }
    virtual uint32 GetUVIndex() const { return m_uvIndex; }
    virtual const std::string& GetUVDescription() const { return m_uvDescription; }
    virtual const std::string& GetSunriseTime() const { return m_sunriseTime; }
    virtual const std::string& GetSunsetTime() const { return m_sunsetTime; }
    virtual const std::string& GetMoonriseTime() const { return m_moonriseTime; }
    virtual const std::string& GetMoonsetTime() const { return m_moonsetTime; }
    virtual uint32 GetMoonPhase() const { return m_moonPhase; }
    virtual uint32 GetUpdateTime() const { return m_updateTime; }
    virtual int GetUTCOffset() const { return m_utcOffset; }

    uint32 m_date;
    float m_highTemperature;
    float m_lowTemperature;
    float m_windSpeed;
    uint32 m_windDirection;
    uint32 m_precipitationProbability;
    uint32 m_relativeHumidity;
    std::string m_condition;
    uint32 m_conditonCode;
    uint32 m_uvIndex;
    std::string m_uvDescription;
    std::string m_sunriseTime;
    std::string m_sunsetTime;
    std::string m_moonriseTime;
    std::string m_moonsetTime;
    uint32 m_moonPhase;
    uint32 m_updateTime;
    int m_utcOffset;
};

class OnboardWeatherConditionsSummaryImpl : public WeatherConditionsSummary
{
public:
    OnboardWeatherConditionsSummaryImpl();
    virtual ~OnboardWeatherConditionsSummaryImpl() {}
    OnboardWeatherConditionsSummaryImpl(const WeatherConditionsSummary& copy);
    OnboardWeatherConditionsSummaryImpl(const OnboardWeatherConditionsSummaryImpl& copy);
    OnboardWeatherConditionsSummaryImpl(protocol::WeatherConditionsSummarySharedPtr summary);

    virtual float GetTemperature() const { return m_temperature; }
    virtual const std::string& GetCondition() const { return m_condition; }
    virtual uint32 GetConditionCode() const { return m_conditonCode; }
    virtual float GetMaxTemperatureIn24Hours() const { return m_maxTemp24Hour; }
    virtual float GetMinTemperatureIn24Hours() const { return m_minTemp24Hour; }
    virtual int GetUTCOffset() const { return m_utcOffset; }

    float m_temperature;
    std::string m_condition;
    uint32 m_conditonCode;
    float m_maxTemp24Hour;
    float m_minTemp24Hour;
    int m_utcOffset;
};

/*! This type provides interface for a mapped place or or point of interest */
class OnboardPlaceImpl : public Place
{
public:
    OnboardPlaceImpl();
    OnboardPlaceImpl(const Place& copy);
    OnboardPlaceImpl(const OnboardPlaceImpl& copy);
    OnboardPlaceImpl(Ncdb::SearchMatch* sm);
    virtual ~OnboardPlaceImpl();

    virtual const std::string& GetName() const {return m_name;}
    virtual const Location* GetLocation() const {return m_location;}
    virtual const std::vector<Phone*>& GetPhoneNumbers() const {return m_phones;}
    virtual const std::vector<Category*>& GetCategories() const {return m_categories;}
    virtual const std::vector<Event*>& GetEvents() const {return m_events;}
    virtual bool GetTheaterId(std::string& theaterId) const;
    virtual const std::vector<WeatherConditionsSummary*> GetWeatherConditionsSummaries() const { return m_weatherConditonSummaries; }
    virtual const WeatherConditions* GetWeatherConditions() const  { return m_weatherConditons; }
    virtual const std::vector<WeatherForecast*> GetWeatherForecast() const { return m_weatherForecast; }
    virtual const PlaceEventCookie* GetPlaceEventCookie() const {return m_cookie;}

    std::string m_name;
    Location* m_location;
    std::vector<Phone*> m_phones;
    std::vector<Category*> m_categories;
    std::vector<Event*> m_events;
    std::string* m_theaterId;
    std::vector<WeatherConditionsSummary*> m_weatherConditonSummaries;
    OnboardWeatherConditionsImpl* m_weatherConditons;
    std::vector<WeatherForecast*> m_weatherForecast;
    OnboardPlaceEventCookieImpl* m_cookie;
};

class OnboardGoldenCookieImpl : public GoldenCookie
{
public:
    OnboardGoldenCookieImpl(const char* provideId, const char* state) : m_provideId(provideId), m_state(state) {}
    OnboardGoldenCookieImpl(const GoldenCookie& copy);
    OnboardGoldenCookieImpl(const OnboardGoldenCookieImpl& copy);
    OnboardGoldenCookieImpl(protocol::GoldenCookieSharedPtr cookie);
    virtual ~OnboardGoldenCookieImpl() {}
    virtual const std::string& GetProviderId() const { return m_provideId; }
    virtual const std::string& GetState() const { return m_state; }
    virtual const std::string& GetSerialized() const { return m_serialized; }

    static std::string Serialize(const protocol::GoldenCookieSharedPtr& placeEventCookieProtocol);
    static protocol::GoldenCookieSharedPtr Deserialize(const std::string& serializedData);

private:
    std::string m_provideId;
    std::string m_state;
    std::string m_serialized;
};

class OnboardPOIContentImpl : public POIContent
{
public:
    OnboardPOIContentImpl();
    OnboardPOIContentImpl(const POIContent& copy);
    OnboardPOIContentImpl(const OnboardPOIContentImpl& copy);
    OnboardPOIContentImpl(protocol::PoiContentSharedPtr poiContent);
    virtual ~OnboardPOIContentImpl();

    virtual const std::vector<POIKey> GetKeys() const;
    virtual bool GetKeyValue(POIKey poiKey, std::vector<std::string>& outValue) const;

    virtual bool GetTagLine(std::string& m_tagLine) const;
    virtual const HoursOfOperation* GetHoursOfOperation() const {return m_hourOfOperation;}
    virtual const std::vector<VendorContent*>& GetVendorContents() const {return m_vendorContents;}
    virtual const std::vector<std::string> GetKeysOfString() const;
    virtual const std::vector<std::string> GetValuesByKey(const std::string& key) const;
    virtual const std::vector<StringPair*> GetStringPairs() const;
    virtual const GoldenCookie* GetGoldenCookie() const {return m_cookie;}
    virtual const std::string& GetId() const { return m_id; }

private:
    std::string* m_tagLine;
    std::string m_id;
    OnboardHoursOfOperationImpl* m_hourOfOperation;
    std::vector<VendorContent*> m_vendorContents;
    std::vector<StringPair*> m_pairs;
    OnboardGoldenCookieImpl* m_cookie;
};

class OnboardFuelTypeImpl : public FuelType
{
public:
    OnboardFuelTypeImpl(){};
    OnboardFuelTypeImpl(const FuelType& copy);
    OnboardFuelTypeImpl(const OnboardFuelTypeImpl& copy);
    OnboardFuelTypeImpl(protocol::FuelTypeSharedPtr type);
    virtual ~OnboardFuelTypeImpl(){};

    virtual const std::string& GetProductName() const {return m_productName;}
    virtual const std::string& GetCode() const {return m_code;}
    virtual const std::string& GetTypeName() const {return m_typeName;}

private:
    std::string m_productName;
    std::string m_code;
    std::string m_typeName;
};

class OnboardPriceImpl : public Price
{
public:
    OnboardPriceImpl(){};
    OnboardPriceImpl(const Price& copy);
    OnboardPriceImpl(const OnboardPriceImpl& copy);
    OnboardPriceImpl(protocol::PriceSharedPtr price);
    virtual ~OnboardPriceImpl() {}

    virtual double GetValue() const {return m_value;}
    virtual const std::string& GetCurrency() const {return m_currency;}
    virtual uint32 GetModTime() const {return m_modTime;}

private:
    double m_value;
    std::string m_currency;
    uint32 m_modTime;
};

class OnboardFuelProductImpl : public FuelProduct
{
public:
    OnboardFuelProductImpl() : m_price(NULL), m_type(NULL) {}
    OnboardFuelProductImpl(protocol::FuelProductSharedPtr product);
    OnboardFuelProductImpl(const FuelProduct& copy);
    OnboardFuelProductImpl(const OnboardFuelProductImpl& copy);
    virtual ~OnboardFuelProductImpl();

    virtual const Price* GetPrice() const {return m_price;}
    virtual const std::string& GetUnits() const {return m_units;}
    virtual const FuelType* GetFuelType() const {return m_type;}

private:
    OnboardPriceImpl* m_price;
    OnboardFuelTypeImpl* m_type;
    std::string m_units;
};

class OnboardFuelSummaryImpl : public FuelSummary
{
public:
    OnboardFuelSummaryImpl() : m_average(NULL), m_low(NULL) {}
    OnboardFuelSummaryImpl(protocol::FuelPriceSummarySharedPtr summary);
    virtual ~OnboardFuelSummaryImpl();

    virtual const FuelProduct* GetAverage() const {return m_average;}
    virtual const FuelProduct* GetLow() const {return m_low;}

private:
    OnboardFuelProductImpl* m_average;
    OnboardFuelProductImpl* m_low;
};

/*! This type provides interface for a type which describing fuel products*/
class OnboardFuelDetailsImpl : public FuelDetails
{
public:
    OnboardFuelDetailsImpl() {}
    OnboardFuelDetailsImpl(protocol::FuelProductSharedPtrList products);
    OnboardFuelDetailsImpl(const FuelDetails& copy);
    OnboardFuelDetailsImpl(const OnboardFuelDetailsImpl& copy);
    virtual ~OnboardFuelDetailsImpl();

    virtual const std::vector<FuelProduct*>& GetFuelProducts() const {return m_products;}

private:
    std::vector<FuelProduct*> m_products;
};

class OnboardResultStateImpl : public ResultState
{
public:
    OnboardResultStateImpl() {}

    OnboardResultStateImpl(const ResultState& copy);
    OnboardResultStateImpl(const OnboardResultStateImpl& copy);
    virtual ~OnboardResultStateImpl();
    virtual OnboardResultStateImpl& operator=(const ResultState& rhs);

    virtual const std::string& GetSerialized() const {return m_serialized;}

    std::string m_serialized;
};

class OnboardSuggestionMatchImpl : public SuggestionMatch
{
public:
    OnboardSuggestionMatchImpl() : m_distance(-1.0), m_type(SMT_None), m_searchFilter(NULL) {}
    OnboardSuggestionMatchImpl(const SuggestionMatch& copy);
    OnboardSuggestionMatchImpl(const OnboardSuggestionMatchImpl& copy);
    OnboardSuggestionMatchImpl(protocol::SuggestMatchSharedPtr suggestmatch);
    virtual ~OnboardSuggestionMatchImpl();

    virtual SearchResultType GetSearchResultType() const {return SRT_SuggestMatch;}
    virtual const SearchFilter* GetSearchFilter() const {return m_searchFilter;}
    virtual double GetDistance() const {return m_distance;}
    virtual const std::string& GetLine1() const {return m_line1;}
    virtual const std::string& GetLine2() const {return m_line2;}
    virtual const std::string& GetLine3() const {return m_line3;}
    virtual const SuggestionMatchType GetMatchType() const {return m_type;}
    virtual const std::vector<std::string>& GetIconIDs() const {return m_iconIDs;}

    double m_distance;
    std::string m_line1;
    std::string m_line2;
    std::string m_line3;
    SuggestionMatchType m_type;
    std::vector<std::string> m_iconIDs;
    SearchFilter* m_searchFilter;
};

class OnboardSuggestionListImpl : public SuggestionList
{
public:
    OnboardSuggestionListImpl(const protocol::SuggestListSharedPtr& suggestList);
    OnboardSuggestionListImpl(const SuggestionList& copy);
    virtual ~OnboardSuggestionListImpl();

    virtual SearchResultType GetSearchResultType() const {return SRT_SuggestList;}
    virtual const std::string& GetName() const {return m_name;}
    virtual const std::vector<SuggestionMatch*>& GetSuggestionMatches() const {return m_suggestMatches;}

    static std::string Serialize(const protocol::SuggestListSharedPtr& suggestListProtocol);
    static protocol::SuggestListSharedPtr Deserialize(const std::string& serializedData);

private:
    std::string m_name;
    std::vector<SuggestionMatch*> m_suggestMatches;
};

class OnboardPOIImpl : public POI
{
public:
    OnboardPOIImpl(Ncdb::SearchMatch* sm);
    OnboardPOIImpl(const OnboardPOIImpl& copy);
    OnboardPOIImpl(const POI& copy);
    virtual ~OnboardPOIImpl();

    virtual SearchResultType GetSearchResultType() const {return SRT_POI;}
    virtual POIType GetPOIType() const {return m_type;}
    virtual const Place* GetPlace() const {return m_place;}
    virtual double GetDistance() const {return m_distance;}
    virtual const SearchFilter* GetSearchFilter() const {return m_searchfilter;}
    virtual const POIContent* GetPOIContent() const {return m_poicontent;}
    virtual const FuelDetails* GetFuelDetails() const {return  m_fuelDetails;}
    virtual const RelatedSearch* GetRelatedSearch() const {return m_relatedSearch;}
    virtual bool IsPremiumPlacement() const { return m_isPremiumPlacement; }
    virtual bool IsUnmappable() const { return m_isUnmappable; }
    virtual bool IsEnhancedPoi() const { return m_isEnahancedPoi; }
    virtual uint32 GetSearchQueryEventId() const { return m_searchQueryEventId; }

    POIType m_type;
    OnboardPlaceImpl* m_place;
    SearchFilter* m_searchfilter;
    OnboardPOIContentImpl* m_poicontent;
    OnboardFuelDetailsImpl* m_fuelDetails;
    OnboardRelatedSearchImpl* m_relatedSearch;
    double m_distance;
    bool m_isPremiumPlacement;
    bool m_isUnmappable;
    bool m_isEnahancedPoi;
    uint32 m_searchQueryEventId;
};

class OnboardSingleSearchInformationImpl : public SingleSearchInformation
{
public:
    OnboardSingleSearchInformationImpl() : m_type(SSIRT_None), m_more(false), m_fuelSummary(NULL), m_resultState(new OnboardResultStateImpl()) {}
    OnboardSingleSearchInformationImpl(const Ncdb::AutoArray<Ncdb::AutoSharedPtr<Ncdb::SearchMatch> >& matches, uint32 searchQueryEventId);
    virtual ~OnboardSingleSearchInformationImpl();

    virtual bool HasMore() const {return m_more;}

    virtual int GetResultCount() const {return m_results.size();}
    virtual SingleSearchInformationResultType GetResultType() const;

    virtual const SearchResultBase* GetResultAtIndex(uint32 index) const;

    virtual const POI* GetPOIAtIndex(uint32 index) const;
    virtual const SuggestionMatch* GetSuggestionMatchAtIndex(uint32 index) const;
    virtual const FuelSummary* GetFuelSummary() const;

    virtual const std::vector<ResultDescription*>& GetResultDescriptions() const;

    virtual const SuggestionList* GetSuggestionListAtIndex(uint32 index) const;

    virtual const ResultState* GetResultState() const;

    virtual const ProxMatchContent* GetProxMatchContentAtIndex(uint32 index) const;

private:
    SingleSearchInformationResultType m_type;
    bool m_more;
    std::vector<ResultDescription*> m_resultDescriptions;
    std::vector<SearchResultBase*> m_results;
    std::vector<ProxMatchContent*> m_proxMatchContents;
    OnboardFuelSummaryImpl* m_fuelSummary;
    Ncdb::AutoArray<Ncdb::AutoSharedPtr<Ncdb::SearchMatch> > m_matches;
    OnboardResultStateImpl* m_resultState;
};

std::string OnboardCreateBased64DataByTpsElement(const protocol::TpsElementPtr& tpsElem);
protocol::TpsElementPtr OnboardCreateTpsElementByBase64Data(const std::string& serializedData);
protocol::SearchFilterSharedPtr OnboardCreateSearchFilterInProtocolBySearchFilter(const SearchFilter& searchFilter);
protocol::SuggestMatchSharedPtr OnboardCreateSuggestMatchInProtocolBySuggestionMatch(const SuggestionMatch& suggestionMatch);
protocol::SuggestListSharedPtr OnboardCreateSuggestListInProtocolBySuggestionList(const SuggestionList& suggestionList);

template<typename T>
static std::vector<T> OnboardCreateDBDataVectorBySuggestionListVector(const std::vector<SuggestionList*>& allData)
{
    std::vector<T> allDBData;

    std::vector<SuggestionList*>::const_iterator suggestionListIter = allData.begin();
    for (; suggestionListIter != allData.end(); ++suggestionListIter)
    {
        T data;

        protocol::SuggestListSharedPtr suggestList(OnboardCreateSuggestListInProtocolBySuggestionList(**suggestionListIter));
        data.m_text = OnboardCreateBased64DataByTpsElement(protocol::SuggestListSerializer::serialize(suggestList));

        allDBData.push_back(data);
    }

    return allDBData;
}

template<typename T>
static std::vector<SuggestionList*> OnboardCreateSuggestionListVectorByDBDataVector(const std::vector<T>& allDBData)
{
    std::vector<SuggestionList*> allData;

    typename std::vector<T>::const_iterator iterDBItem = allDBData.begin();
    for (; iterDBItem != allDBData.end(); ++iterDBItem)
    {
        if (iterDBItem->m_text.empty())
            break;

        protocol::SuggestListSharedPtr suggestList(protocol::SuggestListSerializer::deserialize(OnboardCreateTpsElementByBase64Data(iterDBItem->m_text)));
        allData.push_back(new OnboardSuggestionListImpl(suggestList));
    }

    return allData;
}

inline std::string OnboardCreateFilterStringBySuggestMatch(std::string const& category, std::string const& name, std::string const& categoryCode)
{
    std::stringstream ss;
    ss << "type=" << category << "|name=" << name << "|code=" << categoryCode;
    return ss.str();
}

inline std::string OnboardCreateFilterStringBySuggestMatchBrand(std::string const& category, std::string const& filter, std::string const& categoryCode)
{
    std::stringstream ss;
    ss << "type=" << category << "|brand=" << filter << "|code=" << categoryCode;
    return ss.str();
}

class OnboardSuggestMatchBuilder
{
public:
    OnboardSuggestMatchBuilder(): m_suggestMatch(new protocol::SuggestMatch())
    {
        protocol::SearchFilterSharedPtr searchFilter(new protocol::SearchFilter());
        m_suggestMatch->SetSearchFilter(searchFilter);
        m_suggestMatch->SetLine1(CHAR_PTR_TO_UTF_STRING_PTR(""));
        m_suggestMatch->SetLine2(CHAR_PTR_TO_UTF_STRING_PTR(""));
        m_suggestMatch->SetLine3(CHAR_PTR_TO_UTF_STRING_PTR(""));
    }

    void AddPair(const std::string& key, const std::string& value)
    {
        protocol::PairSharedPtr pair(new protocol::Pair());
        pair->SetKey(CHAR_PTR_TO_UTF_STRING_PTR(key));
        pair->SetValue(CHAR_PTR_TO_UTF_STRING_PTR(value));
        m_suggestMatch->GetPairArray()->push_back(pair);
    }
    void AddPairOfSearchFilter(const std::string& key, const std::string& value)
    {
        protocol::PairSharedPtr pair(new protocol::Pair());
        pair->SetKey(CHAR_PTR_TO_UTF_STRING_PTR(key));
        pair->SetValue(CHAR_PTR_TO_UTF_STRING_PTR(value));
        m_suggestMatch->GetSearchFilter()->GetSearchKeyArray()->push_back(pair);
    }
    void SetResultStyleOfSearchFilter(const std::string& style)
    {
        protocol::ResultStyleSharedPtr resultStyle(new protocol::ResultStyle());
        resultStyle->SetKey(CHAR_PTR_TO_UTF_STRING_PTR(style));
        m_suggestMatch->GetSearchFilter()->SetResultStyle(resultStyle);
    }

    void SetLine1(const std::string& line1)
    {
        m_suggestMatch->SetLine1(CHAR_PTR_TO_UTF_STRING_PTR(line1));
    }

    void SetMatchType(const std::string& matchType)
    {
        m_suggestMatch->SetMatchType(CHAR_PTR_TO_UTF_STRING_PTR(matchType));
    }

    const protocol::SuggestMatchSharedPtr& Protocol() const {return m_suggestMatch;}

    static OnboardSuggestMatchBuilder CreateByMatch(std::string const& iconId, std::string const& category, std::string const& name, std::string const& categoryCode)
    {
        OnboardSuggestMatchBuilder builder;
        if (iconId.compare("MM") == 0) //movie is especial, should handled separately.
        {
            builder.AddPair("icon-id", iconId);
            builder.AddPairOfSearchFilter("name", "");
            builder.AddPairOfSearchFilter("genre", "All");
            builder.AddPairOfSearchFilter("showing", "NowInTheater");
            builder.AddPairOfSearchFilter("sort-by", "MostPopular");
            builder.AddPairOfSearchFilter("source", "movie-screen");
            builder.SetResultStyleOfSearchFilter("movie-list");
            builder.SetLine1(name);
            builder.SetMatchType(category);
        }
        else
       {
            builder.AddPair("icon-id", iconId);
            builder.AddPairOfSearchFilter("search-result-id", OnboardCreateFilterStringBySuggestMatch(category, name, categoryCode));
            builder.AddPairOfSearchFilter("source", "explore-screen");
            builder.SetResultStyleOfSearchFilter("interest");
            builder.SetLine1(name);
            builder.SetMatchType(category);
        }
        return builder;
    }

    static OnboardSuggestMatchBuilder CreateByBrand(std::string const& iconId, std::string const& category, std::string const& name, std::string const& filter, std::string const& categoryCode)
    {
        OnboardSuggestMatchBuilder builder;
        builder.AddPair("icon-id", iconId);
        builder.AddPairOfSearchFilter("search-result-id", OnboardCreateFilterStringBySuggestMatchBrand(category, filter, categoryCode));
        builder.AddPairOfSearchFilter("source", "explore-screen");
        builder.SetResultStyleOfSearchFilter("interest");
        builder.SetLine1(name);
        builder.SetMatchType(category);

        return builder;
    }

private:
    protocol::SuggestMatchSharedPtr m_suggestMatch;
};

class OnboardSuggestListBuilder
{
public:
    OnboardSuggestListBuilder(): m_suggestList(new protocol::SuggestList()) {m_suggestList->SetName(CHAR_PTR_TO_UTF_STRING_PTR(""));}
    void SetName(const std::string& name) {m_suggestList->SetName(CHAR_PTR_TO_UTF_STRING_PTR(name));}
    void AddSuggestMatch(const protocol::SuggestMatchSharedPtr& suggestMatch) {m_suggestList->GetSuggestMatchArray()->push_back(suggestMatch);}
    const protocol::SuggestListSharedPtr& Protocol() const {return m_suggestList;}

private:
    protocol::SuggestListSharedPtr m_suggestList;
};

}

#endif /* __SINGLESEARCHINFORMATIONIMPL__ */
