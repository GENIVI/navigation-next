import QtQuick 2.0

import utils 1.0
import service.valuesource 1.0

Item {
    property string location: ValueSource.destination
    property string address: "Ernst-Reuter-Platz 1-2"
    property string image
    width: Style.screenWidth
    height: Style.cellHeight
    Row {
        id: suggestedPlace

        spacing: Style.paddingXXL
        Image{
            id: icon
            source: Style.symbol(image)
        }
        Column  {
            anchors.verticalCenter: icon.verticalCenter
            Text{
                text: location
                font.pixelSize: Style.fontSizeL
                color: Style.colorWhite
            }
            Text{
                text: address
                font.pixelSize: Style.fontSizeXS
                color: "#8E9399"
            }
        }
    }
    MouseArea {
        anchors.fill: parent
        onClicked: {
            root.mapState = "showRoute"
            Qt.inputMethod.hide()
        }
    }
}
