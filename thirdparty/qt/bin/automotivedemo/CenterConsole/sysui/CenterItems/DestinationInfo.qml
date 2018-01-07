import QtQuick 2.0

import utils 1.0
import controls 1.0
import service.valuesource 1.0

Rectangle {
    id: root
    width: Style.screenWidth
    height: 210
    color: "#303A48"
    opacity: 0.8

    Image{
        id: closeButton
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: Style.paddingXXL
        source: Style.symbol("cancel")
        MouseArea{
            anchors.fill: parent
            onClicked: {
                root.mapState = "selectDestination"
            }
        }
    }
    Column {
        id: column
        width: parent.width
        anchors.top: parent.top
        anchors.margins: Style.paddingXXL
        spacing: 20
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "To " + ValueSource.destination
            font.pixelSize: Style.fontSizeM
        }
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "From " + ValueSource.currentLocation
        }
        Rectangle {
            width: parent.width
            height: 2
            color: "#8E9399"
        }
    }

    Rectangle {
        color: "white"
        anchors.top: column.bottom
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 80
        height: Style.searchInputHeight
        width: parent.width / 2 - 100
        radius: Style.buttonRadius
        Label {
            id: txt
            anchors.centerIn: parent
            text: "Manual drive"
            color: Style.colorPink
        }
        MouseArea {
            anchors.fill: parent
            onClicked: root.mapState = "driveStarted"
        }
    }
    Rectangle {
        color: "white"
        anchors.top: column.bottom
        anchors.right: parent.right
        anchors.rightMargin: 80
        anchors.topMargin: 10
        height: Style.searchInputHeight
        width: parent.width / 2 - 100
        radius: Style.buttonRadius
        Label {
            anchors.centerIn: parent
            text: "Autopilot"
            color: Style.colorPink
        }
        MouseArea {
            anchors.fill: parent
            onClicked: root.mapState = "driveStarted"
        }
    }
}

