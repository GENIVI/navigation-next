import QtQuick 1.1

Rectangle {
    id:endtrip

    color: "#343434"
    radius: 3
    border.color: "white"
    border.width: 2

    signal endTripSignal(bool endtrip)

    Style{
        id: fontStyle
    }

    Text {
        id: confirm
        width: parent.width
        anchors{
            left: parent.left
            top: parent.top
            topMargin: 20
        }

        text: endtripcontroller.MessageContent
        font.pointSize: 21
        font.family: fontStyle.defaultFontName
        style: Text.Raised
        color: "white"
        elide: Text.ElideLeft
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    Rectangle{
        id: yesbutton
        width: 150
        height: 50
        border.color: "white"
        border.width: 2
        color: parent.color

        anchors{
            left: parent.left
            leftMargin: 50
            bottom: parent.bottom
            bottomMargin: 20
        }

        Text{
            anchors{
                left: parent.left
                top: parent.top
            }

            width: parent.width
            height: parent.height

            text: endtripcontroller.ConfirmButtonText
            font.pointSize: 21
            font.family: fontStyle.defaultFontName
            style: Text.Raised
            color: "white"
            elide: Text.ElideLeft
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        MouseArea{
            anchors.fill: parent
            onClicked: {
                endtrip.endTripSignal(true);
            }
        }
    }

    Rectangle{
        id: nobutton
        width: yesbutton.width
        height: yesbutton.height
        border.color: "white"
        border.width: 2
        color: parent.color

        anchors{
            right: parent.right
            rightMargin: yesbutton.anchors.leftMargin
            bottom: parent.bottom
            bottomMargin: yesbutton.anchors.bottomMargin
        }

        Text{
            anchors{
                left: parent.left
                top: parent.top
            }

            width: parent.width
            height: parent.height

            text: endtripcontroller.CancelButtonText
            font.pointSize: 21
            font.family: fontStyle.defaultFontName
            style: Text.Raised
            color: "white"
            elide: Text.ElideLeft
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        MouseArea{
            anchors.fill: parent
            onClicked: {
                endtrip.endTripSignal(false);
            }
        }
    }
}
