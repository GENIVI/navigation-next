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
    property string placeName: "Favorites"
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
        anchors.right: header.right
        anchors.rightMargin: 20
        anchors.verticalCenter: parent.verticalCenter
        spacing:10
        Image{
            id:addIcon
            source:applicationDirPath+ "/resource/images/listview_resources/add_icn.png"
            fillMode: Image.PreserveAspectFit
            MouseArea{
                anchors.fill: parent
            }
        }
        Image{
            id:deleteIcon
            source:applicationDirPath+ "/resource/images/listview_resources/delete_icn.png"
            fillMode: Image.PreserveAspectFit
            MouseArea{
                anchors.fill: parent
            }
        }
        Image{
            id:deleteAllIcon
            source:applicationDirPath+ "/resource/images/listview_resources/deleteall_btn.png"
            fillMode: Image.PreserveAspectFit
            anchors.verticalCenter: parent.verticalCenter
            MouseArea{
                anchors.fill: parent
            }
        }
    }
}
