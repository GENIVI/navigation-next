// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Rectangle {
    id: routeOption
    property int fontSize: 18
    width: layout.width + 40
    height: layout.height + 20
    color: "black"
    radius: 5

    signal routeOptionWidgetClosed()

    Image {
        id: closeimage
        source: "../images/close.png"

        anchors{
            top: parent.top
            topMargin: 6
            right: parent.right
            rightMargin: 6
        }

        MouseArea{
            anchors.fill: parent
            onClicked: {
                routeOptionWidgetClosed();
            }
        }
    }

    Column{
        id: layout
        spacing: 20
        width: checkboxgrid.width
        height: title.height + checkboxgrid.height + spacing
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        Text {
            id: title
            text: qsTr("Avoid")
            font.pointSize: routeOption.fontSize
            font.bold: true
            font.italic: true
            color: "white"
            anchors.left: checkboxArea.left
        }

        Grid{
            id: checkboxgrid
            columns: 2
            spacing: box1.height * 0.5

            CheckBox{
                id: box1
                text: qsTr("Toll Roads")
                optionType: box1.optionAvoidTollRoads
                checked: optionModel.avoidTollRoads
            }
            CheckBox{
                id: box2
                text: qsTr("Ferries")
                optionType: box2.optionAvoidFerries
                checked: optionModel.avoidFerries
            }
            CheckBox{
                id: box3
                text: qsTr("Carpool Lanes")
                optionType: box3.optionAvoidCarpoolLanes
                checked: optionModel.avoidCarpoolLanes
            }
            CheckBox{
                id: box4
                text: qsTr("Highways")
                optionType: box4.optionAvoidHighways
                checked: optionModel.avoidHighways
            }
            CheckBox{
                id: box5
                text: qsTr("Unpaved Ways")
                optionType: box5.optionAvoidUnpaved
                checked: optionModel.avoidUnpaved
            }
        }
    }

}
