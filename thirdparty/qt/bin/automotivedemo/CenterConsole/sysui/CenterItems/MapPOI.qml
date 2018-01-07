import QtQuick 2.7
import utils 1.0
import controls 1.0

Column {
    property string poiText: "Shopping"
    property string poiIcon: "placeofinterest-shopping"
    spacing: Style.paddingL

    Rectangle {
        id: image
        implicitWidth: poiImage.width * 2
        implicitHeight: implicitWidth
        radius: implicitWidth
        color: Style.colorPink
        Image {
            id: poiImage
            anchors.centerIn: parent
            source: Style.symbol(poiIcon)
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                root.mapState = "showRoute"
                Qt.inputMethod.hide()
            }
        }
    }
    Label {
        text: poiText
        anchors.horizontalCenter: image.horizontalCenter
        font.pixelSize: Style.fontSizeXS
    }
}
