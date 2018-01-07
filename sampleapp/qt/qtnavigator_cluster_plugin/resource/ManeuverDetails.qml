import QtQuick 2.1
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.2
import "style" 1.0

Rectangle {

    id:mainRec
    width: parent.width
    height: parent.height
    color: DefaultStyle.colorBlack6 //DefaultStyle.colorTransparent //"transparent"
   // anchors.fill: parent
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.topMargin: 200
    anchors.leftMargin: 500
    anchors.rightMargin: 500
    anchors.bottomMargin: 80
    //FontLoader { id: maneuverFont; source: applicationDirPath1+ "/resource/fonts/guidancefont.ttf" }
    //FontLoader { id: sansFont; source: applicationDirPath1+ "/resource/fonts/DROIDSANS.TTF" }
//    Rectangle {
//        anchors.fill: parent
//        color: DefaultStyle.colorBlack6 //"#13151A"
//    }
    Label {
        id: roadNameLabel
        width: 200
        height: 55
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 45
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: DefaultStyle.fontSize50 //55
        font.family: DefaultStyle.fontSans.name //sansFont.name
        color: DefaultStyle.colorWhite //"white"
        text: roadName  //"I-105 WEST"

    }

    Label {
        id: maneuverIconLabel
        width: 200
        height: 170
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: roadNameLabel.top
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.pixelSize: 160
        font.family: DefaultStyle.fontGuidance.name //maneuverFont.name
        color: DefaultStyle.colorWhite //"white"
        text: directionIcon//"i"

    }

    Label {
        id: distanceLabel
        width: 200
        height: 45
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: maneuverIconLabel.top
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: DefaultStyle.fontSize45 //45
        font.family: DefaultStyle.fontSans.name //sansFont.name
        color: DefaultStyle.colorWhite //"white"
        text: distance //"23.7 mi"

    }

}
