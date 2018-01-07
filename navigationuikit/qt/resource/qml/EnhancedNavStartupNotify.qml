import QtQuick 1.1

Rectangle{
    id: enhancedNavStartupNotify
    width: 380
    height: 100
    color: "#343434"
    radius: 0

    Style{
        id: fontStyle
    }

    Text {
        id: enhancedNavStartupNotifyText
        objectName: "enhancedNavStartupNotify"
        text: qsTr("EnhancedNavStartup Notify bbbb bbbbbb bbbbbbbbbbbbb bbbbbb bbbbb bbbbb")
        anchors.right: parent.right
        anchors.rightMargin: 5
        anchors.left: parent.left
        anchors.leftMargin: 5
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 5
        anchors.top: parent.top
        anchors.topMargin: 5
        wrapMode: Text.WordWrap
        font.pointSize: 11
        font.letterSpacing: 1
        style: Text.Raised
        color: "white"
        elide: Text.ElideNone
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }
}

