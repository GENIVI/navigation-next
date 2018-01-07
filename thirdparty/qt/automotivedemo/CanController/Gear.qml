import QtQuick 2.0

Rectangle {
    id: gearMain
    width: 270
    height: 270
    radius: 135
    color: "#1b1c1d"

    property int gear: 0
    property int previousGear: 0

    property bool animating: false
    property alias knobColor: knob.color
    property color trackColor: "#b6181e"
    property color unselectecColor: "#ffffff"
    property color selectecColor: "#1b1c1d"

    onGearChanged: {
        if (gearMain.animating || gearMain.gear < -1 || gearMain.gear > 5) {
            gearMain.gear = gearMain.previousGear
        }
        else {
            if (gearMain.previousGear === 0) {
                if (gearMain.gear === 1) from0to1.start()
                else if (gearMain.gear === 2) from0to2.start()
                else if (gearMain.gear === 3) from0to3.start()
                else if (gearMain.gear === 4) from0to4.start()
                else if (gearMain.gear === 5) from0to5.start()
                else if (gearMain.gear === -1) from0toR.start()
            }
            else if (gearMain.previousGear === 1) from1to0.start()
            else if (gearMain.previousGear === 2) from2to0.start()
            else if (gearMain.previousGear === 3) from3to0.start()
            else if (gearMain.previousGear === 4) from4to0.start()
            else if (gearMain.previousGear === 5) from5to0.start()
            else if (gearMain.previousGear === -1) fromRto0.start()
            gearMain.previousGear = gearMain.gear
        }
    }

    Rectangle {
        id: verticalLine
        anchors.centerIn: parent
        width: 126
        height: 6
        color: gearMain.trackColor
    }

    Rectangle {
        id: leftHorizontalLine
        anchors {
            left: verticalLine.left
            verticalCenter: parent.verticalCenter
        }
        width: 6
        height: 70
        color: gearMain.trackColor
    }

    Rectangle {
        id: middleHorizontalLine
        anchors.centerIn: parent
        width: 6
        height: 70
        color: gearMain.trackColor
    }

    Rectangle {
        id: rightHorizontalLine
        anchors {
            right: verticalLine.right
            verticalCenter: parent.verticalCenter
        }
        width: 6
        height: 70
        color: gearMain.trackColor
    }

    Rectangle {
        id: knob
        width: 50
        height: 50
        radius: 25
        color: "#b6181e"
        property int middleX: 110
        property int middleY: 110
        property int leftX: 50
        property int rightX: 170
        property int topY: 50
        property int bottomY: 170
        x: knob.middleX
        y: knob.middleY

        SequentialAnimation {
            id: from0to1
            PropertyAnimation { target: knob; property: "x"; from: knob.middleX; to: knob.leftX; }
            PropertyAnimation { target: knob; property: "y"; from: knob.middleY; to: knob.topY; }
            onRunningChanged: gearMain.animating = from0to1.running;
        }
        SequentialAnimation {
            id: from0to2
            PropertyAnimation { target: knob; property: "x"; from: knob.middleX; to: knob.leftX; }
            PropertyAnimation { target: knob; property: "y"; from: knob.middleY; to: knob.bottomY; }
            onRunningChanged: gearMain.animating = from0to2.running;
        }
        PropertyAnimation {
            id: from0to3
            target: knob
            property: "y"
            from: knob.middleY
            to: knob.topY
            onRunningChanged: gearMain.animating = from0to3.running
        }
        PropertyAnimation {
            id: from0to4
            target: knob
            property: "y"
            from: knob.middleY
            to: knob.bottomY
            onRunningChanged: gearMain.animating = from0to4.running
        }
        SequentialAnimation {
            id: from0to5
            PropertyAnimation { target: knob; property: "x"; from: knob.middleX; to: knob.rightX; }
            PropertyAnimation { target: knob; property: "y"; from: knob.middleY; to: knob.topY; }
            onRunningChanged: gearMain.animating = from0to5.running
        }
        SequentialAnimation {
            id: from0toR
            PropertyAnimation { target: knob; property: "x"; from: knob.middleX; to: knob.rightX; }
            PropertyAnimation { target: knob; property: "y"; from: knob.middleY; to: knob.bottomY; }
            onRunningChanged: gearMain.animating = from0toR.running
        }
        SequentialAnimation {
            id: from1to0
            PropertyAnimation { target: knob; property: "y"; from: knob.topY; to: knob.middleY; }
            PropertyAnimation { target: knob; property: "x"; from: knob.leftX; to: knob.middleX; }
            onRunningChanged: {
                if (!from1to0.running) {
                    if (gearMain.gear === 2) from0to2.start()
                    else if (gearMain.gear === 3) from0to3.start()
                    else if (gearMain.gear === 4) from0to4.start()
                    else if (gearMain.gear === 5) from0to5.start()
                    else if (gearMain.gear === -1) from0toR.start()
                    else gearMain.animating = false
                }
                else {
                    gearMain.animating = true
                }
            }
        }
        SequentialAnimation {
            id: from2to0
            PropertyAnimation { target: knob; property: "y"; from: knob.bottomY; to: knob.middleY; }
            PropertyAnimation { target: knob; property: "x"; from: knob.leftX; to: knob.middleX; }
            onRunningChanged: {
                if (!from2to0.running) {
                    if (gearMain.gear === 1) from0to1.start()
                    else if (gearMain.gear === 3) from0to3.start()
                    else if (gearMain.gear === 4) from0to4.start()
                    else if (gearMain.gear === 5) from0to5.start()
                    else if (gearMain.gear === -1) from0toR.start()
                    else gearMain.animating = false
                }
                else {
                    gearMain.animating = true
                }
            }
        }
        PropertyAnimation {
            id: from3to0
            target: knob
            property: "y"
            from: knob.topY
            to: knob.middleY
            onRunningChanged: {
                if (!from3to0.running) {
                    if (gearMain.gear === -1) from0toR.start()
                    else if (gearMain.gear === 1) from0to1.start()
                    else if (gearMain.gear === 2) from0to2.start()
                    else if (gearMain.gear === 4) from0to4.start()
                    else if (gearMain.gear === 5) from0to5.start()
                    else gearMain.animating = false
                }
                else {
                    gearMain.animating = true
                }
            }
        }
        PropertyAnimation {
            id: from4to0
            target: knob
            property: "y"
            from: knob.bottomY
            to: knob.middleY
            onRunningChanged: {
                if (!from4to0.running) {
                    if (gearMain.gear === 1) from0to1.start()
                    else if (gearMain.gear === 2) from0to2.start()
                    else if (gearMain.gear === 3) from0to3.start()
                    else if (gearMain.gear === 5) from0to5.start()
                    else if (gearMain.gear === -1) from0toR.start()
                    else gearMain.animating = false
                }
                else {
                    gearMain.animating = true
                }
            }
        }
        SequentialAnimation {
            id: from5to0
            PropertyAnimation { target: knob; property: "y"; from: knob.topY; to: knob.middleY; }
            PropertyAnimation { target: knob; property: "x"; from: knob.rightX; to: knob.middleX; }
            onRunningChanged: {
                if (!from5to0.running) {
                    if (gearMain.gear === 1) from0to1.start()
                    else if (gearMain.gear === 2) from0to2.start()
                    else if (gearMain.gear === 3) from0to3.start()
                    else if (gearMain.gear === 4) from0to4.start()
                    else if (gearMain.gear === -1) from0toR.start()
                    else gearMain.animating = false
                }
                else {
                    gearMain.animating = true
                }
            }
        }
        SequentialAnimation {
            id: fromRto0
            PropertyAnimation { target: knob; property: "y"; from: knob.bottomY; to: knob.middleY; }
            PropertyAnimation { target: knob; property: "x"; from: knob.rightX; to: knob.middleX; }
            onRunningChanged: {
                if (!fromRto0.running) {
                    if (gearMain.gear === 1) from0to1.start()
                    else if (gearMain.gear === 2) from0to2.start()
                    else if (gearMain.gear === 3) from0to3.start()
                    else if (gearMain.gear === 4) from0to4.start()
                    else if (gearMain.gear === 5) from0to5.start()
                    else gearMain.animating = false
                }
                else {
                    gearMain.animating = true
                }
            }
        }
    }

    Item {
        anchors {
            bottom: leftHorizontalLine.top
            horizontalCenter: leftHorizontalLine.horizontalCenter
        }
        width: 50
        height: 50
        Text {
            id: text1
            anchors.centerIn: parent
            font.pixelSize: 30
            text: "1"
            color: (!gearMain.animating && gearMain.gear === 1 ? gearMain.selectecColor
                                                               : gearMain.unselectecColor)
        }
    }

    Item {
        anchors {
            top: leftHorizontalLine.bottom
            horizontalCenter: leftHorizontalLine.horizontalCenter
        }
        width: 50
        height: 50
        Text {
            id: text2
            anchors.centerIn: parent
            font.pixelSize: 30
            text: "2"
            color: (!gearMain.animating && gearMain.gear === 2 ? gearMain.selectecColor
                                                               : gearMain.unselectecColor)
        }
    }
    Item {
        anchors {
            bottom: middleHorizontalLine.top
            horizontalCenter: middleHorizontalLine.horizontalCenter
        }
        width: 50
        height: 50
        Text {
            id: text3
            anchors.centerIn: parent
            font.pixelSize: 30
            text: "3"
            color: (!gearMain.animating && gearMain.gear === 3 ? gearMain.selectecColor
                                                               : gearMain.unselectecColor)
        }
    }
    Item {
        anchors {
            top: middleHorizontalLine.bottom
            horizontalCenter: middleHorizontalLine.horizontalCenter
        }
        width: 50
        height: 50
        Text {
            id: text4
            anchors.centerIn: parent
            font.pixelSize: 30
            text: "4"
            color: (!gearMain.animating && gearMain.gear === 4 ? gearMain.selectecColor
                                                               : gearMain.unselectecColor)
        }
    }
    Item {
        anchors {
            bottom: rightHorizontalLine.top
            horizontalCenter: rightHorizontalLine.horizontalCenter
        }
        width: 50
        height: 50
        Text {
            id: text5
            anchors.centerIn: parent
            font.pixelSize: 30
            text: "5"
            color: (!gearMain.animating && gearMain.gear === 5 ? gearMain.selectecColor
                                                               : gearMain.unselectecColor)
        }
    }
    Item {
        anchors {
            top: rightHorizontalLine.bottom
            horizontalCenter: rightHorizontalLine.horizontalCenter
        }
        width: 50
        height: 50
        Text {
            id: textR
            anchors.centerIn: parent
            font.pixelSize: 30
            text: "R"
            color: (!gearMain.animating && gearMain.gear === -1 ? gearMain.selectecColor
                                                                : gearMain.unselectecColor)
        }
    }

    Rectangle {
        anchors.centerIn: parent
        width: 32
        height: 32
        radius: 16
        color: gearMain.trackColor
        Rectangle {
            anchors.centerIn: parent
            width: 28
            height: 28
            radius: 14
            color: (!gearMain.animating && gearMain.gear === 0 ? gearMain.unselectecColor
                                                               : gearMain.selectecColor)
            Text {
                anchors.centerIn: parent
                font.pixelSize: 20
                text: "N"
                color: (!gearMain.animating && gearMain.gear === 0 ? gearMain.selectecColor
                                                                   : gearMain.unselectecColor)
            }
        }
    }

    MouseArea {
        x: 0
        y: 0
        width: 105
        height: 135
        enabled: !gearMain.animating
        onClicked: gearMain.gear = 1
    }
    MouseArea {
        x: 105
        y: 0
        width: 60
        height: 105
        enabled: !gearMain.animating
        onClicked: gearMain.gear = 3
    }
    MouseArea {
        x: 165
        y: 0
        width: 105
        height: 135
        enabled: !gearMain.animating
        onClicked: gearMain.gear = 5
    }
    MouseArea {
        x: 0
        y: 135
        width: 105
        height: 135
        enabled: !gearMain.animating
        onClicked: gearMain.gear = 2
    }
    MouseArea {
        x: 105
        y: 165
        width: 60
        height: 105
        enabled: !gearMain.animating
        onClicked: gearMain.gear = 4
    }
    MouseArea {
        x: 165
        y: 135
        width: 105
        height: 135
        enabled: !gearMain.animating
        onClicked: gearMain.gear = -1
    }
    MouseArea {
        x: 105
        y: 105
        width: 60
        height: 60
        enabled: !gearMain.animating
        onClicked: gearMain.gear = 0
    }

}
