import QtQuick 1.1

Rectangle {
    width: anchors.leftMargin + 200;
    height: anchors.topMargin + 100

    Image {
        id: image1
        x: 0
        y: 0
        width: 200
        height: 100
        source: "../images/bubble_left_up_off.png"

        Text {
            id: text1
            x: 54
            y: 27
            width: 119
            height: 31
            text: pinbubbletextcontroller.getBubbleText();
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenterOffset: 12
            anchors.horizontalCenter: parent.horizontalCenter
            font.family: "Times New Roman"
            font.pixelSize: 14
        }
    }
}
