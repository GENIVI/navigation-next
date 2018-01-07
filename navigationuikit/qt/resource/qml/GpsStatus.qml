import QtQuick 1.1

Rectangle{
    id: gpsStatus
// @warning hardcode: width height
    width: gpsStatusText.width + 20
    height: gpsStatusText.height + 10
    color: "#343434"
    radius: 3

    Style{
        id: fontStyle
    }

    Text {
        id: gpsStatusText
        objectName: "gpsStatus"
// @warning hardcode: anchors margin
        anchors{
            left: parent.left
            top: parent.top
            leftMargin: 12
            topMargin: 5
        }

        text: qsTr("No GPS")
        font.pointSize: 21
        font.letterSpacing: 1
        font.family: fontStyle.defaultFontName
        style: Text.Raised
        color: "white"
        elide: Text.ElideLeft
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}

