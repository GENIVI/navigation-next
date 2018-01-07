import QtQuick 2.7
import models 1.0
import utils 1.0

Item {
    width: Style.screenWidth
    height: Style.screenHeight
    ListView {
        id: stack
        anchors.fill: parent
        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem
        highlightMoveDuration: 300
        interactive: false
        currentIndex: 0
        clip: true
        model: VisualItemModel {
            WidgetContainer {
                id: navContainer
                width: stack.width
                height: stack.height
                title: "NAVIGATOR"
            }
            WidgetContainer {
                id: musicContainer
                anchors.centerIn: parent
                width: stack.width
                height: stack.height
                title: "MUSIC"
            }
        }
    }
    Connections {
        target: ApplicationManagerInterface
        onPassengerWidgetReady: {
            var container;
            if (category === "media") {
                container = musicContainer
                stack.currentIndex = 1
            } else if (category === "navigation") {
                container = navContainer
                stack.currentIndex = 0
            } else {
                container = otherContainer
                stack.currentIndex = 2
            }

            item.parent = container
            container.content = item
            item.width = container.width
            item.height = container.height
        }
    }
}
