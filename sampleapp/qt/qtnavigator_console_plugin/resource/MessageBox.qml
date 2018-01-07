import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
//import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import QtQuick.Controls.Styles 1.2
import "style" 1.0

Rectangle {
    signal okButtonClicked();
    id: container
//    width: 1280
//    height: 700
   width:parent.width
  height:parent.height  
  color:DefaultStyle.colorTransparent //"transparent" //"#141B25"

  DefaultMouseArea
  {

  }

    RectangularGlow{
        id:effect
        anchors.fill: popupRec
        glowRadius: 2
        spread:0
        color:DefaultStyle.colorLightBlue   //"#55b1ea"
        cornerRadius: popupRec.radius+glowRadius
    }
    Rectangle{
        id:popupRec
        height:container.height*0.29
        width: container.width*0.4
        color:DefaultStyle.colorBlack4 //"#111D29"
        anchors.verticalCenterOffset: 0
        anchors.horizontalCenterOffset: 0
        anchors.centerIn: parent
        Column{
            id: column1
            anchors.fill: parent
            anchors.top: parent.top
            anchors.topMargin: -2
            Text{
                anchors.horizontalCenter: parent.horizontalCenter
                text:messageText1
                anchors.top: parent.top
                anchors.topMargin: 26
                color:DefaultStyle.colorWhite //"#ffffff"
                font.family:DefaultStyle.fontRegular.name //sfRegular.name
                font.pixelSize:DefaultStyle.fontSize28 //28

            }
            Text{
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottomMargin:  10
                text:messageText2
                anchors.top: parent.top
                anchors.topMargin: 53
                clip: false
                color:DefaultStyle.colorWhite //"#ffffff"
                font.family:DefaultStyle.fontRegular.name //sfRegular.name
                font.pixelSize:DefaultStyle.fontSize28 //28
            }
            Row{
                anchors.top: parent.top
                anchors.topMargin: 120
                anchors.horizontalCenter: parent.horizontalCenter
                spacing:DefaultStyle.paddingL //20
                Button {
                    id: yesButton
                    // text: qsTr("Yes")
                    style: ButtonStyle{
                        background: Rectangle{
                            color:DefaultStyle.colorCyan3 //"#21dcfd"
                            implicitHeight: popupRec.height * 0.25//60
                            implicitWidth: popupRec.width*0.25//166
                            border.color:DefaultStyle.colorWhite //"white"
                            radius:DefaultStyle.rectRadiusXS //4
                            //                            gradient: Gradient{
                            //                                GradientStop{
                            //                                    position: 0;color:"red"}
                            //                                GradientStop{position: 1;color:"yellow"}
                            //                            }

                        }
                        label:Text{
                            renderType: Text.NativeRendering
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            font.family: DefaultStyle.fontRegular.name //sfRegular.name
                            color:DefaultStyle.colorBlack1//"black"
                            font.pixelSize:DefaultStyle.fontSize24 //24
                            text: qsTr("Ok")
                        }
                    }
                    onClicked:okButtonClicked();
                }

            }
        }
    }
}

