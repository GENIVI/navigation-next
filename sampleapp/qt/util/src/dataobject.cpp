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

/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the placeName of The Qt Company Ltd nor the placeNames of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QDebug>
#include "dataobject.h"

DataObject::DataObject(QObject *parent)
    : QObject(parent)
{
}

//DataObject::DataObject(const QString &placeName, const QString &placeAddress, const QString &placeDistance, const bool isCheapestPetrol, const QString &petrolPrice, const double placeRating, const bool isFav, QObject *parent)
DataObject :: DataObject(const QString &placeName,
           const QString &placeAddress,
           const QString &placeAddress1,
           const QString &placeAddress2,
           const QString &placeContact,
           const double placeRating,
           const double placeReviewCount,
           const QString &placeDistance,
           const QString &placeDescription,
           const QString &placeTimingInfo,
           const QString &placeFuelPriceSt,
           const QStringList &placeCategory,
           const QString &placeCategoryImagePath,
           const QString placeLatitude,
           const QString placeLongitude,
           const bool isCheapestPetrol,
           const QString &petrolPrice,
           const bool isFav,
           QObject *parent)
         : QObject(parent),
           m_placeName(placeName),
           m_placeAddress(placeAddress),
           m_placeAddress1(placeAddress1),
           m_placeAddress2(placeAddress2),
           m_placeContact(placeContact),
           m_placeRating(placeRating),
           m_placeReviewCount(placeReviewCount),
           m_placeDistance(placeDistance),
           m_placeDescription(placeDescription),
           m_placeTimingInfo(placeTimingInfo),
           m_placeFuelPriceSt(placeFuelPriceSt),
           m_placeCategory(placeCategory),
           m_placeCategoryImagePath(placeCategoryImagePath),
           m_placeLatitude(placeLatitude),
           m_placeLongitude(placeLongitude),
           m_isCheapestPetrol(isCheapestPetrol),
           m_petrolPrice(petrolPrice),
           m_isFav(isFav)
{

}

QString DataObject::placeName() const
{
    return m_placeName;
}

void DataObject::setPlaceName(const QString &placeName)
{
    if (placeName != m_placeName) {
        m_placeName = placeName;
        emit placeNameChanged();
    }
}

QString DataObject::placeAddress() const
{
    return m_placeAddress;
}

void DataObject::setPlaceAddress(const QString &placeAddress)
{
    if (placeAddress != m_placeAddress) {
        m_placeAddress = placeAddress;
        emit placeAddressChanged();
    }
}

QString DataObject::placeAddress1() const
{
    return m_placeAddress1;
}

void DataObject :: setPlaceAddress1(const QString &placeAddress1) {

    if (placeAddress1 != m_placeAddress1) {
        m_placeAddress1 = placeAddress1;
        emit placeAddress1Changed();
    }
}

QString DataObject::placeAddress2() const
{
    return m_placeAddress2;
}

void DataObject :: setPlaceAddress2(const QString &placeAddress2) {

    if (placeAddress2 != m_placeAddress2) {
        m_placeAddress2 = placeAddress2;
        emit placeAddress2Changed();
    }
}

QString DataObject :: placeContact() const
{
    return m_placeContact;
}

void DataObject :: setPlaceContact(const QString &placeContact) {

    if(placeContact != m_placeContact) {
        m_placeContact = placeContact;
        emit placeContactChanged();
    }

}

double DataObject::placeRating() const
{
    return m_placeRating;
}

void DataObject::setPlaceRating(const double placeRating)
{
    if (placeRating != m_placeRating) {
        m_placeRating = placeRating;
        emit placeRatingChanged();
    }
}


double DataObject::placeReviewCount() const
{
    return m_placeReviewCount;
}

void DataObject::setPlaceReviewCount(const double placeReviewCount)
{
    if (placeReviewCount != m_placeReviewCount) {
        m_placeReviewCount = placeReviewCount;
        emit placeReviewCountChanged();
    }
}

QString DataObject::placeDistance() const
{
    return m_placeDistance;
}

void DataObject::setPlaceDistance(const QString &placeDistance)
{
    if (placeDistance != m_placeDistance) {
        m_placeDistance = placeDistance;
        emit placeDistanceChanged();
    }
}

QString DataObject :: placeDescription() const
{
    return m_placeDescription;
}

void DataObject :: setPlaceDescription(const QString &placeDescription) {
    if(placeDescription != m_placeDescription) {
        m_placeDescription = placeDescription;
        emit placeDescriptionChanged();
    }
}

QString DataObject :: placeTimingInfo() const
{
    return m_placeTimingInfo;
}

void DataObject :: setPlaceTimingInfo(const QString &placeTimingInfo) {
    if(placeTimingInfo != m_placeTimingInfo) {
        m_placeTimingInfo = placeTimingInfo;
        emit placeTimingInfoChanged();
    }
}

QString DataObject :: placeFuelPriceSt() const
{
    return m_placeFuelPriceSt;
}

void DataObject :: setPlaceFuelPriceSt(const QString &placeFuelPriceSt) {
    if(placeFuelPriceSt != m_placeFuelPriceSt) {
        m_placeFuelPriceSt = placeFuelPriceSt;
        emit placeFuelPriceStChanged();
    }
}

QStringList DataObject :: placeCategory() const
{
    return m_placeCategory;
}

void DataObject :: setPlaceCategory(const QStringList &placeCategory) {
    if(placeCategory != m_placeCategory) {
        m_placeCategory = placeCategory;
        emit placeCategoryChanged();
    }
}

QString DataObject :: placeCategoryImagePath() const
{
    return m_placeCategoryImagePath;
}

void DataObject :: setPlaceCategoryImagePath(const QString &placeCategoryImagePath) {
    if(placeCategoryImagePath != m_placeCategoryImagePath) {
        m_placeCategoryImagePath = placeCategoryImagePath;
        emit placeCategoryImagePathChanged();
    }
}

QString DataObject::placeLatitude() const
{
    return m_placeLatitude;
}

void DataObject::setPlaceLatitude(const QString placeLatitude)
{
    if (placeLatitude != m_placeLatitude) {
        m_placeLatitude = placeLatitude;
        emit placeLatitudeChanged();
    }
}


QString DataObject::placeLongitude() const
{
    return m_placeLongitude;
}

void DataObject::setPlaceLongitude(const QString placeLongitude)
{
    if (placeLongitude != m_placeLongitude) {
        m_placeLongitude = placeLongitude;
        emit placeLongitudeChanged();
    }
}


bool DataObject::isCheapestPetrol() const
{
    return m_isCheapestPetrol;
}

void DataObject::setIsCheapestPetrol(const bool isCheapestPetrol)
{
    if (isCheapestPetrol != m_isCheapestPetrol) {
        m_isCheapestPetrol = isCheapestPetrol;
        emit isCheapestPetrolChanged();
    }
}
QString DataObject::petrolPrice() const
{
    return m_petrolPrice;
}

void DataObject::setPetrolPrice(const QString &petrolPrice)
{
    if (petrolPrice != m_petrolPrice) {
        m_petrolPrice = petrolPrice;
        emit petrolPriceChanged();
    }
}

bool DataObject::isFav() const
{
    return m_isFav;
}

void DataObject::setIsFav(const bool isFav)
{
    if (isFav != m_isFav) {
        m_isFav = isFav;
        emit isFavChanged();
    }
}
