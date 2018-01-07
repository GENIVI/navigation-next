import QtQuick 2.7 //2.4
import QtQuick.Controls 2.1 //1.3
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
//import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import QtQuick.Controls.Styles 1.2
import "style" 1.0
Rectangle {
    id:header
    width:parent.width
    height:parent.height/9
    color:DefaultStyle.colorBlack1 //"black"

    //    z:2
    property string placeName: "Navigation"
    signal navBackButtonClicked()
    signal maneuverListClicked()
    signal setClusterUINavMode(int mode)
    DefaultMouseArea
    {

    }

    Image{
        id:backBtn
        height: header.height
        anchors.left: parent.left
        anchors.margins: 10
        anchors.verticalCenter: parent.verticalCenter
        source: applicationDirPath+ "/resource/images/RouteNavigation_images/stop_icn.png"
        fillMode: Image.PreserveAspectFit
        MouseArea{
            anchors.fill: parent
            onClicked: navBackButtonClicked()
        }
    }

    Text{
        id:textName
        anchors.left: backBtn.right
        anchors.verticalCenter: parent.verticalCenter
        text:placeName
        color:DefaultStyle.colorLightBlue1//"#0FD9FD"
        font.pixelSize: DefaultStyle.fontSize32 //32
        //lineHeight: 54
        font.family: DefaultStyle.fontMedium.name//sfMedium.name
    }
//    Column{
//        objectName: "NavModeSwitchObj"
//        spacing: 5
//        width: header.width/6
//        height: header.height - 30
//        anchors.verticalCenter: parent.verticalCenter
//        anchors.horizontalCenter: parent.horizontalCenter
//        visible: false
//        Text {
//            id:navText
//            anchors.horizontalCenter: parent.horizontalCenter
//            text: "Nav Mode"
//            font.pixelSize: DefaultStyle.fontSize20
//            color: DefaultStyle.colorWhite
//        }
    RowLayout{
           id: rowRow
           spacing: 10
           anchors.verticalCenter: parent.verticalCenter
           anchors.horizontalCenter: parent.horizontalCenter
           objectName: "NavModeSwitchObj"
           visible: false
           Text {
               id:onText
               verticalAlignment: Text.AlignVCenter
               text: "Map"  //"On"
               font.pixelSize: DefaultStyle.fontSize28
               color: DefaultStyle.colorWhite
           }
           Switch {
               id: switchComponent
               checked: true
            indicator:Rectangle{
                    implicitWidth: header.width * 0.09
                implicitHeight: header.height * 0.25
                x: switchComponent.leftPadding
                y: parent.height/2 - height/2
                radius:7
                color: switchComponent.checked ? "blue" : "gray"
                Rectangle{
                    x: switchComponent.checked ? parent.width - width : 0
                    width: (header.width * 0.09)/2
                    height: header.height * 0.25
                    radius:7
                    color: switchComponent.down ? "#cccccc" : "#ffffff"
                }
            }

               onClicked: {
                   if(checked)
                        setClusterUINavMode(0)
                   else
                       setClusterUINavMode(1)
               }

           }

           Text {
               verticalAlignment: Text.AlignVCenter
               text: "Nav"  //"Off" //switchComponent.checked ? "Nav Off" : "Nav On"
               font.pixelSize: DefaultStyle.fontSize28
               color: DefaultStyle.colorWhite //platformStyle.colorNormalLight

           }
       }

    Image{
        id:maneuverListButton
        anchors.right: parent.right
        anchors.margins: 10
        anchors.verticalCenter: parent.verticalCenter
        source: applicationDirPath+ "/resource/images/listview_resources/listview_icn.png"
        fillMode: Image.PreserveAspectFit
        MouseArea{
            anchors.fill: parent
            onClicked: maneuverListClicked()
        }
    }

}

