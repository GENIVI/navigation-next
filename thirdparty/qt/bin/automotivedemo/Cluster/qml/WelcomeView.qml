/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.7
import QtQuick.Scene3D 2.0

Image
{
    id: welcomeView
    source: "../images/BackgroundGradient.png"
    width: 1920
    height: 720

    property alias hidden: carModel.hidden
    property bool isReady: false

    Scene3D {
        id: welcomeScene
        width: welcomeView.width / 2
        height: welcomeView.height
        multisample: true

        CarModel {
            id: carModel
            width: welcomeView.width
            height: welcomeView.height
        }
    }

    Rectangle {
        id: infoPane
        anchors.top: welcomeView.top
        anchors.bottom: welcomeView.bottom
        anchors.right: welcomeView.right
        anchors.left: welcomeScene.right
        color: "transparent"

        Loader {
            id: infoPaneLoader
            anchors.fill: parent
            source: "WelcomeDriver.qml"
            active: true
        }
    }

    Image {
        id: passengerDriver
        source: "../images/welcomeview_passenger_icon.png"
        opacity: 0.0
        scale: 2.0
        x: 410
        y: 330
        Behavior on opacity {
            PropertyAnimation { duration: 1000 }
        }
        Behavior on scale {
            PropertyAnimation {
                duration: 1000
                easing.type: Easing.OutCubic
            }
        }
    }

    Image {
        id: passengerShotgun
        source: "../images/welcomeview_passenger_icon.png"
        opacity: 0.0
        scale: 2.0
        x: 500
        y: 330
        Behavior on opacity {
            PropertyAnimation { duration: 1000 }
        }
        Behavior on scale {
            PropertyAnimation {
                duration: 1000
                easing.type: Easing.OutCubic
            }
        }
    }

    Image {
        id: passengerRearLeft
        source: "../images/welcomeview_passenger_icon.png"
        opacity: 0.0
        scale: 2.0
        x: 415
        y: 430
        Behavior on opacity {
            PropertyAnimation { duration: 1000 }
        }
        Behavior on scale {
            PropertyAnimation {
                duration: 1000
                easing.type: Easing.OutCubic
            }
        }
    }

    Image {
        id: passengerRearRight
        source: "../images/welcomeview_passenger_icon.png"
        opacity: 0.0
        scale: 2.0
        x: 495
        y: 430
        Behavior on opacity {
            PropertyAnimation { duration: 1000 }
        }
        Behavior on scale {
            PropertyAnimation {
                duration: 1000
                easing.type: Easing.OutCubic
            }
        }
    }

    WelcomeStatusNote {
        id: washerFluidNote
        text: "<center><font color=\"#FFFFFF\">Washer fluid<br>Low <font color=\"#EF2973\">10%"
        icon: "../images/status_washer_fluid_icon.png"
        highlight: "../images/status_washer_fluid_highlight.png"
        position: "right"
        opacity: 0.0
        x: 535
        y: 200
        Behavior on opacity {
            PropertyAnimation { duration: 1000 }
        }
    }

    WelcomeStatusNote {
        id: batteryLowNote
        text: "<center><font color=\"#FFFFFF\">Battery low<br>Range <font color=\"#EF2973\">35KM"
        icon: "../images/status_battery_icon.png"
        highlight: "../images/status_battery_highlight.png"
        position: "left"
        opacity: 0.0
        x: 205
        y: 497
        Behavior on opacity {
            PropertyAnimation { duration: 1000 }
        }
    }

    // Functions to control highlights from dashboard
    function highlightLamp() {
        var type = carModel.highlightLamp()
        carModel.toggleIdleTimer(true)
        resetModelTimer.restart()
        return type
    }

    function highlightDoors(doors) {
        doorAction = true
        carModel.doorAction = true
        carModel.highlightOpenDoors(doors)
        carModel.toggleIdleTimer(true)
        resetModelTimer.restart()
    }

    function highlightTire() {
        var type = carModel.highlightTire()
        carModel.toggleIdleTimer(true)
        resetModelTimer.restart()
        return type
    }

    Timer {
        running: true
        interval: 6000
        onTriggered: {
            infoPaneLoader.source = "WelcomeTripSummary.qml"
            statusCheckTimer.start()
            passengerDriver.opacity = 1.0
            passengerRearRight.opacity = 1.0
            passengerDriver.scale = 1.0
            passengerRearRight.scale = 1.0
            washerFluidNote.opacity = 1.0
            batteryLowNote.opacity = 1.0
        }
    }

    Timer {
        id: statusCheckTimer
        running: false
        interval: 5000
        onTriggered: {
            infoPaneLoader.source = "WelcomeStatus.qml"
            batteryLowNote.opacity = 0.0
            readyTimer.start()
        }
    }

    Component.onCompleted: {
        carModel.startWelcomeAnimation()
    }

    Timer {
        id: readyTimer
        running: false
        interval: 5000
        onTriggered: {
            isReady = true
        }
    }
}
