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
import service.valuesource 1.0

Item {
    property int iconMargin: 10

    anchors.horizontalCenter: parent.horizontalCenter
    width: bottomPanelImage.width
    height: bottomPanelImage.height

    Image {
        id: bottomPanelImage
        source: "../images/BottomPanel.png"
    }

    TurnIndicator {
        direction: Qt.LeftArrow
        anchors.verticalCenter: textTime.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 250
        active: ValueSource.turnSignal & Qt.LeftArrow
    }

    Image {
        id: iconCoolant
        source: ValueSource.engineTemperature >= 100.0 ? "../images/Icon_Coolant_ON.png"
                                                       : "../images/Icon_Coolant_OFF.png"
        anchors.verticalCenter: textTime.verticalCenter
        anchors.right: iconBattery.left
    }

    Image {
        id: iconBattery
        source: ValueSource.batteryLevel <= 25.0 ? "../images/Icon_Battery_ON.png"
                                                 : "../images/Icon_Battery_OFF.png"
        anchors.verticalCenter: textTime.verticalCenter
        anchors.right: iconFuel.left
    }

    Image {
        id: iconFuel
        source: ValueSource.fuelLevel <= 20.0 ? "../images/Icon_Fuel_ON.png"
                                              : "../images/Icon_Fuel_OFF.png"
        anchors.verticalCenter: textTime.verticalCenter
        anchors.right: iconParkingBrake.left
    }

    Image {
        id: iconParkingBrake
        source: ValueSource.parkingBrake ? "../images/Icon_ParkingBrake_ON.png"
                                         : "../images/Icon_ParkingBrake_OFF.png"
        anchors.verticalCenter: textTime.verticalCenter
        anchors.right: textTime.left
        anchors.rightMargin: 5
    }

    Text {
        id: textTime
        text: ValueSource.time
        font.pixelSize: 24 //18
        color: "white"
        anchors.centerIn: parent
    }

    Image {
        id: iconLowbeam
        source: ValueSource.lowBeam ? "../images/Icon_LowBeam_ON.png"
                                    : "../images/Icon_LowBeam_OFF.png"
        anchors.verticalCenter: textTime.verticalCenter
        anchors.left: textTime.right
        anchors.leftMargin: iconMargin
    }

    Image {
        id: iconTyre
        source: ValueSource.flatTire ? "../images/Icon_TyreMalfunction_ON.png"
                                     : "../images/Icon_TyreMalfunction_OFF.png"
        anchors.verticalCenter: textTime.verticalCenter
        anchors.left: iconLowbeam.right
    }

    Image {
        id: iconLamp
        source: ValueSource.lightFailure ? "../images/Icon_BulbFailure_ON.png"
                                         : "../images/Icon_BulbFailure_OFF.png"
        anchors.verticalCenter: textTime.verticalCenter
        anchors.left: iconTyre.right
    }

    Image {
        id: iconSeatbelt
        //source: ValueSource.seatBelt ? "../images/Icon_SeatBelt_ON.png"
        //                             : "../images/Icon_SeatBelt_OFF.png"
        source: "../images/Icon_SeatBelt_ON.png"
        anchors.verticalCenter: textTime.verticalCenter
        anchors.left: iconLamp.right
    }

    TurnIndicator {
        direction: Qt.RightArrow
        anchors.verticalCenter: textTime.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 250
        active: ValueSource.turnSignal & Qt.RightArrow
    }
}
