import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import QtQuick.Controls.Styles 1.2
import "style" 1.0
import service.valuesource 1.0

Rectangle{
    id: mainRec
    width: 920 //parent.width/2
    height: parent.height*0.08
    color: "#070C11" //DefaultStyle.colorBlack4
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 80
    anchors.horizontalCenter: parent.horizontalCenter
    signal confirmBoxValueClicked(bool val)
    signal lowFuelLimit(bool f)
    property real fuelValue: ValueSource.fuelLevel
    property bool setlimit:false
    onFuelValueChanged: {
        if(fuelValue <= 20 && !setlimit)
        {
            console.log("emit signal from qml----",fuelValue)
            lowFuelLimit(true)
            setlimit=true
        }else if(setlimit && fuelValue >= 20){
            lowFuelLimit(false)
            setlimit=false
        }
    }
    Rectangle {
        id: container
        width: 920 //parent.width/2
        height: parent.height*0.08
        color: "#070C11" //DefaultStyle.colorBlack4
        anchors.horizontalCenter: mainRec.horizontalCenter
        anchors.top: mainRec.top

        Image {
            id: fuelIcon
            source: applicationDirPath1 + "/resource/images/Icon_Fuel_ON.png"
            anchors.right: textMsg.left
            anchors.rightMargin: 20
            fillMode: Image.PreserveAspectFit
        }
        Text{
            id:textMsg
            text: " Time to fill gas, go to nearest Fuel Station" //messageText1
            anchors.top: container.top
            anchors.topMargin: 10
            anchors.verticalCenter: fuelIcon.verticalCenter
            anchors.horizontalCenter: container.horizontalCenter
            color: DefaultStyle.colorWhite
            font.family: DefaultStyle.fontSans.name
            font.pixelSize: DefaultStyle.fontSize24
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

/*
        Row{
            anchors.top: textMsg.bottom
            anchors.topMargin: 10
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
                        font.pixelSize: DefaultStyle.fontSize20
                        text: qsTr("Accept")
                    }
                }
                onClicked: confirmBoxValueClicked(true)
            }
            Button {
                id: noButton
                text: qsTr("Reject")
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
                        font.pixelSize: DefaultStyle.fontSize20
                        text: qsTr("Reject")
                    }
                }
                onClicked:confirmBoxValueClicked(false)
            }
        }
        */
    }

}
