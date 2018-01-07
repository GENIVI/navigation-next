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

#include <QGuiApplication>
#include "searchdetailscontroller.h"
#include "placemanager.h"

SerachDetailsController::SerachDetailsController(QQuickItem* parent):
    QQuickItem(parent),
    isCatergoryPinDetails(false),
    isSearchDetailPinDetails(false),
    mDetailInfoView(NULL){

}
SerachDetailsController::~SerachDetailsController()
{
    if(mDetailInfoView != NULL) {
        delete mDetailInfoView;
        mDetailInfoView = NULL;
    }
    if(mMessageBoxView !=NULL){
        delete mMessageBoxView;
        mMessageBoxView = NULL;
    }
}
void SerachDetailsController::setUp() {
    qDebug()<<"******************* SerachDetailsController::setUp *****************";
    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
    mAppDirPath = engine->rootContext()->contextProperty("applicationDirPath").toString();
    //engine->rootContext()->setContextProperty("itemDetail",mItemDetails);

    QQmlComponent component(engine,QUrl(mAppDirPath+"/resource/FuelDetails.qml"));

    mDetailInfoView  = qobject_cast<QQuickItem*>(component.create());
    QQmlEngine::setObjectOwnership(mDetailInfoView, QQmlEngine::CppOwnership);
    mDetailInfoView->setParentItem(this->parentItem()->parentItem()/*->parentItem()*/);
    //From mainmenu
    connect(mDetailInfoView, SIGNAL(callButtonClicked(QString, QString,QString)) , this, SLOT(on_callButton_clicked(QString, QString,QString)));
    //    connect(mDetailInfoView, SIGNAL(showNearThisController()), this, SLOT(onShowNearThisController()));
    connect(mDetailInfoView, SIGNAL(addToFavoritesClicked()), this, SLOT(onFoundCustomFavourite()));
    //    connect(mDetailInfoView, SIGNAL(pinNavigationClicked(QString, int, QVariant)) , this, SLOT(OnStartNavigationToDestination(QString, int, QVariant)));
    //    connect(mDetailInfoView, SIGNAL(toggleMapViewFromPinInformation()),   this, SLOT(onToggleMapViewFromPinDetail()));
    //from setup
    connect(mDetailInfoView, SIGNAL(detailBackButtonClicked()), this, SLOT(on_backButton_clicked()) );

    //mDir = QFileInfo(QCoreApplication::applicationFilePath()).dir();
    //   connect( mDetailInfoView, SIGNAL(callButtonClicked(QString,QString,QString)), this, SLOT(on_callButton_clicked(QString,QString,QString)) );
    connect( mDetailInfoView, SIGNAL(mapButtonClicked()), this, SLOT(onMapButtonClicked()) );
    connect( mDetailInfoView, SIGNAL(navigationButtonClicked()), this, SLOT(onNavigationButtonClicked()) );
    InitializeToggleBar();
    this->hide();



    qDebug()<<"******************* SerachDetailsController::setUp End*****************";
}
void SerachDetailsController :: onNavigationButtonClicked()
{
    qDebug()<<"SerachDetailsController :: onNavigationButtonClicked";
    QVariant itemVariant = mJsonObject;
    closeAllPopUpWindows();
    mToggleHeaderBar->setVisible(false);

    //emit SignalNavigationButtonClicked("placeName", 1, itemVariant);
    emit SignalNavigationButtonClicked(mItemDetails->placeName(), 0, itemVariant);
}

void SerachDetailsController::InitializeToggleBar()
{
    // create top-bar for toggle between map/list/detail views
    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
    QQmlComponent componentToggleHeaderBar(engine, QUrl(mAppDirPath+"/resource/MapViewHeader.qml"));
    mToggleHeaderBar = qobject_cast<QQuickItem*>(componentToggleHeaderBar.create());
    connect( mToggleHeaderBar, SIGNAL(thisBackButtonClicked()), this, SLOT(onToggleHeaderBarBackButtonClicked()) );
    QQmlEngine::setObjectOwnership(mToggleHeaderBar, QQmlEngine::CppOwnership);

    //    QObject *mapButton = mToggleHeaderBar->findChild<QObject*>("mapIconObject");
    //    if (mapButton) {
    //        mapButton->setProperty("visible", false);
    //    }

    mToggleHeaderBar->setParentItem(mDetailInfoView->parentItem());//to get it visible
    //mToggleHeaderBar->setParent(this);
    mToggleHeaderBar->setPosition(QPoint(0,0));
    mToggleHeaderBar->setSize(QSize(this->width(),  this->height()/9));

    mToggleHeaderBar->setVisible(false);
}

void SerachDetailsController::onToggleHeaderBarBackButtonClicked() {
    emit sideItemVisibility(false);
    qDebug()<<"SerachDetailsController::onToggleHeaderBarBackButtonClicked";
    mToggleHeaderBar->setVisible(false);

    //    if (mCurrentListWidget == mCategorySearchController->getCategorySCQuickItem()) {
    //        //mCurrentListWidget->show();
    //        mCurrentListWidget->setVisible(true);
    //    }
    //    else if (mCurrentListWidget == mPinInformationWidget->getPinInfoQuickItem()) {
    //        //        mCategorySearchController->show();
    //        //        mCurrentListWidget->show();
    //        mCategorySearchController->getCategorySCQuickItem()->setVisible(true);
    //        mCurrentListWidget->setVisible(true);
    //    }
    this->show();
    isSearchDetailPinDetails = false;

}

void  SerachDetailsController :: onMapButtonClicked() {

    qDebug()<<"SerachDetailsController :: onMapButtonClicked()"<<isCatergoryPinDetails;

    if(!isCatergoryPinDetails)
    {
        QObject *mapButton = mToggleHeaderBar->findChild<QObject*>("placeObject");
        if (mapButton) {
            mapButton->setProperty("text", mItemDetails->placeName());
        }
        mToggleHeaderBar->setVisible(true);
        isSearchDetailPinDetails = true;
    }
    closeAllPopUpWindows();
}

void SerachDetailsController::closeAllPopUpWindows() {

    this->hide();
    emit SignalCloseAllPopUpWindows();

}
void SerachDetailsController::setItemDetails(DataObject *itemInfo, QJsonObject jsonObject)
{
    mItemDetails = itemInfo;
    mJsonObject = jsonObject;
    qDebug()<<"SerachDetailsController::setItemDetails::"<<itemInfo->placeCategoryImagePath();
}

void SerachDetailsController::on_callButton_clicked(QString placeName,QString placeContact, QString placeCategoryImagePath) {
    qDebug() << "on_callButton_clicked: ";
    mCallView = new CallViewController();
    QQmlEngine::setObjectOwnership(mCallView, QQmlEngine::CppOwnership);
    mCallView->setParentItem(mDetailInfoView);
    QQmlEngine::setContextForObject(mCallView, QQmlEngine::contextForObject(this));
    mCallView->setWidth(width());
    mCallView->setHeight(height());
    mCallView->setPlaceDetails(placeName,placeContact,placeCategoryImagePath);
    mCallView->setUp();

    mCallView->setVisible(true);
}
void SerachDetailsController :: on_backButton_clicked() {

    qDebug() << "SerachDetailsController :: on_backButton_clicked: ";


    mToggleHeaderBar->setVisible(false);
    this->hide();
    if(isCatergoryPinDetails)
        emit closeAllPopUpWindows();
    else
        emit SignalSetVisible();




}

void SerachDetailsController::onFoundCustomFavourite() {

    qDebug() << "SerachDetailsController::onFoundCustomFavourite()..................." << mAppDirPath;

    PlaceManager* placeManager = PlaceManager::getInstance();
    placeManager->setAppDirPath(mAppDirPath);

    qDebug()<<"onFoundCustomFavourite "<<mItemDetails->isFav();

    if(mItemDetails->isFav()) {
        if(placeManager->addToFavorites(mJsonObject))
        {
            qDebug()<<"Fav Added Added...... ";

        }else{
            qDebug()<<"Fav Already Added...... ";
            if(placeManager->removeFromFavorites(mItemDetails->placeLatitude().toDouble(), mItemDetails->placeLongitude().toDouble()))
            {
                qDebug() <<"Favourite successfully removed...";
                mItemDetails->setIsFav(false);
            }else
            {
                qDebug() << "Error removing Favourites....";
            }

        }
    }
    else {
        qDebug()<<"onFoundCustomFavourite....inside else....... ";

        placeManager->removeFromFavorites(mItemDetails->placeLatitude().toDouble(), mItemDetails->placeLongitude().toDouble());
    }

    //  Get the filename and location to write the favorite POI data
    //    QString filePostFix = getFavouriteWriteLocation();
    //    mCurrentItemDetails["placeLatitude"] = mItemDetails->placeLatitude();
    //    mCurrentItemDetails["placeLongitude"] = mItemDetails->placeLongitude();

    //    qDebug()<< mCurrentItemDetails["placeLatitude"].toString().toDouble();
    //    qDebug()<< mCurrentItemDetails["placeLongitude"].toString().toDouble();

    //    double latitude = mCurrentItemDetails["placeLatitude"].toString().toDouble();
    //    double longitude = mCurrentItemDetails["placeLongitude"].toString().toDouble();

    //    QString* existFavoriteFile = new QString();
    //    //  Check if a favorite POI already exists in local memory
    //    if(!favouriteExists(latitude, longitude, existFavoriteFile,mAppDirPath)) {

    //        QString favouritesFile = mAppDirPath + "/resource/jsons/favourites/favourites_"+ filePostFix +".json";
    //        QFile jsonFile(favouritesFile);
    //        jsonFile.open(QFile::WriteOnly);
    //        mCurrentItemDetails["isFav"] = true;
    //        jsonFile.write(QJsonDocument(mCurrentItemDetails).toJson(QJsonDocument::Compact));
    //        jsonFile.close();

    //        //   Write the subsequent information to the favorites configuration file
    //        if(writeFavouritesConfig(filePostFix, latitude, longitude)) {
    //            //   Custom message box to show pop up information

    //            qDebug()<<"Favorite successfully added";
    //            QObject *listObject = mDetailInfoView->findChild<QObject*>("favObject");
    //            if (listObject)
    //            {
    //                listObject->setProperty("source","file:///" + mAppDirPath +"/resource/images/location_detail/addtofav_icn_selected.png");
    //            }
    //            mMessageBoxView = new Messageboxcontroller();
    //            QQmlEngine::setObjectOwnership(mMessageBoxView, QQmlEngine::CppOwnership);
    //            mMessageBoxView->setParentItem(this);
    //            QQmlEngine::setContextForObject(mMessageBoxView, QQmlEngine::contextForObject(this));
    //            mMessageBoxView->setWidth(width());
    //            mMessageBoxView->setHeight(height());
    //            QString var1,var2;
    //            var1="Favorite successfully";
    //            var2="added!";
    //            mMessageBoxView->messageStrings(var1,var2);
    //            mMessageBoxView->setUp();
    //            connect( mMessageBoxView, SIGNAL(okClicked()), this, SLOT(onOkButtonClicked()) );

    //        }
    //        else {
    //            //   Custom message box to show pop up information

    //            qDebug()<<"Favorite could not be added. Try again!";
    //            mMessageBoxView = new Messageboxcontroller();
    //            QQmlEngine::setObjectOwnership(mMessageBoxView, QQmlEngine::CppOwnership);
    //            mMessageBoxView->setParentItem(this);
    //            QQmlEngine::setContextForObject(mMessageBoxView, QQmlEngine::contextForObject(this));
    //            mMessageBoxView->setWidth(width());
    //            mMessageBoxView->setHeight(height());
    //            QString var1,var2;
    //            var1="Favorite could not be added.";
    //            var2="Try again!";
    //            mMessageBoxView->messageStrings(var1,var2);
    //            mMessageBoxView->setUp();
    //            connect( mMessageBoxView, SIGNAL(okClicked()), this, SLOT(onOkButtonClicked()) );

    //        }
    //    }
    //    else {
    //        //   Custom message box to show pop up information

    //        qDebug()<<"Favorite exists!";
    //        mMessageBoxView = new Messageboxcontroller();
    //        QQmlEngine::setObjectOwnership(mMessageBoxView, QQmlEngine::CppOwnership);
    //        mMessageBoxView->setParentItem(this);
    //        QQmlEngine::setContextForObject(mMessageBoxView, QQmlEngine::contextForObject(this));
    //        mMessageBoxView->setWidth(width());
    //        mMessageBoxView->setHeight(height());
    //        QString var1,var2;
    //        var1="Already added to";
    //        var2="favorites!";
    //        mMessageBoxView->messageStrings(var1,var2);
    //        mMessageBoxView->setUp();
    //        connect( mMessageBoxView, SIGNAL(okClicked()), this, SLOT(onOkButtonClicked()) );
    //    }

}
void SerachDetailsController::onOkButtonClicked(){

    mMessageBoxView->setVisible(false);
    //    delete mMessageBoxView;
    //    mMessageBoxView = NULL;

}

void SerachDetailsController::SetLtkMapkit(MapWidget *LTKMapkit)
{
    mLTKMapKit = LTKMapkit;
}
void SerachDetailsController::show()
{
    this->setVisible(true);
    mDetailInfoView->setVisible(true);
    mToggleHeaderBar->setVisible(false);
    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
    engine->rootContext()->setContextProperty("itemDetail",mItemDetails);
}
void SerachDetailsController::hide()
{
    this->setVisible(false);
    mDetailInfoView->setVisible(false);
}
