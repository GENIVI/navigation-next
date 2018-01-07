import QtQuick 2.0
import QtQuick.Controls 2.0
import QtGraphicalEffects 1.0

import utils 1.0

Item {
    property real scaleFactor: 1.5
    height: Style.bottomBarHeight
    width: Style.tumblerWidth
    property alias index: tempLevel.currentIndex
//    RadialGradient {
//        anchors.fill: parent
//        angle: 45
//        horizontalRadius: 100
//        verticalRadius: 80
//        gradient: Gradient {
//            GradientStop { position: 0.0; color: Style.colorRed }
//            GradientStop { position: 0.8; color: Style.darkGrey }
//        }
//    }

    Tumbler {
        id: tempLevel
        z: 0
        model: 8
        anchors.fill: parent
        visibleItemCount: 3

        contentItem: ListView {
            anchors.fill: parent
            model: tempLevel.model
            delegate: tempLevel.delegate
            snapMode: ListView.SnapToItem
            highlightRangeMode: ListView.StrictlyEnforceRange
            preferredHighlightBegin: height / 2 - (height / tempLevel.visibleItemCount / 2)
            preferredHighlightEnd: height / 2  + (height / tempLevel.visibleItemCount / 2)
            clip: true
        }
        delegate: Label {
            text: ("%1 °").arg(modelData + 18)
            font.pixelSize: Style.fontSizeL
            scale: ListView.isCurrentItem ? scaleFactor : 1
            horizontalAlignment: Text.AlignHCenter
            opacity: 0.4 + Math.max(0, 1 - Math.abs(Tumbler.displacement)) * 0.6
            color: Style.colorWhite
            Behavior on scale {
                NumberAnimation { duration: 200 }
            }
        }
    }
}
