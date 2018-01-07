// import QtQuick 1.0 // to target S60 5th Edition or Maemo 5
import QtQuick 1.1

Rectangle{
    id: footer
    color: "#115783"
    opacity: 0.9

    Style{
        id: fontStyle
    }

    Text{
        id: statusText
        height: parent.height
        width: parent.width

        text: footercontroller.statusText
        font.pointSize: 22
        font.bold: true
        font.letterSpacing: 1
        font.family: fontStyle.defaultFontName
        color: "white"
        style: Text.Raised
        elide: Text.ElideLeft
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    signal footerButtonSignal(int index)
    Row{
        id: footerrow
        width: parent.width
        height: parent.height
        anchors.fill: parent
        visible: footercontroller.footerOptionVisible

        Text{
            id: lefttext
            height: parent.height
            width: parent.width/3

            text: footercontroller.leftText
            font.pointSize: 22
            font.bold: true
            font.letterSpacing: 1
            font.family: fontStyle.defaultFontName
            color: "white"
            style: Text.Raised
            elide: Text.ElideLeft
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    footer.footerButtonSignal(1);
                }
            }
        }

        Text{
            id: midtext
            height: parent.height
            width: parent.width/3

            text: footercontroller.midText
            font.pointSize: 22
            font.letterSpacing: 1
            font.bold: true
            font.family: fontStyle.defaultFontName
            color: "white"
            style: Text.Raised
            elide: Text.ElideLeft
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    footer.footerButtonSignal(2);
                }
            }
        }
        Text{
            id: righttext
            height: parent.height
            width: parent.width/3

            text: footercontroller.rightText
            font.pointSize: 22
            font.letterSpacing: 1
            font.bold: true
            font.family: fontStyle.defaultFontName
            color: "white"
            style: Text.Raised
            elide: Text.ElideLeft
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            MouseArea{
                anchors.fill: parent
                onClicked: {
                    footer.footerButtonSignal(3);
                }
            }
        }
    }
}
