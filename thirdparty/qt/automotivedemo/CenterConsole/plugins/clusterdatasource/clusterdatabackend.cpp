#include "clusterdatabackend.h"

#include <QDebug>
#include <QtGlobal>
#include <QHostAddress>
#include <QNetworkInterface>

#ifndef CLUSTERDATA_SIMULATION
// #define DEBUG_CAN
#include <QtMath>
#include <QCanBus>
#define SOCKETCAN "socketcan"
#define CANINTERFACE "can0"

// J1939 PGNs
#define PGN_65268 0xFEF4 // Tire Condition - TIRE
#define PGN_64933 0xFDA5 // Door Control 2 - DC2
#define PGN_64972 0xFDCC // Light Control
#define PGN_61445 0xF005 // Electronic Transmission Controller 2 - ETC2
#define PGN_65265 0xFEF1 // Cruise Control/Vehicle Speed - CCVS
#define PGN_65267 0xFEF3 // vehicle position - VP
#define PGN_65256 0xFEE8 // vehicle direction/speed
#define PGN_65262 0xFEEE // Engine Temperature 1 - ET1
#define PGN_65263 0xFEEF // Engine Fluid Level/Pressure 1 - EFL/P1
#define PGN_65271 0xFEF7 // Vehicle Electrical Power - VEP
#define PGN_65276 0xFEFC // Dash Display - DD
#define PGN_61444 0xF004 // Electronic Engine Controller 1 - EEC1
#define PGN_60928 0xEE00 // Address Calimed - ACL
#endif

// door defines
#define OPEN_DOOR_1 0x400000000000000
#define OPEN_DOOR_2 0x1000000000000
#define OPEN_DOOR_3 0x40000000000000
#define OPEN_DOOR_4 0x100000000000
#define OPEN_DOOR_5 0x400000000
#define OPEN_DOOR_6 0x1000000

// light defines
#define HEAD_LIGHT 0x200000000000000
#define PARK_LIGHT 0x100000000000000
#define LIGHT_ERROR 0x400000000000000
#define LEFT_TURN_SIGNAL 0x10000000000000
#define RIGHT_TURN_SIGNAL 0x20000000000000
#define HAZARD_SIGNAL 0x4000000000000

#define LEFT_BLINK_ON 0x01
#define RIGHT_BLINK_ON 0x02
#define HAZARD_ON 0x4

// Current gear defines
#define GEAR_PARK 0xFB
#define GEAR_REVERSE 0x7C
#define GEAR_NEUTRAL 0x7D
// #define GEAR_1 0x7E
// #define GEAR_2 0x7F
// #define GEAR_3 0x80
// #define GEAR_4 0x81
// #define GEAR_5 0x82
// #define GEAR_6 0x83

ClusterDataBackend::ClusterDataBackend(QObject* parent) :
    QObject(parent),
#ifndef CLUSTERDATA_SIMULATION
    m_canDevice(Q_NULLPTR),
#endif
    m_vehicleSpeed(0.0),
    m_latitude(52.53471566),
    m_longitude(13.29339621),
    m_direction(0.0),
    m_flatTire(false),
    m_lightFailure(false),
    m_gear(1),
    m_signalLights(0x0),
    m_headLight(false),
    m_parkLight(false),
    m_carId(2),
    m_brake(false),
    m_engineTemp(60),
    m_oilTemp(0.0),
    m_oilPressure(0),
    m_batteryPotential(80.0),
    m_gasLevel(67.0),
    m_rpm(4000)
{

    m_doorOpen.insert("frontLeft", false);
    m_doorOpen.insert("frontRight", false);
    m_doorOpen.insert("rearLeft", false);
    m_doorOpen.insert("rearRight", false);
    m_doorOpen.insert("trunk", false);
    m_doorOpen.insert("hood", false);

#ifndef CLUSTERDATA_SIMULATION
    QCanBus* canBus(QCanBus::instance());
    QList<QByteArray> plugins(canBus->plugins());
    if (plugins.contains(SOCKETCAN)) {
        m_canDevice = canBus->createDevice(SOCKETCAN, CANINTERFACE);
        if (m_canDevice) {
            connect(m_canDevice, SIGNAL(errorOccurred(QCanBusDevice::CanBusError)), this, SLOT(canError(QCanBusDevice::CanBusError)));
            connect(m_canDevice, SIGNAL(framesReceived()), this, SLOT(canFramesReceived()));
#ifdef DEBUG_CAN
            connect(m_canDevice, SIGNAL(stateChanged(QCanBusDevice::CanBusDeviceState)), this, SLOT(canStateChanged(QCanBusDevice::CanBusDeviceState)));
            qDebug() << "ClusterDataBackend::ClusterDataBackend - connectDevice:" <<
#endif // DEBUG_CAN
            m_canDevice->connectDevice();
        }
    }
#endif // !CLUSTERDATA_SIMULATION
}

ClusterDataBackend::~ClusterDataBackend()
{
#ifndef CLUSTERDATA_SIMULATION
    if (m_canDevice) {
        m_canDevice->disconnectDevice();
    }
#else
    if (m_tcpServerConnection)
        m_tcpServerConnection->close();
    m_tcpServer.close();
#endif
}

QStringList ClusterDataBackend::availableZones() const
{
    return m_doorOpen.keys();
}

QMap<QString, QVariantMap> ClusterDataBackend::zoneMap() const
{
    return m_zonedFeatures;
}

void ClusterDataBackend::initializeAttributes()
{
    emit vehicleSpeedChanged(m_vehicleSpeed);
    emit latitudeChanged(m_latitude);
    emit longitudeChanged(m_longitude);
    emit directionChanged(m_direction);
    emit flatTireChanged(m_flatTire);
    foreach (QString zone, m_doorOpen.keys()) {
        emit doorOpenChanged(m_doorOpen.value(zone), zone);
    }
    emit lightFailureChanged(m_lightFailure);
    emit reverseChanged(m_gear < 0);
    emit leftTurnLightChanged(m_signalLights & LEFT_BLINK_ON);
    emit rightTurnLightChanged(m_signalLights & RIGHT_BLINK_ON);
    emit headLightChanged(m_headLight);
    emit carIdChanged(m_carId);

#ifdef CLUSTERDATA_SIMULATION
    QHostAddress simuAddr("127.0.0.1");

    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    foreach (QNetworkInterface interface, interfaces) {
        QList<QNetworkAddressEntry> addressEntries = interface.addressEntries();

        foreach(QNetworkAddressEntry entry, addressEntries) {
            if(entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                simuAddr = entry.ip();
            }
        }
    }
    if (m_tcpServer.listen(simuAddr, 32123)) {
        connect(&m_tcpServer, SIGNAL(newConnection()),
                this, SLOT(acceptConnection()));
        qDebug() << "ClusterData server listening " << simuAddr << m_tcpServer.serverPort();
    } else {
        qWarning() << m_tcpServer.errorString();
    }
#endif
}

#ifdef CLUSTERDATA_SIMULATION
void ClusterDataBackend::acceptConnection()
{
    m_tcpServerConnection = m_tcpServer.nextPendingConnection();
    connect(m_tcpServerConnection, &QTcpSocket::readyRead,
            this, &ClusterDataBackend::updateServerProgress);
}

void ClusterDataBackend::updateServerProgress()
{
    QString msg(m_tcpServerConnection->readAll());

    foreach (QString msgAttr, msg.split(" ",QString::SkipEmptyParts)) {
        QString attribute = msgAttr.section(':',0,0);
        QString value = msgAttr.section(':',1,1);
        if (attribute == "vehicleSpeed") {
            emit vehicleSpeedChanged(value.toDouble());
        } else if (attribute == "latitude") {
            emit latitudeChanged(value.toDouble());
        } else if (attribute == "longitude") {
            emit longitudeChanged(value.toDouble());
        } else if (attribute == "direction") {
            emit directionChanged(value.toDouble());
        } else if (attribute == "headLight") {
            emit headLightChanged((bool)value.toInt());
        } else if (attribute == "lightFailure") {
            emit lightFailureChanged((bool)value.toInt());
        } else if (attribute == "flatTire") {
            emit flatTireChanged((bool)value.toInt());
        } else if (attribute == "reverse") {
            emit reverseChanged((bool)value.toInt());
        } else if (attribute == "leftTurnLight") {
            emit leftTurnLightChanged((bool)value.toInt());
        } else if (attribute == "rightTurnLight") {
            emit rightTurnLightChanged((bool)value.toInt());
        } else if (attribute == "headLight") {
            emit headLightChanged((bool)value.toInt());
        } else if (attribute == "parkLight") {
            emit parkLightChanged((bool)value.toInt());
        } else if (attribute == "carId") {
            emit carIdChanged(value.toInt());
        } else if (attribute == "frontLeftDoorOpen") {
            emit doorOpenChanged((bool)value.toInt(), "frontLeft");
        } else if (attribute == "frontRightDoorOpen") {
            emit doorOpenChanged((bool)value.toInt(), "frontRight");
        } else if (attribute == "rearLeftDoorOpen") {
            emit doorOpenChanged((bool)value.toInt(), "rearLeft");
        } else if (attribute == "rearRightDoorOpen") {
            emit doorOpenChanged((bool)value.toInt(), "rearRight");
        } else if (attribute == "hoodOpen") {
            emit doorOpenChanged((bool)value.toInt(), "hood");
        } else if (attribute == "trunkOpen") {
            emit doorOpenChanged((bool)value.toInt(), "trunk");
        } else if (attribute == "hazardSignal") {
            emit leftTurnLightChanged((bool)value.toInt());
            emit rightTurnLightChanged((bool)value.toInt());
        } else if (attribute == "brake") {
           emit brakeChanged((bool)value.toInt());
        } else if (attribute == "engineTemp") {
           emit engineTempChanged(value.toInt());
        } else if (attribute == "oilTemp") {
           emit oilTempChanged(value.toDouble());
        } else if (attribute == "oilPressure") {
           emit oilPressureChanged(value.toInt());
        } else if (attribute == "batteryPotential") {
           emit batteryPotentialChanged(value.toDouble());
        } else if (attribute == "gasLevel") {
           emit gasLevelChanged(value.toDouble());
        } else if (attribute == "rpm") {
             emit rpmChanged(value.toInt());
        } else if (attribute == "gear") {
            emit gearChanged(value.toInt());
        }
        else {
            qWarning() << "Unknown msg" << msg;
        }
    }
}
#else
// Tire Condition - TIRE
void ClusterDataBackend::handlePGN65268(const QCanBusFrame& frame)
{
    QByteArray data(frame.payload());
    if (data.size() == 8) {
//        char tireId(data.at(0));
//        char pressure(data.at(1));
//        quint16 temperatureData(data.at(2) << 8);
//        temperatureData |= data.at(3);
//        double tireTemperature((0.03125 * (double)temperatureData) - 273);
        m_flatTire = (data.at(4) & 0x30);
#ifdef DEBUG_CAN
        qDebug("ClusterDataBackend::handlePGN65268 - m_flatTire: %s", (m_flatTire ? "true" : "false"));
#endif // DEBUG_CAN
        emit flatTireChanged(m_flatTire);
    }
}

// Door Control 2 - DC2
void ClusterDataBackend::handlePGN64933(const QCanBusFrame& frame)
{
    QByteArray data(frame.payload());
    if (data.size() == 8) {
        quint64 doorData((quint64)data.at(0) << 56);
        doorData |= ((quint64)data.at(1) << 48);
        doorData |= ((quint64)data.at(2) << 40);
        doorData |= ((quint64)data.at(3) << 32);
        doorData |= ((quint64)data.at(4) << 24);
        doorData |= ((quint64)data.at(5) << 16);
        doorData |= ((quint64)data.at(6) << 8);
        doorData |= (quint64)data.at(7);

        bool frontLeft(doorData & OPEN_DOOR_1);
        bool frontRight(doorData & OPEN_DOOR_2);
        bool rearLeft(doorData & OPEN_DOOR_3);
        bool rearRight(doorData & OPEN_DOOR_4);
        bool trunk(doorData & OPEN_DOOR_5);
        bool hood(doorData & OPEN_DOOR_6);

        if (m_doorOpen.value("frontLeft") != frontLeft) {
            m_doorOpen.insert("frontLeft", frontLeft);
#ifdef DEBUG_CAN
            qDebug("ClusterDataBackend::handlePGN64933 - doorOpenChanged, frontLeft: %s", (frontLeft ? "true" : "false"));
#endif // DEBUG_CAN
            emit doorOpenChanged(frontLeft, "frontLeft");
        }
        if (m_doorOpen.value("frontRight") != frontRight) {
            m_doorOpen.insert("frontRight", frontRight);
#ifdef DEBUG_CAN
            qDebug("ClusterDataBackend::handlePGN64933 - doorOpenChanged, frontRight: %s", (frontRight ? "true" : "false"));
#endif // DEBUG_CAN
            emit doorOpenChanged(frontRight, "frontRight");
        }
        if (m_doorOpen.value("rearLeft") != rearLeft) {
            m_doorOpen.insert("rearLeft", rearLeft);
#ifdef DEBUG_CAN
            qDebug("ClusterDataBackend::handlePGN64933 - doorOpenChanged, rearLeft: %s", (rearLeft ? "true" : "false"));
#endif // DEBUG_CAN
            emit doorOpenChanged(rearLeft, "rearLeft");
        }
        if (m_doorOpen.value("rearRight") != rearRight) {
            m_doorOpen.insert("rearRight", rearRight);
#ifdef DEBUG_CAN
            qDebug("ClusterDataBackend::handlePGN64933 - doorOpenChanged, rearRight: %s", (rearRight ? "true" : "false"));
#endif // DEBUG_CAN
            emit doorOpenChanged(rearRight, "rearRight");
        }
        if (m_doorOpen.value("trunk") != trunk) {
            m_doorOpen.insert("trunk", trunk);
#ifdef DEBUG_CAN
            qDebug("ClusterDataBackend::handlePGN64933 - doorOpenChanged, trunk: %s", (trunk ? "true" : "false"));
#endif // DEBUG_CAN
            emit doorOpenChanged(trunk, "trunk");
        }
        if (m_doorOpen.value("hood") != hood) {
            m_doorOpen.insert("hood", hood);
#ifdef DEBUG_CAN
            qDebug("ClusterDataBackend::handlePGN64933 - doorOpenChanged, hood: %s", (hood ? "true" : "false"));
#endif // DEBUG_CAN
            emit doorOpenChanged(hood, "hood");
        }
   }
}

// Light Control
void ClusterDataBackend::handlePGN64972(const QCanBusFrame& frame)
{
    QByteArray data(frame.payload());
    if (data.size() == 8) {
        quint64 lightData((quint64)data.at(0) << 56);
        lightData |= ((quint64)data.at(1) << 48);
        lightData |= ((quint64)data.at(2) << 40);
        lightData |= ((quint64)data.at(3) << 32);
        lightData |= ((quint64)data.at(4) << 24);
        lightData |= ((quint64)data.at(5) << 16);
        lightData |= ((quint64)data.at(6) << 8);
        lightData |= data.at(7);

        if (m_headLight != (lightData & HEAD_LIGHT)) {
            m_headLight = (lightData & HEAD_LIGHT);
#ifdef DEBUG_CAN
            qDebug("ClusterDataBackend::handlePGN64972 - m_headLight: %s", (m_headLight ? "true" : "false"));
#endif // DEBUG_CAN
            emit headLightChanged(m_headLight);
        }
        if (m_parkLight != (lightData & PARK_LIGHT)) {
            m_parkLight = (lightData & PARK_LIGHT);
#ifdef DEBUG_CAN
            qDebug("ClusterDataBackend::handlePGN64972 - m_parkLight: %s", (m_parkLight ? "true" : "false"));
#endif // DEBUG_CAN
            emit parkLightChanged(m_parkLight);
        }
        if (m_lightFailure != (lightData & LIGHT_ERROR)) {
            m_lightFailure = (lightData & LIGHT_ERROR);
#ifdef DEBUG_CAN
            qDebug("ClusterDataBackend::handlePGN64972 - m_lightFailure: %s", (m_lightFailure ? "true" : "false"));
#endif // DEBUG_CAN
            emit lightFailureChanged(m_lightFailure);
        }
        if ((bool)(m_signalLights & LEFT_BLINK_ON) != (bool)(lightData & LEFT_TURN_SIGNAL)) {
            if (lightData & LEFT_TURN_SIGNAL) {
                m_signalLights |= LEFT_BLINK_ON;
            }
            else {
                m_signalLights &= ~LEFT_BLINK_ON;
            }
#ifdef DEBUG_CAN
            qDebug("ClusterDataBackend::handlePGN64972 - leftTurnLight: %s", ((lightData & LEFT_TURN_SIGNAL) ? "true" : "false"));
#endif // DEBUG_CAN
            emit leftTurnLightChanged(lightData & LEFT_TURN_SIGNAL);
        }
        if ((bool)(m_signalLights & RIGHT_BLINK_ON) != (bool)(lightData & RIGHT_TURN_SIGNAL)) {
            if (lightData & RIGHT_TURN_SIGNAL) {
                m_signalLights |= RIGHT_BLINK_ON;
            }
            else {
                m_signalLights &= ~RIGHT_BLINK_ON;
            }
#ifdef DEBUG_CAN
            qDebug("ClusterDataBackend::handlePGN64972 - rightTurnLight: %s", ((lightData & RIGHT_TURN_SIGNAL) ? "true" : "false"));
#endif // DEBUG_CAN
            emit rightTurnLightChanged(lightData & RIGHT_TURN_SIGNAL);
        }
        if ((bool)(m_signalLights & HAZARD_ON) != (bool)(lightData & HAZARD_SIGNAL)) {
            if (lightData & HAZARD_SIGNAL) {
                m_signalLights |= HAZARD_ON;
                if (!(m_signalLights & LEFT_BLINK_ON)) {
#ifdef DEBUG_CAN
                    qDebug("ClusterDataBackend::handlePGN64972 - hazard ON, leftTurnLight: true");
#endif // DEBUG_CAN
                    emit leftTurnLightChanged(true);
                }
                if (!(m_signalLights & RIGHT_BLINK_ON)) {
#ifdef DEBUG_CAN
                    qDebug("ClusterDataBackend::handlePGN64972 - hazard ON, rightTurnLight: true");
#endif // DEBUG_CAN
                    emit rightTurnLightChanged(true);
                }
            }
            else {
                m_signalLights &= ~HAZARD_ON;
                if (!(m_signalLights & LEFT_BLINK_ON)) {
#ifdef DEBUG_CAN
                    qDebug("ClusterDataBackend::handlePGN64972 - hazard OFF, leftTurnLight: false");
#endif // DEBUG_CAN
                    emit leftTurnLightChanged(false);
                }
                if (!(m_signalLights & RIGHT_BLINK_ON)) {
#ifdef DEBUG_CAN
                    qDebug("ClusterDataBackend::handlePGN64972 - hazard OFF, rightTurnLight: false");
#endif // DEBUG_CAN
                    emit rightTurnLightChanged(false);
                }
            }
        }
    }
}

// Electronic Transmission Controller 2 - ETC2
void ClusterDataBackend::handlePGN61445(const QCanBusFrame& frame)
{
    QByteArray data(frame.payload());
    if (data.size() == 8) {
        char gear(data.at(3));
        if (gear == (char)GEAR_PARK) {
            m_gear = 0xff;
        }
        else if (m_gear == -1 && gear != (char)GEAR_REVERSE) {
            m_gear = (gear - GEAR_NEUTRAL);
            emit reverseChanged(false);
        }
        else if (m_gear != -1 && gear == (char)GEAR_REVERSE) {
            m_gear = -1;
            emit reverseChanged(true);
        }
        else {
            m_gear = (gear - GEAR_NEUTRAL);
        }
#ifdef DEBUG_CAN
            qDebug("ClusterDataBackend::handlePGN61445 - gear: %d", m_gear);
#endif // DEBUG_CAN
        emit gearChanged(m_gear);
    }
}

// Cruise Control/Vehicle Speed - CCVS
void ClusterDataBackend::handlePGN65265(const QCanBusFrame& frame)
{
    QByteArray data(frame.payload());
    if (data.size() == 8) {
        qint16 speed((data.at(1) << 8));
        speed |= data.at(2);
        double vehicleSpeed(0.00390625 * (double)speed);

        if (m_vehicleSpeed != vehicleSpeed) {
            m_vehicleSpeed = vehicleSpeed;
#ifdef DEBUG_CAN
            qDebug("ClusterDataBackend::handlePGN65265 - vehicle speed: %f", m_vehicleSpeed);
#endif // DEBUG_CAN
            emit vehicleSpeedChanged(m_vehicleSpeed);
        }

        bool brakeOn(data.at(3) & 0x10);
        if (m_brake != brakeOn) {
            m_brake = brakeOn;
#ifdef DEBUG_CAN
            qDebug("ClusterDataBackend::handlePGN65265 - m_brake: %s", (m_brake ? "true" : "false"));
#endif // DEBUG_CAN
            emit brakeChanged(m_brake);
        }
    }
}

// vehicle position - VP
void ClusterDataBackend::handlePGN65267(const QCanBusFrame& frame)
{
    QByteArray data(frame.payload());

    if (data.size() == 8) {
        quint32 lat((data.at(0) << 24));
        lat |= ((data.at(1) << 16));
        lat |= ((data.at(2) << 8));
        lat |= data.at(3);

        quint32 lon(data.at(4) << 24);
        lon |= (data.at(5) << 16);
        lon |= (data.at(6) << 8);
        lon |= data.at(7);

        double latitude((((double)lat) * 1e-7) - 210);
        double longitude((((double)lon) * 1e-7) - 210);

        bool positionChanged(false);
        if (m_latitude != latitude) {
            m_latitude = latitude;
            positionChanged = true;
        }
        if (m_longitude != longitude) {
            m_longitude = longitude;
            positionChanged = true;
        }
        if (positionChanged) {
#ifdef DEBUG_CAN
            qDebug("ClusterDataBackend::handlePGN129025 - lat: %f, lon: %f", m_latitude, m_longitude);
#endif // DEBUG_CAN
            emit latitudeChanged(m_latitude);
            emit longitudeChanged(m_longitude);
        }
    }
}

// vehicle direction/speed
void ClusterDataBackend::handlePGN65256(const QCanBusFrame& frame)
{
    QByteArray data(frame.payload());
    if (data.size() == 8) {
        quint16 directionData((data.at(0) << 8));
        directionData |= data.at(1);
        double direction(0.0078125 * (double)directionData); // 1/128 deg/bit

        if (m_direction != direction) {
            m_direction = direction;
#ifdef DEBUG_CAN
            qDebug("ClusterDataBackend::handlePGN65256 - m_direction: %f", m_direction);
#endif // DEBUG_CAN
            emit directionChanged(m_direction);
        }
    }
}

// Engine Temperature 1 - ET1
void ClusterDataBackend::handlePGN65262(const QCanBusFrame& frame)
{
    QByteArray data(frame.payload());
    if (data.size() == 8) {
        qint8 engineTemp(data.at(0) - 40);
        if (m_engineTemp != engineTemp) {
            m_engineTemp = engineTemp;
#ifdef DEBUG_CAN
            qDebug("ClusterDataBackend::handlePGN65262 - m_engineTemp: %d", m_engineTemp);
#endif // DEBUG_CAN
            emit engineTempChanged(m_engineTemp);
        }
        quint16 oilTempData(data.at(2) << 8);
        oilTempData |= data.at(3);
        double oilTemp(0.03125 * ((double)oilTempData) - 273.0);
        if (m_oilTemp != oilTemp) {
            m_oilTemp = oilTemp;
#ifdef DEBUG_CAN
            qDebug("ClusterDataBackend::handlePGN65262 - m_oilTemp: %f", m_oilTemp);
#endif // DEBUG_CAN
            emit oilTempChanged(m_oilTemp);
        }
    }
}

// Engine Fluid Level/Pressure 1 - EFL/P1
void ClusterDataBackend::handlePGN65263(const QCanBusFrame& frame)
{
    QByteArray data(frame.payload());
    if (data.size() == 8) {
        int oilPressure(4 * data.at(3));
        if (m_oilPressure != oilPressure) {
            m_oilPressure = oilPressure;
#ifdef DEBUG_CAN
            qDebug("ClusterDataBackend::handlePGN65263 - m_oilPressure: %d", m_oilPressure);
#endif // DEBUG_CAN
            emit oilPressureChanged(m_oilPressure);
        }
    }
}

// Vehicle Electrical Power - VEP
void ClusterDataBackend::handlePGN65271(const QCanBusFrame& frame)
{
    QByteArray data(frame.payload());
    if (data.size() == 8) {
        quint16 batteryPotentialData(data.at(6) << 8);
        batteryPotentialData |= data.at(7);
        double batteryPotential(0.05 * (double)batteryPotentialData);
        if (m_batteryPotential != batteryPotential) {
            m_batteryPotential = batteryPotential;
#ifdef DEBUG_CAN
            qDebug("ClusterDataBackend::handlePGN65271 - m_batteryPotential: %f", m_batteryPotential);
#endif // DEBUG_CAN
            emit batteryPotentialChanged(m_batteryPotential);
        }
    }
}

// Dash Display - DD
void ClusterDataBackend::handlePGN65276(const QCanBusFrame& frame)
{
    QByteArray data(frame.payload());
    if (data.size() == 8) {
        double gasLevel(0.4 * (double)data.at(1));
        if (m_gasLevel != gasLevel) {
            m_gasLevel = gasLevel;
#ifdef DEBUG_CAN
            qDebug("ClusterDataBackend::handlePGN65276 - m_gasLevel: %f", m_gasLevel);
#endif // DEBUG_CAN
            emit gasLevelChanged(m_gasLevel);
        }
    }
}

// Electronic Engine Controller 1 - EEC1
void ClusterDataBackend::handlePGN61444(const QCanBusFrame& frame)
{
    QByteArray data(frame.payload());
    if (data.size() == 8) {
        quint16 rpmData(data.at(3) << 8);
        rpmData |= data.at(4);
        int rpm(0.125 * (double)rpmData);
        if (m_rpm != rpm) {
            m_rpm = rpm;
#ifdef DEBUG_CAN
            qDebug("ClusterDataBackend::handlePGN61444 - m_rpm: %d", m_rpm);
#endif // DEBUG_CAN
            emit rpmChanged(m_rpm);
        }
    }
}

// Address Claimed - ACL
void ClusterDataBackend::handlePGN60928(const QCanBusFrame &frame)
{
    QByteArray data(frame.payload());
    if (data.size() == 8) {
        // only 21 bits allowed (0x0 - 0x1FFFFF), pos 1.1 - 3.5 in message
        quint16 carIdData((data.at(0) & 0xff) << 13);
        carIdData |= ((data.at(1) & 0xff) << 5);
        carIdData |= ((data.at(2) & 0xf8) >> 3);
        if (m_carId != (int)carIdData) {
            m_carId = carIdData;
#ifdef DEBUG_CAN
            qDebug("ClusterDataBackend::handlePGN60928 - carId: %d", m_carId);
#endif // DEBUG_CAN
            emit carIdChanged(m_carId);
        }
    }
}

void ClusterDataBackend::canError(QCanBusDevice::CanBusError error)
{
    qWarning("ClusterDataBackend::canError - code: %d, error: %s", (int)error, qPrintable(m_canDevice->errorString()));
    m_canDevice->disconnectDevice();
}

void ClusterDataBackend::canFramesReceived()
{
    while (m_canDevice && m_canDevice->framesAvailable() > 0) {
        QCanBusFrame frame(m_canDevice->readFrame());

        quint32 frameId(frame.frameId());

        switch (frameId)  {
        case PGN_65268 : handlePGN65268(frame); break; // Tire Condition - TIRE
        case PGN_64933 : handlePGN64933(frame); break; // Door Control 2 - DC2
        case PGN_64972 : handlePGN64972(frame); break; // Light Control
        case PGN_61445 : handlePGN61445(frame); break; // Electronic Transmission Controller 2 - ETC2
        case PGN_65265 : handlePGN65265(frame); break; // Cruise Control/Vehicle Speed - CCVS
        case PGN_65267 : handlePGN65267(frame); break; // vehicle position - VP
        case PGN_65256 : handlePGN65256(frame); break; // vehicle direction/speed
        case PGN_65262 : handlePGN65262(frame); break; // Engine Temperature 1 - ET1
        case PGN_65263 : handlePGN65263(frame); break; // Engine Fluid Level/Pressure 1 - EFL/P1
        case PGN_65271 : handlePGN65271(frame); break; // Vehicle Electrical Power - VEP
        case PGN_65276 : handlePGN65276(frame); break; // Dash Display - DD
        case PGN_61444 : handlePGN61444(frame); break; // Electronic Engine Controller 1 - EEC1
        case PGN_60928 : handlePGN60928(frame); break; // Address Claimed - ACL
        default: qWarning("canFramesReceived - unknown frame id: %d", frameId);
        }
#ifdef DEBUG_CAN
        QByteArray data(frame.payload());
        const char* constData(data.constData());
        int payLoadSize(data.size());
        QString dataString;
        for (int i(0); i < payLoadSize; i++) {
            if (!dataString.isEmpty()) dataString.append(".");
            dataString.append(QString::fromLatin1("%1").arg((int)constData[i], 2, 16, QChar('0')));
        }
        qDebug() << "ClusterDataBackend::canFramesReceived - id:" << frameId << "- data:" << dataString.toLatin1().constData();
#endif // DEBUG_CAN
    }
}

void ClusterDataBackend::canStateChanged(QCanBusDevice::CanBusDeviceState state)
{
#ifdef DEBUG_CAN
    qDebug() << "ClusterDataBackend::canStateChanged - state:" << state;
#else
    Q_UNUSED(state)
#endif // DEBUG_CAN
}
#endif
