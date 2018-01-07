/*
Copyright (c) 2018, TeleCommunication Systems, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
   * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the TeleCommunication Systems, Inc., nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED. IN NO EVENT SHALL TELECOMMUNICATION SYSTEMS, INC.BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//#include "mainwindow.h"
#include "inifile.h"
//#include "devtooldialog.h"
#include "globalsetting.h"
//#include <QApplication>
#include <QDir>
#include <QDebug>
//#include <QMessageBox>
//#include <modeselectordialog.h>
#include <QtQuick/QQuickView>
#include "mainitem.h"
#include <QGuiApplication>



int main(int argc, char *argv[])
{
#ifdef Q_OS_LINUX
    QCoreApplication::setAttribute(Qt::AA_X11InitThreads);
#endif
    QGuiApplication a(argc, argv);

//    RunMode mode;
//    int items = MSD_NavMode;
//    ModeSelectorDialog dlg(mode,items);
//    dlg.exec();

//    if(mode.dataFile.isEmpty()){
//        mode.dataFile = settings.GetMapDataPath();
//    }
//    InitLTKHybridManager(mode.enableOnboard, mode.dataFile);

//    locationtoolkit::MapWidget* globalMapWidget = GetMapWidget();
//    if( globalMapWidget == NULL )
//    {
//        qDebug( "create mapkit3d widget failed!!!\n" );
//        return -1;
//    }

    qmlRegisterType<MainItem>("test.map", 1, 0, "MapView");

    QQuickView view;

    // Rendering in a thread introduces a slightly more complicated cleanup
    // so we ensure that no cleanup of graphics resources happen until the
    // application is shutting down.
    view.setPersistentOpenGLContext(true);
    view.setPersistentSceneGraph(true);
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:///main.qml"));
    view.show();
//    MainWindow w( globalMapWidget );
//    w.show();
    return a.exec();
}
