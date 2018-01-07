import QtQuick 2.4
//import com.locationstudio.qtnavigator.console 1.0
import "style" 1.0


Item {

    //property string speedLimitText: "40"
    //    FontLoader{
    //        id:sfRegular
    //        name:  applicationDirPath+ "/resource/font/SF-UI-Display-Regular.otf"
    //    }
    Rectangle {
        anchors.fill: parent
        color: DefaultStyle.colorBlack1 //"black"
        border.color: DefaultStyle.colorGrey1 //"gray"
        border.width: DefaultStyle.borderWidth1 //1

        DefaultMouseArea
        {

        }
        MouseArea
        {
            anchors.fill: parent
            onClicked:
            {
                console.log("myobject::"+RouteInfo)
                speedLimitLabel.text = "80"

            }

        }


        Image {
            anchors.fill: parent
            anchors.margins: 1
            id: buttonImage
            source: applicationDirPath+ "/resource/images/RouteNavigation_images/Speed-Limit.png"
            fillMode: Image.PreserveAspectFit
        }

        Text{
            id: speedLimitLabel
            anchors.fill: parent
            anchors.topMargin: parent.height/2 + 5
            text:speedLimitText
            color:DefaultStyle.colorWhite  //"white"
            font.family: DefaultStyle.fontRegular.name //sfRegular.name
            font.pixelSize: DefaultStyle.fontSize20 //20
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            onTextChanged:
            {
                console.log("DefaultStyle::"+DefaultStyle)
                RouteInfo.setSpeedLimit(parseInt(speedLimitLabel.text))
            }
        }
    }


}

