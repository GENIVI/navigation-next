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
//import models 1.0
import controls 1.0
import utils 1.0
import com.locationstudio.qtnavigator.cluster 1.0
import com.locationstudio.qtnavigator.console 1.0

Item {
    id: root

    width: Style.clusterWidth
    height: Style.clusterHeight

ClusterView {
        anchors.fill: parent
        width: Style.clusterWidth - 50
        workFolder: "/opt/locationstudio/res/cluster"
        anchors.topMargin: 200
        anchors.leftMargin: 500
        anchors.rightMargin: 500
        anchors.bottomMargin: 80
        zoom: 17.0
        tilt: 15.0
        avatarPosition.x: -118.25 // avatar latitude
        avatarPosition.y: 34.05 // avatar longitude
        avatarHeading: 0.0 // avatar heading
        isFollowMe: true // is follow me mode
        client: "Neptune"
        //token: "<YOUR_APIKEY_HERE>" //Uncomment this line when you set your API key 

    }

    Middle {
        id: widgetBase
        anchors.centerIn: parent
    }

    LeftDial {
        id: leftDial
    }

    RightDial {
        id: rightDial
        x: (root.width - (width + 0.1 * width))
    }

    Top {
        id: topbar
        y: 7
        anchors.horizontalCenter: parent.horizontalCenter
    }

    //    Notifications {
    //        id: notifications
    //        y: root.zoom ? root.height : root.height - notifications.height - 15
    //        anchors.horizontalCenter: parent.horizontalCenter
    //    }

    Image {
        anchors.fill: parent
        source: Style.gfx("cluster/mask_overlay")
    }

    focus: Style.debugMode

    Keys.onPressed: {
        if (event.key === Qt.Key_Space) {
            if (overlay.opacity < 0.5) overlay.opacity = 0.5
            else overlay.opacity = 0
        }
    }

    Keys.forwardTo: Style.debugMode ? [layouter] : widgetBase

    property var layoutTarget//: notifications

    Layouter {
        id: layouter
        target: layoutTarget
    }
}

