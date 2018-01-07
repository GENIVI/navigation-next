import QtQuick 2.0

Rectangle {
    id: buttonMainRect
    width: 80
    height: 50
    radius: 5
    border.width: 2

    property color blinkColor: "#26282a"
    property color textBlinkColor: "#d6d6d6"

    // color: (buttonMainRect.blink ? "pink" : "white")

    property alias text: buttonText.text
    property alias textColor: buttonText.color
    property alias fontPixelSize: buttonText.font.pixelSize
    property string icon
    property string iconPressed
    property bool pressed: false
    property bool blink: false
    property bool blinkingEnabled: false
    property alias mouseEnabled: buttonMouse.enabled
    function synchronize() {
        blinkTimer.restart()
    }

    onPressedChanged: {
        if (buttonMainRect.pressed) {
            if (buttonMainRect.blinkingEnabled) {
                blinkTimer.start()
            } else {
                buttonMainRect.color = buttonMainRect.blinkColor
                buttonText.color = buttonMainRect.textBlinkColor
            }
        }
        else {
            if (buttonMainRect.blinkingEnabled)
                blinkTimer.stop()
            buttonMainRect.color = blinkTimer.baseColor
            buttonText.color = blinkTimer.textBaseColor
        }
    }

    Text {
        id: buttonText
        anchors.centerIn: parent
        visible: !buttonImage.visible
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 14
        color: "#26282a"
    }

    Image {
        id: buttonImage
        visible: (buttonImage.source != "")
        anchors.centerIn: parent
        source: (buttonMainRect.pressed ? buttonMainRect.iconPressed : buttonMainRect.icon)
    }

    MouseArea {
        id: buttonMouse
        anchors.fill: parent
        onClicked: buttonMainRect.pressed = !buttonMainRect.pressed
    }

    Timer {
        id: blinkTimer
        repeat: true
        onRunningChanged: {
            if (!blinkTimer.running && buttonMainRect.blink)
                buttonMainRect.blink = false
        }
        property color baseColor
        property color textBaseColor
        Component.onCompleted: {
            blinkTimer.baseColor = buttonMainRect.color
            blinkTimer.textBaseColor = buttonText.color
        }
        interval: 300
        onTriggered: {
            buttonMainRect.blink = !buttonMainRect.blink
            if (buttonMainRect.blink) {
                buttonMainRect.color = buttonMainRect.blinkColor
                buttonText.color = buttonMainRect.textBlinkColor
            } else {
                buttonMainRect.color = blinkTimer.baseColor
                buttonText.color = blinkTimer.textBaseColor
            }
        }
    }
}
