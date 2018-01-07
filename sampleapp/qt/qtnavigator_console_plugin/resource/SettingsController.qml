import QtQuick 2.3
import QtQuick.Window 2.2
import QtQuick.Layouts 1.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
//import QtQuick.Extras 1.4
import QtQuick.Particles 2.0
import "style" 1.0

Rectangle {

    property int margin: 5
    property int titleMargin: 10
    property int groupBoxMargin: 10

    signal settingControllerClosed()
    signal saveButtonClicked()
    signal tabIndexChanged(int currentIndex)
    signal clearData()
    id: settingsController
    visible: true

    width: parent.width
    height: parent.height

    DefaultMouseArea
    {

    }


    //FontLoader { id: sfDisplayMedium; source: applicationDirPath + "/resource/font/SF-UI-Display-Medium.otf"}
    //FontLoader { id: sfDisplayRegular; source: applicationDirPath + "/resource/font/SF-UI-Display-Regular.otf" }

    Rectangle {
        id: mainLayout
        anchors.fill: parent
        color: DefaultStyle.colorBlack2 //"black"

        Rectangle {
            id: header
            height: 80
            width: settingsController.width
            color: DefaultStyle.colorBlack1 //"black"

            Image {
                id: closeimage
                source: applicationDirPath + "/resource/images/listview_resources/close_icn.png"
                anchors{
                    left: parent.left
                    leftMargin: 0

                }
                MouseArea{
                    anchors.fill: parent
                    onClicked: {
                        settingControllerClosed();
                    }
                }
            }

            TitleLabel {
                objectName: "settingTitle"
                text: "Navigation Options"
                shadow: false
                anchors.left: closeimage.right
                anchors.leftMargin: 0
                height: closeimage.height
            }
        }

        Rectangle {
            id: container
            anchors.top: header.bottom
            width: settingsController.width
            height: settingsController.height - header.height
            color: DefaultStyle.colorBlack2 //"black"
            opacity: DefaultStyle.midOpacity //0.8

            TabView {
                id: tabView

                anchors.bottom: saveContainer.top
                anchors.bottomMargin: 100
                anchors.fill: parent
                anchors.margins: 4

                Tab {
                    id: navigationTab
                    title: "Navigation"
                    anchors.margins: margin
                    source: "NavigationTab.qml"
                    anchors.bottomMargin: 25
                }

                Tab {
                    id: routesTab
                    title: "Routes"
                    anchors.margins: margin
                    source: "RoutesTab.qml"
                }

                Tab {
                    id: searchTab
                    title: "Search"
                    anchors.margins: margin
                    source: "SearchTab.qml"
                }

                Tab {
                    id: miscTab
                    title: "Misc"
                    anchors.margins: margin
                    source: "MiscTab.qml"
                }
                style: tabStyleView
                onCurrentIndexChanged: tabIndexChanged(tabView.currentIndex)
                //style: TabViewStyle
            }

            Connections {
                //                target: miscTab.item
                //                onClearButtonClicked: clearData()
            }
        }
        Item {
            id: saveContainer
            objectName: "objButton"
            height: 80
            Layout.fillWidth: true
            //anchors.top: container.bottom
            //anchors.topMargin: 20
            anchors.bottom: container.bottom
            anchors.bottomMargin: 160 //205
            anchors.left: bottomRow.left
            anchors.leftMargin: margin
            RowLayout {
                id: bottomRow
                height: 22
                anchors.fill: parent
                anchors.margins: margin

                    /*Button {
                        id: saveButton
                        text: "Save"
                        style: buttonStyleView
                    }*/
                    NewButton {
                        text: "Save"
                        MouseArea{
                            anchors.fill: parent
                            onClicked: {
                                saveButtonClicked();
                            }
                        }

                }
                NewButton{
                    //id:button1
                    objectName: "cleanData"
                    implicitWidth: 200
                    text : "Master Clear"
                    visible: false
                    MouseArea{
                        anchors.fill: parent
                        onClicked: {
                            clearData();
                        }
                    }
                }
            }
        }

        Component{
            id: textFieldStyleView
            TextFieldStyle{
                textColor: DefaultStyle.colorWhite //"#FFF"
                placeholderTextColor: DefaultStyle.colorWhite //"#FFF"
                background: Rectangle {
                    radius: DefaultStyle.rectRadiusXS //3
                    color: DefaultStyle.colorTransparent //"transparent"
                    //implicitWidth: 100
                    implicitHeight: 50
                    border.color: DefaultStyle.colorWhite //"#FFF"
                    border.width: DefaultStyle.borderWidth1 //1
                }
            }
        }

        Component{
            id: buttonStyleView
            ButtonStyle {
                background: Rectangle {
                    color: DefaultStyle.colorTransparent //"transparent"
                    implicitWidth: 200
                    implicitHeight: 50
                    border.width: DefaultStyle.borderWidth2 //2
                    border.color: DefaultStyle.colorLightGreen //"#8FED31"
                    radius: DefaultStyle.rectRadiusS //4
                }
                label: Text
                {
                    text: control.text
                    font.pixelSize: DefaultStyle.fontSize24 //24
                    font.family: DefaultStyle.fontRegular.name //"DejaVu Sans"
                    anchors.centerIn: parent
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    anchors.fill: parent
                    color: DefaultStyle.colorLightGreen //"#8FED31"
                    //anchors.bottomMargin: 50
                }
            }
        }

        Component{
            id: checkboxStyleView
            CheckBoxStyle
            {
                indicator: Image
                {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    source: applicationDirPath + "/resource/images/Settings_images/unchecked.png"
                    width: 30
                    height: 30
                    Image
                    {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        source: enabled ? applicationDirPath + "/resource/images/Settings_images/checkmark_checked.png" : applicationDirPath + "/resource/images/Settings_images/unchecked.png"
                        visible: control.checked
                        width: 30
                        height: 30
                    }
                }
                spacing: DefaultStyle.padding//15
                label: Text
                {
                    text: control.text
                    font.pixelSize: DefaultStyle.fontSize21 //22
                    font.family: DefaultStyle.fontRegular.name //"DejaVu Sans"
                    horizontalAlignment: Text.left
                    color: DefaultStyle.colorWhite //"#FFFFFF"
                    anchors.bottomMargin: 50
                }
            }
        }


        Component{
            id: radioButtonStyleView
            RadioButtonStyle
            {
                indicator: Image
                {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    source: applicationDirPath + "/resource/images/Settings_images/unchecked.png"
                    width: 30
                    height: 30
                    Image
                    {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        source: enabled ? applicationDirPath + "/resource/images/Settings_images/radio_checked.png" : applicationDirPath + "/resource/images/Settings_images/unchecked.png"
                        visible: control.checked
                        width: 30
                        height: 30
                    }
                }
                spacing: DefaultStyle.padding//15
                label: Text
                {
                    text: control.text
                    font.pixelSize: DefaultStyle.fontSize21 //22
                    font.family: DefaultStyle.fontRegular.name //"DejaVu Sans"
                    horizontalAlignment: Text.left
                    color: DefaultStyle.colorWhite //"#FFFFFF"
                }
            }
        }

        Component{
            id: tabStyleView
            TabViewStyle {
                frameOverlap: 0
                frame: Rectangle {
                    color: DefaultStyle.colorTransparent //"transparent"
                    //border.color: "#898989"
                    /*Rectangle {
                     anchors.fill: parent ;
                     anchors.margins: 1 ;
                     border.color: "white" ;
                     color: "transparent"
                 }*/
                }
                tab: Rectangle {
                    color: DefaultStyle.colorTransparent //"transparent"
                    border.width: DefaultStyle.borderWidth2 //2
                    //radius: 1
                    border.color: styleData.selected ? DefaultStyle.colorCyan2:DefaultStyle.colorTransparent //"#0ED9FD" : "transparent"
                    implicitWidth: (parent.control.width + 3) / 4
                    implicitHeight: 50

                    Rectangle {
                        width: parent.width
                        height: 2
                        color:DefaultStyle.colorCyan2 //"#0ED9FD"
                        anchors.bottom: parent.bottom
                    }



                    /*Rectangle {
                     id: borderLeft
                     width: 1
                     height: parent.height
                     anchors.left: parent.left
                     color: "#0ED9FD"
                     visible: styleData.selected
                 }
                 Rectangle {
                     id: borderRight
                     width: 1
                     height: parent.height
                     anchors.left: parent.right
                     color: "#0ED9FD"
                     visible: styleData.selected
                 }
                 BorderImage {
                    id: image
                    anchors.fill: parent
                    source: styleData.selected ? "images/tab_selected.png" : "images/tab.png"
                    border.left: 30
                    smooth: false
                    border.right: 30
                 }*/
                    Text {
                        id: text1
                        anchors.centerIn: parent
                        text: styleData.title.split("|")[0]
                        color: styleData.selected ? DefaultStyle.colorCyan2:DefaultStyle.colorWhite //"#0ED9FD" : "#FFFFFF"
                        font.pixelSize: DefaultStyle.fontSize21 //22
                    }
                }

            }
        }


    }
}

