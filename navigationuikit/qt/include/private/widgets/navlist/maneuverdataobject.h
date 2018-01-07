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
    @file maneuverdataobject.h
    @date 12/04/2014
    @addtogroup navigationuikit
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

#ifndef MANEUVEROBJECT_H
#define MANEUVEROBJECT_H

#include <QObject>
#include <QAbstractListModel>
#include <QList>
#include <QSharedPointer>

namespace locationtoolkit
{
class ManeuverDataObject
{
public:
    ManeuverDataObject();
    ~ManeuverDataObject();

public:
    const QString& Distance() const;
    void SetDistance(const QString& distance);

    const QString& Street() const;
    void SetStreet(const QString& street);

    const QString& Character() const;
    void SetCharacter(const QString& character);

    const QString& SplitlineColor() const;
    void SetSplitlineColor(const QString& color);

    quint32 ManeuverId() const;
    void SetManeuverId(quint32 maneuverId);

    const QString& Traffic() const;
    void SetTraffic(const QString& traffic);

    const QString& DelayTime() const;
    void SetDelayTime(const QString& delayTime);

    const QString& CharacterColor() const;
    void SetCharacterColor(const QString& color);

    const QString& CharacterColorLast() const;
    void SetCharacterColorLast(const QString& color);

    bool IsNormalItem() const;
    void SetIsNormalItem(bool value);

private:
    QString mDistance;
    QString mStreet;
    QString mCharacter;
    QString mSplitlineColor;
    quint32 mManeuverId;
    QString mTraffic;
    QString mDelayTime;
    QString mCharacterColor;
    QString mCharacterColorLast;
    bool    mIsNormalItem;
};

class ManeuverDataModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ManeuverDataRoles {
        DistanceRole = Qt::UserRole + 1,
        StreetRole,
        TurnRole,
        TrafficRole,
        DelayTimeRole,
        SplitLineColorRole,
        CharaterColorRole,
        IsNormalItem
    };

    ManeuverDataModel(QObject *parent = 0);
    void AddData( QSharedPointer<ManeuverDataObject> data );
    void AddData( QList< QSharedPointer<ManeuverDataObject> > dataList );
    void Clear();
    void UpdateDistance( const QString& distance );
    void UpdateTurn( const QString& turn );
    void UpdateStreet( const QString& street );

    virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    virtual QHash<int, QByteArray> roleNames() const;

    QSharedPointer<ManeuverDataObject> Find(quint32 maneuverId);

private:
    QList< QSharedPointer<ManeuverDataObject> > mListData;

};
}

#endif // MANEUVEROBJECT_H
