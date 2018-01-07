#ifndef TCPCLUSTERDATACONTROL_H
#define TCPCLUSTERDATACONTROL_H

#include "clusterdatacontrolinterface.h"

#include <QTcpSocket>

class TCPClusterDataControl : public ClusterDataControlInterface
{
    Q_OBJECT

public:
    TCPClusterDataControl(QObject *parent = 0);
    ~TCPClusterDataControl();

    int carId() const;
    bool headLight() const;
    bool parkLight() const;
    bool rightTurnLight() const;
    bool leftTurnLight() const;
    int gear() const;
    bool lightFailure() const;
    bool frontLeftDoorOpen() const;
    bool frontRightDoorOpen() const;
    bool rearLeftDoorOpen() const;
    bool rearRightDoorOpen() const;
    bool hoodOpen() const;
    bool trunkOpen() const;
    bool flatTire() const;
    double direction() const;
    double longitude() const;
    double latitude() const;
    double vehicleSpeed() const;
    bool hazardSignal() const;
    bool brake() const;
    double oilTemp() const;
    int oilPressure() const;
    double batteryPotential() const;
    double gasLevel() const;
    int rpm() const;
    int engineTemp() const;

public slots:
    void connectToServer();
    void onConnectionStateChanged(QAbstractSocket::SocketState socketState);
    void handleTcpError(QAbstractSocket::SocketError socketError);

    void setVehicleSpeed(double vehicleSpeed);
    void setLatitude(double latitude);
    void setLongitude(double longitude);
    void setDirection(double direction);
    void setFlatTire(bool flatTire);
    void setFrontLeftDoorOpen(bool doorOpen);
    void setFrontRightDoorOpen(bool doorOpen);
    void setRearLeftDoorOpen(bool doorOpen);
    void setRearRightDoorOpen(bool doorOpen);
    void setHoodOpen(bool doorOpen);
    void setTrunkOpen(bool doorOpen);
    void setLightFailure(bool lightFailure);
    void setGear(int gear);
    void setLeftTurnLight(bool leftTurnLight);
    void setRightTurnLight(bool rightTurnLight);
    void setHeadLight(bool headLight);
    void setParkLight(bool parkLight);
    void setCarId(int carId);
    void setHazardSignal(bool hazardSignal);
    void setBrake(bool brake);
    void setOilTemp(double oilTemp);
    void setOilPressure(int oilPressure);
    void setBatteryPotential(double batteryPotential);
    void setGasLevel(double gasLevel);
    void setRpm(int rpm);
    void setEngineTemp(int engineTemp);

private:
    void write(const char *, qint64 );
    void write(const char *);

private:
    QTcpSocket m_tcpSocket;
    bool m_frontLeftDoorOpen;
    bool m_frontRightDoorOpen;
    bool m_rearLeftDoorOpen;
    bool m_rearRightDoorOpen;
    bool m_hoodOpen;
    bool m_trunkOpen;
    bool m_headLight;
    bool m_parkLight;
    bool m_rightTurnLight;
    bool m_leftTurnLight;
    bool m_hazardSignal;
    bool m_brake;
};

#endif // TCPCLUSTERDATACONTROL_H
