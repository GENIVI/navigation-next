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

#ifndef SETTINGSCONTROLLER_H
#define SETTINGSCONTROLLER_H


#include <QQuickItem>
#include <QSettings>
#include <QQmlProperty>
#include <QJsonArray>
#include "util.h"
#include "globalsetting.h"
#include <QSettings>
#include "confirmboxcontroller.h"
#include "messageboxcontroller.h"
//const static QString gOrganization = "TCS";
//const static QString gApplication = "NavigationSample";
//const static QString gSearchRadius = "searchRadius";
//const static QString gCurrentLatitude = "currentLatitude";
//const static QString gCurrentLongitude = "currentLongitude";
//const static QString gFuelType = "fuelType";
//const static QString gShowType = "showType";
//const static QString gDefaultViewType = "defaultViewType";
//const static QString gRouteType = "routeType";
//const static QString gAvoidType = "avoidType";
//const static QString gTrafficType = "trafficType";
//const static QString gDistanceType = "distanceType";


class SettingsController : public QQuickItem
{
    Q_OBJECT

    bool flags[4];
    QSettings settings;
    QQuickItem* mSettingsView;
    QString mDefaultLanguage;
    QString mSetLanguage;
    QString mClient;
signals:
    void settingsUpdated();
    void changedCentreCoordinate(QString);

public:
    SettingsController(QQuickItem* parent = 0);
    ~SettingsController();

    void setUp();
    void initiliseUserSettings(int);
    void saveUserSettings();
    void setClient(QString p);
    QString getClient(){return mClient;}
    QStringList getSelectedValue(QString, QStringList);
    QString getSelectedValue(QString);
    void setDefaultValue(QString, QStringList);
    void setDefaultValue(QString, QString);
    // QJsonArray fileStack;
    QStringList fileStack;
    //QSettings *settings;
    QString SettingFileName;
    QString mPrevGPSFileName;
    ConfirmBoxController *mConfirmBox;
    Messageboxcontroller *mMessageBox;
    bool remove_files();
private slots:
    //  Member functions for button clicks
    void onCloseButtonClicked();
    void onSaveButtonClicked();
    void onTabIndexChanged(int);
    void onClearData();
    bool onConfirmDeleteClicked(bool flag);
    void onOkButtonClicked();
};

#endif // SETTINGSCONTROLLER_H
