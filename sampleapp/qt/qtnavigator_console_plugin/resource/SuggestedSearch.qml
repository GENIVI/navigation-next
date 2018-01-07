import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
//import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import QtQuick.Controls.Styles 1.2

Rectangle {
    id: suggestedSearchContainer
    width: mainRec.width
    height: mainRec.height
    color: "#141B25"
    ListView {
        id: listView1
        width: parent.width
        height: parent.height
        //        highlight: Rectangle{
        //            color: "#000000"
        //        }
        focus: true
        //onCurrentItemChanged: handleMenuClick(listView1.currentIndex,model.get(listView1.currentIndex).path);
        function handleMenuClick(item_index,name){
            console.log(item_index.path + "asdasda" +name );
        }
        //clip: true
        snapMode: ListView.SnapToItem
        headerPositioning:ListView.OverlayHeader
        header: SuggestedSearchBarHeader{}

        delegate: Rectangle{
            id:listItem
            width:parent.width
            height:80
            color:"transparent"
            border.width: listView1.currentIndex == index ?2:0
            border.color: listView1.currentIndex == index ? "#0FD9FD":""
            Text{
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 100
                text:placeName
                color:"white"
                font.pixelSize: 23
                font.family: sfRegular.name
            }
            MouseArea {
                anchors.fill:parent
                onClicked: {
                    listView1.currentIndex=index
                }
            }
        }
        model: ListModel {
            ListElement {
                placeName: "Joe's Coffee Shop, CA, USA"
            }
            ListElement {
                placeName: "Joe's Coffee Shop, CA, USA,asdklkljuiasdkli"
            }
            ListElement{
                placeName: "Costco 234234234"
            }
        }
    }
}

