import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.2
import "style" 1.0

Item {
    property alias objectName: route1.objectName
    property alias distanceObjectName: distance.objectName
    property alias distanceText: distance.text
    property alias routeText: routeType.text
    property alias goObjectName: go.objectName
    property string routeColor
    property string buttonHeight
    property string buttonWidth

    signal goButtonClicked()
    signal itemClicked()

    implicitHeight: route1.height
    implicitWidth: route1.width
    Rectangle {
        id: route1        
        //Layout.fillWidth: true
        height: buttonHeight
        width: buttonWidth
        border.color: DefaultStyle.colorWhite //"#FFFFFF"
        border.width: DefaultStyle.borderWidth1 //1
        radius:DefaultStyle.rectRadiusXL // 50
        color: DefaultStyle.colorBlack1 //"black"

        Text {
            id: distance
            //objectName: "distance1"
            //text: qsTr("1.58 Miles")
            anchors.top: route1.top
            anchors.topMargin: 9
            anchors.left: route1.left
            anchors.leftMargin: 30
            color: DefaultStyle.colorWhite //"white"
            font {
                family: DefaultStyle.fontMedium.name //sfDisplayMedium.name;
                pixelSize: buttonHeight * 0.3//28
            }
        }

        Text {
            id: routeType
            objectName: "routeType"
            color: routeColor
            anchors.left: route1.left
            anchors.leftMargin: 30
            anchors.bottom: route1.bottom
            anchors.bottomMargin: 9
            font {
                family: DefaultStyle.fontMedium.name //sfDisplayRegular.name
                pixelSize: buttonHeight * 0.3//28
            }
        }

        Button {
            id: go
            anchors.right: route1.right
            anchors.rightMargin: 30
            anchors.top: route1.top
            anchors.topMargin: 14
            text: "GO"
            onClicked: goButtonClicked()
            style: ButtonStyle {
                background: Rectangle {
                    implicitWidth: route1.height - 30
                    implicitHeight: route1.height - 30
                    border.width: DefaultStyle.borderWidth3 //3
                    border.color: routeColor
                    radius: DefaultStyle.rectRadiusL //40
                    color: DefaultStyle.colorBlack1 //"black"
                }
                label: Text {
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    color: DefaultStyle.colorWhite //"white"
                    text: go.text
                    font {
                        family: DefaultStyle.fontMedium.name //sfDisplayRegular.name
                        pixelSize: buttonHeight * 0.3//28;
                    }
                }
            }

        }
        MouseArea {
            anchors.fill: parent
            onClicked: itemClicked()
        }
    }

}
