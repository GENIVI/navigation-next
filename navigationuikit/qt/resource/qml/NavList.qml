// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Rectangle{
    id: navlist
    color: "#333333"

    Style{
        id: fontStyle
    }

    Rectangle{
        id: navigationheader
        width: parent.width
        height: triptimetext.height
        color: "#555555"
        Image {
            id: trafficImage
            objectName: "trafficimage"
            width: triptimetext.height - 20
            height: triptimetext.height - 20
            source: navlistcontroller.Traffic
            anchors{
                leftMargin: 10
                left: parent.left
                top: parent.top
                topMargin: 10
            }
        }
        Text {
            id: triptimetext
            objectName: "triptime"
            height: 40
            anchors{
                left: trafficImage.right
                leftMargin: 10
            }
            text: navlistcontroller.ArrivalTime
            font.pointSize: 20
            font.letterSpacing: 1
            font.family: fontStyle.defaultFontName
            style: Text.Raised
            color: "#ffffff"
            elide: Text.ElideLeft
            verticalAlignment: Text.AlignVCenter
        }
        Text {
            id: tripdistancetext
            objectName: "tripdistance"
            height: triptimetext.height
            anchors{
                right: navigationheader.right
                rightMargin: 10
            }
            text: navlistcontroller.TripDistance
            font.pointSize: 20
            font.letterSpacing: 1
            font.family: fontStyle.defaultFontName
            style: Text.Raised
            color: "#ffffff"
            elide: Text.ElideLeft
            verticalAlignment: Text.AlignVCenter
        }
    }

    Rectangle{
        id: arrivingHeader
        width: parent.width
        height: parent.height
        anchors{
            top:navigationheader.bottom
        }
        color: parent.color
        visible: navlistcontroller.IsArriving

        Text {
            id: distance
            objectName: "arrivingdistance"
            text: navlistcontroller.TripDistance
            width: parent.width
            height: 60
            anchors{
                top: parent.top
                left: parent.left
            }
            color: "#ffffff"
            opacity: 0.7
            font{
                pointSize: 22
                letterSpacing: 1
            }
            style: Text.Raised
            elide: Text.ElideLeft
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        Image {
            id: laneimage
            objectName: "laneimage"
            opacity: 0.7
            height: 120
            width:120
            source: "image://laneimage/" + "80" + "*"+"red" + "*" + "#333333" +"*"+ navlistcontroller.DestinationChar + "*" + "120"
            anchors{
                top: distance.bottom
                topMargin: 0
                horizontalCenter: parent.horizontalCenter
            }
        }

        Text {
            id: tiptext
            objectName: "arrivingtip"
            anchors{
                left: parent.left
                top: laneimage.bottom
                topMargin: 10
            }
            width: parent.width

            text: navlistcontroller.ArrivingTip
            font.pointSize: 26
            font.letterSpacing: 1
            style: Text.Raised
            color: "#ffffff"
            elide: Text.ElideLeft
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        Text {
            id: destinationext
            objectName: "destinationtext"
            width: parent.width
            anchors{
                left: parent.left
                top: tiptext.bottom
                topMargin: 20
            }
            text: navlistcontroller.DestinationText
            font.pointSize: 26
            font.letterSpacing: 1
            style: Text.Raised
            color: "#ffffff"
            elide: Text.ElideLeft
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignBottom
        }
    }

    ManeuverList{
        id:maneuverlist
        width: parent.width;
        height: parent.height - navigationheader.height
        visible: !navlistcontroller.IsArriving
        anchors{
            top: navigationheader.bottom
        }
    }
}
