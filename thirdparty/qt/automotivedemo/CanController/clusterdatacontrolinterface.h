#ifndef CLUSTERDATACONTROLINTERFACE_H
#define CLUSTERDATACONTROLINTERFACE_H

#include <QObject>

class ClusterDataControlInterface : public QObject
{
    Q_OBJECT
    Q_PROPERTY(double vehicleSpeed READ vehicleSpeed WRITE setVehicleSpeed NOTIFY vehicleSpeedChanged)
    Q_PROPERTY(double latitude READ latitude WRITE setLatitude NOTIFY latitudeChanged)
    Q_PROPERTY(double longitude READ longitude WRITE setLongitude NOTIFY longitudeChanged)
    Q_PROPERTY(double direction READ direction WRITE setDirection NOTIFY directionChanged)
    Q_PROPERTY(bool flatTire READ flatTire WRITE setFlatTire NOTIFY flatTireChanged)
    Q_PROPERTY(bool frontLeftDoorOpen READ frontLeftDoorOpen WRITE setFrontLeftDoorOpen NOTIFY frontLeftDoorOpenChanged)
    Q_PROPERTY(bool frontRightDoorOpen READ frontRightDoorOpen WRITE setFrontRightDoorOpen NOTIFY frontRightDoorOpenChanged)
    Q_PROPERTY(bool rearLeftDoorOpen READ rearLeftDoorOpen WRITE setRearLeftDoorOpen NOTIFY rearLeftDoorOpenChanged)
    Q_PROPERTY(bool rearRightDoorOpen READ rearRightDoorOpen WRITE setRearRightDoorOpen NOTIFY rearRightDoorOpenChanged)
    Q_PROPERTY(bool lightFailure READ lightFailure WRITE setLightFailure NOTIFY lightFailureChanged)
    Q_PROPERTY(bool trunkOpen READ trunkOpen WRITE setTrunkOpen NOTIFY trunkOpenChanged)
    Q_PROPERTY(bool hoodOpen READ hoodOpen WRITE setHoodOpen NOTIFY hoodOpenChanged)
    Q_PROPERTY(int gear READ gear WRITE setGear NOTIFY gearChanged)
    Q_PROPERTY(bool leftTurnLight READ leftTurnLight WRITE setLeftTurnLight NOTIFY leftTurnLightChanged)
    Q_PROPERTY(bool rightTurnLight READ rightTurnLight WRITE setRightTurnLight NOTIFY rightTurnLightChanged)
    Q_PROPERTY(bool headLight READ headLight WRITE setHeadLight NOTIFY headLightChanged)
    Q_PROPERTY(bool parkLight READ parkLight WRITE setParkLight NOTIFY parkLightChanged)
    Q_PROPERTY(int carId READ carId WRITE setCarId NOTIFY carIdChanged)
    Q_PROPERTY(bool hazardSignal READ hazardSignal WRITE setHazardSignal NOTIFY hazardSignalChanged)
    Q_PROPERTY(bool brake READ brake WRITE setBrake NOTIFY brakeChanged)
    Q_PROPERTY(double oilTemp READ oilTemp WRITE setOilTemp NOTIFY oilTempChanged)
    Q_PROPERTY(int oilPressure READ oilPressure WRITE setOilPressure NOTIFY oilPressureChanged)
    Q_PROPERTY(double batteryPotential READ batteryPotential WRITE setBatteryPotential NOTIFY batteryPotentialChanged)
    Q_PROPERTY(double gasLevel READ gasLevel WRITE setGasLevel NOTIFY gasLevelChanged)
    Q_PROPERTY(int rpm READ rpm WRITE setRpm NOTIFY rpmChanged)
    Q_PROPERTY(int engineTemp READ engineTemp WRITE setEngineTemp NOTIFY engineTempChanged)
public:
    ClusterDataControlInterface(QObject *parent) :
        QObject(parent),
        m_carId(2),
        m_gear(1),
        m_lightFailure(false),
        m_flatTire(false),
        m_direction(0.0),
        m_longitude(0.0),
        m_latitude(0.0),
        m_vehicleSpeed(0.0),
        m_oilTemp(40.0),
        m_oilPressure(0),
        m_batteryPotential(80.0),
        m_gasLevel(67.0),
        m_rpm(4000),
        m_engineTemp(60)
    {}

    virtual int carId() const = 0;
    virtual bool headLight() const = 0;
    virtual bool parkLight() const = 0;
    virtual bool rightTurnLight() const = 0;
    virtual bool leftTurnLight() const = 0;
    virtual int gear() const = 0;
    virtual bool lightFailure() const = 0;
    virtual bool frontLeftDoorOpen() const = 0;
    virtual bool frontRightDoorOpen() const = 0;
    virtual bool rearLeftDoorOpen() const = 0;
    virtual bool rearRightDoorOpen() const = 0;
    virtual bool hoodOpen() const = 0;
    virtual bool trunkOpen() const = 0;
    virtual bool flatTire() const = 0;
    virtual double direction() const = 0;
    virtual double longitude() const = 0;
    virtual double latitude() const = 0;
    virtual double vehicleSpeed() const = 0;
    virtual bool hazardSignal() const = 0;
    virtual bool brake() const = 0;
    virtual double oilTemp() const = 0;
    virtual int oilPressure() const = 0;
    virtual double batteryPotential() const = 0;
    virtual double gasLevel() const = 0;
    virtual int rpm() const = 0;
    virtual int engineTemp() const = 0;

public slots:
    virtual void setVehicleSpeed(double vehicleSpeed) = 0;
    virtual void setLatitude(double latitude) = 0;
    virtual void setLongitude(double longitude) = 0;
    virtual void setDirection(double direction) = 0;
    virtual void setFlatTire(bool flatTire) = 0;
    virtual void setFrontLeftDoorOpen(bool doorOpen) = 0;
    virtual void setFrontRightDoorOpen(bool doorOpen) = 0;
    virtual void setRearLeftDoorOpen(bool doorOpen) = 0;
    virtual void setRearRightDoorOpen(bool doorOpen) = 0;
    virtual void setHoodOpen(bool doorOpen) = 0;
    virtual void setTrunkOpen(bool doorOpen) = 0;
    virtual void setLightFailure(bool lightFailure) = 0;
    virtual void setGear(int gear) = 0;
    virtual void setLeftTurnLight(bool leftTurnLight) = 0;
    virtual void setRightTurnLight(bool rightTurnLight) = 0;
    virtual void setHeadLight(bool headLight) = 0;
    virtual void setParkLight(bool parkLight) = 0;
    virtual void setCarId(int carId) = 0;
    virtual void setHazardSignal(bool) = 0;
    virtual void setBrake(bool) = 0;
    virtual void setOilTemp(double oilTemp) = 0;
    virtual void setOilPressure(int oilPressure) = 0;
    virtual void setBatteryPotential(double batteryPotential) = 0;
    virtual void setGasLevel(double gasLevel) = 0;
    virtual void setRpm(int rpm) = 0;
    virtual void setEngineTemp(int engineTemp) = 0;

signals:
    void dataReset();
    void carIdChanged(int carId);
    void headLightChanged(bool headLight);
    void parkLightChanged(bool parkLight);
    void rightTurnLightChanged(bool rightTurnLight);
    void leftTurnLightChanged(bool leftTurnLight);
    void gearChanged(bool gear);
    void lightFailureChanged(bool lightFailure);
    void frontLeftDoorOpenChanged(bool doorOpen);
    void frontRightDoorOpenChanged(bool doorOpen);
    void rearLeftDoorOpenChanged(bool doorOpen);
    void rearRightDoorOpenChanged(bool doorOpen);
    void hoodOpenChanged(bool doorOpen);
    void trunkOpenChanged(bool doorOpen);
    void flatTireChanged(bool flatTire);
    void directionChanged(double direction);
    void longitudeChanged(double longitude);
    void latitudeChanged(double latitude);
    void vehicleSpeedChanged(double vehicleSpeed);
    void hazardSignalChanged(bool hazard);
    void brakeChanged(bool brakeOn);
    void oilTempChanged(double oilTemp);
    void oilPressureChanged(int oilPressure);
    void batteryPotentialChanged(double batteryPotential);
    void gasLevelChanged(double gasLevel);
    void rpmChanged(int rpm);
    void engineTempChanged(int engineTemp);

protected:
    int m_carId;
    int m_gear;
    bool m_lightFailure;
    bool m_flatTire;
    double m_direction;
    double m_longitude;
    double m_latitude;
    double m_vehicleSpeed;
    double m_oilTemp;
    int m_oilPressure;
    double m_batteryPotential;
    double m_gasLevel;
    int m_rpm;
    int m_engineTemp;

};

#endif // CLUSTERDATACONTROLINTERFACE_H
