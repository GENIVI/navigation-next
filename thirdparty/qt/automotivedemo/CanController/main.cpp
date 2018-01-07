#include <QGuiApplication>
#include <QQuickView>
#include <QObject>
#include <QQmlEngine>
#include <QQmlContext>

#include "gpssender.h"

#ifdef TCPCLUSTERDATACONNECTION
#include "tcpclusterdatacontrol.h"
#else
#include <QTime>
#include "canclusterdatacontrol.h"
#endif

int main(int argc, char** argv)
{
    QGuiApplication app(argc, argv);

    ClusterDataControlInterface* dataInterface(Q_NULLPTR);

#ifdef TCPCLUSTERDATACONNECTION
    dataInterface = new TCPClusterDataControl;
#else
    dataInterface = new CanClusterDataControl;
#endif

    Q_ASSERT(dataInterface);

    QString routeFileName(DEFAULT_ROUTE_FILE);
    QStringList args(app.arguments());
    if (args.count() > 1) {
        routeFileName = args.at(1);
    }

    GpsSender gpsSender(routeFileName);

    QObject::connect(&gpsSender, SIGNAL(latitudeChanged(qreal)), dataInterface, SLOT(setLatitude(double)));
    QObject::connect(&gpsSender, SIGNAL(longitudeChanged(qreal)), dataInterface, SLOT(setLongitude(double)));
    QObject::connect(&gpsSender, SIGNAL(directionChanged(qreal)), dataInterface, SLOT(setDirection(double)));
    QObject::connect(&gpsSender, SIGNAL(vehicleSpeedChanged(qreal)), dataInterface, SLOT(setVehicleSpeed(double)));

#ifndef TCPCLUSTERDATACONNECTION
    QObject::connect(&gpsSender, SIGNAL(positionChanged(QTime)), qobject_cast<CanClusterDataControl*>(dataInterface), SLOT(sendPosition(QTime)));
#endif

    QQuickView view;

    view.rootContext()->setContextProperty("clusterDataControl", dataInterface);

    QObject::connect(view.engine(), SIGNAL(quit()), &app, SLOT(quit()));

    view.setSource(QUrl(MAIN_QML_FILE_NAME));

    view.show();

    return app.exec();
}
