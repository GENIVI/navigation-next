import QtQuick 2.3
import QtQuick.Window 2.2
import QtQuick.Layouts 1.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
//import QtQuick.Extras 1.4
import QtQuick.Controls.Private 1.0
import "style" 1.0

Item {

    Rectangle{
        id: routeGroupViewTitle
        anchors.top: parent.top
        anchors.topMargin: titleMargin
        Text{
            text: "Route"
            color: DefaultStyle.colorWhite //"#FFF"
            font.pixelSize: DefaultStyle.fontSize21 //22
        }
    }
    GroupBox {
        id: routeGroupView
        title: ""
        width: navigationTab.width
        //height: settingsController.height - (saveContainer.height + container.height + (margin * 10) )

        anchors.top: routeGroupViewTitle.bottom
        anchors.topMargin: groupBoxMargin

        RowLayout {
            anchors.fill: parent
            anchors.margins: margin
            ExclusiveGroup { id: routeGroup; objectName: "routeGroup"}
            NewRadioButton {
                text: "Fastest"
                exclusiveGroup: routeGroup
                objectName: "rbFastest"
            }
            NewRadioButton {
                text: "Shortest"
                exclusiveGroup: routeGroup                
                objectName: "rbShortest"
            }/*
            NewRadioButton {
                text: "ECO"
                exclusiveGroup: routeGroup                
                objectName: "rbECO"
            }*/
        }
    }

    Rectangle{
        id: avoidGroupTitle
        anchors.top: routeGroupView.bottom
        anchors.topMargin: titleMargin
        Text{
            text: "Avoid"
            color: DefaultStyle.colorWhite //"#FFF"
            font.pixelSize: DefaultStyle.fontSize21 //22
        }
    }

    GroupBox {
        id: avoidGroup
        //title: "<font color=\"white\">Show</font>"
        Layout.fillWidth: true
        width: parent.width
        title: ""
        anchors.top: avoidGroupTitle.bottom
        anchors.topMargin: groupBoxMargin

        GridLayout {
            id: gridLayout
            rows: 2
            flow: GridLayout.TopToBottom
            anchors.fill: parent
            anchors.margins: margin
            rowSpacing: DefaultStyle.paddingL //20
            NewCheckBox {
                id: checkBox_6
                text: qsTr("Toll Roads")                
                objectName: "chkTollRoads"
            }

            NewCheckBox {
                id: checkBox_7
                text: qsTr("Ferries")
                objectName: "chkFerries"
            }

            NewCheckBox {
                id: checkBox_8
                text: qsTr("Hov Lanes")
                objectName: "chkHovLanes"
            }

            NewCheckBox {
                id: checkBox_9
                text: qsTr("Highways")
                objectName: "chkHighways"
            }
        }
    }

    Rectangle{
        id: trafficGroupTitle
        anchors.top: avoidGroup.bottom
        anchors.topMargin: titleMargin
        Text{
            text: "Default Group"
            color: DefaultStyle.colorWhite //"#FFF"
            font.pixelSize: DefaultStyle.fontSize21 //22
        }
    }

    GroupBox {
        id: trafficGroup
        title: ""
        anchors.top: trafficGroupTitle.bottom
        anchors.topMargin: groupBoxMargin
        width: navigationTab.width
        //height: settingsController.height - (saveContainer.height + container.height + (margin * 10) )

        RowLayout {
            anchors.fill: parent
            anchors.margins: margin
            ExclusiveGroup { id: rbDefaultGroup }
            NewCheckBox {
                id: checkBox_10
                text: qsTr("Alerts Only")
                objectName: "chkAlerts"
            }

            NewCheckBox {
                id: checkBox_11
                text: qsTr("Navigation")                                
                objectName: "chkNavigation"
            }
        }
    }
}
