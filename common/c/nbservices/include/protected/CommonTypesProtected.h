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

#include "CommonTypes.h"
namespace nbcommon{

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

    virtual const char* GetFirstLine() const;
    virtual const char* GetSecondLine() const;
    virtual const char* GetThirdLine() const;
    virtual NB_MatchType GetMatchType() const;
    virtual double GetDistance() const;
    virtual const char* GetSuggestionCookie() const;


private:
    void Initialize();

    char* m_firstLine;                   /*!< The first line of text to be displayed for this suggestion.
                                              For POI results, this will be the name of the place. For
                                              address results it will be the address text as formatted by
                                              the server.*/
    char* m_secondLine;                  /*!< The second line of text to be displayed for this suggestion,
                                              if present. For POI results this will be the address of the
                                              place, pre-formatted as text by the server. */
    char* m_thirdLine;					/*!< The third line of text to be displayed for this suggestion,
                                                   if present. For POI results this will be the address of the
                                                   place, city,state & zipcode pre-formatted as text by the server. */
    NB_MatchType m_matchType;            /*!< Identifies the type of result indicated by the suggestion.
                                              Used by the client for showing the appropriate icons. */

    double m_distance;					/*!<  Get the distance of the suggestion.
                                              Used by the client to get the distance of the suggestion. */

    char* m_suggestionCookie;			/*!<  Contains all the information of the suggestion.
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
    void SetCoordinates(LatitudeLongitude* coords);
    void SetName(char* name);
    void SetNumber(char* number);
    void SetStreet(char* street);
    void SetCrossStreet(char* crossStreet);
    void SetCity(char* city);
    void SetCounty(char* county);
    void SetState(char* state);
    void SetZipCode(char* zipCode);
    void SetCountry(char* country);
    void SetAirport(char* airport);
    void SetFreeForm(char* freeForm);

    virtual LocationType GetType(void) const;
    virtual LatitudeLongitude* GetCoordinates(void) const;
    virtual const char* GetName(void) const;
    virtual const char* GetNumber(void) const;
    virtual const char* GetStreet(void) const;
    virtual const char* GetCrossStreet(void) const;
    virtual const char* GetCity(void) const;
    virtual const char* GetCounty(void) const;
    virtual const char* GetState(void) const;
    virtual const char* GetZipCode(void) const;
    virtual const char* GetCountry(void) const;
    virtual const char* GetAirport(void) const;
    virtual const char* GetFreeForm(void) const;
    virtual float GetUTCOffset(void) const;
    virtual const ExtendedAddress* GetExtendedAddress(void) const {return NULL;}
    virtual const CompactAddress* GetCompactAddress(void) const {return NULL;}

private:
    char*  m_name;
    char*  m_number;
    char*  m_street;
    char*  m_crossStreet;
    char*  m_city;
    char*  m_county;
    char*  m_state;
    char*  m_zipCode;
    char*  m_country;
    char*  m_airport;
    char*  m_freeForm;
    float  m_UTCOffset;
    LatitudeLongitude* m_coords;
    LocationType m_type;
};

/*! Implementation of Category interface
    This type contains pair of values - name and code */
class CategoryImpl : public Category
{
public:
    CategoryImpl();
    ~CategoryImpl();

    CategoryImpl(const CategoryImpl& objCategoryImpl);
    CategoryImpl& operator=(const CategoryImpl& objCategoryImpl);

    void SetCode(char* code);
    void SetName(char* name);

    virtual const char* GetCode() const;
    virtual const char* GetName() const;

private:
    void Initialize();

    char* m_code;
    char* m_name;
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
    void SetCountry(char* country);
    void SetArea(char* area);
    void SetNumber(char* number);

    virtual PhoneType GetPhoneType() const;
    virtual const char* GetCountry() const;
    virtual const char* GetArea() const;
    virtual const char* GetNumber() const;

private:
    void Initialize();

    PhoneType m_phoneType;  /*!< Kind of phone contact */
    char* m_country;        /*!< Country code */
    char* m_area;           /*!< Area code */
    char* m_number;         /*!< Actual phone number */
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

    void SetNumber(char* number);
    void SetStreet(char* street);
    void SetCity(char* city);
    void SetCounty(char* county);
    void SetState(char* state);
    void SetZipCode(char* zipCode);
    void SetCountry(char* country);

    virtual const char* GetNumber(void) const;
    virtual const char* GetStreet(void) const;
    virtual const char* GetCity(void) const;
    virtual const char* GetCounty(void) const;
    virtual const char* GetState(void) const;
    virtual const char* GetZipCode(void) const;
    virtual const char* GetCountry(void) const;

private:
    void Initialize();

    char*  m_number;
    char*  m_street;
    char*  m_city;
    char*  m_county;
    char*  m_state;
    char*  m_zipCode;
    char*  m_country;
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

    virtual LatitudeLongitude* GetTopLeftCoordinates() const;
    virtual LatitudeLongitude* GetBottomRightCoordinates() const;

private:
    void Initialize();

    LatitudeLongitude* m_topLeftCoords;
    LatitudeLongitude* m_bottomRightCoords;
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

    void SetName(char* name);
    void SetLocation(LocationImpl* location);
    void SetPhone(PhoneImpl* phone);
    void SetNumPhone(int numPhone);
    void SetCategory(CategoryImpl* category);
    void SetNumCategory(int numCategory);
    void SetBoundingBox(const BoundingBoxImpl* boundingBox);

    virtual const char* GetName() const;
    virtual Location* GetLocation() const;
    virtual Phone* GetPhone() const;
    virtual int GetNumPhone() const;
    virtual Category* GetCategory() const;
    virtual int GetNumCategory() const;
    virtual BoundingBox* GetBoundingBox() const;

private:
    void Initialize();

    char* m_name;              /*!< Name of the place */
    Location* m_location;      /*!< Location of the place */
    Phone* m_phone;            /*!< All the phones of this place */
    int m_numPhone;            /*!< Actual number of phone contacts for this place. */
    Category* m_category;      /*!< Category of this point of interest */
    int m_numCategory;         /*!< Actual number of categories for this place. */
    BoundingBox* m_boundingBox;		   /*!<  Get the bounding box of the place */
};
/*! Implementation of FuelType interface. */
class FuelTypeImpl : public FuelType
{
public:
    FuelTypeImpl();
    ~FuelTypeImpl();

    FuelTypeImpl(const FuelTypeImpl& objFuelTypeImpl);
    FuelTypeImpl& operator=(const FuelTypeImpl& objFuelTypeImpl);

    void SetProductName(char* productName);
    void SetCode(char* code);
    void SetTypeName(char* typeName);

    virtual char* GetProductName() const;
    virtual char* GetCode() const;
    virtual char* GetTypeName() const;

private:
    void Initialize();
    char* m_productName;   /*!< Name of the actual fuel product as sold by providers */
    char* m_code;          /*!< Internal code for this fuel type */
    char* m_typeName;      /*!< The type of the fuel regardless of the provider */
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
    void SetCurrency(char* currency);
    void SetModTime(uint32 modTime);

    virtual double GetValue() const;
    virtual char* GetCurrency() const;
    virtual uint32 GetModTime() const;
private:
    void Initialize();
    double m_value;
    char* m_currency;
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

    void SetPrice(PriceImpl* price);
    void SetUnits(char* units);
    void SetFuelType(FuelTypeImpl* fuelType);

    virtual Price* GetPrice() const;
    virtual char* GetUnits() const;
    virtual FuelType* GetFuelType() const;
private:
    void Initialize();
    Price* m_price;
    char* m_units;
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

    void SetAverage(FuelProduct* average);
    void SetLow(FuelProduct* low);

    virtual FuelProduct* GetAverage() const;
    virtual FuelProduct* GetLow() const;
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

    void SetFuelProduct(int index, FuelProductImpl* fuelProduct);
    void SetNumFuelProducts(int numFuelProducts);

    virtual FuelProduct* GetFuelProduct(int index) const;
    virtual int GetNumFuelProducts() const;
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
    FuelCommonImpl(FuelDetailsImpl* fuelDetails);
    FuelCommonImpl(FuelSummaryImpl* fuelSummary);
    ~FuelCommonImpl();

    FuelCommonImpl(const FuelCommonImpl& objFuelCommonImpl);
    FuelCommonImpl& operator=(const FuelCommonImpl& objFuelCommonImpl);

    void SetFuelDetails(FuelDetailsImpl* fuelDetails);
    void SetFuelSummary(FuelSummaryImpl* fuelSummary);

    virtual FuelDetails* GetFuelDetails() const;
    virtual FuelSummary* GetFuelSummary() const;

private:
    void Initialize();
    FuelDetails* m_fuelDetails;
    FuelSummary* m_fuelSummary;
};

}
#endif
