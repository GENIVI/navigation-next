import QtQuick 2.0

Item {

    signal navigationMapOverview()
    Image {
        anchors.fill: parent
        id: buttonImage
        source: applicationDirPath+ "/resource/images/RouteNavigation_images/Overview.png"
        fillMode: Image.PreserveAspectFit
        MouseArea {
            anchors.fill: parent
            onClicked: {
                navigationMapOverview();
            }
        }
    }
}
