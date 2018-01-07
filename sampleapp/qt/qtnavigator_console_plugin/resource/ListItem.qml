import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
//import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import QtQuick.Controls.Styles 1.2
import "style" 1.0

//Rectangle{
//    id: listItemViewContainer
//    width: mainRec.width
//    height: mainRec.height
//color: "#151E27"

Rectangle{
    id:listItem
    width:parent.width
    height:100
    color:DefaultStyle.colorBlack2//"#141B25"
    //  anchors.top: header.bottom
    border.width:DefaultStyle.borderWidth1 //1
    border.color:DefaultStyle.colorGrey1 //"grey"
    property string placeName: "Some Place"
    property string placeAddress: "Some Address"
    property bool isFav:false
    property string placeDistance: ""
    // For Fuel
    property string petrolPrice: ""
    property bool isCheapestPetrol:false
    // for tripadvisor rating
    property double placeRating
    property bool isRatingVisble: false

    property bool isDeleteButtonClicked: false
    property int modelCount
    property bool isError: false
    signal hideHamburgerMenuButtonClicked()

    signal navigationButtonClicked()
    signal listItemClicked()
    signal deleteItemClicked();

    FontLoader { id: sfRegular; source: applicationDirPath+ "/resource/font/SF-UI-Display-Regular.otf" }

    Row{
        anchors.fill: parent
        Rectangle{
            id:rec1
            color: DefaultStyle.colorBlack2 //"#141B25"
            height: parent.height-1
            width:parent.width*0.7
            Image{
                source: listView1.currentIndex == index ? applicationDirPath+ "/resource/images/listview_resources/cell_selected.png":""
                width:parent.width
                height: parent.height
            }
            Row{
                anchors.fill: parent
                Image{
                    source:isFav===true?applicationDirPath+ "/resource/images/listview_resources/fav_sign.png":""
                    fillMode: Image.PreserveAspectFit
                    width: 57
                    height: 55
                }
                Image{
                    anchors.verticalCenter: parent.verticalCenter
                    source:isDeleteButtonClicked===true? applicationDirPath+ "/resource/images/listview_resources/delete_red_initial.png":""
                    fillMode: Image.PreserveAspectFit
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            deleteItemClicked();
                        }
                    }
                }
                Image{
                    source:isCheapestPetrol===true?applicationDirPath+ "/resource/images/listview_resources/lowest_price_indicator.png":""
                    fillMode: Image.PreserveAspectFit
                    anchors.verticalCenter: parent.verticalCenter
                    width: 44
                    height: 44
                }
                Rectangle{
                    color: DefaultStyle.colorTransparent//"transparent"
                    height: parent.height-1
                    width:parent.width
                    Column{
                        spacing: DefaultStyle.paddingXXS //2
                        anchors.verticalCenter: parent.verticalCenter
                        Text{
                            text:placeName
                            color:DefaultStyle.colorWhite //"white"
                            font.pixelSize: DefaultStyle.fontSize30 //30
                            font.family: DefaultStyle.fontRegular.name //sfRegular.name
                        }
                        Text{
                            text:placeAddress
                            color:DefaultStyle.colorWhite //"white"
                            font.pixelSize: DefaultStyle.fontSize20 //20
                            font.family: DefaultStyle.fontRegular.name //sfRegular.name
                            wrapMode: Text.WordWrap
                        }

                        Row{
                            visible: !isError && !isRatingVisble
                            spacing:DefaultStyle.paddingXXS //2
                            Image{
                                id:tripadvisorRating
                                source:applicationDirPath+ "/resource/images/location_detail/tripadvisor_listview_racoon_details.png"
                                fillMode: Image.PreserveAspectFit
                            }
                            Row{
                                spacing:DefaultStyle.paddingXS //5
                                Repeater {
                                    model: Math.floor(placeRating)

                                    Image{
                                        anchors.top:parent.top
                                        anchors.topMargin: 10
                                        source:applicationDirPath+ "/resource/images/location_detail/tripadvisor_listview_circle_full.png"
                                        fillMode: Image.PreserveAspectFit
                                    }
                                }
                                Repeater {
                                    model: placeRating%1 !==0 ? 1 : 0

                                    Image{
                                        anchors.top:parent.top
                                        anchors.topMargin: 10
                                        source:applicationDirPath+ "/resource/images/location_detail/tripadvisor_listview_circle_half.png"
                                        fillMode: Image.PreserveAspectFit
                                    }
                                }
                                Repeater {
                                    model: 5-Math.ceil(placeRating)

                                    Image{
                                        anchors.top:parent.top
                                        anchors.topMargin: 10
                                        source:applicationDirPath+ "/resource/images/location_detail/tripadvisor_listview_circle0.png"
                                        fillMode: Image.PreserveAspectFit
                                    }
                                }
                            }
                        }
                    }

//                    MouseArea {
//                        anchors.fill: parent
//                        onClicked: {
//                            console.log("List Item Clicked: ");
//                            listView1.currentIndex=index
//                            listItemClicked()
//                        }
//                    }
                }
            }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                console.log("List Item Clicked: ");
                                listItemClicked()
                            }
                        }
        }
        Rectangle{
            id:rec2
            color: DefaultStyle.colorBlack2 //"#141B25"
            height: parent.height-1
            width:parent.width*0.2
            Image{
                source: listView1.currentIndex == index ? applicationDirPath+ "/resource/images/listview_resources/cell_selected.png":""
                width:parent.width
                height: parent.height
            }
            Row{
                //                    anchors.verticalCenterOffset: 0
                //                    anchors.bottomMargin: 0
                //                    anchors.leftMargin: 0
                //                    anchors.topMargin: 0

                //                    anchors.right: parent.right
                //                    anchors.rightMargin: 20
                //                    anchors.verticalCenter: parent.verticalCenter
                //                    spacing:10
                //anchors.right: parent.right

                anchors.fill: parent
                anchors.left: parent.left
                anchors.leftMargin: parent.width*0.1

                Text{

                    anchors.verticalCenter: parent.verticalCenter
                    color:isCheapestPetrol===true?DefaultStyle.colorLightGreen:DefaultStyle.colorLightBlue //"lightgreen":"lightblue"
                    text: petrolPrice
                    font.pixelSize: DefaultStyle.fontSize28 //28
                }
                Text{
                    anchors.verticalCenter: parent.verticalCenter
                    color: DefaultStyle.colorWhite //"white"
                    text: petrolPrice.length !=0? "|":""
                    font.pixelSize: DefaultStyle.fontSize28 //28
                }
                Text{
                    anchors.verticalCenter: parent.verticalCenter
                    color:DefaultStyle.colorWhite //"white"
                    text: placeDistance.length!==0 ? placeDistance:""
                    font.pixelSize: DefaultStyle.fontSize28 //28
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    console.log("List Item Clicked: ");
                    listItemClicked()
                }
            }
        }
        Rectangle{
            id:rec3
            color: DefaultStyle.colorBlack2 //"#141B25"
            height: parent.height-1
            width:parent.width*0.1

            Image{
                source: listView1.currentIndex == index ? applicationDirPath+ "/resource/images/listview_resources/cell_selected.png":""
                width:parent.width
                height: parent.height
            }
            Image {
                visible: !isError
                anchors.centerIn: parent
                id: navButn
                width: 82
                height: 82
                source:applicationDirPath+ "/resource/images/listview_resources/navigate_icn.png"
                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        navigationButtonClicked()
                    }
                }
            }
        }
    }
    //    MouseArea {
    //        anchors.fill:parent
    //        onClicked: {
    //            listView1.currentIndex=index
    //        }
    //    }
}
//}
