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

#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QQuickItem>
#include <QTime>
#include "locationprovider.h"
#include "location.h"
#include "polyline.h"
#include "maneuverdetailcontroller.h"
#include "confirmationboxcontroller.h"
#include <QQmlEngine>
#include <QtDBus/QtDBus>

namespace locationtoolkit {
class MapWidget;
class Location;
}

class MapView : public QQuickItem
{
    Q_OBJECT
    Q_DISABLE_COPY(MapView)

    Q_PROPERTY(QString workFolder READ workFolder WRITE setWorkFolder)
    Q_PROPERTY(QPointF avatarPosition READ avatarPosition WRITE setAvatarPosition)
    Q_PROPERTY(float avatarHeading READ avatarHeading WRITE setAvatarHeading)
    Q_PROPERTY(float tilt READ tilt WRITE setTilt)
    Q_PROPERTY(float zoom READ zoom WRITE setZoom)
    Q_PROPERTY(QString token READ token WRITE setToken)
    Q_PROPERTY(bool isFollowMe READ isFollowMe WRITE setIsFollowMe)
    Q_PROPERTY(QString client READ client WRITE setClient)

public:
    MapView(QQuickItem *parent = 0);
    ~MapView();

    void setWorkFolder(QString path);
    QString workFolder();

    QPointF avatarPosition() const;
    void setAvatarPosition(QPointF p);

    float avatarHeading() const;
    void setAvatarHeading(float t);

    float tilt();
    void setTilt(float t);

    float zoom();
    void setZoom(float t);

    QString token();
    void setToken(QString t);

    bool isFollowMe();
    void setIsFollowMe(bool v);

    QString client();
    void setClient(QString p);

    Q_INVOKABLE void setLowFuelAlert(bool f);
signals:

    //void refreshManeuver(QVariant maneuverIcon, QVariant streetName, QVariant distance);
    void refreshManeuver(QString maneuverIcon, QString streetName, QString distance);
    void testSignal();
    void showManeuverView(bool navMode);
    void showClusterUI(int mode);
    //void refreshManeuverDistance(QVariant distance);
    void refreshManeuverDistance(QString distance);



public slots:
    void init();
    void onMapReady();
    Q_SCRIPTABLE QString onManeuverUpdated(const QString& maneuverIcon, const QString& sreetName, const QString& distance, const QString& trafficWarning);
    Q_SCRIPTABLE QString updateGpsPosition(const QString& lat, const QString& lon, const QString& heading);
    Q_SCRIPTABLE QString setNavigationMode(bool navMode);
    Q_SCRIPTABLE QString setClusterUIMode(int Mode);
    Q_SCRIPTABLE QString updateRemainingManeuverDistance(const QString& distance);
    Q_SCRIPTABLE QString updatePolyline(const QString &polyline);
    Q_SCRIPTABLE QString clearPolyline();
    Q_SCRIPTABLE QString updateManeuverArrow(const QString &pts);
    Q_SCRIPTABLE QString onUpdateSpeedLimit(const QString &pts);
    Q_SCRIPTABLE QString hideFuelAlert(bool mode,bool navmode);
   // Q_SCRIPTABLE QString isFuelNavMode(bool navmode);

    void onSetClusterUI(int);
    void onLowFuelLimit(bool);

    void dbusCallFinishedSlot(QDBusPendingCallWatcher* watcher);



protected:
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *);
    void updateAvatarLocation(const locationtoolkit::Location& location);

private:
    bool mInitDone;
    bool mMapReady;
    locationtoolkit::MapWidget* globalMapWidget;
    locationtoolkit::LocationConfiguration mLocationConfiguration;
    QPointF mAvatarPosition;
    float mAvatarHeading;
    float mTilt;
    float mZoom;
    bool mIsFollowMe;
    QString mClient;
    QString mToken;
    QTime mTime;
    ManeuverDetailController *mManeuverController;
    ConfirmationBoxController *mConfirmationController;
    QQuickItem *mSpeedLimitView;
    void setupManeuverDetailSignalAndSlots();
    void setupConfirmationBox();
    void setupSpeedLimitView();
    QList<locationtoolkit::Polyline*> mManeuverArrows;
    QList<locationtoolkit::Polyline*> activePolylines;

    //QDBusInterface iface;

};

#endif // MAPVIEW_H
