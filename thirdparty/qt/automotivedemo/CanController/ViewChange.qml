import QtQuick 2.0

Rectangle{
    id: viewButton
    height: 60
    radius: 5
    border.width: 2
    color: pressed ? blinkColor : "white"

    property alias text: buttonText.text
    property bool mainView: true
    property bool pressed: false
    property bool optionalIcons: false

    property color blinkColor: "#26282a"
    property color textBlinkColor: "#d6d6d6"

    property url iconOn: optionalIcons ? "qrc:/CenterViewOnOpt.png" : "qrc:/CenterViewOn.png"
    property url iconOff: optionalIcons ? "qrc:/CenterViewOffOpt.png" : "qrc:/CenterViewOff.png"

    onPressedChanged: {
        // TODO: Hack, use oilTemp/oilPressure CAN messages for view change
        if (mainView)
            clusterDataControl.oilTemp = pressed ? 10 : 0
        else
            clusterDataControl.oilPressure = pressed ? 10 : 0
    }

    MouseArea {
        id: buttonMouse
        anchors.fill: parent
        onPressed: {
            buttonText.color = textBlinkColor
            viewButton.pressed = !viewButton.pressed
        }
        onReleased: {
            buttonText.color = blinkColor
            viewButton.pressed = !viewButton.pressed
        }
    }

    Column {
        spacing: 10

        anchors.centerIn: parent
        Image {
            source: viewButton.pressed ? iconOff : iconOn
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Text {
            id: buttonText
            anchors.horizontalCenter: parent.horizontalCenter
            font.pixelSize: 14
            color: "#26282a"
        }
    }
}
