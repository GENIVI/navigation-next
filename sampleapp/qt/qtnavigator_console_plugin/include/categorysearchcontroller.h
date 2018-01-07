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

#ifndef CATEGORYSEARCHCONTROLLER_H
#define CATEGORYSEARCHCONTROLLER_H
#include <QQuickItem>
#include <QQmlContext>
#include<QQmlEngine>
#include <QDebug>
#include <QVariant>
#include <QtCore>
#include <QString>
#include <QFontDatabase>

#include "mapwidget.h"
#include "ltkhybridmanager.h"
#include "util.h"
#include "ltkerror.h"
#include "addresssearchrequest.h"
#include "searchdatatypes.h"
#include "searchrequest.h"
#include "searchreversegeocoderequest.h"
#include "singlesearchrequest.h"
#include "suggestionsearchrequest.h"
#include "datatransformer.h"
#include "dataobject.h"

#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include "messageboxcontroller.h"
#include "searchdetailscontroller.h"
#include "waitdialog.h"
#include "confirmboxcontroller.h"
#include "mapviewcontroller.h"

enum SearchType{
    NONE=0,
    IN_SEARCH=1,
    IN_FAV=2,
    IN_RECENTS=3
};
class   CategorySearchController : public QQuickItem
{
    Q_OBJECT
    QQuickItem* mSearchView;
public:
    explicit CategorySearchController(QQuickItem* parent = 0);
    ~CategorySearchController();
    //  Functions
    void setUp();

    void setfavouriteRecentsFlag(bool fav_b, bool recent_b);
    //   void hideAddCustomFavouriteScreen(bool hideScreen);
    void setCustomFavLabels(QVariant variant);
    void showSingleListItemWidget(QString textTitle,QString textMessage);
    void switchToGoHomeButton(bool b);


    QVector<QString>readExsistingFavourite(QString);
    QVector<QString>readExsistingRecents(QString);



    void SetLtkMapkit(MapWidget*);
    void SetMapViewController(MapViewController* pMapViewController);/* { mMapViewController = pMapViewController; }*/

    void categorySearchResultClicked(QVariant parameter);
    void categoryAddLocationPins(QJsonArray location);   //to be called in mainview
    //   void toggleMapViewFromCategory();
    //   void deleteButtonClicked(bool mSwitchDelete);
    //   void deleteAllButtonClicked(QVector<bool> favouriteAndRecents);
    //   void foundCustomFavourite(QVariant variant);
    //   void deleteSelectiveFav(QVariant itemData);
    //   void backButtonClicked(QString arg = "");
    //   void deleteAllPins();
    //   void addCustomFavClicked();
    //   void closeAllScreens();
    void pinNavigationClicked(QString, int, QVariant);   //navigation to be called in mainview
    void loadListDetailScreen(DataObject *);         //Next level...pininfo
    void show();
    void hide();
    SearchType eSearchType;
private slots:
    //  Button Click functions
    void on_backButton_clicked();
    //   void on_deleteBtn_clicked();
    void on_mapToggleButton_clicked();
    //   void on_addBtn_clicked();
    //   void on_deleteAllButton_clicked();
    //   void on_sortButton_clicked();
    //   void showSortOptions(bool isSorted);
    //   void onSortOptionSelected(QString selectedOption);

    //  Process Functions

    void onStartNavigationClicked(int index);
    void onLoadListDetail(int itemData);
    //void onSaveFavouriteClicked(QVariant variant);
    ////        void onMessageButtonClicked(QPushButton* button);
    void onSearchSuccess(QSharedPointer<locationtoolkit::SearchInformation> info);
    void onSearchError(int errorCode);
    void onSearchProgressUpdated(int percentage);
    void loadMoreSearchSDKRequest();
    //void showMapForList(bool b);
    bool checkLowestPrice(QString price);
    bool isFavoriteExist(double latitude,double longitude);
    //  Event method
    // //       bool eventFilter(QObject* object, QEvent* event);

    void onClearAllResentsClicked();
    void onClearAllFavouritesClicked();
    void onDeleteSelectiveFav(QString latitude,QString longitude);

    bool onConfirmRecentsValueClicked(bool flag);
    bool onConfirmFavouritesValueClicked(bool flag);
    bool onConfirmSelectedFavouriteValueClicked(bool flag);
    //  Search Controller slots
    void OnSearchResultClicked(QJsonObject variant);

    void onOkButtonClicked();

    void OnCategorySearchSelected(QVariant,bool);
    void displayExistingFavourites(bool showDeleteRed = false);
    void displayExistingRecents(bool showDeleteRed = false);
public slots:
    void mainBarSearchSDKRequest(QVariant parameter,QString searchText);
    void searchSDKRequest(QVariant parameter);
    void cleanUp();

protected:
    // //       void resizeEvent(QResizeEvent* event);
    ////        void hideEvent(QHideEvent *event);

private:

    //  Custom UI pop over screens
    waitdialog*     mWaitDialog;
    Messageboxcontroller* mMessageBox;
    SerachDetailsController* mDetailInfoView;
    MapWidget* mLTKMapKit;
    MapViewController* mMapViewController;

    //  Member variables
    QString mIteratorStatus;
    QVariant mCategoryInfo;

    locationtoolkit::SearchRequest* mSearchRequest;
    QSharedPointer<locationtoolkit::SearchInformation> mSearchInfo;
    QString mPriceLowest;
    QString mPriceAverage;
    QSize* mDimension;
    //QJsonArray mLocationPins_10;
    ConfirmBoxController *mConfirmBox;
    //  Member flags
    bool mMapView = false;
    bool mFavouritesPage = false;
    bool mRecentsPage = false;
    bool mSwitchDelete = false;
    bool mGoToHome = false;
    bool mSorted = false;

    QString SettingFileName;
    //  Ui modular design dimension variables
    int mSearchWidgetItemWidth;
    int mSearchWidgetItemHeight;
    int mSearchWidgetItemLoadMoreHeight;
    QList<QObject*> mLocationDataList;

    QString mTempLatitude;
    QString mTempLongitude;

    QJsonObject mCurrentItemDetails;
    //DataObject *mPinInfo;
    QString mAppDirPath;

    QQuickItem* mToggleHeaderBar;
    //QJsonArray            searchedLocations;
    bool mIsNavigationStarted;
    // static pin locations for getting bounds
    qreal mMinPoiPinLat;
    qreal mMinPoiPinLon;
    qreal mMaxPoiPinLat;
    qreal mMaxPoiPinLon;
    //QVariant              mSearchedPinInfo;

    void adjustMapBoundingBox();
    void InitializeToggleBar();
    int updatePinInfo(double latitude, double longitude);
    void showPinInfo(QJsonObject obj1);
    DataObject* getDataObject(QJsonObject,QString type = "");
    QVector<QJsonObject>mSearchedJsonObjectVector;

Q_SIGNALS:
    //close pop-up signal
    void SignalCloseAllPopUpWindows();
    void ToggleTitleBarVisibility(bool);
    void SignalNavigationButtonClicked(QString, int, QVariant);
    void SetListViewFocus();

public slots:
    void onCloseAllPopUpWindows();
    void onToggleMapViewFromCategory();
    void onSetVisible();
    void OnAddLocationPins(QJsonObject locations);
    void onToggleHeaderBarBackButtonClicked();
    void onNavigationButtonClicked(QString, int, QVariant);
    void GetPinInfo(const locationtoolkit::Pin* pin);
    void onFuelSearchRequest(QVariant);


};
#endif // CATEGORYSEARCHCONTROLLER_H
