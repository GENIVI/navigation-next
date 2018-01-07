/*
Copyright (c) 2018, TeleCommunication Systems, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
   * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the TeleCommunication Systems, Inc., nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED. IN NO EVENT SHALL TELECOMMUNICATION SYSTEMS, INC.BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include<QQmlEngine>
#include<QQmlContext>

#include "hamburgermenucontroller.h"

HamburgerMenuController::HamburgerMenuController(QQuickItem* parent):
    QQuickItem(parent),
    mMenuView(NULL){


}
HamburgerMenuController::~HamburgerMenuController()
{
    if(mMenuView != NULL) {
        delete mMenuView;
        mMenuView = NULL;
    }
    //    if(mCategorySearchController != NULL) {
    //        delete mCategorySearchController;
    //        mCategorySearchController = NULL;
    //    }
    if(mNearMeController != NULL) {
        delete mNearMeController;
        mNearMeController = NULL;
    }
}
void HamburgerMenuController::setUp()
{
    qDebug()<<"******************* HamburgerMenuController::setUp *****************";

    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
    mAppDirPath = engine->rootContext()->contextProperty("applicationDirPath").toString();
    QQmlComponent component(engine,QUrl(mAppDirPath+"/resource/HamburgerMenu.qml"));
    mMenuView  = qobject_cast<QQuickItem*>(component.create());

    QQmlEngine::setObjectOwnership(mMenuView, QQmlEngine::CppOwnership);
    mMenuView->setParentItem(this);
    connect(mMenuView, SIGNAL(closeSideMenus()), this, SLOT(onCloseSideMenus()) );
    connect(mMenuView, SIGNAL(nearMeClicked()), this, SLOT(onNearMeButtonClicked()) );
    connect( mMenuView, SIGNAL(moviesButtonClicked()), this, SLOT(onMoviesButtonClicked()) );
    connect( mMenuView, SIGNAL(gasStationButtonClicked()), this, SLOT(onGasStationButtonClicked()) );
    connect(mMenuView, SIGNAL(favouritesButtonClicked()), this , SLOT(onFavouritesButtonClicked()) );
    connect(mMenuView, SIGNAL(recentsButtonClicked()),    this , SLOT(onRecentsButtonClicked()) );

}


void HamburgerMenuController::CloseAllPopUpWindows()
{
    //qDebug()<<"HamburgerMenuController::CloseAllPopUpWindows";
    //this->setVisible(false);
    //    mMenuView->setVisible(false);
    onCloseSideMenus();


}

void HamburgerMenuController::onCloseSideMenus()
{
    QObject *listObject = mMenuView->findChild<QObject*>("listObject");
    if (listObject)
    {
        listObject->setProperty("currentIndex", -1);
    }
    mMenuView->setVisible(false);
    this->setVisible(false);

}
void HamburgerMenuController::onNearMeButtonClicked() {
    //Need to implement NearMeController
    qDebug()<<"HamburgerMenuController::onNearMeButtonClicked called ..................";

    mNearMeController = new NearMeController();
    QQmlEngine::setObjectOwnership(mNearMeController, QQmlEngine::CppOwnership);
    mNearMeController->setParentItem(this);
    QQmlEngine::setContextForObject(mNearMeController, QQmlEngine::contextForObject(this));
    mNearMeController->setWidth(getParentWidth()/3);
    mNearMeController->setHeight(height());
    mNearMeController->setPosition(QPointF(width(),0));

    mNearMeController->setUp();
    connect( mNearMeController, SIGNAL(nearMeItemsClicked(QVariant,bool)), this, SLOT(onNearMeItemsClicked(QVariant,bool)) );
}
void HamburgerMenuController::onNearMeItemsClicked(QVariant parameter,bool isNearByPinLocation){

    onCloseSideMenus();
    mNearMeController->setVisible(false);
    emit CategorySearchSelected(parameter,isNearByPinLocation);

}

void HamburgerMenuController::onMoviesButtonClicked() {

    // onCloseSideMenus();
    //  Set category to Theatres and enable search for Theatres near by
    onCloseSideMenus();
    QJsonObject categoryParameter;
    categoryParameter["name"] = "Theaters";
    categoryParameter["code"] = "ABE";
    QVariant parameter = categoryParameter;

    //  Clear older pre populated list for re-use
    // mCategorySearchController->cleanUp();

    //  Call SDK for making a category search request
    //mCategorySearchController->searchSDKRequest(parameter);

    //  Show List Screen

    emit CategorySearchSelected(parameter);
}

void HamburgerMenuController::onGasStationButtonClicked() {

    //onCloseSideMenus();
    onCloseSideMenus();
    //  Set category to Fuel Stations and enable search for Fuel Stations near by
    QJsonObject categoryParameter;
    categoryParameter["code"] = "ACC";
    categoryParameter["name"] = "Fuel Stations";
    QVariant parameter = categoryParameter;

    emit CategorySearchSelected(parameter);

}
void HamburgerMenuController::onFavouritesButtonClicked() {
    qDebug()<<"HamburgerMenuController::onFavouritesButtonClicked called ..................";

    onCloseSideMenus();


    //  Read Existing favorite POIs from local file

    //       QVector<QString> pathList = mCategorySearchController->readExsistingFavourite(mAppDirPath);
    //       for(QVector<QString>::iterator it = pathList.begin(); it != pathList.end(); ++it) {
    //       qDebug()<<*it<<"\n";
    //       }

    //  Display the Favorites window(Category Search Controller) depending on the
    //  existing files on the local machine
    //    if(pathList.size() > 0) {
    //mCategorySearchController->displayExistingFavourites();
    //    }
    //    else {
    //        mCategorySearchController->showSingleListItemWidget("Favorites","No favorites to show!");
    //    }
    emit FavouriteButtonClicked();
}

void HamburgerMenuController::onRecentsButtonClicked() {

    onCloseSideMenus();


    //  Clear older pre populated list for re-use
    //mCategorySearchController->cleanUp();

    //  Read Existing recent POIs from local file

    //QVector<QString> pathList = mCategorySearchController->readExsistingRecents(mAppDirPath);
    //for(QVector<QString>::iterator it = pathList.begin(); it != pathList.end(); ++it){
    //qDebug()<<*it<<"\n";
    //}

    //    //  Display the Recents window(Category Search Controller) depending on the
    //    //  existing files on the local machine
    //if(pathList.size() > 0) {
    // mCategorySearchController-> displayExistingRecents();
    //}
    // else {
    //    mCategorySearchController->showSingleListItemWidget("Recents","No recents to show!");
    //}
    emit RecentButtonClicked();
}
