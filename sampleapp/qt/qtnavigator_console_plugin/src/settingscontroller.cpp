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

#include "settingscontroller.h"
#include<QQmlEngine>
#include<QQmlContext>
#include<QGuiApplication>
#include<QDirIterator>
//  SearchItem Controller Constructor with default initializations

SettingsController::SettingsController(QQuickItem* parent): 
    QQuickItem(parent),
    mSettingsView(NULL)
{

}

void SettingsController::setUp() {

    //  initilise settings object
    SettingFileName = LTKSampleUtil::GetResourceFolder()+"sampleapp.ini";
    //SettingFileName = "/home/soumen/Desktop/sampleapp.ini";
    //    QSettings::setPath(SettingFileName,settings.scope(), QSettings::IniFormat);

    QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
    QString appDirPath = engine->rootContext()->contextProperty("applicationDirPath").toString();
    QQmlComponent component(engine,QUrl(appDirPath+"/resource/SettingsController.qml"));
    mSettingsView  = qobject_cast<QQuickItem*>(component.create());

    QQmlEngine::setObjectOwnership(mSettingsView, QQmlEngine::CppOwnership);
    mSettingsView->setParentItem(this);
    mSettingsView->setParent(this);
    //SettingView->setFlag(ItemHasContents);
    QObject *saveButton = mSettingsView->findChild<QObject *>("objButton");
    if(saveButton && getClient() == "Neptune")
    {
        QQmlProperty:: write(saveButton, "anchors.bottomMargin", 0);
    }else if(saveButton && getClient() == "Automotive"){
        QQmlProperty:: write(saveButton, "anchors.bottomMargin", 150);
    }

    connect( mSettingsView, SIGNAL(settingControllerClosed()), this, SLOT(onCloseButtonClicked()) );
    connect( mSettingsView, SIGNAL(saveButtonClicked()), this, SLOT(onSaveButtonClicked()) );
    connect( mSettingsView, SIGNAL(tabIndexChanged(int)), this, SLOT(onTabIndexChanged(int)) );
    connect( mSettingsView, SIGNAL(clearData()),this,SLOT(onClearData()));

    initiliseUserSettings(0);
    initiliseUserSettings(1);
    initiliseUserSettings(2);
    initiliseUserSettings(3);
    flags[0] = true;
    flags[1] = flags[2] = flags[3] = false;


}
void SettingsController::setClient(QString p){
    mClient = p;
}


void SettingsController::onTabIndexChanged(int currentIndex) {

    QObject *clearButton = mSettingsView->findChild<QObject *>("cleanData");
    if(clearButton && currentIndex != 3)
    {
        QQmlProperty:: write(clearButton, "visible", false);
    }else{
        QQmlProperty:: write(clearButton, "visible", true);
    }
    if(flags[currentIndex] == false)
    {
        initiliseUserSettings(currentIndex);
        flags[currentIndex] = true;
    }
    qDebug() << "currentIndex: "  << currentIndex;

}

//  Default settings
void SettingsController::initiliseUserSettings(int currentIndex) {

    //  initilise settings object
    QSettings settings(LTKSampleUtil::GetResourceFolder() + gConfigFileName, QSettings::IniFormat);
    settings.beginGroup("AppSettings");
    qDebug() << "settings.fileName(): " << settings.fileName();

    if(currentIndex == 0) {

        QStringList showType = settings.value(gShowType,"Places on Map").toStringList();
        settings.setValue(gShowType, showType);

        QString defaultViewType = settings.value(gDefaultViewType,"Perspective").toString();
        settings.setValue(gDefaultViewType, defaultViewType);

        //  set show type settings

        setDefaultValue("chkSigns", showType);
        setDefaultValue("chkLandmarks", showType);
        setDefaultValue("chkCities", showType);
        setDefaultValue("chkFreeways", showType);
        setDefaultValue("chkPlaces", showType);

        //  default view type settings
        setDefaultValue("rbPerspective", defaultViewType);
        setDefaultValue("rb2DTripView", defaultViewType);
        setDefaultValue("rbListView", defaultViewType);

        //GPS files Drop Down change
        QQmlEngine *engine = QQmlEngine::contextForObject(this)->engine();
        //QString appDirPath = engine->rootContext()->contextProperty("applicationDirPath").toString();
        QString recentsJson = LTKSampleUtil::GetWorkFolder() + "/resource/gpsfiles";
        QDirIterator it(recentsJson, QStringList() << "*.gps", QDir::Files, QDirIterator::Subdirectories);
        while(it.hasNext()){
            fileStack.append(it.fileName());
            it.next();
        }
        fileStack.sort(Qt::CaseInsensitive);

        //this->rootContext()->setContextProperty("gpsFileTitle",fileStack);
        engine->rootContext()->setContextProperty("gpsFileTitle",fileStack);
        QString GpsFileName = settings.value(gGpsFileName,"startup.gps").toString();
        settings.setValue(gGpsFileName, GpsFileName);
        mPrevGPSFileName = GpsFileName;

        QObject *gpsFileEdit = mSettingsView->findChild<QObject *>("gpsCombo");
        settings.setValue(gGpsFileName, GpsFileName);
        if (gpsFileEdit){
            //QMetaObject::invokeMethod(gpsFileEdit,"selectedIndex",Q_RETURN_ARG(QVariant,returnedValue),Q_ARG(QVariant, GpsFileName));

            // gpsFileEdit->setProperty("currentIndex",returnedValue.toInt());
            gpsFileEdit->setProperty("defaultName",GpsFileName);

        }
    }

    else if(currentIndex == 1) {

        QString routeType = settings.value(gRouteType,"Fastest").toString();
        settings.setValue(gRouteType, routeType);

        QStringList avoidType = settings.value(gAvoidType,"").toStringList();
        settings.setValue(gAvoidType, avoidType);

        QStringList trafficType = settings.value(gTrafficType,"Navigation").toStringList();
        settings.setValue(gTrafficType, trafficType);

        setDefaultValue("rbFastest", routeType);
        setDefaultValue("rbShortest", routeType);
        //setDefaultValue("rbECO", routeType);

        setDefaultValue("chkTollRoads", avoidType);
        setDefaultValue("chkFerries", avoidType);
        setDefaultValue("chkHovLanes", avoidType);
        setDefaultValue("chkHighways", avoidType);

        setDefaultValue("chkAlerts", trafficType);
        setDefaultValue("chkNavigation", trafficType);

    }

    else if(currentIndex == 2) {

        //  get current values and update UI
        QString searchRadius = settings.value(gSearchRadius,"30000").toString();
        qDebug() << "Radius: " << searchRadius;
        settings.setValue(gSearchRadius, searchRadius);

        QString currentLatitude = settings.value(gCurrentLatitude,"33.558898").toString();
        qDebug() << "currentLatitude: " << currentLatitude;
        settings.setValue(gCurrentLatitude, currentLatitude);

        QString currentLongitude = settings.value(gCurrentLongitude,"-117.728973").toString();
        settings.setValue(gCurrentLongitude, currentLongitude);
        qDebug() << "currentLongitude: " << currentLongitude;
        QString distanceType = settings.value(gDistanceType,"miles").toString();
        settings.setValue(gDistanceType, distanceType);


        qDebug() << "Radius: " << searchRadius;

        QObject *radiusLineEdit = mSettingsView->findChild<QObject *>("radiusLineEdit");
        QObject *latitudeLineEdit = mSettingsView->findChild<QObject *>("latitudeLineEdit");
        QObject *longitudeLineEdit = mSettingsView->findChild<QObject *>("longitudeLineEdit");

        //radiusLineEdit->setProperty("text",searchRadius);
        QQmlProperty:: write(radiusLineEdit, "text", searchRadius);
        QQmlProperty:: write(latitudeLineEdit, "text", currentLatitude);
        QQmlProperty:: write(longitudeLineEdit, "text", currentLongitude);

        setDefaultValue("rbMiles", distanceType);
        setDefaultValue("rbKms", distanceType);
    }

    else if(currentIndex == 3) {

        QString fuelType = settings.value(gFuelType,"Regular").toString();
        settings.setValue(gFuelType, fuelType);

        setDefaultValue("rbDiesel", fuelType);
        setDefaultValue("rbRegular", fuelType);
        setDefaultValue("rbMidGrage", fuelType);
        setDefaultValue("rbPremium", fuelType);

        mDefaultLanguage = settings.value(gLanguage,"en_US").toString();
        qDebug()<<"init settings Default Language :"<<mDefaultLanguage;

        if(mDefaultLanguage=="zh_CN") {
            setDefaultValue("rbChinese", "Chinese");
        }
        else {
            setDefaultValue("rbEnglish", "English");
        }
    }

    settings.endGroup();
}

void SettingsController :: setDefaultValue(QString name, QStringList savedValue){

    QObject *pName = mSettingsView->findChild<QObject *>(name);
    QString pValue =  QQmlProperty::read(pName, "text").toString();
    foreach (QString value, savedValue) {
        qDebug() << "value " << value << "pValue " << pValue;

        if (pValue == value) {
            QQmlProperty:: write(pName, "checked", true);
        }
    }
}

void SettingsController :: setDefaultValue(QString name, QString savedValue){

    QObject *pName = mSettingsView->findChild<QObject *>(name);
    QString pValue =  QQmlProperty::read(pName, "text").toString();
    qDebug()<<"setDefaultValue pValue:"<<pValue<<savedValue;
    if (pValue == savedValue) {
        QQmlProperty:: write(pName, "checked", true);
    }
}

QStringList SettingsController :: getSelectedValue(QString name, QStringList savedValue) {

    QObject *pName = mSettingsView->findChild<QObject *>(name);
    bool bValue = QQmlProperty::read(pName, "checked").toBool();
    if (bValue) {
        qDebug() << QQmlProperty::read(pName, "text").toString();
        //return QQmlProperty::read(pName, "text").toString();
        savedValue.append(QQmlProperty::read(pName, "text").toString());
    }
    return savedValue;
}

QString SettingsController :: getSelectedValue(QString name) {

    QObject *pName = mSettingsView->findChild<QObject *>(name);
    bool bValue = QQmlProperty::read(pName, "checked").toBool();
    if (bValue) {
        qDebug() << QQmlProperty::read(pName, "text").toString();
        return QQmlProperty::read(pName, "text").toString();
    }
    return "";
}

//  Register User settings
void SettingsController::saveUserSettings() {

    //  initilise settings object

    QSettings settings(LTKSampleUtil::GetResourceFolder() + gConfigFileName, QSettings::IniFormat);
    settings.beginGroup("AppSettings");
    qDebug() << "Savesettings.fileName(): " << settings.fileName();


    qDebug()<<"*******Begin saveUserSettings********";

    //Navigation Tab data save

    QStringList showSelection;
    showSelection = getSelectedValue("chkSigns", showSelection);
    showSelection = getSelectedValue("chkLandmarks", showSelection);
    showSelection = getSelectedValue("chkCities", showSelection);
    showSelection = getSelectedValue("chkFreeways", showSelection);
    showSelection = getSelectedValue("chkPlaces", showSelection);

    settings.setValue(gShowType,showSelection);

    QStringList defaultGroupView;
    defaultGroupView.append("rbPerspective");
    defaultGroupView.append("rb2DTripView");
    defaultGroupView.append("rbListView");

    foreach (QString value, defaultGroupView) {

        QString checkedValue = getSelectedValue(value);
        if (checkedValue != "") {
            //QQmlProperty:: write(value, "checked", true);
            settings.setValue(gDefaultViewType, checkedValue);
            break;
        }
    }

    //get GPS filename from Drop Down.
    QString GpsFileName;
    QObject *gpsFileEdit = mSettingsView->findChild<QObject *>("gpsCombo");
    if (gpsFileEdit)
    {
        GpsFileName=  gpsFileEdit->property("currentText").toString();
        settings.setValue(gGpsFileName, GpsFileName);
    }
    if(GpsFileName != mPrevGPSFileName)
    {
        emit changedCentreCoordinate(GpsFileName);
    }


    // Set Route Tab Data

    QStringList routeGroupView;
    routeGroupView.append("rbFastest");
    routeGroupView.append("rbShortest");
    //routeGroupView.append("rbECO");

    foreach (QString value, routeGroupView) {
        QString checkedValue = getSelectedValue(value);
        if (checkedValue != "") {
            settings.setValue(gRouteType, checkedValue);
            break;
        }
    }

    // Set AvoidGroup data
    QStringList avoidSelection;
    avoidSelection = getSelectedValue("chkTollRoads", avoidSelection);
    avoidSelection = getSelectedValue("chkFerries", avoidSelection);
    avoidSelection = getSelectedValue("chkHovLanes", avoidSelection);
    avoidSelection = getSelectedValue("chkHighways", avoidSelection);

    settings.setValue(gAvoidType,avoidSelection);

    // Set Traffic Group Data

    QStringList trafficGroup;
    trafficGroup = getSelectedValue("chkAlerts", trafficGroup);
    trafficGroup = getSelectedValue("chkNavigation", trafficGroup);

    settings.setValue(gTrafficType, trafficGroup);

    // Set Search tab Data

    QObject *radiusLineEdit = mSettingsView->findChild<QObject *>("radiusLineEdit");
    QObject *latitudeLineEdit = mSettingsView->findChild<QObject *>("latitudeLineEdit");
    QObject *longitudeLineEdit = mSettingsView->findChild<QObject *>("longitudeLineEdit");

    qDebug() << QQmlProperty::read(radiusLineEdit, "text");

    //  get current values and update UI
    settings.setValue(gSearchRadius, QQmlProperty::read(radiusLineEdit, "text"));
    settings.setValue(gCurrentLatitude, QQmlProperty::read(latitudeLineEdit, "text"));
    settings.setValue(gCurrentLongitude, QQmlProperty::read(longitudeLineEdit, "text"));

    QStringList distanceGroupView;
    distanceGroupView.append("rbMiles");
    distanceGroupView.append("rbKms");

    foreach (QString value, distanceGroupView) {
        QString checkedValue = getSelectedValue(value);
        if (checkedValue != "") {
            settings.setValue(gDistanceType, checkedValue);
            break;
        }
    }

    // Set Misc Tab Data
    QStringList fuelType;
    fuelType.append("rbDiesel");
    fuelType.append("rbRegular");
    fuelType.append("rbMidGrage");
    fuelType.append("rbPremium");

    foreach (QString value, fuelType) {
        QString checkedValue = getSelectedValue(value);
        if (checkedValue != "") {
            settings.setValue(gFuelType, checkedValue);
            break;
        }
    }

    QStringList languageGroupView;
    languageGroupView.append("rbEnglish");
    languageGroupView.append("rbChinese");

    mSetLanguage = mDefaultLanguage;

    foreach (QString value, languageGroupView) {

        QString checkedValue = getSelectedValue(value);

        if(checkedValue == "Chinese") {
            mSetLanguage = "zh_CN";
            settings.setValue(gLanguage, mSetLanguage);
            break;
        }
        else if(checkedValue == "English") {
            mSetLanguage ="en_US";
            settings.setValue(gLanguage, mSetLanguage);
            break;
        }
    }
    
    qDebug()<<"saveUserSettings setLanguage : "<<mSetLanguage;

    //  make sure setting is synced

    settings.sync();
    settings.endGroup();

    qDebug()<<"*******End saveUserSettings********";

}
//  Utility function for save button click
void SettingsController::onSaveButtonClicked() {

    // save settings
    saveUserSettings();

    qDebug()<<"onSaveButtonClicked "<<mDefaultLanguage<<" "<<mSetLanguage;

    if(QString::compare(mDefaultLanguage,mSetLanguage,Qt::CaseInsensitive)){
        QString errorString1,errorString2;
        errorString1 = QString("");
        errorString2 = QString( "Relaunch the app again!");
        mMessageBox = new Messageboxcontroller();
        QQmlEngine::setObjectOwnership(mMessageBox, QQmlEngine::CppOwnership);
        mMessageBox->setParentItem(mSettingsView);
        QQmlEngine::setContextForObject(mMessageBox, QQmlEngine::contextForObject(this));
        mMessageBox->setWidth(width());
        mMessageBox->setHeight(height());
        mMessageBox->messageStrings(errorString1,errorString2);
        mMessageBox->setUp();
        connect( mMessageBox, SIGNAL(okClicked()), this, SLOT(onOkButtonClicked()) );
    }else{
        // hide viewController
        this->setVisible(false);

        // emit signal for settings updated
        emit settingsUpdated();
    }
}

void SettingsController::onCloseButtonClicked() {

    //hide();
    this->setVisible(false);
}

void SettingsController ::onClearData(){
    qDebug() << "SettingsController::clearData.........";
    mConfirmBox = new ConfirmBoxController();
    QQmlEngine::setObjectOwnership(mConfirmBox, QQmlEngine::CppOwnership);
    mConfirmBox->setParentItem(mSettingsView);
    QQmlEngine::setContextForObject(mConfirmBox, QQmlEngine::contextForObject(this));
    mConfirmBox->setWidth(width());
    mConfirmBox->setHeight(height());
    QString var1,var2;
    var1="Clear all";
    var2="Data";
    mConfirmBox->messageStrings(var1,var2);
    mConfirmBox->setUp();
    connect( mConfirmBox, SIGNAL(confirmBoxNotification(bool)), this, SLOT(onConfirmDeleteClicked(bool)) );
}
bool SettingsController::onConfirmDeleteClicked(bool flag){
    if(flag){
        mConfirmBox->setVisible(false);
        if(remove_files()){

            QString errorString1,errorString2;
            errorString1 = QString("");
            errorString2 = QString( "Re Launch the app again!");
            mMessageBox = new Messageboxcontroller();
            QQmlEngine::setObjectOwnership(mMessageBox, QQmlEngine::CppOwnership);
            mMessageBox->setParentItem(mSettingsView);
            QQmlEngine::setContextForObject(mMessageBox, QQmlEngine::contextForObject(this));
            mMessageBox->setWidth(width());
            mMessageBox->setHeight(height());
            mMessageBox->messageStrings(errorString1,errorString2);
            mMessageBox->setUp();
            connect( mMessageBox, SIGNAL(okClicked()), this, SLOT(onOkButtonClicked()) );
        }
        else{
            mConfirmBox->setVisible(false);
        }
        delete mConfirmBox;
        mConfirmBox=NULL;
    }
}
void SettingsController::onOkButtonClicked(){
    //    mMessageBox->setVisible(false);
    delete mMessageBox;
    mMessageBox = NULL;


}
bool SettingsController::remove_files(){
    QDir dir(LTKSampleUtil::GetWorkFolder()+"mapKit3Dcontent");
    dir.removeRecursively();
    dir.remove(LTKSampleUtil::GetWorkFolder() + "off_board_metadata_source");
    dir.remove(LTKSampleUtil::GetWorkFolder() + "on_board_metadata_source");
    QDir dir1(LTKSampleUtil::GetWorkFolder() + "../cluster/mapKit3Dcontent");
    dir1.removeRecursively();
    dir1.remove(LTKSampleUtil::GetWorkFolder() + "../cluster/off_board_metadata_source");
    dir1.remove(LTKSampleUtil::GetWorkFolder() + "../cluster/on_board_metadata_source");
    return true;
}
SettingsController::~SettingsController()
{
    if(mSettingsView != NULL) {
        delete mSettingsView;
        mSettingsView = NULL;
    }
}

