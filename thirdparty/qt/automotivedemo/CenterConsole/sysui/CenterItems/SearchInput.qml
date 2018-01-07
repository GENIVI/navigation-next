import QtQuick 2.7
import QtQuick.VirtualKeyboard 2.1
import QtQuick.VirtualKeyboard.Settings 2.1

import utils 1.0
import controls 1.0

Item {
    id: root
    width: Style.screenWidth
    property int panelHeight: panel.height
    Rectangle {
        id: searchInput
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: panel.top
        anchors.bottomMargin: 10
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        radius: Style.buttonRadius
        height: Style.searchInputHeight

        TextInput {
            id: textInput
            width: parent.width
            color: Style.colorBlack
            focus: true
            //hide cursor
            cursorDelegate: Item {}
            horizontalAlignment: TextInput.AlignHCenter
            font.pixelSize: Style.fontSizeS
            Keys.onReleased: {
                if (event.key === Qt.Key_Return) {
                    root.mapState = "showRoute"
                    textInput.clear()
                }
            }

            MouseArea {
                id: focusMouseArea
                anchors.fill: parent
                onClicked: {
                    textInput.forceActiveFocus()
                    Qt.inputMethod.show()
                }
            }
        }
        Label {
            id: previewText
            z: 10
            color: "#a0a1a2"
            visible: !inputPanel.active
            text: "Where would you like to go?"
            font.pixelSize: Style.fontSizeM
            anchors.centerIn: parent
        }
    }
    Image {
        id: closeButton
        anchors.bottom: searchInput.top
        anchors.right: searchInput.right
        anchors.bottomMargin: Style.padding
        source: Style.symbol("cancel")
        visible: inputPanel.active
        MouseArea {
            anchors.fill: parent
            onClicked: Qt.inputMethod.hide()
        }
    }

    Rectangle {
        id: panel
        color: "#262a32"
        opacity: 0.95
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: inputPanel.top
        implicitHeight: childrenRect.height + 50
        Row {
            id: poiRow
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: Style.paddingXL
            spacing: width / 7
            MapPOI { poiText: "Charging stations"; poiIcon: "placeofinterest-charging" }
            MapPOI { poiText: "Parking"; poiIcon: "placeofinterest-parking" }
            MapPOI { poiText: "Coffee"; poiIcon: "placeofinterest-coffee" }
            MapPOI { poiText: "Shopping"; poiIcon: "placeofinterest-shopping" }
        }
        Column {
            anchors.top: poiRow.bottom
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.margins: 40
            Label {
                font.pixelSize: Style.fontSizeL
                text: "SUGGESTED PLACES"
            }

            spacing: 30
            Rectangle { width: parent.width; height: 2; color: "#61656C" }
            SuggestedPlaces { image: "destination-airport" }
            Rectangle { width: parent.width; height: 2; color:"#61656C" }
            SuggestedPlaces { image: "destination-home" }
            Rectangle { width: parent.width; height: 2; color: "#61656C" }
            SuggestedPlaces { image: "destination-work" }
            Rectangle { width: parent.width; height: 2; color: "#61656C" }
        }
    }

    /*  Handwriting input panel for full screen handwriting input.

        This component is an optional add-on for the InputPanel component, that
        is, its use does not affect the operation of the InputPanel component,
        but it also can not be used as a standalone component.

        The handwriting input panel is positioned to cover the entire area of
        application. The panel itself is transparent, but once it is active the
        user can draw handwriting on it.
    */
    HandwritingInputPanel {
        z: 79
        id: handwritingInputPanel
        anchors.fill: parent
        inputPanel: inputPanel
        Rectangle {
            z: -1
            anchors.fill: parent
            color: "black"
            opacity: 0.10
        }
    }

    /*  Container area for the handwriting mode button.

        Handwriting mode button can be moved freely within the container area.
        In this example, a single click changes the handwriting mode and a
        double-click changes the availability of the full screen handwriting input.
    */
    Item {
        z: 89
        visible: handwritingInputPanel.enabled && Qt.inputMethod.visible
        anchors { left: parent.left; top: parent.top; right: parent.right; bottom: inputPanel.top; }
        HandwritingModeButton {
            id: handwritingModeButton
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 10
            floating: true
            flipable: true
            width: 76
            height: width
            state: handwritingInputPanel.state
            onPressed: handwritingInputPanel.active = !handwritingInputPanel.active
            onDoubleClicked: handwritingInputPanel.available = !handwritingInputPanel.available
        }
    }

    InputPanel {
        id: inputPanel
        anchors.left: parent.left
        anchors.right: parent.right
        visible: active

        states: State {
            name: "visible"
            /*  The visibility of the InputPanel can be bound to the Qt.inputMethod.visible property,
                    but then the handwriting input panel and the keyboard input panel can be visible
                    at the same time. Here the visibility is bound to InputPanel.active property instead,
                    which allows the handwriting panel to control the visibility when necessary.
                */
            when: inputPanel.active
            PropertyChanges {
                target: inputPanel
                y: root.height - inputPanel.height - panel.height
            }
        }
        transitions: Transition {
            from: ""
            to: "visible"
            reversible: true
            ParallelAnimation {
                NumberAnimation {
                    properties: "y"
                    duration: 500
                    easing.type: Easing.InOutQuad
                }
            }
        }
        Component.onCompleted: {
            VirtualKeyboardSettings.styleName = "automotive"
        }
    }

}
