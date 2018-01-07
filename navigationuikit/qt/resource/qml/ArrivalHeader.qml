import QtQuick 1.1

Rectangle{
    id: arrivalheader
    width: 410
    height: 646
    radius: 5
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
            source: "../images/traffic_delay_green.png";
            anchors{
                leftMargin: 10
                left: parent.left
                top: parent.top
                topMargin: 10
            }
        }
        Timer{
            interval: 500; running: true; repeat: true
            onTriggered: triptimetext.text = Qt.formatTime(new Date(), "hh:mm AP")
        }

        function getCurrentTime()
        {
            var d = new Date();
            var h = d.getHours();
            var min = add_zero(d.getMinutes());
            var sec = add_zero(d.getSeconds());
            var currentdate = min +":"+sec;
            if(d > 12)
            {
                currentdate += " PM";
            }
            else
            {
                currentdate += " AM";
            }

            return currentdate.toString();
        }

        function add_zero( num )
        {
            if(num < 10)
            {
                return ("0" + num);
            }
            else
            {
                return num;
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
            }
            text: qsTr("0 m")
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
        id: arrivalitem
        width: parent.width
        height: parent.height
        anchors{
            top:navigationheader.bottom
        }
        color: parent.color

        Text {
            id: title
            objectName: "arrivaltitle"
            anchors{
                left: parent.left
                top: parent.top
            }
            width: parent.width

            text: qsTr("You have arrived")
            font.pointSize: 26
            font.family: fontStyle.defaultFontName
            style: Text.Raised
            color: "#ffffff"
            elide: Text.ElideLeft
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        Image {
            id: turn
            objectName: "turn"
            opacity: 0.7
            source: "image://laneimage1/" + "80" + "*"+"red" + "*" + "#333333" +"*"+ "V" + "*" + "120"
            anchors{
                top: title.bottom
                topMargin: 10
                horizontalCenter: parent.horizontalCenter
            }
        }

        Text {
            id: tiptext
            objectName: "arrivaltip"
            anchors{
                left: parent.left
                top: turn.bottom
                topMargin: 10
            }
            width: parent.width

            text: arrivalheadercontroller.ArrivalTip
            font.pointSize: 26
            font.letterSpacing: 1
            font.family: fontStyle.defaultFontName
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
            text: arrivalheadercontroller.Destination
            font.pointSize: 26
            font.letterSpacing: 1
            font.family: fontStyle.defaultFontName
            style: Text.Raised
            color: "#ffffff"
            elide: Text.ElideLeft
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignBottom
        }
    }
}
