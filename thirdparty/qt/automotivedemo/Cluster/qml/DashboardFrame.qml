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

import QtQuick 2.7
import Qt3D.Core 2.0

Item {
    id: root
    width: 1920
    height: 1080//720
    anchors.top: parent.top

    property real faderOpacity: 1.0

    // TODO: Toggle isReady to true after status check has been shown
    Loader {
        id: welcomeLoader
        //source: "qrc:/qml/WelcomeView.qml"
        source: "WelcomeView.qml"
        //source: "DashboardView.qml"
        width: root.width
        height: 720//root.height
        onLoaded: {
            initialFade.start()
        }
    }

    property bool welcomeReady: welcomeLoader.status === Loader.Ready && welcomeLoader.item.isReady

    onWelcomeReadyChanged: {
        faderAnimation.start()
    }

    Loader {
        id: dashLoader
        //source: "qrc:/qml/DashboardView.qml"
        source: "DashboardView.qml"
        visible: false
        active: true
        width: root.width
        height: 720//root.height
    }

    Rectangle {
        id: fader
        color: "black"
        anchors.fill: root
        opacity: faderOpacity
    }

    PropertyAnimation {
        id: initialFade
        target: root
        property: "faderOpacity"
        from: 1.0
        to: 0.0
        duration: 1000
    }

    SequentialAnimation {
        id: faderAnimation
        running: false

        PropertyAnimation {
            target: root
            property: "faderOpacity"
            from: 0.0
            to: 1.0
            duration: 500
        }
        ScriptAction {
            script: {
                welcomeLoader.visible = false
                dashLoader.visible = true
            }
        }
        PropertyAnimation {
            target: root
            property: "faderOpacity"
            from: 1.0
            to: 0.0
            duration: 500
        }
    }

    // TODO: Use if cluster is displayed on 1080p screen
    Rectangle {
        color: "black"
        anchors.bottom: parent.bottom
        height: 360
        width: 1920

        Image {
            source: "../images/Built_with_Qt_RGB_logo_white.png"
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.margins: 30
        }
    }
}
