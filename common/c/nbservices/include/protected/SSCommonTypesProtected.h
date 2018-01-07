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

    @file     CommonTypesProtected.h
*/
/*
    (C) Copyright 2012 by TeleCommunications Systems, Inc.

    The information contained herein is confidential, proprietary
    to TeleCommunication Systems, Inc., and considered a trade secret as
    defined in section 499C of the penal code of the State of
    California. Use of this information by anyone other than
    authorized employees of TeleCommunication Systems is granted only
    under a written non-disclosure agreement, expressly
    prescribing the scope and manner of such use.

---------------------------------------------------------------------------*/
#ifndef COMMONTYPESPROTECTED_H
#define COMMONTYPESPROTECTED_H

#include <map>
#include "SSCommonTypes.h"

namespace nbcommon2{

/*! Implementation of SuggestionMatch interface
    This type contains information for current suggestion. Used by the client */
class SuggestionMatchImpl : public SuggestionMatch
{
public:
    SuggestionMatchImpl();
    ~SuggestionMatchImpl();

    SuggestionMatchImpl(const SuggestionMatchImpl& objSuggestionMatch);
    SuggestionMatchImpl& operator=(const SuggestionMatchImpl& objSuggestionMatch);


    void SetFirstLine(const char* firstLine);
    void SetSecondLine(const char* secondLine);
    void SetThirdLine(const char* thirdLine);
    void SetMatchType(NB_MatchType matchType);
    void SetDistance(const double distance);
    void SetSuggestionCookie(const char* suggestionCookie);

    virtual const std::string& GetFirstLine() const;
    virtual const std::string& GetSecondLine() const;
    virtual const std::string& GetThirdLine() const;
    virtual const NB_MatchType GetMatchType() const;
    virtual const double GetDistance() const;
    virtual const std::string& GetSuggestionCookie() const;


private:
    void Initialize();

    std::string m_firstLine;                   /*!< The first line of text to be displayed for this suggestion.
                                              For POI results, this will be the name of the place. For
                                              address results it will be the address text as formatted by
                                              the server.*/
    std::string m_secondLine;                  /*!< The second line of text to be displayed for this suggestion,
                                              if present. For POI results this will be the address of the
                                              place, pre-formatted as text by the server. */
    std::string m_thirdLine;                    /*!< The third line of text to be displayed for this suggestion,
                                                   if present. For POI results this will be the address of the
                                                   place, city,state & zipcode pre-formatted as text by the server. */
    NB_MatchType m_matchType;            /*!< Identifies the type of result indicated by the suggestion.
                                              Used by the client for showing the appropriate icons. */

    double m_distance;                      /*!<  Get the distance of the suggestion.
                                              Used by the client to get the distance of the suggestion. */

    std::string m_suggestionCookie;         /*!<  Contains all the information of the suggestion.
                                              Used by the client to get the information of the suggestion. */

};

/*! Implementation of Location interface
    Main type describing a location.
    The value of the `type' field describes where to find the location
    information in the other fields. */
class LocationImpl : public Location
{
public:
    LocationImpl();
    ~LocationImpl();

    LocationImpl& operator=(const LocationImpl& objLocation);
    LocationImpl(const LocationImpl& location);

    void Initialize();

    void SetType(LocationType type);
    void SetCoordinates(const LatitudeLongitude* coords);
    void SetName(const char* name);
    void SetNumber(const char* number);
    void SetStreet(const char* street);
    void SetCrossStreet(const char* crossStreet);
    void SetCity(const char* city);
    void SetCounty(const char* county);
    void SetState(const char* state);
    void SetZipCode(const char* zipCode);
    void SetCountry(const char* country);
    void SetAirport(const char* airport);
    void SetCountryName(const char* countryName);
    void SetExtendedAddress(const ExtendedAddress* extendedAddress);
    void SetCompactAddress(const CompactAddress* compactAddress);

    virtual const LocationType GetType(void) const;
    virtual LatitudeLongitude* const GetCoordinates(void) const;
    virtual const std::string& GetName(void) const;
    virtual const std::string& GetNumber(void) const;
    virtual const std::string& GetStreet(void) const;
    virtual const std::string& GetCrossStreet(void) const;
    virtual const std::string& GetCity(void) const;
    virtual const std::string& GetCounty(void) const;
    virtual const std::string& GetState(void) const;
    virtual const std::string& GetZipCode(void) const;
    virtual const std::string& GetCountry(void) const;
    virtual const std::string& GetAirport(void) const;
    virtual const std::string& GetCountryName(void) const;
    virtual const ExtendedAddress& GetExtendedAddress(void) const;
    virtual const CompactAddress& GetCompactAddress(void) const;

private:
    std::string  m_name;
    std::string  m_number;
    std::string  m_street;
    std::string  m_crossStreet;
    std::string  m_city;
    std::string  m_county;
    std::string  m_state;
    std::string  m_zipCode;
    std::string  m_country;
    std::string  m_airport;
    std::string  m_countryName;
    LatitudeLongitude* m_coords;
    ExtendedAddress* m_extendedAddress;
    CompactAddress* m_compactAddress;
    LocationType m_type;
};

/*! Implementation of Category interface
    This type contains pair of values - name and code */
class CategoryImpl : public Category
{
public:
    CategoryImpl();
    CategoryImpl(const char* code, const char* name);
    ~CategoryImpl();

    CategoryImpl(const CategoryImpl& objCategoryImpl);
    CategoryImpl& operator=(const CategoryImpl& objCategoryImpl);

    void SetCode(const char* code);
    void SetName(const char* name);

    virtual const std::string& GetCode() const;
    virtual const std::string& GetName() const;

private:
    void Initialize();

    std::string m_code;
    std::string m_name;
};

/*! Implementation of Phone interface
    This type describes a phone contact */
class PhoneImpl : public Phone
{
public:
    PhoneImpl();
    ~PhoneImpl();

    PhoneImpl(const PhoneImpl& objPhoneImpl);
    PhoneImpl& operator=(const PhoneImpl& objPhoneImpl);

    void SetPhoneType(PhoneType type);
    void SetCountry(const char* country);
    void SetArea(const char* area);
    void SetNumber(const char* number);
    void SetFormattedNumber(const char* formattedNumber);

    virtual const PhoneType GetPhoneType() const;
    virtual const std::string& GetCountry() const;
    virtual const std::string& GetArea() const;
    virtual const std::string& GetNumber() const;
    virtual const std::string& GetFormattedNumber() const;

private:
    void Initialize();

    PhoneType m_phoneType;  /*!< Kind of phone contact */
    std::string m_country;        /*!< Country code */
    std::string m_area;           /*!< Area code */
    std::string m_number;         /*!< Actual phone number */
    std::string m_formattedNumber; /*!< Formatted phone number with counrty and area code */
};

/*! Implementation of Address interface
    This type describes all possible information necessary to geocode an address*/
class AddressImpl : public Address
{
public:
    AddressImpl();
    ~AddressImpl();

    AddressImpl(const AddressImpl& addr);
    AddressImpl& operator=(const AddressImpl& objAddress);

    void SetNumber(const char* number);
    void SetStreet(const char* street);
    void SetCity(const char* city);
    void SetCounty(const char* county);
    void SetState(const char* state);
    void SetZipCode(const char* zipCode);
    void SetCountry(const char* country);

    virtual const std::string& GetNumber(void) const;
    virtual const std::string& GetStreet(void) const;
    virtual const std::string& GetCity(void) const;
    virtual const std::string& GetCounty(void) const;
    virtual const std::string& GetState(void) const;
    virtual const std::string& GetZipCode(void) const;
    virtual const std::string& GetCountry(void) const;

private:
    void Initialize();

    std::string  m_number;
    std::string  m_street;
    std::string  m_city;
    std::string  m_county;
    std::string  m_state;
    std::string  m_zipCode;
    std::string  m_country;
};

/*! Implementation of BoundingBox interface
    This type contains pair of cooordinates - topLeft and BottomRight */
class BoundingBoxImpl : public BoundingBox
{
public:
    BoundingBoxImpl();
    ~BoundingBoxImpl();

    BoundingBoxImpl(const BoundingBoxImpl& objBoundingBoxImpl);
    BoundingBoxImpl& operator=(const BoundingBoxImpl& objBoundingBoxImpl);

    void SetTopLeftCoordinates(LatitudeLongitude* coords);
    void SetBottomRightCoordinates(LatitudeLongitude* coords);

    virtual LatitudeLongitude* const GetTopLeftCoordinates() const;
    virtual LatitudeLongitude* const GetBottomRightCoordinates() const;

private:
    void Initialize();

    LatitudeLongitude* m_topLeftCoords;
    LatitudeLongitude* m_bottomRightCoords;
};

/*! Implementation of TimeRange interface. */
class TimeRangeImpl : public TimeRange
{
public:
    TimeRangeImpl();
    ~TimeRangeImpl();

    TimeRangeImpl(const TimeRangeImpl& objTimeRangeImpl);
    TimeRangeImpl& operator=(const TimeRangeImpl& objTimeRangeImpl);

    void SetStartHour(short startHour);
    void SetStartMinute(short startMin);
    void SetEndHour(short endHour);
    void SetEndMinute(short endMin);
    void SetDayOfWeek(NB_DayOfWeek dayOfWeek);

    virtual const short GetStartHour() const;
    virtual const short GetStartMinute() const;
    virtual const short GetEndHour() const;
    virtual const short GetEndMinute() const;
    virtual const NB_DayOfWeek GetDayOfWeek() const;

private:
    void Initialize();

    short m_startHour;
    short m_startMin;
    short m_endHour;
    short m_endMin;
    NB_DayOfWeek m_dayOfWeek;
};

/*! Implementation of Place interface
    This type describes a mapped place or or point of interest */
class PlaceImpl : public Place
{
public:
    PlaceImpl();
    ~PlaceImpl();

    PlaceImpl(const PlaceImpl& objPlace);
    PlaceImpl& operator=(const PlaceImpl& objPlace);

    void SetName(const char* name);
    void SetLocation(const LocationImpl* location);
    void SetPhone(PhoneImpl* phonenumber);
    void SetPhoneNumbers(const std::vector<PhoneImpl*>& phoneNumbers);
    void SetCategories(const std::map<CategoryPtrWrapper, std::vector<CategoryPtrWrapper> >& categories);
    void SetBoundingBox(const BoundingBoxImpl* boundingBox);
    void SetAverageRating(double averageRating);
    void SetRatingCount(double ratingCount);
    void SetKeyValues(LS_POIKeys key, const std::string& value);
    void SetUtcOffset(int32 utcOffset);
    void SetTimeRanges(const std::vector<TimeRangeImpl*>& timeRanges);
    void SetUtcOffsetValidityFlag(bool hasUtcOffset);

    virtual const std::string& GetName() const;
    virtual Location* const GetLocation() const;
    virtual const std::vector<Phone*>& GetPhoneNumbers() const;
    virtual const std::map<CategoryPtrWrapper, std::vector<CategoryPtrWrapper> >& GetCategories() const;
    virtual BoundingBox* const GetBoundingBox() const;
    virtual const std::vector<LS_POIKeys>& GetKeys() const;
    virtual void GetKeyValue(LS_POIKeys poiKey, std::string& outValue);
    virtual const double GetAverageRating() const;
    virtual const double GetRatingCount() const;
    virtual const std::map<LS_POIKeys, std::string>& GetKeyValuePair() const;
    virtual const std::vector<TimeRange*>& GetTimeRange() const;
    virtual const bool GetUtcOffset(int32& utcOffset) const;

private:
    void Initialize();

    std::string m_name;                         /*!< Name of the place */
    Location* m_location;                       /*!< Location of the place */
    std::vector<Phone*> m_phoneNumbers;         /*!< All the phones of this place */
    std::map<CategoryPtrWrapper, std::vector<CategoryPtrWrapper> > m_categories;        /*!< Category of this point of interest */
    std::map<LS_POIKeys, std::string> m_keyValues; /*< external values mapped with LS_POIKeys*/
    BoundingBox* m_boundingBox;                 /*!<  Get the bounding box of the place */
    double m_averageRating;                     /*!<  Get the average rating for the place */
    double m_ratingCount;                       /*!<  Get the rating count of the place */
    std::vector<LS_POIKeys> m_poiKeys;          /*!<  Get the category keys of the place */
    std::vector<TimeRange*> m_timeRange;        /*!<  Get the opening hours range of the place */
    int32 m_utcOffset;                          /*!<  Get the utc-offset of the place */
    bool m_hasUtcOffset;                        /*!<  A validity flag of utc-offset */
};
/*! Implementation of FuelType interface. */
class FuelTypeImpl : public FuelType
{
public:
    FuelTypeImpl();
    ~FuelTypeImpl();

    FuelTypeImpl(const FuelTypeImpl& objFuelTypeImpl);
    FuelTypeImpl& operator=(const FuelTypeImpl& objFuelTypeImpl);

    void SetProductName(const char* productName);
    void SetCode(const char* code);
    void SetTypeName(const char* typeName);

    virtual const std::string& GetProductName() const;
    virtual const std::string& GetCode() const;
    virtual const std::string& GetTypeName() const;

private:
    void Initialize();
    std::string m_productName;   /*!< Name of the actual fuel product as sold by providers */
    std::string m_code;          /*!< Internal code for this fuel type */
    std::string m_typeName;      /*!< The type of the fuel regardless of the provider */
};
/*! Implementation of Price interface. */
class PriceImpl : public Price
{
public:
    PriceImpl();
    ~PriceImpl();

    PriceImpl(const PriceImpl& objPriceImpl);
    PriceImpl& operator=(const PriceImpl& objPriceImpl);

    void SetValue(double value);
    void SetCurrency(const char* currency);
    void SetModTime(uint32 modTime);

    virtual const double GetValue() const;
    virtual const std::string& GetCurrency() const;
    virtual const uint32 GetModTime() const;
private:
    void Initialize();
    double m_value;
    std::string m_currency;
    uint32 m_modTime;
};

/*! Implementation of FuelProduct interface. */
class FuelProductImpl : public FuelProduct
{
public:
    FuelProductImpl();
    ~FuelProductImpl();

    FuelProductImpl(const FuelProductImpl& objFuelProductImpl);
    FuelProductImpl& operator=(const FuelProductImpl& objFuelProductImpl);

    void SetPrice(const PriceImpl* price);
    void SetUnits(const char* units);
    void SetFuelType(const FuelTypeImpl* fuelType);

    virtual Price* const GetPrice() const;
    virtual const std::string& GetUnits() const;
    virtual FuelType* const GetFuelType() const;
private:
    void Initialize();
    Price* m_price;
    std::string m_units;
    FuelType* m_fuelType;
};
/*! Implementation of FuelSummary interface. */
class FuelSummaryImpl : public FuelSummary
{
public:
    FuelSummaryImpl();
    ~FuelSummaryImpl();

    FuelSummaryImpl(const FuelSummaryImpl& objFuelSummaryImpl);
    FuelSummaryImpl& operator=(const FuelSummaryImpl& objFuelSummaryImpl);

    void SetAverage(const FuelProduct* average);
    void SetLow(const FuelProduct* low);

    virtual FuelProduct* const GetAverage() const;
    virtual FuelProduct* const GetLow() const;
private:
    void Initialize();
    FuelProduct* m_average;
    FuelProduct* m_low;
};
/*! Implementation of FuelDetails interface. */
class FuelDetailsImpl : public FuelDetails
{
public:
    FuelDetailsImpl();
    ~FuelDetailsImpl();

    FuelDetailsImpl(const FuelDetailsImpl& objFuelDetailsImpl);
    FuelDetailsImpl& operator=(const FuelDetailsImpl& objFuelDetailsImpl);

    void SetFuelProduct(int index, const FuelProductImpl* fuelProduct);
    void SetNumFuelProducts(int numFuelProducts);

    virtual FuelProduct* const GetFuelProduct(int index) const;
    virtual const int GetNumFuelProducts() const;
private:
    void Initialize();
    FuelProduct* m_fuelProduct[NB_FUEL_PRODUCTS_NUM];
    int m_numFuelProducts;
};

/*! Implementation of FuelCommon interface. */
class FuelCommonImpl : public FuelCommon
{
public:
    FuelCommonImpl();
    FuelCommonImpl(const FuelDetailsImpl* fuelDetails);
    FuelCommonImpl(const FuelSummaryImpl* fuelSummary);
    ~FuelCommonImpl();

    FuelCommonImpl(const FuelCommonImpl& objFuelCommonImpl);
    FuelCommonImpl& operator=(const FuelCommonImpl& objFuelCommonImpl);

    void SetFuelDetails(const FuelDetailsImpl* fuelDetails);
    void SetFuelSummary(const FuelSummaryImpl* fuelSummary);

    virtual FuelDetails* const GetFuelDetails() const;
    virtual FuelSummary* const GetFuelSummary() const;

private:
    void Initialize();
    FuelDetails* m_fuelDetails;
    FuelSummary* m_fuelSummary;
};

/*! Implementation of ExtendedAddress interface. */
class ExtendedAddressImpl : public ExtendedAddress
{
public:
    ExtendedAddressImpl();
    ~ExtendedAddressImpl();

    ExtendedAddressImpl(const ExtendedAddressImpl& objExtendedAddressImpl);
    ExtendedAddressImpl& operator=(const ExtendedAddressImpl& objExtendedAddressImpl);

    void SetFirstLine(const char* firstLine);
    void SetSecondLine(const char* secondLine);
    void SetThirdLine(const char* thirdLine);
    void SetFourthLine(const char* fourthLine);
    void SetFifthLine(const char* fifthLine);
    void SetLine(uint index, const char* line);     /*!< This function will append new elments to fit the index if
                                                         index is larger than the current vector buffer size*/

    virtual const std::vector<std::string>& GetLines() const;

private:
    void Initialize();

    std::vector<std::string> m_vecBuffer;      /*!< The vector contained all 5 lines*/
};

/*! Implementation of CompactAdress interface. */
class CompactAddressImpl : public CompactAddress
{
public:
    CompactAddressImpl();
    ~CompactAddressImpl();

    CompactAddressImpl(const CompactAddressImpl& objCompactAddressImpl);
    CompactAddressImpl& operator=(const CompactAddressImpl& objCompactAddressImpl);

    void SetFirstLine(const char* firstLine);
    void SetSecondLine(const char* secondLine);
    void SetLineSeparator(const char* seprator);

    virtual const std::string& GetLineSeparator() const;
    virtual const std::string& GetLine1() const;
    virtual const std::string& GetLine2() const;
    virtual const std::string& GetAddressLine() const;

private:
    void Initialize();

    std::string m_firstLine;               /*!< The first line of Address to be displayed. */
    std::string m_secondLine;              /*!< The second line of Address to be displayed. */
    std::string m_lineSeparator;           /*!< The separator between 2 lines . */
    std::string m_formattedAddress;		   /*!< The formatted address. */
};

}
#endif
