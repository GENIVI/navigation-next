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

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQmlApplicationEngine>
#include <QDir>
#include<QDebug>
//#include "qtnavigatorconsoleplugin\MainViewConsole"
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load("./resource/main.qml");

//    QString qmlPath = QDir::currentPath();
//    qmlPath.replace(qmlPath.section('/',-1),"resource/main.qml");
//    engine.load(qmlPath);
//    qDebug()<<"appdir::"<<QDir::currentPath();

    ////load main.qml with QQuickView
    //    MainLoader view;
    //    view.rootContext()->setContextProperty("applicationDirPath", "file:///" + QGuiApplication::applicationDirPath());
    //    qDebug()<<"appdir"<<QGuiApplication::applicationDirPath();
    //    // Rendering in a thread introduces a slightly more complicated cleanup
    //    // so we ensure that no cleanup of graphics resources happen until the
    //    // application is shutting down.

    //    view.setPersistentOpenGLContext(true);
    //    view.setPersistentSceneGraph(true);
    //    view.setResizeMode(QQuickView::SizeRootObjectToView);
    //    view.setSource(QUrl("qrc:///main.qml"));
    //    view.show();
    return app.exec();
}

