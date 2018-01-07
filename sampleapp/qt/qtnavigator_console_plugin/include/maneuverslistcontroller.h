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

#ifndef MANEUVERSLISTCONTROLLER_H
#define MANEUVERSLISTCONTROLLER_H

#include <QQuickItem>
#include <QVariant>
#include <QtCore>

#include <QFontDatabase>
#include <QDir>
#include <QFileInfo>
#include "util.h"
#include "maneuverlist.h"
#include "maneuver.h"
//#include "navigation.h"
//#include "navigationtypes.h"
using namespace locationtoolkit;

class ManeuversListController : public QQuickItem
{

    Q_OBJECT

    QQuickItem* mManeuversListView;

public:

    ManeuversListController(QQuickItem* parent = 0);
    ~ManeuversListController();
    void setUp();
    void setManeuverLabels(QString,QString,QString/*,QString*/);
    void setDestinationName(QString destName);
    void clearFields();
    void show();
    void hide();
    void loadManeuverList();
    void setManeuverList(QList<QObject *>);
Q_SIGNALS:
    void PlayAnounce( int index );
    void StopNavigation();
    void closeButtonClicked();
    void destroyManeuverList();
public slots:
    void onBackButtonClicked();
    void onMapToggleButtonClicked();
    void OnRouteReceived();
    void OnTripRemainingTime(quint32 time);
    void OnTripRemainingDistance(qreal distance);
    void OnManeuverRemainingDistance(qreal distance);
    void OnUpdateManeuverList(const ManeuverList& maneuverlist);
    //   void OnUpdateMeasurmentOption(Preferences::Measurement measurement);
    /*
private slots:
    void on_mapListToggleButton_clicked();
    void onItemClicked(QListWidgetItem* item);

    void on_tweakButton_clicked();
    */
private:
    QString FormatNavTime(quint32 time);
    QString FormatDistance(double dist);
    int getBlockValue(int val);

    QSize* mDimension;

    QString mAppDirPath;
    QString mDirectionIcon,mStreetName,mDistance,mTraffic;
    QList<QObject *> mManeuverDataList;

};

#endif // MANEUVERSLISTCONTROLLER_H
