import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
//import QtQuick.Dialogs 1.2
import QtGraphicalEffects 1.0
import QtQuick.Controls.Styles 1.2
import "style" 1.0

Rectangle {
    id: fueListContainer
    width: parent.width
    height: parent.height
    color:DefaultStyle.colorBlack2 //"#141B25"
    signal backButtonClicked()
    signal mapToggleButtonClicked()
    //property string streetName
   // property string distance

    DefaultMouseArea
    {

    }


    ListView {
        id: listView1
        width: fueListContainer.width
        height: fueListContainer.height
        //        highlight: Rectangle{
        //            color: "#000000"
        //        }
        focus: true
        //onCurrentItemChanged: handleMenuClick(listView1.currentIndex,model.get(listView1.currentIndex).path);
        function handleMenuClick(item_index,name){
            console.log(item_index.path + "asdasda" +name );
        }
        clip: true
        snapMode: ListView.SnapToItem
        headerPositioning:ListView.OverlayHeader
        header:ListHeader_Maneuver{
            height:fueListContainer.height/9
                placeNameHeader: placeName
                placeDistance: pDistance
                placeETA: pETA
                separator:separatorVisible
            onThisBackButtonClicked: {
                console.log("backButtonClicked captured in QML")
                console.log(maneuverListModel[0].streetName,maneuverListModel[0].distance)
                console.log(maneuverListModel[1].streetName,maneuverListModel[1].distance)

              //  console.log(model.modeData.streetName)
                backButtonClicked()
            }
            onThisMapToggleButtonClicked: {
                console.log("mapToggleButtonClicked captured in QML")
                mapToggleButtonClicked()
            }
        }
        delegate: ManeuverList{
            //directionIcon
            imageId:model.modelData.imageId
            streetName:model.modelData.streetName
            distance:model.modelData.distance
            onListItemClicked: {
                console.log("List Item: " + index)
                console.log("model.modelData.imageId :",imageId)
                console.log("model.modeData.streetName :",streetName)
                //loadListDetail(index)
            }
        }
        model:maneuverListModel
//        model: ListModel {
//            ListElement {
//               locName : "1-21 soouth"
//               locDistance : " 0.50 mi."
//            }
//            ListElement {
//                locName : "1-21 soouth"
//                locDistance : " 0.50 mi."
//            }
//            ListElement{
//                locName : "1-21 soouth"
//                locDistance : " 0.50 mi."
//            }
//            ListElement {
//                locName : "1-21 soouth"
//                locDistance : " 0.50 mi."
//            }
//            ListElement{
//                locName : "You have arrived at your destination"
//                locDistance : " 0.50 mi."
//            }
//        }

    }

}

