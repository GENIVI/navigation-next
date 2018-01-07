import QtQuick 2.0

import utils 1.0
import controls 1.0

Column {
    property string arText: arrival.text
    property string text2: arrival2.text

    Label {
        id: arrival
        text: arText
    }
    Label {
        id: arrival2
        text: text2
        anchors.horizontalCenter: arrival.horizontalCenter
    }
}
