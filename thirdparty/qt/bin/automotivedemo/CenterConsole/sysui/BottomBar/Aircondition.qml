import QtQuick 2.1
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0

import utils 1.0
import controls 1.0
import service.climate 1.0

Item {
    id: root
    property real startPoint: 0.5 * Math.PI
    property real endPoint: 2 * Math.PI

    property int ventilationLevels: ClimateService.ventilationLevels - 1
    property int ventilation: ClimateService.ventilation

    //Needed for swiping ventilation speed
    property int previousMouseX: 0

    //mouse.wasHeld does not work, create own
    property bool wasHeld

    property string activeFanDirection: "fandirection-windshieldandlegs"
    property string backgroundImage: Style.symbol("fandirection-knob-up")

    onActiveFanDirectionChanged: {
        currentFanDirection.source = Style.symbol(activeFanDirection)
    }

    width: 290
    height: Style.bottomBarHeight
    state: ""
    Image {
        anchors.centerIn: parent
        source: backgroundImage
    }
    Image {
        anchors.centerIn: parent
        z: 2
        source: Style.symbol("MiddleMask")
    }

    Item {
        id: fanDirection
        visible: root.state === "ventilationSelection"
        width: 400
        height: parent.height
        anchors.centerIn: parent
        z: 3
        Column {
            id: leftSide
            width: parent.width / 3
            height: parent.height - 10
            AirflowItem {
                property alias fanDir: root.activeFanDirection
                anchors.right: parent.right
                icon: "fandirection-head"
                climateId: ClimateService.airflow.dashboard
            }
            AirflowItem {property alias fanDir: root.activeFanDirection
                icon: "fandirection-headandlegs"
                climateId:  ClimateService.airflow.floor | ClimateService.airflow.dashboard
            }
            AirflowItem {
                property alias fanDir: root.activeFanDirection
                anchors.horizontalCenter: parent.horizontalCenter
                icon: "fandirection-legs"
                climateId: ClimateService.airflow.floor
            }
        }

        Column {
            id: rightSide
            anchors.right: parent.right
            width: parent.width / 3
            height: parent.height - 10
            AirflowItem {
                id: image
                property alias fanDir: root.activeFanDirection
                icon: "fandirection-windshield"
                climateId: ClimateService.airflow.windshield
            }
            Item {id: spacer; height: image.height; width: image.width}
            AirflowItem {
                property alias fanDir: root.activeFanDirection
                icon: "fandirection-windshieldandlegs"
                anchors.horizontalCenter: parent.horizontalCenter
                climateId: (ClimateService.airflow.windshield  | ClimateService.airflow.floor)
            }
        }
    }
    Item {
        id: centerImages
        anchors.fill: parent
        Column {
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            Image {
                id: currentFanDirection
                source: Style.symbol(activeFanDirection)
            }
            Image {
                source: root.state === "ventilationSelection"
                        ? Style.symbol("back") : Style.symbol("fandirection")
                anchors.horizontalCenter: currentFanDirection.horizontalCenter
            }
        }

    }

    MouseArea {
        anchors.fill: parent
        propagateComposedEvents: true
        //enabled: root.state === ""
        onReleased: {
            //If mouse was held, then swipe happened. Then ignore click
            if (wasHeld) {
                wasHeld = false
                return
            }
            if (root.state === "ventilationSelection")
                root.state = ""
            else
                root.state = "ventilationSelection"
        }
        onPositionChanged: {
            updateIndex(mouse.x)
            mouse.accepted = true
        }

        function updateIndex(x) {
            //Don't swipe too fast.
            if (Math.abs(previousMouseX - x) < 10)
                return
            //Swiping right, increase ventioation
            if ((x > previousMouseX ) && (root.ventilation < ventilationLevels))
                root.ventilation++
            //Swiping left, decrease ventioation
            else if ((x < previousMouseX ) && (root.ventilation > 0))
                root.ventilation--
            previousMouseX = x
            wasHeld = true
        }
    }

    Item {
        z: 13
        x: 48
        y: -10
        Repeater {
            model: root.ventilation
            Image {
                source: Style.symbol("Highlight")
                rotation: index * 30
            }
        }
    }
    onStateChanged: {
        state === "ventilationSelection" ? backgroundImage = Style.symbol("fandirection-knob-down")
                                         : backgroundImage = Style.symbol("fandirection-knob-up")
    }

    states: [
        State {
            name: "ventilationSelection"
            PropertyChanges {
                target: fanDirection
                visible: true
            }
        }
    ]
}


