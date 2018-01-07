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

#ifndef SEARCHDETAILSCONTROLLER_H
#define SEARCHDETAILSCONTROLLER_H
#include <QQuickItem>
#include <QQmlContext>
#include <QQmlEngine>
#include <QDebug>
#include <QFileInfo>
#include "util.h"
#include "dataobject.h"
#include "messageboxcontroller.h"
#include "callviewcontroller.h"
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

using namespace locationtoolkit;

class SerachDetailsController : public QQuickItem
{
    Q_OBJECT
    QQuickItem* mDetailInfoView;
public:
    explicit SerachDetailsController(QQuickItem* parent = 0);
    ~SerachDetailsController();
    //Functions Pertainign to Top 10 Recents
    //    bool favouriteExists(double latititude,double longitude, QString* filepath,QString appPath);
    //    bool writeFavouritesConfig(QString nextWriteLocation, double latitude, double longitude);
    //    QVector<QString>getExistingFavourites(QString);
    //    QString getFavouriteWriteLocation();
    //    bool writeExistingFavConfig(QVariant variant);
    //    bool deleteExistingFavConfig(double latitude,double longitude);
    //    bool resetFavorites();

    //Functions pertaining to Top 5 Recents
    //    bool recentExists(double latititude,double longitude);
    //    bool writeRecentsConfig(QString nextWriteLocation, double latitude, double longitude);
    //    QVector<QString>getExistingRecents(QString);
    //    QString getRecentWriteLocation();
    //    bool resetRecents();
    void setItemDetails(DataObject *itemInfo, QJsonObject jsonObject);

    //    void addToRecentList(QJsonObject itemDetail);

    void closeAllPopUpWindows();

signals:
    void callButtonClickedEvent(QString, QString,QString);
    void pinNavigationClicked(QString, int, QVariant);
    void toggleMapViewFromPinInformation();
    void sideItemVisibility(bool);
private slots:
    //  Button Click functions
    void on_backButton_clicked();
    void on_callButton_clicked(QString,QString, QString);
    void onMapButtonClicked(QString);
    void onNavigationButtonClicked();
    //    QVariantHash readRecenetsJson(QString);
    //    QVariantHash readFavouritesJson(QString);
    bool fileExists(QString path);
    void onFoundCustomFavourite();

    void onOkButtonClicked();
public slots:
    void setUp();
    void onMapButtonClicked();
    void onToggleHeaderBarBackButtonClicked();

public:
    MapWidget* mLTKMapKit;
    void SetLtkMapkit(MapWidget*);
    void InitializeToggleBar();
    void show();
    void hide();
    bool isCatergoryPinDetails;
    bool isSearchDetailPinDetails;
private:
    QVariantHash mRecentPlaces;
    //QDir mDir;
    QVariantHash mFavPlaces;
    //    QVariantHash mExistingFav;
    DataObject *mItemDetails;
    QJsonObject mJsonObject;
    Messageboxcontroller *mMessageBoxView;
    CallViewController *mCallView;
    QString mAppDirPath;
    QQuickItem* mToggleHeaderBar;
    
Q_SIGNALS:
    //close pop-up signal
    void SignalCloseAllPopUpWindows();
    void SignalSetVisible();
    void SignalNavigationButtonClicked(QString, int, QVariant);

};

#endif // SEARCHDETAILSCONTROLLER_H
