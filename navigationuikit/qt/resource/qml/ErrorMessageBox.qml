import QtQuick 1.1

Rectangle {
    id: rootWindow
    width: 600
    height: boxArea.height + 20
    color: "black"

    signal errorMessageOK()

    Rectangle{
        id: boxArea
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        color: parent.color
        width: textAndbutton.width
        height: textAndbutton.height

        Column{
            id: textAndbutton
            spacing: 18
            Text {
                id: text
                wrapMode: Text.Wrap
                width: rootWindow.width - 40
                text: errorMessageModel.text
                font.pointSize: 24
                color: "white"
            }

            Rectangle{
                id: button
                width: 100
                height: 40
                color: boxArea.color
                border.width: 2
                border.color: "white"
                anchors.horizontalCenter: parent.horizontalCenter

                Text {
                    text: qsTr("OK")
                    color: "white"
                    font.pointSize: 20
                    anchors.centerIn: parent
                }

                MouseArea{
                    anchors.fill: parent
                    onClicked: errorMessageOK()
                }
            }

        }

    }

}
