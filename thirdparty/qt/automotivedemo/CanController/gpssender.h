#ifndef GPSSENDER_H
#define GPSSENDER_H

#include <QObject>
#include <QTimer>
#include <QTime>

class GpsSender : public QObject
{
    Q_OBJECT

public:
    explicit GpsSender(QString routeFileName, QObject* parent = Q_NULLPTR);
    virtual ~GpsSender();

private:
    void parseRouteFile(QString routeFileName);

signals:
    void latitudeChanged(qreal latitude);
    void longitudeChanged(qreal longitude);
    void directionChanged(qreal direction);
    void vehicleSpeedChanged(qreal vehicleSpeed);
#ifndef TCPCLUSTERDATACONNECTION
    void positionChanged(QTime timeStamp);
#endif

private slots:
    void nextData();

private: // data
    QStringList m_data;
    QTimer m_timer;
    int m_dataPosition;
    class NMEA* m_nmea;
    qreal m_latitude;
    qreal m_longitude;
    qreal m_direction;
    qreal m_vehicleSpeed;
    QTime m_timeStamp;

};

#endif // GPSSENDER_H
