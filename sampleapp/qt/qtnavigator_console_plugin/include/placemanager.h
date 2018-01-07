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

#ifndef PLACEMANAGER_H
#define PLACEMANAGER_H

#include <QDebug>
#include <QVariant>
#include <QtCore>
#include <QString>

#include "ltkhybridmanager.h"
#include "util.h"
#include "ltkerror.h"
#include "searchdatatypes.h"
#include "datatransformer.h"
#include "dataobject.h"

#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

class PlaceManager : public QObject
{
    Q_OBJECT

    //  Member variables
    QString mAppDirPath;

    QVariantHash mRecentPlaces;
    QVariantHash mFavPlaces;

    static PlaceManager *sInstance;

    PlaceManager();

public:

    ~PlaceManager();

    static PlaceManager* getInstance();

    void setAppDirPath(QString path);

    QVector<QJsonObject> getFavorites();
    QVector<QJsonObject> getRecents();

    bool addToRecents(QJsonObject);
    bool addToFavorites(QJsonObject);

    bool recentExists(double latitude, double longitude);
    bool favoriteExists(double latitude, double longitude);

    bool removeFromFavorites(double latitude, double longitude);
    bool deleteExistingFavConfig(double latitude,double longitude);

    void clearAllFavorites();
    void clearAllRecents();

private:

    void readRecentsJson() ;
    void readFavoritesJson();
    
    bool writeFavoritesConfig(QString nextWriteLocation, double latitude, double longitude);
    QString getFavoriteWriteLocation();

    bool writeRecentsConfig(QString nextWriteLocation, double latitude, double longitude);
    QString getRecentWriteLocation();

    bool resetFavorites();

    bool fileExists(QString path);
    
private slots:

    void onClearAllRecentsClicked();
    void onClearAllFavoritesClicked();
    bool onDeleteSelectiveFav(double latitude, double longitude);
};

#endif // PLACEMANAGER_H
