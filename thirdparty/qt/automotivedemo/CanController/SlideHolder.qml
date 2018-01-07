import QtQuick 2.0

Item {
    width: parent.width
    height: (0.25 * parent.height)
    property alias text: sliderText.text
    property alias max: slide.max
    property alias min: slide.min
    property alias steps: slide.steps
    property alias value: slide.value
    property alias valueStep: slide.valueStep

    Text {
        id: sliderText
        anchors {
           top: parent.top
           topMargin: 10
           left: parent.left
           leftMargin: 5
        }
        font.pixelSize: 14
    }

    Slide {
        id: slide
        anchors{
            top: sliderText.bottom
            left: parent.left
            right: parent.right
        }
    }
}
