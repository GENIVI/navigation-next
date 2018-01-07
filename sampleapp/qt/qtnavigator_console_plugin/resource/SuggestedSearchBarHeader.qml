import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
//import QtQuick.Dialogs 1.2
import QtQuick.Controls.Styles 1.1

Rectangle{
    id:topMenuWidget
    color:"black"
    width:mainRec.width
    height:80
    Row{
        Rectangle{
            id:closeButton
            color:"transparent"
            width:topMenuWidget.width*0.10
            height:80
            Image{
                id:closeButtonImage
                source: applicationDirPath+ "/resource/images/close_icn_selected.png"
                fillMode: Image.PreserveAspectFit
                MouseArea{
                    anchors.fill: parent
                    //                    onClicked:loader.source = "HamburgerMenu.qml"
                }

            }
        }
        Rectangle{
            id:searchbarSpace
            color:"transparent"
            width:topMenuWidget.width*0.90
            height:80
            Column{
                anchors.fill: parent
                Rectangle{
                    width:parent.width
                    height: parent.height*0.8
                    color:"transparent"
                    Row{
                        anchors.fill: parent
                        anchors.left: parent.right
                        anchors.leftMargin: 30
                        Image{
                            id: searchIcnButton
                            anchors.verticalCenter:  parent.verticalCenter
                            source: applicationDirPath+ "/resource/images/search_icn.png"
                            fillMode: Image.PreserveAspectFit
                        }
                        TextField {
                            id: mainSearchLineEdit
                            anchors.verticalCenter:  parent.verticalCenter
                            width: parent.width*0.8
                            placeholderText: qsTr("Search")
                            //font.bold: true
                            font.pixelSize: 35
                            font.family: sfRegular.name
                            style: TextFieldStyle{
                                background: Rectangle{
                                    color:"transparent"
                                }
                                placeholderTextColor: "grey"
                                textColor: "white"
                            }
                        }
                        Image {
                            id: micButton
                            anchors.verticalCenter:  parent.verticalCenter
                            source: applicationDirPath+ "/resource/images/mic_icn.png"
                            fillMode: Image.PreserveAspectFit
                            MouseArea{
                                anchors.fill: parent
                            }
                        }
                    }
                }
                Image{
                    id: spaceBarUnderline
                    source: applicationDirPath+ "/resource/images/search_bottomline.png"
                    width:parent.width*0.87
                    anchors.left:parent.left
                    anchors.leftMargin: 25
                }
            }
        }
    }
}
