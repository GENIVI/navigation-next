import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
//import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import QtQuick.Controls.Styles 1.2
import "style" 1.0

Rectangle {
    id: locationDetailContainer
//    width: 1280
//    height: 720
	width:parent.width
	height:parent.height
    color: DefaultStyle.colorBlack2 //"#141B25"
    //property string placeName
    property string placeDistance
    property string placeAddress
    property string placeContact
    property double placeRating
    //property string placeOpenTime
    property string placeLogo: ""

    signal showNearThisController()
    signal callButtonClicked(string placeName,string placeContact,string placeCategoryImagePath)
    signal detailBackButtonClicked()
    signal mapButtonClicked()
	signal addToFavoritesClicked()
    signal navigationButtonClicked()

    DefaultMouseArea
    {

    }


    onShowNearThisController: {
        ////console.log("Near This Signal Called");
    }

    onDetailBackButtonClicked: {
        //console.log("Back Button Click from Fuel Details");
    }

    FontLoader { id: sfMedium; source: applicationDirPath+ "/resource/font/SF-UI-Display-Medium.otf"}
    FontLoader { id: sfRegular; source: applicationDirPath+ "/resource/font/SF-UI-Display-Regular.otf" }

    Rectangle{
        id:header
        color: DefaultStyle.colorBlack1 //"black"
        width:locationDetailContainer.width
        height:80
        Row {
            spacing:10
            Image {
                id: backBtn
                source: applicationDirPath+ "/resource/images/location_detail/back_icn_selected.png"
                fillMode: Image.PreserveAspectFit
                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        //console.log("Details Back Button Cliked");
                        locationDetailContainer.detailBackButtonClicked()
                    }
                }
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text:itemDetail.placeName
                color:DefaultStyle.colorWhite //"white"
                font.pixelSize: DefaultStyle.fontSize28 //28
                font.family: DefaultStyle.fontMedium.name //sfMedium.name
                elide:Text.ElideRight
                width: locationDetailContainer.width * 0.25

            }
        }
        Row{
            anchors.centerIn: header.center
            anchors.rightMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            spacing:10
            Text{
                text:"Price:"
                color:DefaultStyle.colorWhite //"white"
                font.pixelSize: DefaultStyle.fontSize28 //28
                font.family: DefaultStyle.fontMedium.name //sfMedium.name
                visible: itemDetail.petrolPrice != "" ? true : false
            }
            Text{
                text: itemDetail.petrolPrice
                color: DefaultStyle.colorLightGreen //"lightgreen"
                font.pixelSize: DefaultStyle.fontSize28 //28
                font.family: DefaultStyle.fontMedium.name //sfMedium.name
            }
        }
        Row{
            anchors.right: header.right
            anchors.rightMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            spacing: DefaultStyle.paddingS//10
            Image{
                id:compassIcn
                source:applicationDirPath+ "/resource/images/location_detail/compass_icn.png"
                fillMode: Image.PreserveAspectFit
            }
            Text{
                text: itemDetail.placeDistance
                color:DefaultStyle.colorWhite //"white"
                font.pixelSize: DefaultStyle.fontSize28 //28
                font.family: DefaultStyle.fontMedium.name //sfMedium.name
            }
        }
    }
    Rectangle{
        id:detailBody
        width: locationDetailContainer.width
        height: locationDetailContainer.height-header.height
        anchors.top: header.bottom
        color: DefaultStyle.colorBlack2 //"#141B25"
        Row{
            anchors.top:parent.top
            anchors.topMargin: 44
            //anchors.left:parent.left
            //anchors.leftMargin: 44
            Rectangle {
                id:locLogo
                color:DefaultStyle.colorBlack2 //"#141B25"
                width: locationDetailContainer.width * 0.25
                height: detailBody.height //locationDetailContainer.height * 0.25
                //                border.width: 2
                //                border.color: "gray"
                //                radius: 4
                Rectangle{
                    width: parent.height * 0.35
                    height: parent.height * 0.35
                    anchors.horizontalCenter: parent.horizontalCenter

                    border.width: DefaultStyle.borderWidth2 //2
                    border.color: DefaultStyle.colorGrey1 //"gray"
                    radius: DefaultStyle.rectRadiusS //4
                    color: DefaultStyle.colorTransparent //"transparent"
                    Image{
                        //width:locLogo.width - 40
                        //height: locLogo.height * 0.25
                        anchors.fill: parent
                        anchors.horizontalCenter: parent.horizontalCenter
                        //source: itemDetail.placeCategoryImagePath
                        //source: applicationDirPath + itemDetail.placeCategoryImagePath
                        source: applicationDirPath+ "/resource/images/Detailscreen_images/category_icons/category_icons_512/" + itemDetail.placeCategoryImagePath
                        fillMode: Image.PreserveAspectFit
                    }
                }
            }
            Rectangle{
                id:locDetails
                color:DefaultStyle.colorBlack2 //"#141B25"
                width:locationDetailContainer.width*0.5
                height:detailBody.height
                Column{
                    id: column1
                    width:locDetails.width
                    height:detailBody.height
                    //anchors.centerIn: parent
                    //anchors.top: parent.top
                    anchors.left:parent.left
                    anchors.leftMargin: 20
                    spacing: DefaultStyle.paddingS //10
                    Row {
                        spacing: DefaultStyle.paddingS //10
                        Text{
                            //id:placename
                            text: itemDetail.placeName
                            color:DefaultStyle.colorWhite //"white"
                            font.pixelSize:DefaultStyle.fontSize30 //30
                            font.family: DefaultStyle.fontRegular.name //sfRegular.name
                        }
                        Image {
                            id: openNowbtn
                            source: applicationDirPath+ "/resource/images/location_detail/open_now_lbl.png"
                            MouseArea{
                                anchors.fill: parent
                            }
                        }
                    }
                    Text {
                        id:address
                        text:itemDetail.placeAddress
                        color:DefaultStyle.colorWhite //"white"
                        width: locDetails.width-50
                        wrapMode: Text.Wrap
                        font.pixelSize: DefaultStyle.fontSize28 //28
                        font.family: DefaultStyle.fontRegular.name //sfRegular.name
                    }
                    Text{
                        id: phone
                        text: itemDetail.placeContact
                        color: DefaultStyle.colorGreen //"green"
                        font.underline: true
                        font.pixelSize: DefaultStyle.fontSize28 //28
                        font.family: DefaultStyle.fontRegular.name //sfRegular.name
                    }
                    Row {
			visible: itemDetail.placeRating!=="0"?false:true
                        spacing:DefaultStyle.paddingXXS //2
                        Image{
                            id:tripadvisorRating
                            source:applicationDirPath+ "/resource/images/location_detail/tripadvisor_listview_racoon_details.png"
                            fillMode: Image.PreserveAspectFit
                        }
                        Row{
                            spacing:DefaultStyle.paddingXS //5
                            Repeater {
                                model: Math.floor(itemDetail.placeRating)

                                Image{
                                    anchors.top:parent.top
                                    anchors.topMargin: 10
                                    source:applicationDirPath+ "/resource/images/location_detail/tripadvisor_listview_circle_full.png"
                                    fillMode: Image.PreserveAspectFit
                                }
                            }
                            Repeater {
                                model: itemDetail.placeRating % 1 !==0 ? 1 : 0

                                Image{
                                    anchors.top:parent.top
                                    anchors.topMargin: 10
                                    source:applicationDirPath+ "/resource/images/location_detail/tripadvisor_listview_circle_half.png"
                                    fillMode: Image.PreserveAspectFit
                                }
                            }
                            Repeater {
                                model: 5-Math.ceil(itemDetail.placeRating)

                                Image{
                                    anchors.top:parent.top
                                    anchors.topMargin: 10
                                    source:applicationDirPath+ "/resource/images/location_detail/tripadvisor_listview_circle0.png"
                                    fillMode: Image.PreserveAspectFit
                                }
                            }
                            /*Text {
                                text: "(" + itemDetail.placeReviewCount + ")"
                                color: "white"
                                font.pixelSize: 28
                                font.family: sfRegular.name
                            }*/
                        }
                    }
                    //                    Image{
                    //                        id:tripadvisorRating
                    //                        source:applicationDirPath+ "/resource/images/location_detail/tripadvisor_listview_racoon_details.png"
                    //                        fillMode: Image.PreserveAspectFit
                    //                    }
                    Image{
                        id:detailsDevider1
                        source: applicationDirPath+ "/resource/images/location_detail/devider_details.png"
                        width:parent.width
                        visible: itemDetail.placeTimingInfo != "" ? true : false
                    }
                    Row {
                        spacing: DefaultStyle.paddingXS //5
                        Text{
                            text: itemDetail.placeTimingInfo
                            color:DefaultStyle.colorWhite //"white"
                            width: locDetails.width
                            font.pixelSize: DefaultStyle.fontSize21 //22
                            font.family: DefaultStyle.fontRegular.name //sfRegular.name
                            wrapMode: Text.WordWrap
                        }
                    }

                    /*Row{
                        spacing: 5
                        Text{
                            text:"Mon - Fro"
                            color:"#43FFFe"
                            font.pixelSize: 24
                            font.family: sfRegular.name
                        }
                        Text{
                            text:"4:30 am - 4 pm"
                            color:"white"
                            font.pixelSize: 24
                            font.family: sfRegular.name
                        }
                    }
                    Row{
                        spacing: 5
                        Text{
                            text:"Sat"
                            color:"#43FFFe"
                            font.pixelSize: 24
                            font.family: sfRegular.name
                        }
                        Text{
                            text:"4:30 am - 4 pm"
                            color:"white"
                            font.pixelSize: 24
                            font.family: sfRegular.name
                        }
                    }
                    Row{
                        spacing: 5
                        Text{
                            text:"Sun"
                            color:"#43FFFe"
                            font.pixelSize: 24
                            font.family: sfRegular.name
                        }
                        Text{
                            text:"4:30 am - 4 pm"
                            color:"white"
                            font.pixelSize: 24
                            font.family: sfRegular.name
                        }
                    }*/
                    Image{
                        source: applicationDirPath+ "/resource/images/location_detail/devider_details.png"
                        width:parent.width
                    }
                    Rectangle {
                        //just for extra space
                        height: 5
                        width: 30
                        color:"transparent"
                    }

                    Image {
                        id: nearLocbtn
                        source: applicationDirPath+ "/resource/images/location_detail/nearthislocation_icn.png"
                        /*MouseArea{
                            anchors.fill: parent
                        }*/
                        MouseArea{
                            anchors.fill: parent
                            onClicked: {
                                //console.log("Near This Location Cliked");
                                //locationDetailContainer.detailBackButtonClicked()
                                showNearThisController()
                            }
                        }
                    }
                }
            }
            Rectangle{
                id:header3
                color:DefaultStyle.colorBlack2 //"#141B25"
                width:locationDetailContainer.width*0.25
                height:detailBody.height
                Column{
                    spacing: DefaultStyle.paddingL //20
                    anchors.right: parent.right
                    anchors.rightMargin: 44
                    Image {
                        id: map
                        source: applicationDirPath+ "/resource/images/location_detail/onmap_icn_details.png"
                        fillMode: Image.PreserveAspectFit
                        MouseArea{
                            anchors.fill: parent
                            onClicked: mapButtonClicked()
                        }
                    }Image {
                        id: call
                        source: applicationDirPath+ "/resource/images/location_detail/call_icn.png"
                        fillMode: Image.PreserveAspectFit
                        MouseArea{
                            anchors.fill: parent
                            onClicked: {
                                //console.log("callButton Clicked") ;
                                //console.log(itemDetail);
                                //console.log(JSON.stringify(itemDetail));

                                callButtonClicked(itemDetail.placeName,itemDetail.placeContact,itemDetail.placeCategoryImagePath);
                            }
                        }
                    }
                    Image {
                        id: markFavorite
                         objectName: "favObject"
                        source:itemDetail.isFav==true?applicationDirPath+ "/resource/images/location_detail/addtofav_icn_selected.png":applicationDirPath+ "/resource/images/location_detail/addtofav_icn.png"
                        fillMode: Image.PreserveAspectFit
                        MouseArea{
                            anchors.fill: parent
                            onClicked: {
                                itemDetail.isFav === true ? itemDetail.isFav=false:itemDetail.isFav=true;
                                //console.log("fav!!"+ itemDetail.isFav);
                                addToFavoritesClicked();
                            }
                        }
                    }
                    Image {
                        id: nav
                        source: applicationDirPath+ "/resource/images/location_detail/navigate_icn_details.png"
                        fillMode: Image.PreserveAspectFit
                        MouseArea{
                            anchors.fill: parent
                            onClicked: navigationButtonClicked()
                        }
                    }
                }
            }
        }
    }
}

