import QtQuick 2.3
import QtQuick.Window 2.2
import QtQuick.Layouts 1.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import "style" 1.0

Item{
    Rectangle{
        id: textFields
        //height: 150
        Layout.fillWidth: true
        width: parent.width
        height: gridLayout.implicitHeight
        color: DefaultStyle.colorTransparent //"transparent"
        anchors.top: parent.top
        anchors.topMargin: titleMargin

        GridLayout {
            id: gridLayout
            rows: 3
            flow: GridLayout.TopToBottom
            anchors.fill: parent
            rowSpacing: DefaultStyle.paddingS //10

            Label {
                text: "Search Radius: "
                objectName: "lblRadius"
                font {
                    family: DefaultStyle.fontMedium.name //sfDisplayRegular.name
                    pixelSize: DefaultStyle.fontSize21 //22
                }
                color: DefaultStyle.colorWhite //"#FFFFFF"
            }
            Label {
                text: "Current Latitude: "
                objectName: "lblLatitude"
                font {
                    family: DefaultStyle.fontMedium.name //sfDisplayRegular.name
                    pixelSize: DefaultStyle.fontSize21 //22
                }
                color: DefaultStyle.colorWhite //"#FFFFFF"
            }
            Label {
                text: "Current Longitude: "
                objectName: "lblLongitude"
                font {
                    family: DefaultStyle.fontMedium.name //sfDisplayRegular.name
                    pixelSize: DefaultStyle.fontSize21 //22
                }
                color: DefaultStyle.colorWhite //"#FFFFFF"
            }

            TextField {
                //property string radiusLineEdit: text
                id:radiusLineEdit;
                objectName: "radiusLineEdit";
                placeholderText: "Enter radius";
                Layout.fillWidth: true;
                style: textFieldStyleView

            }
            TextField { id:latitudeLineEdit; objectName: "latitudeLineEdit"; placeholderText: "Enter current latitude"; Layout.fillWidth: true; style: textFieldStyleView}
            TextField { id:longitudeLineEdit; objectName: "longitudeLineEdit"; placeholderText: "Enter current longitude"; Layout.fillWidth: true; style: textFieldStyleView}

        }
    }

    Rectangle{
        id: distanceTypeGroupTitle
        anchors.topMargin: titleMargin
        anchors.top: textFields.bottom
        Text{
            text: "Distance"
            color: DefaultStyle.colorWhite //"#FFFFFF"
            font.pixelSize: DefaultStyle.fontSize21 //22
        }
    }

    GroupBox {
        id: distanceTypeGroup
        //title: "<font color=\"white\">Show</font>"
        Layout.fillWidth: true
        width: parent.width
        title: ""
        anchors.top: distanceTypeGroupTitle.bottom
        anchors.topMargin: groupBoxMargin

        RowLayout {
            id: gridLayout_Distance
            anchors.fill: parent
            anchors.margins: margin
            ExclusiveGroup { id: rbDistanceTypeGroup; objectName: "rbfuelTypeGroup" }
            NewRadioButton {
                text: "miles"
                exclusiveGroup: rbDistanceTypeGroup
                objectName: "rbMiles"
            }
            NewRadioButton {
                text: "kms"
                exclusiveGroup: rbDistanceTypeGroup
                objectName: "rbKms"
            }
        }
    }
}
