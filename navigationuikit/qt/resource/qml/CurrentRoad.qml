import QtQuick 1.1

Rectangle{
    id: currentRoad

    color: "#343434"
    radius: 3

    Style{
        id: fontStyle
    }

    Text {
        id: currentRoadText
        objectName: "currentRoad"
        width: parent.width
        height: parent.height
        anchors{
            left: parent.left
            top: parent.top
        }

        text: qsTr("Current Road Name")
        font.pointSize: 21
        font.family: fontStyle.defaultFontName
        style: Text.Raised
        color: "white"
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
