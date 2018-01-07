import QtQuick 1.1

Rectangle {
    id: routeselectheader
    width: parent.width
    height: routeinformation.height
    Style{
        id: fontStyle
    }

    Item {
        id: routeinformation
        width: 300
        height: 100

        Text {
            id: selectroutename
            objectName: "selectroutename"
            width: parent.width
            text: selectrouteinformationcontroller.SelectRouteName
            font.family: fontStyle.defaultFontName
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 14
        }

        Text {
            id: selectroutetime
            objectName: "selectroutetime"
            anchors.top: selectroutename.bottom
            width: parent.width
            text: selectrouteinformationcontroller.SelectRouteTime +"  "+ selectroutedistance.text
            font.family: fontStyle.defaultFontName
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 14
        }

        Text {
            id: selectroutedistance
            objectName: "selectroutedistance"
            anchors.top: selectroutetime.bottom
            width: parent.width
            text: selectrouteinformationcontroller.SelectRouteDistance
            font.family: fontStyle.defaultFontName
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 14
        }

        Text {
            id: selectroutetype
            objectName: "selectroutetype"
            anchors.top: selectroutedistance.bottom
            width: parent.width
            text: selectrouteinformationcontroller.SelectRouteType
            font.family: fontStyle.defaultFontName
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
            font.pixelSize: 12
        }
    }
}
