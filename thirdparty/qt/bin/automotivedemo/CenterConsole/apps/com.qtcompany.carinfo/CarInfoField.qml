import QtQuick 2.0

import controls 1.0
import utils 1.0

Column {
    spacing: 20

    property string value: ""
    property string title: ""
    property string unit: "KM"
    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        Label {
            text: value
            font.pixelSize: Style.fontSizeL
        }
        Label {
            text: unit
            font.pixelSize: Style.fontSizeXXS
        }
    }
    Label {
        text: title
        font.pixelSize: Style.fontSizeXXS
        horizontalAlignment: Text.AlignHCenter
    }
}
