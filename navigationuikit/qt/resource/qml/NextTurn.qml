import QtQuick 1.1

Rectangle{
    id: nextturn

    width: then.width > turn.width ? then.width : turn.width
    height: then.height + turn.height
    radius: 5
    color: "white"

    Style{
        id: fontStyle
    }

    Text {
        id: then
        anchors{
            left: parent.left
            top: parent.top
            topMargin: 2
            leftMargin: 14
        }

        text: qsTr("THEN")
        font{
            pointSize: 15
            letterSpacing: 1
            family: fontStyle.defaultFontName
        }
        style: Text.Raised
        color: "grey"
        elide: Text.ElideLeft
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    Text {
        id: turn
        objectName: "turn"
        anchors{
            left: parent.left
            bottom: parent.bottom
        }

        text: qsTr("a")
        font{
            pointSize: 50
            letterSpacing: 1
            family: "LG NexGen"
        }
        style: Text.Raised
        color: "blue"
        elide: Text.ElideLeft
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
