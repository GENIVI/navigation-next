import QtQuick 2.0

import utils 1.0
import service.movie 1.0
import controls 1.0
import "."

Item {
    id: musicLibrary
    height: Style.launchBarHeight / 2
    width: 512
    anchors.margins: Style.padding
    anchors.rightMargin: Style.paddingXXL
    property alias model: repeater.model
    property string modelType


    Image {
        source: Style.symbol("RectangleGradient256x512")
    }

    Image {
        z: 2
        source: Style.symbol("MusicOverlayGradient2")
    }

    Label {
        id: title
        z: 3
        text: modelType
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: Style.paddingXXL
    }

    Grid {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        rows: 2
        columns: 4
        Repeater {
            id: repeater

            delegate: Item {
                width: musicLibrary.width/4
                height: musicLibrary.height/2
                Image {
                    source: modelType === "Music" ? MusicProvider.coverPath(model.cover) : MovieService.coverPath(model.cover)
                    anchors.fill: parent
                }
            }
        }
    }
    MouseArea {
        anchors.fill: parent
        onClicked:
            root.modelType = modelType
    }
}
