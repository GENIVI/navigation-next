// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Rectangle{
    id: routeStatus
    width: 1024
    height: 110
    color: "#115783"
    opacity: 0.9

    Style{
        id: fontStyle
    }

    Text {
        id: statusText
        objectName: "status"
        width: parent.width
        height: parent.height
        anchors{
            left: parent.left
            top: parent.top
        }
        text: qsTr("Getting...")
        font.pointSize: 24
        font.bold: true
        font.family: fontStyle.defaultFontName
        style: Text.Raised
        color: "#ffffff"
        elide: Text.ElideLeft
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
