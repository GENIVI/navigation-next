import QtQuick 2.3
import QtQuick.Window 2.2
import QtQuick.Layouts 1.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
//import QtQuick.Extras 1.4
import QtQuick.Controls.Private 1.0
import "style" 1.0

Item{
   //signal comboclicked(int)
    width:parent.width
    height: parent.height
    property var filename
    Rectangle{

        id: showGroupViewTitle
        anchors.top: parent.top
        anchors.topMargin: titleMargin
        Text{
            text: "Show"
            color: DefaultStyle.colorWhite //"#FFF"
            font.pixelSize: DefaultStyle.fontSize21 //22
        }
    }
    GroupBox {
        id: showGroupView
        objectName: "showGroup"
        //title: "<font color=\"white\">Show</font>"
        title: ""
        Layout.fillWidth: true
        width: parent.width

        anchors.top: showGroupViewTitle.bottom
        anchors.topMargin: groupBoxMargin

        GridLayout {
            id: gridLayout
            rows: 2
            flow: GridLayout.TopToBottom
            anchors.fill: parent
            anchors.margins: margin
            rowSpacing: DefaultStyle.paddingL //20

            NewCheckBox {
                id: checkBox
                text: qsTr("Overhead Signs")
                objectName: "chkSigns"
            }

            NewCheckBox {
                id: checkBox_4
                text: qsTr("3D Landmarks")
                objectName: "chkLandmarks"
            }

            NewCheckBox {
                id: checkBox_3
                text: qsTr("3D Cities")
                objectName: "chkCities"
            }

            NewCheckBox {
                id: checkBox_2
                text: qsTr("3D Freeways")
                objectName: "chkFreeways"
            }

            NewCheckBox {
                id: checkBox_5
                text: qsTr("Places on Map")                
                objectName: "chkPlaces"
            }
        }
    }

    Rectangle{
        id: defaultGroupViewTitle
        anchors.top: showGroupView.bottom
        anchors.topMargin: titleMargin
        Text{
            text: "Default View"
            color: DefaultStyle.colorWhite //"#FFF"
            font.pixelSize: DefaultStyle.fontSize21 //22
        }
    }
    GroupBox {
        id: defaultGroupView
        objectName: "defaultViewGroup"
        title: qsTr("")
        //anchors.top: showGroupView.bottom
        // anchors.topMargin: 10
        width: navigationTab.width        
        anchors.top: defaultGroupViewTitle.bottom
        anchors.topMargin: groupBoxMargin

        RowLayout {
            anchors.fill: parent
            anchors.margins: margin
            ExclusiveGroup { id: defaultViewGroup }

            NewRadioButton {
                text: "Perspective"
                exclusiveGroup: defaultViewGroup                
                objectName: "rbPerspective"
            }
            NewRadioButton {
                text: "2D Trip Overview"
                exclusiveGroup: defaultViewGroup
                objectName: "rb2DTripView"

            }
            NewRadioButton {
                text: "List View"
                exclusiveGroup: defaultViewGroup
                objectName: "rbListView"
            }
        }
    }

    // FOR GPS OPTIONS
    Rectangle{
        id: defaultGPSFileitle
        anchors.top: defaultGroupView.bottom
        anchors.topMargin: titleMargin
        Text{
            text: "Select GPS file"
            color: DefaultStyle.colorWhite //"#FFF"
            font.pixelSize: DefaultStyle.fontSize21//22
        }
    }
//    ComboBox {
//            width: parent.width*0.35
//            height: 40
//            anchors.top: defaultGPSFileitle.bottom
//            anchors.topMargin: titleMargin*2
//            model: gpsFileTitle
//            objectName: "gpsCombo"
//            function selectedIndex(text){
//                console.log("selectedIndex..............................",text)

//                return find(text);
//            }
//    }

    MyTestComboBox{
        id:combo
	height: 40
        anchors.top: defaultGPSFileitle.bottom
        anchors.topMargin: titleMargin*4
        modelName:gpsFileTitle
        objectName: "gpsCombo"

   }

}
