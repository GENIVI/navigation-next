import QtQuick 2.3

MouseArea
{
    id: defaultMouseArea
    anchors.fill: parent
    onClicked: mouse.accepted = true;
    onPressed: mouse.accepted = true;
    onReleased: mouse.accepted = true;
    onDoubleClicked: mouse.accepted = true;
    onWheel: mouse.accepted = true;
}

