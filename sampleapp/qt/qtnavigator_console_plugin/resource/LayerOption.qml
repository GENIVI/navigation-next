import QtQuick 2.3
import QtQuick.Layouts 1.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import "style" 1.0

Rectangle {

//    color: "black"
    height: parent.height//720
    width: parent.width //1280

    color: DefaultStyle.colorTransparent //"transparent"

    DefaultMouseArea
    {

    }

    Rectangle {
        anchors.fill: parent
        color: DefaultStyle.colorBlack1 //"black"
        opacity: DefaultStyle.midOpacity //0.8
    }

    signal layerOptionWidgetClosed()
    signal viewOptionClicked(bool statusCheck)
    signal landmarkBuildingsClicked(bool statusCheck)
    signal poiClicked(bool statusCheck)
    signal satelliteClicked(bool statusCheck)
    signal trafficClicked(bool statusCheck)


    Image {
        id: closeimage
        source: applicationDirPath + "/resource/images/listview_resources/close_icn.png"
        anchors{
            top: parent.top
            topMargin: 6
            right: parent.right
            rightMargin: 5
        }
        MouseArea{
            anchors.fill: parent
            onClicked: {
                layerOptionWidgetClosed();
            }
        }
    }
    Rectangle {
        color: DefaultStyle.colorTransparent //"transparent"
        height: layerOption.height + 40
        width: 420
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter

        GridLayout {
            id: layerOption
            columns: 1
            rowSpacing: DefaultStyle.paddingL //25
            columnSpacing: DefaultStyle.paddingL //20
            flow: GridLayout.TopToBottom

            NewCheckBox {
                id: box1
                objectName: "chk3DView"
                text: qsTr("3D View")
                onCheckedChanged:{
                    viewOptionClicked(box1.checked)
                }
            }

            NewCheckBox {
                id: box2
                objectName: "chk3DLandMarks"
                text: qsTr("3D Landmarks & Building")
                onCheckedChanged:{
                    landmarkBuildingsClicked(box2.checked)
                }
            }

            NewCheckBox {
                id: box3
                objectName: "chkPOI"
                text: qsTr("Points of Interest")
                onCheckedChanged:{
                    poiClicked(box3.checked)
                }
            }

            NewCheckBox {
                id: box4
                objectName: "chkSatellite"
                text: qsTr("Satellite")
                onCheckedChanged:{
                    satelliteClicked(box4.checked)
                }
            }

            NewCheckBox {
                id: box5
                objectName: "chkTraffic"
                text: qsTr("Traffic")
                onCheckedChanged:{
                    trafficClicked(box5.checked)
                }
            }
        }
    }

}
