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

import QtQuick 2.0
import QtLocation 5.5
import QtPositioning 5.5

import controls 1.0
import utils 1.0
import service.valuesource 1.0

import "Launcher"
import "CenterItems"

import QtApplicationManager 1.0
//import com.comtech 1.0

Item {
    id: root
//    property int visibleScreenHeight: Style.screenHeight - Style.bottomBarHeight
//                                      - Style.statusBarHeight
//    width: Style.screenWidth
//    height: visibleScreenHeight
//    state: "enterCar"

//    property int searchInvisibleHeight: visibleScreenHeight + Style.searchInputHeight
//                                        + Style.bottomBarHeight + searchInput.panelHeight + 30
//    property int searchVisibleHeight: visibleScreenHeight + searchInput.panelHeight
//    property int invisibleY: visibleScreenHeight + Style.bottomBarHeight


//    MapPage {
//        id: mapPage
//        property alias mapState: root.state
//        height: visibleScreenHeight
//    }

//    SearchInput {
//        id: searchInput
//        y: searchInvisibleHeight
//        property alias mapState: root.state
//    }

//    DestinationInfo {
//        id: destinationInfo
//        y: invisibleY
//        property alias mapState: root.state
//    }

//    LauncherBar {
//        id: launcherBar
//        y: visibleScreenHeight - launcherBar.height
//        anchors.left: parent.left
//        anchors.right: parent.right
//    }
//    states: [
//        State {
//            name: "enterCar"
//            PropertyChanges {
//                target: mapPage
//                navigationStarted: false
//            }
//        },
//        State {
//            name: "selectDestination"
//            PropertyChanges {
//                target: launcherBar
//                y: invisibleY
//            }
//            PropertyChanges {
//                target: destinationInfo
//                y: invisibleY
//            }
//            PropertyChanges {
//                target: mapPage
//                navigationStarted: false
//            }
//            PropertyChanges {
//                target: searchInput
//                y: searchVisibleHeight
//            }
//        },
//        State {
//            name: "showRoute"
//            PropertyChanges {
//                target: launcherBar
//                y: invisibleY
//            }
//            PropertyChanges {
//                target: destinationInfo
//                y: visibleScreenHeight - destinationInfo.height
//            }
//            PropertyChanges {
//                target: mapPage
//                navigationStarted: false
//            }
//            StateChangeScript {
//                script: {
//                    mapPage.showRoute()
//                }
//            }
//        },
//        State {
//            name: "driveStarted"
//            PropertyChanges {
//                target: launcherBar
//                y: invisibleY
//            }
//            PropertyChanges {
//                target: destinationInfo
//                y: invisibleY
//            }
//            PropertyChanges {
//                target: searchInput
//                y: searchVisibleHeight
//            } //Start route quidance
//            PropertyChanges {
//                target: mapPage
//                navigationStarted: true
//            }
//        },
//        State {
//            name: "navigationRunning"
//            PropertyChanges {
//                target: mapPage
//                navigationStarted: true
//            }
//        },
//        State {
//            name: "tripSummary"
//        }
//    ]

//    transitions: [
//        Transition {
//            from: "enterCar"; to: "selectDestination"; reversible: true
//            SequentialAnimation {
//                NumberAnimation { target: launcherBar; properties: "y"; duration: 300 }
//                NumberAnimation { target: searchInput; properties: "y"; duration: 300 }
//            }
//        },
//        Transition {
//            from: "selectDestination"; to: "showRoute"; reversible: true
//            SequentialAnimation {
//                NumberAnimation { target: destinationInfo; properties: "y"; duration: 300 }
//            }
//        },
//        Transition {
//            from: "showRoute"; to: "driveStarted"; reversible: true
//            SequentialAnimation {
//                NumberAnimation { target: destinationInfo; properties: "y"; duration: 300 }
//                NumberAnimation { target: searchInput; properties: "y"; duration: 300 }
//            }

//        },
//        Transition {
//            from: "showRoute"; to: "enterCar"; reversible: true
//            SequentialAnimation {
//                NumberAnimation { target: destinationInfo; properties: "y"; duration: 300 }
//                NumberAnimation { target: launcherBar; properties: "y"; duration: 300 }
//            }
//        },
//        Transition {
//            from: "driveStarted"; to: "navigationRunning"; reversible: true
//            SequentialAnimation {
//                NumberAnimation { target: destinationInfo; properties: "y"; duration: 300 }
//                NumberAnimation { target: launcherBar; properties: "y"; duration: 300 }
//            }
//        }
//    ]
//    onStateChanged: console.log("State: "+state)


    Component.onCompleted: {
        console.log("appIds=" + ApplicationManager.applicationIds())
        ApplicationManager.startApplication("com.locationstudio.qtnavigator")
    }
}

