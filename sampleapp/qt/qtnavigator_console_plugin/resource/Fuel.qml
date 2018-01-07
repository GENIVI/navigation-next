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
    color: DefaultStyle.colorBlack2 //"#141B25"
    focus: true
//    FontLoader{
//        id:sfRegular
//        name:  applicationDirPath+ "/resource/font/SF-UI-Display-Regular.otf"
//    }
    signal backButtonClicked()
    signal clearAllResentsClicked()
    signal clearAllFavouritesClicked()
    property int listItemPosition
    property alias listView1:listView1
    signal startNavigationClicked(int Object)
    signal deleteSelectedItem(string placeLatitude, string placeLongitude)
    signal loadListDetail(int Object)
    signal loadMoreData()
    signal mapToggleButtonClicked()
    signal focusOnListView()
    property bool showDelBtn: false

    DefaultMouseArea
    {

    }


    onFocusOnListView:{
        listView1.forceActiveFocus()
        listView1.focus=true
    }

    ListView {
        id: listView1
        width: fueListContainer.width
        height: fueListContainer.height - (fueListContainer.height/9 * 2)
        anchors.fill: fueListContainer
        clip: true
        //anchors.fill: parent
        //onCurrentItemChanged: handleMenuClick(listView1.currentIndex,model.get(listView1.currentIndex).path);
        function handleMenuClick(item_index,name){
            //console.log(item_index.path + "asdasda" +name );
        }
        Keys.onReturnPressed: {
            loadListDetail(listView1.currentIndex)
        }
        //clip: true
        // snapMode: ListView.SnapToItem
        headerPositioning:ListView.OverlayHeader
        header: ListHeaderCommon{
            height: fueListContainer.height/9
            objectName: "headerObject"
            placeName:categoryTitle
            lowestPetrolPrice: lowestFuelPrice
            avgPetrolPrice: AverageFuelPrice
            isRecents:categoryTitle==="Recents"
            isFavorite: categoryTitle==="Favorites"
            isListEmpty:locationListModel.length===0?true:false
            errorMessage:ErrorMessage
            onThisBackButtonClicked: {
                //console.log("backButtonClicked captured in QML")
                backButtonClicked()
            }
            onThisClearAllResentsClicked: {
                //console.log("thisClearAllResentsClicked captured in QML")
                clearAllResentsClicked()
            }
            onThisClearAllFavouritesClicked:{
                //console.log("onThisClearAllFavouritesClicked captured in QML")
                clearAllFavouritesClicked()
            }
            onThisDeleteFavourite: {
                showDelBtn= !showDelBtn;
            }

            Rectangle{
                id:noResultBox
                visible:locationListModel.length===0?true:false
                anchors.top: parent.bottom
                width:fueListContainer.width
                height:fueListContainer.width/9
                color:DefaultStyle.colorBlack2 //"#141B25"
                //  anchors.top: header.bottom
                border.width: DefaultStyle.borderWidth1 //1
                border.color: DefaultStyle.colorGrey1 //"grey"
                Text{
                    text:setNoResultBoxText//"No results found!"
                    anchors.centerIn: parent
                    //anchors.margins: 20
                    color: DefaultStyle.colorWhite //"white"
                    font.pixelSize: DefaultStyle.fontSize30 //30
                    font.family: DefaultStyle.fontRegular.name //sfRegular.name
                }
            }
            onMapButtonClicked: {

                //console.log("Fuel: mapToggleButtonClicked")
                mapToggleButtonClicked()

            }
        }

        delegate: ListItem{
            placeName: model.modelData.placeName!=="Error"?model.modelData.placeName:ErrorMessage
            placeAddress:  model.modelData.placeAddress
            petrolPrice:  model.modelData.petrolPrice
            placeDistance:  model.modelData.placeDistance
            isCheapestPetrol: model.modelData.isCheapestPetrol
            placeRating: model.modelData.placeRating
            isRatingVisble: model.modelData.placeRating==="0"?false:true
            isFav:  model.modelData.isFav
            modelCount: locationListModel.length
            isError: model.modelData.placeName!=="Error" /*&& model.modelData.placeRating==="0"*/?false:true
            isDeleteButtonClicked:model.modelData.placeName==="Error"?false:showDelBtn
            onListItemClicked: {
                //console.log("List Item: " + index)
                loadListDetail(index)
            }
            onNavigationButtonClicked: {
                //console.log("Fuel model: " + model.placeName)
                startNavigationClicked(index)
            }
            onDeleteItemClicked: {
                //console.log("selected model: " + model.placeName)
                deleteSelectedItem(model.placeLatitude,model.placeLongitude);
            }
        }
        footerPositioning:ListView.OverlayFooter
        footer:Rectangle {
            visible:hasMoreResults
            width:fueListContainer.width
            height:fueListContainer.height/9
            color:DefaultStyle.colorBlack2 //"#141B25"
            //  anchors.top: header.bottom
            border.width: DefaultStyle.borderWidth1 //1
            border.color: DefaultStyle.colorGrey1 //"grey"
            Text{
                text:"Load more results..."
                anchors.centerIn: parent
                //anchors.margins: 20
                color: DefaultStyle.colorWhite //"white"
                font.pixelSize: DefaultStyle.fontSize30 //30
                font.family: DefaultStyle.fontRegular.name //sfRegular.name
            }
            MouseArea{
                anchors.fill: parent

                onClicked:{
                    listItemPosition = locationListModel.length
                    loadMoreData()
                }
            }
        }
        onCountChanged: {
            //console.log(" last listItemPosition: "+listItemPosition)
            if(listItemPosition!==0){
                currentIndex = listItemPosition
                positionViewAtIndex(listItemPosition,
                                    ListView.Beginning);
            }

        }
        onModelChanged: {
            //console.log("locationListModel: "+locationListModel[0].placeName)
            //console.log(locationListModel.length)
            //console.log(JSON.stringify(locationListModel))
            showDelBtn= false;
        }

        //        MouseArea {
        //            anchors.fill:parent
        //            onClicked: {
        //                console.log("onClicked current index: " + listView1.currentIndex)
        //                console.log(JSON.stringify(listView1.currentItem.data))
        //            }
        //        }

        onCurrentIndexChanged: {
            //console.log("onClicked current index: " + listView1.currentIndex)
            if(listView1.currentIndex>listView1.count)
                listView1.currentIndex=listView1.currentIndex-2

        }

        model:locationListModel
        //        model:ListModel
        //            {
        //            ListElement {
        //                placeName: "Costco34@@"
        //                placeAddress:"12643 Aliso Creek, Aliso Viejo, CA 92656"
        //                petrolPrice:"$3.70"
        //                placeDistance:"0.2 mi."
        //                isFav:true
        //                isCheapestPetrol:false
        //                placeRating: 3.5
        //            }
        //            ListElement {
        //                placeName: "Costco 234234234"
        //                placeAddress:"12643 Aliso "
        //                petrolPrice:"$3.70"
        //                placeDistance:"0.2 mi."
        //                isFav:false
        //                isCheapestPetrol:true
        //                placeRating: 2
        //            }
        //            ListElement{
        //                placeName: "Costco 234234234"
        //                placeAddress:"12643 Aliso "
        //                // petrolPrice:"$3.70"
        //                placeDistance:"0.2 mi."

        //                isFav:false
        //                isCheapestPetrol:false
        //                placeRating: 5
        //            }
        //            ListElement {
        //                placeName: "Costco34@@"
        //                placeAddress:"12643 Aliso Creek, Aliso Viejo, CA 92656"
        //                petrolPrice:"$3.70"
        //                placeDistance:"0.2 mi."
        //                isFav:true
        //                isCheapestPetrol:false
        //                placeRating: 0
        //            }
        //            ListElement {
        //                placeName: "Costco 234234234"
        //                placeAddress:"12643 Aliso "
        //                petrolPrice:"$3.70"
        //                placeDistance:"0.2 mi."
        //                isFav:false
        //                isCheapestPetrol:true
        //                placeRating: 4.5
        //            }
        //            ListElement{
        //                placeName: "Costco 234234234"
        //                placeAddress:"12643 Aliso "
        //                // petrolPrice:"$3.70"
        //                placeDistance:"0.2 mi."

        //                isFav:false
        //                isCheapestPetrol:false
        //                placeRating: 1
        //            }
        //        }

    }

}

