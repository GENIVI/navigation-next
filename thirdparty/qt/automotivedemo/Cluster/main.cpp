/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

//#ifdef QT_3DCORE_LIB
//#include "scenehelper.h"
//#endif
//#include "gauge.h"
//#include "qtiviclusterdata.h"

#include <QtQml/QQmlApplicationEngine>
#include <QtGui/QFont>
#include <QtGui/QFontDatabase>
#include <QtGui/QGuiApplication>
#include <QtQuick/QQuickView>

//#include "etcprovider.h"

#ifdef STATIC
#include <QtPlugin>
#include <QQmlExtensionPlugin>

Q_IMPORT_PLUGIN(QtQuick2Plugin)
Q_IMPORT_PLUGIN(QtQuickScene3DPlugin)
Q_IMPORT_PLUGIN(Qt3DQuick3DCorePlugin)
Q_IMPORT_PLUGIN(Qt3DQuick3DRenderPlugin)
Q_IMPORT_PLUGIN(QtIVIVehicleFunctionsPlugin)
//Q_IMPORT_PLUGIN(QMultimediaDeclarativeModule)
//Q_IMPORT_PLUGIN(QtIVIVehicleFunctionsPlugin)
//Q_IMPORT_PLUGIN(QtIVICorePlugin)
//Q_IMPROT_PLUGIN(QtMultimediaPlugin)
#endif

int main(int argc, char **argv)
{
    qputenv("QT_QPA_EGLFS_HIDECURSOR", "1");
    qputenv("QT_QPA_EGLFS_DISABLE_INPUT","1");
    qputenv("QT_QPA_EGLFS_INTEGRATION","eglfs_viv");
    qputenv("FB_MULTI_BUFFER","2");
    qputenv("QT_QPA_EGLFS_WIDTH","1280");
    qputenv("QT_QPA_EGLFS_HEIGHT","480");
    qputenv("QT_QPA_EGLFS_PHYSICAL_WIDTH","1280");
    qputenv("QT_QPA_EGLFS_PHYSICAL_HEIGHT","480");
    qputenv("QT_QPA_FONTDIR",".");

#ifdef STATIC
    qobject_cast<QQmlExtensionPlugin*>(qt_static_plugin_QtQuick2Plugin().instance())->registerTypes("QtQuick");
    qobject_cast<QQmlExtensionPlugin*>(qt_static_plugin_QtQuickScene3DPlugin().instance())->registerTypes("QtQuick.Scene3D");
    qobject_cast<QQmlExtensionPlugin*>(qt_static_plugin_Qt3DQuick3DCorePlugin().instance())->registerTypes("Qt3D.Core");
    qobject_cast<QQmlExtensionPlugin*>(qt_static_plugin_Qt3DQuick3DRenderPlugin().instance())->registerTypes("Qt3D.Render");
    //qobject_cast<QQmlExtensionPlugin*>(qt_static_plugin_QMultimediaDeclarativeModule().instance())->registerTypes("QtMultimedia");

    //qobject_cast<QQmlExtensionPlugin*>(qt_static_plugin_QtIVICorePlugin().instance())->registerTypes("QtIVICore");
    qobject_cast<QQmlExtensionPlugin*>(qt_static_plugin_QtIVIVehicleFunctionsPlugin().instance())->registerTypes("QtIVIVehicleFunctions");
    //qobject_cast<QQmlExtensionPlugin*>(qt_static_plugin_QMultimediaDeclarativeModule().instance())->registerTypes("QtMultimedia");
#endif
    QGuiApplication app(argc, argv);

    //QGuiApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);
    //QGuiApplication::setAttribute(Qt::AA_UseOpenGLES);

    QFontDatabase::addApplicationFont(":/fonts/Lato-Bold.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Lato-Semibold.ttf");
    int latoID = QFontDatabase::addApplicationFont(":/fonts/Lato-Regular.ttf");
    QStringList loadedFontFamilies = QFontDatabase::applicationFontFamilies(latoID);
    if (!loadedFontFamilies.empty()) {
        QString fontName = loadedFontFamilies.at(0);
        QGuiApplication::setFont(QFont(fontName));
    } else {
        qWarning("Error: fail to load Open Sans font");
    }

#ifdef QT_3DCORE_LIB
//    qmlRegisterType<SceneHelper>("Qt3D.Examples", 2, 0, "SceneHelper");
#endif
//    qmlRegisterType<QtIVIClusterData>("ClusterDemoData", 1, 0, "ClusterData");
//    qmlRegisterType<Gauge>("ClusterDemo", 1, 0, "GaugeFiller");
//    qmlRegisterSingletonType(QUrl("qrc:/qml/ValueSource.qml"), "ClusterDemo", 1, 0, "ValueSource");

    QQuickView view;

//    EtcProvider *provider = new EtcProvider();
//    provider->setBaseUrl(QUrl("qrc:///images/"));
//    view.engine()->addImageProvider("etc", provider);

    view.setColor(QColor(Qt::black));
    view.setWidth(1280);
    view.setHeight(480);
    //view.setResizeMode(QQuickView::SizeRootObjectToView);

    view.setSource(QUrl("qrc:/qml/DashboardFrame.qml"));
    view.show();
    return app.exec();
}
