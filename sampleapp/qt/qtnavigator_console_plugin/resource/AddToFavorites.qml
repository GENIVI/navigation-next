import QtQuick 2.4
import QtQuick.Window 2.2
import QtQuick.Layouts 1.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
//import QtQuick.Extras 1.4
import QtGraphicalEffects 1.0

ApplicationWindow {

    property int margin: 11

    id: favorites_save
    visible: true
    opacity: 1
    width: 1024
    height: 745

    Rectangle {
        id: header
        height: favorites_save.height/10
        width:favorites_save.width
        color: "black"

        Image {
            id: closeimage
            source: "qrc:/resource/images/listview_resources/close_icn.png"
            anchors{
                left: parent.left
                leftMargin: 0

            }
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    //routeOptionWidgetClosed();
                    Qt.quit()
                }
            }
        }

        TitleLabel {
           text: "Add to Favorites"
           shadow: true
           anchors.left: closeimage.right
           anchors.leftMargin: 0
           height: closeimage.height
        }
    }

    DropShadow {
        anchors.fill: header
        horizontalOffset: 0
        verticalOffset: 4
        radius: 16
        samples: 33
        color: "#0ED9FD"
        source: header
        transparentBorder: true
    }

    Rectangle{
        id:container
        width: favorites_save.width
        height: favorites_save.height-header.height
        anchors.top: header.bottom
        anchors.topMargin: 4
        color: "#08202C"
        //color: "transparent"
        anchors.margins: margin

        Rectangle{
            id: boxContainer
            anchors.margins: margin
            width: parent.width * 0.75
            height: parent.height * 0.75
            color: "transparent"
//            border.color: "red"
//            border.width: 2
            Layout.fillWidth: true
            anchors {
                horizontalCenter: parent.horizontalCenter;
                top: parent.top;
                topMargin: 25
            }

            GridLayout {
                id: gridLayout
                rows: 3
                flow: GridLayout.TopToBottom
                width: parent.width
                anchors.margins: margin
                columnSpacing: 20
                rowSpacing: 20

                /*Label {
                    text: "Name "
                    id: lblName
                    font.pixelSize: 22
                    LinearGradient  {
                        anchors.fill: lblName
                        source: lblName
                        gradient: Gradient {
                            GradientStop { position: 0; color: "#0ed9fd" }
                            GradientStop { position: 1; color: "#ddf8ff" }
                        }
                    }
                }*/
                NewLabel { text: "Name "; objectName: "lblName" ; }
                NewLabel { text: "Phone "; objectName: "lblPhone" }
                NewLabel { text: "Address "; objectName: "lblAddress" }


                //Label { text: "Phone: "; color: "#FFF" }
                //Label { text: "Address: "; color: "#FFF" }



                TextField {
                    id: name
                    Layout.fillWidth: true;
                    style: textFieldStyleView
                    objectName: "txtName"


                    /*DropShadow {
                            anchors.fill: name
                            horizontalOffset: 5
                            verticalOffset: 5
                            radius: 6
                            samples: 25
                            color: "#0ed9fd"
                            source: name
                        }*/

                    /*Glow {
                        cached: false
                        anchors.fill: parent
                        radius: 8
                        samples: 25
                        color: "#0ed9fd"
                        source: name
                    }*/

                }



                TextField {
                    id: phone;                    
                    Layout.fillWidth: true;
                    style: textFieldStyleView
                }

                Label {
                    id: addressField;
                    text: "Joe's Coffee Shop, 896 Sutter St, \nSan Francisco, CA 94109, \nUnited States. ";
                    color: "#FFF"                    
                }
            }

            RowLayout {
                id: bottomRow
                //height: 22
                anchors.fill: parent
                anchors.margins: margin
                anchors.left: parent.left
                anchors.leftMargin: 100

                /*Button {
                    id: saveButton
                    text: "Save"
                    style: buttonStyleView
                }*/
                NewButton {
                    text: "Save"
                    //style: buttonStyleView
                }
            }

        }
    }


    Component{
        id: textFieldStyleView
        TextFieldStyle{
            textColor: "#FFF"
            placeholderTextColor: "#FFF"
            background: Rectangle {
                radius: 3
                color: "transparent"
                //implicitWidth: 100
                implicitHeight: 50
                border.color: "#FFF"
                border.width: 1
            }
        }
    }

    Component{
        id: buttonStyleView
        ButtonStyle {
            background: Rectangle {
                color: "transparent"
                implicitWidth: 200
                implicitHeight: 50
                border.width: 2
                border.color: "#8FED31"
                radius: 4
            }
            label: Text
            {
                text: control.text
                font.pixelSize: 24
                font.family: "DejaVu Sans"
                anchors.centerIn: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                anchors.fill: parent
                color: "#8FED31"
                //anchors.bottomMargin: 50
            }
        }
    }

}
