// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Rectangle {
    color: "black"

    signal layerOptionWidgetClosed()
    width: 420
    height: layout.height + 40

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
                layerOptionWidgetClosed();
            }
        }
    }

    Grid{
        id: layout
        columns: 1
        spacing: box1.height * 0.5
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        CheckBox{
            id: box1
            text: qsTr("Building Footprints")
            optionType: box1.layerFootPrints
            checked: layerOptionModel.footprintsChecked
            visible: layerOptionModel.footprintsVisible
        }
        CheckBox{
            id: box2
            text: qsTr("Weather")
            optionType: box2.layerWeather
            checked: layerOptionModel.weatherChecked
            visible: layerOptionModel.weatherVisible
        }
        CheckBox{
            id: box3
            text: qsTr("3D Landmarks & Buildings")
            optionType: box3.layerLandMark
            checked: layerOptionModel.landmarksChecked
            visible: layerOptionModel.landmarksVisible
        }
        CheckBox{
            id: box4
            text: qsTr("Points of Interest")
            optionType: box4.layerPointOfInterest
            checked: layerOptionModel.pointsOfInterestChecked
            visible: layerOptionModel.pointsOfInterestVisible
        }
        CheckBox{
            id: box5
            text: qsTr("Satellite")
            optionType: box4.layerSatellite
            checked: layerOptionModel.satelliteChecked
            visible: layerOptionModel.satelliteVisible
        }
        CheckBox{
            id: box6
            text: qsTr("Traffic")
            optionType: box4.layerTraffic
            checked: layerOptionModel.trafficChecked
            visible: layerOptionModel.trafficVisible
        }
    }

}
