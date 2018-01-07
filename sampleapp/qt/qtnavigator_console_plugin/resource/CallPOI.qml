import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
//import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import QtQuick.Controls.Styles 1.2
import "style" 1.0

Rectangle {
    id: callPOIContainer
    width: parent.width
    height: parent.height
    color: DefaultStyle.colorBlack2  //"#141B25"
    opacity: DefaultStyle.maxOpacity    //0.95

//    property string placeName
//    property string placeContact
//    property string placeCategoryImagePath
    signal callEndButton()

    DefaultMouseArea
    {

    }

    Column{
        spacing: 0
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        Rectangle{
            id:locationLogoContainer
            anchors.horizontalCenter: parent.horizontalCenter
            color: DefaultStyle.colorTransparent//"transparent"
            width:callPOIContainer.width*0.19
            height:callPOIContainer.width*0.19
            border.color: DefaultStyle.colorGrey1   //"grey"
            border.width: DefaultStyle.borderWidth2 //2
            radius: DefaultStyle.rectRadiusS  //5
            Image{
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                id:favLogo
                source: applicationDirPath + placeCategoryImagePath
                width: locationLogoContainer.width-20
                height: locationLogoContainer.height-20
                fillMode: Image.PreserveAspectFit
            }
        }
//        Rectangle{
//            height:25
//            width: 50
//            color:"transparent"
//        }

        Text{
            anchors.horizontalCenter: parent.horizontalCenter
            id:locationName
            text:placeName
            font.family: DefaultStyle.fontRegular.name //sfRegular.name
            font.pointSize: DefaultStyle.fontSize32 //32
            color: DefaultStyle.colorCyan1  //"#43fffe"
        }
        Text{
            id:locaionContact
            anchors.horizontalCenter: parent.horizontalCenter
            text:placeContact
            font.family: DefaultStyle.fontRegular.name //sfRegular.name
            font.pointSize: DefaultStyle.fontSize31 //31
            color: DefaultStyle.colorWhite //"white"
        }
        Text{
            anchors.horizontalCenter: parent.horizontalCenter
            text:"calling..."
            font.pointSize:DefaultStyle.fontSize28 //28
            font.family: DefaultStyle.fontRegular.name //sfRegular.name
            color: DefaultStyle.colorWhite //"white"
        }
//        Rectangle{
//            height:25
//            width: 50
//            color:"transparent"
//        }
        Image{
            anchors.horizontalCenter: parent.horizontalCenter
            id:callReject
            source: applicationDirPath+ "/resource/images/End_Call.png"
            fillMode: Image.PreserveAspectFit
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    callEndButton();
                }
            }
        }
    }

}

