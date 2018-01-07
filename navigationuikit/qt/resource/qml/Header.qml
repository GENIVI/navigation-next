import QtQuick 1.1

Rectangle{
    id: header
    width: 568/*turn.width + street.width + exit.width + 80*/
    height: 140/*turn.height + 30*/
    radius: 5
    color: "white"

    Style{
        id: fontStyle
    }

    Item{
        id: turn
        width: turnText.width > distanceText.width ? turnText.width : distanceText.width
        height: distanceText.height + turnText.height
        anchors{
            left: parent.left
            leftMargin: 12
            top: parent.top
            topMargin: 12
        }

        Text {
            id: distanceText
            objectName: "distance"
            anchors.left: parent.left
            anchors.top: parent.top
            text: qsTr("88.8 mi")
            font.pointSize: 22
            font.letterSpacing: 1
            font.family: fontStyle.defaultFontName
            style: Text.Raised
            color: "black"
            elide: Text.ElideLeft
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        Text {
            id: turnText
            objectName: "turn"
            anchors{
                left: parent.left
                top: distanceText.bottom
            }

            text: qsTr("a")
            font{
                pointSize: 50
                letterSpacing: 1
                family: "LG NexGen"
            }
            style: Text.Raised
            elide: Text.ElideLeft
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    Item{
        id: street
        width: streetText.width
        height: streetText.height + secondaryText.height

        anchors{
            left: turn.right
            leftMargin: 12
            top: parent.top
            topMargin: 20
        }

        Text {
            id: streetText
            objectName: "primaryStreet"
            anchors.left: parent.left
            anchors.top: parent.top
            text: qsTr("Single Line Street")
            font.pointSize: 30
            font.letterSpacing: 1
            font.family: fontStyle.defaultFontName
            style: Text.Raised
            color: "black"
            elide: Text.ElideLeft
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignBottom
        }

        Text {
            id: secondaryText
            objectName: "secondaryStreet"
            anchors.left: parent.left
            anchors.top: streetText.bottom
            text: qsTr("Secondary Info")
            font.pointSize: 22
            font.letterSpacing: 1
            font.family: fontStyle.defaultFontName
            style: Text.Raised
            color: "grey"
            elide: Text.ElideLeft
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignTop
        }
    }

    Text{
        id: exit
        objectName: "exit"
        anchors{
            right: parent.right
            rightMargin: 12
            top: parent.top
            topMargin: 12
        }

        text: qsTr("ext 8")
        font.pointSize: 20
        font.family: fontStyle.defaultFontName
        style: Text.Raised
        color: "grey"
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignTop
        visible: false
    }
}
