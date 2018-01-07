#include "canclusterdatacontrol.h"

#include <QCanBus>
#include <QtGlobal>
#include <QDateTime>

#include <QDebug>

#define SOCKETCAN "socketcan"

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

// door defines
#define DOOR_INIT 0xF33CCFF1FCFFFF0F
#define OPEN_DOOR_1 0x400000000000000
#define OPEN_DOOR_2 0x1000000000000
#define OPEN_DOOR_3 0x40000000000000
#define OPEN_DOOR_4 0x100000000000
#define OPEN_DOOR_5 0x400000000
#define OPEN_DOOR_6 0x1000000

// light defines
#define LIGHT_INIT 0xF000FFFFFFFFFFFF
#define HEAD_LIGHT 0x200000000000000
#define PARK_LIGHT 0x100000000000000
#define LIGHT_ERROR 0x400000000000000
#define LEFT_TURN_SIGNAL 0x10000000000000
#define RIGHT_TURN_SIGNAL 0x20000000000000
#define HAZARD_SIGNAL 0x4000000000000

// Current gear
#define GEAR_PARK 0xFB
#define GEAR_REVERSE 0x7C
#define GEAR_NEUTRAL 0x7D
// #define GEAR_1 0x7E
// #define GEAR_2 0x7F
// #define GEAR_3 0x80
// #define GEAR_4 0x81
// #define GEAR_5 0x82
// #define GEAR_6 0x83

//#define _CAN_DEBUG

CanClusterDataControl::CanClusterDataControl(QObject* parent) :
    ClusterDataControlInterface(parent),
    m_canBus(QCanBus::instance()),
    m_device(Q_NULLPTR),
    m_doorStatus(DOOR_INIT),
    m_lightStatus(LIGHT_INIT),
    m_brake(false)
{
    Q_ASSERT(m_canBus);
    qsrand(QDateTime::currentMSecsSinceEpoch());
    QStringList plugins(m_canBus->plugins());
    if (plugins.contains(SOCKETCAN)) {
        m_device = m_canBus->createDevice(SOCKETCAN, "can0");
        m_device->setParent(this);
        connect(m_device, SIGNAL(errorOccurred(QCanBusDevice::CanBusError)), this, SLOT(canError(QCanBusDevice::CanBusError)));
        connect(m_device, SIGNAL(framesReceived()), this, SLOT(canFramesReceived()));
        connect(m_device, SIGNAL(stateChanged(QCanBusDevice::CanBusDeviceState)), this, SLOT(canStateChanged(QCanBusDevice::CanBusDeviceState)));
#ifdef _CAN_DEBUG
        connect(m_device, SIGNAL(framesWritten(qint64)), this, SLOT(canFramesWritten(qint64)));
        qDebug() << "CanClusterDataControl::CanClusterDataControl - connect device:" <<
#endif
        m_device->connectDevice();
    }
}

int CanClusterDataControl::carId() const
{
    return m_carId;
}

bool CanClusterDataControl::headLight() const
{
    return (m_lightStatus & HEAD_LIGHT);
}

bool CanClusterDataControl::parkLight() const
{
    return (m_lightStatus & PARK_LIGHT);
}

bool CanClusterDataControl::rightTurnLight() const
{
    return (m_lightStatus & RIGHT_TURN_SIGNAL);
}

bool CanClusterDataControl::leftTurnLight() const
{
    return (m_lightStatus & LEFT_TURN_SIGNAL);
}

int CanClusterDataControl::gear() const
{
    return m_gear;
}

bool CanClusterDataControl::lightFailure() const
{
    return (m_lightStatus & LIGHT_ERROR);
}

bool CanClusterDataControl::frontLeftDoorOpen() const
{
    return (m_doorStatus & OPEN_DOOR_1);
}

bool CanClusterDataControl::frontRightDoorOpen() const
{
    return (m_doorStatus & OPEN_DOOR_2);
}

bool CanClusterDataControl::rearLeftDoorOpen() const
{
    return (m_doorStatus & OPEN_DOOR_3);
}

bool CanClusterDataControl::rearRightDoorOpen() const
{
    return (m_doorStatus & OPEN_DOOR_4);
}

bool CanClusterDataControl::trunkOpen() const
{
    return (m_doorStatus & OPEN_DOOR_5);
}

bool CanClusterDataControl::hoodOpen() const
{
    return (m_doorStatus & OPEN_DOOR_6);
}

bool CanClusterDataControl::flatTire() const
{
    return m_flatTire;
}

double CanClusterDataControl::direction() const
{
    return m_direction;
}

double CanClusterDataControl::longitude() const
{
    return m_longitude;
}

double CanClusterDataControl::latitude() const
{
    return m_latitude;
}

double CanClusterDataControl::vehicleSpeed() const
{
    return m_vehicleSpeed;
}

bool CanClusterDataControl::hazardSignal() const
{
    return (m_lightStatus & HAZARD_SIGNAL);
}

bool CanClusterDataControl::brake() const
{
    return m_brake;
}

double CanClusterDataControl::oilTemp() const
{
    return m_oilTemp;
}

int CanClusterDataControl::oilPressure() const
{
    return m_oilPressure;
}

double CanClusterDataControl::batteryPotential() const
{
    return m_batteryPotential;
}

double CanClusterDataControl::gasLevel() const
{
    return m_gasLevel;
}

int CanClusterDataControl::rpm() const
{
    return m_rpm;
}

int CanClusterDataControl::engineTemp() const
{
    return m_engineTemp;
}

void CanClusterDataControl::setVehicleSpeed(double vehicleSpeed)
{
    if (m_vehicleSpeed == vehicleSpeed)
        return;

#ifdef _CAN_DEBUG
    qDebug("setVehicleSpeed: %f", vehicleSpeed);
#endif

    m_vehicleSpeed = vehicleSpeed;

    sendCCVS();

    emit vehicleSpeedChanged(vehicleSpeed);
}

void CanClusterDataControl::setLatitude(double latitude)
{
    if (m_latitude == latitude)
        return;

#ifdef _CAN_DEBUG
    qDebug("setLatitude: %f", latitude);
#endif

    m_latitude = latitude;
    emit latitudeChanged(latitude);
}

void CanClusterDataControl::setLongitude(double longitude)
{
    if (m_longitude == longitude)
        return;

#ifdef _CAN_DEBUG
    qDebug("setLongitude: %f", longitude);
#endif

    m_longitude = longitude;
    emit longitudeChanged(longitude);
}

void CanClusterDataControl::setDirection(double direction)
{
    if (m_direction == direction)
        return;

#ifdef _CAN_DEBUG
    qDebug("setDirection: %f", direction);
#endif

    quint16 convertedDirection(128.0 * direction); // 1/128 deg/bit
    QByteArray directionData;
    directionData.append((convertedDirection >> 8) & 0xff);
    directionData.append(convertedDirection & 0xff);
    char emptyData[6] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
    directionData.append(emptyData, 6);

    QCanBusFrame directionFrame(PGN_65256, directionData);

    if (!m_device->writeFrame(directionFrame)) {
        qWarning("CanClusterDataControl::setDirection - Error in pgn 65256 - vehicle direction/speed - VDS: %s", qPrintable(m_device->errorString()));
        return;
    }

    m_direction = direction;
    emit directionChanged(direction);
}

void CanClusterDataControl::setFlatTire(bool flatTire)
{
    if (m_flatTire == flatTire)
        return;

    m_flatTire = flatTire;

    char tireId(0x00); // front left
    // tireId = 0x01;  // front right
    // tireId = 0x10;  // back left
    // tireId = 0x11;  // back right

    // status 2 bits:
    // 00 Ok (no fault)
    // 01 Tire leak detected
    // 10 Error
    // 11 Not Supported

    // temperature, Resolution: 0.03125 deg C/bit, -273 deg C offset
    //    spn242 - Tire Temperature - Temperature at the surface of the tire sidewall.
    //    Data Length: 2 bytes
    //    Resolution: 0.03125 deg C/bit , -273 deg C offset
    //    Data Range: -273 to 1735 deg C
    quint16 temperature(0x1dc0 + (qrand() % 0x960)); // -35 to +40 C

    if (flatTire) {
        // pressure: 0 kPa
        // status: 01 Tire leak detected
        sendTireConditionData(tireId, 0x0, temperature, 0x01);
    }
    else {
        char pressure(0x31 + (qrand() % 0xf)); // 196 - 256 kPa
        // status: 00 Ok (no fault)
        sendTireConditionData(tireId, pressure, temperature, 0x00);
    }

    emit flatTireChanged(flatTire);
}

void CanClusterDataControl::setFrontLeftDoorOpen(bool doorOpen)
{
    if ((m_doorStatus & OPEN_DOOR_1) == doorOpen)
        return;

    sendDoorOpenStatus(OPEN_DOOR_1, doorOpen);

    emit frontLeftDoorOpenChanged(doorOpen);
}

void CanClusterDataControl::setFrontRightDoorOpen(bool doorOpen)
{
    if ((m_doorStatus & OPEN_DOOR_2) == doorOpen)
        return;

    sendDoorOpenStatus(OPEN_DOOR_2, doorOpen);

    emit frontRightDoorOpenChanged(doorOpen);
}

void CanClusterDataControl::setRearLeftDoorOpen(bool doorOpen)
{
    if ((m_doorStatus & OPEN_DOOR_3) == doorOpen)
        return;

    sendDoorOpenStatus(OPEN_DOOR_3, doorOpen);

    emit rearLeftDoorOpenChanged(doorOpen);
}

void CanClusterDataControl::setRearRightDoorOpen(bool doorOpen)
{
    if ((m_doorStatus & OPEN_DOOR_4) == doorOpen)
        return;

    sendDoorOpenStatus(OPEN_DOOR_4, doorOpen);

    emit rearRightDoorOpenChanged(doorOpen);
}

void CanClusterDataControl::setTrunkOpen(bool doorOpen)
{
    if ((m_doorStatus & OPEN_DOOR_5) == doorOpen)
        return;

    sendDoorOpenStatus(OPEN_DOOR_5, doorOpen);

    emit trunkOpenChanged(doorOpen);
}

void CanClusterDataControl::setHoodOpen(bool doorOpen)
{
    if ((m_doorStatus & OPEN_DOOR_6) == doorOpen)
        return;

    sendDoorOpenStatus(OPEN_DOOR_6, doorOpen);

    emit hoodOpenChanged(doorOpen);
}

void CanClusterDataControl::setLightFailure(bool lightFailure)
{
    if ((m_lightStatus & LIGHT_ERROR) == lightFailure)
        return;

#ifdef _CAN_DEBUG
    qDebug("setLightFailure - status: %s", (lightFailure ? "on" : "off"));
#endif

    sendLightStatus(LIGHT_ERROR, lightFailure);
    emit lightFailureChanged(lightFailure);
}

void CanClusterDataControl::setGear(int gear)
{
    if (m_gear == gear)
        return;

#ifdef _CAN_DEBUG
    qDebug("setGear: %d", gear);
#endif

    m_gear = gear;

    char driveGear;
    if (gear == 0xff ) {
        driveGear = GEAR_PARK;
    }
    else if (gear < 0) {
        driveGear = GEAR_REVERSE;
    }
    else {
        driveGear = (GEAR_NEUTRAL + gear);
    }

    QByteArray gearData;
    char emptyData[3] = { 0x0, 0x0, 0x0 };
    gearData.append(emptyData, 3);
    gearData.append(driveGear);
    char emptyData2[4] = { 0x0, 0x0, 0x0, 0x0 };
    gearData.append(emptyData2, 4);
    QCanBusFrame gearFrame(PGN_61445, gearData);
    if (!m_device->writeFrame(gearFrame)) {
        qWarning("CanClusterDataControl::setGear - Error in pgn 61445 - Electronic Transmission Controller 2 - ETC2 send: %s", qPrintable(m_device->errorString()));
    }

    emit gearChanged(gear);
}

void CanClusterDataControl::setLeftTurnLight(bool leftTurnLight)
{
    if ((m_lightStatus & HAZARD_SIGNAL) || (m_lightStatus & LEFT_TURN_SIGNAL) == leftTurnLight)
        return;

#ifdef _CAN_DEBUG
    qDebug("setLeftTurnLight - status: %s", (leftTurnLight ? "on" : "off"));
#endif

    sendLightStatus(LEFT_TURN_SIGNAL, leftTurnLight);
    emit leftTurnLightChanged(leftTurnLight);
}

void CanClusterDataControl::setRightTurnLight(bool rightTurnLight)
{
    if ((m_lightStatus & HAZARD_SIGNAL) || (m_lightStatus & RIGHT_TURN_SIGNAL) == rightTurnLight)
        return;

#ifdef _CAN_DEBUG
    qDebug("setRightTurnLight - status: %s", (rightTurnLight ? "on" : "off"));
#endif

    sendLightStatus(RIGHT_TURN_SIGNAL, rightTurnLight);
    emit rightTurnLightChanged(rightTurnLight);
}

void CanClusterDataControl::setHeadLight(bool headLight)
{
    if ((m_lightStatus & HEAD_LIGHT) == headLight)
        return;

#ifdef _CAN_DEBUG
    qDebug("setHeadLight - status: %s", (headLight ? "on" : "off"));
#endif

    sendLightStatus(HEAD_LIGHT, headLight);
    emit headLightChanged(headLight);
}

void CanClusterDataControl::setParkLight(bool parkLight)
{
    if ((m_lightStatus & PARK_LIGHT) == parkLight)
        return;

#ifdef _CAN_DEBUG
    qDebug("setParkLight - status: %s", (parkLight ? "on" : "off"));
#endif

    sendLightStatus(PARK_LIGHT, parkLight);
    emit parkLightChanged(parkLight);
}

void CanClusterDataControl::setCarId(int carId)
{
    if (m_carId == carId)
        return;

    m_carId = carId;

#ifdef _CAN_DEBUG
    qDebug("setCarId - carId: %d", carId);
#endif

    quint16 carIdData(carId); // only 21 bits allowed (0x0 - 0x1FFFFF), pos 1.1 - 3.5 in message

    QByteArray aclData;
    aclData.append((char)(carIdData >> 13 & 0xff));
    aclData.append((char)(carIdData >> 5 & 0xff));
    aclData.append((char)(carIdData << 3 & 0xf8));
    char emptyData[5] = { 0x0, 0x0, 0x0, 0x0, 0x0  };
    aclData.append(emptyData, 5);
    QCanBusFrame aclFrame(PGN_60928, aclData);
    if (!m_device->writeFrame(aclFrame)) {
        qWarning("CanClusterDataControl::setCarId - Error in pgn 60928 - Address Claimed - ACL send: %s", qPrintable(m_device->errorString()));
    }

    emit carIdChanged(carId);
}

void CanClusterDataControl::setHazardSignal(bool hazard)
{
    if ((m_lightStatus & HAZARD_SIGNAL) == hazard)
        return;

#ifdef _CAN_DEBUG
    qDebug("setHazardSignal - status: %s", (hazard ? "on" : "off"));
#endif

    sendLightStatus(HAZARD_SIGNAL, hazard);
    emit hazardSignalChanged(hazard);
}

void CanClusterDataControl::setBrake(bool brakeOn)
{
    if (m_brake == brakeOn) return;
    m_brake = brakeOn;

#ifdef _CAN_DEBUG
    qDebug("setBrake - status: %s", (brakeOn ? "on" : "off"));
#endif

    sendCCVS();
    emit brakeChanged(brakeOn);
}

void CanClusterDataControl::setOilTemp(double oilTemp)
{
    if (m_oilTemp == oilTemp)
        return;

#ifdef _CAN_DEBUG
    qDebug("setOilTemp - level: %f", oilTemp);
#endif

    m_oilTemp = oilTemp;
    sendET1();
    emit oilTempChanged(oilTemp);
}

void CanClusterDataControl::setOilPressure(int oilPressure)
{
    if (m_oilPressure == oilPressure)
        return;

#ifdef _CAN_DEBUG
    qDebug("setOilPressure - level: %u", oilPressure);
#endif

    m_oilPressure = oilPressure;

    QByteArray eflp1Data;
    char emptyData[3] = { 0x0, 0x0, 0x0 };
    eflp1Data.append(emptyData, 3);

    // spn100 - Engine Oil Pressure - Gage pressure of oil in engine lubrication system as provided by oil pump.
    // Data Length: 1 byte
    // Resolution: 4 kPa/bit , 0 offset
    // Data Range: 0 to 1000 kPa
    eflp1Data.append((char)(0.25 * (double)oilPressure));

    char emptyData2[4] = { 0x0, 0x0, 0x0, 0x0 };
    eflp1Data.append(emptyData2, 4);
    QCanBusFrame eflp1Frame(PGN_65263, eflp1Data);
    if (!m_device->writeFrame(eflp1Frame)) {
        qWarning("CanClusterDataControl::setOilPressure - Error in pgn 65263 - Engine Fluid Level/Pressure 1 - EFL/P1: %s", qPrintable(m_device->errorString()));
    }

    emit oilPressureChanged(oilPressure);
}


void CanClusterDataControl::setBatteryPotential(double batteryPotential)
{
    if (m_batteryPotential == batteryPotential)
        return;

#ifdef _CAN_DEBUG
    qDebug("setbatteryPotential - level: %f", batteryPotential);
#endif

    m_batteryPotential = batteryPotential;

    // spn158 - Battery Potential (Voltage), Switched
    // Data Length: 2 bytes
    // Resolution: 0.05 V/bit , 0 offset
    // Data Range: 0 to 3212.75 V
    quint16 convertedBatteryPotential(20.0 * batteryPotential);

    QByteArray vepData;
    //    1 1 byte Net Battery Current 114
    //    2 1 byte Alternator Current 115
    //    3-4 2 bytes Alternator Potential (Voltage) 167
    //    5-6 2 bytes Electrical Potential (Voltage) 168
    char emptyData[6] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
    vepData.append(emptyData, 6);

    vepData.append((convertedBatteryPotential >> 8 ) & 0xff);
    vepData.append(convertedBatteryPotential & 0xff);

    QCanBusFrame vepFrame(PGN_65271, vepData);
    if (!m_device->writeFrame(vepFrame)) {
        qWarning("CanClusterDataControl::setBatteryPotential - Error in pgn 65271 - Vehicle Electrical Power - VEP: %s", qPrintable(m_device->errorString()));
    }

    emit batteryPotentialChanged(batteryPotential);
}

void CanClusterDataControl::setGasLevel(double gasLevel)
{
    if (m_gasLevel == gasLevel)
        return;

#ifdef _CAN_DEBUG
    qDebug("setGasLevel - level: %f", gasLevel);
#endif

    m_gasLevel = gasLevel;

    QByteArray ddData;

    // 1 1 byte Washer Fluid Level 80
    ddData.append((char)0x00);

    // spn96 - Fuel Level - Ratio of volume of fuel to the total volume of fuel storage container.
    // Data Length: 1 byte
    // Resolution: 0.4 %/bit , 0 offset
    // Data Range: 0 to 100 %
    ddData.append((char)(2.5 * gasLevel));

    //    3 1 byte Fuel Filter Differential Pressure 95
    //    4 1 byte Engine Oil Filter Differential Pressure 99
    //    5-6 2 bytes Cargo Ambient Temperature 169
    //    7-8 2 bytes undefined
    char emptyData[6] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
    ddData.append(emptyData, 6);

    QCanBusFrame ddFrame(PGN_65276, ddData);
    if (!m_device->writeFrame(ddFrame)) {
        qWarning("CanClusterDataControl::setbatteryPotential - Error in pgn 65276 - Dash Display - DD: %s", qPrintable(m_device->errorString()));
    }

    emit gasLevelChanged(gasLevel);
}

void CanClusterDataControl::setRpm(int rpm)
{
    if (m_rpm == rpm)
        return;

#ifdef _CAN_DEBUG
    qDebug("setRpm - rpm: %d", rpm);
#endif

    m_rpm = rpm;

    QByteArray eec1Data;
//    1.1 4 bits Engine Torque Mode 899
//    2 1 byte Driver's Demand Engine - Percent Torque 512
//    3 1 byte Actual Engine - Percent Torque 513
    char emptyData[3] = { 0x0, 0x0, 0x0 };
    eec1Data.append(emptyData, 3);

    //    spn190 - Engine Speed - Actual engine speed which is calculated over a minimum crankshaft angle of 720 degrees divided by
    //    the number of cylinders.
    //    Data Length: 2 bytes
    //    Resolution: 0.125 rpm/bit , 0 offset
    //    Data Range: 0 to 8,031.875 rpm

    quint16 convertedRpm(8.0 * m_rpm);

    eec1Data.append((convertedRpm >> 8 ) & 0xff);
    eec1Data.append(convertedRpm & 0xff);

    //    6 1 byte Source Address of Controlling Device for Engine Control 1483
    //    7.1 4 bits Engine Starter Mode 1675
    //    8 1 byte Engine Demand - Percent Torque 2432
    eec1Data.append(emptyData, 3);

    QCanBusFrame eec1Frame(PGN_61444, eec1Data);
    if (!m_device->writeFrame(eec1Frame)) {
        qWarning("CanClusterDataControl::setbatteryPotential - Error in pgn 61444 - Electronic Engine Controller 1 - EEC1: %s", qPrintable(m_device->errorString()));
    }

    emit rpmChanged(rpm);
}

void CanClusterDataControl::setEngineTemp(int engineTemp)
{
    if (m_engineTemp == engineTemp)
        return;

#ifdef _CAN_DEBUG
    qDebug("setengineTemp - engineTemp: %d", engineTemp);
#endif

    m_engineTemp = engineTemp;
    sendET1();
    emit engineTempChanged(engineTemp);
}

void CanClusterDataControl::sendPosition(QTime timeStamp)
{
#ifdef _CAN_DEBUG
    qDebug("CanClusterDataControl::sendPosition - time stamp: %d:%d:%d.%d", timeStamp.hour(), timeStamp.minute(), timeStamp.second(), timeStamp.msec());
#endif

    QByteArray rapidPositionData;

    // 10^-7 deg/bit , -210 deg offset
    quint32 lat((m_latitude + 210.0) * 10000000.0);
    rapidPositionData.append((lat >> 24) & 0xff);
    rapidPositionData.append((lat >> 16) & 0xff);
    rapidPositionData.append((lat >> 8) & 0xff);
    rapidPositionData.append(lat & 0xff);

    quint32 lon((m_longitude + 210.0) * 10000000.0);
    rapidPositionData.append((lon >> 24) & 0xff);
    rapidPositionData.append((lon >> 16) & 0xff);
    rapidPositionData.append((lon >> 8) & 0xff);
    rapidPositionData.append(lon & 0xff);

    // PGN_65267 – Vehicle position - VP
    QCanBusFrame positionFrame(PGN_65267, rapidPositionData);
    // timestamp not needed
    // QCanBusFrame::TimeStamp ts(timeStamp.second(), timeStamp.msec() * 1000);
    // positionFrame.setTimeStamp(ts);
    if (!m_device->writeFrame(positionFrame)) {
        qWarning("CanClusterDataControl::sendPosition - Error in pgn 65267 - Vehicle position - VP: %s", qPrintable(m_device->errorString()));
    }
}

void CanClusterDataControl::canError(QCanBusDevice::CanBusError error)
{
#ifdef _CAN_DEBUG
    qDebug() << "CanClusterDataControl::canError - error:" << error << "- string:" << m_device->errorString();
#endif
    m_device->disconnectDevice();
}

void CanClusterDataControl::canFramesReceived()
{
    while (m_device->framesAvailable() > 0) {
#ifdef _CAN_DEBUG
        QCanBusFrame frame(m_device->readFrame());
        QByteArray data(frame.payload());
        qDebug() << "CanClusterDataControl::canFramesReceived - data:" << data;
#else
        m_device->readFrame();
#endif
    }
}

void CanClusterDataControl::canFramesWritten(qint64 framesCount)
{
#ifdef _CAN_DEBUG
    qDebug() << "CanClusterDataControl::canFramesWritten - framesCount:" << framesCount;
#else
    Q_UNUSED(framesCount)
#endif
}

void CanClusterDataControl::canStateChanged(QCanBusDevice::CanBusDeviceState state)
{
#ifdef _CAN_DEBUG
    qDebug() << "CanClusterDataControl::canStateChanged - state:" << state;
#else
    Q_UNUSED(state)
#endif
}

void CanClusterDataControl::sendTireConditionData(char tireId, char pressure, quint16 temperature, char status)
{
    QByteArray tireConditionData;
    tireConditionData.append(tireId); //  8 bits, SPN 929 Tire Location
    tireConditionData.append(pressure); //  8 bits, SPN 241 Tire Pressure

    tireConditionData.append((char)(temperature >> 8 & 0xff));
    tireConditionData.append((char)(temperature & 0xff));

    //    5.1 2 bits CTI Wheel Sensor Status 1699, 11 = not supported
    //    5.3 2 bits CTI Tire Status 1698, ok = 00, leak = 01
    //    5.5 2 bits CTI Wheel End Electrical Fault 1697, 11 = not supported

    tireConditionData.append(((status << 4) & 0x30) | 0xcc); // SPN 1698 CTI Tire Status
    char emptyData[3] = { 0x0, 0x0, 0xe };

    //    6-7 2 bytes Tire Air Leakage Rate 2586, 0
    //    8.6 3 bits Tire Pressure Threshold Detection 2587, 111 = Not available
    tireConditionData.append(emptyData, 3);
    QCanBusFrame tireConditionFrame(PGN_65268, tireConditionData);
    if (!m_device->writeFrame(tireConditionFrame)) {
        qWarning("CanClusterDataControl::sendTireConditionData - Error in pgn 65268 - Tire Condition - TIRE send: %s", qPrintable(m_device->errorString()));
    }
}

void CanClusterDataControl::sendDoorOpenStatus(quint64 door, bool open)
{
    if (open) {
        m_doorStatus |= door;
    }
    else {
        m_doorStatus &= ~door;
    }

    QByteArray doorOpenData;
    doorOpenData.append((char)(m_doorStatus >> 56 & 0xff));
    doorOpenData.append((char)(m_doorStatus >> 48 & 0xff));
    doorOpenData.append((char)(m_doorStatus >> 40 & 0xff));
    doorOpenData.append((char)(m_doorStatus >> 32 & 0xff));
    doorOpenData.append((char)(m_doorStatus >> 24 & 0xff));
    doorOpenData.append((char)(m_doorStatus >> 16 & 0xff));
    doorOpenData.append((char)(m_doorStatus >> 8 & 0xff));
    doorOpenData.append((char)(m_doorStatus & 0xff));

    QCanBusFrame doorFrame(PGN_64933, doorOpenData); // Door Control 2 - DC2
    if (!m_device->writeFrame(doorFrame)) {
        qWarning("CanClusterDataControl::sendDoorOpenStatus - Error in pgn 64933 - Door Control 2 send: %s", qPrintable(m_device->errorString()));
    }
}

void CanClusterDataControl::sendLightStatus(quint64 lightID, bool lightOn)
{
    if (lightOn) {
        m_lightStatus |= lightID;
    }
    else {
        m_lightStatus &= ~lightID;
    }

    QByteArray lightData;
    lightData.append((char)(m_lightStatus >> 56 & 0xff));
    lightData.append((char)(m_lightStatus >> 48 & 0xff));
    lightData.append((char)(m_lightStatus >> 40 & 0xff));
    lightData.append((char)(m_lightStatus >> 32 & 0xff));
    lightData.append((char)(m_lightStatus >> 24 & 0xff));
    lightData.append((char)(m_lightStatus >> 16 & 0xff));
    lightData.append((char)(m_lightStatus >> 8 & 0xff));
    lightData.append((char)(m_lightStatus & 0xff));

    QCanBusFrame lightFrame(PGN_64972, lightData);
    if (!m_device->writeFrame(lightFrame)) {
        qWarning("RouteSender::sendNextData - Error in pgn 64972 - Light control send: %s", qPrintable(m_device->errorString()));
    }
}

void CanClusterDataControl::sendCCVS()
{
    QByteArray ccvsData;
    ccvsData.append((char)0x0);

     //  16 bits, Wheel-Based Vehicle Speed (SPN 84), Resolution: 1/256 km/h per bit, Data Range: 0 to 250.996 km/h
    quint16 speed(256.0 * m_vehicleSpeed);
    ccvsData.append((speed >> 8) & 0xff);
    ccvsData.append(speed & 0xff);

    // 2 bits Brake Switch (SPN 597)
    // 00 = pedal released
    // 01 = pedal depressed
    if (m_brake) {
        ccvsData.append((char)0x10);
    }
    else {
        ccvsData.append((char)0x00);
    }
    char emptyData[4] = { 0x0, 0x0, 0x0, 0x0 };
    ccvsData.append(emptyData, 4);
    QCanBusFrame ccvsFrame(PGN_65265, ccvsData);
    if (!m_device->writeFrame(ccvsFrame)) {
        qWarning("CanClusterDataControl::sendCCVS - Error in pgn 65265 - Cruise Control/Vehicle Speed - CCVS: %s", qPrintable(m_device->errorString()));
    }
}

void CanClusterDataControl::sendET1()
{
    QByteArray et1Data;
    // SPN 110 - Engine Coolant Temperature
    // Data Length: 1 byte
    // Resolution: 1 deg C/bit , -40 deg C offset
    // Data Range: -40 to 210 deg C
    et1Data.append((quint8)(m_engineTemp + 40));
    et1Data.append((char)0x00); // SPN 174 Fuel Temperature
    // spn175 - Engine Oil Temperature 1 - Temperature of the engine lubricant.
    // Data Length: 2 bytes
    // Resolution: 0.03125 deg C/bit , -273 deg C offset
    // Data Range: -273 to 1735 deg C
    quint16 convertedEngOilTemp(32.0 * (m_oilTemp + 273.0));
    et1Data.append((convertedEngOilTemp >> 8) & 0xff);
    et1Data.append(convertedEngOilTemp & 0xff);
    // 5-6 2 bytes Turbo Oil Temperature 176
    // 7 1 byte Engine Intercooler Temperature 52
    // 8 1 byte Engine Intercooler Thermostat Opening 1134
    char emptyData[4] = { 0x0, 0x0, 0x0, 0x0 };
    et1Data.append(emptyData, 4);
    QCanBusFrame et1Frame(PGN_65262, et1Data);
    if (!m_device->writeFrame(et1Frame)) {
        qWarning("CanClusterDataControl::sendET1 - Error in pgn 65262 - Engine Temperature 1 - ET1: %s", qPrintable(m_device->errorString()));
    }
}
