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

#ifndef PININFORMATIONWIDGET_H
#define PININFORMATIONWIDGET_H

#include <QQuickView>
#include <QQuickItem>
#include <QQmlContext>
#include <QDebug>

#include "callviewcontroller.h"
#include "util.h"

class PinInformationWidget : public QObject//QQuickItem//QQuickView
{
    Q_OBJECT

public:
     SettingsController(QQuickItem* parent = 0);
    //Functions Pertainign to Top 10 Recents
    bool favouriteExists(double latititude,double longitude, QString* filepath);
    bool writeFavouritesConfig(QString nextWriteLocation, double latitude, double longitude);
    QVector<QString>getExistingFavourites();
    QString getFavouriteWriteLocation();
    bool writeExistingFavConfig(QVariant variant);
    bool deleteExistingFavConfig(double latitude,double longitude);
    bool resetFavorites();
    //Functions pertaining to Top 5 Recents
    bool recentExists(double latititude,double longitude);
    bool writeRecentsConfig(QString nextWriteLocation, double latitude, double longitude);
    QVector<QString>getExistingRecents();
    QString getRecentWriteLocation();
    bool resetRecents();

signals:
    void callButtonClickedEvent(QString, QString,QString);
    void pinNavigationClicked(QString, int, QVariant);
    void toggleMapViewFromPinInformation();

private slots:
    //  Button Click functions
   void on_backButton_clicked();
   void on_callButton_clicked(QString,QString, QString);
   void onMapButtonClicked();
   void onNavigationButtonClicked();
   QVariantHash readRecenetsJson();
   QVariantHash readFavouritesJson();
   bool fileExists(QString path);

public slots:
    void setUp();

private:
    CallViewController* mCallView;
    QVariantHash mRecentPlaces;
    QDir mDir;
    QVariantHash mFavPlaces;
    QVariantHash mExistingFav;

};

#endif // PININFORMATIONWIDGET_H
