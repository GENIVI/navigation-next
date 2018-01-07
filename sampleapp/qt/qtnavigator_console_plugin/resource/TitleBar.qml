import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.1
import "style" 1.0
Rectangle {
    id:titlebar
    width: parent.width
    height: parent.height
    color: DefaultStyle.colorBlack1

    signal hamburgerMenuButtonClicked()
    signal settingsButtonClicked()
    signal showSuggestedSearch(string searchText)
    signal mapInitialized()
    signal hideAllPopUp()

    MouseArea{
        anchors.fill: parent
        onClicked:{
            titlebar.focus = true
            console.log("hideAllPopUp")
            hideAllPopUp();
        }
    }
    Image{
        id:hamburgerMenuButton
        source: applicationDirPath+ "/resource/images/hamburger_icn.png"
        anchors.verticalCenter: parent.verticalCenter
        fillMode: Image.PreserveAspectFit
        MouseArea{
            anchors.fill: parent
            onClicked: {
                mainSearchLineEdit.focus = false
                console.log("Hamburger Button Clicked")
                hamburgerMenuButtonClicked();
            }

        }

    }
    Item{
        id: searchBox
        width: searchLine.width + serachButton.width
        height: parent.height
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        RowLayout{
        ColumnLayout{
            Row{
                Image {
                    id: searchIcon
                    anchors.verticalCenter:  parent.verticalCenter
                    source: applicationDirPath+ "/resource/images/search_icn.png"
                    fillMode: Image.PreserveAspectFit
                        height: searchBox.height * 0.6
                }
                TextField {
                    id: mainSearchLineEdit
                    anchors.verticalCenter:  parent.verticalCenter
                    //focus: isfocus
                    placeholderText: qsTr("Search")
                        font.pixelSize: DefaultStyle.fontSize30
                    style: TextFieldStyle{
                        background: Rectangle{
                                color:DefaultStyle.colorTransparent
                        }
                            placeholderTextColor: DefaultStyle.colorGrey1
                            textColor: DefaultStyle.colorWhite
                    }
                    Keys.onReturnPressed  :{
                        mainSearchLineEdit.focus=false
                        showSuggestedSearch(mainSearchLineEdit.text)
                        mainSearchLineEdit.text= "";
                    }
                    Keys.onEnterPressed: {
                        mainSearchLineEdit.focus=false
                        showSuggestedSearch(mainSearchLineEdit.text)
                        mainSearchLineEdit.text= "";
                    }
                    onCursorPositionChanged:
                    {
                        if(mainSearchLineEdit.length==3){
                            console.log(mainSearchLineEdit.text)
                        }
                    }
                    MouseArea{
                        anchors.fill: mainSearchLineEdit
                        onPressed:{
                            mouse.accepted = false;
                            mainSearchLineEdit.focus = true
                            console.log("hideAllPopUp")
                            hideAllPopUp();
                        }
                    }

                }
                }
                Image {
                    id: searchLine
                    width: mainSearchLineEdit.width
                    source: applicationDirPath+ "/resource/images/search_bottomline.png"
                    fillMode: Image.PreserveAspectFit
                }
            }
                Image {
                    id: micButton
                height: searchBox.height * 0.6
                    anchors.verticalCenter:  parent.verticalCenter
                    source: applicationDirPath+ "/resource/images/mic_icn.png"
                    fillMode: Image.PreserveAspectFit
                    MouseArea{
                        anchors.fill: parent
                        onClicked:{
                            console.log(mainSearchLineEdit.text)
                            showSuggestedSearch(mainSearchLineEdit.text)
                            mainSearchLineEdit.text= "";
                        }
                    }
                }
                Image {
                    id: serachButton
                    anchors.verticalCenter:  parent.verticalCenter
                    source: applicationDirPath+ "/resource/images/Search.png"
                    fillMode: Image.PreserveAspectFit
                    MouseArea{
                        anchors.fill: parent
                        onClicked:{
                            mainSearchLineEdit.focus=false
                            console.log(mainSearchLineEdit.text)
                            showSuggestedSearch(mainSearchLineEdit.text)
                            mainSearchLineEdit.text= "";
                        }
                    }
                }
            }
    }
    Image {
        id: settingButton
        source: applicationDirPath+ "/resource/images/settings_icn.png"
        anchors.verticalCenter: parent.verticalCenter
        anchors.right: parent.right
        anchors.rightMargin: 10
        fillMode: Image.PreserveAspectFit
        MouseArea{
            anchors.fill: parent
            onClicked:{
                mainSearchLineEdit.focus = false
                console.log("Settings Button Clicked")
                settingsButtonClicked();
            }
        }
    }
}
