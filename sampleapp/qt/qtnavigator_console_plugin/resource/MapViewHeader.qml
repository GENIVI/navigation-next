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
    height:parent.height/9
    //z:2

    signal thisBackButtonClicked()


    Row{
        spacing:DefaultStyle.paddingS //10
        Image{
            id:backBtn
            source: applicationDirPath+ "/resource/images/location_detail/back_icn_selected.png"
            fillMode: Image.PreserveAspectFit
            MouseArea{
                anchors.fill: parent
                onClicked: thisBackButtonClicked()
            }
        }

        Text{
            objectName: "placeObject"
            anchors.verticalCenter: parent.verticalCenter
            text:placeName
            color:DefaultStyle.colorWhite //"white"
            font.pixelSize: DefaultStyle.fontSize32 //32
            //lineHeight: 54
            font.family: DefaultStyle.fontMedium.name //sfMedium.name
        }
    }
}
