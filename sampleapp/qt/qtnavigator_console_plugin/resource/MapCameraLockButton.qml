import QtQuick 2.4

Item {
    signal cameraLockButtonClicked()
    signal navigationMapOverview()
    signal panView()
    signal resetButton()
    property string imageSource: applicationDirPath+ "/resource/images/RouteNavigation_images/Overview.png"
    property bool isCamera: true

    onPanView:{
        buttonImage.source=applicationDirPath+ "/resource/images/RouteNavigation_images/Perspective.png"
        isCamera=true
    }
    onResetButton: {
        buttonImage.source=applicationDirPath+ "/resource/images/RouteNavigation_images/Overview.png"
        isCamera=true
    }

    Image {
        id: buttonImage
        anchors.fill: parent
        source:  imageSource
        fillMode: Image.PreserveAspectFit
        MouseArea {
            anchors.fill: parent
            onClicked: {

                if(isCamera){
                    console.log("Inside if.............",isCamera)
                    cameraLockButtonClicked();
                    buttonImage.source=applicationDirPath+ "/resource/images/RouteNavigation_images/Overview.png"
                    isCamera=false
                }else{
                    console.log("Inside else.............",isCamera)
                    navigationMapOverview()
                    buttonImage.source=applicationDirPath+ "/resource/images/RouteNavigation_images/Perspective.png"
                    isCamera=true

                }

            }
        }
    }
}
