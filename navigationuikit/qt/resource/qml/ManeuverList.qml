import QtQuick 1.1

Item {

    Style{
        id: fontStyle
    }

    Component{
        id: normalManeuverDelegate
        Item {
            id: maneuver
            width: maneuverList.width
            height: 88 + 30

            Text {
                id: distance0
                text: manevuer_dis
                anchors{
                    top: maneuver.top
                    left: maneuver.left
                }
                color: "#ffffff"
                opacity: 0.7
                font{
                    pointSize: 22
                    letterSpacing: 1
                    family: fontStyle.defaultFontName
                }
                style: Text.Raised
                elide: Text.ElideLeft
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            Image{
                id: trafficEvent
                width: 32
                height: 32
                anchors{
                    top: distance0.top
                    topMargin: 2
                    right: delaytimeText.left
                    rightMargin: 4
                }
                source: manevuer_traffic
            }

            Text{
                id: delaytimeText
                text: manevuer_delay
                anchors{
                    top: distance0.top
                    topMargin: 2
                    right: maneuver.right
                    rightMargin: delaytimeText.text.length > 0 ? 25 : 0
                }

                color: manevuer_splitlinecolor
                opacity: 0.7
                font{
                    pointSize: 20
                    letterSpacing: 1
                    family: fontStyle.defaultFontName
                }
                style: Text.Raised
                elide: Text.ElideLeft
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            Rectangle{
                id: splitline
                anchors{
                    top: distance0.top
                    topMargin: 17
                    right: trafficEvent.left
                    rightMargin: 4
                }

                width: maneuver.width - distance0.width
                       - trafficEvent.width
                       - delaytimeText.width
                       - (delaytimeText.text.length > 0 ? (delaytimeText.anchors.rightMargin + 10) : 10)
                height: 1
                opacity: 0.7
                color: manevuer_splitlinecolor
            }

            Image {
                id: laneimage
                objectName: "laneimage"
                opacity: 0.7
                source: "image://laneimage/" + "50" + "*"+manevuer_charcolor + "*" + "#333333" +"*"+ manevuer_char + "*" + "70"
                anchors{
                    top: distance0.bottom
                    left: distance0.left
                }
            }
            Text {
                id: street0
                anchors{
                    top: distance0.bottom
                    topMargin: 22
                    left: laneimage.right
                    leftMargin: 10
                    right: maneuver.right
                    rightMargin: 25
                }
                text: manevuer_street
                color: "#ffffff"
                opacity: 0.7
                font{
                    pointSize: 22
                    letterSpacing: 1
                    family: fontStyle.defaultFontName
                }
                style: Text.Raised
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignHLeft
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    Component{
        id: firstManeuverDelegate
        Item {
            id: maneuver1
            width: parent.height
            height: distance1.height + turnimage.height + street1.height + 50
            Text {
                id: distance1
                text: manevuer_dis
                x: 0
                y: 10
                width: maneuverlist.width - maneuverList.anchors.leftMargin - maneuverList.anchors.rightMargin
                color: "#ffffff"
                font{
                    pointSize: 32
                    letterSpacing: 1
                    family: fontStyle.defaultFontName
                }
                style: Text.Raised
                elide: Text.ElideLeft
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            Image {
                id: turnimage
                objectName: "turnimage"
                height: 120
                width:120
                x: (distance1.width - width) / 2
                source: "image://laneimage/" + "80" + "*"+manevuer_charcolor + "*" + "#333333" +"*"+ manevuer_char + "*" + "120"
                anchors{
                    topMargin: 0
                    top: distance1.bottom
                    horizontalCenter: panel.horizontalCenter
                }
            }
            Text {
                id: street1
                anchors{
                    top: turnimage.bottom
                    horizontalCenter: panel.horizontalCenter
                    topMargin: 5
                }
                width: maneuverlist.width - maneuverList.anchors.leftMargin - maneuverList.anchors.rightMargin
                text: manevuer_street
                color: "#ffffff"
                font{
                    pointSize: 26
                    letterSpacing: 1
                    family: fontStyle.defaultFontName
                }
                style: Text.Raised
                elide: Text.ElideLeft
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

        }
    }

    ListView {
        id: maneuverList
        width: parent.width
        height: parent.height
        clip: true
        anchors{
            top: parent.top
            left: parent.left
            leftMargin: 20
            rightMargin: 20
        }

        model: ManeuverModel
        delegate: Component {
            Loader {
                property string manevuer_dis: distance
                property string manevuer_char: character
                property string manevuer_street: street
                property string manevuer_traffic: traffic
                property string manevuer_delay: delaytime
                property string manevuer_splitlinecolor: splitlinecolor
                property string manevuer_charcolor: charatercolor
                sourceComponent: isnormalitem ? normalManeuverDelegate : firstManeuverDelegate
            }
        }
    }
}
