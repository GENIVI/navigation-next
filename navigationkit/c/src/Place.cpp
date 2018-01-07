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

#include "NavApiTypes.h"

namespace nbnav
{

Place::Place()
{
    m_name = "";
}

Place::Place(const MapLocation& location)
{
    m_name = "";
    m_location = location;
}

Place::Place(const string& name, const MapLocation& location)
{
    m_name = name;
    m_location = location;
}

MapLocation Place::GetLocation() const
{
    return m_location;
}

void Place::SetLocation(const MapLocation& mapLocation)
{
    m_location = mapLocation;
}

int Place::GetPhoneNumberCount() const
{
    return (int)m_phones.size();
}

void Place::ClearPhoneNumbers()
{
    m_phones.clear();
}

NB_Error Place::GetPhoneNumber(int index, Phone& phoneNumber) const
{
    if ( index < 0 || index >= static_cast<int>(m_phones.size()) )
    {
        return NE_RANGE;
    }

    phoneNumber = m_phones[index];
    return NE_OK;
}

void Place::AddPhoneNumber(const Phone& phoneNumber)
{
    m_phones.push_back(phoneNumber);
}

int Place::GetCategoriesCount() const
{
    return (int)m_categories.size();
}

void Place::ClearCategories()
{
    m_categories.clear();
}

NB_Error Place::GetCategory(int index, Category& category) const
{
    if ( index < 0 || index >= static_cast<int>(m_categories.size()) )
    {
        return NE_RANGE;
    }

    category = m_categories[index];
    return NE_OK;
}

void Place::AddCategory(const Category& category)
{
    m_categories.push_back(category);
}

string Place::GetName() const
{
    return m_name;
}

void Place::SetName(const string& name)
{
    m_name = name;
}

}
