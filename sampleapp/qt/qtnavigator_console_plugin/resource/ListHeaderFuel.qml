import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
//import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import QtQuick.Controls.Styles 1.2

Rectangle {
    id:header
    color:"black"
    width:mainRec.width
    height:80
    z:2
    property string placeName: "Fuel"
    property string lowestPetrolPrice: ""
    property string avgPetrolPrice: ""
    Row{
        spacing:10
        Image{
            id:backBtn
            source:applicationDirPath+ "/resource/images/location_detail/back_icn_selected.png"
            fillMode: Image.PreserveAspectFit
            MouseArea{
                anchors.fill: parent
            }
        }

        Text{
            anchors.verticalCenter: parent.verticalCenter
            text:placeName
            color:"white"
            font.pixelSize: 32
            //lineHeight: 54
            font.family: sfMedium.name
        }
    }
    Row{
        anchors.centerIn: header.center
        anchors.rightMargin: 20
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        spacing:20
        Text{
            text:"Lowest:"
            color:"white"
            font.pixelSize: 28
            font.family: sfMedium.name
        }
        Text{
            text:lowestPetrolPrice
            color:"lightgreen"
            font.pixelSize: 28
            font.family: sfMedium.name
        }
        Text{
            text:"Average:"
            color:"white"
            font.pixelSize: 28
            font.family: sfMedium.name
        }
        Text{
            text:avgPetrolPrice
            color:"lightblue"
            font.pixelSize: 28
            font.family: sfMedium.name
        }
    }
    Row{
        anchors.right: header.right
        anchors.rightMargin: 20
        anchors.verticalCenter: parent.verticalCenter
        spacing:10
        Image{
            id:sortIcon
            source:applicationDirPath+ "/resource/images/listview_resources/sorting_icn.png"
            fillMode: Image.PreserveAspectFit
            MouseArea{
                anchors.fill: parent
            }
        }
        Image{
            id:mapIcon
            source:applicationDirPath+ "/resource/images/listview_resources/mapview_icn.png"
            fillMode: Image.PreserveAspectFit
            MouseArea{
                anchors.fill: parent
            }
        }
    }
}
