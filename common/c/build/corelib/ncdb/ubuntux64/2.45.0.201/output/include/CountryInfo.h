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

/*
 * (C) Copyright 2012 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
 *
 */

#pragma once
#ifndef file__NCDB_INCLUDE_COUNTRY_INFO_H
#define file__NCDB_INCLUDE_COUNTRY_INFO_H

#include "NcdbTypes.h"

class CountryMgr;

class NCDB_API CountryInfo
{
public:
    typedef CountryInfo self_t;

    CountryInfo() : m_id(-1) {}
    CountryInfo( int countryId ) : m_id(countryId) {}
    explicit CountryInfo( const self_t& v ) : m_id(v.m_id) {}

    //! @brief retrieve country name
    const char* GetName(const CountryMgr* countryMgr) const;

    //! @brief get 2 letter country abbrev
    const char* GetAbbrev(const CountryMgr* countryMgr) const;

    //! @brief get 3 letter country abbrev
    const char* GetAbbrev3(const CountryMgr* countryMgr) const;

    //! @brief get driving side as string
    const char* GetDrivingSide(const CountryMgr* countryMgr) const;

    //! @brief get country-specific traffic side (left or right)
    bool IsRightDriving(const CountryMgr* countryMgr) const;


    //! @brief assign country id
    //! @param[in] countryId -- unique country id, or -1 for unspecified country
    void SetId( int countryId ) { m_id = countryId; }

    //! @brief retrieve assigned country id
    int  GetId() const { return m_id; }

    bool operator == ( const self_t& c ) const { return m_id == c.m_id; }
    bool operator != ( const self_t& c ) const { return m_id != c.m_id; }

private:
    int m_id;   // countryId;
};


#endif // file__NCDB_INCLUDE_COUNTRY_INFO_H

