import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.1
import "style" 1.0

Rectangle {
    id: nearMeSubMenu

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

    height: parent.height
    width:parent.width
    color: DefaultStyle.colorBlack1 //"#000000"
    signal nearMeItemsTriggered(string name, string code)
    signal closeNearMeMenu()

    onNearMeItemsTriggered: {
        console.log(name + ' has ' + code);
    }

    Image {
        id: closeButton
        source: applicationDirPath+ "/resource/images/Detailscreen_images/cancel_btn.png"
        width: 64
        height: 54
        anchors.right: parent.right
        visible: isNearByPinLocation
        MouseArea {
            anchors.fill: parent
            onClicked: {
                console.log("closeNearMeMenu")
                closeNearMeMenu()
            }
        }
    }

//    Rectangle {
//        id: scrollRect
//        color: "#000000"
//        anchors.fill: parent
        ScrollView {
            id: listScrollView
            width: parent.width
            height: parent.height - (parent.height/9 * 2)
            flickableItem.interactive: true
            anchors.top: isNearByPinLocation ? closeButton.bottom : parent.top
            ListView {
                id: listView1
                width: parent.width
                height: parent.height
                delegate: Item {
                    width: parent.width
                    height: 82
                    Rectangle{
                        width: parent.width
                        height: 82
                        color: DefaultStyle.colorBlack1 //"#000000"
                        border.color: DefaultStyle.colorBlack5 //"#273133"
                        border.width: DefaultStyle.borderWidth1 //1
                        Row {
                            id: row1
                            spacing: DefaultStyle.paddingS //10
                            Image {
                                id: menuImage
                                source: applicationDirPath+ "/resource/images/Homescreen_images/"+ model.modelData.code + '.png'
                                width: nearMeSubMenu.width*0.2 //82
                                height: nearMeSubMenu.width*0.2 //80
                            }
                            Text {
                                text: model.modelData.name
                                color: DefaultStyle.colorWhite //"white"
                                font.family: DefaultStyle.fontRegular.name //sfRegular.name
                                font.pixelSize: Math.round(nearMeSubMenu.width*0.08/* / 12*/)//22
                                anchors.verticalCenter: menuImage.verticalCenter
                                anchors.left: menuImage.right
                            }

                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            console.log(model.modelData.name + " clicked" );
                            console.log(parent.width + " nearMeSubMenu");
                            console.log(nearMeSubMenu.width + " nearMeSubMenu");
                            nearMeSubMenu.nearMeItemsTriggered(model.modelData.name, model.modelData.code);
                        }
                    }
                }
                model: nearMeModel
            }
            style: ScrollViewStyle {
                transientScrollBars: true
                handle: Item {
                    implicitWidth: 14
                    implicitHeight: 26
                    Rectangle {
                        color: DefaultStyle.colorBlack5 //"#424246"
                        anchors.fill: parent
                        anchors.topMargin: 6
                        anchors.leftMargin: 4
                        anchors.rightMargin: 4
                        anchors.bottomMargin: 6
                    }
                }
                scrollBarBackground: Item {
                    implicitWidth: 14
                    implicitHeight: 26
                }
            }
        }
    //}
}
