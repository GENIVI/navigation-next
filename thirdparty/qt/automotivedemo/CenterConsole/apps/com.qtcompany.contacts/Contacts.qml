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

import QtQuick 2.7

import controls 1.0
import utils 1.0
import "."

AppUIScreen {
    id: root
    title: "Contacts"

    onVisibleChanged: {
        if (visible) {
            contactList.currentIndex = -1
            contactWidget.icon = Style.symbol("contacts")
            contactWidget.name = ""
        }
    }

    cluster: ClusterWidget {
        id: contactWidget
        icon: Style.symbol("contacts")
    }

    UIScreen {
        width: Style.screenWidth
        height: Style.screenHeight - Style.bottomBarHeight - Style.statusBarHeight
        onBackScreen: root.back()
        title: "Contacts"
        Image {
            anchors.top: parent.top
            anchors.topMargin: 100
            source: Style.symbol("background-contacts")
        }

        ListView {
            id: contactList
            anchors.top: parent.top
            anchors.topMargin: 200
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.left: parent.left
            currentIndex: -1
            highlightFollowsCurrentItem: true
            spacing: 20
            clip: true
            focus: true
            highlight: Rectangle {
                color: Style.colorWhite
                opacity: 0.25
                border.color: Qt.lighter(color, 1.2)
            }

            model: ListModel {
                function populateList() {
                    clear()
                    append({
                               icon: Style.symbol("jane"),
                               name: "Jane",
                               number: "+358 55 555 1234"
                           })
                    append({
                               icon: Style.symbol("john"),
                               name: "John",
                               number: "+358 55 555 4321"
                           })
                }

                Component.onCompleted: {
                    populateList()
                }
            }

            delegate: Item {
                anchors.horizontalCenter: parent.horizontalCenter
                width: 1080
                height: 136

                readonly property string currenticon: contactIcon.source
                readonly property string currentname: contactName.text

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 50

                    Image {
                        id: contactIcon
                        source: icon
                    }

                    Column {
                        anchors.verticalCenter: contactIcon.verticalCenter
                        spacing: 5

                        Label {
                            id: contactName
                            text: name
                            font.pixelSize: Style.fontSizeL
                        }

                        Label {
                            text: number
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: contactList.currentIndex = index
                }
            }

            onCurrentIndexChanged: {
                contactWidget.icon = currentItem.currenticon
                contactWidget.name = currentItem.currentname
            }
        }
    }
}
