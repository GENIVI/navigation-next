import QtQuick 1.1

Rectangle {
    id: navigationstatusbar
    width: anchors.leftMargin + 300;
    height: anchors.topMargin + 50

    Text {
        id: shownavigationstatus
        x: 8
        y: 8
        width: 300
        height: 50
        text: "Navigation:" + navstatuscontroller.GetNavigationStatusString();
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        font.family: "Times New Roman"
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: 14
    }
}
