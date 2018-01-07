/****************************************************************************
**
** Copyright (C) 2016 Pelagicore AG
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Neptune IVI UI.
**
** $QT_BEGIN_LICENSE:GPL-QTAS$
** Commercial License Usage
** Licensees holding valid commercial Qt Automotive Suite licenses may use
** this file in accordance with the commercial license agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and The Qt Company.  For
** licensing terms and conditions see https://www.qt.io/terms-conditions.
** For further information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
** SPDX-License-Identifier: GPL-3.0
**
****************************************************************************/

import QtQuick 2.1
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.4
import controls 1.0
import utils 1.0
import "."

AppUIScreen {
    id: root
    title: "Calendar"

    cluster: ClusterWidget {
        appointment: "No appointments"
    }

    UIScreen {
        width: Style.screenWidth
        height: Style.screenHeight - Style.bottomBarHeight - Style.statusBarHeight
        onBackScreen: root.back()
        title: "Calendar"
        Calendar {
            weekNumbersVisible: false
            width: root.width - 150
            height: root.height - 150
            x: 75
            y: 75
            style: CalendarStyle {
                gridVisible: false
                gridColor: "transparent"
                background: Rectangle {
                   color: "transparent"
                }

                dayOfWeekDelegate: Item {
                    implicitHeight: control.height * 0.13

                    Label {
                       text: localeDayName.length == 0 || localeDayName.length > 1
                           ? control.__locale.dayName(styleData.dayOfWeek, Locale.ShortFormat)
                           : localeDayName

                       anchors.fill: parent
                       horizontalAlignment: Text.AlignHCenter

                       property string localeDayName: control.__locale.dayName(styleData.dayOfWeek, Locale.NarrowFormat)
                    }
                }

                dayDelegate: Rectangle {
                    color: "transparent"
                    Rectangle {
                       anchors.fill: parent
                       anchors.margins: 2
                       gradient: Gradient {
                           GradientStop {
                               position: 0.0
                               color: styleData.selected ? "#666" : (styleData.visibleMonth && styleData.valid ? "#333" : "#111");
                           }
                           GradientStop {
                               position: 1.00
                               color: "transparent";
                           }
                       }
                       Label {
                           text: styleData.date.getDate()
                           color: styleData.valid ? "white" : "grey"
                       }
                    }
                }
                navigationBar: Rectangle {
                   implicitHeight: Math.round(control.height * 0.1)
                   color: "transparent"
                   RowLayout {
                       anchors.fill: parent
                       Text {
                           id: prevButton
                           text: "\u25c0" // LEFT-POINTING TRIANGLE
                           font.pixelSize: Style.fontSizeXXS
                           color: "white"
                           MouseArea {
                               anchors.fill: parent
                               onClicked: control.showPreviousMonth()
                           }
                       }
                       Label {
                           id: dateText
                           Layout.alignment: Qt.AlignCenter
                           text: styleData.title
                           horizontalAlignment: Text.AlignHCenter
                           font.pixelSize: Style.fontSizeL
                       }
                       Text {
                           id: nextButton
                           Layout.alignment: Qt.AlignRight
                           text: "\u25b6" // RIGHT-POINTING TRIANGLE
                           font.pixelSize: Style.fontSizeXXS
                           color: "white"
                           MouseArea {
                               anchors.fill: parent
                               onClicked: control.showNextMonth()
                           }
                       }
                   }
                }
            }
        }
    }
}
