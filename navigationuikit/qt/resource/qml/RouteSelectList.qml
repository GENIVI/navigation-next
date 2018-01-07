import QtQuick 1.1

Rectangle{
    id: routelist
    width: 400
    height: 646
    color: "#333333"

    signal swiproute(int offset)

    Style{
        id: fontStyle
    }

    Component{
        id: normalManeuverDelegate
        Item {
            id: maneuver
            width: rtsList.width
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
                source: "image://laneimage/" + "50" + "*"+manevuer_charcolor + "*" + routelist.color +"*"+ manevuer_char + "*" + "70"
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
        id: rtsHeaderDelegate
        Item {
            id: rtsHeaderitem
            width: rtsList.width
            height: rtslistcontroller.IsRouteTipsEmpty()? 116:152
            Rectangle {
                id: rectangle1
                x: 0
                width: rtsList.width - rtsList.anchors.leftMargin*2
                height: parent.height
                color: routelist.color

                MouseArea {
                    id: mouseArea
                    x: 0
                    width: rectangle1.width
                    height: rectangle1.height
                    drag.target: rectangle1
                    drag.axis: "XAxis"
                    drag.minimumX:-rectangle1.width
                    drag.maximumX:rectangle1.width

                    onReleased:
                    {
                        if(rectangle1.x > 50)
                            routelist.swiproute(1)
                        else if(rectangle1.x < -50)
                            routelist.swiproute(-1)
                    }


                    Rectangle {
                        id: routeinforect
                        x: 68
                        width: triptime.paintedWidth + tripdistance.paintedWidth + 35
                               + traffic.paintedWidth
                        height: 38
                        color: rectangle1.color
                        anchors.top: viaRoad.bottom
                        anchors.topMargin: 3
                        anchors.horizontalCenterOffset: 6
                        anchors.horizontalCenter: rectangle1.horizontalCenter

                        Text {
                            id: triptime
                            y: 0
                            height: routeinforect.height
                            text: rtslistcontroller.TripTime
                            anchors.left: traffic.right
                            anchors.leftMargin: 10
                            color: "#ffffff"
                            opacity: 0.7
                            font{
                                pixelSize: 20
                                letterSpacing: 1
                                family: fontStyle.defaultFontName
                            }
                            style: Text.Raised
                            elide: Text.ElideLeft
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        Text {
                            id: tripdistance
                            x: 94
                            y: 0
                            height: routeinforect.height
                            text: rtslistcontroller.TripDistance
                            anchors.left: triptime.right
                            anchors.leftMargin: 25
                            color: "#ffffff"
                            opacity: 0.7
                            font{
                                pixelSize: 20
                                letterSpacing: 1
                                family: fontStyle.defaultFontName
                            }
                            style: Text.Raised
                            elide: Text.ElideLeft
                            horizontalAlignment: Text.AlignLeft
                            verticalAlignment: Text.AlignVCenter
                        }

                        Image {
                            id: traffic
                            x: 0
                            y: 0
                            width: 24
                            height: 24
                            anchors.verticalCenter: routeinforect.verticalCenter
                            source: rtslistcontroller.Traffic
                        }
                    }

                    Text {
                        id: viaRoad
                        x: 0
                        y: 8
                        width: parent.width
                        height: 27
                        color: "#ffffff"
                        opacity: 0.7
                        text: rtslistcontroller.ViaRoadName
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        elide: Text.ElideRight
                        font.bold: true
                        anchors.horizontalCenter: rectangle1.horizontalCenter
                        font.pixelSize: 25
                        font.family: fontStyle.defaultFontName
                    }

                    Text {
                        id: tipinfo
                        x: 0
                        y: 68
                        height: rtslistcontroller.IsRouteTipsEmpty()? 0:36
                        width: parent.width
                        color: "#ffffff"
                        opacity: 0.7
                        text: rtslistcontroller.RouteTip
                        clip: false
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        anchors.horizontalCenter: rectangle1.horizontalCenter
                        font.italic: true
                        font.pixelSize: 13
                        font.family: fontStyle.defaultFontName
                    }

                    Text {
                        id: starton
                        x: 0
                        y: 114
                        color: "#ffffff"
                        opacity: 0.7
                        text: rtslistcontroller.StartOn
                        font.pixelSize: 20
                        font.family: fontStyle.defaultFontName
                        anchors.top: tipinfo.bottom
                        anchors.topMargin: rtslistcontroller.IsRouteTipsEmpty()? 12:2
                    }

                    Text {
                        id: currentroad
                        y: 114
                        color: "#ffffff"
                        opacity: 0.7
                        text: rtslistcontroller.CurrentRoad
                        font.bold: false
                        font.family: fontStyle.defaultFontName
                        anchors.left: starton.right
                        anchors.leftMargin: 11
                        anchors.top: starton.top
                        font.pixelSize: 20
                    }
                }

            }
        }
    }

    ListView {
        id: rtsList
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
                sourceComponent: index > 0? normalManeuverDelegate : rtsHeaderDelegate
            }
        }
    }
}


