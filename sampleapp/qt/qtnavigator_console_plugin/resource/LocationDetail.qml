import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
//import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import QtQuick.Controls.Styles 1.2

Rectangle {
    id: locationDetailContainer
    width: mainRec.width
    height: mainRec.height
    color: "#141B25"
    property string placeName
    property string placeDistance
    property string placeAddress
    property string placeContact
    // property double rating:""
    //property string placeOpenTime
    property string placeLogo: ""
    Rectangle{
        id:header
        color:"black"
        width:locationDetailContainer.width
        height:80
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
                id:compassIcn
                source:applicationDirPath+ "/resource/images/location_detail/compass_icn.png"
                fillMode: Image.PreserveAspectFit
            }
            Text{
                text:placeDistance
                color:"white"
                font.pixelSize: 28
                font.family: sfMedium.name
            }
        }
    }
    Rectangle{
        id:detailBody
        width: locationDetailContainer.width
        height: locationDetailContainer.height-header.height
        anchors.top: header.bottom
        color: "#141B25"
        Row{
            anchors.top:parent.top
            anchors.topMargin: 44
            //            anchors.left:parent.left
            //            anchors.leftMargin: 44
            Rectangle{
                id:locLogo
                color:"#141B25"
                width:locationDetailContainer.width*0.25
                height:detailBody.height
                //                 {
                //                    id: name
                //                    source: placeLogo
                //fillMode: Image.PreserveAspectFit
                //                }
                Image{
                    width:locLogo.width-40
                    height: locLogo.height*0.25
                    anchors.horizontalCenter: parent.horizontalCenter
                    source: placeLogo
                    fillMode: Image.PreserveAspectFit
                }
            }
            Rectangle{
                id:locDetails
                color:"#141B25"
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
                    spacing:10
                    Row{
                        spacing:10
                        Text{
                            id:placename
                            text:placeName
                            color:"white"
                            font.pixelSize:30
                            font.family: sfRegular.name
                        }

                        Image {
                            id: openNowbtn
                            source: applicationDirPath+ "/resource/images/location_detail/open_now_lbl.png"
                            MouseArea{
                                anchors.fill: parent
                            }
                        }

                    }

                    Text{
                        id:address
                        text:placeAddress
                        color:"white"
                        font.pixelSize: 28
                        font.family: sfRegular.name
                    }
                    Text{
                        id:phone
                        text:placeContact
                        color:"green"
                        font.underline: true
                        font.pixelSize: 28
                        font.family: sfRegular.name
                    }
                    Image{
                        id:tripadvisorRating
                        source:applicationDirPath+ "/resource/images/location_detail/tripadvisor_listview_racoon_details.png"
                        fillMode: Image.PreserveAspectFit
                    }
                    Image{
                        id:detailsDevider1
                        source: applicationDirPath+ "/resource/images/location_detail/devider_details.png"
                        width:parent.width
                    }
                    Row{
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
                    }
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
                        MouseArea{
                            anchors.fill: parent
                        }

                    }


                }
            }
            Rectangle{
                id:header3
                color:"#141B25"
                width:locationDetailContainer.width*0.25
                height:detailBody.height
                Column{
                    spacing:20
                    anchors.right: parent.right
                    anchors.rightMargin: 44
                    Image {
                        id: map
                        source: applicationDirPath+ "/resource/images/location_detail/onmap_icn_details.png"
                        fillMode: Image.PreserveAspectFit
                        MouseArea{
                            anchors.fill: parent
                        }
                    }Image {
                        id: call
                        source: applicationDirPath+ "/resource/images/location_detail/call_icn.png"
                        fillMode: Image.PreserveAspectFit
                        MouseArea{
                            anchors.fill: parent
                        }
                    }
                    Image {
                        id: markFavorite
                        source: applicationDirPath+ "/resource/images/location_detail/addtofav_icn.png"
                        fillMode: Image.PreserveAspectFit
                        MouseArea{
                            anchors.fill: parent
                        }
                    }
                    Image {
                        id: nav
                        source: applicationDirPath+ "/resource/images/location_detail/navigate_icn_details.png"
                        fillMode: Image.PreserveAspectFit
                        MouseArea{
                            anchors.fill: parent
                        }
                    }
                }
            }
        }
    }
}

