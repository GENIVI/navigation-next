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

#include <QQmlEngine>
#include <QQmlContext>
#include <QQmlProperty>

#include "mainviewcontroller.h"
#include "util.h"
#include "globalsetting.h"
#include "hamburgermenucontroller.h"
#include "titlebarcontroller.h"
#include "settingscontroller.h"
#include "mapviewcontroller.h"
#include "categorysearchcontroller.h"

MainViewController::MainViewController(QQuickItem *parent):
    QQuickItem(parent),
    mInitDone(false),
    mZoom(17.0f),
    mTilt(90.0f),
    mClient(""),
    mCategorySearchController(NULL)
{
    setFlag(ItemHasContents, true);
    qDebug()<<"mainviewConstructor";
}

MainViewController::~MainViewController()
{
}

QSGNode* MainViewController::updatePaintNode(QSGNode *n, UpdatePaintNodeData *d)
{
    if (!mInitDone)
    {
        QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
    }
    return QQuickItem::updatePaintNode(n, d);
}

void MainViewController::init()
{
    qDebug("MapView::init Width : %lf Height : %lf\n", width(), height());
    GlobalSetting::InitInstance();
    GlobalSetting& settings = GlobalSetting::GetInstance();

    // set token
    if(mToken.length() > 0) {
        settings.setToken(mToken);
    }

    locationtoolkit::LTKContext* ltkContext = GetLTKContext();

    QSettings setting(LTKSampleUtil::GetResourceFolder() + gConfigFileName, QSettings::IniFormat);
    setting.beginGroup("hybrid");
    QString dataPath = setting.value(gOnBoardDataPath).toString();
    bool IsOnBoard = setting.value("onBoard","false").toBool();

    qDebug()<<"MainViewController::init::"<<dataPath<<IsOnBoard;
    if(IsOnBoard)
        InitLTKHybridManager(IsOnBoard, dataPath);
    else
        InitLTKHybridManager(IsOnBoard);

    setting.endGroup();

    mInitDone = true;

    // Add TitleBar

    qDebug()<<"MainViewController::init add TitleBar start";
    TitleBarController *mTitleController = new TitleBarController(this);

    QQmlEngine::setObjectOwnership(mTitleController, QQmlEngine::CppOwnership);
    mTitleController->setParentItem(this);
    QQmlEngine::setContextForObject(mTitleController, QQmlEngine::contextForObject(this));

    mTitleController->setWidth(width());
    mTitleController->setHeight(height()/9);

    mTitleController->setUp();
    mTitleController->setVisible(true);
    qDebug()<<"MainViewController::init add TitleBar stop";

    // Add MapView

    qDebug()<<"MainViewController::init add MapView start";
    mMapController = new MapViewController(this);

    mMapController->setWidth(width());
    mMapController->setHeight(height()-height()/9);
    //mMapController->setTilt(tilt());
    //mMapController->setZoom(zoom());

    QPoint p(-117.689, 33.604);
    mMapController->setAvatarPosition(p);
    mMapController->setAvatarHeading(0.0);


    QQmlEngine::setObjectOwnership(mMapController, QQmlEngine::CppOwnership);
    mMapController->setParentItem(this);
    QQmlEngine::setContextForObject(mMapController, QQmlEngine::contextForObject(this));
    //QQmlEngine::setContextForObject(mTitleController, QQmlEngine::contextForObject(this));

    mMapController->setProperty("x", 0);
    mMapController->setProperty("y", height()/9);

    //mTitleController->setUp();
    mMapController->setVisible(true);
    qDebug()<<"MainViewController::init add MapView stop";

    // Connect
    connect(mTitleController, SIGNAL(onSettingsClicked()), this, SLOT(onSettingsButtonClicked()) );
    connect(mTitleController, SIGNAL(onSearchButtonClicked(QString)), this, SLOT(onSearchButtonClicked(QString)) );
    connect(mTitleController,SIGNAL(onMenuClicked()),this,SLOT(onMenuClicked()));
    connect(mTitleController,SIGNAL(hideAllPopUp()),this,SIGNAL(HideSideMenu()));

    connect(this,SIGNAL(ToggleTitleBarVisibility(bool)),mTitleController,SLOT(onToggleTitleBarVisibility(bool)));
    connect(mMapController,SIGNAL(ToggleTitleBarVisibility(bool)),mTitleController,SLOT(onToggleTitleBarVisibility(bool)));
    //    InitializeCategoryController();

    mMapController->init();

    InitializeCategoryController();
    mMapController->setUp_sideBarItems();

    mLTKMapKit = mMapController->globalMapWidget;
    mCategorySearchController->SetMapViewController(mMapController);
    mCategorySearchController->SetLtkMapkit(mLTKMapKit);

}

void MainViewController::setWorkFolder(QString path)
{
    qDebug("MapView::SetWorkFolder %s", path.toStdString().c_str());
    LTKSampleUtil::SetWorkFolder(path);

    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
    engine->rootContext()->setContextProperty("applicationDirPath", path);

    qDebug("MainViewController::setWorkFolder setContextProperty completed %s", engine->rootContext()->contextProperty("applicationDirPath").toString().toStdString().c_str());
}

QString MainViewController::workFolder()
{
    return LTKSampleUtil::GetWorkFolder();
}

float MainViewController::tilt()
{
    return mTilt;
}

void MainViewController::setTilt(float t)
{
    mTilt = t;
}

float MainViewController::zoom()
{
    return mZoom;
}

void MainViewController::setZoom(float t)
{
    mZoom = t;
}

QString MainViewController::token()
{
    return mToken;
}

void MainViewController::setToken(QString t)
{
    mToken = t;
}

QString MainViewController::client()
{
    return mClient;
}

void MainViewController::setClient(QString t)
{
    mClient = t;
}

bool MainViewController::isFollowMe()
{
    return mIsFollowMe;
}

void MainViewController::setIsFollowMe(bool f)
{
    mIsFollowMe = f;
}

void MainViewController::onMenuClicked()
{
    HamburgerMenuController *mMenuController = new HamburgerMenuController(/*width()*/);

    QQmlEngine::setObjectOwnership(mMenuController, QQmlEngine::CppOwnership);
    QQmlEngine::setContextForObject(mMenuController, QQmlEngine::contextForObject(this));
    
    mMenuController->setParentItem(this);
    mMenuController->setWidth(width()/5);
    mMenuController->setHeight(height());
    mMenuController->setParentWidth(width());
    mMenuController->setUp();
    mMenuController->setVisible(true);

    connect(this,SIGNAL(HideSideMenu()),mMenuController,SLOT(CloseAllPopUpWindows()));
    //connect(mMapController,SIGNAL(MapClicked()),mMenuController,SLOT(CloseAllPopUpWindows()));
    connect(mMenuController,SIGNAL(CategorySearchSelected(QVariant,bool)),mCategorySearchController,SLOT(OnCategorySearchSelected(QVariant,bool)));
    connect(mMenuController,SIGNAL(CategorySearchSelected(QVariant)),mCategorySearchController,SLOT(searchSDKRequest(QVariant)));
    connect(mMenuController,SIGNAL(FavouriteButtonClicked()),mCategorySearchController,SLOT(displayExistingFavourites()));
    connect(mMenuController,SIGNAL(RecentButtonClicked()),mCategorySearchController,SLOT(displayExistingRecents()));
    connect(mMapController,SIGNAL(HideSideMenu()),mMenuController,SLOT(CloseAllPopUpWindows()));
}

void MainViewController::onSettingsButtonClicked()
{
    emit HideSideMenu();

    SettingsController *mSettingController = new SettingsController(this);

    QQmlEngine::setObjectOwnership(mSettingController, QQmlEngine::CppOwnership);
    mSettingController->setParentItem(this);
    QQmlEngine::setContextForObject(mSettingController, QQmlEngine::contextForObject(this));
    
    mSettingController->setWidth(width());
    mSettingController->setHeight(height());

    mSettingController->setClient(client());
    mSettingController->setUp();
    mSettingController->setVisible(true);
    connect(mSettingController,SIGNAL(changedCentreCoordinate(QString)),mMapController,SLOT(onChangedCentreCoordinate(QString)));
}

void MainViewController::onSearchButtonClicked(const QString& searchString) {


    if(searchString.length() > 2) {


        //emit getUpdatedLocation(location);
        emit HideSideMenu();

        qDebug() << "suggested search for: "<<searchString;
        QJsonObject categoryParameter;
        categoryParameter["name"] = "all categories";
        categoryParameter["code"] = "";
        QVariant parameter = categoryParameter;
        //  Show List Screen
        //  Call SDK for making a category search request

        mCategorySearchController->mainBarSearchSDKRequest(parameter,searchString);
        mCategorySearchController->setVisible(true);
    }
}
void MainViewController::InitializeCategoryController()
{

    mCategorySearchController = new CategorySearchController();

    QQmlEngine::setObjectOwnership(mCategorySearchController, QQmlEngine::CppOwnership);
    mCategorySearchController->setParentItem(this);
    QQmlEngine::setContextForObject(mCategorySearchController, QQmlEngine::contextForObject(this));

    mCategorySearchController->setWidth(width());
    mCategorySearchController->setHeight(height());
    mCategorySearchController->setUp();
    //mCategorySearchController->SetMapViewController(mMapController);



    connect(mCategorySearchController,SIGNAL(ToggleTitleBarVisibility(bool)),this,SIGNAL(ToggleTitleBarVisibility(bool)));
    connect(mCategorySearchController,SIGNAL(SignalNavigationButtonClicked(QString, int, QVariant)),mMapController,SLOT(OnStartNavigationToDestination(QString, int, QVariant)));
    connect(mMapController,SIGNAL(SignalPinInfoClicked(const locationtoolkit::Pin*)),mCategorySearchController,SLOT(GetPinInfo(const locationtoolkit::Pin*)));
    connect(mMapController,SIGNAL(fuelSearchRequest(QVariant)),mCategorySearchController,SLOT(onFuelSearchRequest(QVariant)));

    // mCategorySearchController->SetLtkMapkit(mLTKMapKit);

}
