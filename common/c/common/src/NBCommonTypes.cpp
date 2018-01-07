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
 @file     NBCommonTypes.cpp
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

#include "NBCommonTypesImpl.h"

using namespace nbcommon;

ExtendedAddressImpl::ExtendedAddressImpl(const ExtendedAddress& copy)
    :m_lines(copy.GetLines())
{
}

ExtendedAddressImpl::ExtendedAddressImpl(const ExtendedAddressImpl& copy)
    :m_lines(copy.GetLines())
{
}

ExtendedAddressImpl::ExtendedAddressImpl(protocol::ExtendedAddressSharedPtr extendedAddress)
{
    shared_ptr<std::vector<shared_ptr<protocol::AddressLine> > > addresses = extendedAddress->GetAddressLineArray();
    std::vector<shared_ptr<protocol::AddressLine> >::iterator it = addresses->begin();
    for (; it != addresses->end(); ++it)
    {
        m_lines.push_back(*(*it)->GetText());
    }
}

CompactAddressImpl::CompactAddressImpl(const CompactAddress& copy)
    : m_lineSeparator(copy.GetLineSeparator()),
      m_line1(copy.GetLine1()),
      m_line2(copy.GetLine2()),
      m_addressLine(copy.GetAddressLine())
{
}

CompactAddressImpl::CompactAddressImpl(const CompactAddressImpl& copy)
    : m_lineSeparator(copy.GetLineSeparator()),
      m_line1(copy.GetLine1()),
      m_line2(copy.GetLine2()),
      m_addressLine(copy.GetAddressLine())
{
}

CompactAddressImpl::CompactAddressImpl(protocol::CompactAddressSharedPtr compactaddress)
    : m_lineSeparator(*compactaddress->GetLineSeparator()->GetSeparator())
{
    protocol::AddressLineSharedPtrList addresses = compactaddress->GetAddressLineArray();
    protocol::AddressLineSharedPtrArray::iterator it = addresses->begin();
    int size = addresses->size();
    std::string lines[2] = {""};
    for (int i = 0; it != addresses->end() && i < size; ++it, ++i)
    {
        lines[i] = *(*it)->GetText();
    }
    m_addressLine += lines[0] + m_lineSeparator + lines[1];
    m_line1 = lines[0];
    m_line2 = lines[1];
}

NBLocationImpl::NBLocationImpl()
    : m_type(Location_None),
      m_point(NULL),
      m_extAddress(NULL),
      m_compAddress(NULL)
{
}

NBLocationImpl::NBLocationImpl(const NBLocationImpl& copy)
    : m_type(copy.m_type),
      m_point(copy.m_point != NULL ? new LatitudeLongitude(*copy.m_point) : NULL),
      m_name(copy.m_name),
      m_number(copy.m_number),
      m_street(copy.m_street),
      m_crossStreet(copy.m_crossStreet),
      m_city(copy.m_city),
      m_county(copy.m_county),
      m_state(copy.m_state),
      m_zipcode(copy.m_zipcode),
      m_country(copy.m_country),
      m_airport(copy.m_airport),
      m_countryName(copy.m_countryName),
      m_freeForm(copy.m_freeForm),
      m_extAddress(copy.m_extAddress != NULL ? new ExtendedAddressImpl(*copy.m_extAddress) : NULL),
      m_compAddress(copy.m_compAddress != NULL ? new CompactAddressImpl(*copy.m_compAddress) : NULL),
      m_UTCOffset(copy.GetUTCOffset())
{
}

NBLocationImpl::NBLocationImpl(const Location& copy)
    : m_type(copy.GetType()),
      m_point(copy.GetCoordinates() != NULL ? new LatitudeLongitude(*copy.GetCoordinates()) : NULL),
      m_name(copy.GetName()),
      m_number(copy.GetNumber()),
      m_street(copy.GetStreet()),
      m_crossStreet(copy.GetCrossStreet()),
      m_city(copy.GetCity()),
      m_county(copy.GetCounty()),
      m_state(copy.GetState()),
      m_zipcode(copy.GetZipCode()),
      m_country(copy.GetCountry()),
      m_airport(copy.GetAirport()),
      m_countryName(copy.GetCountry()),
      m_freeForm(copy.GetFreeForm()),
      m_extAddress(copy.GetExtendedAddress() != NULL ? new ExtendedAddressImpl(*copy.GetExtendedAddress()) : NULL),
      m_compAddress(copy.GetCompactAddress() != NULL ? new CompactAddressImpl(*copy.GetCompactAddress()) : NULL),
      m_UTCOffset(copy.GetUTCOffset())
{
}

NBLocationImpl::~NBLocationImpl()
{
    delete m_point;
    m_point = NULL;
    delete m_extAddress;
    m_extAddress = NULL;
    delete m_compAddress;
    m_compAddress = NULL;
}

NBLocationImpl::NBLocationImpl(protocol::LocationSharedPtr location)
    : m_type(Location_None),
      m_point(new LatitudeLongitude(location->GetPoint()->GetLat(), location->GetPoint()->GetLon())),
      m_name(*location->GetName()),
      m_number(*location->GetAddress()->GetStreetAddress()),
      m_street(*location->GetAddress()->GetStreet()),
      m_crossStreet(*location->GetAddress()->GetCrossStreet()),
      m_city(*location->GetAddress()->GetCity()),
      m_county(*location->GetAddress()->GetCounty()),
      m_state(*location->GetAddress()->GetState()),
      m_zipcode(*location->GetAddress()->GetPostal()),
      m_country(*location->GetAddress()->GetCountry()),
      m_airport(*location->GetAddress()->GetAirport()),
      m_countryName(*location->GetAddress()->GetCountryName()),
      m_freeForm(*location->GetAddress()->GetFreeform()),
      m_extAddress(NULL),
      m_compAddress(NULL),
      m_UTCOffset(0)
{
    if (location->GetAddress()->GetType()->compare("street") == 0)
    {
        m_type = Location_Address;
    }
    else if (location->GetAddress()->GetType()->compare("intersect") == 0)
    {
        m_type = Location_Intersection;
    }
    else if (location->GetAddress()->GetType()->compare("airport") == 0)
    {
        m_type = Location_Airport;
    }
    else if (location->GetAddress()->GetType()->compare("freeform") == 0)
    {
        m_type = Location_AddressFreeForm;
    }
    if (location->GetAddress()->GetExtendedAddress() != NULL)
    {
        m_extAddress = new ExtendedAddressImpl(location->GetAddress()->GetExtendedAddress());
    }
    if (location->GetAddress()->GetCompactAddress() != NULL)
    {
        m_compAddress = new CompactAddressImpl(location->GetAddress()->GetCompactAddress());
    }
}

bool NBLocationImpl::operator==(const Location &location) const
{
    bool result = false;
    if (m_point == NULL)
    {
        if (location.GetCoordinates() == NULL)
        {
            result = true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        if (location.GetCoordinates() == NULL)
        {
            return false;
        }
        else
        {
            result = (m_point->GetLatitude() == location.GetCoordinates()->GetLatitude()) &&
            (m_point->GetLongitude() == location.GetCoordinates()->GetLongitude());
        }
    }
    result = result &&
    m_name == location.GetName() &&
    m_number == location.GetNumber() &&
    m_street == location.GetStreet() &&
    m_crossStreet == location.GetCrossStreet() &&
    m_city == location.GetCity() &&
    m_county == location.GetCounty() &&
    m_state == location.GetState() &&
    m_zipcode == location.GetZipCode() &&
    m_country == location.GetCountry() &&
    m_airport == location.GetAirport() &&
    m_countryName == location.GetCountry();
    return result;
}

/*! @} */
