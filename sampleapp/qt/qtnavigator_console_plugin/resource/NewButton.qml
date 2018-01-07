import QtQuick 2.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

Item {
    id: newbutton

    property alias text: buttonText.text

    signal clicked

    implicitWidth: buttonText.implicitWidth + 150
    implicitHeight: buttonText.implicitHeight + 15

    Button {
        id: buttonText
        width: parent.width
        height: parent.height

        style: ButtonStyle {
            background: Rectangle {
                color: "transparent"
                //implicitWidth: 200
                //implicitHeight: 50
                border.width: 2
                border.color: "#8FED31"
                radius: 4
            }
            label: Component{
                Text {
                    text: buttonText.text
                    font.pixelSize: 24
                    font.family: "DejaVu Sans"
                    anchors.centerIn: parent
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    anchors.fill: parent
                    color: "#8FED31"
                    wrapMode: Text.WordWrap
                    clip: true
                }
            }
        }
        onClicked: newbutton.clicked()
    }
}
