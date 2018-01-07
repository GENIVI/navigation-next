import QtQuick 2.0
import QtQuick.Controls 2.0

import utils 1.0

Button {
    id: control
    checkable: true
    property alias buttonText: control.text

    property bool check: control.checked
    property string imageSource: Style.symbol("acautosync-knob-up")

    contentItem:  Item {
        Text {
            anchors.centerIn: parent
            text: control.text
            color: Style.colorWhite
            font.pixelSize: Style.fontSizeXXS
        }
    }

    background: Image {
        id: backgroundImage
        source: imageSource
    }
    onCheckChanged: {
        if (checked)
            imageSource = Style.symbol("acautosync-knob-down")
        else
            imageSource =  Style.symbol("acautosync-knob-up")
    }
}
