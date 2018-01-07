import QtQuick 2.0
import "style" 1.0

Item {

    property string trafficAlertText: "Heavy traffic ahead! Please drive carefully."

    FontLoader{
        id:sfRegular
        name:  applicationDirPath+ "/resource/font/SF-UI-Display-Regular.otf"
    }

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(20/255,40/255,49/255,255/255)
        border.color: DefaultStyle.colorGrey1 //"gray"
        border.width: DefaultStyle.borderWidth1 //1
        opacity: DefaultStyle.midOpacity //0.8

        Text{
            opacity: 1.0
            anchors.fill: parent
            anchors.margins: 10
            text:trafficAlertText
            color:DefaultStyle.colorWhite //"white"
            font.family: DefaultStyle.fontRegular.name //sfRegular.name
            font.pixelSize: DefaultStyle.fontSize20 //20
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
    }


}
