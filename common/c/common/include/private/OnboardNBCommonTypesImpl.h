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
 @file        NBCommonTypesImpl.h
 @description Common types defined for all kits.
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

#ifndef __ONBOARDNBCOMMONTYPESIMPL_H__
#define __ONBOARDNBCOMMONTYPESIMPL_H__

#include "CommonTypes.h"
#include "NBProtocolExtendedAddress.h"
#include "NBProtocolCompactAddress.h"
#include "NBProtocolLocation.h"
#include <ncdb/include/GeocodeTypes.h>

namespace nbcommon
{
using namespace protocol;

class OnboardExtendedAddressImpl : public ExtendedAddress
{
public:
    OnboardExtendedAddressImpl() {}
    OnboardExtendedAddressImpl(const ExtendedAddress& copy);
    OnboardExtendedAddressImpl(const OnboardExtendedAddressImpl& copy);
    OnboardExtendedAddressImpl(protocol::ExtendedAddressSharedPtr extendedAddress);
    virtual ~OnboardExtendedAddressImpl() {}

    virtual const std::vector<std::string> & GetLines() const {return m_lines;}

private:
    std::vector<std::string> m_lines;
};

class OnboardCompactAddressImpl : public CompactAddress
{
public:
    OnboardCompactAddressImpl() {}
    OnboardCompactAddressImpl(const CompactAddress& copy);
    OnboardCompactAddressImpl(const OnboardCompactAddressImpl& copy);
    OnboardCompactAddressImpl(protocol::CompactAddressSharedPtr compactAddress);
    virtual ~OnboardCompactAddressImpl() {}

    virtual const std::string& GetLineSeparator() const {return m_lineSeparator;}
    virtual const std::string& GetLine1() const {return m_line1;}
    virtual const std::string& GetLine2() const {return m_line2;}
    virtual const std::string& GetAddressLine() const {return m_addressLine;}

    std::string m_lineSeparator;
    std::string m_line1;
    std::string m_line2;
    std::string m_addressLine;
};

class OnboardNBLocationImpl : public Location
{
public:
    OnboardNBLocationImpl();
    OnboardNBLocationImpl(const Location& copy);
    OnboardNBLocationImpl(const OnboardNBLocationImpl& copy);
    OnboardNBLocationImpl(const Ncdb::Address& address, const Ncdb::WorldPoint& point, float UTCOffset);
    bool operator ==(const Location& location) const;
    virtual ~OnboardNBLocationImpl();

    virtual LocationType GetType(void) const {return m_type;}
    virtual LatitudeLongitude* GetCoordinates(void) const {return m_point;}
    virtual const char* GetName(void) const {return m_name.c_str();}
    virtual const char* GetNumber(void) const {return m_number.c_str();}
    virtual const char* GetStreet(void) const {return m_street.c_str();}
    virtual const char* GetCrossStreet(void) const {return m_crossStreet.c_str();}
    virtual const char* GetCity(void) const {return m_city.c_str();}
    virtual const char* GetCounty(void) const {return m_county.c_str();}
    virtual const char* GetState(void) const {return m_state.c_str();}
    virtual const char* GetZipCode(void) const {return m_zipcode.c_str();}
    virtual const char* GetCountry(void) const {return m_country.c_str();}
    virtual const char* GetAirport(void) const {return m_airport.c_str();}
    virtual const char* GetCountryName(void) const {return m_countryName.c_str();}
    virtual const char* GetFreeForm(void) const {return m_freeForm.c_str(); }
    virtual float GetUTCOffset(void) const {return m_UTCOffset;}
    virtual const ExtendedAddress* GetExtendedAddress(void) const {return m_extAddress;}
    virtual const CompactAddress* GetCompactAddress(void) const {return m_compAddress;}

    LocationType m_type;
    LatitudeLongitude* m_point;
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
    float       m_UTCOffset;
    OnboardExtendedAddressImpl* m_extAddress;
    OnboardCompactAddressImpl* m_compAddress;
};

}

#endif

/*! @} */
