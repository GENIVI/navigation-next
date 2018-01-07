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

    @file     CommonTypes.h
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
#ifndef NBCOMMONTYPES_H
#define NBCOMMONTYPES_H

#include "base.h"
#include <vector>
#include <string>

extern "C"
{
#include "nbsearchtypes.h"
#include "nbsuggestmatch.h"
}

namespace nbcommon{

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
/*! invocation context. */
typedef struct
{
    std::string sourceModule;
    std::string inputSource;
    std::string invocationMethod;
    std::string screenID;
} InvocationContext;
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

    double GetLatitude() const;
    double GetLongitude() const;

private:
    void   Initialize();

    double      m_latitude;
    double      m_longitude;
};

/*! This type provides interface to pair of values - name and code */
class Category : public Base
{
public:
    virtual ~Category(){}

    virtual const char* GetCode() const = 0;
    virtual const char* GetName() const = 0;
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

    virtual ~Phone(){}

    virtual PhoneType GetPhoneType() const = 0;
    virtual const char* GetCountry() const = 0;
    virtual const char* GetArea() const = 0;
    virtual const char* GetNumber() const = 0;
};

/*! This type provides interface which describes all possible information necessary to geocode an address*/
class Address : public Base
{
public:
    virtual ~Address(){}

    virtual const char* GetNumber(void) const = 0;
    virtual const char* GetStreet(void) const = 0;
    virtual const char* GetCity(void) const = 0;
    virtual const char* GetCounty(void) const = 0;
    virtual const char* GetState(void) const = 0;
    virtual const char* GetZipCode(void) const = 0;
    virtual const char* GetCountry(void) const = 0;
};

/*! This type provides interface which contains information for current suggestion. Used by the client */
class SuggestionMatch
{
public:
    virtual ~SuggestionMatch(){}

    virtual const char* GetFirstLine() const = 0;
    virtual const char* GetSecondLine() const = 0;
    virtual const char* GetThirdLine() const = 0;
    virtual NB_MatchType GetMatchType() const = 0;
    virtual double GetDistance() const = 0;
    virtual const char* GetSuggestionCookie() const = 0;
};

/*! Formatted address line, max 5 lines return by server.
 */
class ExtendedAddress
{
public:
    virtual ~ExtendedAddress() {}

    virtual const std::vector<std::string> & GetLines() const = 0;
};

/*! Formatted address line, max 2 lines return by server.
 */
class CompactAddress
{
public:
    virtual ~CompactAddress() {}

    virtual const std::string& GetLineSeparator() const = 0;
    virtual const std::string& GetLine1() const = 0;
    virtual const std::string& GetLine2() const = 0;
    virtual const std::string& GetAddressLine() const = 0;
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

    virtual ~Location(){}

    virtual LocationType GetType(void) const = 0;
    virtual LatitudeLongitude* GetCoordinates(void) const = 0;
    virtual const char* GetName(void) const = 0;
    virtual const char* GetNumber(void) const = 0;
    virtual const char* GetStreet(void) const = 0;
    virtual const char* GetCrossStreet(void) const = 0;
    virtual const char* GetCity(void) const = 0;
    virtual const char* GetCounty(void) const = 0;
    virtual const char* GetState(void) const = 0;
    virtual const char* GetZipCode(void) const = 0;
    virtual const char* GetCountry(void) const = 0;
    virtual const char* GetAirport(void) const = 0;
    virtual const char* GetFreeForm(void) const = 0;
    virtual float GetUTCOffset(void) const = 0;
    virtual const ExtendedAddress* GetExtendedAddress(void) const = 0;
    virtual const CompactAddress* GetCompactAddress(void) const = 0;
};

/*! This type provides interface to values of bounding box - topleft and bottomright */
class BoundingBox : public Base
{
public:
    virtual ~BoundingBox(){}

	virtual LatitudeLongitude* GetTopLeftCoordinates(void) const = 0;
	virtual LatitudeLongitude* GetBottomRightCoordinates(void) const = 0;
};


/*! This type provides interface for a mapped place or or point of interest */
class Place : public Base
{
public:
    virtual ~Place(){}

    virtual const char* GetName() const = 0;
    virtual Location* GetLocation() const = 0;
    virtual Phone* GetPhone() const = 0;
    virtual int GetNumPhone() const = 0;
    virtual Category* GetCategory() const = 0;
    virtual int GetNumCategory() const = 0;
    virtual BoundingBox* GetBoundingBox() const = 0;
};

/*! This type provides interface for a type which describing information for different kinds of fuels */
class FuelType : public Base
{
public:
    virtual ~FuelType(){}

    virtual char* GetProductName() const = 0; /*!< Name of the actual fuel product as sold by providers */
    virtual char* GetCode() const = 0;        /*!< Internal code for this fuel type */
    virtual char* GetTypeName() const = 0;    /*!< The type of the fuel regardless of the provider */
};

/*! This type provides interface for a type which describing price information. */
class Price : public Base
{
public:
    virtual ~Price(){}

    virtual double GetValue() const = 0;
    virtual char* GetCurrency() const = 0;
    virtual uint32 GetModTime() const = 0;
};

/*! This type provides interface for a type which describing a fuel product as found in various locations */
class FuelProduct : public Base
{
public:
    virtual ~FuelProduct(){}

    virtual Price* GetPrice() const = 0;
    virtual char* GetUnits() const = 0;
    virtual FuelType* GetFuelType() const = 0;
};

/*! This type provides interface for a type which describing summary information for a given fuel product */
class FuelSummary: public Base
{
public:
    virtual ~FuelSummary(){}

    virtual FuelProduct* GetAverage() const = 0;
    virtual FuelProduct* GetLow() const = 0;
};

/*! This type provides interface for a type which describing fuel products*/
class FuelDetails : public Base
{
public:
    virtual ~FuelDetails(){}

    virtual FuelProduct* GetFuelProduct(int index) const = 0;
    virtual int GetNumFuelProducts() const = 0;
};

/*! This type provides interface for a type which  contain fuel details and fuel summary information*/
class FuelCommon : public Base
{
public:
    virtual ~FuelCommon(){}

    virtual FuelDetails* GetFuelDetails() const = 0;
    virtual FuelSummary* GetFuelSummary() const = 0;
};


#ifndef INVALID_LATLON
#define INVALID_LATLON      (-999.0)
#endif

}//namespace nbcommon

#endif
