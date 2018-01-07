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

#ifndef MANEUVERDETAILCONTROLLER_H
#define MANEUVERDETAILCONTROLLER_H

#include <QQuickItem>
#include <QQmlContext>
#include <QDebug>
#include <QFontDatabase>
#include <QPainter>
#include <QPixmap>
#include <QQuickImageProvider>

#include "util.h"
#include "maneuver.h"
#include "navigation.h"
#include "navigationtypes.h"

using namespace locationtoolkit;

class ManeuverDetailController : public QQuickItem
{
    Q_OBJECT
    QQuickItem* mManeuverView;
Q_SIGNALS:
    void AddManeuverArrow(const Maneuver* maneuver);

public:
    ManeuverDetailController(QQuickItem *parent = 0);
    void setUp();
    void updateRouteInfoLabels(QString time, QString distance, QString arrival);
    void updateManeuverInfoLabels(QString nextArrow, QString nextStreetName, QString streetArrow, QString streetName, QString streetDistance, QString currentStreet);
    void clearFields();
    void show();
    void hide();
    QQmlContext* rootContext();
    //    ManeuverList& mManeuverlist;
    //    ManeuverList& getManeuverList();

private slots:
    void on_closeButton_clicked();

public slots:
    void OnTripRemainingTime(quint32 time);
    void OnTripRemainingDistance(qreal dist);
    void OnCurrentRoadName(const QString& primaryName, const QString& secondaryName);
    void OnNextRoadName(const QString& primaryName, const QString& secondaryName);
    void OnStackTurnImageTTF(const QString& stackImageTTF);
    void OnManeuverRemainingDistance(qreal distance);
    //void OnUpdateManeuverList(const ManeuverList& maneuverlist);
    void OnManeuverImageId(const QString& imageId);

    void OnLaneInformation(const locationtoolkit::LaneInformation& laneInfo);
    void OnTripTrafficColor(char color);
    void OnRoadSign(const locationtoolkit::RoadSign& roadSign);

private:
    int getBlockValue(int val);

};

#endif // MANEUVERDETAILCONTROLLER_H
