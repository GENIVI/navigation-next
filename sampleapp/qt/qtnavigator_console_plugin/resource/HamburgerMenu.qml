import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
//import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import "style" 1.0

Rectangle {
    id: hamburgerMenuContainer
    width:parent.width
    height:parent.height
    color:DefaultStyle.colorBlack1 //"#000"
    anchors.top: parent.top
    anchors.topMargin: 0
    signal hideHamburgerMenuButtonClicked()
    signal moviesButtonClicked()
    signal gasStationButtonClicked()
    signal recentsButtonClicked()
    signal favouritesButtonClicked()
    signal nearMeClicked()
    signal closeSideMenus()
    property int fontSize: Math.round(hamburgerMenuContainer.width / 8)

    DefaultMouseArea
    {

    }

    ListView {
        id: listView1
        objectName: "listObject"
        width: hamburgerMenuContainer.width
        height: hamburgerMenuContainer.height
        highlight: Rectangle{
            color: "#000000"
        }
        focus: true
        clip:true
        // onCurrentItemChanged: handleMenuClick(listView1.currentIndex,model.get(listView1.currentIndex).path);
        function handleMenuClick(item_index,name){
           //console.log(item_index.path + "asdasda" +name );
        }
        currentIndex:-1
        delegate: Item {
            width: hamburgerMenuContainer.width
            height: 82
    //            FontLoader{
    //                id:sfRegular
    //                name:  applicationDirPath+ "/resource/font/SF-UI-Display-Regular.otf"
    //            }
            Rectangle{
                width: hamburgerMenuContainer.width
                height: 82
                color: DefaultStyle.colorTransparent //"transparent"
                border.color: DefaultStyle.colorBlack5//"#273133"
                border.width: DefaultStyle.borderWidth1 //1
                Image {
                    id: borderGlow
                    source: listView1.currentIndex == index ? applicationDirPath+ "/resource/images/selected_row_glow_bg.png":""
                    width: hamburgerMenuContainer.width+10
		    height: parent.height+30
                    y:-15
                }
                Row {
                    id: row1
                    spacing: DefaultStyle.paddingS //10
                    Image {
                        id: menuImage
                        source: listView1.currentIndex == index ? applicationDirPath+ "/resource/"+path.split(".")[0].concat("_selected.png"):applicationDirPath+ "/resource/"+path
                        width: hamburgerMenuContainer.width/3
                        height: hamburgerMenuContainer.width/3
                       //  anchors.left: rectangle3.left
                    }
                    Text {
                        text: name
                        //font.bold: true
                        color: listView1.currentIndex == index ? "#55b1ea":"white"
                        font.family: DefaultStyle.fontRegular.name //sfRegular.name
                        //font.pixelSize: 24
                        font.pixelSize: fontSize
                        anchors.verticalCenter: menuImage.verticalCenter
                        anchors.left: menuImage.right
                    }

                }
            }
//            Loader {
//                id: subItemLoader
//                width: 200
//                height: parent.height
//                property variant subItemModel: null
//                sourceComponent: subItemModel == null? null: subItemColumnDelegate
//                function setModel() {
//                    if (status == Loader.Ready) item.model = subItemModel
//                }
//                onStatusChanged: setModel()
//                onSubItemModelChanged: setModel()
//            }
            MouseArea {
                anchors.fill:parent
                onPressed: {
                    listView1.currentIndex=index
//                    if (subItemLoader.subItemModel != subItems)
//                        subItemLoader.subItemModel = subItems
//                    else
//                        subItemLoader.subItemModel = null

                    //  listView1.model.get(index).path = "images/nearby_icn.png"
                    //console.log((listView1.model.get(index).path))
                    //console.log((listView1.model.get(index).path).split(".")[0].concat("_selected.png"))
                    //console.log("index: ")
                    //console.log(index)
                    //console.log("applicationDirPath :: ")
                    //console.log(applicationDirPath)

                    if(index==0){
                        closeSideMenus()
                     //   hideHamburgerMenuButtonClicked()
                    }
                    //  hide hamburger menu
                    /*if(index!=1){
                        closeSideMenus()
                       //  hideHamburgerMenuButtonClicked()
                    }*/



                        if(index==1){
                            nearMeClicked()
                        }
                        if(index==2){
                            favouritesButtonClicked()
                        }
                        if(index==3){
                            //console.log("recentsButtonClicked")
                            recentsButtonClicked()
                        }
                    if (index == 5) {
                        moviesButtonClicked()
                    }
                    else if (index == 6) {
                        //console.log("gasStationButtonClicked")
                        gasStationButtonClicked()
                    }
                }
            }
        }
        model: ListModel {
            ListElement {
                //name: "Near Me"
                path:"images/close_icn.png"
            }
            ListElement {
                name: "Near Me"
                path: "images/nearby_icn.png"
//                subItems: [
//                    ListElement {
//                        itemName: "Restaurants"
//                        subPath:"images/restaurants_icn.png"
//                    },
//                    ListElement {
//                        itemName: "Banks & ATMs"
//                        subPath:"images/banks_icn.png"
//                    },
//                    ListElement {
//                        itemName: "Airports"
//                        subPath:"images/airport_icn.png"
//                    },
//                    ListElement {
//                        itemName: "Parking"
//                        subPath:"images/parking_icn.png"
//                    },          ListElement {
//                        itemName: "Lodging"
//                        subPath:"images/lodging_icn.png"
//                    },          ListElement {
//                        itemName: "Healthcare"
//                        subPath:"images/healthcare_icn.png"
//                    },          ListElement {
//                        itemName: "Shopping"
//                        subPath:"images/shopping_icn.png"
//                    }
//                ]
            }

            ListElement {
                name: "Favorites"
                path:"images/favorites_icn.png"
            }

            ListElement {
                name: "Recents"
                path:"images/recent_icn.png"
            }

            ListElement {
                name: "Weather"
                path:"images/weather_icn.png"
            }
            ListElement {
                name: "Theater"
                path:"images/theatre_icn.png"
            }

            ListElement {
                name: "Gasoline"
                path:"images/gasoline_icn.png"
            }
        }

    }
//    Component {
//        id: subItemColumnDelegate
//        Column {
//            property alias model : subItemRepeater.model
//            width: 200
//            height: 500
//            Repeater {
//                id: subItemRepeater
//                delegate: Rectangle {
//                    color: "black"
//                    height: 82
//                    width: 286
//                    border.color: "#273133"
//                    border.width: 1
//                    x:218
//                    Image {
//                        id: menuImage
//                        source: applicationDirPath+ "/resource/"+subPath
//                        width: 82
//                        height: 82
//                        //anchors.left: rectangle3.left
//                    }
//                    Text {
//                        text: itemName
//                       // font.bold: true
//                        color: "white"
//                        font.family: sfRegular.name
//                        font.pixelSize: 28
//                        anchors.verticalCenter: menuImage.verticalCenter
//                        anchors.left: menuImage.right
//                    }
//                    //                            Text {
//                    //                                anchors.verticalCenter: parent.verticalCenter
//                    //                                x: 30
//                    //                                font.pixelSize: 18
//                    //                                text: itemName
//                    //                            }
//                }
//            }
//        }
//    }

}
