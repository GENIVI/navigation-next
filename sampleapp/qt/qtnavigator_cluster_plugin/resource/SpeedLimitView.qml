import QtQuick 2.4
import "style" 1.0


Rectangle {
    id: speedLimitRect
    width : parent.width / 25
    height: parent.height / 8
    color: DefaultStyle.colorBlack1// "black"
    border.color: DefaultStyle.colorGrey1// "gray"
    border.width: DefaultStyle.borderWidth1// 1
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 100
    anchors.right: parent.right
    anchors.rightMargin: 680
    Image {
        anchors.fill: speedLimitRect
        anchors.margins: 1
        id: buttonImage
        source: applicationDirPath1+ "/resource/images/Speed-Limit.png"
        fillMode: Image.PreserveAspectFit

    }

    Text{
        id: speedLimitLabel
        anchors.fill: speedLimitRect
        anchors.topMargin:  speedLimitRect.height/2 + 5
        anchors.horizontalCenter: speedLimitRect.horizontalCenter
        text:speedLimitText
        color:DefaultStyle.colorWhite  //"white"
        font.family: DefaultStyle.fontRegular.name// sfRegular.name
        font.pixelSize: DefaultStyle.fontSize20 //20
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter

    }

}




