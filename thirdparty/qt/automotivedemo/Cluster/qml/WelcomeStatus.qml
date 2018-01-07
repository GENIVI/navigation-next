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
import service.valuesource 1.0

Item {
    anchors.fill: parent

    Column {
        anchors.centerIn: parent
        spacing: 20

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 10

            Image {
                id: statusImage
                source: "../images/welcomeview_status_icon.png"
            }

            Text {
                id: statusText
                anchors.verticalCenter: statusImage.verticalCenter
                text: "READY FOR THE ROAD"
                font.pixelSize: 18
                color: "white"
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 20

            Rectangle {
                id: line
                height: 2
                width: 205
                color: "gray"
            }

            Text {
                text: "Status"
                font.pixelSize: 18
                color: "white"
                anchors.verticalCenter: line.verticalCenter
            }

            Rectangle {
                height: 2
                width: 205
                color: "gray"
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 50

            WelcomeInfoNote {
                firstline: ValueSource.totalDistance.toString()
                firstlineunit: "KM"
                secondline: "Total\ndistance"
            }

            WelcomeInfoNote {
                firstline: ValueSource.kmSinceCharge.toString()
                firstlineunit: "KM"
                secondline: "Since last\ncharge"
            }

            WelcomeInfoNote {
                firstline: ValueSource.avRangePerCharge.toString()
                firstlineunit: "KM"
                secondline: "Average\nrange/charge"
            }

            WelcomeInfoNote {
                firstline: ValueSource.energyPerKm.toString()
                firstlineunit: "WH"
                secondline: "energy/km"
            }
        }

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            height: 2
            width: 500
            color: "gray"
        }
    }
}
