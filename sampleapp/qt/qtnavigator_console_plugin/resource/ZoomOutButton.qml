import QtQuick 2.0
import "style" 1.0

Item {

    signal mapZoomOut()

    Rectangle {

        width: parent.width
        height: parent.height
        radius: parent.width/2
        border.color: DefaultStyle.colorLightGray //"lightgray"
        border.width:DefaultStyle.borderWidth1 //1
        //        color: "#80000000"

        color: DefaultStyle.colorTransparent //"transparent"
        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: DefaultStyle.colorBlack1 //"black"
            opacity: DefaultStyle.minOpacity //0.5
        }

        Image {
            anchors.fill: parent
            id: buttonImage
            source: applicationDirPath+ "/resource/images/Homescreen_images/map_icons/ZoomOut.png"
            fillMode: Image.PreserveAspectFit
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    mapZoomOut();
                }
            }
        }
    }
}
