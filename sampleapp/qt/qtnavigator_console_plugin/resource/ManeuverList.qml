import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
//import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import QtQuick.Controls.Styles 1.2
import "style" 1.0

Rectangle{
    id:listItem
    width:parent.width
    height:125
    color:DefaultStyle.colorBlack2 //"#141B25"
    border.width: DefaultStyle.borderWidth1 //1
    border.color: DefaultStyle.colorGrey1 //"grey"
    property string imageId
    property string streetName//: "Some Place"
    property string distance//: "Some Distance"
    signal listItemClicked()

    Row{
        anchors.fill: parent
        Rectangle{
            id:directionIcon
            color:DefaultStyle.colorBlack2 //"#141B25"
            height: parent.height-1
            width:parent.width*0.2
//            Image{
//                anchors.centerIn: parent
//                source: imageId
//            }
            Text {
                id: maneuverIconLabel
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: DefaultStyle.fontSize50
                font.family: DefaultStyle.fontIcon.name //maneuverFont.name
                color: DefaultStyle.colorCyan4 //"white"
                text: imageId//"i"

            }
        }
        Rectangle{
            id:streetNameId
            color:DefaultStyle.colorBlack2 //"#141B25"
            height: parent.height-1
            width:parent.width*0.6
            Text{
                anchors.centerIn: parent
                anchors.margins: 50
                color:DefaultStyle.colorWhite //"white"
                text: streetName
                width: parent.width-50
                elide: Text.ElideRight
                font.pixelSize: DefaultStyle.fontSize28 //28
                font.family: DefaultStyle.fontRegular.name //sfRegular.name
            }
        }
        Rectangle{
            id:nextTurnDistance
            color:DefaultStyle.colorBlack2 //"#141B25"
            height: parent.height-1
            width:parent.width*0.2
            Text{
                anchors.centerIn: parent
                anchors.margins: 50
                color:DefaultStyle.colorWhite //"white"
                text: distance
                //                width: parent.width
                //                elide: Text.ElideRight
                font.pixelSize: DefaultStyle.fontSize28 //28
                font.family: DefaultStyle.fontRegular.name //sfRegular.name
            }
        }
//        Rectangle{
//            id:trafficWarning
//            color:"#141B25"
//            height: parent.height-1
//            width:parent.width*0.2
//            Text{
//                anchors.centerIn: parent
//                anchors.margins: 50
//                color:"white"
//                text: trafficWarningText
//                //                width: parent.width
//                //                elide: Text.ElideRight
//                font.pixelSize: 28
//                font.family: sfRegular.name
//            }
//        }
    }
    MouseArea {
        anchors.fill:parent
        onClicked: {
            //console.log("List Item Clicked: ")
            //console.log("streetName :: ",streetName,"     ",distance)
            listItemClicked()
            listView1.currentIndex=index
        }
    }
}

