import QtQuick 2.3
import QtQuick.Window 2.2
import QtQuick.Layouts 1.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import "style" 1.0

Item{

//    signal clearButtonClicked()
    Rectangle{
        id: fuelTypeGroupTitle
        anchors.top: parent.top
        anchors.topMargin: titleMargin
        Text{
            text: "Fuel Type"
            color:DefaultStyle.colorWhite //"#FFF"
            font.pixelSize:DefaultStyle.fontSize21 //22
        }
    }
    GroupBox {
        id: fuelTypeGroup
        //title: "<font color=\"white\">Show</font>"
        Layout.fillWidth: true
        width: parent.width
        title: ""

        anchors.top: fuelTypeGroupTitle.bottom
        anchors.topMargin: groupBoxMargin

        GridLayout {
            id: gridLayout_Fuel
            //columns: 3
            rows: 2
            flow: GridLayout.TopToBottom
            anchors.fill: parent
            anchors.margins: margin
            rowSpacing:DefaultStyle.paddingL //20
            //columnSpacing: parent.width / 3
            ExclusiveGroup { id: rbfuelTypeGroup; objectName: "rbfuelTypeGroup"}
            NewRadioButton {
                text: "Diesel"
                exclusiveGroup: rbfuelTypeGroup
                objectName: "rbDiesel"
            }
            NewRadioButton {
                text: "Regular"
                exclusiveGroup: rbfuelTypeGroup
                objectName: "rbRegular"
            }
            NewRadioButton {
                text: "Mid-grade"
                exclusiveGroup: rbfuelTypeGroup
                objectName: "rbMidGrage"
            }
            NewRadioButton {
                text: "Premium"
                exclusiveGroup: rbfuelTypeGroup
                objectName: "rbPremium"
            }
        }
    }
    Rectangle{
        id: languageGroupViewTitle
        anchors.top: fuelTypeGroup.bottom
        anchors.topMargin: titleMargin
        Text{
            text: "Language"
            color:DefaultStyle.colorWhite //"#FFF"
            font.pixelSize:DefaultStyle.fontSize21 //22
        }
    }
    GroupBox {
        id: languageGroupView
        objectName: "languageGroup"
        title: qsTr("")
        width: parent.width
        anchors.top: languageGroupViewTitle.bottom
        anchors.topMargin: groupBoxMargin

        RowLayout {
            anchors.fill: parent
            anchors.margins: margin
            ExclusiveGroup { id: languageGroup }

            NewRadioButton {
                text: "English"
                exclusiveGroup: languageGroup
                objectName: "rbEnglish"
            }
            NewRadioButton {
                text: "Chinese"
                exclusiveGroup: languageGroup
                objectName: "rbChinese"

            }
        }
    }

//    Row{
//        objectName: "cleanData"

////        anchors.top: languageGroupView.bottom
////        anchors.topMargin: groupBoxMargin
//        anchors.bottom: parent.bottom
//        anchors.bottomMargin: 30
//        anchors.left: parent.left
//        anchors.leftMargin: button1.implicitWidth + 100
//        spacing: 100
//        Text{
//            text: "Master Clear"
//            color: "#FFF"
//            font.pixelSize: 22
//           // anchors.fill: parent
//            anchors.verticalCenter: parent.verticalCenter
//        }
//        NewButton{
//            id:button1
//            implicitWidth: 200
//            text : "Master Clear "
//            //visible:false
//            MouseArea{
//                anchors.fill: parent
//                onClicked: {
//                    clearButtonClicked();
//                }
//            }
//        }
   // }
}
