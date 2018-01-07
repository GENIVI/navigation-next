import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
//import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import QtQuick.Controls.Styles 1.2


Rectangle {
    id: addedToFavContainer
    width: mainRec.width
    height: mainRec.height
    color: "#141B25"
    opacity: 0.95

    property string placeName

    Column{
        spacing: 5
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        Image{
            anchors.horizontalCenter: parent.horizontalCenter
            id:favLogo
            source: applicationDirPath+ "/resource/images/location_detail/addtofav_icn_selected.png"
            width: sourceSize.width*2
            height: sourceSize.height*2
            fillMode: Image.PreserveAspectFit
        }
        Text{
            anchors.horizontalCenter: parent.horizontalCenter
            id:favLocation
            text:placeName
            font.pointSize: 38
            font.family: sfRegular.name
            color: "white"
        }
        Text{
            anchors.horizontalCenter: parent.horizontalCenter
            text:"Added to your favorites"
            font.pointSize: 21
             font.family: sfRegular.name
            color: "white"
        }
    }

}

