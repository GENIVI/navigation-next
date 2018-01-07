import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
//import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import QtQuick.Controls.Styles 1.2
import "style" 1.0

Rectangle {
    id:header
    color:DefaultStyle.colorBlack1 //"black"
    width:parent.width
    height:parent.height
    z:2
//    FontLoader{
//        id:sfMedium
//        name:  applicationDirPath+ "/resource/font/SF-UI-Display-Medium.otf"
//    }
    property bool separator:false
    property string placeNameHeader//: "Destination Name"
    property string placeDistance//: "Distance: 1.50 Mi."
    property string placeETA//: "Remaining time: 02:25 m"
    //property string firstIcon: applicationDirPath+ "/resource/images/RouteNavigation_images/stop_icn.png"
    property string firstIcon: applicationDirPath+ "/resource/images/RouteNavigation_images/back_icon.png"
    property string lastIcon: applicationDirPath+ "/resource/images/listview_resources/mapview_icn.png"
    //property string lastIcon: applicationDirPath+ "/resource/images/listview_resources/listview_icn.png"
    signal thisBackButtonClicked()
    signal thisMapToggleButtonClicked()
    Rectangle{
        id:leftRect
        width: parent.width*0.32
        height: parent.height
        color:DefaultStyle.colorBlack1 //"black"

        Row{
            spacing:DefaultStyle.paddingXS//5
            Image{

                anchors.top: header.top
                anchors.topMargin: 20
                anchors.left: header.right
                anchors.leftMargin: 20
                id:backBtn
                source: firstIcon
                fillMode: Image.PreserveAspectFit
                width: 80
                height: 80
                MouseArea{
                    anchors.fill: parent
                    onClicked: thisBackButtonClicked()
                }
            }

            Text{
                anchors.verticalCenter: parent.verticalCenter
                text:placeNameHeader
                color:DefaultStyle.colorCyan2//"#0ED9FD"
                font.pixelSize: DefaultStyle.fontSize32 //32
                width:leftRect.width - backBtn.width //header.width*0.35
                elide: Text.ElideRight
                //lineHeight: 54
                font.family: DefaultStyle.fontMedium.name //sfMedium.name.name
            }

        }
    }
    Rectangle{
        id:middleRect
        width: parent.width*0.32
        height: parent.height*0.5
        anchors.left: leftRect.right
        anchors.leftMargin: 20
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        color:DefaultStyle.colorBlack1 //"black"

        //        anchors.top: header.top
        //        anchors.topMargin: 50
        Row{
            spacing:DefaultStyle.paddingXS //5

            Text {
                id : distance

                //                anchors.left: parent.left
                //                anchors.leftMargin: 10
                //                anchors.bottom: parent.bottom
                //anchors.bottomMargin: 10
                text:placeDistance
                font.family: DefaultStyle.fontMedium.name //sfMedium.name.name
                font.pixelSize: DefaultStyle.fontSize36 //36
                color: DefaultStyle.colorWhite //"white"
            }
            Text {

                //                anchors.left: parent.left
                //                anchors.leftMargin: 10
                //                anchors.bottom: parent.bottom
                //anchors.bottomMargin: 10
                text:" | "
                font.family: DefaultStyle.fontMedium.name //sfMedium.name.name
                font.pixelSize:DefaultStyle.fontSize36 //36
                color: DefaultStyle.colorWhite //"white"
                visible: separator
            }
            Text {
                //                anchors.right: parent.right
                //                anchors.rightMargin: 10
                //                anchors.bottom: parent.bottom
                //anchors.bottomMargin: 10
                text:placeETA
                font.family: DefaultStyle.fontMedium.name //sfMedium.name.name
                font.pixelSize:DefaultStyle.fontSize36 //36
                color: DefaultStyle.colorWhite //"white"
            }

        }
    }
    Image{
        anchors.right: header.right
        anchors.rightMargin: 20
        //   anchors.verticalCenter: parent.verticalCenter
        id:mapIcon
        source:lastIcon
        fillMode: Image.PreserveAspectFit
        MouseArea{
            anchors.fill: parent
            onClicked: thisMapToggleButtonClicked()
        }
    }
}
