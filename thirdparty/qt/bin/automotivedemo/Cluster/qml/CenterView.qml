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
import models 1.0
import controls 1.0
import utils 1.0
import com.locationstudio.qtnavigator.cluster 1.0

Item {
    anchors.fill: parent
    property real defaultScale: 1.0
    property var previousView: initialView


    Image {
        id: initialView
        source: Style.symbol("john")
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 400
    }

    Item {
        id: musicContainer
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 270
        width: 354
        height: 354
        default property alias content: musiccontainer.children

        Item {
            id: musiccontainer
            anchors.fill: parent
        }
    }

    Item {
        id: contactContainer
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 350
        width: 136
        height: 250
        default property alias content: contactcontainer.children

        Item {
            id: contactcontainer
            anchors.fill: parent
        }
    }

    Item {
        id: carinfoContainer
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 340
        width: 265
        height: 250
        default property alias content: carinfocontainer.children

        Item {
            id: carinfocontainer
            anchors.fill: parent
        }
    }

    Item {
        id: calendarContainer
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 340
        width: 220
        height: 250
        default property alias content: calendarcontainer.children

        Item {
            id: calendarcontainer
            anchors.fill: parent
        }
    }

    ClusterView {
        id: map2
        workFolder: "/opt/locationstudio/res/cluster"
        anchors.fill: parent
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
	    client: "Automotive"
        //token: "<YOUR_APIKEY_HERE>" //Uncomment this line when you set your API key

    }


    Connections {
        target: ApplicationManagerInterface
        onClusterWidgetReady: {
            handleWidgetChange(category, item)
        }
        onClusterWidgetActivated: {
            handleWidgetChange(category, item)
        }
    }

    PropertyAnimation {
        id: shrinkCenter
        property: "scale"
        to: 0.0
        running: false
        duration: 500
        onStopped: {
            target.visible = false
        }
    }

    function handleWidgetChange(category, item) {
        var container
        if (category === "media")
            container = musicContainer
        else if (category === "contacts")
            container = contactContainer
        else if (category === "carinfo")
            container = carinfoContainer
        else if (category === "calendar")
            container = calendarContainer
        if (previousView !== container) {
           // container.scale = defaultScale
            container.visible = true
            previousView.visible = false
          //  shrinkCenter.target = previousView
            previousView = container
           // shrinkCenter.start()
            item.parent = container
            container.content = item
            item.width = container.width
            item.height = container.height
        }
    }
}
