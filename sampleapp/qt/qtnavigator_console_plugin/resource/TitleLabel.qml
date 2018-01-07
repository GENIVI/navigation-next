import QtQuick 2.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtGraphicalEffects 1.0
import "style" 1.0

Item {
    id: newLabel

    property alias objectName: labelText.objectName
    property alias text: labelText.text
    property bool shadow: false

    implicitWidth: labelText.implicitWidth
    implicitHeight: labelText.implicitHeight

    //FontLoader { id: sfDisplayMedium; source: applicationDirPath + "/resource/font/SF-UI-Display-Medium.otf"}
    //FontLoader { id: sfDisplayRegular; source: applicationDirPath + "/resource/font/SF-UI-Display-Regular.otf" }

    Text {
        id: labelText
        wrapMode: Text.Wrap
        //anchors.left: image.right; anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        color: DefaultStyle.colorWhite //"#FFF"
        font {
            family: DefaultStyle.fontMedium.name //sfDisplayMedium.name;
            pixelSize: DefaultStyle.fontSize32 //32;
        }
        //style: Text.Raised
        //styleColor: "red"

    }

    DropShadow {
        enabled: newLabel.shadow
        visible: newLabel.shadow
        anchors.fill: labelText
        horizontalOffset: 0
        verticalOffset: 0
        radius: DefaultStyle.rectRadiusM //16
        samples: 33
        color: DefaultStyle.colorCyan2 //"#0ED9FD"
        source: labelText
        spread: 0.6
        transparentBorder: true
    }

    /*Label {
        id: labelText
        font {
            family: sfDisplayMedium.name;
            pixelSize: 32;
        }
        color: "#FFF"
        anchors.verticalCenter: parent.verticalCenter
    }*/

    /*LinearGradient  {
        anchors.fill: labelText
        source: labelText
        gradient: Gradient {
            GradientStop { position: 0; color: "#0ed9fd" }
            GradientStop { position: 1; color: "#ddf8ff" }
        }
    }*/

   /* Glow {
        //enabled: newLabel.glow
        //visible: newLabel.glow
        anchors.fill: labelText
        radius: 1
        samples: 3
        color: "#0ed9fd"
        source: labelText
        transparentBorder: true
        //spread: 0.5
    }*/
}
