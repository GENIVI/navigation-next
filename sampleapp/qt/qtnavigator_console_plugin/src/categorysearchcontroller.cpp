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

#include "categorysearchcontroller.h"
#include <QGuiApplication>
#include "custombubble.h"
#include "placemanager.h"
#include "GPSFile.h"

CategorySearchController::CategorySearchController(QQuickItem* parent):
    QQuickItem(parent),
    mIsNavigationStarted(false),
    mSearchView(NULL){


    //    mWaitDialog = new waitdialog();
    //    QQmlEngine::setObjectOwnership(mWaitDialog, QQmlEngine::CppOwnership);
    //    mWaitDialog->setParentItem(this);
    //    QQmlEngine::setContextForObject(mWaitDialog, QQmlEngine::contextForObject(this));
    //    mWaitDialog->setWidth(width());
    //    mWaitDialog->setHeight(height());
    //    mWaitDialog->setUp();
}
CategorySearchController::~CategorySearchController()
{
    if(mSearchView != NULL) {
        delete mSearchView;
        mSearchView = NULL;
    }

    if(mWaitDialog !=NULL) {
        delete mWaitDialog;
        mWaitDialog = NULL;
    }

    if(mMessageBox !=NULL) {
        delete mMessageBox;
        mMessageBox = NULL;
    }

}

void CategorySearchController::setUp() {
    qDebug()<<"******************* CategorySearchController::setUp *****************";

    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
    mAppDirPath = engine->rootContext()->contextProperty("applicationDirPath").toString();
    QQmlComponent component(engine,QUrl(mAppDirPath+"/resource/Fuel.qml"));

    mSearchView  = qobject_cast<QQuickItem*>(component.create());

    QQmlEngine::setObjectOwnership(mSearchView, QQmlEngine::CppOwnership);
    mSearchView->setParentItem(this->parentItem());
    QQmlEngine::setContextForObject(mSearchView, QQmlEngine::contextForObject(this));

    qDebug()<<"mSearchView Width : "<<mSearchView->width()<<"Height : "<<mSearchView->height();

    PlaceManager* placeManager = PlaceManager::getInstance();
    placeManager->setAppDirPath(mAppDirPath);

    // SearchView->setModality(Qt::WindowModal);
    mSearchView->setFlag(ItemHasContents);
    connect( mSearchView, SIGNAL(backButtonClicked()), this, SLOT(on_backButton_clicked()) );
    connect(mSearchView, SIGNAL(clearAllResentsClicked()) , this, SLOT(onClearAllResentsClicked()));
    connect(mSearchView, SIGNAL(clearAllFavouritesClicked()) , this, SLOT(onClearAllFavouritesClicked()));
    connect(mSearchView, SIGNAL(deleteSelectedItem(QString,QString)) , this, SLOT(onDeleteSelectiveFav(QString,QString)));
    connect(mSearchView, SIGNAL(mapToggleButtonClicked()) , this, SLOT(on_mapToggleButton_clicked()));
    connect( mSearchView, SIGNAL(startNavigationClicked(int)), this, SLOT(onStartNavigationClicked(int)) );
    connect( mSearchView, SIGNAL(loadListDetail(int)), this, SLOT(onLoadListDetail(int)) );
    connect( mSearchView, SIGNAL(loadMoreData()), this, SLOT(loadMoreSearchSDKRequest()) );
    connect( this, SIGNAL(SetListViewFocus()),mSearchView,SIGNAL(focusOnListView()));

    mSearchView->setVisible(false);

    mWaitDialog = new waitdialog();
    QQmlEngine::setObjectOwnership(mWaitDialog, QQmlEngine::CppOwnership);
    mWaitDialog->setParentItem(this->parentItem());
    QQmlEngine::setContextForObject(mWaitDialog, QQmlEngine::contextForObject(this));
    mWaitDialog->setWidth(width());
    mWaitDialog->setHeight(height());
    mWaitDialog->setUp();

    mWaitDialog->setVisible(false);

    mDetailInfoView = new SerachDetailsController();
    mDetailInfoView->setParentItem(this);
    QQmlEngine::setObjectOwnership(mDetailInfoView, QQmlEngine::CppOwnership);
    QQmlEngine::setContextForObject(mDetailInfoView, QQmlEngine::contextForObject(this));
    mDetailInfoView->setWidth(width());
    mDetailInfoView->setHeight(height());
    mDetailInfoView->setUp();

    connect(mDetailInfoView,SIGNAL(SignalCloseAllPopUpWindows()),this,SLOT(onCloseAllPopUpWindows()));
    connect(mDetailInfoView,SIGNAL(SignalNavigationButtonClicked(QString, int, QVariant)),this,SLOT(onNavigationButtonClicked(QString, int, QVariant)));
    connect(mDetailInfoView,SIGNAL(SignalSetVisible()),this,SLOT(onSetVisible()));

    SettingFileName = LTKSampleUtil::GetResourceFolder()+"sampleapp.ini";

    InitializeToggleBar();
    qDebug()<<"******************* CategorySearchController::setUp End *****************";
}
void CategorySearchController::InitializeToggleBar()
{
    // create top-bar for toggle between map/list/detail views
    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
    QQmlComponent componentToggleHeaderBar(engine,QUrl(mAppDirPath+"/resource/MapViewHeader.qml"));
    mToggleHeaderBar = qobject_cast<QQuickItem*>(componentToggleHeaderBar.create());
    connect( mToggleHeaderBar, SIGNAL(thisBackButtonClicked()), this, SLOT(onToggleHeaderBarBackButtonClicked()) );
    QQmlEngine::setObjectOwnership(mToggleHeaderBar, QQmlEngine::CppOwnership);

    //    QObject *mapButton = mToggleHeaderBar->findChild<QObject*>("mapIconObject");
    //    if (mapButton) {
    //        mapButton->setProperty("visible", false);
    //    }

    qDebug()<<"mToggleHeaderBar::"<<mToggleHeaderBar;
    mToggleHeaderBar->setParentItem(this->parentItem());
    //mToggleHeaderBar->setParent(this);
    mToggleHeaderBar->setPosition(QPoint(0,0));
    mToggleHeaderBar->setSize(QSize(this->parentItem()->width(), this->parentItem()->height()/9));
    mToggleHeaderBar->setVisible(false);


}

void CategorySearchController::onToggleHeaderBarBackButtonClicked() {

    qDebug()<<"CategorySearchController::onToggleHeaderBarBackButtonClicked";
    mToggleHeaderBar->setVisible(false);
    mSearchView->setVisible(true);
    this->setVisible(true);
    mMapViewController->onSideItemVisibility(false);
    mDetailInfoView->isCatergoryPinDetails = false;
    mLTKMapKit->SetGpsMode(locationtoolkit::MapWidget::GM_FOLLOW_ME,false);

}
void CategorySearchController::onNavigationButtonClicked(QString placeName, int val, QVariant itemDetails)
{
    mMapViewController->onSideItemVisibility(true);
    mIsNavigationStarted = true;
    if(mDetailInfoView->isCatergoryPinDetails)
    {
        mDetailInfoView->isCatergoryPinDetails=false;
        mToggleHeaderBar->setVisible(false);
    }
    emit SignalNavigationButtonClicked(placeName,val,itemDetails);
}

void CategorySearchController::onSetVisible()
{
    mSearchView->setVisible(true);
    switch (eSearchType) {
    case IN_FAV:
        displayExistingFavourites(false);
        qDebug() << "Back To Fav List++++++++++++++++++++++++++++++++++";
        break;
    case IN_RECENTS:
        displayExistingRecents(false);
        qDebug() << "Back To Recents List++++++++++++++++++++++++++++++++++";
        break;
    case IN_SEARCH:
        qDebug() << "Back To Search List++++++++++++++++++++++++++++++++++";
        break;
    default:
        break;
    }
}

void CategorySearchController::on_backButton_clicked() {

    qDebug()<<"CategorySearchController::on_backButton_clicked";
    eSearchType = NONE;
    //    this->setVisible(false);
    //    mSearchView->setVisible(false);
    this->hide();
    mMapViewController->onSideItemVisibility(true);
    mLTKMapKit->DeleteAllPins();
    mMapViewController->onUpdateCameraParam();
    emit ToggleTitleBarVisibility(true);
    qDebug() << "on_backButton_clicked";
}
void CategorySearchController::onLoadListDetail(int index) {
    qDebug() << "\n\n Clicked Item" << index;
    qDebug() << mLocationDataList[index];

    QJsonObject currentItemDetails = mSearchedJsonObjectVector.at(index);
    OnSearchResultClicked(currentItemDetails);

    PlaceManager* placeManager = PlaceManager::getInstance();
    placeManager->setAppDirPath(mAppDirPath);
    placeManager->addToRecents(mSearchedJsonObjectVector.at(index));//mLocationPins_10.at(index).toObject());
    DataObject *itemDetail = (DataObject *) mLocationDataList[index];
    mDetailInfoView->setItemDetails(itemDetail, currentItemDetails);//mLocationPins_10.at(index).toObject());
    mDetailInfoView->show();
    this->hide();
    mMapViewController->onSideItemVisibility(false);
    mMapViewController->setPOIPosition(QPointF(itemDetail->placeLatitude().toDouble(),itemDetail->placeLongitude().toDouble()),itemDetail->placeName());

}
void CategorySearchController::onStartNavigationClicked(int index) {
    qDebug() << "CategorySearchController - onStartNavigationClicked" << index;
    DataObject *itemDetail = (DataObject *) mLocationDataList[index];
    mSearchView->setVisible(false);
    this->setVisible(false);
    mToggleHeaderBar->setVisible(false);
    mIsNavigationStarted = true;
    QVariant itemVariant = mSearchedJsonObjectVector.at(index);

    PlaceManager* placeManager = PlaceManager::getInstance();
    placeManager->addToRecents(mSearchedJsonObjectVector.at(index));//mLocationPins_10.at(index).toObject());
    mMapViewController->onSideItemVisibility(true);
    mMapViewController->setPOIPosition(QPointF(itemDetail->placeLatitude().toDouble(),itemDetail->placeLongitude().toDouble()),itemDetail->placeName());

    emit SignalNavigationButtonClicked(itemDetail->placeName(), 1, itemVariant);

}

void CategorySearchController::onCloseAllPopUpWindows() {

    qDebug()<<"CategorySearchController::onCloseAllPopUpWindows"<<mDetailInfoView->isCatergoryPinDetails;
    this->setfavouriteRecentsFlag(false, false);

    mSearchView->setVisible(false);

    if(mDetailInfoView->isCatergoryPinDetails)
        mToggleHeaderBar->setVisible(true);
    mMapViewController->onSideItemVisibility(true);
    emit SignalCloseAllPopUpWindows();

}

void CategorySearchController::loadMoreSearchSDKRequest() {
    locationtoolkit::SearchRequest* request = locationtoolkit::SingleSearchRequest::createRequest(*GetLTKContext(),mSearchRequest, *mSearchInfo, locationtoolkit::SD_Next);

    connect(request, SIGNAL(OnSuccess(QSharedPointer<locationtoolkit::SearchInformation>)),
            this, SLOT(onSearchSuccess(QSharedPointer<locationtoolkit::SearchInformation>)));
    connect(request, SIGNAL(OnError(int)), this, SLOT(onSearchError(int)));
    connect(request, SIGNAL(OnProgressUpdated(int)), this, SLOT(onSearchProgressUpdated(int)));
    request->StartRequest();

    // show loading indicator
    mWaitDialog->setVisible(true);

}
void CategorySearchController::mainBarSearchSDKRequest(QVariant parameter,QString searchText) {
    qDebug() << "ListData : :: " << mLocationDataList.count();
    mIsNavigationStarted = false;
    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();

    if(!mLocationDataList.isEmpty()){
        qDebug()<<"data cleared";
        mLocationDataList.clear();
        //engine->rootContext()->setContextProperty("locationListModel", "");
    }

    mCategoryInfo = parameter;

    QJsonObject jsonObject = mCategoryInfo.value<QJsonObject>();

    // initilise settings object

    QSettings settings(LTKSampleUtil::GetResourceFolder() + gConfigFileName, QSettings::IniFormat);
    settings.beginGroup("AppSettings");
    //QSettings settings(gOrganization, gApplication);

    // get current values and update UI

    QString searchRadius = settings.value(gSearchRadius,"30000").toString();
    QString currentLatitude = settings.value(gCurrentLatitude,"33.558898").toString();
    QString currentLongitude = settings.value(gCurrentLongitude,"-117.728973").toString();
    qDebug()<<"currentLatitude: " << currentLatitude << ", currentLongitude: " << currentLongitude;

    QString gpsfile = settings.value(gGpsFileName, "demo.gps").toString();
    QString gpsPath = LTKSampleUtil::GetResourceFolder() + "gpsfiles/" + gpsfile;

    double lat = currentLatitude.toDouble();
    double lon = currentLongitude.toDouble();
    locationtoolkit::Coordinates userCoordinates(lat, lon);
    locationtoolkit::Coordinates centerCoordinates;

    if(mMapViewController != NULL) {

        Place destination, origin;

        Coordinates mapCenter = mMapViewController->getMapCenter();
        GetDefaultDestinationAndOrigin(gpsPath.toStdString(), &destination, &origin);

        qDebug()<<"Path : "<<gpsPath<<" des "<<destination.GetLocation().center.latitude<<destination.GetLocation().center.longitude<<" org "<<origin.GetLocation().center.latitude<<origin.GetLocation().center.longitude;
        centerCoordinates = mapCenter;
        userCoordinates = origin.GetLocation().center;

        qDebug()<<"CategorySearchController::mainbar Center : ("<<mapCenter.latitude<<","<<mapCenter.longitude<<") user : ("<<userCoordinates.latitude<<","<<userCoordinates.longitude<<")";
    }
    else {
        if (jsonObject["isNearByPinLocation"].toBool()) {
            centerCoordinates.latitude = jsonObject["latitude"].toDouble();
            centerCoordinates.longitude = jsonObject["longitude"].toDouble();
        }
        else {
            centerCoordinates.latitude = lat;
            centerCoordinates.longitude = lon;
        }
    }

    QString categoryCode = jsonObject["code"].toString();

    //  Set categoryLabel size
    if (jsonObject["name"].toString() == "Fuel Stations") {
        //        ui->categoryLabel->setFixedSize(this->width()*0.25, ui->topBarWidget->height() - 18);
    }
    else {
        //        ui->categoryLabel->setFixedSize(this->width()*0.75, ui->topBarWidget->height() - 18);
    }

    //    ui->categoryLabel->setText(jsonObject["name"].toString());



    engine->rootContext()->setContextProperty("categoryTitle", jsonObject["name"].toString());

    locationtoolkit::SearchRequest* request = locationtoolkit::SingleSearchRequest::createRequest(*GetLTKContext(), searchText, userCoordinates, centerCoordinates,false);
    connect(request, SIGNAL(OnSuccess(QSharedPointer<locationtoolkit::SearchInformation>)),
            this, SLOT(onSearchSuccess(QSharedPointer<locationtoolkit::SearchInformation>)));
    connect(request, SIGNAL(OnError(int)), this, SLOT(onSearchError(int)));
    connect(request, SIGNAL(OnProgressUpdated(int)), this, SLOT(onSearchProgressUpdated(int)));

    //  keep reference of base request to use later for creating load more request
    mSearchRequest = request;

    //  start request
    request->StartRequest();

    //  show loading indicator
    mWaitDialog->setVisible(true);
    settings.endGroup();

}
//void CategorySearchController::onSearchSuccess(QSharedPointer<locationtoolkit::SearchInformation> info)
//{
//    qDebug()<<"CategorySearchController::onSearchSuccess";
//    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();

//    emit SetListViewFocus();

//    // hide loading indicator
//    mWaitDialog->setVisible(false);
//    this->show();
//    emit ToggleTitleBarVisibility(false);
//    eSearchType = IN_SEARCH;
//    // keep reference of info to create load more request if required
//    mSearchInfo = info;

//    QJsonObject json;
//    bool result = locationtoolkit::DataTransformer::toJson(*info, json);
//    if (result) {

//        QJsonDocument doc(json);
//        QJsonObject obj;
//        QJsonArray jsonArray;

//        //mLocationDataList.clear();
//        qDebug() << "onSearchSuccess called";
//        //qDebug() <<"Getting JSON Results: \n";

//        // check validity of the document
//        if (!doc.isNull()) {

//            if (doc.isObject()) {
//                obj = doc.object();
//                QJsonObject fuelSummary = obj["fuel_summary"].toObject();

//                // if response includes fuel summary perform fuel price related operations to get lowest and average price
//                if (!fuelSummary.isEmpty()) {
//                    double fuelPriceLowest = fuelSummary["low"].toObject()["price"].toObject()["value"].toDouble();

//                    if (mPriceLowest.length() > 0) {
//                        double currentLowest = mPriceLowest.toDouble();

//                        if (fuelPriceLowest < currentLowest) {
//                            mPriceLowest = QString::number(fuelPriceLowest, 'f', 2);
//                        }
//                    }
//                    else {
//                        mPriceLowest = QString::number(fuelPriceLowest, 'f', 2);
//                    }

//                    double fuelPriceAverage = fuelSummary["average"].toObject()["price"].toObject()["value"].toDouble();

//                    if (mPriceAverage.length() > 0) {
//                        double average = (mPriceAverage.toDouble() + fuelPriceAverage)/2;
//                        mPriceAverage = QString::number(average, 'f', 2);
//                    }
//                    else {
//                        mPriceAverage = QString::number(fuelPriceAverage, 'f', 2);
//                    }

//                    QString fuelLowest_St = "Low:";
//                    //                    ui->lowestLabel->setText(fuelLowest_St);

//                    QString fuelLowest_Pr = "$"+mPriceLowest;
//                    //                    ui->lowestPrice->setText(fuelLowest_Pr);

//                    QString fuelAvg_St = "Avg.:";
//                    //                    ui->averageLabel->setText(fuelAvg_St);

//                    QString fuelAvg_Pr = "$"+mPriceAverage;
//                    //                    ui->averagePrice->setText(fuelAvg_Pr);

//                    qDebug() << "fuelLowest_Pr " << fuelLowest_Pr;
//                    qDebug() << "fuelAvg_Pr " << fuelAvg_Pr;

//                    engine->rootContext()->setContextProperty("lowestFuelPrice", "$" + mPriceLowest);
//                    engine->rootContext()->setContextProperty("AverageFuelPrice", "$" + mPriceAverage);
//                    //                    ui->fuelWidget->setHidden(false);
//                }
//                else {
//                    qDebug() << "No Fuel Info ";
//                    engine->rootContext()->setContextProperty("lowestFuelPrice", "");
//                    engine->rootContext()->setContextProperty("AverageFuelPrice", "");
//                    // hide fuel price widget if no fual information included in response
//                    //                    ui->fuelWidget->setHidden(true);
//                }

//                int count = 0;
//                jsonArray = obj["results"].toArray();

//                // Emit Signal to Add Locations to MapView in MainWindow
//                mLocationPins_10 = jsonArray;//emit categoryAddLocationPins(jsonArray);

//                //qDebug() << "jSON Array :" << jsonArray;

//                // prepare model to display in QML ListView

//                // iterate through all locations and add item widget in list view for each locations
//                foreach (const QJsonValue & value, jsonArray) {
//                    ++count;
//                    QJsonObject obj1 = value.toObject();

//                    //qDebug()<<"------------- START Result "<<count<<" -------------";

//                    QJsonObject json_place = obj1["place"].toObject();
//                    QString placeName = json_place["name"].toString();
//                    //qDebug() << "Name is:"<< placeName;

//                    QJsonObject json_location = json_place["location"].toObject();
//                    QString address = json_location["compact_address"].toObject()["formatted_line"].toString();
//                    int firstComma = address.indexOf(',');
//                    int length = address.length();

//                    QString addressLine1 = address.left(firstComma + 1);
//                    firstComma = firstComma == -1 ? -2 : firstComma;
//                    QString addressLine2 = address.right(length - firstComma - 2);

//                    QString latitude = QString::number(json_location["latitude"].toDouble(), 'f', 6);
//                    QString longitude = QString::number(json_location["longitude"].toDouble(), 'f', 6);

//                    qDebug() <<"Latitude is: "<< latitude;
//                    qDebug() <<"Longitude is: "<< longitude;

//                    QJsonObject json_phone = obj1["custom_phone"].toObject();
//                    QString contact = "+" + json_phone["country"].toString()+
//                            "("+json_phone["area"].toString()+")"+
//                            json_phone["number"].toString().left(3)+"-"+
//                            json_phone["number"].toString().right(4);

//                    if(contact.length() == 4) {
//                        QJsonArray json_phone = json_place["phones"].toArray();
//                        contact = "+" + json_phone[0].toObject()["country"].toString()+
//                                "("+json_phone[0].toObject()["area"].toString()+")"+
//                                json_phone[0].toObject()["number"].toString().left(3)+"-"+
//                                json_phone[0].toObject()["number"].toString().right(4);
//                    }

//                    //  Conditioning to show Not Available instead of a blank contact number
//                    if(contact.length() < 5) {
//                        contact = ""; //Not Available
//                    }

//                    /*QJsonArray json_phone = json_place["phones"].toArray();
//                    QString contactNum = "+" + json_phone[0].toObject()["country"].toString()+
//                                "("+json_phone[0].toObject()["area"].toString()+")"+
//                                json_phone[0].toObject()["number"].toString().left(3)+"-"+
//                                json_phone[0].toObject()["number"].toString().right(4);
//                    qDebug() <<"Contact is: "<<contactNum;*/


//                    //  Get Category
//                    QStringList categoryStringArray;
//                    QJsonArray categoryArray = json_place["categories"].toArray();

//                    foreach (const QJsonValue & value, categoryArray) {
//                        if (!value.toArray().isEmpty()) {
//                            categoryStringArray.append(value.toArray()[0].toString());
//                        }
//                    }
//                    //  set place icon
//                    QString categoryImagePath;
//                    if (!categoryStringArray.isEmpty()) {
//                        foreach (QString categoryString, categoryStringArray) {
//                            QString filePath = mAppDirPath + QString("/resource/images/Detailscreen_images/category_icons/category_icons_512/%1.png").arg(categoryString);

//                            qDebug()<<"@@@@@@ category image path "<<filePath;

//                            QFileInfo checkFile(filePath);

//                            // check if file exists and if yes: Is it really a file and no directory?
//                            if (checkFile.exists() && checkFile.isFile()) {
//                                categoryImagePath = QString("%1.png").arg(categoryString);
//                                //categoryImagePath = filePath;
//                                break;
//                            }
//                        }
//                    }
//                    else {
//                        categoryImagePath = "AAB.png";
//                    }

//                    //categoryImagePath = QGuiApplication::applicationDirPath() + QString("/resource/images/Detailscreen_images/category_icons/category_icons_512/%1.png").arg("ABJ");
//                    //categoryImagePath = QString("/resource/images/Detailscreen_images/category_icons/category_icons_512/%1.png").arg("ABJ");
//                    //qDebug() << "categoryImagePath" << categoryImagePath;
//                    //categoryImagePath = "ABJ.png";
//                    //qDebug() << "categoryImagePath" << categoryImagePath;

//                    //  Get Review Count and trip advisor rating information
//                    QJsonObject json_content = obj1["content"].toObject();
//                    QJsonArray json_vendors = json_content["vendors"].toArray();
//                    QVariant rating = json_vendors[0].toObject()["average_rating"].toDouble();
//                    QVariant reviewCount = json_vendors[0].toObject()["rating_count"].toDouble();

//                    //  Get Distance
//                    //QString distance = QString::number(obj1["distance"].toDouble()*0.00062137, 'f', 2);
//                    QString distance = QString("%1 mi").arg(QString::number(obj1["distance"].toDouble()*0.00062137, 'f', 2 ));
//                    QString description = obj1["enhanced_contents"].toObject()["description"].toString();


//                    QJsonArray timing = json_content["attributes"].toArray();
//                    QString timingInfo;
//                    //qDebug() << timing;
//                    //  Get timing information
//                    foreach (const QJsonValue & value, timing) {
//                        QJsonObject obj = value.toObject();
//                        //qDebug() << obj["value"].toString();
//                        if(obj["name"].toString() == "hours_of_operation") {
//                            timingInfo = obj["value"].toString();
//                            break;
//                        }
//                    }

//                    QString fuelPriceSt;
//                    QString price;

//                    if (obj["type"].toString() == "fuel") {
//                        QJsonArray fuelDetails = obj1["fuelDetails"].toArray();

//                        if (fuelDetails.count() > 0) {

//                            foreach (const QJsonValue &value, fuelDetails) {

//                                QString fuelType = value.toObject()["type"].toObject()["type"].toString();

//                                // get current setting value
//                                QSettings settings(LTKSampleUtil::GetResourceFolder() + gConfigFileName, QSettings::IniFormat);
//                                settings.beginGroup("AppSettings");
//                                //QSettings settings(gOrganization, gApplication);
//                                QString currentFuelType = settings.value(gFuelType).toString();

//                                if (fuelType == currentFuelType) {
//                                    double fuelPrice = value.toObject()["price"].toObject()["value"].toDouble();
//                                    price = QString::number(fuelPrice, 'f', 2);
//                                    fuelPriceSt = "$" + price;
//                                }
//                                settings.endGroup();
//                            }

//                        }
//                    }

//                    bool isLowestPrice = checkLowestPrice(price);
//                    bool isFav = isFavoriteExist(latitude.toDouble(),longitude.toDouble());
//                    qDebug()<<"isFavoriteExist :: "<<isFav<<latitude.toDouble()<<longitude.toDouble();
//                    // Add Search Result Row to List View Widget
//                    //                    SearchItemWidget* searchWidget = new SearchItemWidget();
//                    //                    searchWidget->setGeometry(0,0,mSearchWidgetItemWidth,mSearchWidgetItemHeight);
//                    //                    bool isLowestPrice = checkLowestPrice(price);
//                    //                    searchWidget->setSearchLabels(placeName, address, distance,"", isLowestPrice, fuelPriceSt);
//                    //                    searchWidget->setStartNavIcon(true);
//                    //                    connect(searchWidget, SIGNAL(startNavigationClicked(QListWidgetItem*)),this, SLOT(onStartNavigationClicked(QListWidgetItem*)) );

//                    //                    if (!json_vendors[0].toObject()["average_rating"].isNull()) {
//                    //                        searchWidget->setTripAdvisorLayout(true, rating, reviewCount);
//                    //                    }
//                    //                    else {
//                    //                        searchWidget->setTripAdvisorLayout(false, rating, reviewCount);
//                    //                    }

//                    //                    QListWidgetItem *item = new QListWidgetItem();
//                    //                    item->setSizeHint(searchWidget->size());
//                    //                    obj1["rowType"] = "SearchResultRow";
//                    //                    QVariant variant = obj1;
//                    //                    item->setData(Qt::UserRole,variant);
//                    //                    searchWidget->widgetItem = item;

//                    //                    ui->listWidget->addItem(item);
//                    //                    ui->listWidget->setItemWidget(item,searchWidget);

//                    //qDebug()<<"------------- END Result "<<count<<" -------------\n\n";

//                    // add to mLocationDataList
//                    mLocationDataList.append(new DataObject(
//                                                 placeName,
//                                                 address,
//                                                 addressLine1,
//                                                 addressLine2,
//                                                 contact,
//                                                 rating.toDouble(),
//                                                 reviewCount.toDouble(),
//                                                 distance,
//                                                 description,
//                                                 timingInfo,
//                                                 fuelPriceSt,
//                                                 categoryStringArray,
//                                                 categoryImagePath,
//                                                 latitude,
//                                                 longitude,
//                                                 isLowestPrice,
//                                                 fuelPriceSt,
//                                                 isFav));
//                }

//                engine->rootContext()->setContextProperty("locationListModel", QVariant::fromValue(mLocationDataList));

//                bool hasMoreResults = obj["has_more"].toBool();
//                qDebug()<<"hasMoreResults"<<hasMoreResults;
//                if (hasMoreResults) {
//                    engine->rootContext()->setContextProperty("hasMoreResults", hasMoreResults);
//                    engine->rootContext()->setContextProperty("ErrorMessage", "" );
//                    // mLocationDataList.append(new DataObject("Load more data", "", "","","",false,false, "", 0, false));

//                    // Append Load More Row to List View Widget
//                    //                    SearchItemWidget *searchWidget = new SearchItemWidget();
//                    //                    searchWidget->setGeometry(0,0,mSearchWidgetItemWidth,mSearchWidgetItemHeight);

//                    //                    searchWidget->setLoadMoreItem("Load more results...");
//                    //                    QListWidgetItem *item = new QListWidgetItem();
//                    //                    item->setSizeHint(searchWidget->size());
//                    //                    QJsonObject jsonObject;
//                    //                    jsonObject["rowType"] = "LoadMoreRow";
//                    //                    QVariant variant = jsonObject;
//                    //                    item->setData(Qt::UserRole,variant);
//                    //                    ui->listWidget->addItem(item);
//                    //                    ui->listWidget->setItemWidget(item,searchWidget);
//                }
//                else{
//                    engine->rootContext()->setContextProperty("hasMoreResults", false);
//                    engine->rootContext()->setContextProperty("ErrorMessage", "" );
//                }

//            }
//        }
//    }
//    else {
//        //        QJsonObject button1;
//        //        button1["buttonTitle"] = "OK";
//        //        button1["context"] = "closePopUp";
//        //        QVariant button1Info = button1;
//        //        mMessageBox->setUpUI("Failed to convert search information to JSON object","Search Error",button1Info,0);

//        //        if (mMessageBox->isHidden()) {
//        //            mMessageBox->show();
//        //        }

//    }
//    searchedLocations = mLocationPins_10;
//}

void CategorySearchController::onSearchSuccess(QSharedPointer<locationtoolkit::SearchInformation> info)
{
    qDebug()<<"CategorySearchController::onSearchSuccess";
    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();

    emit SetListViewFocus();

    // hide loading indicator
    mWaitDialog->setVisible(false);
    this->show();
    mMapViewController->onSideItemVisibility(false);
    emit ToggleTitleBarVisibility(false);
    eSearchType = IN_SEARCH;
    // keep reference of info to create load more request if required
    mSearchInfo = info;
    if(mSearchedJsonObjectVector.length()>0)
        mSearchedJsonObjectVector.clear();
    if(mLocationDataList.length()>0)
        mLocationDataList.clear();

    QJsonObject json;
    bool result = locationtoolkit::DataTransformer::toJson(*info, json);
    if (result == false)
    {
        return;
    }

    QJsonDocument doc(json);
    QJsonObject obj;
    QJsonArray jsonArray;
    qDebug() << "onSearchSuccess called";

    // check validity of the document
    if (!doc.isNull() && doc.isObject()) {

        obj = doc.object();
        QJsonObject fuelSummary = obj["fuel_summary"].toObject();

        // if response includes fuel summary perform fuel price related operations to get lowest and average price
        if (!fuelSummary.isEmpty()) {
            double fuelPriceLowest = fuelSummary["low"].toObject()["price"].toObject()["value"].toDouble();

            if (mPriceLowest.length() > 0) {
                double currentLowest = mPriceLowest.toDouble();

                if (fuelPriceLowest < currentLowest) {
                    mPriceLowest = QString::number(fuelPriceLowest, 'f', 2);
                }
            }
            else {
                mPriceLowest = QString::number(fuelPriceLowest, 'f', 2);
            }

            double fuelPriceAverage = fuelSummary["average"].toObject()["price"].toObject()["value"].toDouble();

            if (mPriceAverage.length() > 0) {
                double average = (mPriceAverage.toDouble() + fuelPriceAverage)/2;
                mPriceAverage = QString::number(average, 'f', 2);
            }
            else {
                mPriceAverage = QString::number(fuelPriceAverage, 'f', 2);
            }

            QString fuelLowest_Pr = "$"+mPriceLowest;

            QString fuelAvg_Pr = "$"+mPriceAverage;

            qDebug() << "fuelLowest_Pr " << fuelLowest_Pr;
            qDebug() << "fuelAvg_Pr " << fuelAvg_Pr;

            engine->rootContext()->setContextProperty("lowestFuelPrice", "$" + mPriceLowest);
            engine->rootContext()->setContextProperty("AverageFuelPrice", "$" + mPriceAverage);
        }
        else {
            qDebug() << "No Fuel Info ";
            engine->rootContext()->setContextProperty("lowestFuelPrice", "");
            engine->rootContext()->setContextProperty("AverageFuelPrice", "");

        }

        int count = 0;
        jsonArray = obj["results"].toArray();

        // Emit Signal to Add Locations to MapView in MainWindow
        //mLocationPins_10 = jsonArray;//emit categoryAddLocationPins(jsonArray);

        // prepare model to display in QML ListView
        // iterate through all locations and add item widget in list view for each locations
        foreach (const QJsonValue & value, jsonArray) {
            ++count;
            QJsonObject obj1 = value.toObject();
            mSearchedJsonObjectVector.append(obj1);
            mLocationDataList.append(getDataObject(obj1,obj["type"].toString()));
        }

        engine->rootContext()->setContextProperty("locationListModel", QVariant::fromValue(mLocationDataList));
        engine->rootContext()->setContextProperty("setNoResultBoxText","No results found!");

        bool hasMoreResults = obj["has_more"].toBool();
        qDebug()<<"hasMoreResults"<<hasMoreResults;
        if (hasMoreResults) {
            engine->rootContext()->setContextProperty("hasMoreResults", hasMoreResults);
            engine->rootContext()->setContextProperty("ErrorMessage", "" );
        }
        else{
            engine->rootContext()->setContextProperty("hasMoreResults", false);
            engine->rootContext()->setContextProperty("ErrorMessage", "" );
        }
    }
}



DataObject *CategorySearchController::getDataObject(QJsonObject pJsonobj, QString type)
{
    //qDebug()<<"------------- START Result "<<count<<" -------------";

    QJsonObject json_place = pJsonobj["place"].toObject();
    QString placeName = json_place["name"].toString();
    //qDebug() << "Name is:"<< placeName;

    QJsonObject json_location = json_place["location"].toObject();
    QString address = json_location["compact_address"].toObject()["formatted_line"].toString();
    int firstComma = address.indexOf(',');
    int length = address.length();

    QString addressLine1 = address.left(firstComma + 1);
    firstComma = firstComma == -1 ? -2 : firstComma;
    QString addressLine2 = address.right(length - firstComma - 2);

    QString latitude = QString::number(json_location["latitude"].toDouble(), 'f', 6);
    QString longitude = QString::number(json_location["longitude"].toDouble(), 'f', 6);

    qDebug() <<"Latitude is: "<< latitude;
    qDebug() <<"Longitude is: "<< longitude;

    QJsonObject json_phone = pJsonobj["custom_phone"].toObject();
    QString contact = "+" + json_phone["country"].toString()+
            "("+json_phone["area"].toString()+")"+
            json_phone["number"].toString().left(3)+"-"+
            json_phone["number"].toString().right(4);

    if(contact.length() == 4) {
        QJsonArray json_phone = json_place["phones"].toArray();
        contact = "+" + json_phone[0].toObject()["country"].toString()+
                "("+json_phone[0].toObject()["area"].toString()+")"+
                json_phone[0].toObject()["number"].toString().left(3)+"-"+
                json_phone[0].toObject()["number"].toString().right(4);
    }

    //  Conditioning to show Not Available instead of a blank contact number
    if(contact.length() < 5) {
        contact = ""; //Not Available
    }

    //  Get Category
    QStringList categoryStringArray;
    QJsonArray categoryArray = json_place["categories"].toArray();

    foreach (const QJsonValue & value, categoryArray) {
        if (!value.toArray().isEmpty()) {
            categoryStringArray.append(value.toArray()[0].toString());
        }
    }
    //  set place icon
    QString categoryImagePath;
    if (!categoryStringArray.isEmpty()) {
        foreach (QString categoryString, categoryStringArray) {
            QString filePath = mAppDirPath + QString("/resource/images/Detailscreen_images/category_icons/category_icons_512/%1.png").arg(categoryString);

            qDebug()<<"@@@@@@ category image path "<<filePath;

            QFileInfo checkFile(filePath);

            // check if file exists and if yes: Is it really a file and no directory?
            if (checkFile.exists() && checkFile.isFile()) {
                categoryImagePath = QString("%1.png").arg(categoryString);
                //categoryImagePath = filePath;
                break;
            }
        }
    }
    else {
        categoryImagePath = "AAB.png";
    }

    //  Get Review Count and trip advisor rating information
    QJsonObject json_content = pJsonobj["content"].toObject();
    QJsonArray json_vendors = json_content["vendors"].toArray();
    QVariant rating = json_vendors[0].toObject()["average_rating"].toDouble();
    QVariant reviewCount = json_vendors[0].toObject()["rating_count"].toDouble();

    //  Get Distance
    //QString distance = QString::number(obj1["distance"].toDouble()*0.00062137, 'f', 2);
    // QString distance = QString("%1 mi").arg(QString::number(pJsonobj["distance"].toDouble()*0.00062137, 'f', 2 ));
    QString distance = LTKSampleUtil::FormatDistance(pJsonobj["distance"].toDouble());
    QString description = pJsonobj["enhanced_contents"].toObject()["description"].toString();

    QJsonArray timing = json_content["attributes"].toArray();
    QString timingInfo;

    //  Get timing information
    foreach (const QJsonValue & value, timing) {
        QJsonObject obj = value.toObject();
        //qDebug() << obj["value"].toString();
        if(obj["name"].toString() == "hours_of_operation") {
            timingInfo = obj["value"].toString();
            break;
        }
    }

    QString fuelPriceSt;
    QString price;

    if (type == "fuel") {
        QJsonArray fuelDetails = pJsonobj["fuelDetails"].toArray();

        if (fuelDetails.count() > 0) {

            foreach (const QJsonValue &value, fuelDetails) {

                QString fuelType = value.toObject()["type"].toObject()["type"].toString();

                // get current setting value
                QSettings settings(LTKSampleUtil::GetResourceFolder() + gConfigFileName, QSettings::IniFormat);
                settings.beginGroup("AppSettings");
                //QSettings settings(gOrganization, gApplication);
                QString currentFuelType = settings.value(gFuelType).toString();

                if (fuelType == currentFuelType) {
                    double fuelPrice = value.toObject()["price"].toObject()["value"].toDouble();
                    price = QString::number(fuelPrice, 'f', 2);
                    fuelPriceSt = "$" + price;
                }
                settings.endGroup();
            }

        }
    }

    bool isLowestPrice = checkLowestPrice(price);
    bool isFav = isFavoriteExist(latitude.toDouble(),longitude.toDouble());
    qDebug()<<"isFavoriteExist :: "<<isFav<<latitude.toDouble()<<longitude.toDouble();

    // add to mLocationDataList
    return new DataObject(
                placeName,
                address,
                addressLine1,
                addressLine2,
                contact,
                rating.toDouble(),
                reviewCount.toDouble(),
                distance,
                description,
                timingInfo,
                fuelPriceSt,
                categoryStringArray,
                categoryImagePath,
                latitude,
                longitude,
                isLowestPrice,
                fuelPriceSt,
                isFav);
}

void CategorySearchController::onSearchError(int errorCode) {

    qDebug()<<"CategorySearchController::onSearchError";
    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();

    mWaitDialog->setVisible(false);

    // prepare model to display in QML ListView
    QList<QObject*> mLocationDataList;

    QStringList s;
    s.append("");

    //mLocationDataList.append(new DataObject("Error","","", false, "", 0, false));
    mLocationDataList.append(new DataObject("Error","","","","",0,0,"","","","",s,"","","",false,0, false));
    engine->rootContext()->setContextProperty("ErrorMessage", "Cannot perform the function.");

    engine->rootContext()->setContextProperty("hasMoreResults", false);
    engine->rootContext()->setContextProperty("lowestFuelPrice", "");
    engine->rootContext()->setContextProperty("AverageFuelPrice", "");
    //QQmlContext *ctxt = this->rootContext();
    //qDebug() << "mLocationDataList Model:"<< QVariant::fromValue(mLocationDataList);

    engine->rootContext()->setContextProperty("locationListModel", QVariant::fromValue(mLocationDataList));

    QString errorString1,errorString2;
    locationtoolkit::LTKError errorObject = locationtoolkit::LTKError(errorCode);

    //    // if error code has description it will show otherwise only error code will be displayed in alert message
    QString error = errorObject.description();
    if (error.isEmpty()) {
        errorString1 = QString("Error %1").arg(errorCode);
        //errorString2 = QString("Cannot perform the function. Please try again");
        errorString2 = QString("Please try again");
    }
    else {
        errorString1 = QString("Error returned from search:");
        errorString2 = QString( "%1").arg(error);
    }

    //    QJsonObject button1;
    //    button1["buttonTitle"] = "OK";
    //    button1["context"] = "closePopUp";
    //    QVariant button1Info = button1;
    //    mMessageBox->setUpUI(errorString,"",button1Info,0);

    //    if (mMessageBox->isHidden()) {
    //        mMessageBox->show();
    //    }
    mMessageBox = new Messageboxcontroller();
    QQmlEngine::setObjectOwnership(mMessageBox, QQmlEngine::CppOwnership);
    mMessageBox->setParentItem(this->parentItem());
    QQmlEngine::setContextForObject(mMessageBox, QQmlEngine::contextForObject(this));
    mMessageBox->setWidth(width());
    mMessageBox->setHeight(height());
    mMessageBox->messageStrings(errorString1,errorString2);
    mMessageBox->setUp();
    connect( mMessageBox, SIGNAL(okClicked()), this, SLOT(onOkButtonClicked()) );

}
void CategorySearchController::onOkButtonClicked(){

    mMessageBox->setVisible(false);
    //    delete mMessageBoxView;
    //    mMessageBoxView = NULL;

}
void CategorySearchController::onSearchProgressUpdated(int percentage) {

    //qDebug()<<"onSearchProgressUpdated:"<<percentage;

}
void CategorySearchController::showSingleListItemWidget(QString textTitle,QString textMessage) {
    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();

    mLocationDataList.clear();
    engine->rootContext()->setContextProperty("categoryTitle", textTitle);
    engine->rootContext()->setContextProperty("setNoResultBoxText",textMessage);
    //    mLocationDataList.append(new DataObject("Error","","","","",0,0,"","","","",{},"","","",false,0, false));
    //    engine->rootContext()->setContextProperty("ErrorMessage", textMessage );
    //    engine->rootContext()->setContextProperty("hasMoreResults", false);
    //    engine->rootContext()->setContextProperty("lowestFuelPrice", "");
    //    engine->rootContext()->setContextProperty("AverageFuelPrice", "");

    // qDebug() << "mLocationDataList Model:"<< QVariant::fromValue(mLocationDataList);

    engine->rootContext()->setContextProperty("locationListModel", QVariant::fromValue(mLocationDataList));
}

void CategorySearchController::onClearAllResentsClicked() {
    //  Re Use: Clear the list items to free resources used earlier
    // mCategorySearchController->cleanUp();
    mConfirmBox = new ConfirmBoxController();
    QQmlEngine::setObjectOwnership(mConfirmBox, QQmlEngine::CppOwnership);
    mConfirmBox->setParentItem(mSearchView);
    QQmlEngine::setContextForObject(mConfirmBox, QQmlEngine::contextForObject(this));
    mConfirmBox->setWidth(width());
    mConfirmBox->setHeight(height());
    QString var1,var2;
    var1="Delete all your";
    var2="recents";
    mConfirmBox->messageStrings(var1,var2);
    mConfirmBox->setUp();
    connect( mConfirmBox, SIGNAL(confirmBoxNotification(bool)), this, SLOT(onConfirmRecentsValueClicked(bool)) );
}
bool CategorySearchController::onConfirmRecentsValueClicked(bool flag){

    qDebug()<<"confirm box value"<<flag;
    //    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
    //    engine->rootContext()->setContextProperty("messageText1","Delete all your");
    //    engine->rootContext()->setContextProperty("messageText2", "recents");

    if(flag){
        mConfirmBox->setVisible(false);
        //mDetailInfoView->resetRecents();
        PlaceManager* placeManager = PlaceManager::getInstance();
        placeManager->clearAllRecents();
        this->showSingleListItemWidget("Recents","No recents to show!");
    }
    else{
        mConfirmBox->setVisible(false);
    }
    delete mConfirmBox;
    mConfirmBox=NULL;
}

void CategorySearchController::onClearAllFavouritesClicked() {
    //  Re Use: Clear the list items to free resources used earlier
    //mCategorySearchController->cleanUp();
    mConfirmBox = new ConfirmBoxController();
    QQmlEngine::setObjectOwnership(mConfirmBox, QQmlEngine::CppOwnership);
    mConfirmBox->setParentItem(mSearchView);
    QQmlEngine::setContextForObject(mConfirmBox, QQmlEngine::contextForObject(this));
    mConfirmBox->setWidth(width());
    mConfirmBox->setHeight(height());
    QString var1,var2;
    var1="Delete all your";
    var2="favorites";
    mConfirmBox->messageStrings(var1,var2);

    mConfirmBox->setUp();
    connect( mConfirmBox, SIGNAL(confirmBoxNotification(bool)), this, SLOT(onConfirmFavouritesValueClicked(bool)) );
}

bool CategorySearchController::onConfirmFavouritesValueClicked(bool flag){
    qDebug()<<"confirm box value"<<flag;
    // QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
    // engine->rootContext()->setContextProperty("messageText1","Delete all your");
    // engine->rootContext()->setContextProperty("messageText2", "favorites");
    if(flag){
        mConfirmBox->setVisible(false);
        //mDetailInfoView->resetFavorites();

        PlaceManager* placeManager = PlaceManager::getInstance();
        placeManager->clearAllFavorites();
        this->showSingleListItemWidget("Favorites","No favorites to show!");
    }
    else{
        mConfirmBox->setVisible(false);
    }
    delete mConfirmBox;
    mConfirmBox=NULL;
}

void CategorySearchController::onDeleteSelectiveFav(QString latitude,QString longitude){
    //mCategorySearchController->cleanUp();
    mConfirmBox = new ConfirmBoxController();
    QQmlEngine::setObjectOwnership(mConfirmBox, QQmlEngine::CppOwnership);
    mConfirmBox->setParentItem(mSearchView);
    QQmlEngine::setContextForObject(mConfirmBox, QQmlEngine::contextForObject(this));
    mConfirmBox->setWidth(width());
    mConfirmBox->setHeight(height());
    QString var1,var2;
    var1="Delete all selected";
    var2="favorites";
    mConfirmBox->messageStrings(var1,var2);

    mConfirmBox->setUp();
    mTempLatitude = latitude;
    mTempLongitude = longitude;
    connect( mConfirmBox, SIGNAL(confirmBoxNotification(bool)), this, SLOT(onConfirmSelectedFavouriteValueClicked(bool)) );
}

bool CategorySearchController::onConfirmSelectedFavouriteValueClicked(bool flag){
    qDebug()<<"confirm box value"<<flag;
    //QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
    //engine->rootContext()->setContextProperty("messageText1","Delete the selected");
    // engine->rootContext()->setContextProperty("messageText2", "favorite");
    if(flag){
        mConfirmBox->setVisible(false);
        PlaceManager* placeManager = PlaceManager::getInstance();
        placeManager->deleteExistingFavConfig(mTempLatitude.toDouble(),mTempLongitude.toDouble());
        mTempLatitude = "";
        mTempLongitude = "";
        //        onFavouritesButtonClicked();
        displayExistingFavourites(false);
    }
    else{
        mConfirmBox->setVisible(false);
        mTempLatitude = "";
        mTempLongitude = "";
    }
    delete mConfirmBox;
    mConfirmBox=NULL;
}
void CategorySearchController::on_mapToggleButton_clicked() {

    qDebug()<<"CategorySearchController::on_mapToggleButton_clicked";
    //emit categoryAddLocationPins(mLocationPins_10);
    mMapViewController->onSideItemVisibility(true);
    mLTKMapKit->DeleteAllPins();
    foreach (const QJsonObject value, mSearchedJsonObjectVector) {
        OnAddLocationPins(value);
    }

    // adjust map to all locations bounding box
    if(!mSearchedJsonObjectVector.isEmpty())
    {
        adjustMapBoundingBox();
    }

}
void CategorySearchController::OnAddLocationPins(QJsonObject jsonObject) {

    onToggleMapViewFromCategory();

    //  Clear all pins from the map


    QJsonObject json_location = jsonObject["place"].toObject()["location"].toObject();
    QString pinLat = QString::number(json_location["latitude"].toDouble(), 'f', 6);
    QString pinLon = QString::number(json_location["longitude"].toDouble(), 'f', 6);

    bool ok = false;
    //qDebug() << "searched location lat: " << pinLat.toDouble(&ok);
    //qDebug() << "searched location long: " << pinLon.toDouble(&ok);

    //  Get lat and long for one search item
    locationtoolkit::Coordinates coordinate;
    coordinate.latitude = pinLat.toDouble(&ok);
    coordinate.longitude = pinLon.toDouble(&ok);

    if(!mIsNavigationStarted) {
        QPixmap *selectImage, *unselectImage;
        QString resourceFolder = LTKSampleUtil::GetResourceFolder();

        //  Setup pin selected/unselected images
        //            selectImage = new QPixmap(resourceFolder + "/resources/images/gps_location.png");
        //            unselectImage = new QPixmap("/home/tcs/Downloads/pin_start_jpeg.jpg");

        selectImage = new QPixmap(resourceFolder + "images/pin_select.png");
        unselectImage = new QPixmap("images/pin_unselect.png");

        //  TODO: Remove in final publish - Temporary debugging
        QByteArray byteBuff;
        QBuffer buffer(&byteBuff);
        buffer.open(QBuffer::WriteOnly);
        bool saved = selectImage->save(&buffer, "JPEG");
        //qDebug()<<"saved pixel, saved:"<<saved <<" length="<<byteBuff.size();

        locationtoolkit::PinImageInfo selectedImage;
        selectedImage.SetPixmap(*selectImage);
        selectedImage.SetPinAnchor(50, 100);
        locationtoolkit::PinImageInfo unSelectedImage;
        unSelectedImage.SetPixmap(*unselectImage);
        unSelectedImage.SetPinAnchor(50, 100);
        locationtoolkit::RadiusParameters radiusPara(50, 0x6721D826);

        bool visible = true;
        locationtoolkit::Bubble* bubble = new CustomBubble();
        locationtoolkit::PinParameters pinpara(coordinate,
                                               selectedImage,
                                               unSelectedImage,
                                               radiusPara,
                                               "",
                                               "",
                                               bubble,
                                               visible);

        pinpara.SetSelectedImage(&selectedImage);
        pinpara.SetUnselectedImage(&unSelectedImage);

        //  Create a Pin with the specifics
        Pin* searchPin = mLTKMapKit->CreatePin(pinpara);
    }

}

//  InOrder to display all the created pin onto the map view
//  Greater the pins higher the zoom level i.e. zoom out - Used in Map/List toggle view
void CategorySearchController::adjustMapBoundingBox() {

    QJsonObject firstObject = mSearchedJsonObjectVector.at(0);//searchedLocations[0].toObject();
    QJsonObject firstLocation = firstObject["place"].toObject()["location"].toObject();

    // set initial location
    mMinPoiPinLat = mMaxPoiPinLat = firstLocation["latitude"].toDouble();
    mMinPoiPinLon = mMaxPoiPinLon = firstLocation["longitude"].toDouble();;

    foreach(const QJsonObject jsonObject, mSearchedJsonObjectVector) {
        QJsonObject json_location = jsonObject["place"].toObject()["location"].toObject();

        double location_lat= json_location["latitude"].toDouble();
        double location_lon = json_location["longitude"].toDouble();

        mMinPoiPinLat = location_lat < mMinPoiPinLat ? location_lat : mMinPoiPinLat;
        mMaxPoiPinLat = location_lat > mMaxPoiPinLat ? location_lat : mMaxPoiPinLat;
        mMinPoiPinLon = location_lon < mMinPoiPinLon ? location_lon : mMinPoiPinLon;
        mMaxPoiPinLon = location_lon > mMinPoiPinLon ? location_lon : mMaxPoiPinLon;
    }
    if(mSearchedJsonObjectVector.size() == 1)
    {
        mMinPoiPinLat = mMapViewController->getMapCenter().getLatitude();
        mMinPoiPinLon = mMapViewController->getMapCenter().getLongitude();
    }
    if( mMinPoiPinLat < 90.0 ) {
        mLTKMapKit->ZoomToBoundingBox( mMinPoiPinLat, mMinPoiPinLon, mMaxPoiPinLat, mMaxPoiPinLon);
    }

}
void CategorySearchController::onToggleMapViewFromCategory() {

    //  Set the current list widget to Category search controller
    // mCurrentListWidget = mCategorySearchController;
    //  explicitly close all the pop up windows
    //closeAllPopUpWindows();
    //  show top header bar
    //QQmlEngine *engine = QQmlEngine::contextForObject(mSearchView)->engine();
    QObject *headerObject = mSearchView->findChild<QObject*>("headerObject");

    QObject *mapButton = mToggleHeaderBar->findChild<QObject*>("placeObject");
    if (mapButton) {
        mapButton->setProperty("text", headerObject->property("placeName").toString());
    }


    this->hide();
    mToggleHeaderBar->setVisible(true);
    mDetailInfoView->hide();
    mDetailInfoView->isCatergoryPinDetails = true;

}
void CategorySearchController::displayExistingFavourites(bool showDeleteRed) {
    if(!mLocationDataList.isEmpty()){
        qDebug()<<"data cleared";
        mLocationDataList.clear();
    }
    if(!mSearchedJsonObjectVector.isEmpty()){
        mSearchedJsonObjectVector.clear();
    }
    mIsNavigationStarted = false;
    //  hide loading indicator
    mWaitDialog->setVisible(false);
    mMapViewController->onSideItemVisibility(false);

    this->show();
    eSearchType = IN_FAV;
    emit SetListViewFocus();

    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();

    engine->rootContext()->setContextProperty("categoryTitle", "Favorites");
    engine->rootContext()->setContextProperty("hasMoreResults", false);
    engine->rootContext()->setContextProperty("ErrorMessage", "" );
    engine->rootContext()->setContextProperty("lowestFuelPrice", "");
    engine->rootContext()->setContextProperty("AverageFuelPrice", "");

    PlaceManager* placeManager = PlaceManager::getInstance();
    qDebug()<<"######## before setAppDirPath"<<mAppDirPath;
    placeManager->setAppDirPath(mAppDirPath);
    QVector<QJsonObject> favoriteObjects = placeManager->getFavorites();

    qDebug()<<"##################### Favorites count : "<<favoriteObjects.size();

    for (int i = 0; i < favoriteObjects.size(); i++) {

        // Get JSON object
        mSearchedJsonObjectVector.append(favoriteObjects.at(i));
        mLocationDataList.append(getDataObject(favoriteObjects.at(i)));
    }

    engine->rootContext()->setContextProperty("locationListModel", QVariant::fromValue(mLocationDataList));
    engine->rootContext()->setContextProperty("setNoResultBoxText","No favorites to show!");

}
void CategorySearchController::displayExistingRecents(bool showDeleteRed) {
    if(!mLocationDataList.isEmpty()){
        qDebug()<<"data cleared";
        mLocationDataList.clear();
    }
    if(!mSearchedJsonObjectVector.isEmpty()){
        mSearchedJsonObjectVector.clear();
    }
    mIsNavigationStarted = false;
    mWaitDialog->setVisible(false);
    this->show();
    mMapViewController->onSideItemVisibility(false);

    eSearchType = IN_RECENTS;
    emit SetListViewFocus();

    qDebug()<<"#################### CategorySearchController::displayExistingRecents";

    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();

    engine->rootContext()->setContextProperty("categoryTitle", "Recents");
    engine->rootContext()->setContextProperty("hasMoreResults", false);
    engine->rootContext()->setContextProperty("ErrorMessage", "" );
    engine->rootContext()->setContextProperty("lowestFuelPrice", "");
    engine->rootContext()->setContextProperty("AverageFuelPrice", "");

    PlaceManager* placeManager = PlaceManager::getInstance();
    qDebug()<<"######## before setAppDirPath"<<mAppDirPath;
    placeManager->setAppDirPath(mAppDirPath);
    QVector<QJsonObject> recentObjects = placeManager->getRecents();

    qDebug()<<"##################### Recents count : "<<recentObjects.size();

    for (int i = 0; i <recentObjects.size() ; i++) {

        // Get JSON object
        mSearchedJsonObjectVector.append(recentObjects.at(i));
        mLocationDataList.append(getDataObject(recentObjects.at(i)));
    }
    engine->rootContext()->setContextProperty("locationListModel", QVariant::fromValue(mLocationDataList));
    engine->rootContext()->setContextProperty("setNoResultBoxText","No recents to show!");

}

bool CategorySearchController::checkLowestPrice(QString price) {

    bool isLowestPrice = false;

    if (price.length() > 0) {

        if (mPriceLowest.length() > 0) {
            double currentLowestPrice = mPriceLowest.toDouble();
            double parameterPrice = price.toDouble();

            if (parameterPrice <= currentLowestPrice) {
                isLowestPrice = true;
            }

        }

    }

    return isLowestPrice;

}
bool CategorySearchController::isFavoriteExist(double latitude,double longitude) {

    // Check if a favorite file exists in local memory and favorite config
    PlaceManager* placeManager = PlaceManager::getInstance();

    return placeManager->favoriteExists(latitude, longitude);
}

void CategorySearchController::searchSDKRequest(QVariant paramater) {

    if(!mLocationDataList.isEmpty()){
        qDebug()<<"data cleared";
        mLocationDataList.clear();
    }
    mIsNavigationStarted = false;
    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();

    mCategoryInfo = paramater;
    //    ui->fuelWidget->setHidden(true);

    QJsonObject jsonObject = mCategoryInfo.value<QJsonObject>();

    // initilise settings object
    QSettings settings(SettingFileName, QSettings::IniFormat);
    settings.beginGroup("AppSettings");
    //QSettings settings(gOrganization, gApplication);

    // get current values and update UI
    //    QString searchRadius = settings.value(gSearchRadius).toString();
    //    QString currentLatitude = settings.value(gCurrentLatitude).toString();
    //    QString currentLongitude = settings.value(gCurrentLongitude).toString();
    QString searchRadius = settings.value(gSearchRadius,"30000").toString();
    QString currentLatitude = settings.value(gCurrentLatitude,"33.558898").toString();
    QString currentLongitude = settings.value(gCurrentLongitude,"-117.728973").toString();
    qDebug()<<"currentLatitude: " << currentLatitude << ", currentLongitude: " << currentLongitude;

    //    double lat = currentLatitude.toDouble();
    //    double lon = currentLongitude.toDouble();
    //    locationtoolkit::Coordinates userCoordinates(lat, lon);

    //    //    qDebug()<<"jsonObject for key isNearByPinLocation: "<<jsonObject["isNearByPinLocation"] << "BoolValue"<<jsonObject["isNearByPinLocation"].toBool();

    //    locationtoolkit::Coordinates centerCoordinates;

    //    if (jsonObject["isNearByPinLocation"].toBool()) {
    //        centerCoordinates.latitude = jsonObject["latitude"].toDouble();
    //        centerCoordinates.longitude = jsonObject["longitude"].toDouble();
    //    }
    //    else {
    //        centerCoordinates.latitude = lat;
    //        centerCoordinates.longitude = lon;
    //    }
    QString gpsfile = settings.value(gGpsFileName, "demo.gps").toString();
    QString gpsPath = LTKSampleUtil::GetResourceFolder() + "gpsfiles/" + gpsfile;

    double lat = currentLatitude.toDouble();
    double lon = currentLongitude.toDouble();
    locationtoolkit::Coordinates userCoordinates(lat, lon);
    locationtoolkit::Coordinates centerCoordinates;

    if(mMapViewController != NULL) {

        Place destination, origin;

        Coordinates mapCenter = mMapViewController->getMapCenter();
        GetDefaultDestinationAndOrigin(gpsPath.toStdString(), &destination, &origin);

        qDebug()<<"Path : "<<gpsPath<<" des "<<destination.GetLocation().center.latitude<<destination.GetLocation().center.longitude<<" org "<<origin.GetLocation().center.latitude<<origin.GetLocation().center.longitude;
        centerCoordinates = mapCenter;
        userCoordinates = origin.GetLocation().center;

        qDebug()<<"CategorySearchController::mainbar Center : ("<<mapCenter.latitude<<","<<mapCenter.longitude<<") user : ("<<userCoordinates.latitude<<","<<userCoordinates.longitude<<")";
    }
    else {
        if (jsonObject["isNearByPinLocation"].toBool()) {
            centerCoordinates.latitude = jsonObject["latitude"].toDouble();
            centerCoordinates.longitude = jsonObject["longitude"].toDouble();
        }
        else {
            centerCoordinates.latitude = lat;
            centerCoordinates.longitude = lon;
        }
    }

    QString categoryCode = jsonObject["code"].toString();

    //  Set categoryLabel size
    if (jsonObject["name"].toString() == "Fuel Stations") {
        //        ui->categoryLabel->setFixedSize(this->width()*0.25, ui->topBarWidget->height() - 18);
    }
    else {
        //        ui->categoryLabel->setFixedSize(this->width()*0.75, ui->topBarWidget->height() - 18);
    }

    //    ui->categoryLabel->setText(jsonObject["name"].toString());

    engine->rootContext()->setContextProperty("categoryTitle", jsonObject["name"].toString());

    locationtoolkit::SearchRequest* request = locationtoolkit::SingleSearchRequest::createRequestWithCategory(*GetLTKContext(), categoryCode, userCoordinates, centerCoordinates);
    connect(request, SIGNAL(OnSuccess(QSharedPointer<locationtoolkit::SearchInformation>)),
            this, SLOT(onSearchSuccess(QSharedPointer<locationtoolkit::SearchInformation>)));
    connect(request, SIGNAL(OnError(int)), this, SLOT(onSearchError(int)));
    connect(request, SIGNAL(OnProgressUpdated(int)), this, SLOT(onSearchProgressUpdated(int)));

    //  keep reference of base request to use later for creating load more request
    mSearchRequest = request;

    //  start request
    request->StartRequest();

    //  show loading indicator
    mWaitDialog->setVisible(true);

    settings.endGroup();

}
void CategorySearchController::switchToGoHomeButton(bool b) {

    //    if(b) {
    //        mGoToHome = true;
    //        QString filePath = LTKSampleUtil::GetResourceFolder() + "images/listview_resources/home.png";
    //        LTKSampleUtil::setButtonIcon(filePath, ui->backButton, ui->backButton->size()*0.8);
    //    }
    //    else {
    //        mGoToHome = false;
    //        QString filePath = LTKSampleUtil::GetResourceFolder() + "images/listview_resources/back_icon.png";
    //        LTKSampleUtil::setButtonIcon(filePath, ui->backButton, ui->backButton->size()*1.5);
    //    }

}
void CategorySearchController::setCustomFavLabels(QVariant variant) {

    //    hide();
    //    mAddCustomFavourite->selectedCustomFavPOI(variant);
    //    mAddCustomFavourite->show();
    //    mAddCustomFavourite->raise();

}
void CategorySearchController::setfavouriteRecentsFlag(bool fav_b, bool recent_b) {


    //    mRecentsPage = recent_b;
    //    mFavouritesPage = fav_b;
    //    ui->addBtn->hide();
    //    ui->deleteBtn->hide();
    //    ui->deleteAllButton->hide();

}
void CategorySearchController::OnSearchResultClicked(QJsonObject jsonObject) {
    qDebug()<<"CategorySearchController::OnSearchResultClicked....";
    
    //  Get :at long of the selected list item
    qDebug() << jsonObject["place"].toString().toDouble() << jsonObject["location"].toString().toDouble();
    locationtoolkit::Coordinates coordinate;
    QJsonObject json_location = jsonObject["place"].toObject()["location"].toObject();
    coordinate.latitude= json_location["latitude"].toDouble();
    coordinate.longitude = json_location["longitude"].toDouble();


    //Tempcode to be removed
    //bool mIsNavigationStarted = false;

    if(!mIsNavigationStarted) {
        // clear all the pins form the map
        mLTKMapKit->DeleteAllPins();

        //  Setup Pin selected/unselected images
        QPixmap *selectImage, *unselectImage;
        QString resourceFolder = LTKSampleUtil::GetResourceFolder();
        //        selectImage = new QPixmap(resourceFolder + "images/pin_end.png");
        //        unselectImage = new QPixmap(resourceFolder + "images/pin_start.jpg");
        selectImage = new QPixmap(resourceFolder + "images/pin_select.png");
        unselectImage = new QPixmap("images/pin_unselect.png");


        //  TODO: Remove in final publish - Temporary debugging
        QByteArray* byteBuff = new QByteArray();
        QBuffer* buffer = new QBuffer(byteBuff);
        buffer->open(QBuffer::WriteOnly);
        bool saved = selectImage->save(buffer, "JPEG");
        //qDebug()<<"saved pixel, saved:"<<saved <<" length="<<byteBuff->size();

        //  TODO: Remove in final publish - Temporary debugging
        QByteArray* byteBuff1 = new QByteArray();
        QBuffer* buffer1 = new QBuffer(byteBuff1);
        buffer1->open(QBuffer::WriteOnly);
        bool saved1 = selectImage->save(buffer1, "JPEG");
        //qDebug()<<"saved pixel, saved:"<<saved1 <<" length="<<byteBuff1->size();

        locationtoolkit::PinImageInfo selectedImage;
        selectedImage.SetPixmap(*selectImage);
        selectedImage.SetPinAnchor(50, 100);
        locationtoolkit::PinImageInfo unSelectedImage;
        unSelectedImage.SetPixmap(*unselectImage);
        unSelectedImage.SetPinAnchor(50, 100);
        locationtoolkit::RadiusParameters radiusPara(50, 0x6721D826);

        bool visible = true;
        locationtoolkit::Bubble* bubble = new CustomBubble();
        locationtoolkit::PinParameters pinpara(coordinate,
                                               selectedImage,
                                               unSelectedImage,
                                               radiusPara,
                                               "",
                                               "",
                                               bubble,
                                               visible);

        pinpara.SetSelectedImage(&selectedImage);
        pinpara.SetUnselectedImage(&unSelectedImage);

        //  Create the pin and move it to the updated lat long
        Pin* searchPin = mLTKMapKit->CreatePin(pinpara);
        locationtoolkit::CameraParameters param(coordinate);
        param.SetPosition( coordinate );
        param.SetZoomLevel(18);
        mLTKMapKit->SetGpsMode(locationtoolkit::MapWidget::GM_STANDBY,false);
        mLTKMapKit->MoveTo( param );

        //qDebug()<< "\n\n"<<"Searched Pin (Inside OnSearch Result Clicked )"<< mSearchedPinInfo;

        searchPin->SetSelected(true);

        //  For displaying a pin directly from a click on search list item
        //        showPinInfo(mSearchedPinInfo);
    }
}

void CategorySearchController::OnCategorySearchSelected(QVariant parameter,bool iNearByPinLocation) {

    //  Explicitly close all pop-up windows
    //closeAllPopUpWindows();

    QVariant parameterVariant;

    //qDebug()<<"mNearMeController->isNearByPinLocation: "<<mNearMeController->isNearByPinLocation;

    //  Re-Use: When Near this place button is clicked update search center to
    //  the lat long of the searched pin

    if (iNearByPinLocation) {
        // add current selected pin to parameter

        //QJsonObject jsonObject = mSearchedPinInfo.value<QJsonObject>();
        //QJsonObject jsonObject = currentItemDetails
        //QJsonObject json_location = jsonObject["place"].toObject()["location"].toObject();
        double location_lat= mCurrentItemDetails["placeLatitude"].toString().toDouble();
        double location_lon = mCurrentItemDetails["placeLongitude"].toString().toDouble();
        QJsonObject parameterObject = parameter.value<QJsonObject>();
        parameterObject["latitude"] = location_lat;
        parameterObject["longitude"] = location_lon;
        parameterObject["isNearByPinLocation"] = iNearByPinLocation;
        parameterVariant = parameterObject;
    }
    else {
        parameterVariant = parameter;
    }
    qDebug() << "parameterVariant " <<parameterVariant;

    //  Show the Category search controller
    //  Clear older pre populated list for re-use
    // mCategorySearchController->cleanUp();
    //  Show List Screen
    //  Call SDK for making a category search request
    searchSDKRequest(parameterVariant);
}
void CategorySearchController::GetPinInfo(const locationtoolkit::Pin* pin) {
    qDebug("A Pin is clicked!");


    if (!mIsNavigationStarted) {
        //  Close Side Menu
        //onToggleSideMenu(false);
        qDebug("A Pin is clicked!");

        //  Position Pin Information Widget
        //        mPinInformationWidget->move(0, height()- mPinInformationWidget->height());
        locationtoolkit::Pin* dyPin = const_cast<locationtoolkit::Pin*>(pin);
        locationtoolkit::Coordinates coord = dyPin->GetPosition();

        //qDebug() << QString::number(coord.latitude,'f',6);
        //qDebug() << QString::number(coord.longitude, 'f', 6);

        //setGPSFile(pin);

        //  Update Pin Information depending on the pin clicked from the displayed pins
        int index = updatePinInfo(coord.latitude,coord.longitude);

        //  Display the pin Information view with completed labels and images
        if(index != -1)
        {

            mDetailInfoView->setItemDetails((DataObject*)mLocationDataList.at(index), mSearchedJsonObjectVector.at(index));
            mDetailInfoView->show();
            mMapViewController->onSideItemVisibility(false);
            mToggleHeaderBar->setVisible(false);
            DataObject* itemDetail = (DataObject*)mLocationDataList.at(index);
            mMapViewController->setPOIPosition(QPointF(itemDetail->placeLatitude().toDouble(),itemDetail->placeLongitude().toDouble()),itemDetail->placeName());
            if(mDetailInfoView->isCatergoryPinDetails)
            {
                PlaceManager* placeManager = PlaceManager::getInstance();
                placeManager->setAppDirPath(mAppDirPath);
                placeManager->addToRecents(mSearchedJsonObjectVector.at(index));
            }
        }
        else
        {
            qDebug()<<"Pin Info Not Found";
        }

    }
}
//  During Map/List toggle find the pin from the plotted pins
int CategorySearchController::updatePinInfo(double latitude, double longitude) {

    QString c_lat = QString::number(latitude,'f',6);
    QString c_long = QString::number(longitude,'f',6);

    int index = -1;
    foreach(const QJsonObject jsonObject, mSearchedJsonObjectVector) {
        index++;
        //  Parse Json data to fetch information
        QJsonObject json_location = jsonObject["place"].toObject()["location"].toObject();

        double location_lat= json_location["latitude"].toDouble();
        double location_lon = json_location["longitude"].toDouble();

        QString t_lat = QString::number(location_lat,'f',6);
        QString t_long = QString::number(location_lon,'f',6);

        //  Iterate through the list to find the clicke pin
        if(t_lat == c_lat && t_long == c_long) {

            return index;
        }
    }
    return index;

}

void CategorySearchController::SetLtkMapkit(MapWidget *LTKMapkit)
{
    mLTKMapKit = LTKMapkit;
    mDetailInfoView->SetLtkMapkit(mLTKMapKit);
}
void CategorySearchController::show()
{
    this->setVisible(true);
    mSearchView->setVisible(true);
}
void CategorySearchController::hide()
{
    this->setVisible(false);
    mSearchView->setVisible(false);
}
void CategorySearchController::SetMapViewController(MapViewController* pMapViewController)
{
    mMapViewController = pMapViewController;
    connect(mDetailInfoView,SIGNAL(sideItemVisibility(bool)),mMapViewController,SLOT(onSideItemVisibility(bool)));

}
void CategorySearchController::onFuelSearchRequest(QVariant info)
{

    if(!mLocationDataList.isEmpty()){
        qDebug()<<"data cleared";
        mLocationDataList.clear();
    }
    mIsNavigationStarted = false;
    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();

    mCategoryInfo = info;
    //    ui->fuelWidget->setHidden(true);

    QJsonObject jsonObject = mCategoryInfo.value<QJsonObject>();

    double lat = jsonObject["lattitude"].toString().toDouble();
    double lon = jsonObject["longitude"].toString().toDouble();
    locationtoolkit::Coordinates userCoordinates(lat, lon);
    locationtoolkit::Coordinates centerCoordinates;

    if(mMapViewController != NULL) {

        Coordinates mapCenter = mMapViewController->getMapCenter();
        centerCoordinates = mapCenter;
    }

    QString categoryCode = jsonObject["code"].toString();

    engine->rootContext()->setContextProperty("categoryTitle", jsonObject["name"].toString());

    locationtoolkit::SearchRequest* request = locationtoolkit::SingleSearchRequest::createRequestWithCategory(*GetLTKContext(), categoryCode, userCoordinates, centerCoordinates);
    connect(request, SIGNAL(OnSuccess(QSharedPointer<locationtoolkit::SearchInformation>)),
            this, SLOT(onSearchSuccess(QSharedPointer<locationtoolkit::SearchInformation>)));
    connect(request, SIGNAL(OnError(int)), this, SLOT(onSearchError(int)));
    connect(request, SIGNAL(OnProgressUpdated(int)), this, SLOT(onSearchProgressUpdated(int)));

    //  keep reference of base request to use later for creating load more request
    mSearchRequest = request;

    //  start request
    request->StartRequest();

    //  show loading indicator
    mWaitDialog->setVisible(true);


}
