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

    @file     SSCommonTypes.h
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
#ifndef SSCOMMONTYPES_H
#define SSCOMMONTYPES_H

#include "base.h"
#include <string>
#include <vector>
#include <map>
#include "smartpointer.h"

extern "C"
{
#include "nbsearchtypes.h"
#include "nbsuggestmatch.h"
}

namespace nbcommon2{

/*!< Match type for the result of suggestion search */
enum LS_POIKeys
{
    LS_PK_description = 0,
    LS_PK_cuisines = 1,
    LS_PK_features = 2,
    LS_PK_hoursOfOperation =3,
    LS_PK_parking = 4 ,
    LS_PK_price = 5,
    LS_PK_paymentMethod = 6,
    LS_PK_reservations = 7,
    LS_PK_siteUrl = 8,
    LS_PK_specialFeatures = 9,
    LS_PK_thumbnailPhotoUrl = 10

    /*!< Category match type */
};


typedef enum {
    SS_User = 0,                                 /*!< No search source */
    SS_Address,                                  /*!< Address search source */
    SS_Carousel,                                 /*!< Carousel search source */
    SS_Map,                                      /*!< Map search source */
    SS_Place                                     /*!< Place search source */
} SingleSearchSource;
typedef enum
{
    ST_None = 0,
    ST_Center,                                   /*!< Use the center point for the search */
    ST_RouteAndCenter,                           /*!< Use the route and the center point for the search */
    ST_DirectionAndCenter,                       /*!< Use the direction and the center point for the search */
    ST_BoundingBox                               /*!< Use the bounding box for the search */
} SearchRegionType;
/* This type contains point latitude and longitude */
class LatitudeLongitude : public Base
{
public:

    LatitudeLongitude();
    LatitudeLongitude(double latitude, double longitude);
    ~LatitudeLongitude();

    LatitudeLongitude(const LatitudeLongitude& objLatitudeLongitude);
    LatitudeLongitude& operator=(const LatitudeLongitude& objLatitudeLongitude);


    void SetLatitude(double latitude);
    void SetLongitude(double longitude);

    const double GetLatitude() const;
    const double GetLongitude() const;

private:
    void   Initialize();

    double      m_latitude;
    double      m_longitude;
};

/*! This type provides interface to pair of values - name and code */
class Category : public Base
{
public:
    virtual ~Category(){};

    virtual const std::string& GetCode() const = 0;
    virtual const std::string& GetName() const = 0;
};

/*! An utility to wrap a Category raw pointer for convenience in std::map. */
class CategoryPtrWrapper
{
public:
    CategoryPtrWrapper(Category* category): m_category(category) {}

    shared_ptr<Category> const& p() const { return m_category; }
    shared_ptr<Category>&       p()       { return m_category; }

    bool operator==(CategoryPtrWrapper const& r) const
    {
        if (this == &r)
            return true;

        if (m_category->GetCode() == r.p()->GetCode())
            return true;

        return false;
    }

    bool operator<(CategoryPtrWrapper const& r) const
    {
        if (this == &r)
            return false;

        if (m_category->GetCode() < r.p()->GetCode())
            return true;

        return false;
    }

private:
    shared_ptr<Category> m_category;
};

/*! This type provides interface to access a phone contact */
class Phone : public Base
{
public:
    typedef enum
    {
        Phone_Primary = 0,
        Phone_Secondary,
        Phone_National,
        Phone_Mobile,
        Phone_Fax
    } PhoneType;

    virtual ~Phone(){};

    virtual const PhoneType GetPhoneType() const = 0;
    virtual const std::string& GetCountry() const = 0;
    virtual const std::string& GetArea() const = 0;
    virtual const std::string& GetNumber() const = 0;
    virtual const std::string& GetFormattedNumber() const = 0;
};

/*! This type provides interface which describes all possible information necessary to geocode an address*/
class Address : public Base
{
public:
    virtual ~Address(){};

    virtual const std::string& GetNumber(void) const = 0;
    virtual const std::string& GetStreet(void) const = 0;
    virtual const std::string& GetCity(void) const = 0;
    virtual const std::string& GetCounty(void) const = 0;
    virtual const std::string& GetState(void) const = 0;
    virtual const std::string& GetZipCode(void) const = 0;
    virtual const std::string& GetCountry(void) const = 0;
};

class ExtendedAddress : public Base
{
public:
    virtual ~ExtendedAddress(){};

    virtual const std::vector<std::string>& GetLines() const = 0;
};

class CompactAddress : public Base
{
public:
    virtual ~CompactAddress(){};

    virtual const std::string& GetLineSeparator() const = 0;
    virtual const std::string& GetLine1() const = 0;
    virtual const std::string& GetLine2() const = 0;
    virtual const std::string& GetAddressLine() const = 0;


};

/*! This type provides interface which contains information for current suggestion. Used by the client */
class SuggestionMatch
{
public:
    virtual ~SuggestionMatch(){};

    virtual const std::string& GetFirstLine() const = 0;
    virtual const std::string& GetSecondLine() const = 0;
    virtual const std::string& GetThirdLine() const = 0;
    virtual const NB_MatchType GetMatchType() const = 0;
    virtual const double GetDistance() const = 0;
    virtual const std::string& GetSuggestionCookie() const = 0;
};

/*! Main type which provide interface to describe location.

    The value of the `type' field describes where to find the location
    information in the other fields.
 */
class Location : public Base
{
public:
    typedef enum {
        Location_None = 0,             /*!< No location information available */
        Location_Address,              /*!< Address as in Street number, Street, City, State, ZIP code */
        Location_Intersection,         /*!< Intersection between Street 1, Street 2 at City, State, ZIP code */
        Location_Airport,              /*!< Airport name */
        Location_LatLon,               /*!< Latitude, Longitude */
        Location_MyGPS,                /*!< GPS location */
        Location_Route,                /*!< Location information along a route */
        Location_RouteGPS,             /*!< GPS location, along a route */
        Location_InMyDirection,        /*!< To indicate a location in my direction */
        Location_AddressFreeForm       /*!< Address as in Street number, Street, City, State, ZIP code in free form*/
    } LocationType;

    virtual ~Location(){};

    virtual const LocationType GetType(void) const = 0;
    virtual LatitudeLongitude* const GetCoordinates(void) const = 0;
    virtual const std::string& GetName(void) const = 0;
    virtual const std::string& GetNumber(void) const = 0;
    virtual const std::string& GetStreet(void) const = 0;
    virtual const std::string& GetCrossStreet(void) const = 0;
    virtual const std::string& GetCity(void) const = 0;
    virtual const std::string& GetCounty(void) const = 0;
    virtual const std::string& GetState(void) const = 0;
    virtual const std::string& GetZipCode(void) const = 0;
    virtual const std::string& GetCountry(void) const = 0;
    virtual const std::string& GetAirport(void) const = 0;
    virtual const std::string& GetCountryName(void) const = 0;
    virtual const ExtendedAddress& GetExtendedAddress(void) const = 0;
    virtual const CompactAddress& GetCompactAddress(void) const = 0;
};

/*! This type provides interface to values of bounding box - topleft and bottomright */
class BoundingBox : public Base
{
public:
    virtual ~BoundingBox(){};

    virtual LatitudeLongitude* const GetTopLeftCoordinates(void) const = 0;
    virtual LatitudeLongitude* const GetBottomRightCoordinates(void) const = 0;
};

class TimeRange : public Base
{
public:
    virtual ~TimeRange(){};

    virtual const short GetStartHour() const = 0;
    virtual const short GetStartMinute() const = 0;
    virtual const short GetEndHour() const = 0;
    virtual const short GetEndMinute() const = 0;
    virtual const NB_DayOfWeek GetDayOfWeek() const = 0;

};

/*! This type provides interface for a mapped place or or point of interest */
class Place : public Base
{
public:
    virtual ~Place(){};

    virtual const std::string& GetName() const = 0;
    virtual Location* const GetLocation() const = 0;
    virtual const std::vector<Phone*>& GetPhoneNumbers() const = 0;
    virtual const std::map<CategoryPtrWrapper, std::vector<CategoryPtrWrapper> >& GetCategories() const = 0;
    virtual BoundingBox* const GetBoundingBox() const = 0;
    virtual const std::vector<LS_POIKeys>& GetKeys() const = 0;
    virtual void GetKeyValue(LS_POIKeys  poiKey, std::string& outValue) = 0;
    virtual const double GetAverageRating() const = 0;
    virtual const double GetRatingCount() const = 0;
    virtual const std::map<LS_POIKeys, std::string>& GetKeyValuePair() const = 0;
    virtual const std::vector<TimeRange*>& GetTimeRange() const = 0;
    virtual const bool GetUtcOffset(int32& utcOffset) const = 0;
};

/*! This type provides interface for a type which describing information for different kinds of fuels */
class FuelType : public Base
{
public:
    virtual ~FuelType(){};

    virtual const std::string& GetProductName() const = 0; /*!< Name of the actual fuel product as sold by providers */
    virtual const std::string& GetCode() const = 0;        /*!< Internal code for this fuel type */
    virtual const std::string& GetTypeName() const = 0;    /*!< The type of the fuel regardless of the provider */
};

/*! This type provides interface for a type which describing price information. */
class Price : public Base
{
public:
    virtual ~Price(){};

    virtual const double GetValue() const = 0;
    virtual const std::string& GetCurrency() const = 0;
    virtual const uint32 GetModTime() const = 0;
};

/*! This type provides interface for a type which describing a fuel product as found in various locations */
class FuelProduct : public Base
{
public:
    virtual ~FuelProduct(){};

    virtual Price* const GetPrice() const = 0;
    virtual const std::string& GetUnits() const = 0;
    virtual FuelType* const GetFuelType() const = 0;
};

/*! This type provides interface for a type which describing summary information for a given fuel product */
class FuelSummary: public Base
{
public:
    virtual ~FuelSummary(){};

    virtual FuelProduct* const GetAverage() const = 0;
    virtual FuelProduct* const GetLow() const = 0;
};

/*! This type provides interface for a type which describing fuel products*/
class FuelDetails : public Base
{
public:
    virtual ~FuelDetails(){};

    virtual FuelProduct* const GetFuelProduct(int index) const = 0;
    virtual const int GetNumFuelProducts() const = 0;
};

/*! This type provides interface for a type which  contain fuel details and fuel summary information*/
class FuelCommon : public Base
{
public:
    virtual ~FuelCommon(){};

    virtual FuelDetails* const GetFuelDetails() const = 0;
    virtual FuelSummary* const GetFuelSummary() const = 0;
};


#define INVALID_LATLON      -999
}//namespace nbcommon

#endif
