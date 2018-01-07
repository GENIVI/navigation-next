import QtQuick 2.0
import test.map 1.0
//import com.comtech 1.0


Item {
    width: 1280
    height: 720

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

    MapView {
        id: renderer
        anchors.fill: parent
        anchors.margins: 10
        workFolder: "/opt/comtech/res/console"
        avatarPosition.x: -118.25
        avatarPosition.y: 34.05
//        avatarHeading: 0.0
        tilt: 20.0
        zoom: 16.0
//        isFollowMe: true
    }


//    // Just to show something interesting
//    SequentialAnimation {
//        PauseAnimation { duration: 2000 }

//        ParallelAnimation {
//            NumberAnimation { target: renderer; property: "avatarPosition.y"; to: 33.624; duration: 5000; easing.type: Easing.InOutCubic }
//            NumberAnimation { target: renderer; property: "avatarHeading"; to: 80.0; duration: 5000; easing.type: Easing.InOutCubic }
//        }

//        running: true
//        loops: Animation.Infinite
//        onLoopsChanged: {
//            renderer.avatarPosition.avatarPosition.x = -117.689;
//            renderer.avatarPosition.avatarPosition.y = 33.604;
//            renderer.avatarPosition.avatarHeading = 0.0;
//        }
//    }

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
