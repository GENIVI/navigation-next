import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
//import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import QtQuick.Controls.Styles 1.2
import "style" 1.0

Rectangle {
    id:mainRec
    width:parent.width
    height: parent.height

Rectangle {
    id:mainContainer
    color:DefaultStyle.colorTransparent //"transparent"
    width: mainRec.width
    height: mainRec.height //- mainRec.height/5 - (mainRec.height/5 - mainRec.height/8.5) //temp fix
//    width: mainRec.width*0.4
//    height: mainRec.height-mainRec.height/5-((mainRec.height/5)-(mainRec.height/7))
//    anchors.top:parent.top
//    anchors.topMargin: mainRec.height*0.20
//    anchors.left:parent.left
//    anchors.leftMargin: mainRec.width*0.055
    //property for streetContainer
    property string upNextStreetName1: "Great American Peat American Peat American Peat American Peat American Plw"
    property string upNextStreetName2: "Some Secondary address"
    property string upNextStreetDistance: "0.25 Mi."
    property string upNextDirectionArrow //: applicationDirPath+ "/resource/images/RouteNavigation_images/detour_icn.png"
    //property for currentStreetContainer
     property string currentStreetName: "Street Name Dr."
    //property for routeInfoContainer
    property string timeText: "12:26 m"
    property string distanceText: "8.00 Mi."
    property string arrivalText: "01:54 PM"

    DefaultMouseArea
    {

    }


//    FontLoader{
//        id:sfRegular
//        name:  applicationDirPath+ "/resource/font/SF-UI-Display-Regular.otf"
//    }
//    FontLoader{
//        id:sfMedium
//        name:  applicationDirPath+ "/resource/font/SF-UI-Display-Medium.otf"
//    }
//    FontLoader{
//        id:nextGen
//        name:  applicationDirPath+ "/resource/font/LG_NexGen.ttf"
//    }

    Column{
        Rectangle{
            id:streetContainer
            color: DefaultStyle.colorDeepBlue1 //"#22566F"
            width: mainRec.width
            height: mainContainer.height/2
            Column{
                anchors.left: parent.left
                anchors.leftMargin:20
                anchors.fill: parent
                spacing: DefaultStyle.paddingXS //5//15
                anchors.verticalCenter: parent.verticalCenter
                Text {
                    id: maneuverIconLabel
                    anchors.horizontalCenter: parent.horizontalCenter
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.pixelSize: mainContainer.height*0.2//140
                    font.family: DefaultStyle.fontIcon.name //maneuverFont.name
                    color: DefaultStyle.colorCyan4 //"white"
                    text: upNextDirectionArrow//"i"
                    width:parent.width-20
                    elide:Text.Center

                }
//                Image {
//                    anchors.horizontalCenter: parent.horizontalCenter
//                    source: upNextDirectionArrow
//                    fillMode: Image.PreserveAspectFit
//                    width:parent.width-20
//                }

                Text{
                    text:upNextStreetName1
                    color: DefaultStyle.colorCyan2 //"#0ED9FD"
                    width:parent.width-20
                    elide:Text.ElideRight
                    font.family: DefaultStyle.fontRegular.name //sfRegular.name
                    font.pixelSize: mainContainer.height*0.05
                }
                Text{
                    text:upNextStreetName2
                    font.family: DefaultStyle.fontRegular.name //sfRegular.name
                    color:DefaultStyle.colorCyan2 //"#0ED9FD"
                    font.pixelSize: mainContainer.height*0.042
                    width:Text.text.length!=0? parent.width-20:0
                    elide:Text.ElideRight

                }
                Text{
                    text:upNextStreetDistance
                    color: DefaultStyle.colorWhite //"white"
                    font.family: DefaultStyle.fontRegular.name //sfRegular.name
                    font.pixelSize: mainContainer.height*0.05
                }
            }
        }
        Rectangle{
            id:currentStreetContainer
            color:DefaultStyle.colorDeepBlue2 //"#194255"
            width: mainRec.width
            height: mainContainer.height/5.8
            clip : true
            Column{
                spacing: DefaultStyle.paddingXS //5
                anchors.left:parent.left
                anchors.leftMargin: 20
                anchors.verticalCenter: parent.verticalCenter
                width:parent.width
                Text{
                    text:"Currently on:"
                    font.family: DefaultStyle.fontRegular.name //sfRegular.name
                    color: DefaultStyle.colorWhite //"white"
                    font.pixelSize: mainContainer.height*0.042
                }
                Text{
                    text:currentStreetName
                    font.family: DefaultStyle.fontRegular.name //sfRegular.name
                    color: DefaultStyle.colorWhite //"white"
                    font.pixelSize: mainContainer.height*0.042
                    width:parent.width-10//parent.width-30
                    elide:Text.ElideRight
                }
            }
        }
        Rectangle{
            id:routeInfoContainer
            color: DefaultStyle.colorDeepBlue3 //"#142831"
            width: mainRec.width
            height: mainContainer.height/2-mainContainer.height/6
            Row{
                anchors.bottom: parent.bottom
                Rectangle{
                    id:timeContainer
                    width:mainRec.width/3
                    color:DefaultStyle.colorDeepBlue3 //"#142831"
                    height: mainContainer.height/3-30
                    Column{
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing:DefaultStyle.paddingS //10
                        Image {
                            anchors.horizontalCenter: parent.horizontalCenter
                            id: clockIcon
                            source: applicationDirPath+ "/resource/images/RouteNavigation_images/clock_icn.png"
                        }
                        Text{
                            anchors.horizontalCenter: parent.horizontalCenter
                            text:timeText
                            font.family: DefaultStyle.fontRegular.name //sfRegular.name
                            color:DefaultStyle.colorWhite //"white"
                            font.pixelSize: mainContainer.height*0.042

                        }
                        Text{
                            anchors.horizontalCenter: parent.horizontalCenter
                            text:"Time"
                            color:DefaultStyle.colorDarkGray//"#7D7F7F"
                            font.family: DefaultStyle.fontRegular.name //sfRegular.name
                            font.pixelSize: mainContainer.height*0.035

                        }
                    }
                }
                Rectangle{
                    id:distanceContainer
                    width:mainRec.width/3
                    color:DefaultStyle.colorDeepBlue3 //"#142831"
                    height: mainContainer.height/3-30
                    Column{
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing:DefaultStyle.paddingS //10
                        Image {
                            anchors.horizontalCenter: parent.horizontalCenter
                            id: distanceIcon
                            source: applicationDirPath+ "/resource/images/RouteNavigation_images/distance_icn.png"
                            width: 27
                            height: 27
                            fillMode: Image.PreserveAspectFit

                        }
                        Text{
                            anchors.horizontalCenter: parent.horizontalCenter
                            text:distanceText
                            font.family: DefaultStyle.fontRegular.name //sfRegular.name
                            color:DefaultStyle.colorWhite //"white"
                            font.pixelSize: mainContainer.height*0.042

                        }
                        Text{
                            anchors.horizontalCenter: parent.horizontalCenter
                            text:"Distance"
                            color:DefaultStyle.colorDarkGray //"#7D7F7F"
                            font.family: DefaultStyle.fontRegular.name //sfRegular.name
                            font.pixelSize: mainContainer.height*0.035

                        }
                    }
                }
                Rectangle{
                    id:arrivalContainer
                    width:mainRec.width/3
                    color:DefaultStyle.colorDeepBlue3 //"#142831"
                    height: mainContainer.height/3-30
                    Column{
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing:DefaultStyle.paddingS //10
                        Image {
                            anchors.horizontalCenter: parent.horizontalCenter
                            id: destinationIcon
                            source: applicationDirPath+ "/resource/images/RouteNavigation_images/destination_icn.png"
                            width: 27
                            height: 27
                            fillMode: Image.PreserveAspectFit
                        }
                        Text{
                            anchors.horizontalCenter: parent.horizontalCenter
                            text:arrivalText
                            font.family: DefaultStyle.fontRegular.name //sfRegular.name
                            color:DefaultStyle.colorWhite //"white"
                            font.pixelSize: mainContainer.height*0.042

                        }
                        Text{
                            anchors.horizontalCenter: parent.horizontalCenter
                            text:"Arrival"
                            color:DefaultStyle.colorDarkGray //"#7D7F7F"
                            font.family: DefaultStyle.fontRegular.name //sfRegular.name
                            font.pixelSize: mainContainer.height*0.035

                        }
                    }
                }
            }
        }
    }
//    Image{
//        id:perspecitiveImage
//        anchors.left: parent.right
//        anchors.bottom: parent.bottom
//        anchors.leftMargin: 20
//        source: applicationDirPath+ "/resource/images/RouteNavigation_images/Perspective.png"
//        fillMode: Image.PreserveAspectFit
//        width: 80
//        height: 80
//        MouseArea{
//            anchors.fill: parent
//        }

//    }
//    Image{
//        id:overViewImage
//        anchors.left: parent.right
//        anchors.leftMargin: 20
//        anchors.bottom : perspecitiveImage.top
//        anchors.bottomMargin: 20
//        width: 80
//        height: 80
//        source: applicationDirPath+ "/resource/images/RouteNavigation_images/Overview.png"
//        fillMode: Image.PreserveAspectFit
//        MouseArea{
//            anchors.fill: parent
//        }

//    }
}

}
