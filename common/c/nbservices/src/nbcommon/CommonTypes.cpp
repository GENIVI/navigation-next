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

    @file       nbcommontypes.cpp

    See header file for description.
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

#include "CommonTypes.h"
#include "nbsearchtypes.h"
#include "CommonTypesProtected.h"
using namespace nbcommon;

#define SET_STRING(class_member, param)        \
    if (class_member) {                        \
        nsl_free(class_member);                \
        class_member = NULL;  }                \
    if (param) {                               \
        class_member = nsl_strdup(param); }

#define CHECK_AND_DELETE(param)                \
    if (NULL != param) {                       \
    delete param;                              \
    param = NULL;  }                           \

#define CHECK_AND_DELETE_CHAR(param)           \
    if (NULL != param) {                       \
    nsl_free(param);                           \
    param = NULL;  }                           \


//CategoryImpl
CategoryImpl::CategoryImpl(const nbcommon::CategoryImpl &objCategory)
{
    Initialize();

    SET_STRING(m_code, objCategory.GetCode())
    SET_STRING(m_name, objCategory.GetName())
}
CategoryImpl& CategoryImpl::operator=(const CategoryImpl& objCategory)
{
    // Handle self-assignment:
    if(this == &objCategory)
    {
        return *this;
    }

    Initialize();

    SET_STRING(m_code, objCategory.GetCode())
    SET_STRING(m_name, objCategory.GetName())

    return *this;
}
CategoryImpl::CategoryImpl() : Category()
{
    Initialize();
}
CategoryImpl::~CategoryImpl()
{
    CHECK_AND_DELETE_CHAR(m_code)
    CHECK_AND_DELETE_CHAR(m_name)
}

void CategoryImpl::Initialize()
{
    m_code = NULL;
    m_name = NULL;
}

const char* CategoryImpl::GetCode() const
{
    return m_code;
}

const char* CategoryImpl::GetName() const
{
    return m_name;
}

void CategoryImpl::SetCode(char *code)
{
    SET_STRING(m_code, code);
}

void CategoryImpl::SetName(char *name)
{
    SET_STRING(m_name, name);
}

//PhoneImpl
PhoneImpl::PhoneImpl(const nbcommon::PhoneImpl &objPhone)
{

    Initialize();

    SET_STRING(m_area, objPhone.GetArea())
    SET_STRING(m_country, objPhone.GetCountry())
    SET_STRING(m_number, objPhone.GetNumber())

    m_phoneType = objPhone.GetPhoneType();

}
PhoneImpl& PhoneImpl::operator=(const PhoneImpl& objPhone)
{
    // Handle self-assignment:
    if(this == &objPhone)
    {
        return *this;
    }

    Initialize();

    SET_STRING(m_area, objPhone.GetArea())
    SET_STRING(m_country, objPhone.GetCountry())
    SET_STRING(m_number, objPhone.GetNumber())
    m_phoneType = objPhone.GetPhoneType();

    return *this;
}

PhoneImpl::PhoneImpl()
{
    Initialize();
}
PhoneImpl::~PhoneImpl()
{
    CHECK_AND_DELETE_CHAR(m_area)
    CHECK_AND_DELETE_CHAR(m_country)
    CHECK_AND_DELETE_CHAR(m_number)
}
void PhoneImpl::Initialize()
{
    m_area = NULL;
    m_country = NULL;
    m_number = NULL;
    m_phoneType = Phone_Primary;
}

const char* PhoneImpl::GetArea() const
{
    return m_area;
}
const char* PhoneImpl::GetCountry() const
{
    return m_country;
}
const char* PhoneImpl::GetNumber() const
{
    return m_number;
}
Phone::PhoneType PhoneImpl::GetPhoneType() const
{
    return m_phoneType;
}

void PhoneImpl::SetArea(char* area)
{
     SET_STRING(m_area, area);
}
void PhoneImpl::SetCountry(char* country)
{
    SET_STRING(m_country, country);
}
void PhoneImpl::SetNumber(char* number)
{
    SET_STRING(m_number, number);
}

void PhoneImpl::SetPhoneType(PhoneType type)
{
    m_phoneType = type;
}
//PlaceImpl
PlaceImpl& PlaceImpl::operator=(const PlaceImpl& objPlace)
{
    // Handle self-assignment:
    if(this == &objPlace)
    {
        return *this;
    }

    Initialize();

    SET_STRING(m_name, objPlace.GetName())

    if (objPlace.GetCategory())
    {
        CHECK_AND_DELETE(m_category)
        m_category = new CategoryImpl(*(static_cast<CategoryImpl*>(objPlace.GetCategory())));
    }
    if (objPlace.GetLocation())
    {
        CHECK_AND_DELETE(m_location)
        m_location = new LocationImpl(*(static_cast<LocationImpl*>(objPlace.GetLocation())));
    }

    m_numCategory = objPlace.GetNumCategory();

    m_numPhone = objPlace.GetNumPhone();

    if (objPlace.GetPhone())
    {
        CHECK_AND_DELETE(m_phone)
        m_phone = new PhoneImpl(*(static_cast<PhoneImpl*>(objPlace.GetPhone())));
    }

    m_boundingBox = objPlace.GetBoundingBox();

    return *this;
}
PlaceImpl::PlaceImpl(const nbcommon::PlaceImpl &objPlace)
{
    Initialize();

    SET_STRING(m_name, objPlace.GetName())

    if (objPlace.GetCategory())
    {
        CHECK_AND_DELETE(m_category)
        m_category = new CategoryImpl(*(static_cast<CategoryImpl*>(objPlace.GetCategory())));
    }
    if (objPlace.GetLocation())
    {
        CHECK_AND_DELETE(m_location)
        m_location = new LocationImpl(*(static_cast<LocationImpl*>(objPlace.GetLocation())));
    }

    m_numCategory = objPlace.GetNumCategory();

    m_numPhone = objPlace.GetNumPhone();

    if (objPlace.GetPhone())
    {
        CHECK_AND_DELETE(m_phone)
        m_phone = new PhoneImpl(*(static_cast<PhoneImpl*>(objPlace.GetPhone())));
    }

    m_boundingBox = objPlace.GetBoundingBox();

}

PlaceImpl::PlaceImpl()
{
    Initialize();
}
PlaceImpl::~PlaceImpl()
{
    CHECK_AND_DELETE_CHAR(m_name);
    CHECK_AND_DELETE(m_location);
    CHECK_AND_DELETE(m_phone);
    CHECK_AND_DELETE(m_category);
}
void PlaceImpl::Initialize()
{
    m_location = NULL;
    m_phone = NULL;
    m_category = NULL;
    m_name = NULL;
    m_numPhone = 0;
    m_numCategory = 0;
    m_boundingBox = NULL;
}
Category* PlaceImpl::GetCategory() const
{
    return m_category;
}
Location* PlaceImpl::GetLocation() const
{
    return m_location;
}
const char* PlaceImpl::GetName() const
{
    return m_name;
}
int PlaceImpl::GetNumCategory() const
{
    return m_numCategory;
}
int PlaceImpl::GetNumPhone() const
{
    return m_numPhone;
}
Phone* PlaceImpl::GetPhone() const
{
    return m_phone;
}

BoundingBox* PlaceImpl::GetBoundingBox() const
{
    return m_boundingBox;
}

void PlaceImpl::SetCategory(CategoryImpl* category)
{
    CHECK_AND_DELETE(m_category)
    m_category = new CategoryImpl(*category);
}
void PlaceImpl::SetLocation(LocationImpl* location)
{
    CHECK_AND_DELETE(m_location)
    m_location = new LocationImpl(*location);
}
void PlaceImpl::SetName(char* name)
{
    SET_STRING(m_name, name);
}
void PlaceImpl::SetNumCategory(int numCategory)
{
    m_numCategory = numCategory;
}

void PlaceImpl::SetNumPhone(int numPhone)
{
    m_numPhone = numPhone;
}
void PlaceImpl::SetPhone(PhoneImpl* phone)
{
    CHECK_AND_DELETE(m_phone)
    m_phone = new PhoneImpl(*phone);
}

void PlaceImpl::SetBoundingBox(const BoundingBoxImpl* boundingBox)
{
	CHECK_AND_DELETE(m_boundingBox)
    m_boundingBox = new BoundingBoxImpl(*boundingBox);
}

//Location
LocationImpl::LocationImpl(const LocationImpl& location)
{

    Initialize();

    CHECK_AND_DELETE(m_coords)
    m_coords = new LatitudeLongitude();
    m_coords->SetLatitude(location.GetCoordinates()->GetLatitude());
    m_coords->SetLongitude(location.GetCoordinates()->GetLongitude());

    if (location.GetName())
    {
        m_name = nsl_strdup(location.GetName());
    }
    if (location.GetNumber())
    {
        m_number = nsl_strdup(location.GetNumber());
    }
    if (location.GetStreet())
    {
        m_street = nsl_strdup(location.GetStreet());
    }
    if (location.GetCrossStreet())
    {
       m_crossStreet = nsl_strdup(location.GetCrossStreet());
    }
    if (location.GetCity())
    {
        m_city = nsl_strdup(location.GetCity());
    }
    if (location.GetCounty())
    {
       m_county = nsl_strdup(location.GetCounty());
    }
    if (location.GetState())
    {
        m_state = nsl_strdup(location.GetState());
    }
    if (location.GetZipCode())
    {
        m_zipCode = nsl_strdup(location.GetZipCode());
    }
    if (location.GetCountry())
    {
        m_country = nsl_strdup(location.GetCountry());
    }
    if (location.GetAirport())
    {
        m_airport = nsl_strdup(location.GetAirport());
    }
    if (location.GetFreeForm())
    {
        m_freeForm = nsl_strdup(location.GetFreeForm());
    }
    m_UTCOffset = location.GetUTCOffset();
}

LocationImpl& LocationImpl::operator=(const LocationImpl& location)
{
    if (this == &location)
    {
        return *this;
    }

    Initialize();
    m_coords = new LatitudeLongitude();
    m_coords->SetLatitude(location.GetCoordinates()->GetLatitude());
    m_coords->SetLongitude(location.GetCoordinates()->GetLongitude());

    if (location.GetName())
    {
        m_name = nsl_strdup(location.GetName());
    }
    if (location.GetNumber())
    {
        m_number = nsl_strdup(location.GetNumber());
    }
    if (location.GetStreet())
    {
        m_street = nsl_strdup(location.GetStreet());
    }
    if (location.GetCrossStreet())
    {
       m_crossStreet = nsl_strdup(location.GetCrossStreet());
    }
    if (location.GetCity())
    {
        m_city = nsl_strdup(location.GetCity());
    }
    if (location.GetCounty())
    {
       m_county = nsl_strdup(location.GetCounty());
    }
    if (location.GetState())
    {
        m_state = nsl_strdup(location.GetState());
    }
    if (location.GetZipCode())
    {
        m_zipCode = nsl_strdup(location.GetZipCode());
    }
    if (location.GetCountry())
    {
        m_country = nsl_strdup(location.GetCountry());
    }
    if (location.GetAirport())
    {
        m_airport = nsl_strdup(location.GetAirport());
    }
    if (location.GetFreeForm())
    {
        m_freeForm = nsl_strdup(location.GetFreeForm());
    }
    m_UTCOffset = location.GetUTCOffset();
    return *this;
}
//LocationImpl
LocationImpl::LocationImpl(void)
{
    Initialize();
}
void LocationImpl::Initialize()
{
    m_coords = NULL;
    m_type = Location_None;
    m_name = NULL;
    m_number = NULL;
    m_street = NULL;
    m_crossStreet = NULL;
    m_city = NULL;
    m_county = NULL;
    m_state = NULL;
    m_zipCode = NULL;
    m_country = NULL;
    m_airport = NULL;
    m_freeForm = NULL;
    m_UTCOffset = 0;
}

LocationImpl::~LocationImpl(void)
{
    CHECK_AND_DELETE(m_coords)
    CHECK_AND_DELETE_CHAR(m_name)
    CHECK_AND_DELETE_CHAR(m_number)
    CHECK_AND_DELETE_CHAR(m_street)
    CHECK_AND_DELETE_CHAR(m_crossStreet)
    CHECK_AND_DELETE_CHAR(m_city)
    CHECK_AND_DELETE_CHAR(m_county)
    CHECK_AND_DELETE_CHAR(m_state)
    CHECK_AND_DELETE_CHAR(m_zipCode)
    CHECK_AND_DELETE_CHAR(m_country)
    CHECK_AND_DELETE_CHAR(m_airport)
    CHECK_AND_DELETE_CHAR(m_freeForm)
}

Location::LocationType LocationImpl::GetType(void) const
{
    return m_type;
}

LatitudeLongitude* LocationImpl::GetCoordinates(void) const
{
    return m_coords;
}

const char* LocationImpl::GetName(void) const
{
    return m_name;
}

const char* LocationImpl::GetNumber(void) const
{
    return m_number;
}

const char* LocationImpl::GetStreet(void) const
{
    return m_street;
}

const char* LocationImpl::GetCrossStreet(void) const
{
    return m_crossStreet;
}

const char* LocationImpl::GetCity(void) const
{
    return m_city;
}

const char* LocationImpl::GetCounty(void) const
{
    return m_county;
}

const char* LocationImpl::GetState(void) const
{
    return m_state;
}

const char* LocationImpl::GetZipCode(void) const
{
    return m_zipCode;
}

const char* LocationImpl::GetCountry(void) const
{
    return m_country;
}

const char* LocationImpl::GetAirport(void) const
{
    return m_airport;
}

const char* LocationImpl::GetFreeForm(void) const
{
    return m_freeForm;
}

float LocationImpl::GetUTCOffset() const
{
    return m_UTCOffset;
}

void LocationImpl::SetAirport(char *airport)
{
    SET_STRING(m_airport, airport)
}
void LocationImpl::SetCity(char* city)
{
    SET_STRING(m_city, city)
}
void LocationImpl::SetCoordinates(LatitudeLongitude* coords)
{
    CHECK_AND_DELETE(m_coords)
    m_coords = new LatitudeLongitude(*coords);
}
void LocationImpl::SetCountry(char* country)
{
    SET_STRING(m_country, country)
}
void LocationImpl::SetCounty(char* county)
{
    SET_STRING(m_county, county)
}
void LocationImpl::SetCrossStreet(char *crossStreet)
{
    SET_STRING(m_crossStreet, crossStreet)
}
void LocationImpl::SetName(char *name)
{
    SET_STRING(m_name, name)
}
void LocationImpl::SetNumber(char *number)
{
    SET_STRING(m_number, number)
}
void LocationImpl::SetState(char* state)
{
    SET_STRING(m_state, state)
}
void LocationImpl::SetStreet(char* street)
{
    SET_STRING(m_street, street)
}
void LocationImpl::SetType(Location::LocationType type)
{
    m_type = type;
}

void LocationImpl::SetZipCode(char* zipCode)
{
    SET_STRING(m_zipCode, zipCode)
}

void LocationImpl::SetFreeForm(char* freeForm)
{
    SET_STRING(m_freeForm, freeForm)
}

//Address
AddressImpl::AddressImpl(const AddressImpl& addr)
{

    Initialize();

    if (addr.GetNumber())
    {
        m_number = nsl_strdup(addr.GetNumber());
    }
    if (addr.GetStreet())
    {
        m_street = nsl_strdup(addr.GetStreet());
    }
    if (addr.GetCity())
    {
        m_city = nsl_strdup(addr.GetCity());
    }
    if (addr.GetCounty())
    {
        m_county = nsl_strdup(addr.GetCounty());
    }
    if (addr.GetState())
    {
        m_state = nsl_strdup(addr.GetState());
    }
    if (addr.GetZipCode())
    {
        m_zipCode = nsl_strdup(addr.GetZipCode());
    }
    if (addr.GetCountry())
    {
        m_country = nsl_strdup(addr.GetCountry());
    }
}
AddressImpl& AddressImpl::operator=(const AddressImpl& addr)
{
    if(this == &addr)
    {
        return *this;
    }

    Initialize();

    if (addr.GetNumber())
    {
        m_number = nsl_strdup(addr.GetNumber());
    }
    if (addr.GetStreet())
    {
        m_street = nsl_strdup(addr.GetStreet());
    }
    if (addr.GetCity())
    {
        m_city = nsl_strdup(addr.GetCity());
    }
    if (addr.GetCounty())
    {
        m_county = nsl_strdup(addr.GetCounty());
    }
    if (addr.GetState())
    {
        m_state = nsl_strdup(addr.GetState());
    }
    if (addr.GetZipCode())
    {
        m_zipCode = nsl_strdup(addr.GetZipCode());
    }
    if (addr.GetCountry())
    {
        m_country = nsl_strdup(addr.GetCountry());
    }

    return *this;
}
//AddressImpl
AddressImpl::AddressImpl(void)
{
    Initialize();
}
void AddressImpl::Initialize()
{
    m_number = NULL;
    m_street = NULL;
    m_city = NULL;
    m_county = NULL;
    m_state = NULL;
    m_zipCode = NULL;
    m_country = NULL;
}

AddressImpl::~AddressImpl(void)
{
    CHECK_AND_DELETE_CHAR(m_number)
    CHECK_AND_DELETE_CHAR(m_street)
    CHECK_AND_DELETE_CHAR(m_city)
    CHECK_AND_DELETE_CHAR(m_county)
    CHECK_AND_DELETE_CHAR(m_state)
    CHECK_AND_DELETE_CHAR(m_zipCode)
    CHECK_AND_DELETE_CHAR(m_country)
}

const char* AddressImpl::GetNumber(void) const
{
    return m_number;
}

const char* AddressImpl::GetStreet(void) const
{
    return m_street;
}

const char* AddressImpl::GetCity(void) const
{
    return m_city;
}

const char* AddressImpl::GetCounty(void) const
{
    return m_county;
}

const char* AddressImpl::GetState(void) const
{
    return m_state;
}

const char* AddressImpl::GetZipCode(void) const
{
    return m_zipCode;
}

const char* AddressImpl::GetCountry(void) const
{
    return m_country;
}

void AddressImpl::SetCity(char *city)
{
    SET_STRING(m_city, city)
}
void AddressImpl::SetCountry(char* country)
{
    SET_STRING(m_country, country)
}
void AddressImpl::SetCounty(char* county)
{
    SET_STRING(m_county, county)
}
void AddressImpl::SetNumber(char* number)
{
    SET_STRING(m_number, number)
}

void AddressImpl::SetState(char* state)
{
    SET_STRING(m_state, state)
}
void AddressImpl::SetStreet(char* street)
{
    SET_STRING(m_street, street)
}
void AddressImpl::SetZipCode(char* zipCode)
{
    SET_STRING(m_zipCode, zipCode)
}
//SuggestionMatch
SuggestionMatchImpl::SuggestionMatchImpl()
{
    Initialize();
}

SuggestionMatchImpl::~SuggestionMatchImpl()
{
    CHECK_AND_DELETE_CHAR(m_firstLine);
    CHECK_AND_DELETE_CHAR(m_secondLine);
    CHECK_AND_DELETE_CHAR(m_thirdLine);
    CHECK_AND_DELETE_CHAR(m_suggestionCookie);
}
SuggestionMatchImpl& SuggestionMatchImpl::operator=(const SuggestionMatchImpl& objSuggestionMatch)
{

    // Handle self-assignment:
    if(this == &objSuggestionMatch)
    {
        return *this;
    }

    Initialize();

    if (NULL != objSuggestionMatch.GetFirstLine())
    {
        SET_STRING(m_firstLine, objSuggestionMatch.GetFirstLine());
    }
    if (NULL != objSuggestionMatch.GetSecondLine())
    {
        SET_STRING(m_secondLine, objSuggestionMatch.GetSecondLine());
    }
	if (NULL != objSuggestionMatch.GetThirdLine())
    {
        SET_STRING(m_thirdLine, objSuggestionMatch.GetThirdLine());
    }
    if (NULL != objSuggestionMatch.GetSuggestionCookie())
    {
        SET_STRING(m_suggestionCookie, objSuggestionMatch.GetSuggestionCookie());
    }

    m_distance = objSuggestionMatch.GetDistance();

    m_matchType = objSuggestionMatch.GetMatchType();

    return *this;
}
SuggestionMatchImpl::SuggestionMatchImpl(const nbcommon::SuggestionMatchImpl &objSuggestionMatch)
{
    Initialize();


    if (NULL != objSuggestionMatch.GetFirstLine())
    {
        SET_STRING(m_firstLine, objSuggestionMatch.GetFirstLine());
    }
    if (NULL != objSuggestionMatch.GetSecondLine())
    {
        SET_STRING(m_secondLine, objSuggestionMatch.GetSecondLine());
    }
    if (NULL != objSuggestionMatch.GetThirdLine())
    {
        SET_STRING(m_thirdLine, objSuggestionMatch.GetThirdLine());
    }
    if (NULL != objSuggestionMatch.GetSuggestionCookie())
    {
        SET_STRING(m_suggestionCookie, objSuggestionMatch.GetSuggestionCookie());
    }


    m_distance = objSuggestionMatch.GetDistance();

    m_matchType = objSuggestionMatch.GetMatchType();

}
//SuggestionMatchImpl
void SuggestionMatchImpl::Initialize()
{
    m_firstLine = NULL;
    m_secondLine = NULL;
    m_thirdLine = NULL;
    m_suggestionCookie = NULL;
    m_distance = 0;

}
const char* SuggestionMatchImpl::GetFirstLine() const
{
    return m_firstLine;
}
const char* SuggestionMatchImpl::GetSecondLine() const
{
    return m_secondLine;
}
const char* SuggestionMatchImpl::GetThirdLine() const
{
    return m_thirdLine;
}
NB_MatchType SuggestionMatchImpl::GetMatchType() const
{
    return m_matchType;
}

double SuggestionMatchImpl::GetDistance() const
{
    return m_distance;
}
const char* SuggestionMatchImpl::GetSuggestionCookie() const
{
    return m_suggestionCookie;
}


void SuggestionMatchImpl::SetFirstLine(const char *firstLine)
{
    SET_STRING(m_firstLine, firstLine);
}
void SuggestionMatchImpl::SetSecondLine(const char *secondLine)
{
    SET_STRING(m_secondLine, secondLine);
}
void SuggestionMatchImpl::SetThirdLine(const char *thirdLine)
{
    SET_STRING(m_thirdLine, thirdLine);
}
void SuggestionMatchImpl::SetMatchType(NB_MatchType matchType)
{
    m_matchType = matchType;
}
void SuggestionMatchImpl::SetDistance(const double distance)
{
	m_distance = distance;
}
void SuggestionMatchImpl::SetSuggestionCookie(const char* suggestionCookie)
{
	 SET_STRING(m_suggestionCookie, suggestionCookie);
}


//LatitudeLongitude
LatitudeLongitude::LatitudeLongitude()
{
    Initialize();
}
LatitudeLongitude::LatitudeLongitude(double latitude, double longitude)
{
    Initialize();
    m_latitude  = latitude;
    m_longitude = longitude;
}
LatitudeLongitude::~LatitudeLongitude()
{
}
LatitudeLongitude& LatitudeLongitude::operator=(const LatitudeLongitude& objLatitudeLongitude)
{
    // Handle self-assignment:
    if(this == &objLatitudeLongitude)
    {
        return *this;
    }

    m_latitude = objLatitudeLongitude.GetLatitude();
    m_longitude = objLatitudeLongitude.GetLongitude();

    return *this;
}

LatitudeLongitude::LatitudeLongitude(const nbcommon::LatitudeLongitude &objLatitudeLongitude)
{
    m_latitude = objLatitudeLongitude.GetLatitude();
    m_longitude = objLatitudeLongitude.GetLongitude();
}
double LatitudeLongitude::GetLatitude() const
{
    return m_latitude;
}

double LatitudeLongitude::GetLongitude() const
{
    return m_longitude;
}

void LatitudeLongitude::Initialize()
{
    m_latitude = 0.0;
    m_longitude = 0.0;
}

void LatitudeLongitude::SetLatitude(double latitude)
{
    m_latitude = latitude;
}

void LatitudeLongitude::SetLongitude(double longitude)
{
    m_longitude = longitude;
}

//FuelTypeImpl
void FuelTypeImpl::Initialize()
{
    m_productName = NULL;
    m_code = NULL;
    m_typeName = NULL;
}
FuelTypeImpl::FuelTypeImpl()
{
    Initialize();
}
FuelTypeImpl::~FuelTypeImpl()
{
    CHECK_AND_DELETE_CHAR(m_productName)
    CHECK_AND_DELETE_CHAR(m_code)
    CHECK_AND_DELETE_CHAR(m_typeName)
}
FuelTypeImpl::FuelTypeImpl(const nbcommon::FuelTypeImpl &objFuelTypeImpl)
{
    Initialize();
    SET_STRING(m_productName, objFuelTypeImpl.GetProductName())
    SET_STRING(m_code, objFuelTypeImpl.GetCode())
    SET_STRING(m_typeName, objFuelTypeImpl.GetTypeName())
}
FuelTypeImpl& FuelTypeImpl::operator=(const FuelTypeImpl& objFuelTypeImpl)
{
    // Handle self-assignment:
    if(this == &objFuelTypeImpl)
    {
        return *this;
    }

    Initialize();
    SET_STRING(m_productName, objFuelTypeImpl.GetProductName())
    SET_STRING(m_code, objFuelTypeImpl.GetCode())
    SET_STRING(m_typeName, objFuelTypeImpl.GetTypeName())

    return *this;
}
char* FuelTypeImpl::GetCode() const
{
    return m_code;
}
char* FuelTypeImpl::GetProductName() const
{
    return m_productName;
}
char* FuelTypeImpl::GetTypeName() const
{
    return m_typeName;
}
void FuelTypeImpl::SetCode(char *code)
{
    SET_STRING(m_code, code)
}
void FuelTypeImpl::SetProductName(char *productName)
{
    SET_STRING(m_productName, productName)
}
void FuelTypeImpl::SetTypeName(char *typeName)
{
    SET_STRING(m_typeName, typeName)
}

//PriceImpl
void PriceImpl::Initialize()
{
    m_currency = NULL;
}
PriceImpl::PriceImpl()
{
    Initialize();
}
PriceImpl::~PriceImpl()
{
    CHECK_AND_DELETE_CHAR(m_currency)
}
PriceImpl::PriceImpl(const nbcommon::PriceImpl &objPriceImpl)
{
    Initialize();
    m_value = objPriceImpl.GetValue();
    SET_STRING(m_currency, objPriceImpl.GetCurrency())
    m_modTime = objPriceImpl.GetModTime();
}

PriceImpl& PriceImpl::operator=(const PriceImpl& objPriceImpl)
{
    // Handle self-assignment:
    if(this == &objPriceImpl)
    {
        return *this;
    }

    Initialize();
    m_value = objPriceImpl.GetValue();
    SET_STRING(m_currency, objPriceImpl.GetCurrency())
    m_modTime = objPriceImpl.GetModTime();

    return *this;
}
double PriceImpl::GetValue() const
{
    return m_value;
}
uint32 PriceImpl::GetModTime() const
{
    return m_modTime;
}
char* PriceImpl::GetCurrency() const
{
    return m_currency;
}
void PriceImpl::SetCurrency(char *currency)
{
    SET_STRING(m_currency, currency)
}
void PriceImpl::SetModTime(uint32 modTime)
{
    m_modTime = modTime;
}
void PriceImpl::SetValue(double value)
{
    m_value = value;
}

//FuelProductImpl
void FuelProductImpl::Initialize()
{
    m_price = NULL;
    m_units = NULL;
    m_fuelType = NULL;
}
FuelProductImpl::FuelProductImpl()
{
    Initialize();
}
FuelProductImpl::~FuelProductImpl()
{
    CHECK_AND_DELETE(m_price)
    CHECK_AND_DELETE_CHAR(m_units)
    CHECK_AND_DELETE(m_fuelType)
}
FuelProductImpl::FuelProductImpl(const nbcommon::FuelProductImpl &objFuelProductImpl)
{
    Initialize();
    CHECK_AND_DELETE(m_price)
    m_price = new PriceImpl(*(static_cast<PriceImpl*>(objFuelProductImpl.GetPrice())));
    SET_STRING(m_units, objFuelProductImpl.GetUnits());
    CHECK_AND_DELETE(m_fuelType)
    m_fuelType = new FuelTypeImpl(*(static_cast<FuelTypeImpl*>(objFuelProductImpl.GetFuelType())));
}
FuelProductImpl& FuelProductImpl::operator=(const FuelProductImpl& objFuelProductImpl)
{
    // Handle self-assignment:
    if(this == &objFuelProductImpl)
    {
        return *this;
    }

    Initialize();
    CHECK_AND_DELETE(m_price)
    m_price = new PriceImpl(*(static_cast<PriceImpl*>(objFuelProductImpl.GetPrice())));
    SET_STRING(m_units, objFuelProductImpl.GetUnits());
    CHECK_AND_DELETE(m_fuelType)
    m_fuelType = new FuelTypeImpl(*(static_cast<FuelTypeImpl*>(objFuelProductImpl.GetFuelType())));

    return *this;
}
Price* FuelProductImpl::GetPrice() const
{
    return m_price;
}
char* FuelProductImpl::GetUnits() const
{
    return m_units;
}
FuelType* FuelProductImpl::GetFuelType() const
{
    return m_fuelType;
}
void FuelProductImpl::SetFuelType(FuelTypeImpl *fuelType)
{
    CHECK_AND_DELETE(m_fuelType)
    m_fuelType = new FuelTypeImpl(*(static_cast<FuelTypeImpl*>(fuelType)));
}
void FuelProductImpl::SetUnits(char *units)
{
    SET_STRING(m_units, units)
}
void FuelProductImpl::SetPrice(PriceImpl *price)
{
    CHECK_AND_DELETE(m_price)
    m_price = new PriceImpl(*(static_cast<PriceImpl*>(price)));
}

//FuelSummaryImpl
void FuelSummaryImpl::Initialize()
{
    m_average = NULL;
    m_low = NULL;
}
FuelSummaryImpl::FuelSummaryImpl()
{
    Initialize();
}
FuelSummaryImpl::~FuelSummaryImpl()
{
    CHECK_AND_DELETE(m_average)
    CHECK_AND_DELETE(m_low)
}
FuelSummaryImpl::FuelSummaryImpl(const nbcommon::FuelSummaryImpl &objFuelSummaryImpl)
{
    Initialize();
    CHECK_AND_DELETE(m_average)
    m_average = new FuelProductImpl(*(static_cast<FuelProductImpl*>(objFuelSummaryImpl.GetAverage())));
    CHECK_AND_DELETE(m_low)
    m_low = new FuelProductImpl(*(static_cast<FuelProductImpl*>(objFuelSummaryImpl.GetLow())));
}
FuelSummaryImpl& FuelSummaryImpl::operator=(const FuelSummaryImpl& objFuelSummaryImpl)
{
    if(this == &objFuelSummaryImpl)
    {
        return *this;
    }

    Initialize();
    CHECK_AND_DELETE(m_average)
    m_average = new FuelProductImpl(*(static_cast<FuelProductImpl*>(objFuelSummaryImpl.GetAverage())));
    CHECK_AND_DELETE(m_low)
    m_low = new FuelProductImpl(*(static_cast<FuelProductImpl*>(objFuelSummaryImpl.GetLow())));

    return *this;
}
FuelProduct* FuelSummaryImpl::GetAverage() const
{
    return m_average;
}
FuelProduct* FuelSummaryImpl::GetLow() const
{
    return m_low;
}
void FuelSummaryImpl::SetAverage(FuelProduct *average)
{
    CHECK_AND_DELETE(m_average)
    m_average = new FuelProductImpl(*(static_cast<FuelProductImpl*>(average)));
}
void FuelSummaryImpl::SetLow(FuelProduct *low)
{
    CHECK_AND_DELETE(m_low)
    m_low = new FuelProductImpl(*(static_cast<FuelProductImpl*>(low)));
}
//FuelDetailsImpl
void FuelDetailsImpl::Initialize()
{
    for (int i = 0; i < NB_FUEL_PRODUCTS_NUM; i++)
    {
        m_fuelProduct[i] = NULL;
    }
    m_numFuelProducts = 0;
}
FuelDetailsImpl::FuelDetailsImpl()
{
    Initialize();
}
FuelDetailsImpl::~FuelDetailsImpl()
{
    for (int i = 0; i < m_numFuelProducts; i++)
    {
        CHECK_AND_DELETE(m_fuelProduct[i])
    }
}
FuelDetailsImpl::FuelDetailsImpl(const nbcommon::FuelDetailsImpl &objFuelDetailsImpl)
{
    Initialize();
    for (int i = 0; i < objFuelDetailsImpl.GetNumFuelProducts(); i++)
    {
        CHECK_AND_DELETE(m_fuelProduct[i])
        if (NULL != objFuelDetailsImpl.GetFuelProduct(i))
        {
            m_fuelProduct[i] = new FuelProductImpl(*(static_cast<FuelProductImpl*>(objFuelDetailsImpl.GetFuelProduct(i))));
        }
    }
    m_numFuelProducts = objFuelDetailsImpl.GetNumFuelProducts();
}
FuelDetailsImpl& FuelDetailsImpl::operator=(const FuelDetailsImpl& objFuelDetailsImpl)
{
    if(this == &objFuelDetailsImpl)
    {
        return *this;
    }

    Initialize();
    for (int i = 0; i < objFuelDetailsImpl.GetNumFuelProducts(); i++)
    {
        CHECK_AND_DELETE(m_fuelProduct[i])
        if (NULL != objFuelDetailsImpl.GetFuelProduct(i))
        {
            m_fuelProduct[i] = new FuelProductImpl(*(static_cast<FuelProductImpl*>(objFuelDetailsImpl.GetFuelProduct(i))));
        }
    }
    m_numFuelProducts = objFuelDetailsImpl.GetNumFuelProducts();

    return *this;
}
FuelProduct* FuelDetailsImpl::GetFuelProduct(int index) const
{
    if (index < 0 || index > m_numFuelProducts)
    {
        //out of range
        return NULL;
    }
    return m_fuelProduct[index];
}
int FuelDetailsImpl::GetNumFuelProducts() const
{
    return m_numFuelProducts;
}
void FuelDetailsImpl::SetFuelProduct(int index, FuelProductImpl *fuelProduct)
{
    if (index < 0 || index > m_numFuelProducts)
    {
        //out of range
        return;
    }
    CHECK_AND_DELETE(m_fuelProduct[index])
    m_fuelProduct[index] = new FuelProductImpl(*(static_cast<FuelProductImpl*>(fuelProduct)));

}
void FuelDetailsImpl::SetNumFuelProducts(int numFuelProducts)
{
    if (numFuelProducts < 0 || numFuelProducts > NB_FUEL_PRODUCTS_NUM)
    {
        //out of range
        return;
    }
    m_numFuelProducts = numFuelProducts;
}
//FuelCommonImpl
void FuelCommonImpl::Initialize()
{
    m_fuelDetails = NULL;
    m_fuelSummary = NULL;
}
FuelCommonImpl::FuelCommonImpl()
{
    Initialize();
}
FuelCommonImpl::FuelCommonImpl(FuelDetailsImpl* fuelDetails)
{
    Initialize();
    m_fuelDetails = new FuelDetailsImpl(*(static_cast<FuelDetailsImpl*>(fuelDetails)));
    CHECK_AND_DELETE(m_fuelSummary)
}
FuelCommonImpl::FuelCommonImpl(FuelSummaryImpl* fuelSummary)
{
    Initialize();
    CHECK_AND_DELETE(m_fuelDetails)
    m_fuelSummary = new FuelSummaryImpl(*(static_cast<FuelSummaryImpl*>(fuelSummary)));
}
FuelCommonImpl::~FuelCommonImpl()
{
    CHECK_AND_DELETE(m_fuelDetails)
    CHECK_AND_DELETE(m_fuelSummary)
}
FuelCommonImpl::FuelCommonImpl(const nbcommon::FuelCommonImpl &objFuelCommonImpl)
{
    Initialize();
    CHECK_AND_DELETE(m_fuelDetails)
    if (objFuelCommonImpl.GetFuelDetails())
    {
        m_fuelDetails = new FuelDetailsImpl(*(static_cast<FuelDetailsImpl*>(objFuelCommonImpl.GetFuelDetails())));
    }
    CHECK_AND_DELETE(m_fuelSummary)
    if (objFuelCommonImpl.GetFuelSummary())
    {
        m_fuelSummary = new FuelSummaryImpl(*(static_cast<FuelSummaryImpl*>(objFuelCommonImpl.GetFuelSummary())));
    }
}
FuelCommonImpl& FuelCommonImpl::operator=(const FuelCommonImpl& objFuelCommonImpl)
{
    if(this == &objFuelCommonImpl)
    {
        return *this;
    }

    Initialize();
    CHECK_AND_DELETE(m_fuelDetails)
    if (objFuelCommonImpl.GetFuelDetails())
    {
        m_fuelDetails = new FuelDetailsImpl(*(static_cast<FuelDetailsImpl*>(objFuelCommonImpl.GetFuelDetails())));
    }
    CHECK_AND_DELETE(m_fuelSummary)
    if (objFuelCommonImpl.GetFuelSummary())
    {
        m_fuelSummary = new FuelSummaryImpl(*(static_cast<FuelSummaryImpl*>(objFuelCommonImpl.GetFuelSummary())));
    }

    return *this;
}
FuelDetails* FuelCommonImpl::GetFuelDetails() const
{
    return m_fuelDetails;
}
FuelSummary* FuelCommonImpl::GetFuelSummary() const
{
    return m_fuelSummary;
}
void FuelCommonImpl::SetFuelDetails(FuelDetailsImpl *fuelDetails)
{
    CHECK_AND_DELETE(m_fuelDetails)
    m_fuelDetails = new FuelDetailsImpl(*(static_cast<FuelDetailsImpl*>(fuelDetails)));
}
void FuelCommonImpl::SetFuelSummary(FuelSummaryImpl *fuelSummary)
{
    CHECK_AND_DELETE(m_fuelSummary)
    m_fuelSummary = new FuelSummaryImpl(*(static_cast<FuelSummaryImpl*>(fuelSummary)));
}


//BoundingBoxImpl
BoundingBoxImpl::BoundingBoxImpl(const BoundingBoxImpl& boundingBox)
{

    Initialize();

    CHECK_AND_DELETE(m_topLeftCoords)
    m_topLeftCoords = new LatitudeLongitude();
    m_topLeftCoords->SetLatitude(boundingBox.GetTopLeftCoordinates()->GetLatitude());
    m_topLeftCoords->SetLongitude(boundingBox.GetTopLeftCoordinates()->GetLongitude());

    CHECK_AND_DELETE(m_bottomRightCoords)
    m_bottomRightCoords = new LatitudeLongitude();
    m_bottomRightCoords->SetLatitude(boundingBox.GetBottomRightCoordinates()->GetLatitude());
    m_bottomRightCoords->SetLongitude(boundingBox.GetBottomRightCoordinates()->GetLongitude());

}

BoundingBoxImpl& BoundingBoxImpl::operator=(const BoundingBoxImpl& boundingBox)
{
    if (this == &boundingBox)
    {
        return *this;
    }

    Initialize();
    m_topLeftCoords = new LatitudeLongitude();
    m_topLeftCoords->SetLatitude(boundingBox.GetTopLeftCoordinates()->GetLatitude());
    m_topLeftCoords->SetLongitude(boundingBox.GetTopLeftCoordinates()->GetLongitude());

    m_bottomRightCoords = new LatitudeLongitude();
    m_bottomRightCoords->SetLatitude(boundingBox.GetBottomRightCoordinates()->GetLatitude());
    m_bottomRightCoords->SetLongitude(boundingBox.GetBottomRightCoordinates()->GetLongitude());

     return *this;
}


BoundingBoxImpl::BoundingBoxImpl(void)
{
    Initialize();
}
void BoundingBoxImpl::Initialize()
{
	m_topLeftCoords = NULL;
	m_bottomRightCoords = NULL;
}

BoundingBoxImpl::~BoundingBoxImpl(void)
{
    CHECK_AND_DELETE(m_topLeftCoords)
	CHECK_AND_DELETE(m_bottomRightCoords)
}


LatitudeLongitude* BoundingBoxImpl::GetTopLeftCoordinates(void) const
{
    return m_topLeftCoords;
}

LatitudeLongitude* BoundingBoxImpl::GetBottomRightCoordinates(void) const
{
    return m_bottomRightCoords;
}

void BoundingBoxImpl::SetTopLeftCoordinates(LatitudeLongitude* coords)
{
    CHECK_AND_DELETE(m_topLeftCoords)
    m_topLeftCoords = new LatitudeLongitude(*coords);
}

void BoundingBoxImpl::SetBottomRightCoordinates(LatitudeLongitude* coords)
{
    CHECK_AND_DELETE(m_bottomRightCoords)
    m_bottomRightCoords = new LatitudeLongitude(*coords);
}


