#ifndef CANCLUSTERDATACONTROL_H
#define CANCLUSTERDATACONTROL_H

#include "clusterdatacontrolinterface.h"
#include <QTime>
#include <QCanBusDevice>

class CanClusterDataControl : public ClusterDataControlInterface
{
    Q_OBJECT

public:
    CanClusterDataControl(QObject* parent = Q_NULLPTR);

public: // from class ClusterDataControlInterface
    virtual int carId() const;
    virtual bool headLight() const;
    virtual bool parkLight() const;
    virtual bool rightTurnLight() const;
    virtual bool leftTurnLight() const;
    virtual int gear() const;
    virtual bool lightFailure() const;
    virtual bool frontLeftDoorOpen() const;
    virtual bool frontRightDoorOpen() const;
    virtual bool rearLeftDoorOpen() const;
    virtual bool rearRightDoorOpen() const;
    virtual bool hoodOpen() const;
    virtual bool trunkOpen() const;
    virtual bool flatTire() const;
    virtual double direction() const;
    virtual double longitude() const;
    virtual double latitude() const;
    virtual double vehicleSpeed() const;
    virtual bool hazardSignal() const;
    virtual bool brake() const;
    virtual double oilTemp() const;
    virtual int oilPressure() const;
    virtual double batteryPotential() const;
    virtual double gasLevel() const;
    virtual int rpm() const;
    virtual int engineTemp() const;

public slots: // from class ClusterDataControlInterface
    virtual void setVehicleSpeed(double vehicleSpeed);
    virtual void setLatitude(double latitude);
    virtual void setLongitude(double longitude);
    virtual void setDirection(double direction);
    virtual void setFlatTire(bool flatTire);
    virtual void setFrontLeftDoorOpen(bool doorOpen);
    virtual void setFrontRightDoorOpen(bool doorOpen);
    virtual void setRearLeftDoorOpen(bool doorOpen);
    virtual void setRearRightDoorOpen(bool doorOpen);
    virtual void setHoodOpen(bool doorOpen);
    virtual void setTrunkOpen(bool doorOpen);
    virtual void setLightFailure(bool lightFailure);
    virtual void setGear(int gear);
    virtual void setLeftTurnLight(bool leftTurnLight);
    virtual void setRightTurnLight(bool rightTurnLight);
    virtual void setHeadLight(bool headLight);
    virtual void setParkLight(bool parkLight);
    virtual void setCarId(int carId);
    virtual void setHazardSignal(bool hazard);
    virtual void setBrake(bool brakeOn);
    virtual void setOilTemp(double oilTemp);
    virtual void setOilPressure(int oilPressure);
    virtual void setBatteryPotential (double batteryPotential);
    virtual void setGasLevel(double gasLevel);
    virtual void setRpm(int rpm);
    virtual void setEngineTemp(int engineTemp);

public slots: // new local slots
    void sendPosition(QTime timeStamp);

private slots:
    void canError(QCanBusDevice::CanBusError error);
    void canFramesReceived();
    void canFramesWritten(qint64 framesCount);
    void canStateChanged(QCanBusDevice::CanBusDeviceState state);

private:
    void sendTireConditionData(char tireId, char pressure, quint16 temperature, char status);
    void sendDoorOpenStatus(quint64 door, bool open);
    void sendLightStatus(quint64 lightID, bool lightOn);
    void sendCCVS();
    void sendET1();

private: // data
    class QCanBus* m_canBus;
    class QCanBusDevice* m_device;
    quint64 m_doorStatus;
    quint64 m_lightStatus;
    bool m_brake;
};

#endif // CANCLUSTERDATACONTROL_H
