/****************************************************************************
**
** Copyright (C) 2015 Pelagicore AG
** Contact: http://www.qt.io/ or http://www.pelagicore.com/
**
** This file is part of the QtIVI module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3-PELAGICORE$
** Commercial License Usage
** Licensees holding valid commercial Qt IVI licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Pelagicore. For licensing terms
** and conditions, contact us at http://www.pelagicore.com.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** $QT_END_LICENSE$
**
** SPDX-License-Identifier: LGPL-3.0
**
****************************************************************************/

#ifndef CLUSTERDATABACKEND_H
#define CLUSTERDATABACKEND_H

#include <QObject>
#include <QMap>
#include <QVariant>

#ifndef CLUSTERDATA_SIMULATION
#include <QCanBusDevice>
#include <QCanBusFrame>
#else
#include <QTcpServer>
#include <QTcpSocket>
class QTimer;
#endif

class ClusterDataBackend : public QObject
{
    Q_OBJECT

public:
    ClusterDataBackend(QObject* parent=0);
    ~ClusterDataBackend();

public:
    QStringList availableZones() const;
    QMap<QString, QVariantMap> zoneMap() const;
    void initializeAttributes();

signals:
    void vehicleSpeedChanged(double vehicleSpeed, const QString &zone = QString());
    void latitudeChanged(double latitude, const QString &zone = QString());
    void longitudeChanged(double longitude, const QString &zone = QString());
    void directionChanged(double direction, const QString &zone = QString());
    void flatTireChanged(bool flatTire, const QString &zone = QString());
    void doorOpenChanged(bool doorOpen, const QString &zone = QString());
    void lightFailureChanged(bool lightFailure, const QString &zone = QString());
    void reverseChanged(bool Reverse, const QString &zone = QString());
    void leftTurnLightChanged(bool leftTurnLight, const QString &zone = QString());
    void rightTurnLightChanged(bool rightTurnLight, const QString &zone = QString());
    void headLightChanged(bool headLight, const QString &zone = QString());
    void parkLightChanged(bool parkLight, const QString &zone = QString());
    void carIdChanged(int carId, const QString &zone = QString());
    void brakeChanged(bool brakeOn, const QString &zone = QString());
    void engineTempChanged(int engineTemp, const QString &zone = QString());
    void oilTempChanged(double oilTemp, const QString &zone = QString());
    void oilPressureChanged(int oilPressure, const QString &zone = QString());
    void batteryPotentialChanged(double batteryPotential, const QString &zone = QString());
    void gasLevelChanged(double gasLevel, const QString &zone = QString());
    void rpmChanged(int rpm, const QString &zone = QString());
    void gearChanged(int gear, const QString &zone = QString());

private:
#ifndef CLUSTERDATA_SIMULATION
    void handlePGN65268(const QCanBusFrame& frame); // Tire Condition - TIRE
    void handlePGN64933(const QCanBusFrame& frame); // Door Control 2 - DC2
    void handlePGN64972(const QCanBusFrame& frame); // Light Control
    void handlePGN61445(const QCanBusFrame& frame); // Electronic Transmission Controller 2 - ETC2
    void handlePGN65265(const QCanBusFrame& frame); // Cruise Control/Vehicle Speed - CCVS
    void handlePGN65267(const QCanBusFrame& frame); // vehicle position - VP
    void handlePGN65256(const QCanBusFrame& frame); // vehicle direction/speed
    void handlePGN65262(const QCanBusFrame& frame); // Engine Temperature 1 - ET1
    void handlePGN65263(const QCanBusFrame& frame); // Engine Fluid Level/Pressure 1 - EFL/P1
    void handlePGN65271(const QCanBusFrame& frame); // Vehicle Electrical Power - VEP
    void handlePGN65276(const QCanBusFrame& frame); // Dash Display - DD
    void handlePGN61444(const QCanBusFrame& frame); // Electronic Engine Controller 1 - EEC1
    void handlePGN60928(const QCanBusFrame& frame); // Address Claimed - ACL

private slots:
    void canError(QCanBusDevice::CanBusError error);
    void canFramesReceived();
    void canStateChanged(QCanBusDevice::CanBusDeviceState state);
#else
public slots:
    void acceptConnection();
private slots:
    void updateServerProgress();
#endif

private:
#ifndef CLUSTERDATA_SIMULATION
    QCanBusDevice* m_canDevice;
#else
    QTcpServer m_tcpServer;
    QTcpSocket *m_tcpServerConnection;
#endif
    double m_vehicleSpeed;
    double m_latitude;
    double m_longitude;
    double m_direction;
    bool m_flatTire;
    QMap<QString,bool> m_doorOpen;
    QMap<QString, QVariantMap> m_zonedFeatures;
    bool m_lightFailure;
    int m_gear;
    char m_signalLights;
    bool m_headLight;
    bool m_parkLight;
    int m_carId;
    bool m_brake;
    qint8 m_engineTemp;
    double m_oilTemp;
    int m_oilPressure;
    double m_batteryPotential;
    double m_gasLevel;
    int m_rpm;

};

#endif // CLUSTERDATABACKEND_H
