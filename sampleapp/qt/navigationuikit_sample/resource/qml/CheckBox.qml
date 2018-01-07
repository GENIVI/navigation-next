// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

FocusScope {
    id: checkbox

    //Accessible.role: Accessible.CheckBox

    property string text: "CheckBox"
    property bool checked // required variable

    property int optionAvoidTollRoads: 1
    property int optionAvoidFerries: 2
    property int optionAvoidCarpoolLanes: 3
    property int optionAvoidHighways: 4
    property int optionAvoidUnpaved: 5

    property int layerFootPrints: 6
    property int layerWeather: 7
    property int layerLandMark: 8
    property int layerPointOfInterest: 9
    property int layerSatellite: 10
    property int layerTraffic: 11

    property int optionType
    property int fontSize: 18

    width: boxRect.width + boxText.width + boxRow.spacing
    height: boxRow.height

    Row {
        id: boxRow
        spacing: 12

        Rectangle {
            id: boxRect
            width: 24
            height: 24
            color: "black"
            border.width: checkbox.focus ? 2 : 1
            border.color: "white"

            Text {
                id: checkboxText
                text: checkbox.checked ? "√" : ""
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 24
                anchors.centerIn: parent
                color: "white"
            }
        }

        Text {
            id: boxText
            text: checkbox.text
            font.pointSize: checkbox.fontSize
            color: "white"
            anchors.baseline: boxRect.bottom
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            checkbox.checked = !checkbox.checked

            switch( checkbox.optionType )
            {
            case checkbox.optionAvoidTollRoads:
                optionModel.avoidTollRoads = checkbox.checked
                break;
            case checkbox.optionAvoidFerries:
                optionModel.avoidFerries = checkbox.checked
                break;
            case checkbox.optionAvoidCarpoolLanes:
                optionModel.avoidCarpoolLanes = checkbox.checked
                break;
            case checkbox.optionAvoidHighways:
                optionModel.avoidHighways = checkbox.checked
                break;
            case checkbox.optionAvoidUnpaved:
                optionModel.avoidUnpaved = checkbox.checked
                break;

            case checkbox.layerFootPrints:
                layerOptionModel.footprintsChecked = checkbox.checked
                break;
            case checkbox.layerWeather:
                layerOptionModel.weatherChecked = checkbox.checked
                break;
            case checkbox.layerLandMark:
                layerOptionModel.landmarksChecked = checkbox.checked
                break;
            case checkbox.layerPointOfInterest:
                layerOptionModel.pointsOfInterestChecked = checkbox.checked
                break;
            case checkbox.layerSatellite:
                layerOptionModel.satelliteChecked = checkbox.checked
                break;
            case checkbox.layerTraffic:
                layerOptionModel.trafficChecked = checkbox.checked
                break;
            }

        }
    }

    Keys.onSpacePressed: checkbox.checked = !checkbox.checked
}
