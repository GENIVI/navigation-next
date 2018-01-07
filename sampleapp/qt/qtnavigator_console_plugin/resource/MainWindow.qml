import QtQuick 2.4

Rectangle {
    property alias mouseArea: mouseArea

    width: 360
    height: 360

    /*MouseArea {
        id: mouseArea
        anchors.fill: parent
    }*/

    Text {
        anchors.centerIn: parent
        text: "Hello World"
    }

    Rectangle{
                color:"black"
                width:250
                height:100
                id:settingsBtnContainer
                objectName: "btnLogin"

                /*Image {
                    anchors.right: parent.right
                    id: settingsButton
                    source: "images/settings_icn.png"
                    fillMode: Image.PreserveAspectFit
                    MouseArea{
                        anchors.fill: parent
                        onClicked: {
                            //settingControllerClosed();
                        }
    //                    onClicked: showSettings()
    //                    function showSettings(){
    //                        // hamburgerMenuButton.source = "images/close_icn.png"
                        //                        messageDialog.show(qsTr("setting Pending"))
                        //                    }
                    }
                }*/
    }
}
