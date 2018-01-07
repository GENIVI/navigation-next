import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
//import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import QtQuick.Controls.Styles 1.2
import "style" 1.0

Rectangle{
    id:mainRec
    width: parent.width/1.5
    height: parent.height/9
    color: "#070C11" //DefaultStyle.colorBlack4
//    anchors.bottom: parent.bottom
//    anchors.bottomMargin: 80
    anchors.horizontalCenter: parent.horizontalCenter
    signal alertBoxValueClicked(bool val)

    Rectangle {
        id: container
        width: mainRec.width
        height: parent.height/9
        color: "#070C11" //DefaultStyle.colorBlack4
        anchors.horizontalCenter: mainRec.horizontalCenter
        anchors.top: mainRec.top

        Image {
            id: fuelIcon
            source: applicationDirPath + "/resource/images/Icon_Fuel_ON.png"
            anchors.right: textMsg.left
            anchors.rightMargin: 5
            fillMode: Image.PreserveAspectFit
        }
        Text{
            id:textMsg
            text: alertTextMsg
            anchors.top: container.top
            anchors.topMargin: 10
            anchors.verticalCenter: fuelIcon.verticalCenter
            anchors.horizontalCenter: container.horizontalCenter
            color: DefaultStyle.colorWhite
            font.family: DefaultStyle.fontSans.name
            font.pixelSize: mainRec.width * 0.04//DefaultStyle.fontSize24
        }
        //    Text{
        //        id:text2
        //        anchors.horizontalCenter: container.horizontalCenter
        //        anchors.verticalCenter: container.verticalCenter
        //        text: "AMSH" //messageText1
        //        anchors.top: text1.bottom
        //        anchors.topMargin: 2
        //        color: DefaultStyle.colorWhite
        //        font.family: DefaultStyle.fontSans.name
        //        font.pixelSize: DefaultStyle.fontSize24
        //    }


        Row{
            anchors.top: textMsg.bottom
            anchors.topMargin: 5
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 10
            Button {
                id: yesButton
                style: ButtonStyle{
                    background: Rectangle{
                        color: DefaultStyle.colorBlack1
                        implicitHeight: container.height * 0.25
                        implicitWidth: container.width*0.20
                        border.color:DefaultStyle.colorGrey1
                        radius: DefaultStyle.rectRadiusS
                    }
                    label:Text{
                        renderType: Text.NativeRendering
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        font.family: DefaultStyle.fontSans.name
                        color: DefaultStyle.colorWhite
                        font.pixelSize: mainRec.width * 0.03 //DefaultStyle.fontSize20
                        text: qsTr("Ok")
                    }
                }
                onClicked: alertBoxValueClicked(true)
            }
            Button {
                id: noButton
                text: qsTr("Cancel")
                style: ButtonStyle{
                    background: Rectangle{
                        color:  DefaultStyle.colorBlack4
                        implicitHeight: container.height * 0.25
                        implicitWidth: container.width*0.20
                        border.color: DefaultStyle.colorGrey1
                        radius: DefaultStyle.rectRadiusS
                    }
                    label:Text{
                        renderType: Text.NativeRendering
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        font.family: DefaultStyle.fontSans.name
                        color:DefaultStyle.colorWhite
                        font.pixelSize: mainRec.width * 0.03 //DefaultStyle.fontSize20
                        text: qsTr("Cancel")
                    }
                }
                onClicked:alertBoxValueClicked(false)
            }
        }
    }
}
