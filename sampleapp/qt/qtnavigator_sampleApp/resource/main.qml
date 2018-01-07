import QtQuick 2.3
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import com.locationstudio.qtnavigator.console 1.0

Window {
    id: root
    visible: true
    width: 1024
    height: 768
//    Rectangle {
//        anchors.fill: parent
//        color: "black"
//    }

    ConsoleView {
        id: consoleview
        width: parent.width
        height: parent.height
        workFolder: "/opt/locationstudio/res/console"
        zoom: 17
        tilt: 45
        isFollowMe: true
        //token : "<YOUR API Key>"
    }
    Loader {
        id: keyboardLoader
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        source: "/opt/locationstudio/res/console/resource/Keyboard.qml"
    }
}

