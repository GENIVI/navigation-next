import QtQuick 2.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

Item {
    id: newRadioButton

    property alias objectName: radioButton.objectName
    property alias text: radioButton.text
    property alias checked: radioButton.checked
    property alias exclusiveGroup: radioButton.exclusiveGroup

    implicitWidth: radioButton.implicitWidth
    implicitHeight: radioButton.implicitHeight

    RadioButton {
        id: radioButton
        text: radioButton.text
        checked: radioButton.checked
        objectName: radioButton.objectName

        exclusiveGroup: radioButton.exclusiveGroup

        style: RadioButtonStyle
        {
            indicator: Image
            {
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                source: applicationDirPath + "/resource/images/Settings_images/unchecked.png"
                width: 30
                height: 30
                Image
                {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                    source: applicationDirPath + "/resource/images/Settings_images/radio_checked.png"
                    visible: control.checked
                    width: 30
                    height: 30
                }
            }
            spacing: 15
            label: Text
            {
                text: control.text
                font.pixelSize: 22
                font.family: "DejaVu Sans"
                horizontalAlignment: Text.left
                color: "#FFFFFF"
            }
        }
    }

}

