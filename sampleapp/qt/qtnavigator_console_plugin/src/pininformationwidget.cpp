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

#include "pininformationwidget.h"
#include<QQmlEngine>
#include <QGuiApplication>
PinInformationWidget::PinInformationWidget(QQuickItem* parent):
            QQuickItem(parent)
{

}

void PinInformationWidget::setUp()
{

//    this->PinInfoQuickItem->setProperty("height",720);
//    this->PinInfoQuickItem->setProperty("width",1280);
    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
    QString appDirPath = engine->rootContext()->contextProperty("applicationDirPath").toString();
    QQmlComponent component(engine,QUrl(appDirPath+"/resource/FuelDetails.qml"));
    QQuickItem *mPinInformationWidget  = qobject_cast<QQuickItem*>(component.create());

    QQmlEngine::setObjectOwnership(mSettingsView, QQmlEngine::CppOwnership);
    mSettingsView->setParentItem(this);
    mSettingsView->setParent(this);
    connect( this, SIGNAL(detailBackButtonClicked()), this, SLOT(on_backButton_clicked()) );

    //connect( this->rootObject(), SIGNAL(callButtonClicked()), this, SLOT(on_callButton_clicked()) );

//    mCallView = new CallViewController();
//    mCallView->rootContext()->setContextProperty("applicationDirPath", "file:///" + QGuiApplication::applicationDirPath());
//    mCallView->setSource(QUrl("qrc:///CallPOI.qml"));
//    //mCallView->setUp();
//    mCallView->setParent(this);
//    mCallView->setModality(Qt::WindowModal);
    mDir = QFileInfo(QCoreApplication::applicationFilePath()).dir();
    connect( this->PinInfoQuickItem, SIGNAL(callButtonClicked(QString,QString,QString)), this, SLOT(on_callButton_clicked(QString,QString,QString)) );
    connect( this->PinInfoQuickItem, SIGNAL(mapButtonClicked()), this, SLOT(onMapButtonClicked()) );
    connect( this->PinInfoQuickItem, SIGNAL(navigationButtonClicked()), this, SLOT(onNavigationButtonClicked()) );

    mRecentPlaces = readRecenetsJson();
    mFavPlaces = readFavouritesJson();
}

void  PinInformationWidget :: onMapButtonClicked() {

    emit toggleMapViewFromPinInformation();

}

void  PinInformationWidget :: onNavigationButtonClicked() {

    QVariant itemVariant;
    emit pinNavigationClicked("placeName", 1, itemVariant);
}

void PinInformationWidget :: on_backButton_clicked() {

    qDebug() << "PinInformationWidget: ";

//    this->lower();
//    this->hide();
    this->PinInfoQuickItem->setVisible(false);
}

void PinInformationWidget :: on_callButton_clicked(QString placeName,QString placeContact, QString placeCategoryImagePath) {
    qDebug() << "on_callButton_clicked: ";

    emit callButtonClickedEvent(placeName,placeContact,placeCategoryImagePath);
    //mCallView->raise();
    //mCallView->show();

}

//  Read the recents config file to retrieve recent items
QVariantHash PinInformationWidget::readRecenetsJson() {

    QString recentsJson = mDir.absolutePath() + "/resource/jsons/recents/recents.json";
    // Read JSON file
    QFile file(recentsJson);
    file.open(QIODevice::ReadOnly);
    QByteArray rawData = file.readAll();

    // Parse document
    QJsonDocument doc(QJsonDocument::fromJson(rawData));

    // Get JSON object
    QJsonObject json = doc.object();
    QVariantHash recentPlaces;

    // Access properties
    //qDebug()<<"\n\n\n\nFavourites List:";

    for(int i = 0; i <= 5; i++) {
        QString idx = QString::number(i);
        qDebug()<<json[idx].toObject()["lat"].toString()<<", "<<json[idx].toObject()["long"].toString();
        QVariantMap tempMap;
        tempMap.insert("lat", json[idx].toObject()["lat"].toString() );
        tempMap.insert("long", json[idx].toObject()["long"].toString() );

        recentPlaces.insert(idx, tempMap);
    }

    //qDebug()<<"\n\n\n\n";

    return recentPlaces;

}

//  Check if a directory and the subsequent file exists in the memory
bool PinInformationWidget::fileExists(QString path) {

    QFileInfo checkFile(path);
    // check if file exists and if yes: Is it really a file and no directory?

    if (checkFile.exists() && checkFile.isFile()) {
        return true;
    }
    else {
        return false;
    }

}

//  Check if a recent POI already exists in memory
bool PinInformationWidget::recentExists(double latitude, double longitude) {

    QString c_lat = QString::number(latitude);
    QString c_long = QString::number(longitude);

    for(int i = 1; i <= 5; i++) {
        QString t_lat = mRecentPlaces[QString::number(i)].toMap()["lat"].toString();
        QString t_long = mRecentPlaces[QString::number(i)].toMap()["long"].toString();

        if(t_lat == c_lat && t_long == c_long)
            return true;

    }

    return false;

}

//  Write POI details to recents's config
bool PinInformationWidget::writeRecentsConfig(QString nextWriteLocation, double latitude, double longitude) {

    //qDebug()<<"Latitude is: "<<latitude<<" Longitude is: "<<longitude;
    //qDebug()<<QString::number(latitude)<<", "<<QString::number(longitude);

    QVariantMap* tempMap = new QVariantMap();
    tempMap->insert("lat", QString::number(latitude) );
    tempMap->insert("long", QString::number(longitude) );
    mRecentPlaces.insert(nextWriteLocation, *tempMap);

    int nextWriteLocationInt = (nextWriteLocation.toInt() + 1)%5;

    if(nextWriteLocationInt == 0)
        nextWriteLocationInt = 5;

    tempMap = new QVariantMap();
    tempMap->insert("lat", QString::number(nextWriteLocationInt) );
    tempMap->insert("long", QString::number(nextWriteLocationInt) );
    mRecentPlaces.insert("0", *tempMap);

    //qDebug()<<mRecentPlaces;
    QJsonObject jsonObject = QJsonObject::fromVariantHash(mRecentPlaces);

    QString recentsFile = mDir.absolutePath() + "/resource/jsons/recents/recents.json";
    QFile jsonFile(recentsFile);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(QJsonDocument(jsonObject).toJson(QJsonDocument::Compact));

    return fileExists(recentsFile);

}

//  Get all the active recents POIs in memory
QVector<QString> PinInformationWidget::getExistingRecents() {

    QVector<QString> files;

    for(int i = 1; i <= 5; ++i){
        QString path = mDir.absolutePath()+"/resource/jsons/recents/recents_" + QString::number(i) + ".json";

        if( fileExists(path) ){
            files.push_back(path);
        }

    }

    return files;

}

//  Get the instance number 1 - 5 where the recent pOI info needs to be written
QString PinInformationWidget::getRecentWriteLocation() {

    QVariantMap temp = mRecentPlaces["0"].toMap();
    //qDebug()<<"Write at file no. "<<mRecentPlaces["0"].toMap()["lat"].toString();

    return mRecentPlaces["0"].toMap()["lat"].toString();

}

//  Reset to no recents!
bool PinInformationWidget::resetRecents() {

    QVector<QString> pathList = getExistingRecents();
    int count = 0;

    for(QVector<QString>::iterator it = pathList.begin(); it != pathList.end(); ++it) {
        QFile file(*it);
        if(file.remove())
            ++count;
    }

    QVariantMap* tempMap;
    for(int i = 0; i <= 5; ++i) {
        tempMap = new QVariantMap();

        if(i == 0) {
            tempMap->insert("lat", "1" );
            tempMap->insert("long", "1" );
            mRecentPlaces.insert("0", *tempMap);
        }
        else {
            tempMap->insert("lat", "" );
            tempMap->insert("long", "" );
            mRecentPlaces.insert(QString::number(i), *tempMap);
        }

    }

    //qDebug()<<mRecentPlaces;
    QJsonObject jsonObject = QJsonObject::fromVariantHash(mRecentPlaces);

    QString recentsFile = mDir.absolutePath() + "/resource/jsons/recents/recents.json";
    QFile jsonFile(recentsFile);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(QJsonDocument(jsonObject).toJson(QJsonDocument::Compact));

    return (fileExists(recentsFile) && count==pathList.size());

}
//==============FAVOURITES FUNCTIONS========================
//  Reference the existing favorite items and read the config to register the same
QVariantHash PinInformationWidget::readFavouritesJson() {

    QString favouritesJson = mDir.absolutePath() + "/resource/jsons/favourites/favourites.json";
    QString existingFavouritesJson = mDir.absolutePath() + "/resource/jsons/favourites/existingFavourites.json";

    // Read JSON file
    QFile file1(favouritesJson);
    file1.open(QIODevice::ReadOnly);
    QByteArray rawData1 = file1.readAll();

    QFile file2(existingFavouritesJson);
    file2.open(QIODevice::ReadOnly);
    QByteArray rawData2 = file2.readAll();

    // Parse document
    QJsonDocument doc1(QJsonDocument::fromJson(rawData1));
    QJsonDocument doc2(QJsonDocument::fromJson(rawData2));

    // Get JSON object
    QJsonObject json1 = doc1.object();
    QVariantHash favouritePlaces;

    QJsonObject json2 = doc2.object();

    // Access properties
    //qDebug()<<"\n\n\n\nFavourites List:";

    for(int i = 0; i <= 10;i++) {
        QString idx = QString::number(i);

        //qDebug()<<json1[idx].toObject()["lat"].toString()<<", "<<json1[idx].toObject()["long"].toString();
        QVariantMap tempMap1;
        tempMap1.insert("lat", json1[idx].toObject()["lat"].toString() );
        tempMap1.insert("long", json1[idx].toObject()["long"].toString() );

        favouritePlaces.insert(idx, tempMap1);

        if(i > 0){
            //qDebug()<<json2[idx].toObject()["isExists"].toString();
            QVariantMap tempMap2;
            tempMap2.insert("isExists", json2[idx].toObject()["isExists"].toString() );
            mExistingFav.insert(idx, tempMap2);
        }

    }

    //qDebug()<<"\n\n\n\n";

    return favouritePlaces;

}

//  Check if a favorite POI already exists in memory
bool PinInformationWidget::favouriteExists(double latitude, double longitude, QString* filePath) {
 mFavPlaces = readFavouritesJson();
    QString c_lat = QString::number(latitude);
    QString c_long = QString::number(longitude);

    for(int i = 1; i <= 10; i++) {

        if(mExistingFav[QString::number(i)].toMap()["isExists"].toInt() ==  1) {
            QString t_lat = mFavPlaces[QString::number(i)].toMap()["lat"].toString();
            QString t_long = mFavPlaces[QString::number(i)].toMap()["long"].toString();
            qDebug()<<"i is :"<<i<<t_lat<<t_long;
            if(t_lat == c_lat && t_long == c_long) {
                *filePath = mDir.absolutePath() +
                         "/resource/jsons/favourites/favourites_"+ QString::number(i) +".json";
                return true;
            }

        }

    }

    return false;

}
//  Write POI details to favorite's config
bool PinInformationWidget::writeFavouritesConfig(QString nextWriteLocation, double latitude, double longitude) {

    //qDebug()<<"Latitude is: "<<latitude<<" Longitude is: "<<longitude;
    //qDebug()<<QString::number(latitude)<<", "<<QString::number(longitude);

    QVariantMap* tempMap = new QVariantMap();
    tempMap->insert("lat", QString::number(latitude) );
    tempMap->insert("long", QString::number(longitude) );
    mFavPlaces.insert(nextWriteLocation, *tempMap);

    tempMap = new QVariantMap();
    tempMap->insert("isExists", "1");
    mExistingFav.insert(nextWriteLocation,*tempMap );

    int nextWriteLocationInt = -1;
    for(int i = 1; i <= 10; ++i) {

        if(mExistingFav[QString::number(i)].toMap()["isExists"].toInt() ==  0){
            nextWriteLocationInt = i;
            break;
        }

    }

    if(nextWriteLocationInt == -1)
        nextWriteLocationInt = 1;

    tempMap = new QVariantMap();
    tempMap->insert("lat", QString::number(nextWriteLocationInt) );
    tempMap->insert("long", QString::number(nextWriteLocationInt) );
    mFavPlaces.insert("0", *tempMap);

    //qDebug()<<mFavPlaces;
    QJsonObject jsonObject1 = QJsonObject::fromVariantHash(mFavPlaces);

    QString favouritesFile = mDir.absolutePath() + "/resource/jsons/favourites/favourites.json";
    QFile jsonFile1(favouritesFile);
    jsonFile1.open(QFile::WriteOnly);
    jsonFile1.write(QJsonDocument(jsonObject1).toJson(QJsonDocument::Compact));

    QJsonObject jsonObject2 = QJsonObject::fromVariantHash(mExistingFav);

    QString existingFavouritesFile = mDir.absolutePath() + "/resource/jsons/favourites/existingFavourites.json";
    QFile jsonFile2(existingFavouritesFile);
    jsonFile2.open(QFile::WriteOnly);
    jsonFile2.write(QJsonDocument(jsonObject2).toJson(QJsonDocument::Compact));

    return fileExists(favouritesFile) && fileExists(existingFavouritesFile);
}

//  Write POI details to existing favorite's config
bool PinInformationWidget::writeExistingFavConfig(QVariant variant) {

    QJsonObject jsonObject = variant.value<QJsonObject>();
    QJsonObject json_place = jsonObject["place"].toObject();
    QJsonObject json_location = json_place["location"].toObject();
    QString latitude = QString::number(json_location["latitude"].toDouble(), 'f', 6);
    QString longitude = QString::number(json_location["longitude"].toDouble(), 'f', 6);
    QString fileName = jsonObject["fileName"].toString();
    //qDebug() << fileName;

    QString locEnd = fileName.right(6);
    QString currentLocation = locEnd.left(1);

    QVariantMap* tempMap = new QVariantMap();
    tempMap->insert("lat", "" );
    tempMap->insert("long", "" );
    mFavPlaces.insert(currentLocation, *tempMap);

    tempMap = new QVariantMap();
    tempMap->insert("isExists", "0");
    mExistingFav.insert(currentLocation,*tempMap );

    QJsonObject jsonObject1 = QJsonObject::fromVariantHash(mFavPlaces);

    QString favouritesFile = mDir.absolutePath() + "/resource/jsons/favourites/favourites.json";
    QFile jsonFile1(favouritesFile);
    jsonFile1.open(QFile::WriteOnly);
    jsonFile1.write(QJsonDocument(jsonObject1).toJson(QJsonDocument::Compact));

    QJsonObject jsonObject2 = QJsonObject::fromVariantHash(mExistingFav);

    QString existingFavouritesFile = mDir.absolutePath() + "/resource/jsons/favourites/existingFavourites.json";
    QFile jsonFile2(existingFavouritesFile);
    jsonFile2.open(QFile::WriteOnly);
    jsonFile2.write(QJsonDocument(jsonObject2).toJson(QJsonDocument::Compact));

    return fileExists(favouritesFile) && fileExists(existingFavouritesFile);

}

//To delete selected favourite

bool PinInformationWidget::deleteExistingFavConfig(double lat,double lon) {

   // QJsonObject jsonObject = variant.value<QJsonObject>();
   // QJsonObject json_place = jsonObject["place"].toObject();
    //QJsonObject json_location = json_place["location"].toObject();
    QString latitude = QString::number(lat, 'g', 6);
    QString longitude = QString::number(lon, 'g', 6);
    QString fileName;
    //==================================
        QString c_lat = latitude;
        QString c_long = longitude;
        QString* filePath = new QString();
        for(int i = 1; i <= 10; i++) {

            if(mExistingFav[QString::number(i)].toMap()["isExists"].toInt() ==  1) {
                QString t_lat = mFavPlaces[QString::number(i)].toMap()["lat"].toString();
                QString t_long = mFavPlaces[QString::number(i)].toMap()["long"].toString();

                if(t_lat == c_lat && t_long == c_long) {
                    *filePath = mDir.absolutePath() +
                             "/resource/jsons/favourites/favourites_"+ QString::number(i) +".json";
                    //return true;
                    qDebug()<<"filePathis-909"<<mDir.absolutePath() +
                              "/resource/jsons/favourites/favourites_"+ QString::number(i) +".json";
                    fileName = mDir.absolutePath() +
                            "/resource/jsons/favourites/favourites_"+ QString::number(i) +".json" ;

                }

            }

        }

    //===================================
    qDebug() << "fileName"<<latitude<<longitude<<fileName;

   QString locEnd = fileName.right(6);
    QString currentLocation = locEnd.left(1);

    QVariantMap* tempMap = new QVariantMap();
    tempMap->insert("lat", "" );
    tempMap->insert("long", "" );
    mFavPlaces.insert(currentLocation, *tempMap);

    tempMap = new QVariantMap();
    tempMap->insert("isExists", "0");
    mExistingFav.insert(currentLocation,*tempMap );

   QJsonObject jsonObject1 = QJsonObject::fromVariantHash(mFavPlaces);

    QString favouritesFile = mDir.absolutePath() + "/resource/jsons/favourites/favourites.json";
   QFile jsonFile1(favouritesFile);
   jsonFile1.open(QFile::WriteOnly);
    jsonFile1.write(QJsonDocument(jsonObject1).toJson(QJsonDocument::Compact));

    QJsonObject jsonObject2 = QJsonObject::fromVariantHash(mExistingFav);

    QString existingFavouritesFile = mDir.absolutePath() + "/resource/jsons/favourites/existingFavourites.json";
    QFile jsonFile2(existingFavouritesFile);
    jsonFile2.open(QFile::WriteOnly);
    jsonFile2.write(QJsonDocument(jsonObject2).toJson(QJsonDocument::Compact));
qDebug() << "fileName23"<<fileExists(favouritesFile) <<"233dsf"<<fileExists(existingFavouritesFile);
    return fileExists(favouritesFile) && fileExists(existingFavouritesFile);

}
//  Reset to no favorites
bool PinInformationWidget::resetFavorites() {

    QVector<QString> pathList = getExistingFavourites();
    int count = 0;

    for(QVector<QString>::iterator it = pathList.begin(); it != pathList.end(); ++it) {
        QFile file(*it);

        if(file.remove())
            ++count;

    }

    QVariantMap* tempMap;
    QVariantMap* tempMap1;

    for(int i = 0; i <= 10; ++i) {
        tempMap = new QVariantMap();
        tempMap1 = new QVariantMap();

        if(i == 0){
            tempMap->insert("lat", "1" );
            tempMap->insert("long", "1" );
            mFavPlaces.insert("0", *tempMap);
        }
        else{
            tempMap->insert("lat", "" );
            tempMap->insert("long", "" );
            mFavPlaces.insert(QString::number(i), *tempMap);
            tempMap1->insert("isExists","0");
            mExistingFav.insert(QString::number(i), *tempMap1);
        }

    }

    //qDebug()<<mFavPlaces;
    QJsonObject jsonObject = QJsonObject::fromVariantHash(mFavPlaces);
    QJsonObject jsonObject1 = QJsonObject::fromVariantHash(mExistingFav);

    QString favoriteFile = mDir.absolutePath() + "/resource/jsons/favourites/favourites.json";
    QFile jsonFile(favoriteFile);
    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(QJsonDocument(jsonObject).toJson(QJsonDocument::Compact));
    jsonFile.close();

    QString existingFavouriteFile = mDir.absolutePath() + "/resource/jsons/favourites/existingFavourites.json";
    QFile jsonFile1(existingFavouriteFile);
    jsonFile1.open(QFile::WriteOnly);
    jsonFile1.write(QJsonDocument(jsonObject1).toJson(QJsonDocument::Compact));
    jsonFile1.close();

    return (fileExists(favoriteFile) && fileExists(existingFavouriteFile) && count==pathList.size());

}
//  Get all the active favorite POIs in memory
QVector<QString> PinInformationWidget::getExistingFavourites() {

    QVector<QString> files;

    for(int i = 1; i <= 10; ++i) {

        if(mExistingFav[QString::number(i)].toMap()["isExists"].toInt() == 1) {
            QString path = mDir.absolutePath()+"/resource/jsons/favourites/favourites_" + QString::number(i) + ".json";

            if( fileExists(path) ) {
                files.push_back(path);
            }

        }

    }

    return files;
}

//  Get the instance number 1 - 10 where the favorite pOI info needs to be written
QString PinInformationWidget::getFavouriteWriteLocation() {

    QVariantMap temp = mFavPlaces["0"].toMap();
    //qDebug()<<"Write at file no. "<<mFavPlaces["0"].toMap()["lat"].toString();

    return mFavPlaces["0"].toMap()["lat"].toString();

}
