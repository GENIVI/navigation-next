import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
//import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import QtQuick.Controls.Styles 1.2
import "style" 1.0

Rectangle {
    id: container
//    width:1280
//    height: 720
    width:parent.width
    height:parent.height
     color: DefaultStyle.colorTransparent //"transparent"

     DefaultMouseArea
     {

     }

       Rectangle{
        id:popupRec
        height:100
        width:250
        color:DefaultStyle.colorBlack1 //"black"
        anchors.centerIn: parent
        Row{
             anchors.verticalCenter: parent.verticalCenter
             anchors.horizontalCenter: parent.horizontalCenter
             //anchors.left: parent.left
             spacing: DefaultStyle.paddingS //10
            AnimatedImage{
                id:loaderIcon
                source:applicationDirPath+ "/resource/images/Loading_wheel.gif"
                anchors.verticalCenter: parent.verticalCenter

            }
            Text{
                text:"Loading..."
                color:DefaultStyle.colorLightBlue1 //"#0FD9FD"
                font.family: DefaultStyle.fontRegular.name //sfRegular.name
                font.pixelSize: DefaultStyle.fontSize30 //30
                anchors.verticalCenter: parent.verticalCenter

            }
        }



    }
}

