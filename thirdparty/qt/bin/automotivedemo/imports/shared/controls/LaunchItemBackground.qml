import QtQuick 2.0

import QtApplicationManager 1.0
import utils 1.0

Item {
    property alias text: title.text
    property string applicationName
    scale: area.pressed?0.85:1.0

    Behavior on scale {
        NumberAnimation {}
    }

    Image {
        source: Style.symbol("RectangleGradient256x512")
        anchors.fill: parent
    }

    Label {
        id: title
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: Style.paddingXXL
    }

    MouseArea {
        id: area
        anchors.fill: parent
        onClicked: {
            ApplicationManager.startApplication(applicationName)
        }
    }
}
