import QtQuick 2.0

Rectangle {
    id: calendar
    width: parent.width; height: parent.height
    color: "white"
    property date today: new Date()
    property date showDate: new Date()
    property int daysInMonth: new Date(showDate.getFullYear(), showDate.getMonth() + 1, 0).getDate()
    property int firstDay: new Date(showDate.getFullYear(), showDate.getMonth(), 1).getDay()

    Item {
        id:title
        anchors.top: parent.top
        anchors.topMargin: 10
        width: parent.width
        height: childrenRect.height

        Image {
            id:monthright
            source: "Images/previous.png"
            anchors.left: parent.left
            anchors.leftMargin: 10

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    showDate = new Date(showDate.getFullYear(), showDate.getMonth(), 0)
                }
            }
        }
        Text {
            id:month
            color: "white"
            text: Qt.formatDateTime(showDate, "MMMM")
            font.bold: true
            anchors.left:monthright.right
        }
        Image {
            source: "Images/next.png"
            anchors.left:month.right
            MouseArea {
                anchors.fill: parent
                onClicked:
                {
                    showDate = new Date( showDate.getFullYear(), showDate.getMonth() + 1, 1)
                    console.log(showDate)
                }
            }
        }
        Image {
            source: "Images/previous.png"
            anchors.right: yearleft.left
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    showDate = new Date(showDate.getFullYear()-1,0)
                    console.log(showDate)

                }
            }
        }
        Text {
            id:yearleft
            color: "white"
            text: Qt.formatDateTime(showDate, "yyyy")
            font.bold: true
            anchors.right:year.left
        }

        Image {
            id:year
            source: "Images/next.png"
            anchors.right: parent.right
            anchors.rightMargin: 10

            MouseArea {
                anchors.fill: parent
                onClicked:
                {
                    showDate = new Date(showDate.getFullYear()+1, 1)
                    boxtext.text=new Date
                    console.log(showDate)
                }
            }
        }
    }

    function isToday(index) {
        if (today.getFullYear() != showDate.getFullYear())
            return false;
        if (today.getMonth() != showDate.getMonth())
            return false;

        return (index === today.getDate() - 1)
    }

    Item {
        id: dateLabels
        anchors.top: title.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10

        height: calendar.height - title.height - 20 - title.anchors.topMargin
        property int rows: 6
        Item {
            id: dayLabels
            width: parent.width
            height: childrenRect.height

            Grid {
                columns: 7
                spacing: 10

                Repeater {
                    model: 7

                    Rectangle {
                        color: "#00ffffff"
                        width: (calendar.width - 20 - 60)/7
                        height: childrenRect.height

                        Text {
                            color: "#00000C"
                            // Qt dates (for formatting) and JavaScript dates use different ranges
                            // (1-7 and 0-6 respectively), so we add 1 to the day number.
                            text: Qt.formatDate(new Date(showDate.getFullYear(), showDate.getMonth(), index - firstDay +1), "ddd");
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                }
            }
        }
        Item {
            id: dateGrid
            width: parent.width
            anchors.top: dayLabels.bottom
            anchors.topMargin: 5
            anchors.bottom: parent.bottom

            Grid {
                columns: 7
                rows: dateLabels.rows
                spacing: 10

                Repeater {
                    model: firstDay + daysInMonth

                    Rectangle {

                        color: {
                            if (index < firstDay)
                                calendar.color="green";
                            else
                            {
                                if(isToday(index-firstDay))
                                    color="yellow";
                                else
                                    color="#eeeeee"
                            }
                            // isToday(index - firstDay) ? "#ffffdd" : "#eeeeee";
                        }
                        width: (calendar.width - 20 - 60)/7
                        height: (dateGrid.height - (dateLabels.rows - 1)*10)/dateLabels.rows

                        Text {
                            anchors.centerIn: parent
                            text: index + 1 - firstDay
                            opacity: (index < firstDay) ? 0 : 1
                            font.bold: isToday(index - firstDay)
                        }
                    }
                }
            }
        }
    }
}
