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
import QtLocation 5.5
import QtPositioning 5.5

import controls 1.0
import utils 1.0
import service.valuesource 1.0
import service.statusbar 1.0

Item {
    id: root
    width: Style.screenWidth
    height: parent.height
    property var startCoordinate: QtPositioning.coordinate(52.53471566, 13.29339621)
    property var endCoordinate: QtPositioning.coordinate(52.51329679, 13.32095033)
    property bool navigationStarted: false
    property string totalTravelTime
    property string totalDistance
    property string arrivalTime
    property bool offlineMode: false

    Image {
        z: 2
        source: Style.symbol("MapOverlayEdgeGradient")
    }

    onNavigationStartedChanged: {
        if (navigationStarted) {
            ValueSource.automaticDemoMode = false
            positionSource.start()
            routeStopped.running = true
        } else {
            ValueSource.automaticDemoMode = true
            positionSource.stop()
            routeStopped.running = false
        }
    }

    Map {
        id: map
        width: parent.width
        height: parent.height
        color: "black"
        property real speed
        property bool mapManuallyMoved: false
        property bool flickStarted: false

        plugin: Plugin {
            id: plugin
            preferred: ["mapbox"]
            PluginParameter { name: "mapbox.access_token"; value: "pk.eyJ1IjoicXRjbHVzdGVyIiwiYSI6ImNpdHh4NTkzNzAwMzUzbm9heWxmZGZ5eW8ifQ.1DH7_D3VyURcEcl8fbayMA" }
            PluginParameter { name: "mapbox.map_id"; value: "qtcluster.d7eed331" }
//            PluginParameter { name: "mapbox.cache.directory"; value: "/tmp" }
//            PluginParameter { name: "mapbox.cache.memory.size"; value: "4096" }
//            PluginParameter { name: "mapbox.cache.texture.size"; value: "9192" }
        }

        center: startCoordinate

        //Helper timer to notice when position is not updated anymore,
        //then we need to start the route over.
        Timer {
            id: routeStopped
            interval: 5000
            running: false
            onTriggered: {
                positionSource.nmeaSource = ""
                positionSource.nmeaSource = "route.txt"
                positionSource.start()
            }
        }

        PositionSource {
            id: positionSource
            nmeaSource: "route.txt"
            onPositionChanged: {
                if (position.speedValid) {
                    ValueSource.kph = position.speed
                    routeStopped.restart()
                }
            }
        }

        Behavior on center {
            id: centerBehavior
            enabled: true
            CoordinateAnimation { duration: 1500 }
        }

        zoomLevel: 16
        gesture.enabled: true
        gesture.acceptedGestures: MapGestureArea.PinchGesture | MapGestureArea.PanGesture
                                  | MapGestureArea.FlickGesture
        gesture.onFlickFinished: {
            flickStarted = false
            checkMapMoving()
        }
        gesture.onFlickStarted: {
            flickStarted = true
        }
        gesture.onPanFinished: {
            if (!flickStarted)
                checkMapMoving()
        }
        function checkMapMoving() {
            if (!map.visibleRegion.contains(positionQuickItem.coordinate))
                map.mapManuallyMoved = true
            else
                map.mapManuallyMoved = false
        }

        RouteModel {
            id: routeModel
            plugin: map.plugin
            query: RouteQuery {
                id: routeQuery
            }
            onRoutesChanged: {
                totalTravelTime = routeModel.count == 0
                        ? "" : formatTime(routeModel.get(0).travelTime)
                totalDistance = routeModel.count == 0
                        ? "" : formatDistance(routeModel.get(0).distance)
                arrivalTime = routeModel.count == 0
                        ? "" : formatArrivalTime(routeModel.get(0).travelTime)
            }
            onErrorChanged:{
                console.log("Map route error: "+errorString)
                offlineMode = true
            }
        }

        MapItemView {
            model: routeModel
            delegate: routeDelegate
            autoFitViewport: true
        }

        Component {
            id: routeDelegate

            MapRoute {
                id: route
                route: routeData
                line.color: Style.colorPink
                line.width: 5
                smooth: true
                opacity: 0.8
            }
        }

        MapQuickItem {
            id: positionQuickItem
            z: 3
            coordinate: root.navigationStarted && positionSource.position.latitudeValid
                        ? positionSource.position.coordinate
                        : startCoordinate
            anchorPoint.x: positionImage.width / 2
            anchorPoint.y: positionImage.height / 2
            sourceItem: Image {
                id: positionImage
                source: root.mapState === "enterCar"
                        ? Style.symbol("search") : Style.symbol("NavRoundmarker")
            }
            Behavior on coordinate {
                enabled: true
                CoordinateAnimation { duration: 1500 }
            }
        }
        //In offline mode this replaces MapRoute
        MapPolyline {
            id: offlineRoute
            visible: false
            line.color: Style.colorPink
            line.width: 5
            opacity: 0.8
            smooth: true
        }

        MouseArea {
            anchors.fill: parent
            propagateComposedEvents: true
            enabled: root.mapState === "selectDestination"
                     || root.mapState === "enterCar"
                     || root.mapState === "navigationRunning" ? true : false
            onClicked: {
                if (root.mapState === "navigationRunning")
                    root.mapState = "driveStarted"
                else
                    root.mapState = "selectDestination"
            }
        }

        Timer {
            interval: 3000
            repeat: true
            running: !map.mapManuallyMoved && (root.mapState === "navigationRunning"
                                               || root.mapState === "driveStarted")
            onTriggered: {
                //Check if positionQuickItem has moved out of visible area. We cannot do that in
                //onCoordinageChanged function as that fires too often
                if (map.mapManuallyMoved || map.panOngoing)
                    return
                if (!map.visibleRegion.contains(positionQuickItem.coordinate))
                    map.center = positionQuickItem.coordinate
            }
        }
    }

    Image {
        id: backButton
        z: 3
        anchors.left: parent.left
        anchors.top: root.top
        anchors.margins: 20
        visible: (root.mapState !== "enterCar") && (root.mapState !== "navigationRunning")
        source: Style.symbol("back-background")
        MouseArea {
            id: backArea
            anchors.fill: parent
            onClicked: {
                //When back button is pressed, return to enter car screen.
                //If navigation is running, then show navi info
                if (root.mapState !== "driveStarted")
                    root.mapState = "enterCar"
                else
                    root.mapState = "navigationRunning"

                if (Qt.inputMethod.visible)
                    Qt.inputMethod.hide()
            }
        }
        scale: backArea.pressed ? 0.85 : 1.0
        Behavior on scale {
            NumberAnimation {}
        }
    }

    Image {
        id: carFocusButton
        z: 3
        visible: map.mapManuallyMoved
        source: Style.symbol("CarFocusButton")
        anchors.right: parent.right
        anchors.top: root.top
        anchors.margins: 20
        MouseArea {
            id: area
            anchors.fill: parent
            onClicked: {
                map.mapManuallyMoved = false
                if (root.mapState === "navigationRunning" || root.mapState === "driveStarted")
                    map.center = positionSource.position.coordinate
                else
                    map.center = startCoordinate
            }
        }
        scale: area.pressed ? 0.85 : 1.0

        Behavior on scale {
            NumberAnimation {}
        }
    }

    Label {
        id: currentLocationText
        z: 3
        visible: !navigationStarted | root.mapState === "navigationRunning"
        y: 10
        anchors.horizontalCenter: parent.horizontalCenter
        font.pixelSize: Style.fontSizeL
        text: root.mapState === "navigationRunning" ? "DESTINATION" : "YOU ARE NOW AT"
    }

    Label {
        id: currentLocationText2
        z: 3
        visible: !navigationStarted | root.mapState === "navigationRunning"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: currentLocationText.bottom
        font.pixelSize: Style.fontSizeXL
        text: root.mapState === "navigationRunning"
              ? ValueSource.destination : ValueSource.currentLocation
    }

    Row {
        visible: root.mapState === "navigationRunning"
        z: 3
        anchors.top: currentLocationText2.bottom
        anchors.topMargin: Style.padding
        anchors.horizontalCenter: currentLocationText2.horizontalCenter
        spacing: Style.paddingXL
        ArrivalInfo { arText: arrivalTime; text2: "arrival time" }
        ArrivalInfo { arText: totalTravelTime; text2: "duration" }
        ArrivalInfo { arText: totalDistance; text2: "distance" }
    }

    Label {
        z: 3
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: Style.padding
        anchors.topMargin:5
        font.pixelSize: 9
        text: "© Mapbox © OpenStreetMap"
    }

    function showRoute() {
        if (offlineMode) {
            console.log("application in offline mode")
            var doc = new XMLHttpRequest();
            doc.onreadystatechange = function() {
                console.log("readychanged "+doc.readyState)
                if (doc.readyState === XMLHttpRequest.DONE) {
                    var a = JSON.parse(doc.responseText)
                    offlineRoute.path = a
                    offlineRoute.visible = true
                }
            }
            doc.open("GET", "offlineRoute.txt");
            doc.send();
        }
        else {
            routeQuery.clearWaypoints()

            // add the start and end coords as waypoints on the route
            routeQuery.addWaypoint(startCoordinate)
            routeQuery.addWaypoint(endCoordinate)
            routeQuery.travelModes = RouteQuery.CarTravel
            routeQuery.routeOptimizations = RouteQuery.FastestRoute

            routeModel.update()
        }
    }

    function hideRoute() {
        if (offlineMode) {
            offlineRoute.visible = false
        }
        else {
            routeQuery.clearWaypoints()
            routeModel.reset()
            routeModel.update()
         }
    }

    function formatTime(sec)
    {
        var value = sec
        var seconds = value % 60
        value /= 60
        value = (value > 1) ? Math.round(value) : 0
        var minutes = value % 60
        value /= 60
        value = (value > 1) ? Math.round(value) : 0
        var hours = value
        if (hours > 0)
            value = hours + "h:" + minutes + "m"
        else
            value = minutes + " min"
        return value
    }

    function formatDistance(meters)
    {
        var dist = Math.round(meters)
        if (dist > 1000) {
            if (dist > 100000) {
                dist = Math.round(dist / 1000)
            } else {
                dist = Math.round(dist / 100)
                dist = dist / 10
            }
            dist = dist + " km"
        } else {
            dist = dist + " m"
        }
        return dist
    }

    function formatArrivalTime(sec)
    {
        var date = StatusBarService.currentDate
        var secs = date.getUTCSeconds() + sec
        date.setUTCSeconds(secs)
        date = Qt.formatDateTime(date, "hh:mm")
        return date
    }
}

