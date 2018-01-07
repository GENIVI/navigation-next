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

#ifndef DATAOBJECT_H
#define DATAOBJECT_H

#include <QObject>

//![0]
class DataObject : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString placeName READ placeName WRITE setPlaceName NOTIFY placeNameChanged)
    Q_PROPERTY(QString placeAddress READ placeAddress WRITE setPlaceAddress NOTIFY placeAddressChanged)
    Q_PROPERTY(QString placeAddress1 READ placeAddress1 WRITE setPlaceAddress1 NOTIFY placeAddress1Changed)
    Q_PROPERTY(QString placeAddress2 READ placeAddress2 WRITE setPlaceAddress2 NOTIFY placeAddress2Changed)
    Q_PROPERTY(QString placeContact READ placeContact WRITE setPlaceContact NOTIFY placeContactChanged)
    Q_PROPERTY(double placeRating READ placeRating WRITE setPlaceRating NOTIFY placeRatingChanged)
    Q_PROPERTY(double placeReviewCount READ placeReviewCount WRITE setPlaceReviewCount NOTIFY placeReviewCountChanged)
    Q_PROPERTY(QString placeDistance READ placeDistance WRITE setPlaceDistance NOTIFY placeDistanceChanged)
    Q_PROPERTY(QString placeDescription READ placeDescription WRITE setPlaceDescription NOTIFY placeDescriptionChanged)
    Q_PROPERTY(QString placeTimingInfo READ placeTimingInfo WRITE setPlaceTimingInfo NOTIFY placeTimingInfoChanged)
    Q_PROPERTY(QString placeFuelPriceSt READ placeFuelPriceSt WRITE setPlaceFuelPriceSt NOTIFY placeFuelPriceStChanged)
    Q_PROPERTY(QStringList placeCategory READ placeCategory WRITE setPlaceCategory NOTIFY placeCategoryChanged)
    Q_PROPERTY(QString placeCategoryImagePath READ placeCategoryImagePath WRITE setPlaceCategoryImagePath NOTIFY placeCategoryImagePathChanged)

    Q_PROPERTY(QString placeLatitude READ placeLatitude WRITE setPlaceLatitude NOTIFY placeLatitudeChanged)
    Q_PROPERTY(QString placeLongitude READ placeLongitude WRITE setPlaceLongitude NOTIFY placeLongitudeChanged)

    Q_PROPERTY(bool isCheapestPetrol READ isCheapestPetrol WRITE setIsCheapestPetrol NOTIFY isCheapestPetrolChanged)
    Q_PROPERTY(QString petrolPrice READ petrolPrice WRITE setPetrolPrice NOTIFY petrolPriceChanged)
    Q_PROPERTY(bool isFav READ isFav WRITE setIsFav NOTIFY isFavChanged)

//![0]

public:
    DataObject(QObject *parent=0);
    DataObject(const QString &placeName,
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
               QObject *parent=0);

    QString placeName() const;
    void setPlaceName(const QString &placeName);

    QString placeAddress() const;
    void setPlaceAddress(const QString &placeAddress);

    QString placeAddress1() const;
    void setPlaceAddress1(const QString &placeAddress1);

    QString placeAddress2() const;
    void setPlaceAddress2(const QString &placeAddress2);

    QString placeContact() const;
    void setPlaceContact(const QString &placeContact);

    double placeRating() const;
    void setPlaceRating(const double placeRating);

    double placeReviewCount() const;
    void setPlaceReviewCount(const double placeReviewCount);

    QString placeDistance() const;
    void setPlaceDistance(const QString &placeDistance);

    QString placeDescription() const;
    void setPlaceDescription(const QString &placeDescription);

    QString placeTimingInfo() const;
    void setPlaceTimingInfo(const QString &placeTimingInfo);

    QString placeFuelPriceSt() const;
    void setPlaceFuelPriceSt(const QString &placeFuelPriceSt);

    QStringList placeCategory() const;
    void setPlaceCategory(const QStringList &placeCategory);

    QString placeCategoryImagePath() const;
    void setPlaceCategoryImagePath(const QString &placeCategoryImagePath);

    QString placeLatitude() const;
    void setPlaceLatitude(const QString placeLatitude);

    QString placeLongitude() const;
    void setPlaceLongitude(const QString placeLongitude);

    bool isCheapestPetrol() const;
    void setIsCheapestPetrol(const bool isCheapestPetrol);

    QString petrolPrice() const;
    void setPetrolPrice(const QString &petrolPrice);

    bool isFav() const;
    void setIsFav(const bool isFav);

signals:
    void placeNameChanged();
    void placeAddressChanged();
    void placeAddress1Changed();
    void placeAddress2Changed();
    void placeContactChanged();
    void placeRatingChanged();
    void placeReviewCountChanged();
    void placeDistanceChanged();
    void placeDescriptionChanged();
    void placeTimingInfoChanged();
    void placeFuelPriceStChanged();
    void placeCategoryChanged();
    void placeCategoryImagePathChanged();

    void placeLatitudeChanged();
    void placeLongitudeChanged();

    void isCheapestPetrolChanged();
    void petrolPriceChanged();
    void isFavChanged();

private:
    QString m_placeName;
    QString m_placeAddress;
    QString m_placeAddress1;
    QString m_placeAddress2;
    QString m_placeContact;
    double m_placeRating;
    double m_placeReviewCount;
    QString m_placeDistance;
    QString m_placeDescription;
    QString m_placeTimingInfo;
    QString m_placeFuelPriceSt;
    QStringList m_placeCategory;
    QString m_placeCategoryImagePath;

    QString m_placeLatitude;
    QString m_placeLongitude;

    bool m_isCheapestPetrol;
    QString m_petrolPrice;
    bool m_isFav;

//![1]
};
//![1]

#endif // DATAOBJECT_H
