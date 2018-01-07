import QtQuick 2.0

Item {
    width: 80
    height: 85
    property alias icon: buttonHolderButton.icon
    property alias iconPressed: buttonHolderButton.iconPressed
    property alias text: buttonHolderText.text
    property alias blinkingEnabled: buttonHolderButton.blinkingEnabled
    property alias pressed: buttonHolderButton.pressed
    property alias mouseEnabled: buttonHolderButton.mouseEnabled

    function synchronize() {
        buttonHolderButton.synchronize()
    }

    Button {
        id: buttonHolderButton
    }

    Text {
        id: buttonHolderText
        anchors {
            top: buttonHolderButton.bottom
            topMargin: 7
            horizontalCenter: parent.horizontalCenter
        }
        font.pixelSize: 12
    }
}
