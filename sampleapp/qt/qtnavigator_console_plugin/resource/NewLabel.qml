import QtQuick 2.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtGraphicalEffects 1.0

Item {
    id: newLabel

    property alias objectName: labelText.objectName
    property alias text: labelText.text
    property bool shadow: false

    implicitWidth: labelText.implicitWidth
    implicitHeight: labelText.implicitHeight

    FontLoader { id: sfDisplayMedium; source: "qrc:/resource/font/SF-UI-Display-Medium.otf"}
    FontLoader { id: sfDisplayRegular; source: "qrc:/resource/font/SF-UI-Display-Regular.otf" }

    Text {
        id: labelText
        wrapMode: Text.Wrap
        anchors.verticalCenter: parent.verticalCenter
        font {
            family: sfDisplayMedium.name;
            pixelSize: 28;
        }
        LinearGradient  {
            anchors.fill: labelText
            source: labelText
            gradient: Gradient {
                GradientStop { position: 0; color: "#0ed9fd" }
                GradientStop { position: 1; color: "#ddf8ff" }
            }
        }
    }

    DropShadow {
        enabled: newLabel.shadow
        visible: newLabel.shadow
        anchors.fill: labelText
        horizontalOffset: 0
        verticalOffset: 0
        radius: 12
        samples: 25
        color: "#0ED9FD"
        source: labelText
        //spread: 0.6
        transparentBorder: true
    }

    /*Label {
        id: labelText        
        //font.pixelSize: 24
        font {
            family: sfDisplayRegular.name;
            pixelSize: 28;
            capitalization: Font.Capitalize
        }
        LinearGradient  {
            anchors.fill: labelText
            source: labelText
            gradient: Gradient {
                GradientStop { position: 0; color: "#0ed9fd" }
                GradientStop { position: 1; color: "#ddf8ff" }
            }
        }
    }*/

    /*Glow {
        enabled: newLabel.glow
        visible: newLabel.glow
        anchors.fill: labelText
        radius: 1
        samples: 3
        color: "#0ed9fd"
        source: labelText
        transparentBorder: true
    }*/
}
