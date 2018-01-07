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
    @file place.cpp
    @date 10/08/2014
    @addtogroup ltkcommon
*/
/*
 * (C) Copyright 2014 by TeleCommunication Systems, Inc.
 *
 * The information contained herein is confidential, proprietary
 * to TeleCommunication Systems, Inc., and considered a trade secret
 * as defined in section 499C of the penal code of the State of
 * California. Use of this information by anyone other than
 * authorized employees of TeleCommunication Systems is granted only
 * under a written non-disclosure agreement, expressly prescribing
 * the scope and manner of such use.
---------------------------------------------------------------------------*/

#include "place.h"
namespace locationtoolkit
{
/*! Creates an empty place. */
Place::Place()
{
    m_name = "";
}
/*! Creates a place with the given location.*/
Place::Place(const MapLocation& location)
{
    m_name = "";
    m_location = location;
}
/*! Creates a place with the given name and map location.  */
Place::Place(const QString& name, const MapLocation& mapLocation)
{
    m_name = name;
    m_location = mapLocation;
}
/*! Gets place location */
MapLocation Place::GetLocation() const
{
    return m_location;
}
/*! Sets place location. */
void Place::SetLocation(const MapLocation& mapLocation)
{
    m_location = mapLocation;
}
/*! Gets number of phone numbers available for this place.   */
qint32 Place::GetPhoneNumberCount() const
{
    return (qint32)m_phones.size();
}
/*! Clears all the phone numbers associated with this place. */
void Place::ClearPhoneNumbers()
{
    m_phones.clear();
}
/*! Gets the phone number at the given index.  */
bool Place::GetPhoneNumber(qint32 index, Phone& phoneNumber) const
{
    if ( index < 0 || index >= static_cast<qint32>(m_phones.size()) )
    {
        return false;
    }

    phoneNumber = m_phones[index];
    return true;
}
/*! Adds a new phone number to this place.   */
void Place::AddPhoneNumber(const Phone& phoneNumber)
{
    m_phones.push_back(phoneNumber);
}
/*! Gets the number of categories associated for this place.*/
qint32 Place::GetCategoriesCount() const
{
    return (qint32)m_categories.size();
}
/*! Clears all the Categories  */
void Place::ClearCategories()
{
    m_categories.clear();
}
/*! Returns the Category at the given index.   */
bool Place::GetCategory(qint32 index, Category& category) const
{
    if ( index < 0 || index >= static_cast<qint32>(m_categories.size()) )
    {
        return false;
    }

    category = m_categories[index];
    return true;
}
/*! Adds a new category to this place.   */
void Place::AddCategory(const Category& category)
{
    m_categories.push_back(category);
}
/*! Returns place name  */
QString Place::GetName() const
{
    return m_name;
}
/*! Sets place name   */
void Place::SetName(const QString& name)
{
    m_name = name;
}

}
