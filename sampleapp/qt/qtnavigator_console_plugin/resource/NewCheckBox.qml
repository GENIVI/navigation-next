import QtQuick 2.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

Item {
    id: newCheckBox

    property alias objectName: checkBox.objectName
    property alias text: checkBox.text
    property string checkedIcon
    property string uncheckedIcon
    property alias checked: checkBox.checked
    //property alias background: backgroundRect.color
    //property int fontSize: 14
    //property alias fontColor: label.color
    signal clicked()

    //height: 25

    implicitWidth: checkBox.implicitWidth
    implicitHeight: checkBox.implicitHeight

    clip: true

    CheckBox {
        id: checkBox
        text: checkBox.text
        checked: checkBox.checked
        objectName: checkBox.objectName
        style: CheckBoxStyle
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
                    source: enabled ? applicationDirPath + "/resource/images/Settings_images/checkmark_checked.png" : applicationDirPath + "/resource/images/Settings_images/unchecked.png"
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
                anchors.bottomMargin: 50
            }
        }
    }
}
