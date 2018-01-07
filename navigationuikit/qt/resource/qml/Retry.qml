import QtQuick 1.1

Rectangle {
    id: navigationrequestretrywiget
    width: anchors.leftMargin + 300;
    height: anchors.topMargin + 100
    color: "#e5edf1"

    Style{
        id: fontStyle
    }

    Text {
        id: navigationrequestretry
        x: 20
        y: 13
        width: 256
        height: 16
        text: controller.Message
        anchors.horizontalCenter: parent.horizontalCenter
        font.family: fontStyle.defaultFontName
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 14
    }

    signal navrequestretrysignal()
    signal navrequestcancelsignal()

    MouseArea {
        id: navrequestretry
        x: 22
        y: 40
        width: 119
        height: 49
        clip: false
        scale: 1
        enabled: true
        hoverEnabled: true
        opacity: 1

        Text {
            id: shownavrequestretry
            x: 16
            y: 14
            width: 89
            height: 20
            text: controller.Confirm
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 12
        }

        onClicked:{
            navigationrequestretrywiget.navrequestretrysignal();
        }
    }

    MouseArea {
        id: navrequestcancel
        x: 155
        y: 40
        width: 123
        height: 49
        hoverEnabled: true

        Text {
            id: shownavrequestcancel
            x: 17
            y: 13
            width: 85
            height: 23
            text: controller.Cancel
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 12
        }

        onClicked:{
            navigationrequestretrywiget.navrequestcancelsignal();
        }
    }
}
