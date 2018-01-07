import QtQuick 2.0
import "style" 1.0

Item {

    property string roadSignImage : applicationDirPath+ "/resource/images/RouteNavigation_images/hide_show_track_icn.png"

//    FontLoader{
//        id:sfRegular
//        name:  applicationDirPath+ "/resource/font/SF-UI-Display-Regular.otf"
//    }

    Rectangle {
        border.color: DefaultStyle.colorGrey1 //"gray"
        border.width: DefaultStyle.borderWidth1 //1
        anchors.fill: parent
        color: Qt.rgba(20/255,40/255,49/255,255/255)
        opacity: DefaultStyle.midOpacity //0.8

        Image {
            opacity: 1.0
            anchors.fill: parent
            id: buttonImage
            source: roadSignImage
            fillMode: Image.PreserveAspectFit
        }
    }

}
