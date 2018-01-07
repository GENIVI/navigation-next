import QtQuick 2.7
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import "style" 1.0

Rectangle {
    id:comboBox
    property variant modelName
    property string defaultName
    property int currentIndex
    property alias currentText: chosenItemText.text;
    property alias selectedIndex: listView.currentIndex;
    property int count
    signal comboClicked;
    signal comboBoxClicked
    width: 400;
    height: 30;
    z: 100;
    smooth:true;

    Rectangle {
        id:chosenItem
        radius:DefaultStyle.rectRadiusS //4;
        width:parent.width;
        height:comboBox.height;
        color:DefaultStyle.colorLightGray //"lightgrey"
        smooth:true;
        Text {
            id:chosenItemText
            anchors.top: parent.top;
            anchors.left: parent.left;
            anchors.margins: 8;
            text:defaultName
            font.family:DefaultStyle.fontRegular.name// "Arial"
            font.pointSize: DefaultStyle.fontSize14 //14;
            smooth:true
        }

        MouseArea {
            anchors.fill: parent;
            onClicked: {
                //                console.log("Model Dataatatatat :: ",comboBox.modelName)
                //                console.log("Model Length :: ",comboBox.modelName.length)
                count=comboBox.modelName.length
                comboBox.state = comboBox.state==="dropDown"?"":"dropDown"
                comboBox.comboBoxClicked()
            }
        }
    }

    Rectangle {
        id:dropDown
        width:comboBox.width;
        height:0;
        clip:true;
        radius:DefaultStyle.rectRadiusS//4;
        anchors.bottom: chosenItem.top;
        anchors.margins: 2;
        color: DefaultStyle.colorBlack2 //"#141B25"
        opacity: 1

        ListView {
            id:listView
            height:dropDown.height
            model: comboBox.modelName
            highlight: highlight
            highlightFollowsCurrentItem:true
            highlightRangeMode:ListView.StrictlyEnforceRange
            // focus: listviewfocus
            anchors.fill:parent

            delegate: Item{
                width:comboBox.width;
                height: comboBox.height;

                Text {
                    text: modelData
                    color:DefaultStyle.colorWhite //"white"
                    anchors.top: parent.top;
                    anchors.left: parent.left;
                    anchors.margins: 5;
                    font.family:DefaultStyle.fontRegular.name// "Arial"
                    font.pointSize: DefaultStyle.fontSize14 //14;
                }
                MouseArea {
                    anchors.fill: parent;
                    onClicked: {
                        comboBox.state = ""
                        var prevSelection = chosenItemText.text
                        chosenItemText.text = modelData
                        //console.log("Model Data :: ",chosenItemText.text)
                        if(chosenItemText.text != prevSelection){
                            comboBox.comboClicked();
                        }
                        listView.currentIndex = index;
                        currentIndex=index;
                    }
                }
            }
        }

    }

    Component {
        id: highlight
        Rectangle {
            width:comboBox.width;
            height:comboBox.height;
            color: "blue";
            radius: DefaultStyle.rectRadiusS//4
        }
    }
    states: State {
        name: "dropDown";
        PropertyChanges { target: dropDown; height:300 }
    }

    transitions: Transition {
        NumberAnimation { target: dropDown; properties: "height"; easing.type: Easing.OutExpo; duration: 1000 }
    }

}
