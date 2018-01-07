#include "tcpclusterdatacontrol.h"
#include <QHostAddress>
#include <QDebug>
#include <QTimer>

TCPClusterDataControl::TCPClusterDataControl(QObject *parent)
    : ClusterDataControlInterface(parent),
      m_frontLeftDoorOpen(false),
      m_frontRightDoorOpen(false),
      m_rearLeftDoorOpen(false),
      m_rearRightDoorOpen(false),
      m_hoodOpen(false),
      m_trunkOpen(false),
      m_headLight(false),
      m_parkLight(false),
      m_rightTurnLight(false),
      m_leftTurnLight(false),
      m_hazardSignal(false),
      m_brake(false)
{
    connect(&m_tcpSocket, &QAbstractSocket::stateChanged,
            this, &TCPClusterDataControl::onConnectionStateChanged);
    connect(&m_tcpSocket, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)> (&QAbstractSocket::error),
            this, &TCPClusterDataControl::handleTcpError);

    connectToServer();
}

TCPClusterDataControl::~TCPClusterDataControl()
{
    m_tcpSocket.close();
}

static bool printIP = true;

void TCPClusterDataControl::connectToServer()
{
    QByteArray simu = qgetenv("QT_CLUSTER_SIMU");
    QHostAddress simuAddr("127.0.0.1");

    if (!simu.isEmpty()) {
        if (!simuAddr.setAddress(QString(simu))) {
            qWarning("Failed to set simulator address! Falling back to localhost");
            simuAddr.setAddress("127.0.0.1");
        }
    }

    if(printIP) {
        printIP = false;
        qDebug() << "Connecting to" << simuAddr;
    }

    m_tcpSocket.connectToHost(simuAddr,32123);
}

void TCPClusterDataControl::onConnectionStateChanged(QAbstractSocket::SocketState socketState)
{
    if (socketState == QAbstractSocket::UnconnectedState) {
        QTimer::singleShot(1000,Qt::VeryCoarseTimer, this, SLOT(connectToServer()));
    } else if (socketState == QAbstractSocket::ConnectedState) {
        qDebug() << "Connected";
        emit dataReset();
    }
}

void TCPClusterDataControl::handleTcpError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    qWarning() << "TCP Error:" << m_tcpSocket.errorString();
}

int TCPClusterDataControl::carId() const
{
    return m_carId;
}

bool TCPClusterDataControl::headLight() const
{
    return m_headLight;
}

bool TCPClusterDataControl::parkLight() const
{
    return m_parkLight;
}

bool TCPClusterDataControl::rightTurnLight() const
{
    return m_rightTurnLight;
}

bool TCPClusterDataControl::leftTurnLight() const
{
    return m_leftTurnLight;
}

int TCPClusterDataControl::gear() const
{
    return m_gear;
}

bool TCPClusterDataControl::lightFailure() const
{
    return m_lightFailure;
}

bool TCPClusterDataControl::frontLeftDoorOpen() const
{
    return m_frontLeftDoorOpen;
}

bool TCPClusterDataControl::frontRightDoorOpen() const
{
    return m_frontRightDoorOpen;
}

bool TCPClusterDataControl::rearLeftDoorOpen() const
{
    return m_rearLeftDoorOpen;
}

bool TCPClusterDataControl::rearRightDoorOpen() const
{
    return m_rearRightDoorOpen;
}

bool TCPClusterDataControl::hoodOpen() const
{
    return m_hoodOpen;
}

bool TCPClusterDataControl::trunkOpen() const
{
    return m_trunkOpen;
}

bool TCPClusterDataControl::flatTire() const
{
    return m_flatTire;
}

double TCPClusterDataControl::direction() const
{
    return m_direction;
}

double TCPClusterDataControl::longitude() const
{
    return m_longitude;
}

double TCPClusterDataControl::latitude() const
{
    return m_latitude;
}

double TCPClusterDataControl::vehicleSpeed() const
{
    return m_vehicleSpeed;
}

bool TCPClusterDataControl::hazardSignal() const
{
    return m_hazardSignal;
}

bool TCPClusterDataControl::brake() const
{
    return m_brake;
}

double TCPClusterDataControl::oilTemp() const
{
    return m_oilTemp;
}

int TCPClusterDataControl::oilPressure() const
{
    return m_oilPressure;
}

double TCPClusterDataControl::batteryPotential() const
{
    return m_batteryPotential;
}

double TCPClusterDataControl::gasLevel() const
{
    return m_gasLevel;
}

int TCPClusterDataControl::rpm() const
{
    return m_rpm;
}

int TCPClusterDataControl::engineTemp() const
{
    return m_engineTemp;
}

void TCPClusterDataControl::setVehicleSpeed(double vehicleSpeed)
{
    if (m_vehicleSpeed == vehicleSpeed)
        return;

    QString msg("vehicleSpeed:" + QString::number(vehicleSpeed) + " ");
    write(msg.toLatin1());

    m_vehicleSpeed = vehicleSpeed;
    emit vehicleSpeedChanged(vehicleSpeed);
}

void TCPClusterDataControl::setLatitude(double latitude)
{
    if (m_latitude == latitude)
        return;

    QString msg("latitude:" + QString::number(latitude) + " ");
    write(msg.toLatin1(), msg.length());

    m_latitude = latitude;
    emit latitudeChanged(latitude);
}

void TCPClusterDataControl::setLongitude(double longitude)
{
    if (m_longitude == longitude)
        return;

    QString msg("longitude:" + QString::number(longitude) + " ");
    write(msg.toLatin1());

    m_longitude = longitude;
    emit longitudeChanged(longitude);
}

void TCPClusterDataControl::setDirection(double direction)
{
    if (m_direction == direction)
        return;

    QString msg("direction:" + QString::number(direction) + " ");
    write(msg.toLatin1());

    m_direction = direction;
    emit directionChanged(direction);
}

void TCPClusterDataControl::setFlatTire(bool flatTire)
{
    if (m_flatTire == flatTire)
        return;

    QString msg("flatTire:" + QString::number(flatTire) + " ");
    write(msg.toLatin1());

    m_flatTire = flatTire;
    emit flatTireChanged(flatTire);
}

void TCPClusterDataControl::setFrontLeftDoorOpen(bool doorOpen)
{
    if (m_frontLeftDoorOpen == doorOpen)
        return;

    QString msg("frontLeftDoorOpen:" + QString::number(doorOpen) + " ");
    write(msg.toLatin1());

    m_frontLeftDoorOpen = doorOpen;
    emit frontLeftDoorOpenChanged(doorOpen);
}

void TCPClusterDataControl::setFrontRightDoorOpen(bool doorOpen)
{
    if (m_frontRightDoorOpen == doorOpen)
        return;

    QString msg("frontRightDoorOpen:" + QString::number(doorOpen) + " ");
    write(msg.toLatin1());

    m_frontRightDoorOpen = doorOpen;
    emit frontRightDoorOpenChanged(doorOpen);
}

void TCPClusterDataControl::setRearLeftDoorOpen(bool doorOpen)
{
    if (m_rearLeftDoorOpen == doorOpen)
        return;

    QString msg("rearLeftDoorOpen:" + QString::number(doorOpen) + " ");
    write(msg.toLatin1());

    m_rearLeftDoorOpen = doorOpen;
    emit rearLeftDoorOpenChanged(doorOpen);
}

void TCPClusterDataControl::setHoodOpen(bool doorOpen)
{
    if (m_hoodOpen == doorOpen)
        return;

    QString msg("hoodOpen:" + QString::number(doorOpen) + " ");
    write(msg.toLatin1());

    m_hoodOpen = doorOpen;
    emit hoodOpenChanged(doorOpen);
}

void TCPClusterDataControl::setTrunkOpen(bool doorOpen)
{
    if (m_trunkOpen == doorOpen)
        return;

    QString msg("trunkOpen:" + QString::number(doorOpen) + " ");
    write(msg.toLatin1());

    m_trunkOpen = doorOpen;
    emit rearRightDoorOpenChanged(doorOpen);
}

void TCPClusterDataControl::setRearRightDoorOpen(bool doorOpen)
{
    if (m_rearRightDoorOpen == doorOpen)
        return;

    QString msg("rearRightDoorOpen:" + QString::number(doorOpen) + " ");
    write(msg.toLatin1());

    m_rearRightDoorOpen = doorOpen;
    emit rearRightDoorOpenChanged(doorOpen);
}

void TCPClusterDataControl::setLightFailure(bool lightFailure)
{
    if (m_lightFailure == lightFailure)
        return;

    QString msg("lightFailure:" + QString::number(lightFailure) + " ");
    write(msg.toLatin1());

    m_lightFailure = lightFailure;
    emit lightFailureChanged(lightFailure);
}

void TCPClusterDataControl::setGear(int gear)
{
    if (m_gear == gear)
        return;

    QString msg("gear:" + QString::number(gear) + " ");
    write(msg.toLatin1());

    m_gear = gear;
    emit gearChanged(gear);
}

void TCPClusterDataControl::setLeftTurnLight(bool leftTurnLight)
{
    if (m_leftTurnLight == leftTurnLight)
        return;

    QString msg("leftTurnLight:" + QString::number(leftTurnLight) + " ");
    write(msg.toLatin1());

    m_leftTurnLight = leftTurnLight;
    emit leftTurnLightChanged(leftTurnLight);
}

void TCPClusterDataControl::setRightTurnLight(bool rightTurnLight)
{
    if (m_rightTurnLight == rightTurnLight)
        return;

    QString msg("rightTurnLight:" + QString::number(rightTurnLight) + " ");
    write(msg.toLatin1());

    m_rightTurnLight = rightTurnLight;
    emit rightTurnLightChanged(rightTurnLight);
}

void TCPClusterDataControl::setHeadLight(bool headLight)
{
    if (m_headLight == headLight)
        return;

    QString msg("headLight:" + QString::number(headLight) + " ");
    write(msg.toLatin1());

    m_headLight = headLight;
    emit headLightChanged(headLight);
}

void TCPClusterDataControl::setParkLight(bool parkLight)
{
    if (m_parkLight == parkLight)
        return;

    QString msg("parkLight:" + QString::number(parkLight) + " ");
    write(msg.toLatin1());

    m_parkLight = parkLight;
    emit parkLightChanged(parkLight);
}

void TCPClusterDataControl::setCarId(int carId)
{
    if (m_carId == carId)
        return;

    QString msg("carId:" + QString::number(carId) + " ");
    write(msg.toLatin1());

    m_carId = carId;
    emit carIdChanged(carId);
}

void TCPClusterDataControl::setHazardSignal(bool hazardSignal)
{
    if (m_hazardSignal == hazardSignal)
        return;

    QString msg("hazardSignal:" + QString::number(hazardSignal) + " ");
    write(msg.toLatin1());

    m_hazardSignal = hazardSignal;
    emit hazardSignalChanged(hazardSignal);
}

void TCPClusterDataControl::setBrake(bool brake)
{
    if (m_brake == brake)
        return;

    QString msg("brake:" + QString::number(brake) + " ");
    write(msg.toLatin1());

    m_brake = brake;
    emit brakeChanged(brake);
}

void TCPClusterDataControl::setOilTemp(double oilTemp)
{
    if (m_oilTemp == oilTemp)
        return;

    QString msg("oilTemp:" + QString::number(oilTemp) + " ");
    write(msg.toLatin1());

    m_oilTemp = oilTemp;
    emit oilTempChanged(oilTemp);
}

void TCPClusterDataControl::setOilPressure(int oilPressure)
{
    if (m_oilPressure == oilPressure)
        return;

    QString msg("oilPressure:" + QString::number(oilPressure) + " ");
    write(msg.toLatin1());

    m_oilPressure = oilPressure;
    emit oilPressureChanged(oilPressure);
}

void TCPClusterDataControl::setBatteryPotential(double batteryPotential)
{
    if (m_batteryPotential == batteryPotential)
        return;

    QString msg("batteryPotential:" + QString::number(batteryPotential) + " ");
    write(msg.toLatin1());

    m_batteryPotential = batteryPotential;
    emit batteryPotentialChanged(batteryPotential);
}

void TCPClusterDataControl::setGasLevel(double gasLevel)
{
    if (m_gasLevel == gasLevel)
        return;

    QString msg("gasLevel:" + QString::number(gasLevel) + " ");
    write(msg.toLatin1());

    m_gasLevel = gasLevel;
    emit gasLevelChanged(gasLevel);
}

void TCPClusterDataControl::setRpm(int rpm)
{
    if (m_rpm == rpm)
        return;

    QString msg("rpm:" + QString::number(rpm) + " ");
    write(msg.toLatin1());

    m_rpm = rpm;
    emit rpmChanged(rpm);
}

void TCPClusterDataControl::setEngineTemp(int engineTemp)
{
    if (m_engineTemp == engineTemp)
        return;

    QString msg("engineTemp:" + QString::number(engineTemp) + " ");
    write(msg.toLatin1());

    m_engineTemp = engineTemp;
    emit engineTempChanged(engineTemp);
}

void TCPClusterDataControl::write(const char *data, qint64 length)
{
    if ((m_tcpSocket.state() == QTcpSocket::ConnectedState)) {
        m_tcpSocket.write(data, length);
    }
}

void TCPClusterDataControl::write(const char *data)
{
    if ((m_tcpSocket.state() == QTcpSocket::ConnectedState)) {
        m_tcpSocket.write(data);
    }
}
