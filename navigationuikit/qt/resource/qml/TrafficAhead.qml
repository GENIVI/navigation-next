// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Rectangle{
    id: trafficAhead
    width: 724
    height: 110
    color: "red"
    opacity: 0.8

    signal closeSignal()
    signal detourSignal()

    Style{
        id: fontStyle
    }

    Item{
        id: detour

        anchors{
            left: parent.left
            top: parent.top
        }

        width: parent.width
        height: parent.height

        Text {
            id: trafficText
            anchors{
                left: parent.left
                leftMargin: 15
                top: parent.top
                topMargin: 10
            }
            width: parent.width
            text: qsTr("Heavy Traffic Ahead")
            font.pointSize: 14
            font.letterSpacing: 1
            font.family: fontStyle.defaultFontName
            style: Text.Raised
            color: "white"
            elide: Text.ElideLeft
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        Text {
            id: detourText
            anchors{
                left: parent.left
                leftMargin: 15
                top: trafficText.bottom
                topMargin: 10
            }
            width: parent.width
            text: qsTr("Tap to Show Detour")
            font.pointSize: 14
            font.bold: true
            font.family: fontStyle.defaultFontName
            style: Text.Raised
            color: "grey"
            elide: Text.ElideLeft
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        Item{
            id: maneuver
            anchors{
                right: parent.right
                rightMargin: 15
                top: parent.top
                topMargin: 25
            }
            BorderImage {
                id: maneuverImage
                source: ""
                border.left: 5;
                border.top: 5
                border.right: 5;
                border.bottom: 5
            }

            Text{
                id: maneuverText
                objectName: "maneuver"
                anchors{
                    right: parent.right
                    top: parent.top
                }

                text: qsTr("D")
                font{
        //                pointSize: 50
                    pixelSize: 50
                    letterSpacing: 1
                    family: "LG NexGen"
                }
                color: "#ffffff"
                style: Text.Raised
                elide: Text.ElideLeft
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                detourSignal();
            }
        }
    }

    Image{
        id: close
        source: "../images/close_x.png"
        anchors{
            left: parent.left
            leftMargin: 15
            top: parent.top
            topMargin: 40
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                closeSignal();
            }
        }
    }
}
