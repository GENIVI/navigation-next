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
    @file maneuverdataobject.cpp
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

#include "maneuverdataobject.h"

using namespace locationtoolkit;

ManeuverDataObject::ManeuverDataObject()
{}

ManeuverDataObject::~ManeuverDataObject()
{

}

const QString &ManeuverDataObject::Distance() const
{
    return mDistance;
}

void ManeuverDataObject::SetDistance(const QString &distance)
{
    mDistance = distance;
}

const QString &ManeuverDataObject::Street() const
{
    return mStreet;
}

void ManeuverDataObject::SetStreet(const QString &street)
{
    mStreet = street;
}

const QString &ManeuverDataObject::Character() const
{
    return mCharacter;
}

void ManeuverDataObject::SetCharacter(const QString &character)
{
    mCharacter = character;
}

const QString &ManeuverDataObject::SplitlineColor() const
{
    return mSplitlineColor;
}

void ManeuverDataObject::SetSplitlineColor(const QString &color)
{
    mSplitlineColor = color;
}

quint32 ManeuverDataObject::ManeuverId() const
{
    return mManeuverId;
}

void ManeuverDataObject::SetManeuverId(quint32 maneuverId)
{
    mManeuverId = maneuverId;
}

const QString &ManeuverDataObject::Traffic() const
{
    return mTraffic;
}

void ManeuverDataObject::SetTraffic(const QString &traffic)
{
    mTraffic = traffic;
}

const QString &ManeuverDataObject::DelayTime() const
{
    return mDelayTime;
}

void ManeuverDataObject::SetDelayTime(const QString &delayTime)
{
    mDelayTime = delayTime;
}

const QString &ManeuverDataObject::CharacterColor() const
{
    return mCharacterColor;
}

void ManeuverDataObject::SetCharacterColor(const QString &color)
{
    mCharacterColor = color;
}

const QString &ManeuverDataObject::CharacterColorLast() const
{
    return mCharacterColorLast;
}

void ManeuverDataObject::SetCharacterColorLast(const QString &color)
{
    mCharacterColorLast = color;
}

bool ManeuverDataObject::IsNormalItem() const
{
    return mIsNormalItem;
}
void ManeuverDataObject::SetIsNormalItem(bool value)
{
    mIsNormalItem = value;
}

/*********************ManeuverDataModel class**************************/
ManeuverDataModel::ManeuverDataModel(QObject */*parent*/)
{
}

void ManeuverDataModel::AddData(QSharedPointer<ManeuverDataObject> data)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    mListData.push_back( data );
    endInsertRows();
}

void ManeuverDataModel::AddData(QList<QSharedPointer<ManeuverDataObject> > dataList)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    mListData = dataList;
    endInsertRows();
}

void ManeuverDataModel::Clear()
{
    beginRemoveRows( QModelIndex(), 0, rowCount() );
    mListData.clear();
    endRemoveRows();
}

void ManeuverDataModel::UpdateDistance( const QString& distance )
{
    if( mListData.count() > 0 )
    {
        mListData[0]->SetDistance( distance );
        QModelIndex index = createIndex( 0, 0 );
        emit QAbstractItemModel::dataChanged( index, index );
    }
}

void ManeuverDataModel::UpdateTurn( const QString& turn )
{
    if( mListData.count() > 0 )
    {
        mListData[0]->SetCharacter( turn );
        QModelIndex index = createIndex( 0, 0 );
        emit QAbstractItemModel::dataChanged( index, index );
    }
}

void ManeuverDataModel::UpdateStreet( const QString& street )
{
    if( mListData.count() > 0 )
    {
        mListData[0]->SetStreet( street );
        QModelIndex index = createIndex( 0, 0 );
        emit QAbstractItemModel::dataChanged( index, index );
    }
}

int ManeuverDataModel::rowCount(const QModelIndex &parent) const
{
    return mListData.count();
}

QVariant ManeuverDataModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() > mListData.count())
        return QVariant();

    const QSharedPointer<ManeuverDataObject>& maneuver = mListData[ index.row() ];

    switch ( role ) {
    case DistanceRole:
        return maneuver->Distance();
        break;

    case StreetRole:
        return maneuver->Street();
        break;

    case TurnRole:
        return maneuver->Character();
        break;

    case TrafficRole:
        return maneuver->Traffic();

    case DelayTimeRole:
        return maneuver->DelayTime();

    case SplitLineColorRole:
        return maneuver->SplitlineColor();
    case IsNormalItem:
        return maneuver->IsNormalItem();

    case CharaterColorRole:
    {
        if(index.row() == (mListData.count() - 1))
        {
            return maneuver->CharacterColorLast();
        }
        else
        {
            return maneuver->CharacterColor();
        }
        break;
    }
    default:
        break;
    }
    return QVariant();
}

QHash<int, QByteArray> ManeuverDataModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[DistanceRole] = "distance";
    roles[StreetRole] = "street";
    roles[TurnRole] = "character";
    roles[TrafficRole] = "traffic";
    roles[DelayTimeRole] = "delaytime";
    roles[SplitLineColorRole] = "splitlinecolor";
    roles[CharaterColorRole] = "charatercolor";
    roles[IsNormalItem] = "isnormalitem";
    return roles;
}

QSharedPointer<ManeuverDataObject> ManeuverDataModel::Find(quint32 maneuverId)
{
    for(int i = 0; i < mListData.size(); ++i)
    {
        QSharedPointer<ManeuverDataObject> maneuver = mListData[i];
        if(maneuver.data()->ManeuverId() == maneuverId)
        {
            return maneuver;
        }
    }

    return QSharedPointer<ManeuverDataObject>();
}
