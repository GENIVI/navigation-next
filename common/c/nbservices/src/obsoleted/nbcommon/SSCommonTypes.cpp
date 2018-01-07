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

#include "SSCommonTypes.h"
#include "nbsearchtypes.h"
#include "SSCommonTypesProtected.h"
#include <limits>
using namespace nbcommon2;

#define SET_STRING(class_member, param)        \
       class_member.clear();                   \
       class_member = param;				   \

#define SET_CHAR_STRING(class_member, param)        \
	if(NULL != param){						   \
       class_member.clear();                   \
       class_member = param;}				   \

#define CHECK_AND_DELETE(param)                \
    if (NULL != param) {                       \
    delete param;                              \
    param = NULL;  }                           \

#define CHECK_AND_DELETE_CHAR(param)           \
    if (NULL != param) {                       \
    nsl_free(param);                           \
    param = NULL;  }                           \

#define CHECK_AND_DELETE_VEC(type, param)            \
    for (std::vector<type*>::const_iterator iter  \
            = param.begin();                         \
            iter != param.end(); iter++)             \
    {                                                \
        if (NULL != *iter)                           \
        {                                            \
            delete *iter;                            \
        }                                            \
    }                                                \
    param.clear();                                   \

#define COPY_INSTANCE_PTR_VEC(dsttype, dst, srctype, src, createtype)    \
    for (std::vector<srctype*>::const_iterator iter  \
                = src.begin();                          \
                iter != src.end(); iter++)              \
        {                                               \
            if (NULL != *iter)                          \
            {                                           \
                srctype* ptr = *iter;                      \
                dst.push_back((dsttype*)new createtype(*((createtype*)ptr)));    \
            }                                           \
        }                                               \

#define ASSIGN_AS(type, dst, src)                           \
        if(src)                                             \
        {                                                   \
            if(!dst)                                        \
            {                                               \
                dst = new type();                           \
            }                                               \
            *(type*)(dst) = *static_cast<const type*>(src); \
        }

//CategoryImpl
CategoryImpl::CategoryImpl(const nbcommon2::CategoryImpl &objCategory)
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
CategoryImpl::CategoryImpl(const char* code, const char* name) : Category()
{
    Initialize();
    SET_CHAR_STRING(m_code, code);
    SET_CHAR_STRING(m_name, name);
}
CategoryImpl::~CategoryImpl()
{
	m_code.clear();
	m_name.clear();
 //   CHECK_AND_DELETE_CHAR(m_code)
 //   CHECK_AND_DELETE_CHAR(m_name)
}

void CategoryImpl::Initialize()
{
    m_code = "";
    m_name = "";
}

const std::string& CategoryImpl::GetCode() const
{
    return m_code;
}

const std::string& CategoryImpl::GetName() const
{
    return m_name;
}

void CategoryImpl::SetCode(const char *code)
{
    SET_CHAR_STRING(m_code, code);
}

void CategoryImpl::SetName(const char *name)
{
    SET_CHAR_STRING(m_name, name);
}

//PhoneImpl
PhoneImpl::PhoneImpl(const nbcommon2::PhoneImpl &objPhone)
{

    Initialize();

    SET_STRING(m_area, objPhone.GetArea())
    SET_STRING(m_country, objPhone.GetCountry())
    SET_STRING(m_number, objPhone.GetNumber())
    SET_STRING(m_formattedNumber, objPhone.GetFormattedNumber())

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
    SET_STRING(m_formattedNumber, objPhone.GetFormattedNumber())
    m_phoneType = objPhone.GetPhoneType();

    return *this;
}

PhoneImpl::PhoneImpl()
{
    Initialize();
}
PhoneImpl::~PhoneImpl()
{
  //  CHECK_AND_DELETE_CHAR(m_area)
  //  CHECK_AND_DELETE_CHAR(m_country)
  //  CHECK_AND_DELETE_CHAR(m_number)
}
void PhoneImpl::Initialize()
{
    m_area = "";
    m_country = "";
    m_number = "";
    m_formattedNumber = "";
    m_phoneType = Phone_Primary;
}

const std::string& PhoneImpl::GetArea() const
{
    return m_area;
}
const std::string& PhoneImpl::GetCountry() const
{
    return m_country;
}
const std::string& PhoneImpl::GetNumber() const
{
    return m_number;
}
const Phone::PhoneType PhoneImpl::GetPhoneType() const
{
    return m_phoneType;
}
const std::string& PhoneImpl::GetFormattedNumber() const
{
    return m_formattedNumber;
}

void PhoneImpl::SetArea(const char* area)
{
     SET_CHAR_STRING(m_area, area);
}
void PhoneImpl::SetCountry(const char* country)
{
    SET_CHAR_STRING(m_country, country);
}
void PhoneImpl::SetNumber(const char* number)
{
    SET_CHAR_STRING(m_number, number);
}

void PhoneImpl::SetPhoneType(PhoneType type)
{
    m_phoneType = type;
}

void PhoneImpl::SetFormattedNumber(const char* formattedNumber)
{
    SET_CHAR_STRING(m_formattedNumber, formattedNumber);
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

    m_categories = objPlace.m_categories;

    if (objPlace.GetLocation())
    {
        ASSIGN_AS(LocationImpl, m_location, objPlace.GetLocation());
    }

    COPY_INSTANCE_PTR_VEC(Phone, m_phoneNumbers, Phone, objPlace.GetPhoneNumbers(), PhoneImpl);

    if(objPlace.GetBoundingBox())
    {
        ASSIGN_AS(BoundingBoxImpl, m_boundingBox, objPlace.GetBoundingBox());
    }

    m_averageRating = objPlace.GetAverageRating();
    m_ratingCount = objPlace.GetRatingCount();

    m_poiKeys.clear();
    m_poiKeys = objPlace.GetKeys();

    m_keyValues.clear();
    m_keyValues = objPlace.GetKeyValuePair();

    m_utcOffset = objPlace.m_utcOffset;

    COPY_INSTANCE_PTR_VEC(TimeRange, m_timeRange, TimeRange, objPlace.GetTimeRange(), TimeRangeImpl);

    m_hasUtcOffset = objPlace.m_hasUtcOffset;

    return *this;
}
PlaceImpl::PlaceImpl(const PlaceImpl &objPlace):
                m_location(NULL),
                m_boundingBox(NULL)
{
    *this = objPlace;
}

PlaceImpl::PlaceImpl():
        m_location(NULL),
        m_boundingBox(NULL)
{
    Initialize();
}
PlaceImpl::~PlaceImpl()
{
    m_name.clear();
   // CHECK_AND_DELETE_CHAR(m_name);
    CHECK_AND_DELETE(m_location);
    CHECK_AND_DELETE(m_boundingBox);
    CHECK_AND_DELETE_VEC(Phone, m_phoneNumbers);
    CHECK_AND_DELETE_VEC(TimeRange, m_timeRange);

    m_categories.clear();
}
void PlaceImpl::Initialize()
{
    CHECK_AND_DELETE(m_location);
    CHECK_AND_DELETE(m_boundingBox);
    CHECK_AND_DELETE_VEC(Phone, m_phoneNumbers);
    CHECK_AND_DELETE_VEC(TimeRange, m_timeRange);

    m_categories.clear();
    m_location = new LocationImpl();
    m_name = "";
    m_boundingBox = new BoundingBoxImpl();
    m_averageRating = 0;
    m_ratingCount = 0;
    m_utcOffset = 0;
    m_hasUtcOffset = false;
}

const std::map<CategoryPtrWrapper, std::vector<CategoryPtrWrapper> >& PlaceImpl::GetCategories() const
{
    return m_categories;
}
Location* const PlaceImpl::GetLocation() const
{
    return m_location;
}
const std::string& PlaceImpl::GetName() const
{
    return m_name;
}

const std::vector<Phone*>& PlaceImpl::GetPhoneNumbers() const
{
    return m_phoneNumbers;
}

BoundingBox* const PlaceImpl::GetBoundingBox() const
{
    return m_boundingBox;
}

const std::vector<LS_POIKeys>& PlaceImpl::GetKeys() const
{
    return m_poiKeys;
}

void PlaceImpl::GetKeyValue(LS_POIKeys poiKey, std::string& outValue)
{
    outValue = m_keyValues[poiKey];
}

const double PlaceImpl::GetAverageRating() const
{
    return m_averageRating;
}

const double PlaceImpl::GetRatingCount() const
{
    return m_ratingCount;
}

const std::map<LS_POIKeys, std::string>& PlaceImpl::GetKeyValuePair() const
{
    return m_keyValues;
}

const bool PlaceImpl::GetUtcOffset(int32& utcOffset) const
{
    utcOffset = m_utcOffset;
    return m_hasUtcOffset;
}

const std::vector<TimeRange*>& PlaceImpl::GetTimeRange() const
{
    return m_timeRange;
}

void PlaceImpl::SetCategories(const std::map<CategoryPtrWrapper, std::vector<CategoryPtrWrapper> >& categories)
{
    m_categories = categories;
}
void PlaceImpl::SetLocation(const LocationImpl* location)
{
    ASSIGN_AS(LocationImpl, m_location, location);
}
void PlaceImpl::SetName(const char* name)
{
    SET_CHAR_STRING(m_name, name);
}

void PlaceImpl::SetPhone(PhoneImpl* phonenumber)
{
    std::vector<PhoneImpl*> tmpVec;
    tmpVec.push_back(phonenumber);
    SetPhoneNumbers(tmpVec);
}

void PlaceImpl::SetPhoneNumbers(const std::vector<PhoneImpl*>& phoneNumbers)
{
    CHECK_AND_DELETE_VEC(Phone, m_phoneNumbers);
    for (std::vector<PhoneImpl*>::const_iterator iter
                = phoneNumbers.begin();
                iter != phoneNumbers.end(); iter++)
    {
        if (NULL != *iter)
        {
            m_phoneNumbers.push_back(static_cast<Phone*>(*iter));
        }
    }
}

void PlaceImpl::SetBoundingBox(const BoundingBoxImpl* boundingBox)
{
    ASSIGN_AS(BoundingBoxImpl, m_boundingBox, boundingBox);
}

void PlaceImpl::SetAverageRating(double averageRating)
{
    m_averageRating = averageRating;
}

void PlaceImpl::SetRatingCount(double ratingCount)
{
    m_ratingCount = ratingCount;
}

void PlaceImpl::SetKeyValues(LS_POIKeys key, const std::string& value)
{
    for(std::vector<LS_POIKeys>::iterator iter = m_poiKeys.begin();
            iter != m_poiKeys.end(); iter++)
    {
        if(*iter == key)
        {
            m_keyValues[key] = value;
            return;
        }
    }
    m_poiKeys.push_back(key);
    m_keyValues[key] = value;
}

void PlaceImpl::SetUtcOffset(int32 utcOffset)
{
    m_utcOffset = utcOffset;
}

void PlaceImpl::SetTimeRanges(const std::vector<TimeRangeImpl*>& timeRanges)
{
    CHECK_AND_DELETE_VEC(TimeRange, m_timeRange);
    for (std::vector<TimeRangeImpl*>::const_iterator iter
                = timeRanges.begin();
                iter != timeRanges.end(); iter++)
    {
        if (NULL != *iter)
        {
            m_timeRange.push_back(static_cast<TimeRange*>(*iter));
        }
    }
}

void PlaceImpl::SetUtcOffsetValidityFlag(bool hasUtcOffset)
{
    m_hasUtcOffset = hasUtcOffset;
}

//Location
LocationImpl::LocationImpl(const LocationImpl& location):
        m_coords(NULL),
        m_extendedAddress(NULL),
        m_compactAddress(NULL)
{

    Initialize();

    ASSIGN_AS(LatitudeLongitude, m_coords, location.GetCoordinates());
    ASSIGN_AS(ExtendedAddressImpl, m_extendedAddress, &location.GetExtendedAddress());
    ASSIGN_AS(CompactAddressImpl, m_compactAddress, &location.GetCompactAddress());

    m_name = location.GetName();
    m_number = location.GetNumber();
    m_street = location.GetStreet();
    m_crossStreet = location.GetCrossStreet();
    m_city = location.GetCity();
    m_county = location.GetCounty();
    m_state = location.GetState();
    m_zipCode = location.GetZipCode();
    m_country = location.GetCountry();
    m_airport = location.GetAirport();
    m_countryName = location.GetCountryName();
}

LocationImpl& LocationImpl::operator=(const LocationImpl& location)
{
    if (this == &location)
    {
        return *this;
    }

    Initialize();

    ASSIGN_AS(LatitudeLongitude, m_coords, location.GetCoordinates());
    ASSIGN_AS(ExtendedAddressImpl, m_extendedAddress, &location.GetExtendedAddress());
    ASSIGN_AS(CompactAddressImpl, m_compactAddress, &location.GetCompactAddress());

    m_name = location.GetName();
    m_number = location.GetNumber();
    m_street = location.GetStreet();
    m_crossStreet = location.GetCrossStreet();
    m_city = location.GetCity();
    m_county = location.GetCounty();
    m_state = location.GetState();
    m_zipCode = location.GetZipCode();
    m_country = location.GetCountry();
    m_airport = location.GetAirport();
    m_countryName = location.GetCountryName();

    return *this;
}
//LocationImpl
LocationImpl::LocationImpl(void):
        m_coords(NULL),
        m_extendedAddress(NULL),
        m_compactAddress(NULL)
{
    Initialize();
}
void LocationImpl::Initialize()
{
    CHECK_AND_DELETE(m_coords);
    CHECK_AND_DELETE(m_compactAddress);
    CHECK_AND_DELETE(m_extendedAddress);
    m_coords = new LatitudeLongitude();
    m_compactAddress = static_cast<CompactAddress*>(new CompactAddressImpl());
    m_extendedAddress = static_cast<ExtendedAddress*>(new ExtendedAddressImpl());
    m_type = Location_None;
    m_name = "";
    m_number = "";
    m_street = "";
    m_crossStreet = "";
    m_city = "";
    m_county = "";
    m_state = "";
    m_zipCode = "";
    m_country = "";
    m_airport = "";
}

LocationImpl::~LocationImpl(void)
{
    CHECK_AND_DELETE(m_coords);
    CHECK_AND_DELETE(m_compactAddress);
    CHECK_AND_DELETE(m_extendedAddress);
}

const Location::LocationType LocationImpl::GetType(void) const
{
    return m_type;
}

LatitudeLongitude* const LocationImpl::GetCoordinates(void) const
{
    return m_coords;
}

const std::string& LocationImpl::GetName(void) const
{
    return m_name;
}

const std::string& LocationImpl::GetNumber(void) const
{
    return m_number;
}

const std::string& LocationImpl::GetStreet(void) const
{
    return m_street;
}

const std::string& LocationImpl::GetCrossStreet(void) const
{
    return m_crossStreet;
}

const std::string& LocationImpl::GetCity(void) const
{
    return m_city;
}

const std::string& LocationImpl::GetCounty(void) const
{
    return m_county;
}

const std::string& LocationImpl::GetState(void) const
{
    return m_state;
}

const std::string& LocationImpl::GetZipCode(void) const
{
    return m_zipCode;
}

const std::string& LocationImpl::GetCountry(void) const
{
    return m_country;
}

const std::string& LocationImpl::GetAirport(void) const
{
    return m_airport;
}

const std::string& LocationImpl::GetCountryName(void) const
{
    return m_countryName;
}

const ExtendedAddress& LocationImpl::GetExtendedAddress(void) const
{
    return *m_extendedAddress;
}
const CompactAddress& LocationImpl::GetCompactAddress(void) const
{
    return *m_compactAddress;
}


void LocationImpl::SetAirport(const char *airport)
{
    SET_CHAR_STRING(m_airport, airport)
}

void LocationImpl::SetCity(const char* city)
{
    SET_CHAR_STRING(m_city, city)
}
void LocationImpl::SetCoordinates(const LatitudeLongitude* coords)
{
    *m_coords = *coords;
}
void LocationImpl::SetCountry(const char* country)
{
    SET_CHAR_STRING(m_country, country)
}
void LocationImpl::SetCountryName(const char* countryName)
{
    SET_CHAR_STRING(m_countryName, countryName);
}
void LocationImpl::SetCounty(const char* county)
{
    SET_CHAR_STRING(m_county, county)
}
void LocationImpl::SetCrossStreet(const char *crossStreet)
{
    SET_CHAR_STRING(m_crossStreet, crossStreet)
}
void LocationImpl::SetName(const char *name)
{
    SET_CHAR_STRING(m_name, name)
}
void LocationImpl::SetNumber(const char *number)
{
    SET_CHAR_STRING(m_number, number)
}
void LocationImpl::SetState(const char* state)
{
    SET_CHAR_STRING(m_state, state)
}
void LocationImpl::SetStreet(const char* street)
{
    SET_CHAR_STRING(m_street, street)
}
void LocationImpl::SetType(Location::LocationType type)
{
    m_type = type;
}

void LocationImpl::SetZipCode(const char* zipCode)
{
    SET_CHAR_STRING(m_zipCode, zipCode)
}

void LocationImpl::SetExtendedAddress(const ExtendedAddress* extendedAddress)
{
    ASSIGN_AS(ExtendedAddressImpl, m_extendedAddress, extendedAddress);
}

void LocationImpl::SetCompactAddress(const CompactAddress* compactAddress)
{
    ASSIGN_AS(CompactAddressImpl, m_compactAddress, compactAddress);
}
//ExtendedAddressImpl

ExtendedAddressImpl::ExtendedAddressImpl()
{
    Initialize();
}

ExtendedAddressImpl::~ExtendedAddressImpl()
{
}

ExtendedAddressImpl::ExtendedAddressImpl(const ExtendedAddressImpl& objExtendedAddressImpl)
{
    Initialize();
    m_vecBuffer = objExtendedAddressImpl.m_vecBuffer;
}

ExtendedAddressImpl& ExtendedAddressImpl::operator=(const ExtendedAddressImpl& objExtendedAddressImpl)
{
    if (this == &objExtendedAddressImpl)
    {
        return *this;
    }

    Initialize();
    m_vecBuffer = objExtendedAddressImpl.m_vecBuffer;

    return *this;
}

void ExtendedAddressImpl::SetFirstLine(const char* firstLine)
{
    std::vector<std::string>::iterator iter = m_vecBuffer.begin();
    *iter = std::string(firstLine);
}
void ExtendedAddressImpl::SetSecondLine(const char* secondLine)
{
    std::vector<std::string>::iterator iter = m_vecBuffer.begin();
    *(iter + 1) = std::string(secondLine);
}
void ExtendedAddressImpl::SetThirdLine(const char* thirdLine)
{
    std::vector<std::string>::iterator iter = m_vecBuffer.begin();
    *(iter + 2) = std::string(thirdLine);
}
void ExtendedAddressImpl::SetFourthLine(const char* fourthLine)
{
    std::vector<std::string>::iterator iter = m_vecBuffer.begin();
    *(iter + 3) = std::string(fourthLine);
}
void ExtendedAddressImpl::SetFifthLine(const char* fifthLine)
{
    std::vector<std::string>::iterator iter = m_vecBuffer.begin();
    *(iter + 4) = std::string(fifthLine);
}

void ExtendedAddressImpl::SetLine(uint index, const char* line)
{
    while (m_vecBuffer.size() <= index)
    {
        m_vecBuffer.push_back("");
    }
    std::vector<std::string>::iterator iter = m_vecBuffer.begin();
    *(iter + index) = std::string(line);
}

const std::vector<std::string>& ExtendedAddressImpl::GetLines() const
{
    return m_vecBuffer;
}

void ExtendedAddressImpl::Initialize()
{
    m_vecBuffer.clear();
    m_vecBuffer.push_back("");
    m_vecBuffer.push_back("");
    m_vecBuffer.push_back("");
    m_vecBuffer.push_back("");
    m_vecBuffer.push_back("");
}

//CompactAddressImpl
CompactAddressImpl::CompactAddressImpl()
{
    Initialize();
}

CompactAddressImpl::~CompactAddressImpl()
{
}

CompactAddressImpl::CompactAddressImpl(const CompactAddressImpl& objCompactAddressImpl)
{
    Initialize();
    m_firstLine = objCompactAddressImpl.m_firstLine;
    m_secondLine = objCompactAddressImpl.m_secondLine;
    m_lineSeparator = objCompactAddressImpl.m_lineSeparator;
    m_formattedAddress = objCompactAddressImpl.m_formattedAddress;
}

CompactAddressImpl& CompactAddressImpl::operator=(const CompactAddressImpl& objCompactAddressImpl)
{
    if (this == &objCompactAddressImpl)
    {
        return *this;
    }
    Initialize();
    m_firstLine = objCompactAddressImpl.m_firstLine;
    m_secondLine = objCompactAddressImpl.m_secondLine;
    m_lineSeparator = objCompactAddressImpl.m_lineSeparator;
    m_formattedAddress = objCompactAddressImpl.m_formattedAddress;
    return *this;
}

void CompactAddressImpl::SetFirstLine(const char* firstLine)
{
    SET_CHAR_STRING(m_firstLine, firstLine);
}

void CompactAddressImpl::SetSecondLine(const char* secondLine)
{
    SET_CHAR_STRING(m_secondLine, secondLine);
}

void CompactAddressImpl::SetLineSeparator(const char* separator)
{
    SET_CHAR_STRING(m_lineSeparator, separator);

    if(separator != NULL)
    {
    	if(m_secondLine.empty())
    	{
    		m_formattedAddress = m_firstLine;
    	}
    	else
    	{
    		m_formattedAddress = m_firstLine + m_lineSeparator + m_secondLine;
    	}

    }
    else
    {
    	if(m_secondLine.empty())
		{
			m_formattedAddress = m_firstLine;
		}
		else
		{
			m_formattedAddress = m_firstLine + " " + m_secondLine;
		}

    }
}

const std::string& CompactAddressImpl::GetLineSeparator() const
{
    return m_lineSeparator;
}

const std::string& CompactAddressImpl::GetLine1() const
{
    return m_firstLine;
}

const std::string& CompactAddressImpl::GetLine2() const
{
    return m_secondLine;
}

const std::string& CompactAddressImpl::GetAddressLine() const
{
	return m_formattedAddress;
}

void CompactAddressImpl::Initialize()
{
    m_firstLine = "";
    m_secondLine = "";
    m_lineSeparator = "";
    m_formattedAddress = "";
}

//Address
AddressImpl::AddressImpl(const AddressImpl& addr)
{

    Initialize();

    if (!addr.GetNumber().empty())
    {
        m_number = addr.GetNumber();
    }
    if (!addr.GetStreet().empty())
    {
        m_street = addr.GetStreet();
    }
    if (!addr.GetCity().empty())
    {
        m_city = addr.GetCity();
    }
    if (!addr.GetCounty().empty())
    {
        m_county = addr.GetCounty();
    }
    if (!addr.GetState().empty())
    {
        m_state = addr.GetState();
    }
    if (!addr.GetZipCode().empty())
    {
        m_zipCode = addr.GetZipCode();
    }
    if (!addr.GetCountry().empty())
    {
        m_country = addr.GetCountry();
    }
}
AddressImpl& AddressImpl::operator=(const AddressImpl& addr)
{
    if(this == &addr)
    {
        return *this;
    }

    Initialize();

    if (!addr.GetNumber().empty())
    {
        m_number = addr.GetNumber();
    }
    if (!addr.GetStreet().empty())
    {
        m_street = addr.GetStreet();
    }
    if (!addr.GetCity().empty())
    {
        m_city = addr.GetCity();
    }
    if (!addr.GetCounty().empty())
    {
        m_county = addr.GetCounty();
    }
    if (!addr.GetState().empty())
    {
        m_state = addr.GetState();
    }
    if (!addr.GetZipCode().empty())
    {
        m_zipCode = addr.GetZipCode();
    }
    if (!addr.GetCountry().empty())
    {
        m_country = addr.GetCountry();
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
    m_number = "";
    m_street = "";
    m_city = "";
    m_county = "";
    m_state = "";
    m_zipCode = "";
    m_country = "";
}

AddressImpl::~AddressImpl(void)
{
   // CHECK_AND_DELETE_CHAR(m_number)
  //  CHECK_AND_DELETE_CHAR(m_street)
 //   CHECK_AND_DELETE_CHAR(m_city)
  //  CHECK_AND_DELETE_CHAR(m_county)
  //  CHECK_AND_DELETE_CHAR(m_state)
  //  CHECK_AND_DELETE_CHAR(m_zipCode)
  //  CHECK_AND_DELETE_CHAR(m_country)
}

const std::string& AddressImpl::GetNumber(void) const
{
    return m_number;
}

const std::string& AddressImpl::GetStreet(void) const
{
    return m_street;
}

const std::string& AddressImpl::GetCity(void) const
{
    return m_city;
}

const std::string& AddressImpl::GetCounty(void) const
{
    return m_county;
}

const std::string& AddressImpl::GetState(void) const
{
    return m_state;
}

const std::string& AddressImpl::GetZipCode(void) const
{
    return m_zipCode;
}

const std::string& AddressImpl::GetCountry(void) const
{
    return m_country;
}

void AddressImpl::SetCity(const char *city)
{
    SET_CHAR_STRING(m_city, city)
}
void AddressImpl::SetCountry(const char* country)
{
    SET_CHAR_STRING(m_country, country)
}
void AddressImpl::SetCounty(const char* county)
{
    SET_CHAR_STRING(m_county, county)
}
void AddressImpl::SetNumber(const char* number)
{
    SET_CHAR_STRING(m_number, number)
}

void AddressImpl::SetState(const char* state)
{
    SET_CHAR_STRING(m_state, state)
}
void AddressImpl::SetStreet(const char* street)
{
    SET_CHAR_STRING(m_street, street)
}
void AddressImpl::SetZipCode(const char* zipCode)
{
    SET_CHAR_STRING(m_zipCode, zipCode)
}
//SuggestionMatch
SuggestionMatchImpl::SuggestionMatchImpl()
{
    Initialize();
}

SuggestionMatchImpl::~SuggestionMatchImpl()
{
   // CHECK_AND_DELETE_CHAR(m_firstLine);
  //  CHECK_AND_DELETE_CHAR(m_secondLine);
  //  CHECK_AND_DELETE_CHAR(m_thirdLine);
   // CHECK_AND_DELETE_CHAR(m_suggestionCookie);
}
SuggestionMatchImpl& SuggestionMatchImpl::operator=(const SuggestionMatchImpl& objSuggestionMatch)
{

    // Handle self-assignment:
    if(this == &objSuggestionMatch)
    {
        return *this;
    }

    Initialize();

    if ((objSuggestionMatch.GetFirstLine()).empty())
    {
        SET_STRING(m_firstLine, objSuggestionMatch.GetFirstLine());
    }
    if (objSuggestionMatch.GetSecondLine().empty())
    {
        SET_STRING(m_secondLine, objSuggestionMatch.GetSecondLine());
    }
	if (objSuggestionMatch.GetThirdLine().empty())
    {
        SET_STRING(m_thirdLine, objSuggestionMatch.GetThirdLine());
    }
    if (objSuggestionMatch.GetSuggestionCookie().empty())
    {
        SET_STRING(m_suggestionCookie, objSuggestionMatch.GetSuggestionCookie());
    }

    m_distance = objSuggestionMatch.GetDistance();

    m_matchType = objSuggestionMatch.GetMatchType();

    return *this;
}
SuggestionMatchImpl::SuggestionMatchImpl(const nbcommon2::SuggestionMatchImpl &objSuggestionMatch)
{
    Initialize();


    if (!objSuggestionMatch.GetFirstLine().empty())
    {
        SET_STRING(m_firstLine, objSuggestionMatch.GetFirstLine());
    }
    if (!objSuggestionMatch.GetSecondLine().empty())
    {
        SET_STRING(m_secondLine, objSuggestionMatch.GetSecondLine());
    }
    if (!objSuggestionMatch.GetThirdLine().empty())
    {
        SET_STRING(m_thirdLine, objSuggestionMatch.GetThirdLine());
    }
    if (!objSuggestionMatch.GetSuggestionCookie().empty())
    {
        SET_STRING(m_suggestionCookie, objSuggestionMatch.GetSuggestionCookie());
    }


    m_distance = objSuggestionMatch.GetDistance();

    m_matchType = objSuggestionMatch.GetMatchType();

}
//SuggestionMatchImpl
void SuggestionMatchImpl::Initialize()
{
    m_firstLine = "";
    m_secondLine = "";
    m_thirdLine = "";
    m_suggestionCookie = "";
    m_distance = 0;

}
const std::string& SuggestionMatchImpl::GetFirstLine() const
{
    return m_firstLine;
}
const std::string& SuggestionMatchImpl::GetSecondLine() const
{
    return m_secondLine;
}
const std::string& SuggestionMatchImpl::GetThirdLine() const
{
    return m_thirdLine;
}
const NB_MatchType SuggestionMatchImpl::GetMatchType() const
{
    return m_matchType;
}

const double SuggestionMatchImpl::GetDistance() const
{
    return m_distance;
}
const std::string& SuggestionMatchImpl::GetSuggestionCookie() const
{
    return m_suggestionCookie;
}


void SuggestionMatchImpl::SetFirstLine(const char *firstLine)
{
    SET_CHAR_STRING(m_firstLine, firstLine);
}
void SuggestionMatchImpl::SetSecondLine(const char *secondLine)
{
    SET_CHAR_STRING(m_secondLine, secondLine);
}
void SuggestionMatchImpl::SetThirdLine(const char *thirdLine)
{
    SET_CHAR_STRING(m_thirdLine, thirdLine);
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
	 SET_CHAR_STRING(m_suggestionCookie, suggestionCookie);
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

LatitudeLongitude::LatitudeLongitude(const nbcommon2::LatitudeLongitude &objLatitudeLongitude)
{
    m_latitude = objLatitudeLongitude.GetLatitude();
    m_longitude = objLatitudeLongitude.GetLongitude();
}
const double LatitudeLongitude::GetLatitude() const
{
    return m_latitude;
}

const double LatitudeLongitude::GetLongitude() const
{
    return m_longitude;
}

void LatitudeLongitude::Initialize()
{
    m_latitude = std::numeric_limits<double>::quiet_NaN();
    m_longitude = std::numeric_limits<double>::quiet_NaN();
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
    m_productName = "";
    m_code = "";
    m_typeName = "";
}
FuelTypeImpl::FuelTypeImpl()
{
    Initialize();
}
FuelTypeImpl::~FuelTypeImpl()
{
   /* CHECK_AND_DELETE_CHAR(m_productName)
    CHECK_AND_DELETE_CHAR(m_code)
    CHECK_AND_DELETE_CHAR(m_typeName)*/
}
FuelTypeImpl::FuelTypeImpl(const nbcommon2::FuelTypeImpl &objFuelTypeImpl)
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
const std::string& FuelTypeImpl::GetCode() const
{
    return m_code;
}
const std::string& FuelTypeImpl::GetProductName() const
{
    return m_productName;
}
const std::string& FuelTypeImpl::GetTypeName() const
{
    return m_typeName;
}
void FuelTypeImpl::SetCode(const char *code)
{
    SET_CHAR_STRING(m_code, code)
}
void FuelTypeImpl::SetProductName(const char *productName)
{
    SET_CHAR_STRING(m_productName, productName)
}
void FuelTypeImpl::SetTypeName(const char *typeName)
{
    SET_CHAR_STRING(m_typeName, typeName)
}

//PriceImpl
void PriceImpl::Initialize()
{
    m_currency = "";
}
PriceImpl::PriceImpl()
{
    Initialize();
}
PriceImpl::~PriceImpl()
{
    //CHECK_AND_DELETE_CHAR(m_currency)
}
PriceImpl::PriceImpl(const nbcommon2::PriceImpl &objPriceImpl)
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
const double PriceImpl::GetValue() const
{
    return m_value;
}
const uint32 PriceImpl::GetModTime() const
{
    return m_modTime;
}
const std::string& PriceImpl::GetCurrency() const
{
    return m_currency;
}
void PriceImpl::SetCurrency(const char *currency)
{
    SET_CHAR_STRING(m_currency, currency)
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
    CHECK_AND_DELETE(m_price);
    CHECK_AND_DELETE(m_fuelType);
    m_units = "";
}
FuelProductImpl::FuelProductImpl():
        m_price(NULL)
{
    Initialize();
}
FuelProductImpl::~FuelProductImpl()
{
    CHECK_AND_DELETE(m_price);
    CHECK_AND_DELETE(m_fuelType);
}
FuelProductImpl::FuelProductImpl(const nbcommon2::FuelProductImpl &objFuelProductImpl):
        m_price(NULL)
{
    Initialize();
    ASSIGN_AS(PriceImpl, m_price, objFuelProductImpl.GetPrice());
    SET_STRING(m_units, objFuelProductImpl.GetUnits());
    ASSIGN_AS(FuelTypeImpl, m_fuelType, objFuelProductImpl.GetFuelType());
}
FuelProductImpl& FuelProductImpl::operator=(const FuelProductImpl& objFuelProductImpl)
{
    // Handle self-assignment:
    if(this == &objFuelProductImpl)
    {
        return *this;
    }

    Initialize();
    ASSIGN_AS(PriceImpl, m_price, objFuelProductImpl.GetPrice());
    SET_STRING(m_units, objFuelProductImpl.GetUnits());
    ASSIGN_AS(FuelTypeImpl, m_fuelType, objFuelProductImpl.GetFuelType());

    return *this;
}
Price* const FuelProductImpl::GetPrice() const
{
    return m_price;
}
const std::string& FuelProductImpl::GetUnits() const
{
    return m_units;
}
FuelType* const FuelProductImpl::GetFuelType() const
{
    return m_fuelType;
}
void FuelProductImpl::SetFuelType(const FuelTypeImpl *fuelType)
{
    ASSIGN_AS(FuelTypeImpl, m_fuelType, fuelType);
}
void FuelProductImpl::SetUnits(const char *units)
{
    SET_CHAR_STRING(m_units, units)
}
void FuelProductImpl::SetPrice(const PriceImpl *price)
{
    ASSIGN_AS(PriceImpl, m_price, price);
}

//FuelSummaryImpl
void FuelSummaryImpl::Initialize()
{
    CHECK_AND_DELETE(m_average);
    CHECK_AND_DELETE(m_low);
}
FuelSummaryImpl::FuelSummaryImpl():
        m_average(NULL),
        m_low(NULL)
{
    Initialize();
}
FuelSummaryImpl::~FuelSummaryImpl()
{
    CHECK_AND_DELETE(m_average)
    CHECK_AND_DELETE(m_low)
}
FuelSummaryImpl::FuelSummaryImpl(const nbcommon2::FuelSummaryImpl &objFuelSummaryImpl)
{
    Initialize();
    ASSIGN_AS(FuelProductImpl, m_average, objFuelSummaryImpl.GetAverage());
    ASSIGN_AS(FuelProductImpl, m_low, objFuelSummaryImpl.GetLow());
}
FuelSummaryImpl& FuelSummaryImpl::operator=(const FuelSummaryImpl& objFuelSummaryImpl)
{
    if(this == &objFuelSummaryImpl)
    {
        return *this;
    }

    Initialize();
    ASSIGN_AS(FuelProductImpl, m_average, objFuelSummaryImpl.GetAverage());
    ASSIGN_AS(FuelProductImpl, m_low, objFuelSummaryImpl.GetLow());

    return *this;
}
FuelProduct* const FuelSummaryImpl::GetAverage() const
{
    return m_average;
}
FuelProduct* const FuelSummaryImpl::GetLow() const
{
    return m_low;
}
void FuelSummaryImpl::SetAverage(const FuelProduct *average)
{
    ASSIGN_AS(FuelProductImpl, m_average, average);
}
void FuelSummaryImpl::SetLow(const FuelProduct *low)
{
    ASSIGN_AS(FuelProductImpl, m_low, low);
}
//FuelDetailsImpl
void FuelDetailsImpl::Initialize()
{
    for (int i = 0; i < NB_FUEL_PRODUCTS_NUM; i++)
    {
        CHECK_AND_DELETE(m_fuelProduct[i]);
    }
    m_numFuelProducts = 0;
}
FuelDetailsImpl::FuelDetailsImpl():
        m_fuelProduct({NULL})
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
FuelDetailsImpl::FuelDetailsImpl(const nbcommon2::FuelDetailsImpl &objFuelDetailsImpl):
        m_fuelProduct({NULL})
{
    Initialize();
    for (int i = 0; i < objFuelDetailsImpl.GetNumFuelProducts(); i++)
    {
        ASSIGN_AS(FuelProductImpl, m_fuelProduct[i], objFuelDetailsImpl.GetFuelProduct(i));
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
        ASSIGN_AS(FuelProductImpl, m_fuelProduct[i], objFuelDetailsImpl.GetFuelProduct(i));
    }
    m_numFuelProducts = objFuelDetailsImpl.GetNumFuelProducts();

    return *this;
}
FuelProduct* const FuelDetailsImpl::GetFuelProduct(int index) const
{
    if (index < 0 || index > m_numFuelProducts)
    {
        //out of range
        return NULL;
    }
    return m_fuelProduct[index];
}
const int FuelDetailsImpl::GetNumFuelProducts() const
{
    return m_numFuelProducts;
}
void FuelDetailsImpl::SetFuelProduct(int index, const FuelProductImpl *fuelProduct)
{
    if (index < 0 || index > m_numFuelProducts)
    {
        //out of range
        return;
    }
    ASSIGN_AS(FuelProductImpl, m_fuelProduct[index], fuelProduct);
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
    CHECK_AND_DELETE(m_fuelDetails);
    CHECK_AND_DELETE(m_fuelSummary);
}
FuelCommonImpl::FuelCommonImpl():
        m_fuelSummary(NULL),
        m_fuelDetails(NULL)
{
    Initialize();
}
FuelCommonImpl::FuelCommonImpl(const FuelDetailsImpl* fuelDetails):
                m_fuelSummary(NULL),
                m_fuelDetails(NULL)
{
    Initialize();
    ASSIGN_AS(FuelDetailsImpl, m_fuelDetails, fuelDetails);
}
FuelCommonImpl::FuelCommonImpl(const FuelSummaryImpl* fuelSummary):
                m_fuelSummary(NULL),
                m_fuelDetails(NULL)
{
    Initialize();
    ASSIGN_AS(FuelSummaryImpl, m_fuelSummary, fuelSummary);
}
FuelCommonImpl::~FuelCommonImpl()
{
    CHECK_AND_DELETE(m_fuelDetails)
    CHECK_AND_DELETE(m_fuelSummary)
}
FuelCommonImpl::FuelCommonImpl(const nbcommon2::FuelCommonImpl &objFuelCommonImpl)
{
    Initialize();
    ASSIGN_AS(FuelSummaryImpl, m_fuelSummary, objFuelCommonImpl.GetFuelSummary());
    ASSIGN_AS(FuelDetailsImpl, m_fuelDetails, objFuelCommonImpl.GetFuelDetails());
}
FuelCommonImpl& FuelCommonImpl::operator=(const FuelCommonImpl& objFuelCommonImpl)
{
    if(this == &objFuelCommonImpl)
    {
        return *this;
    }

    Initialize();
    ASSIGN_AS(FuelSummaryImpl, m_fuelSummary, objFuelCommonImpl.GetFuelSummary());
    ASSIGN_AS(FuelDetailsImpl, m_fuelDetails, objFuelCommonImpl.GetFuelDetails());

    return *this;
}
FuelDetails* const FuelCommonImpl::GetFuelDetails() const
{
    return m_fuelDetails;
}
FuelSummary* const FuelCommonImpl::GetFuelSummary() const
{
    return m_fuelSummary;
}
void FuelCommonImpl::SetFuelDetails(const FuelDetailsImpl *fuelDetails)
{
    ASSIGN_AS(FuelDetailsImpl, m_fuelDetails, fuelDetails);
}
void FuelCommonImpl::SetFuelSummary(const FuelSummaryImpl *fuelSummary)
{
    ASSIGN_AS(FuelSummaryImpl, m_fuelSummary, fuelSummary);
}


//BoundingBoxImpl
BoundingBoxImpl::BoundingBoxImpl(const BoundingBoxImpl& boundingBox):
                m_topLeftCoords(NULL),
                m_bottomRightCoords(NULL)
{

    Initialize();
    ASSIGN_AS(LatitudeLongitude, m_topLeftCoords, boundingBox.GetTopLeftCoordinates());
    ASSIGN_AS(LatitudeLongitude, m_bottomRightCoords, boundingBox.GetBottomRightCoordinates());
}

BoundingBoxImpl& BoundingBoxImpl::operator=(const BoundingBoxImpl& boundingBox)
{
    if (this == &boundingBox)
    {
        return *this;
    }

    Initialize();
    ASSIGN_AS(LatitudeLongitude, m_topLeftCoords, boundingBox.GetTopLeftCoordinates());
    ASSIGN_AS(LatitudeLongitude, m_bottomRightCoords, boundingBox.GetBottomRightCoordinates());

    return *this;
}


BoundingBoxImpl::BoundingBoxImpl(void):
        m_topLeftCoords(NULL),
        m_bottomRightCoords(NULL)
{
    Initialize();
}
void BoundingBoxImpl::Initialize()
{
    CHECK_AND_DELETE(m_topLeftCoords);
    CHECK_AND_DELETE(m_bottomRightCoords);
    m_topLeftCoords = new LatitudeLongitude();
    m_bottomRightCoords = new LatitudeLongitude();
}

BoundingBoxImpl::~BoundingBoxImpl(void)
{
    CHECK_AND_DELETE(m_topLeftCoords)
	CHECK_AND_DELETE(m_bottomRightCoords)
}


LatitudeLongitude* const BoundingBoxImpl::GetTopLeftCoordinates(void) const
{
    return m_topLeftCoords;
}

LatitudeLongitude* const BoundingBoxImpl::GetBottomRightCoordinates(void) const
{
    return m_bottomRightCoords;
}

void BoundingBoxImpl::SetTopLeftCoordinates(LatitudeLongitude* coords)
{
    ASSIGN_AS(LatitudeLongitude, m_topLeftCoords, coords);
}

void BoundingBoxImpl::SetBottomRightCoordinates(LatitudeLongitude* coords)
{
    ASSIGN_AS(LatitudeLongitude, m_bottomRightCoords, coords);
}

//TimeRangeImpl
TimeRangeImpl::TimeRangeImpl(const TimeRangeImpl& objTimeRangeImpl)
{
    *this = objTimeRangeImpl;
}

TimeRangeImpl& TimeRangeImpl::operator=(const TimeRangeImpl& objTimeRangeImpl)
{
    if (this == &objTimeRangeImpl)
    {
        return *this;
    }

    Initialize();

    m_startHour = objTimeRangeImpl.m_startHour;
    m_startMin = objTimeRangeImpl.m_startMin;
    m_endHour = objTimeRangeImpl.m_endHour;
    m_endMin = objTimeRangeImpl.m_endMin;
    m_dayOfWeek = objTimeRangeImpl.m_dayOfWeek;

    return *this;
}

TimeRangeImpl::TimeRangeImpl()
{
    Initialize();
}

void TimeRangeImpl::Initialize()
{
    m_startHour = 0;
    m_startMin = 0;
    m_endHour = 0;
    m_endMin = 0;
    m_dayOfWeek = NB_DOW_Sunday;
}

TimeRangeImpl::~TimeRangeImpl(void)
{
}

void TimeRangeImpl::SetStartHour(short startHour)
{
    m_startHour = startHour;
}

void TimeRangeImpl::SetStartMinute(short startMin)
{
    m_startMin = startMin;
}

void TimeRangeImpl::SetEndHour(short endHour)
{
    m_endHour = endHour;
}

void TimeRangeImpl::SetEndMinute(short endMin)
{
    m_endMin = endMin;
}

void TimeRangeImpl::SetDayOfWeek(NB_DayOfWeek dayOfWeek)
{
    m_dayOfWeek = dayOfWeek;
}

const short TimeRangeImpl::GetStartHour() const
{
    return m_startHour;
}

const short TimeRangeImpl::GetStartMinute() const
{
    return m_startMin;
}

const short TimeRangeImpl::GetEndHour() const
{
    return m_endHour;
}

const short TimeRangeImpl::GetEndMinute() const
{
    return m_endMin;
}

const  NB_DayOfWeek TimeRangeImpl::GetDayOfWeek() const
{
    return m_dayOfWeek;
}
