import QtQuick 2.4
import QtQuick.Layouts 1.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
//import QtQuick.Extras 1.4
import QtGraphicalEffects 1.0
import "style" 1.0

Rectangle {

    property int margin: 11
    property real routeHeight: mainWidget / 5.75

    id: navigationRouteController
    visible: true
    opacity: 1
    width: 1024
    height: 768
    border.width: DefaultStyle.borderWidth1 //1
    color: DefaultStyle.colorDeepBlue3 //"#142831"

    signal routeFirstClicked()
    signal routeSecondClicked()
    signal routeThirdClicked()
    signal routeFirstGoClicked()
    signal routeSecondGoClicked()
    signal routeThirdGoClicked()

    DefaultMouseArea
    {

    }


    Rectangle {
        id: mainWidget
        width: parent.width //*0.3//temp fix
        height: parent.height //- (parent.height * 2 / 7)//temp fix
        border.color: DefaultStyle.colorWhite//"#FFFFFF"
        color: DefaultStyle.colorDeepBlue3//"#142831"


        ColumnLayout {
            anchors.fill: parent
            anchors.margins: margin
            spacing: DefaultStyle.paddingL //20

            RouteGoButton {                
                objectName: "route1"
                distanceObjectName: "distance1"
                distanceText: "1.58 Miles"
                routeText: "Route 1"
                goObjectName: "go1"
                routeColor: DefaultStyle.colorOrange //"#FF7C00"
                buttonHeight: parent.height / 5.25
                buttonWidth: parent.width
                onGoButtonClicked: routeFirstGoClicked()
                onItemClicked: routeFirstClicked()
            }

            RouteGoButton {
                objectName: "route2"
                distanceObjectName: "distance2"
                distanceText: "2.14 Miles"
                routeText: "Route 2"
                goObjectName: "go2"
                routeColor: DefaultStyle.colorCyan4 //"#00DEFF"
                buttonHeight: parent.height / 5.25
                buttonWidth: parent.width
                onGoButtonClicked: routeSecondGoClicked()
                onItemClicked: routeSecondClicked()
            }

            RouteGoButton {
                objectName: "route3"
                distanceObjectName: "distance3"
                distanceText: "2.15 Miles"
                routeText: "Route 3"
                goObjectName: "go3"
                routeColor: DefaultStyle.colorLightGreen1//"#00F938"
                buttonHeight: parent.height / 5.25
                buttonWidth: parent.width
                onGoButtonClicked: routeThirdGoClicked()
                onItemClicked: routeThirdClicked()
            }

            //Layout.fillWidth: true

            /*Rectangle {
                id: route1
                objectName: "route1"
                Layout.fillWidth: true
                height: parent.height / 5.25
                border.color: "#FFFFFF"
                border.width: 1
                radius: 50
                color: "black"

                Text {
                    id: distance1
                    objectName: "distance1"
                    text: qsTr("1.58 Miles")
                    anchors.top: parent.top
                    anchors.topMargin: 9
                    anchors.left: parent.left
                    anchors.leftMargin: 30
                    color: "white"
                    font {
                        family: sfDisplayMedium.name;
                        pixelSize: 28
                    }
                }

                Text {
                    id: routeType1
                    objectName: "routeType1"
                    text: qsTr("Route1")
                    color: "#FF7C00"
                    anchors.left: parent.left
                    anchors.leftMargin: 30
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 9
                    font {
                        family: sfDisplayRegular.name
                        pixelSize: 28
                    }
                }

                Button {
                    id: go1
                    objectName: "go1"
                    anchors.right: parent.right
                    anchors.rightMargin: 30
                    anchors.top: parent.top
                    anchors.topMargin: 14
                    text: "GO"
                    style: ButtonStyle {
                        background: Rectangle {
                            implicitWidth: route1.height - 30
                            implicitHeight: route1.height - 30
                            border.width: control.activeFocus ? 4 : 3
                            border.color: "#FF7C00"
                            radius: 40
                            color: "black"
                        }
                        label: Text {
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            color: "white"
                            text: go1.text
                            font {
                                family: sfDisplayMedium.name;
                                pixelSize: 28;
                            }
                        }
                    }
                }
            }*/

            /*Rectangle {
                id: route2
                objectName: "route2"
                Layout.fillWidth: true
                height: parent.height / 5.25
                border.color: "#FFFFFF"
                border.width: 1
                radius: 50
                color: "black"
            }

            Rectangle {
                id: route3
                objectName: "route3"
                Layout.fillWidth: true
                height: parent.height / 5.25
                border.color: "#FFFFFF"
                border.width: 1
                radius: 50
                color: "black"
            }*/
        }
    }
}
