import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
//import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import QtQuick.Controls.Styles 1.2
import "style" 1.0

Rectangle{
    id:nearThisLocationContainer
    width:parent.width //mainRec.width
    height:parent.height //mainRec.height
    color:DefaultStyle.colorBlack2 //"#141B25"

    DefaultMouseArea
    {

    }

    Rectangle{
        id:listItemsContainer
        //  anchors.top: topBar.bottom
        width:mainRec.width
        height:mainRec.height
        color:DefaultStyle.colorBlack2 //"#141B25"

        ListView{
            id: listView1
            width: listItemsContainer.width
            height: listItemsContainer.height
            snapMode: ListView.SnapToItem
            headerPositioning:ListView.OverlayHeader
            header: Rectangle{
                id:topBar
                width:mainRec.width
                height:mainRec.height*0.3
                color:DefaultStyle.colorBlack2 //"#141B25"
                z:2
                Image{
                    source:applicationDirPath+ "/resource/images/cancel_btn.png"
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                }
            }
            delegate: Item {
                width: listItemsContainer.width
                height: 80
                Rectangle{
                    id:itemContainer
                    width: listItemsContainer.width
                    height: 80
                    color:DefaultStyle.colorBlack1 //"black"
                    border.color: DefaultStyle.colorBlack5 //"#273133"
                    border.width: DefaultStyle.borderWidth1 //1
                    opacity: DefaultStyle.minOpacity//0.2(0.5)
                }
                //TIP: keep Rectangle container out of the inner items for applying opacity only to the rectangle and not text or images.
                Row {
                    id: row1
                    spacing: DefaultStyle.paddingS //10
                    anchors.fill: itemContainer
                    Image {
                        id: menuImage
                        source: applicationDirPath+ "/resource/"+path
                        width: 82
                        height: 80
                    }
                    Text {
                        text: name
                        color: DefaultStyle.colorWhite //"white"
                        font.family: DefaultStyle.fontRegular.name //sfRegular.names
                        font.pixelSize: DefaultStyle.fontSize28 //28
                        anchors.verticalCenter: menuImage.verticalCenter
                        anchors.left: menuImage.right
                    }
                }
                MouseArea {
                    anchors.fill:parent
                    onClicked: {
                        listView1.currentIndex=index
                    }
                }
            }
            model: ListModel {
                ListElement {
                    name: "Restaurants"
                    path:"images/restaurants_icn.png"
                }
                ListElement {
                    name: "Banks & ATMs"
                    path:"images/banks_icn.png"
                }
                ListElement {
                    name: "Airports"
                    path:"images/airport_icn.png"
                }
                ListElement {
                    name: "Parking"
                    path:"images/parking_icn.png"
                }
                ListElement {
                    name: "Lodging"
                    path:"images/lodging_icn.png"
                }          ListElement {
                    name: "Healthcare"
                    path:"images/healthcare_icn.png"
                }         ListElement {
                    name: "Shopping"
                    path:"images/shopping_icn.png"
                }
            }
        }
    }
}
