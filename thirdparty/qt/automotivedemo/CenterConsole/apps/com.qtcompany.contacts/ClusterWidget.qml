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
import utils 1.0

Item {
    id: contactContainer
    opacity: 0.5
    property int defaultYPos: 0
    y: defaultYPos
    property alias icon: contactImage.source
    property alias name: contactName.text

    Image {
        id: contactImage
        width: 136
        height: 136
    }

    Text {
        id: title
        anchors.top: contactImage.bottom
        anchors.horizontalCenter: contactImage.horizontalCenter
        text: name === "" ? "Browsing\ncontacts" : "Calling"
        color: "gray"
        font.pixelSize: 20
    }

    Text {
        id: contactName
        anchors.top: title.bottom
        anchors.horizontalCenter: contactImage.horizontalCenter
        color: "lightGray"
        font.pixelSize: 24
    }

    Timer {
        id: fadeOutTimer
        interval: 5000
        running: false
        repeat: false
        onTriggered: {
            fadeOut.start()
        }
    }

    PropertyAnimation on opacity {
        id: fadeIn
        to: 1.0
        duration: 500
    }

    PropertyAnimation on opacity {
        id: fadeOut
        to: 0.5
        duration: 500
    }

//    PropertyAnimation on y {
//        id: startupAnimation
//        to: 0
//        duration: 500
//        easing.type: Easing.InCubic
//    }

    Component.onCompleted: {
//        startupAnimation.start()
//        fadeIn.start()
//        fadeOutTimer.start()
    }

    onVisibleChanged: {
        if (visible) {
//            y = defaultYPos
//            startupAnimation.start()
            fadeIn.start()
            fadeOutTimer.restart()
        }
    }

    onIconChanged:  {
        fadeIn.start()
        fadeOutTimer.restart()
    }
}

