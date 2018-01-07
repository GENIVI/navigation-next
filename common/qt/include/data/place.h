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
    @file place.h
    @date 10/09/2014
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

/*! @{ */
#ifndef LOCATIONTOOLKIT_LTK_PLACE_H
#define LOCATIONTOOLKIT_LTK_PLACE_H

#include "maplocation.h"
#include "phone.h"
#include "category.h"
#include <QVector>
namespace locationtoolkit
{
/*! class Place

    Represents a basic place.
*/
class Place
{
public:
    /*! Creates an empty place. */
    Place();

    /*! Creates a place with the given location.

        @param location The location for which the user wants to create a place.
    */
    Place(const MapLocation& location);

    /*! Creates a place with the given name and map location.

        @param name The name of the place.
        @param mapLocation The map location from which the user wants to create a place.
    */
    Place(const QString& name, const MapLocation& mapLocation);

    /*! Gets place location

        @return place location.
    */
    MapLocation GetLocation() const;

    /*! Sets place location.

        @param mapLocation {@link MapLocation} object to set.
        @return None
    */
    void SetLocation(const MapLocation& mapLocation);

    /*! Gets number of phone numbers available for this place.

        @return phone numbers count
    */
    qint32 GetPhoneNumberCount() const;

    /*! Clears all the phone numbers associated with this place.

        @return None
    */
    void ClearPhoneNumbers();

    /*! Gets the phone number at the given index.

        @param index The 0 based index of the phone number.
        @param phoneNumber Phone structure. Result will be stored here.
        @return {@code true} if get phone number succeesfully, {@code false} get failed.
    */
    bool GetPhoneNumber(qint32 index, Phone& phoneNumber) const;

    /*! Adds a new phone number to this place.

        @param phoneNumber Phone number to be added.
        @return None
    */
    void AddPhoneNumber(const Phone& phoneNumber);

    /*! Gets the number of categories associated for this place.

        @return number of categories
    */
    qint32 GetCategoriesCount() const;

    /*! Clears all the Categories

        @return None
    */
    void ClearCategories();

    /*! Returns the Category at the given index.

        @param index The 0 based index of the Category
        @param category Category structure. Result will be stored here.
        @return {@code true} if get Category succeesfully, {@code false} get failed.
    */
    bool GetCategory(qint32 index, Category& category) const;

    /*! Adds a new category to this place.

        @param category Category to add
        @return None
    */
    void AddCategory(const Category& category);

    /*! Returns place name

        @return place name
    */
    QString GetName() const;

    /*! Sets place name

        @param name the name to set
        @return None
    */
    void SetName(const QString& name);

private:
    QString           m_name;
    MapLocation           m_location;
    QVector<Category> m_categories;
    QVector<Phone>    m_phones;

};
}// namespace locationtoolkit
#endif //LOCATIONTOOLKIT_LTK_PLACE_H
