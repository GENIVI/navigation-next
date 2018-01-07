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

//#include <QGuiApplication>
#include "placemanager.h"

#define MAX_RECENT_COUNT 100
#define MAX_FAV_COUNT 200

PlaceManager *PlaceManager::sInstance = 0;

PlaceManager::PlaceManager() {

    qDebug()<<"PlaceManager::PlaceManager()";
    //    mRecentPlaces = readRecentsJson();
    //    mFavPlaces = readFavoritesJson();
}

PlaceManager::~PlaceManager() {
}

PlaceManager* PlaceManager::getInstance() {

    if (!sInstance)
        sInstance = new PlaceManager();
    return sInstance;
}

void PlaceManager::setAppDirPath(QString path) {
    
    qDebug()<<"PlaceManager::setAppDirPath "<<path;
    mAppDirPath = path;

    readRecentsJson();
    readFavoritesJson();
}

void PlaceManager::readRecentsJson() {

    QString recentsJsonPath = mAppDirPath + "/resource/jsons/recents/recents.json";

    // Read JSON file
    QFile file(recentsJsonPath);
    file.open(QIODevice::ReadOnly);
    QByteArray rawData = file.readAll();
    file.close();

    // Parse document
    QJsonDocument doc(QJsonDocument::fromJson(rawData));

    // Get JSON object
    QJsonObject json = doc.object();

    mRecentPlaces.clear();

    // Access properties
    for(int i = 0; i < MAX_RECENT_COUNT; i++) {

        QString idx = QString::number(i);
        QVariantMap tempMap;

        tempMap.insert("lat", json[idx].toObject()["lat"].toString() );
        tempMap.insert("lon", json[idx].toObject()["lon"].toString() );
        tempMap.insert("isExists", json[idx].toObject()["isExists"].toString());

        mRecentPlaces.insert(idx, tempMap);
    }
}

void PlaceManager::readFavoritesJson() {

    QString favouritesJson = mAppDirPath + "/resource/jsons/favourites/favourites.json";

    // Read JSON file
    QFile file(favouritesJson);
    file.open(QIODevice::ReadOnly);
    QByteArray rawData = file.readAll();
    file.close();

    // Parse document
    QJsonDocument doc(QJsonDocument::fromJson(rawData));

    // Get JSON object
    QJsonObject json = doc.object();

    mFavPlaces.clear();

    // Access properties
    for(int i = 0; i < MAX_FAV_COUNT; i++) {

        QString idx = QString::number(i);

        QVariantMap tempMap;

        tempMap.insert("lat", json[idx].toObject()["lat"].toString() );
        tempMap.insert("lon", json[idx].toObject()["lon"].toString() );
        tempMap.insert("isExists", json[idx].toObject()["isExists"].toString());

        mFavPlaces.insert(idx, tempMap);
    }
}

bool PlaceManager::favoriteExists(double latitude, double longitude) {

    QString c_lat = QString::number(latitude);
    QString c_long = QString::number(longitude);

    for(int i = 0; i <MAX_FAV_COUNT; i++) {

        QString t_lat = mFavPlaces[QString::number(i)].toMap()["lat"].toString();
        QString t_long = mFavPlaces[QString::number(i)].toMap()["lon"].toString();
        bool isExists = mFavPlaces[QString::number(i)].toMap()["isExists"].toBool();
        if(t_lat == c_lat && t_long == c_long && isExists)
            return true;
        //qDebug() << "PlaceManager::favoriteExists................true";
    }

    // qDebug() << "PlaceManager::favoriteExists...............false";

    return false;
}

bool PlaceManager::addToFavorites(QJsonObject jsonObject) {

    qDebug()<<"######################## PlaceManager::addToFavorites";
    QJsonObject location = jsonObject["place"].toObject()["location"].toObject();

    double latitude = location["latitude"].toDouble();
    double longitude = location["longitude"].toDouble();

    //  Check if favorite exists - Favorites list contains only one instance of POI
    if(!favoriteExists(latitude, longitude)) {

        QString filePostFix = getFavoriteWriteLocation();
        QString favoritesFile = mAppDirPath + "/resource/jsons/favourites/favourites_"+ filePostFix +".json";

        //  Write to file on local machine
        QFile jsonFile(favoritesFile);
        jsonFile.open(QFile::WriteOnly);
        jsonFile.write(QJsonDocument(jsonObject).toJson(QJsonDocument::Compact));
        jsonFile.close();

        //  Should not show any popOver - Inherent action
        if(writeFavoritesConfig(filePostFix, latitude, longitude) ){
            qDebug()<<"Favorites Successfully added";
            return true;
        }
        else{
            qDebug()<<"Favorites could not be added";
        }
    }else{
        return false;
        qDebug()<<"Favorites Already added";
    }
}

//  Write POI details to favorite's config
bool PlaceManager::writeFavoritesConfig(QString nextWriteLocation, double latitude, double longitude) {

    QVariantMap tempMap;

    tempMap.insert("lat", QString::number(latitude));
    tempMap.insert("lon", QString::number(longitude));
    tempMap.insert("isExists", "1");

    mFavPlaces.insert(nextWriteLocation, tempMap);

    QJsonObject jsonObject = QJsonObject::fromVariantHash(mFavPlaces);

    QString favoritesFile = mAppDirPath + "/resource/jsons/favourites/favourites.json";
    QFile jsonFile(favoritesFile);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(QJsonDocument(jsonObject).toJson(QJsonDocument::Compact));
    jsonFile.close();

    return fileExists(favoritesFile);
}

//  Reset to no favorites
bool PlaceManager::resetFavorites() {

    return false;
    //    QVector<QString> pathList = getExistingFavourites();
    //    int count = 0;

    //    for(QVector<QString>::iterator it = pathList.begin(); it != pathList.end(); ++it) {
    //        QFile file(*it);

    //        if(file.remove())
    //            ++count;
    //    }

    //    QVariantMap* tempMap;
    //    QVariantMap* tempMap1;

    //    for(int i = 0; i <= 10; ++i) {
    //        tempMap = new QVariantMap();
    //        tempMap1 = new QVariantMap();

    //        if(i == 0){
    //            tempMap->insert("lat", "1" );
    //            tempMap->insert("lon", "1" );
    //            mFavPlaces.insert("0", *tempMap);
    //        }
    //        else{
    //            tempMap->insert("lat", "" );
    //            tempMap->insert("lon", "" );
    //            mFavPlaces.insert(QString::number(i), *tempMap);
    //            tempMap1->insert("isExists","0");
    //            mExistingFav.insert(QString::number(i), *tempMap1);
    //        }
    //    }

    //    //qDebug()<<mFavPlaces;
    //    QJsonObject jsonObject = QJsonObject::fromVariantHash(mFavPlaces);
    //    QJsonObject jsonObject1 = QJsonObject::fromVariantHash(mExistingFav);

    //    QString favoriteFile = mAppDirPath + "/resource/jsons/favourites/favourites.json";
    //    QFile jsonFile(favoriteFile);
    //    jsonFile.open(QFile::WriteOnly);
    //    jsonFile.write(QJsonDocument(jsonObject).toJson(QJsonDocument::Compact));
    //    jsonFile.close();

    //    QString existingFavouriteFile = mAppDirPath + "/resource/jsons/favourites/existingFavourites.json";
    //    QFile jsonFile1(existingFavouriteFile);
    //    jsonFile1.open(QFile::WriteOnly);
    //    jsonFile1.write(QJsonDocument(jsonObject1).toJson(QJsonDocument::Compact));
    //    jsonFile1.close();

    //    return (fileExists(favoriteFile) && fileExists(existingFavouriteFile) && count==pathList.size());
}

//  Get all the active favorite POIs in memory
QVector<QJsonObject> PlaceManager::getFavorites() {

    QVector<QJsonObject> favObjects;

    for(int i = 0; i < MAX_FAV_COUNT; ++i) {

        QString path = mAppDirPath + "/resource/jsons/favourites/favourites_" + QString::number(i) + ".json";
        if( fileExists(path)) {

            // Read JSON file
            QFile file(path);
            file.open(QIODevice::ReadOnly);
            QByteArray rawData = file.readAll();
            file.close();

            // Parse document
            QJsonDocument doc(QJsonDocument::fromJson(rawData));

            // Get JSON object
            QJsonObject obj = doc.object();

            favObjects.append(obj);
        }
    }

    return favObjects;
}

//  Get the instance number 0 - MAX_FAV_COUNT-1 where the favorite pOI info needs to be written
QString PlaceManager::getFavoriteWriteLocation() {

    // TODO: Consider to replace the oldest
    int i;

    for(i = 0; i < mFavPlaces.count(); i++) {

        // check for empty(becuase of delete) recent slot
        if(mFavPlaces[QString::number(i)].toMap()["isExists"].toInt() != 1)
            break;
    }

    // use slots upto max count or replace the 0 index
    if(i >= MAX_FAV_COUNT)
        i = 0;

    qDebug()<<"PlaceManager::getRecentWriteLocation "<<i;

    return QString::number(i);
}

bool PlaceManager::fileExists(QString path) {

    QFileInfo checkFile(path);
    // check if file exists and if yes: Is it really a file and no directory?

    if (checkFile.exists() && checkFile.isFile()) {
        return true;
    }
    else {
        return false;
    }
}

//  Get the instance number 0 - MAX_RECENT_COUNT-1 where the recent pOI info needs to be written
QString PlaceManager::getRecentWriteLocation() {

    // TODO: Consider to replace the oldest
    int i;

    for(i = 0; i < mRecentPlaces.count(); i++) {

        // check for empty(becuase of delete) recent slot
        if(mRecentPlaces[QString::number(i)].toMap()["isExists"].toInt() != 1)
            break;
    }

    // use slots upto max count or replace the 0 index
    if(i >= MAX_RECENT_COUNT)
        i = 0;

    qDebug()<<"PlaceManager::getRecentWriteLocation "<<i;

    return QString::number(i);
}

QVector<QJsonObject> PlaceManager::getRecents() {

    QVector<QJsonObject> recentObjects;

    for(int i = 0; i < MAX_RECENT_COUNT; ++i) {

        QString path = mAppDirPath + "/resource/jsons/recents/recents_" + QString::number(i) + ".json";
        if( fileExists(path)) {

            // Read JSON file
            QFile file(path);
            file.open(QIODevice::ReadOnly);
            QByteArray rawData = file.readAll();
            file.close();

            // Parse document
            QJsonDocument doc(QJsonDocument::fromJson(rawData));

            // Get JSON object
            QJsonObject obj = doc.object();

            recentObjects.append(obj);
        }
    }

    return recentObjects;
}

bool PlaceManager::writeRecentsConfig(QString nextWriteLocation, double latitude, double longitude) {

    QVariantMap tempMap;

    tempMap.insert("lat", QString::number(latitude));
    tempMap.insert("lon", QString::number(longitude));
    tempMap.insert("isExists", "1");
    
    mRecentPlaces.insert(nextWriteLocation, tempMap);

    QJsonObject jsonObject = QJsonObject::fromVariantHash(mRecentPlaces);

    QString recentsFile = mAppDirPath + "/resource/jsons/recents/recents.json";
    QFile jsonFile(recentsFile);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(QJsonDocument(jsonObject).toJson(QJsonDocument::Compact));
    jsonFile.close();

    return fileExists(recentsFile);
}

bool PlaceManager::recentExists(double latitude, double longitude) {

    QString c_lat = QString::number(latitude);
    QString c_long = QString::number(longitude);

    for(int i = 0; i < MAX_RECENT_COUNT; i++) {
        QString t_lat = mRecentPlaces[QString::number(i)].toMap()["lat"].toString();
        QString t_long = mRecentPlaces[QString::number(i)].toMap()["lon"].toString();
        bool isExists = mRecentPlaces[QString::number(i)].toMap()["isExists"].toBool();
        if(t_lat == c_lat && t_long == c_long && isExists)
            return true;
        //       qDebug() << "PlaceManager::recentExists................true";
    }

    //   qDebug() << "PlaceManager::recentExists...............false";

    return false;
}

bool PlaceManager::addToRecents(QJsonObject jsonObject) {

    qDebug()<<"######################## PlaceManager::addToRecents";
    QJsonObject location = jsonObject["place"].toObject()["location"].toObject();

    double latitude = location["latitude"].toDouble();
    double longitude = location["longitude"].toDouble();

    //  Check if recent exists - Recents list contains only one instance of POI
    if(!recentExists(latitude, longitude)) {

        QString filePostFix = getRecentWriteLocation();
        QString recentsFile = mAppDirPath + "/resource/jsons/recents/recents_"+ filePostFix +".json";

        //  Write to file on local machine
        QFile jsonFile(recentsFile);
        jsonFile.open(QFile::WriteOnly);
        jsonFile.write(QJsonDocument(jsonObject).toJson(QJsonDocument::Compact));
        jsonFile.close();

        //  Should not show any popOver - Inherent action
        if(writeRecentsConfig(filePostFix, latitude, longitude) ){
            qDebug()<<"Recents Successfully added";
        }
        else{
            qDebug()<<"Recents could not be added";
        }
    }
}

void PlaceManager::onClearAllRecentsClicked() {

    mRecentPlaces.clear();

    QJsonObject jsonObject = QJsonObject::fromVariantHash(mRecentPlaces);

    QString recentsFile = mAppDirPath + "/resource/jsons/recents/recents.json";
    QFile jsonFile(recentsFile);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(QJsonDocument(jsonObject).toJson(QJsonDocument::Compact));
    jsonFile.close();

    for(int i = 0; i < MAX_RECENT_COUNT; ++i) {

        QString path = mAppDirPath + "/resource/jsons/recents/recents_" + QString::number(i) + ".json";
        if( fileExists(path)) {

            QFile file(path);
            file.remove();
        }
    }
}

void PlaceManager::onClearAllFavoritesClicked() {

    mFavPlaces.clear();

    QJsonObject jsonObject = QJsonObject::fromVariantHash(mFavPlaces);

    QString recentsFile = mAppDirPath + "/resource/jsons/favourites/favourites.json";
    QFile jsonFile(recentsFile);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(QJsonDocument(jsonObject).toJson(QJsonDocument::Compact));
    jsonFile.close();

    for(int i = 0; i < MAX_FAV_COUNT; ++i) {

        QString path = mAppDirPath + "/resource/jsons/favourites/favourites_" + QString::number(i) + ".json";
        if( fileExists(path)) {

            QFile file(path);
            file.remove();
        }
    }
}

bool PlaceManager::onDeleteSelectiveFav(double latitude, double longitude) {

    qDebug() << "PlaceManager::onDeleteSelectiveFav..............................";
    //    QString latitude = QString::number(lat, 'g', 6);
    //    QString longitude = QString::number(lon, 'g', 6);
    QString fileName;
    QString c_lat = QString::number(latitude);
    QString c_long = QString::number(longitude);
    QString* filePath = new QString();
    for(int i =0; i <= MAX_FAV_COUNT; i++) {
        QString t_lat = mFavPlaces[QString::number(i)].toMap()["lat"].toString();
        QString t_long = mFavPlaces[QString::number(i)].toMap()["lon"].toString();
        bool isExist = mFavPlaces[QString::number(i)].toMap()["isExists"].toBool();
        if(t_lat == c_lat && t_long == c_long && isExist) {
            *filePath = mAppDirPath + "/resource/jsons/favourites/favourites_"+ QString::number(i) +".json";
            fileName = mAppDirPath + "/resource/jsons/favourites/favourites_"+ QString::number(i) +".json" ;
        }
    }

    qDebug() << "fileName"<<latitude<<longitude<<fileName;

    QString locEnd = fileName.right(6);
    QString currentLocation = locEnd.left(1);

    QVariantMap* tempMap = new QVariantMap();
    tempMap->insert("lat", "" );
    tempMap->insert("long", "" );
    tempMap->insert("isExists", "");
    mFavPlaces.insert(currentLocation, *tempMap);

    QJsonObject jsonObject1 = QJsonObject::fromVariantHash(mFavPlaces);

    QString favouritesFile = mAppDirPath + "/resource/jsons/favourites/favourites.json";
    QFile jsonFile1(favouritesFile);
    jsonFile1.open(QFile::WriteOnly);
    jsonFile1.write(QJsonDocument(jsonObject1).toJson(QJsonDocument::Compact));
    jsonFile1.close();

    QString path = mAppDirPath + "/resource/jsons/favourites/favourites_" + currentLocation + ".json";
    if(fileExists(path)) {

        QFile file(path);
        file.remove();
        return true;
    }
    return false;
}

bool PlaceManager::removeFromFavorites(double latitude, double longitude) {

    QString c_lat = QString::number(latitude);
    QString c_long = QString::number(longitude);

    for(int i = 0; i < MAX_FAV_COUNT; i++) {

        QString t_lat = mFavPlaces[QString::number(i)].toMap()["lat"].toString();
        QString t_long = mFavPlaces[QString::number(i)].toMap()["lon"].toString();
        bool isExists = mFavPlaces[QString::number(i)].toMap()["isExists"].toBool();

        if(t_lat == c_lat && t_long == c_long && isExists) {

            QVariantMap tempMap;

            tempMap.insert("lat", "");
            tempMap.insert("lon", "");
            tempMap.insert("isExists", "");

            mFavPlaces.insert(QString::number(i), tempMap);

            QJsonObject jsonObject = QJsonObject::fromVariantHash(mFavPlaces/*mRecentPlaces*/);

            QString favoritesFile = mAppDirPath + "/resource/jsons/favourites/favourites.json";
            QFile jsonFile(favoritesFile);
            jsonFile.open(QFile::WriteOnly);
            jsonFile.write(QJsonDocument(jsonObject).toJson(QJsonDocument::Compact));
            jsonFile.close();

            QString path = mAppDirPath + "/resource/jsons/favourites/favourites_" + QString::number(i) + ".json";
            if(fileExists(path)) {

                QFile file(path);
                file.remove();
            }

            return true;
        }
    }

    return false;
}
bool PlaceManager::deleteExistingFavConfig(double lat,double lon)
{
    onDeleteSelectiveFav(lat,lon);
}
void PlaceManager::clearAllFavorites() {
    onClearAllFavoritesClicked();
}

void PlaceManager::clearAllRecents() {
    onClearAllRecentsClicked();
}
