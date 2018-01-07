import QtQuick 1.1

Rectangle {
    id:routeSelectStatusBar
    width: 400
    height: 60
    color: "#333333"
    signal routeSelected(int index)
    signal setCurrentRoute(int index)
    Style{
        id: fontStyle
    }

    onSetCurrentRoute:
   {
        if(index == 0)
        {
            textBackground1.color = "#999999"
            textBackground2.color = routeSelectStatusBar.color
            textBackground3.color = routeSelectStatusBar.color
        }
        else if( index == 1)
        {
            textBackground2.color="#999999"
            textBackground1.color= routeSelectStatusBar.color
            textBackground3.color= routeSelectStatusBar.color
        }
        else if( index == 2)
        {
            textBackground3.color="#999999"
            textBackground1.color=routeSelectStatusBar.color
            textBackground2.color=routeSelectStatusBar.color
        }
   }
    MouseArea {
        id: mouseArea1
        objectName:"mouseArea1"
        x: 135
        y: 4
        width: 43
        height: 43
        visible: true
        onClicked:
        {
            routeSelectStatusBar.routeSelected(0)
            routeSelectStatusBar.setCurrentRoute(0)
        }
        Rectangle {
            id: textBackground1
            objectName:"textBackground1"
            x: 0
            y: 0
            width: mouseArea1.width
            height: mouseArea1.height
            color: "#00000000"
            visible: true

            Text {
                id: text1
                x: 0
                y: 0
                width: mouseArea1.width
                height: mouseArea1.height
                color:"white"
                text: qsTr("1")
                font.bold: false
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                styleColor: "#00000000"
                font.pixelSize: 17
                font.family: fontStyle.defaultFontName
            }
        }
    }

    MouseArea {
        id: mouseArea2
        objectName:"mouseArea2"
        x: mouseArea1.x+mouseArea1.width
        y: 4
        width: mouseArea1.width
        height: mouseArea1.height
        onClicked:
        {
            routeSelectStatusBar.routeSelected(1)
            routeSelectStatusBar.setCurrentRoute(1)
        }

        Rectangle {
            id: textBackground2
            objectName:"textBackground2"
            x: 0
            y: 0
            width: mouseArea1.width
            height: mouseArea1.height
            color: "#00000000"
            Text {
                id: text2
                x: 0
                y: 0
                width: mouseArea1.width
                height: mouseArea1.height
                color:"white"
                text: qsTr("2")
                font.bold: false
                horizontalAlignment: Text.AlignHCenter
                styleColor: "#00000000"
                font.pixelSize: 17
                font.family: fontStyle.defaultFontName
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    MouseArea {
        id: mouseArea3
        objectName:"mouseArea3"
        x: mouseArea2.x+mouseArea2.width
        y: 4
        width: mouseArea1.width
        height: mouseArea1.height
        onClicked:
        {
            routeSelectStatusBar.routeSelected(2)
            routeSelectStatusBar.setCurrentRoute(2)
        }

        Rectangle {
            id: textBackground3
            objectName:"textBackground3"
            x: 0
            y: 0
            width: mouseArea1.width
            height: mouseArea1.height
            color: "#00000000"
            Text {
                id: text3
                x: 0
                y: 0
                width: mouseArea1.width
                height: mouseArea1.height
                color:"white"
                text: qsTr("3")
                font.bold: false
                horizontalAlignment: Text.AlignHCenter
                styleColor: "#00000000"
                font.pixelSize: 17
                font.family: fontStyle.defaultFontName
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
}
