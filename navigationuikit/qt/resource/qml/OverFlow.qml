// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Rectangle{
    id: footer
    width: settingImage.width + 20
    height: 110/*settingImage.height + 30*/
    color: "#0080FF"

    Image {
        id: settingImage
        source: "../images/more_white.png";
        anchors{
            right: parent.right
            rightMargin: 15
            top: parent.top
            topMargin: 15
        }
    }
}
