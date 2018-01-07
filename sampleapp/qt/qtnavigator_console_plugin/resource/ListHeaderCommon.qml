import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
//import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.3
import "style" 1.0

Rectangle {
    id:header
    color:DefaultStyle.colorBlack1 //"black"
    width:parent.width
    height:parent.height
    z:2
    property string placeName
    property string lowestPetrolPrice: ""
    property string avgPetrolPrice: ""
    property bool isRecents: false
    property bool isFavorite: false
    property bool  isListEmpty: true
    property string errorMessage: ""
    signal thisBackButtonClicked()
    signal thisClearAllResentsClicked()
    signal thisClearAllFavouritesClicked()
    signal thisDeleteFavourite()
    signal mapButtonClicked()

    DefaultMouseArea
    {

    }

    // FontLoader { id: sfMedium; source: applicationDirPath+ "/resource/font/SF-UI-Display-Medium.otf"}
    RowLayout{
        anchors.fill: parent
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        Item{
            implicitHeight: row1.height
            implicitWidth: row1.width
        Row{
            id:row1
            //spacing:DefaultStyle.paddingS//10

            Image{
                id:backBtn
                source: applicationDirPath+ "/resource/images/location_detail/back_icn_selected.png"
                fillMode: Image.PreserveAspectFit
                MouseArea{
                    anchors.fill: parent
                    onClicked: thisBackButtonClicked()
                }
            }

            Text{
                objectName: "placeObject"
                anchors.verticalCenter:/* parent*/backBtn.verticalCenter
                text:placeName
                color: DefaultStyle.colorWhite //"white"
                font.pixelSize: DefaultStyle.fontSize32 //32
                //lineHeight: 54
                font.family: DefaultStyle.fontMedium.name //sfMedium.name.name
            }
        }
        }
        Item{
            implicitHeight: row2.height
            implicitWidth: header.width * 0.4
            anchors.verticalCenter: header.verticalCenter
        Row{
            id:row2
            spacing: DefaultStyle.paddingS //20
            Text{
                text:lowestPetrolPrice.length!=0?"Lowest:":""
                color: DefaultStyle.colorWhite //"white"
                font.pixelSize: DefaultStyle.fontSize28
                font.family: DefaultStyle.fontMedium.name //sfMedium.name.name
            }
            Text{
                text:lowestPetrolPrice
                color: DefaultStyle.colorLightGreen //"lightgreen"
                font.pixelSize:  DefaultStyle.fontSize28
                font.family: DefaultStyle.fontMedium.name //sfMedium.name.name
            }
            Text{
                text:avgPetrolPrice.length!=0?"Average:":""
                color: DefaultStyle.colorWhite //"white"
                font.pixelSize:  DefaultStyle.fontSize28
                font.family: DefaultStyle.fontMedium.name //sfMedium.name.name
            }
            Text{
                text:avgPetrolPrice
                color: DefaultStyle.colorLightBlue //"lightblue"
                font.pixelSize:  DefaultStyle.fontSize28
                font.family: DefaultStyle.fontMedium.name //sfMedium.name.name
            }
        }
        }
        Item{
            implicitHeight: row3.height
            implicitWidth: row3.width
            anchors.verticalCenter: parent.verticalCenter
        Row{
            id:row3
            spacing: DefaultStyle.paddingS //10
            Image{
                //            visible: !isRecents && !isFavorite
                visible: false
                id:sortIcon
                source:applicationDirPath+ "/resource/images/listview_resources/sorting_icn.png"
                fillMode: Image.PreserveAspectFit
                MouseArea{
                    anchors.fill: parent
                }
            }
            Image{
                visible: !isRecents && !isFavorite
                id:mapIcon
                objectName: "mapIconObject"
                source:applicationDirPath+ "/resource/images/listview_resources/mapview_icn.png"
                fillMode: Image.PreserveAspectFit
                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        console.log("ListHeaderCommon: mapToggleButtonClicked")
                        mapButtonClicked()
                    }
                }
            }
            Image{
                visible: isRecents && errorMessage=="" && !isListEmpty
                id:clearAllIcon
                source:applicationDirPath+ "/resource/images/listview_resources/clear_all_btn.png"
                fillMode: Image.PreserveAspectFit
                MouseArea{
                    anchors.fill: parent
                    onClicked: thisClearAllResentsClicked()
                }
            }
            Image{
                // visible: isFavorite
                visible: false
                id:addIcon
                source:applicationDirPath+ "/resource/images/listview_resources/add_icn.png"
                fillMode: Image.PreserveAspectFit
                MouseArea{
                    anchors.fill: parent
                }
            }
            Image{
                visible: isFavorite && errorMessage=="" && !isListEmpty
                id:deleteIcon
                source:applicationDirPath+ "/resource/images/listview_resources/delete_icn.png"
                fillMode: Image.PreserveAspectFit
                MouseArea{
                    anchors.fill: parent
                    onClicked: thisDeleteFavourite()
                }
            }
            Image{
                visible: isFavorite && errorMessage=="" && !isListEmpty
                id:deleteAllIcon
                source:applicationDirPath+ "/resource/images/listview_resources/deleteall_btn.png"
                fillMode: Image.PreserveAspectFit
                anchors.verticalCenter: deleteIcon.verticalCenter
                MouseArea{
                    anchors.fill: parent
                    onClicked: thisClearAllFavouritesClicked()
                }
            }
        }
        }
    }
}
