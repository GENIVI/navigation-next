import QtQuick 1.1

Rectangle {
    id: announcement
    width: 48/*voiceImage.width*/
    height: 140/*voiceImage.height*/
    color: "white"

    Image {
        id: voiceImage
        objectName: "volumeImage"
        source: "";
        anchors{
            right: parent.right
            bottom: parent.bottom
        }
    }
}
