import QtQuick 2.0
import test.map 1.0
//import com.comtech 1.0
import QtQuick 2.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

Item {
    width: 1280
    height: 720
    id: baseParent
    signal hamburgerMenuButtonClicked()
    signal settingsButtonClicked()

    FontLoader{
        id:sfRegular
        name:  applicationDirPath+ "/resource/font/SF-UI-Display-Regular.otf"
    }
    FontLoader{
        id:sfMedium
        name:  applicationDirPath+ "/resource/font/SF-UI-Display-Medium.otf"
    }

    Loader{
        id:loader
        anchors.fill: parent
    }

    Rectangle {
        id:topMenuWidget
        color:"red"
        anchors.top: parent.top
        width:parent.width
        height:100

        Row {
            Rectangle {
                id:hamburgerMenuSpace
                color:"black"
                width:topMenuWidget.width*0.25
                height:100

                Image{
                    id:hamburgerMenuButton
                    source: applicationDirPath+ "/resource/images/hamburger_icn.png"
                    fillMode: Image.PreserveAspectFit
                    MouseArea{
                        anchors.fill: parent
                        onClicked: {
                            hamburgerMenuButtonClicked();
                        }
    //                    onClicked:loader.source = "HamburgerMenu.qml"
                    }

                }
            }
            Rectangle{
                id:searchbarSpace
                color:"black"
                width:topMenuWidget.width*0.5
                height:100
                Column{
                    anchors.fill: parent
                    Rectangle{
                        width:parent.width
                        height: parent.height*0.8
                        color:"black"
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
                                placeholderText: qsTr("Search")
                                font.bold: true
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
                        width:parent.width*0.9
                    }
                }
            }
            Rectangle {
                color:"black"
                width:topMenuWidget.width*0.25
                height:100
                id:settingsBtnContainer

                Image {
                    anchors.right: parent.right
                    id: settingsButton
                    source: applicationDirPath+ "/resource/images/settings_icn.png"
                    fillMode: Image.PreserveAspectFit
                    MouseArea{
                        anchors.fill: parent
                        onClicked: settingsButtonClicked()
    //                    function showSettings(){
    //                        // hamburgerMenuButton.source = applicationDirPath+ "/resource/images/close_icn.png"
    //                        messageDialog.show(qsTr("setting Pending"))
    //                    }
                    }
                }
            }
        }
    }





    // The checkers background
    ShaderEffect {
        id: tileBackground
        anchors.fill: parent

        property real tileSize: 16
        property color color1: Qt.rgba(0.9, 0.9, 0.9, 1);
        property color color2: Qt.rgba(0.85, 0.85, 0.85, 1);

        property size pixelSize: Qt.size(width / tileSize, height / tileSize);

        fragmentShader:
            "
            uniform lowp vec4 color1;
            uniform lowp vec4 color2;
            uniform highp vec2 pixelSize;
            varying highp vec2 qt_TexCoord0;
            void main() {
                highp vec2 tc = sign(sin(3.14152 * qt_TexCoord0 * pixelSize));
                if (tc.x != tc.y)
                    gl_FragColor = color1;
                else
                    gl_FragColor = color2;
            }
            "
    }

    Rectangle {
        anchors.top: topMenuWidget.bottom
        anchors.topMargin: 10
        width: parent.width
        height: parent.height - 100

        MapView {
            id: renderer
            anchors.fill: parent
            anchors.margins: 0

            workFolder: "/opt/comtech/res/console"

    //        // The transform is just to show something interesting..
    //        transform: [
    //            Rotation { id: rotation; axis.x: 0; axis.z: 0; axis.y: 1; angle: 0; origin.x: renderer.width / 2; origin.y: renderer.height / 2; },
    //            Translate { id: txOut; x: -renderer.width / 2; y: -renderer.height / 2 },
    //            Scale { id: scale; },
    //            Translate { id: txIn; x: renderer.width / 2; y: renderer.height / 2 }
    //        ]

    //        Behavior on opacity { NumberAnimation { duration: 500 } }
    //        opacity: 0
    //        Component.onCompleted: renderer.opacity = 1;
        }

        Rectangle {
            id: labelFrame
            anchors.margins: -10
            radius: 5
            color: "white"
            border.color: "black"
            opacity: 0.8
            anchors.fill: label
        }

        Text {
            id: label
            anchors.bottom: renderer.bottom
            anchors.left: renderer.left
            anchors.right: renderer.right
            anchors.margins: 20
            wrapMode: Text.WordWrap
            text: "The blue rectangle with the vintage 'Q' is an FBO, rendered by the application in a dedicated background thread. The background thread juggles two FBOs, one that is being rendered to and one for displaying. The texture to display is posted to the scene graph and displayed using a QSGSimpleTextureNode."
        }

    }


//    // Just to show something interesting
//    SequentialAnimation {
//        PauseAnimation { duration: 5000 }
//        ParallelAnimation {
//            NumberAnimation { target: scale; property: "xScale"; to: 0.6; duration: 1000; easing.type: Easing.InOutBack }
//            NumberAnimation { target: scale; property: "yScale"; to: 0.6; duration: 1000; easing.type: Easing.InOutBack }
//        }
//        NumberAnimation { target: rotation; property: "angle"; to: 80; duration: 1000; easing.type: Easing.InOutCubic }
//        NumberAnimation { target: rotation; property: "angle"; to: -80; duration: 1000; easing.type: Easing.InOutCubic }
//        NumberAnimation { target: rotation; property: "angle"; to: 0; duration: 1000; easing.type: Easing.InOutCubic }
//        NumberAnimation { target: renderer; property: "opacity"; to: 0.5; duration: 1000; easing.type: Easing.InOutCubic }
//        PauseAnimation { duration: 1000 }
//        NumberAnimation { target: renderer; property: "opacity"; to: 0.8; duration: 1000; easing.type: Easing.InOutCubic }
//        ParallelAnimation {
//            NumberAnimation { target: scale; property: "xScale"; to: 1; duration: 1000; easing.type: Easing.InOutBack }
//            NumberAnimation { target: scale; property: "yScale"; to: 1; duration: 1000; easing.type: Easing.InOutBack }
//        }
//        running: true
//        loops: Animation.Infinite
//    }





//    Button {
//        id: buttonText
//        width: 170
//        height: 50

//        style: ButtonStyle {
//            background: Rectangle {
//                color: "green"
//                //implicitWidth: 200
//                //implicitHeight: 50
//                border.width: 2
//                border.color: "#8FED31"
//                radius: 4
//            }
//            label: Component{
//                Text {
//                    text: buttonText.text
//                    font.pixelSize: 24
//                    font.family: "DejaVu Sans"
//                    anchors.centerIn: parent
//                    verticalAlignment: Text.AlignVCenter
//                    horizontalAlignment: Text.AlignHCenter
//                    anchors.fill: parent
//                    color: "#8FED31"
//                    wrapMode: Text.WordWrap
//                    clip: true
//                }
//            }
//        }
//        onClicked: newbutton.clicked()
//    }


}
